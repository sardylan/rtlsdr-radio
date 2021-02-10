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

#include <opus.h>

// Frame duration 20ms, at 8000Hz we have 160 samples
#define CODEC_FRAME_SIZE 160

#define CODEC_FRAME_BUFFER 8192

struct codec_ctx_t {
    opus_int32 sample_rate;
    opus_int32 bitrate;

    OpusEncoder *encoder;

    opus_int16 *input;
};

typedef struct codec_ctx_t codec_ctx;

codec_ctx *codec_init(int32_t, int32_t);

void codec_free(codec_ctx *);

int codec_encode(codec_ctx *, const int8_t *, uint8_t *, size_t, size_t *);

#endif
