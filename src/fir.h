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


#ifndef __RTLSDR_RADIO__FIR__H
#define __RTLSDR_RADIO__FIR__H

#include <stdint.h>

struct fir_ctx_t {
    size_t kernel_size;
    FP_FLOAT *kernel;
    int8_t *prev;
};

typedef struct fir_ctx_t fir_ctx;

fir_ctx *fir_init(const FP_FLOAT *, size_t);

void fir_free(fir_ctx *);

int fir_convolve(fir_ctx *ctx, int8_t *output, const int8_t *input, size_t size);

fir_ctx *fir_init_lpf(int);

#endif
