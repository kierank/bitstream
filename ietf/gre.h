/*****************************************************************************
 * gre.h: Generic Routing Encapsulation (GRE)
 *****************************************************************************
 * Copyright (C) 2021 VideoLAN
 *
 * Authors: Kieran Kunhya <kierank@obe.tv>
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
 *  - IETF RFC 2784 Generic Routing Encapsulation (GRE)
 *  - IETF RFC 2890 Key and Sequence Number Extensions to GRE
 */

#ifndef __BITSTREAM_IETF_GRE_H__
#define __BITSTREAM_IETF_GRE_H__

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Reminder: GRE header
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |C| |K|S| Reserved0       | Ver |         Protocol Type         |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |      Checksum (optional)      |       Reserved1 (Optional)    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                         Key (optional)                        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                 Sequence Number (Optional)                    |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

#define GRE_HEADER_MINSIZE   4
#define GRE_CHECKSUM_LEN     2
#define GRE_RESERVED1_LEN    2
#define GRE_KEY_LEN          4
#define GRE_SEQNUM_LEN       4

static inline void gre_header_init(uint8_t *p_gre)
{
    memset(p_gre, 0, GRE_HEADER_MINSIZE);
}

static inline void gre_set_checksum_flag(uint8_t *p_gre)
{
    p_gre[0] |= 0x80;
}

static inline bool gre_check_checksum_flag(uint8_t *p_gre)
{
    return !!(p_gre[0] & 0x80);
}

static inline void gre_set_key_flag(uint8_t *p_gre)
{
    p_gre[0] |= 0x20;
}

static inline bool gre_check_key_flag(uint8_t *p_gre)
{
    return !!(p_gre[0] & 0x20);
}

static inline bool gre_check_seqnum_flag(uint8_t *p_gre)
{
    return !!(p_gre[0] & 0x10);
}

static inline void gre_set_seqnum_flag(uint8_t *p_gre)
{
    p_gre[0] |= 0x10;
}

static inline void gre_set_protocol_type(uint8_t *p_gre, uint16_t i_type)
{
    p_gre[2] = (i_type >> 8) & 0xff;
    p_gre[3] = (i_type >> 0) & 0xff;
}

static inline uint16_t gre_get_protocol_type(const uint8_t *p_gre)
{
    return (p_gre[2] << 8) | p_gre[3];
}

static inline void gre_set_checksum(uint8_t *p_gre, uint16_t i_checksum)
{
    p_gre[4] = (i_checksum >> 8) & 0xff;
    p_gre[5] = (i_checksum >> 0) & 0xff;
}

static inline uint16_t gre_get_checksum(const uint8_t *p_gre)
{
    return (p_gre[4] << 8) | p_gre[5];
}

static inline void gre_set_key(uint8_t *p_gre, uint32_t i_key)
{
    if (gre_check_checksum_flag(p_gre))
        p_gre += GRE_CHECKSUM_LEN + GRE_RESERVED1_LEN;

    p_gre[4] = (i_key >> 8) & 0xff;
    p_gre[5] = (i_key >> 0) & 0xff;
}

static inline uint16_t gre_get_key(uint8_t *p_gre)
{
    if (gre_check_checksum_flag(p_gre))
        p_gre += GRE_CHECKSUM_LEN + GRE_RESERVED1_LEN;

    return (p_gre[4] << 8) | p_gre[5];
}

static inline void gre_set_seqnum(uint8_t *p_gre, uint32_t i_seqnum)
{
    if (gre_check_checksum_flag(p_gre))
        p_gre += GRE_CHECKSUM_LEN + GRE_RESERVED1_LEN;

    if (gre_check_key_flag(p_gre))
        p_gre += GRE_KEY_LEN;

    p_gre[4] = (i_seqnum >> 8) & 0xff;
    p_gre[5] = (i_seqnum >> 0) & 0xff;
}

static inline uint16_t gre_get_seqnum(uint8_t *p_gre)
{
    if (gre_check_checksum_flag(p_gre))
        p_gre += GRE_CHECKSUM_LEN + GRE_RESERVED1_LEN;

    if (gre_check_key_flag(p_gre))
        p_gre += GRE_KEY_LEN;

    return (p_gre[4] << 8) | p_gre[5];
}

static inline uint8_t *gre_payload(uint8_t *p_gre)
{
    p_gre += GRE_HEADER_MINSIZE;

    if (gre_check_checksum_flag(p_gre))
        p_gre += GRE_CHECKSUM_LEN + GRE_RESERVED1_LEN;

    if (gre_check_key_flag(p_gre))
        p_gre += GRE_KEY_LEN;

    if (gre_check_seqnum_flag(p_gre))
        p_gre += GRE_SEQNUM_LEN;

    return p_gre;
}

#ifdef __cplusplus
}
#endif

#endif
