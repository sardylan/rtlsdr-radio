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


#ifndef __RTLSDR_RADIO__WAV__H
#define __RTLSDR_RADIO__WAV__H

#define WAV_HEADER_LENGTH 44

#include <stdint.h>

struct wav_ctx_t {
    char *file_path;

    uint16_t channels;
    uint32_t sample_rate;
    uint16_t bit_per_sample;

    uint32_t data_size;

    FILE *fp;
};

typedef struct wav_ctx_t wav_ctx;

wav_ctx *wav_init(const char *, uint16_t, uint32_t, uint16_t);

void wav_free(wav_ctx *);

int wav_write_begin(wav_ctx *);

int wav_write_end(wav_ctx *);

int wav_write_data_uint8(wav_ctx *, uint8_t *, size_t);

int wav_write_data_int16(wav_ctx *, int16_t *, size_t);

#endif
