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

enum fft_data_type_t {
    FFT_DATA_TYPE_COMPLEX = 'C',
    FFT_DATA_TYPE_REAL = 'R'
};

typedef enum fft_data_type_t fft_data_type;

struct fft_ctx_t {
    size_t size;
    fft_data_type data_type;

    double *real_input;
    double *real_output;

    fftw_complex *complex_input;
    fftw_complex *complex_output;

    fftw_plan plan;
};

typedef struct fft_ctx_t fft_ctx;

fft_ctx *fft_init(size_t, int, fft_data_type);

void fft_free(fft_ctx *);

int fft_complex_compute(fft_ctx *, double complex *, double complex *);

void fft_real_manual_compute(fft_ctx *);

int fft_real_compute(fft_ctx *, double *, double *);

int fft_real_compute_int8(fft_ctx *, const int8_t *, int8_t *);

#endif
