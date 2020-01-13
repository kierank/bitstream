/*****************************************************************************
 * 35.h: SCTE 35 Digital Program Insertion Cueing Message for Cable
 *****************************************************************************
 * Copyright (C) 2015 VideoLAN
 *
 * Authors: Christophe Massiot <massiot@via.ecp.fr>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject
 * to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *****************************************************************************/

/*
 * Normative references:
 *  - SCTE 35 2013 (Digital Program Insertion Cueing Message for Cable)
 */

#ifndef __BITSTREAM_SCTE_35_H__
#define __BITSTREAM_SCTE_35_H__

#include <bitstream/common.h>
#include <bitstream/mpeg/psi/psi.h>
#include <bitstream/mpeg/psi/descriptors.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************
 * Splice Information Table
 *****************************************************************************/
#define SCTE35_TABLE_ID                 0xfc
#define SCTE35_HEADER_SIZE              (PSI_HEADER_SIZE + 11)
#define SCTE35_HEADER2_SIZE             2

static inline void scte35_init(uint8_t *p_scte35)
{
    psi_set_tableid(p_scte35, SCTE35_TABLE_ID);
    psi_init(p_scte35, false);
    p_scte35[1] &= ~0x40; /* private indicator */
    p_scte35[3] = 0;
    p_scte35[4] = 0;
    p_scte35[9] = 0;
    p_scte35[10] = 0xff;
    p_scte35[11] = 0xf0;
}

static inline uint8_t scte35_get_protocol(const uint8_t *p_scte35)
{
    return p_scte35[3];
}

static inline void scte35_set_protocol(uint8_t *p_scte35, uint8_t i_version)
{
    p_scte35[3] = i_version;
}

static inline bool scte35_is_encrypted(const uint8_t *p_scte35)
{
    return !!(p_scte35[4] & 0x80);
}

static inline void scte35_set_encrypted(uint8_t *p_scte35, bool b_encrypted)
{
    if (b_encrypted)
        p_scte35[4] |= 0x80;
    else
        p_scte35[4] &= ~0x80;
}

static inline uint64_t scte35_get_pts_adjustment(const uint8_t *p_scte35)
{
    return ((uint64_t)(p_scte35[4] & 0x1) << 32) |
           ((uint64_t)p_scte35[5] << 24) | ((uint64_t)p_scte35[6] << 16) |
           ((uint64_t)p_scte35[7] << 8) | (uint64_t)p_scte35[8];
}

static inline void scte35_set_pts_adjustment(uint8_t *p_scte35,
                                             uint64_t i_adjustment)
{
    p_scte35[4] &= ~0x1;
    p_scte35[4] |= (i_adjustment >> 32) & 0x1;
    p_scte35[5] = (i_adjustment >> 24) & 0xff;
    p_scte35[6] = (i_adjustment >> 16) & 0xff;
    p_scte35[7] = (i_adjustment >> 8) & 0xff;
    p_scte35[8] = i_adjustment & 0xff;
}

static inline uint16_t scte35_get_command_length(const uint8_t *p_scte35)
{
    return ((p_scte35[11] & 0xf) << 8) | p_scte35[12];
}

static inline void scte35_set_command_length(uint8_t *p_scte35,
                                             uint16_t i_length)
{
    p_scte35[11] &= ~0xf;
    p_scte35[11] |= (i_length >> 8) & 0xf;
    p_scte35[12] = i_length & 0xff;
}

static inline uint8_t scte35_get_command_type(const uint8_t *p_scte35)
{
    return p_scte35[13];
}

static inline void scte35_set_command_type(uint8_t *p_scte35, uint8_t i_type)
{
    p_scte35[13] = i_type;
}

static inline uint8_t *scte35_get_command(const uint8_t *p_scte35)
{
    return (uint8_t *)p_scte35 + SCTE35_HEADER_SIZE;
}

static inline uint16_t scte35_get_desclength(const uint8_t *p_scte35)
{
    uint16_t i_command_length = scte35_get_command_length(p_scte35);
    if (i_command_length == 0xfff)
        return 0;
    const uint8_t *pi_desclength = scte35_get_command(p_scte35) +
                                   i_command_length;
    return (pi_desclength[0] << 8) | pi_desclength[1];
}

static inline void scte35_set_desclength(uint8_t *p_scte35, uint16_t i_length)
{
    uint8_t *pi_desclength = scte35_get_command(p_scte35) +
                             scte35_get_command_length(p_scte35);
    pi_desclength[0] = (i_length >> 8) & 0xff;
    pi_desclength[1] = i_length & 0xff;
}

static inline uint8_t *scte35_get_descl(const uint8_t *p_scte35)
{
    uint16_t i_command_length = scte35_get_command_length(p_scte35);
    if (i_command_length == 0xfff)
        return NULL;
    return scte35_get_command(p_scte35) + i_command_length +
           SCTE35_HEADER2_SIZE;
}

/*****************************************************************************
 * Splice Information Table - splice_time structure
 *****************************************************************************/
#define SCTE35_SPLICE_TIME_HEADER_SIZE              1
#define SCTE35_SPLICE_TIME_TIME_SIZE                4

static inline void scte35_splice_time_init(uint8_t *p_splice_time)
{
    p_splice_time[0] = 0x7f;
}

static inline bool scte35_splice_time_has_time_specified(const uint8_t *p_splice_time)
{
    return !!(p_splice_time[0] & 0x80);
}

static inline void scte35_splice_time_set_time_specified(uint8_t *p_splice_time,
        bool b_time_specified)
{
    if (b_time_specified)
        p_splice_time[0] |= 0x80;
    else
        p_splice_time[0] &= ~0x80;
}

static inline uint64_t scte35_splice_time_get_pts_time(const uint8_t *p_splice_time)
{
    return ((uint64_t)(p_splice_time[0] & 0x1) << 32) |
           ((uint64_t)p_splice_time[1] << 24) |
           ((uint64_t)p_splice_time[2] << 16) |
           ((uint64_t)p_splice_time[3] << 8) |
           (uint64_t)p_splice_time[4];
}

static inline void scte35_splice_time_set_pts_time(uint8_t *p_splice_time,
                                                   uint64_t i_pts_time)
{
    p_splice_time[0] &= ~0x1;
    p_splice_time[0] |= (i_pts_time >> 32) & 0x1;
    p_splice_time[1] = (i_pts_time >> 24) & 0xff;
    p_splice_time[2] = (i_pts_time >> 16) & 0xff;
    p_splice_time[3] = (i_pts_time >> 8) & 0xff;
    p_splice_time[4] = i_pts_time & 0xff;
}

static inline uint8_t scte35_splice_time_size(const uint8_t *p_splice_time)
{
    return SCTE35_SPLICE_TIME_HEADER_SIZE +
        (scte35_splice_time_has_time_specified(p_splice_time) ?
         SCTE35_SPLICE_TIME_TIME_SIZE : 0);
}

/*****************************************************************************
 * Splice Information Table - break_duration structure
 *****************************************************************************/
#define SCTE35_BREAK_DURATION_HEADER_SIZE              5

static inline void scte35_break_duration_init(uint8_t *p_break_duration)
{
    p_break_duration[0] = 0xff;
}

static inline bool scte35_break_duration_has_auto_return(const uint8_t *p_break_duration)
{
    return !!(p_break_duration[0] & 0x80);
}

static inline void scte35_break_duration_set_auto_return(uint8_t *p_break_duration, bool b_auto_return)
{
    if (b_auto_return)
        p_break_duration[0] |= 0x80;
    else
        p_break_duration[0] &= ~0x80;
}

static inline uint64_t scte35_break_duration_get_duration(const uint8_t *p_break_duration)
{
    return ((uint64_t)(p_break_duration[0] & 0x1) << 32) |
           ((uint64_t)p_break_duration[1] << 24) |
           ((uint64_t)p_break_duration[2] << 16) |
           ((uint64_t)p_break_duration[3] << 8) |
           (uint64_t)p_break_duration[4];
}

static inline void scte35_break_duration_set_duration(uint8_t *p_break_duration,
                                                      uint64_t i_duration)
{
    p_break_duration[0] &= ~0x1;
    p_break_duration[0] |= (i_duration >> 32) & 0x1;
    p_break_duration[1] = (i_duration >> 24) & 0xff;
    p_break_duration[2] = (i_duration >> 16) & 0xff;
    p_break_duration[3] = (i_duration >> 8) & 0xff;
    p_break_duration[4] = i_duration & 0xff;
}

/*****************************************************************************
 * Splice Information Table - null command
 *****************************************************************************/
#define SCTE35_NULL_COMMAND                         0
#define SCTE35_NULL_HEADER_SIZE                     0

static inline void scte35_null_init(uint8_t *p_scte35)
{
    scte35_init(p_scte35);
    scte35_set_command_type(p_scte35, SCTE35_NULL_COMMAND);
    scte35_set_command_length(p_scte35, SCTE35_NULL_HEADER_SIZE);
    scte35_set_desclength(p_scte35, 0);
}

static inline bool scte35_null_validate(const uint8_t *p_scte35)
{
    return true;
}

/*****************************************************************************
 * Splice Information Table - schedule command
 *****************************************************************************/
#define SCTE35_SCHEDULE_COMMAND                     4

/* TODO Not implemented (useless) */

/*****************************************************************************
 * Splice Information Table - insert command
 *****************************************************************************/
#define SCTE35_INSERT_COMMAND                       5
#define SCTE35_INSERT_HEADER_SIZE                   5
#define SCTE35_INSERT_HEADER2_SIZE                  1
#define SCTE35_INSERT_COMPONENT_COUNT_SIZE          1
#define SCTE35_INSERT_COMPONENT_HEADER_SIZE         1
#define SCTE35_INSERT_FOOTER_SIZE                   4

static inline void scte35_insert_init(uint8_t *p_scte35, uint16_t i_length)
{
    scte35_init(p_scte35);
    scte35_set_command_type(p_scte35, SCTE35_INSERT_COMMAND);
    scte35_set_command_length(p_scte35,
                              SCTE35_INSERT_HEADER_SIZE + i_length);
    scte35_set_desclength(p_scte35, 0);

    uint8_t *p_command = scte35_get_command(p_scte35);
    p_command[4] = 0xff;
}

static inline uint32_t scte35_insert_get_event_id(const uint8_t *p_scte35)
{
    const uint8_t *p_command = scte35_get_command(p_scte35);
    return ((uint32_t)p_command[0] << 24) | (p_command[1] << 16) |
           (p_command[2] << 8) | p_command[3];
}

static inline void scte35_insert_set_event_id(uint8_t *p_scte35,
                                              uint32_t i_event_id)
{
    uint8_t *p_command = scte35_get_command(p_scte35);
    p_command[0] = (i_event_id >> 24) & 0xff;
    p_command[1] = (i_event_id >> 16) & 0xff;
    p_command[2] = (i_event_id >> 8) & 0xff;
    p_command[3] = i_event_id & 0xff;
}

static inline bool scte35_insert_has_cancel(const uint8_t *p_scte35)
{
    const uint8_t *p_command = scte35_get_command(p_scte35);
    return !!(p_command[4] & 0x80);
}

static inline void scte35_insert_set_cancel(const uint8_t *p_scte35,
                                            bool b_cancel)
{
    uint8_t *p_command = scte35_get_command(p_scte35);
    if (b_cancel)
        p_command[4] |= 0x80;
    else {
        p_command[4] &= ~0x80;
        p_command[5] = 0xff;
    }
}

static inline bool scte35_insert_has_out_of_network(const uint8_t *p_scte35)
{
    const uint8_t *p_command = scte35_get_command(p_scte35);
    return !!(p_command[5] & 0x80);
}

static inline void scte35_insert_set_out_of_network(const uint8_t *p_scte35,
                                                    bool b_out_of_network)
{
    uint8_t *p_command = scte35_get_command(p_scte35);
    if (b_out_of_network)
        p_command[5] |= 0x80;
    else
        p_command[5] &= ~0x80;
}

static inline bool scte35_insert_has_program_splice(const uint8_t *p_scte35)
{
    const uint8_t *p_command = scte35_get_command(p_scte35);
    return !!(p_command[5] & 0x40);
}

static inline void scte35_insert_set_program_splice(const uint8_t *p_scte35,
                                                    bool b_program_splice)
{
    uint8_t *p_command = scte35_get_command(p_scte35);
    if (b_program_splice)
        p_command[5] |= 0x40;
    else
        p_command[5] &= ~0x40;
}

static inline bool scte35_insert_has_duration(const uint8_t *p_scte35)
{
    const uint8_t *p_command = scte35_get_command(p_scte35);
    return !!(p_command[5] & 0x20);
}

static inline void scte35_insert_set_duration(const uint8_t *p_scte35,
                                              bool b_duration)
{
    uint8_t *p_command = scte35_get_command(p_scte35);
    if (b_duration)
        p_command[5] |= 0x20;
    else
        p_command[5] &= ~0x20;
}

static inline bool scte35_insert_has_splice_immediate(const uint8_t *p_scte35)
{
    const uint8_t *p_command = scte35_get_command(p_scte35);
    return !!(p_command[5] & 0x10);
}

static inline void scte35_insert_set_splice_immediate(const uint8_t *p_scte35,
                                                      bool b_splice_immediate)
{
    uint8_t *p_command = scte35_get_command(p_scte35);
    if (b_splice_immediate)
        p_command[5] |= 0x10;
    else
        p_command[5] &= ~0x10;
}

static inline uint8_t *scte35_insert_get_splice_time(const uint8_t *p_scte35)
{
    uint8_t *p_command = scte35_get_command(p_scte35);
    return p_command + SCTE35_INSERT_HEADER_SIZE + SCTE35_INSERT_HEADER2_SIZE;
}

static inline uint8_t scte35_insert_get_component_count(const uint8_t *p_scte35)
{
    uint8_t *p_command = scte35_get_command(p_scte35);
    return p_command[SCTE35_INSERT_HEADER_SIZE + SCTE35_INSERT_HEADER2_SIZE];
}

static inline void scte35_insert_set_component_count(uint8_t *p_scte35,
                                                     uint8_t i_component_count)
{
    uint8_t *p_command = scte35_get_command(p_scte35);
    p_command[SCTE35_INSERT_HEADER_SIZE + SCTE35_INSERT_HEADER2_SIZE] =
        i_component_count;
}

static inline uint8_t scte35_insert_component_get_component_tag(const uint8_t *p_component)
{
    return p_component[0];
}

static inline void scte35_insert_component_set_component_tag(uint8_t *p_component, uint8_t i_component_tag)
{
    p_component[0] = i_component_tag;
}

static inline uint8_t *scte35_insert_component_get_splice_time(const uint8_t *p_component)
{
    return (uint8_t *)p_component + SCTE35_INSERT_COMPONENT_HEADER_SIZE;
}

static inline uint8_t *scte35_insert_get_component(const uint8_t *p_scte35,
                                                   uint8_t n)
{
    uint16_t i_section_size = psi_get_length(p_scte35) + PSI_HEADER_SIZE
                               - PSI_CRC_SIZE;
    bool b_splice_immediate = scte35_insert_has_splice_immediate(p_scte35);
    uint8_t *p_scte35_n = scte35_get_command(p_scte35) +
        SCTE35_INSERT_HEADER_SIZE + SCTE35_INSERT_HEADER2_SIZE +
        SCTE35_INSERT_COMPONENT_COUNT_SIZE;
    if (p_scte35_n - p_scte35 > i_section_size)
        return NULL;

    while (n) {
        if (p_scte35_n + SCTE35_INSERT_COMPONENT_HEADER_SIZE - p_scte35 >
                i_section_size)
            return NULL;
        p_scte35_n += SCTE35_INSERT_COMPONENT_HEADER_SIZE +
            (b_splice_immediate ?
             scte35_splice_time_size(
                 scte35_insert_component_get_splice_time(p_scte35_n)) :
             0);
        n--;
    }
    if (p_scte35_n - p_scte35 >= i_section_size) return NULL;
    return p_scte35_n;
}

static inline uint8_t *scte35_insert_get_break_duration(const uint8_t *p_scte35)
{
    if (!scte35_insert_has_program_splice(p_scte35))
        return scte35_insert_get_component(p_scte35,
                scte35_insert_get_component_count(p_scte35) + 1);

    if (scte35_insert_has_splice_immediate(p_scte35))
        return scte35_get_command(p_scte35) + SCTE35_INSERT_HEADER_SIZE +
               SCTE35_INSERT_HEADER2_SIZE;

    uint8_t *p_splice_time = scte35_insert_get_splice_time(p_scte35);
    return p_splice_time + scte35_splice_time_size(p_splice_time);
}

static inline uint8_t *scte35_insert_get_footer(const uint8_t *p_scte35)
{
    return scte35_insert_get_break_duration(p_scte35) +
        (scte35_insert_has_duration(p_scte35) ?
         SCTE35_BREAK_DURATION_HEADER_SIZE : 0);
}

static inline uint16_t scte35_insert_get_unique_program_id(const uint8_t *p_scte35)
{
    uint8_t *p_footer = scte35_insert_get_footer(p_scte35);
    return (p_footer[0] << 8) | p_footer[1];
}

static inline void scte35_insert_set_unique_program_id(uint8_t *p_scte35,
        uint16_t i_unique_program_id)
{
    uint8_t *p_footer = scte35_insert_get_footer(p_scte35);
    p_footer[0] = (i_unique_program_id >> 8) & 0xff;
    p_footer[1] = i_unique_program_id & 0xff;
}

static inline uint8_t scte35_insert_get_avail_num(const uint8_t *p_scte35)
{
    uint8_t *p_footer = scte35_insert_get_footer(p_scte35);
    return p_footer[2];
}

static inline void scte35_insert_set_avail_num(uint8_t *p_scte35,
                                               uint8_t i_avail_num)
{
    uint8_t *p_footer = scte35_insert_get_footer(p_scte35);
    p_footer[2] = i_avail_num;
}

static inline uint8_t scte35_insert_get_avails_expected(const uint8_t *p_scte35)
{
    uint8_t *p_footer = scte35_insert_get_footer(p_scte35);
    return p_footer[3];
}

static inline void scte35_insert_set_avails_expected(uint8_t *p_scte35,
                                                     uint8_t i_avails_expected)
{
    uint8_t *p_footer = scte35_insert_get_footer(p_scte35);
    p_footer[3] = i_avails_expected;
}

static inline bool scte35_insert_validate(const uint8_t *p_scte35)
{
    size_t i_length = scte35_get_command_length(p_scte35);
    if (i_length < SCTE35_INSERT_HEADER_SIZE)
        return false;

    if (scte35_insert_has_cancel(p_scte35))
        return i_length <= SCTE35_INSERT_HEADER_SIZE;
    if (i_length < SCTE35_INSERT_HEADER_SIZE + SCTE35_INSERT_HEADER2_SIZE)
        return false;

    if (scte35_insert_has_program_splice(p_scte35)) {
        if (scte35_insert_has_splice_immediate(p_scte35)) {
            if (scte35_insert_has_duration(p_scte35))
                return i_length >= SCTE35_INSERT_HEADER_SIZE +
                                   SCTE35_INSERT_HEADER2_SIZE +
                                   SCTE35_BREAK_DURATION_HEADER_SIZE +
                                   SCTE35_INSERT_FOOTER_SIZE;
            return i_length >= SCTE35_INSERT_HEADER_SIZE +
                               SCTE35_INSERT_HEADER2_SIZE +
                               SCTE35_INSERT_FOOTER_SIZE;
        }

        if (i_length < SCTE35_INSERT_HEADER_SIZE + SCTE35_INSERT_HEADER2_SIZE +
                       SCTE35_SPLICE_TIME_HEADER_SIZE ||
            i_length < SCTE35_INSERT_HEADER_SIZE + SCTE35_INSERT_HEADER2_SIZE +
                       scte35_splice_time_size(
                           scte35_insert_get_splice_time(p_scte35)))
            return false;

        if (scte35_insert_has_duration(p_scte35))
            return i_length >= SCTE35_INSERT_HEADER_SIZE +
                SCTE35_INSERT_HEADER2_SIZE +
                scte35_splice_time_size(scte35_insert_get_splice_time(p_scte35))
                + SCTE35_BREAK_DURATION_HEADER_SIZE +
                SCTE35_INSERT_FOOTER_SIZE;
        return i_length >= SCTE35_INSERT_HEADER_SIZE +
            SCTE35_INSERT_HEADER2_SIZE +
            scte35_splice_time_size(scte35_insert_get_splice_time(p_scte35))
            + SCTE35_INSERT_FOOTER_SIZE;
    }

    if (i_length < SCTE35_INSERT_HEADER_SIZE + SCTE35_INSERT_HEADER2_SIZE +
                   SCTE35_INSERT_COMPONENT_COUNT_SIZE)
        return false;

    const uint8_t *p_command = scte35_get_command(p_scte35);
    const uint8_t *p_end = scte35_insert_get_component(p_scte35,
            scte35_insert_get_component_count(p_scte35) + 1);
    if (p_end == NULL)
        return false;

    if (scte35_insert_has_duration(p_scte35))
        return i_length >= p_end + SCTE35_BREAK_DURATION_HEADER_SIZE +
                                   SCTE35_INSERT_FOOTER_SIZE - p_command;
    return i_length >= p_end + SCTE35_INSERT_FOOTER_SIZE - p_command;
}

/*****************************************************************************
 * Splice Information Table - time_signal command
 *****************************************************************************/
#define SCTE35_TIME_SIGNAL_COMMAND              6
#define SCTE35_TIME_SIGNAL_HEADER_SIZE          SCTE35_SPLICE_TIME_HEADER_SIZE
#define SCTE35_TIME_SIGNAL_TIME_SIZE            SCTE35_SPLICE_TIME_TIME_SIZE

static inline void scte35_time_signal_init(uint8_t *p_scte35, uint16_t i_length)
{
    scte35_init(p_scte35);
    scte35_set_command_type(p_scte35, SCTE35_TIME_SIGNAL_COMMAND);
    scte35_set_command_length(p_scte35,
                              SCTE35_TIME_SIGNAL_HEADER_SIZE + i_length);
    scte35_set_desclength(p_scte35, 0);
}

static inline uint8_t *scte35_time_signal_get_splice_time(const uint8_t *p_scte35)
{
    return scte35_get_command(p_scte35);
}

static inline bool scte35_time_signal_validate(const uint8_t *p_scte35)
{
    return scte35_get_command_length(p_scte35) >=
        scte35_splice_time_size(scte35_time_signal_get_splice_time(p_scte35));
}

/*****************************************************************************
 * Splice Information Table - bandwidth_reservation command
 *****************************************************************************/
#define SCTE35_BANDWIDTH_RESERVATION_COMMAND        7
#define SCTE35_BANDWIDTH_RESERVATION_HEADER_SIZE    0

static inline void scte35_bandwidth_reservation_init(uint8_t *p_scte35)
{
    scte35_init(p_scte35);
    scte35_set_command_type(p_scte35, SCTE35_BANDWIDTH_RESERVATION_COMMAND);
    scte35_set_command_length(p_scte35,
                              SCTE35_BANDWIDTH_RESERVATION_HEADER_SIZE);
    scte35_set_desclength(p_scte35, 0);
}

static inline bool scte35_bandwidth_reservation_validate(const uint8_t *p_scte35)
{
    return true;
}

/*****************************************************************************
 * Splice Information Table - private command
 *****************************************************************************/
#define SCTE35_PRIVATE_COMMAND                      0xff
#define SCTE35_PRIVATE_HEADER_SIZE                  4

static inline void scte35_private_init(uint8_t *p_scte35, uint16_t i_length)
{
    scte35_init(p_scte35);
    scte35_set_command_type(p_scte35, SCTE35_PRIVATE_COMMAND);
    scte35_set_command_length(p_scte35,
                              SCTE35_PRIVATE_HEADER_SIZE + i_length);
    scte35_set_desclength(p_scte35, 0);
}

static inline uint32_t scte35_private_get_identifier(const uint8_t *p_scte35)
{
    const uint8_t *p_command = scte35_get_command(p_scte35);
    return ((uint32_t)p_command[0] << 24) | (p_command[1] << 16) |
           (p_command[2] << 8) | p_command[3];
}

static inline void scte35_private_set_identifier(uint8_t *p_scte35,
                                                 uint32_t i_identifier)
{
    uint8_t *p_command = scte35_get_command(p_scte35);
    p_command[0] = (i_identifier >> 24) & 0xff;
    p_command[1] = (i_identifier >> 16) & 0xff;
    p_command[2] = (i_identifier >> 8) & 0xff;
    p_command[3] = i_identifier & 0xff;
}

static inline bool scte35_private_validate(const uint8_t *p_scte35)
{
    return scte35_get_command_length(p_scte35) >= SCTE35_PRIVATE_HEADER_SIZE;
}

/*****************************************************************************
 * Splice Information Table validation
 *****************************************************************************/
static inline bool scte35_validate(const uint8_t *p_scte35)
{
    if (psi_get_syntax(p_scte35) ||
        psi_get_tableid(p_scte35) != SCTE35_TABLE_ID ||
        psi_get_length(p_scte35) <  SCTE35_HEADER2_SIZE + PSI_CRC_SIZE)
        return false;

    if (!psi_check_crc(p_scte35))
        return false;
    if (scte35_get_protocol(p_scte35))
        return false;

    uint16_t i_section_size = psi_get_length(p_scte35) + PSI_HEADER_SIZE
                               - PSI_CRC_SIZE;
    if (i_section_size < SCTE35_HEADER_SIZE)
        return false;

    uint16_t i_command_length = scte35_get_command_length(p_scte35);
    if (i_command_length != 0xfff &&
        (i_section_size < SCTE35_HEADER_SIZE + i_command_length +
                          SCTE35_HEADER2_SIZE ||
         i_section_size < SCTE35_HEADER_SIZE + i_command_length +
                          SCTE35_HEADER2_SIZE +
                          scte35_get_desclength(p_scte35)))
        return false;

    switch (scte35_get_command_type(p_scte35)) {
        case SCTE35_NULL_COMMAND:
            if (!scte35_null_validate(p_scte35))
                return false;
            break;
        case SCTE35_INSERT_COMMAND:
            if (!scte35_insert_validate(p_scte35))
                return false;
            break;
        case SCTE35_TIME_SIGNAL_COMMAND:
            if (!scte35_time_signal_validate(p_scte35))
                return false;
            break;
        case SCTE35_BANDWIDTH_RESERVATION_COMMAND:
            if (!scte35_bandwidth_reservation_validate(p_scte35))
                return false;
            break;
        case SCTE35_PRIVATE_COMMAND:
            if (!scte35_private_validate(p_scte35))
                return false;
            break;
        default:
            break;
    }

    if (i_command_length != 0xfff &&
        !descl_validate(scte35_get_descl(p_scte35),
                        scte35_get_desclength(p_scte35)))
        return false;

    return true;
}

/*****************************************************************************
 * 10.3.3. segmentation_descriptor()
 *****************************************************************************/

#define SCTE35_SD_HEADER_SIZE 11
#define SCTE35_SD_TAG         2
#define SCTE35_SD_IDENTIFIER  0x43554549

#define SCTE35_SD_SEGMENTATION_TYPE_PROV_START 0x34
#define SCTE35_SD_SEGMENTATION_TYPE_DIST_START 0x36

static inline void scte35sd_init(uint8_t *p)
{
    p[10] = 0x7f;
}

static inline uint8_t scte35sd_get_splice_descriptor_tag(const uint8_t *p)
{
    return p[0];
}

static inline void scte35sd_set_splice_descriptor_tag(uint8_t *p, uint8_t splice_descriptor_tag)
{
    p[0] = splice_descriptor_tag;
}

static inline uint8_t scte35sd_get_descriptor_length(const uint8_t *p)
{
    return p[1];
}

static inline void scte35sd_set_descriptor_length(uint8_t *p, uint8_t descriptor_length)
{
    p[1] = descriptor_length;
}

static inline uint32_t scte35sd_get_identifier(const uint8_t *p)
{
    return ((uint32_t)p[2] << 24) |
           ((uint32_t)p[3] << 16) |
           ((uint32_t)p[4] << 8) |
           (uint32_t)p[5];
}

static inline void scte35sd_set_identifier(uint8_t *p, uint32_t identifier)
{
    p[2] = (identifier >> 24) & 0xff;
    p[3] = (identifier >> 16) & 0xff;
    p[4] = (identifier >> 8) & 0xff;
    p[5] = identifier & 0xff;
}

static inline uint32_t scte35sd_get_segmentation_event_id(const uint8_t *p)
{
    return ((uint32_t)p[6] << 24) |
           ((uint32_t)p[7] << 16) |
           ((uint32_t)p[8] << 8) |
           (uint32_t)p[9];
}

static inline void scte35sd_set_segmentation_event_id(uint8_t *p, uint32_t segmentation_event_id)
{
    p[6] = (segmentation_event_id >> 24) & 0xff;
    p[7] = (segmentation_event_id >> 16) & 0xff;
    p[8] = (segmentation_event_id >> 8) & 0xff;
    p[9] = segmentation_event_id & 0xff;
}

static inline bool scte35sd_get_segmentation_event_cancel_indicator(const uint8_t *p)
{
    return !!(p[10] & 0x80);
}

static inline void scte35sd_set_segmentation_event_cancel_indicator(uint8_t *p, bool segmentation_event_cancel_indicator)
{
    if (segmentation_event_cancel_indicator)
        p[10] |= 0x80;
    else
        p[10] &= ~0x80;
}

static inline bool scte35sd_get_program_segmentation_flag(const uint8_t *p)
{
    return !!(p[11] & 0x80);
}

static inline void scte35sd_set_program_segmentation_flag(uint8_t *p, bool program_segmentation_flag)
{
    if (program_segmentation_flag)
        p[11] |= 0x80;
    else
        p[11] &= ~0x80;
}

static inline bool scte35sd_get_segmentation_duration_flag(const uint8_t *p)
{
    return !!(p[11] & 0x40);
}

static inline void scte35sd_set_segmentation_duration_flag(uint8_t *p, bool segmentation_duration_flag)
{
    if (segmentation_duration_flag)
        p[11] |= 0x40;
    else
        p[11] &= ~0x40;
}

static inline bool scte35sd_get_delivery_not_restricted_flag(const uint8_t *p)
{
    return !!(p[11] & 0x20);
}

static inline void scte35sd_set_delivery_not_restricted_flag(uint8_t *p, bool delivery_not_restricted_flag)
{
    if (delivery_not_restricted_flag)
        p[11] |= 0x20;
    else
        p[11] &= ~0x20;
}

static inline void scte35sd_init_delivery_not_restricted(uint8_t *p)
{
    p[11] |= 0x1f;
}

static inline bool scte35sd_get_web_delivery_allowed_flag(const uint8_t *p)
{
    return !!(p[11] & 0x10);
}

static inline void scte35sd_set_web_delivery_allowed_flag(uint8_t *p, bool web_delivery_allowed_flag)
{
    if (web_delivery_allowed_flag)
        p[11] |= 0x10;
    else
        p[11] &= ~0x10;
}

static inline bool scte35sd_get_no_regional_blackout_flag(const uint8_t *p)
{
    return !!(p[11] & 0x08);
}

static inline void scte35sd_set_no_regional_blackout_flag(uint8_t *p, bool no_regional_blackout_flag)
{
    if (no_regional_blackout_flag)
        p[11] |= 0x08;
    else
        p[11] &= ~0x08;
}

static inline bool scte35sd_get_archive_allowed_flag(const uint8_t *p)
{
    return !!(p[11] & 0x04);
}

static inline void scte35sd_set_archive_allowed_flag(uint8_t *p, bool archive_allowed_flag)
{
    if (archive_allowed_flag)
        p[11] |= 0x04;
    else
        p[11] &= ~0x04;
}

static inline uint8_t scte35sd_get_device_restrictions(const uint8_t *p)
{
    return p[11] & 0x03;
}

static inline void scte35sd_set_device_restrictions(uint8_t *p, uint8_t device_restrictions)
{
    p[11] = (p[11] & ~0x03) | device_restrictions;
}

static inline uint8_t scte35sd_get_component_count(const uint8_t *p)
{
    return p[12];
}

static inline void scte35sd_set_component_count(uint8_t *p, uint8_t component_count)
{
    p[12] = component_count;
}

/* pointer address change */

static inline uint8_t scte35sd_get_component_tag(const uint8_t *p)
{
    return p[0];
}

static inline void scte35sd_set_component_tag(uint8_t *p, uint8_t component_tag)
{
    p[0] = component_tag;
}

static inline uint64_t scte35sd_get_pts_offset(const uint8_t *p)
{
    return (((uint64_t)p[1] & 0x01) << 32) |
           ((uint64_t)p[2] << 24) |
           ((uint64_t)p[3] << 16) |
           ((uint64_t)p[4] << 8) |
           (uint64_t)p[5];
}

static inline void scte35sd_set_pts_offset(uint8_t *p, uint64_t pts_offset)
{
    p[1] |= (pts_offset >> 32) & 0x01;
    p[2] = (pts_offset >> 24) & 0xff;
    p[3] = (pts_offset >> 16) & 0xff;
    p[4] = (pts_offset >> 8) & 0xff;
    p[5] = pts_offset & 0xff;
}

/* pointer address change */

static inline uint64_t scte35sd_get_segmentation_duration(const uint8_t *p)
{
    return ((uint64_t)p[0] << 32) |
           ((uint64_t)p[1] << 24) |
           ((uint64_t)p[2] << 16) |
           ((uint64_t)p[3] << 8) |
           (uint64_t)p[4];
}

static inline void scte35sd_set_segmentation_duration(uint8_t *p, uint64_t segmentation_duration)
{
    p[0] = (segmentation_duration >> 32) & 0xff;
    p[1] = (segmentation_duration >> 24) & 0xff;
    p[2] = (segmentation_duration >> 16) & 0xff;
    p[3] = (segmentation_duration >> 8) & 0xff;
    p[4] = segmentation_duration & 0xff;
}

static inline uint8_t scte35sd_get_segmentation_upid_type(const uint8_t *p)
{
    return p[0];
}

static inline void scte35sd_set_segmentation_upid_type(uint8_t *p, uint8_t segmentation_upid_type)
{
    p[0] = segmentation_upid_type;
}

static inline uint8_t scte35sd_get_segmentation_upid_length(const uint8_t *p)
{
    return p[1];
}

static inline void scte35sd_set_segmentation_upid_length(uint8_t *p, uint8_t segmentation_upid_length)
{
    p[1] = segmentation_upid_length;
}

/* segmentation_upid() */

static inline uint8_t scte35sd_get_segmentation_type_id(const uint8_t *p)
{
    return p[0];
}

static inline void scte35sd_set_segmentation_type_id(uint8_t *p, uint8_t segmentation_type_id)
{
    p[0] = segmentation_type_id;
}

static inline uint8_t scte35sd_get_segment_num(const uint8_t *p)
{
    return p[1];
}

static inline void scte35sd_set_segment_num(uint8_t *p, uint8_t segment_num)
{
    p[1] = segment_num;
}

static inline uint8_t scte35sd_get_segments_expected(const uint8_t *p)
{
    return p[2];
}

static inline void scte35sd_set_segments_expected(uint8_t *p, uint8_t segments_expected)
{
    p[2] = segments_expected;
}

static inline uint8_t scte35sd_get_sub_segment_num(const uint8_t *p)
{
    return p[3];
}

static inline void scte35sd_set_sub_segment_num(uint8_t *p, uint8_t sub_segment_num)
{
    p[3] = sub_segment_num;
}

static inline uint8_t scte35sd_get_sub_segments_expected(const uint8_t *p)
{
    return p[4];
}

static inline void scte35sd_set_sub_segments_expected(uint8_t *p, uint8_t sub_segments_expected)
{
    p[4] = sub_segments_expected;
}


#ifdef __cplusplus
}
#endif

#endif
