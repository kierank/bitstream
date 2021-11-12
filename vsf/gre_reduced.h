/*****************************************************************************
 * gre_reduced.h: GRE Reduced Mode
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
 *  - Video Services Forum TR-06-2 2021-04-26
 */

#ifndef __BITSTREAM_VSF_GRE_REDUCED_H__
#define __BITSTREAM_VSF_GRE_REDUCED_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Reminder: GRE Reduced header
 0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|          gre_reduced Source Port     |       gre_reduced Destination Port      |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

#define GRE_REDUCED_HEADER_SIZE 4

static inline void gre_reduced_set_srcport(uint8_t *p_gre_reduced, uint16_t port)
{
    p_gre_reduced[0] = (port & 0xff00) >> 8;
    p_gre_reduced[1] = (port & 0xff);
}

static inline uint16_t gre_reduced_get_srcport(const uint8_t *p_gre_reduced)
{
    return (p_gre_reduced[0] << 8) | p_gre_reduced[1];
}

static inline void gre_reduced_set_dstport(uint8_t *p_gre_reduced, uint16_t port)
{
    p_gre_reduced[2] = (port & 0xff00) >> 8;
    p_gre_reduced[3] = (port & 0xff);
}

static inline uint16_t gre_reduced_get_dstport(const uint8_t *p_gre_reduced)
{
    return (p_gre_reduced[2] << 8) | p_gre_reduced[3];
}

static inline uint8_t *gre_reduced_payload(uint8_t *p_gre_reduced)
{
    return p_gre_reduced + GRE_REDUCED_HEADER_SIZE;
}

#ifdef __cplusplus
}
#endif

#endif
