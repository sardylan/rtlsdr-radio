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


#ifndef __RTLSDR_RADIO__RESAMPLE__H
#define __RTLSDR_RADIO__RESAMPLE__H

#include <stdint.h>
#include <stddef.h>

#include "fir.h"

struct resample_ctx_t {
    uint32_t src_rate;
    uint32_t dst_rate;

    uint32_t ratio;
};

typedef struct resample_ctx_t resample_ctx;

resample_ctx *resample_init(uint32_t, uint32_t);

void resample_free(resample_ctx *);

size_t resample_compute_output_size(resample_ctx *, size_t);

int resample_do(resample_ctx *ctx, const int8_t *, size_t, int8_t *, size_t);

int resample_float_to_int16(resample_ctx *ctx, const FP_FLOAT *, size_t, int16_t *, size_t);

#endif
