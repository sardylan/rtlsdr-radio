/*
 * rtlsdr-radio
 * Copyright (C) 2020  Luca Cireddu (sardylan@gmail.com)
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


#ifndef __RTLSDR_RADIO__FFT__H
#define __RTLSDR_RADIO__FFT__H

#include <stddef.h>
#include <complex.h>
#include <fftw3.h>

struct fft_ctx_t {
    size_t size;

    fftw_complex *input;
    fftw_complex *output;

    fftw_plan plan;
};

typedef struct fft_ctx_t fft_ctx;

fft_ctx *fft_init(size_t);

void fft_free(fft_ctx *);

#endif
