/*
 * rtlsdr-radio
 * Copyright (C) 2020 - 2021  Luca Cireddu (sardylan@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */


#ifndef __RTLSDR_RADIO__CODEC__H
#define __RTLSDR_RADIO__CODEC__H

#include <stdint.h>

#include <codec2/codec2.h>

struct codec_ctx_t {
    int mode;

    struct CODEC2 *codec2;

    size_t pcm_size;
    size_t data_size;
};

typedef struct codec_ctx_t codec_ctx;

codec_ctx *codec_init(int);

void codec_free(codec_ctx *);

size_t codec_get_pcm_size(codec_ctx *);

size_t codec_get_data_size(codec_ctx *);

void codec_encode(codec_ctx *, int16_t *, uint8_t *);

void codec_decode(codec_ctx *, uint8_t *, int16_t *);

#endif
