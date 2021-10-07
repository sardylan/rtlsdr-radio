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


#ifndef __RTLSDR_RADIO__AGC__H
#define __RTLSDR_RADIO__AGC__H

#include <stddef.h>
#include <stdint.h>

#include "buildflags.h"

struct agc_ctx_t {
    FP_FLOAT gain;
    FP_FLOAT dc_offset;

    FP_FLOAT gain_factor;
    FP_FLOAT dc_offset_factor;
};

typedef struct agc_ctx_t agc_ctx;

agc_ctx *agc_init();

void agc_free();

int agc_perform_gain(agc_ctx *ctx, FP_FLOAT *data, size_t len);

//int agc_perform_gain(int8_t *, size_t);

#endif
