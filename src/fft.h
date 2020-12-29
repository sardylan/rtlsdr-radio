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

#include "buildflags.h"
#include "utils.h"

#define FFT_FLOAT FP_FLOAT

#ifdef RTLSDR_RADIO_FP_FLOAT
#define FFT_prefix fftwf
#else
#define FFT_prefix fftw
#endif

#define FFT_plan CONCAT(FFT_prefix, _plan)
#define FFT_complex CONCAT(FFT_prefix, _complex)
#define FFT_alloc_complex CONCAT(FFT_prefix, _alloc_complex)
#define FFT_alloc_real CONCAT(FFT_prefix, _alloc_real)
#define FFT_plan_dft_1d CONCAT(FFT_prefix, _plan_dft_1d)
#define FFT_plan_r2r_1d CONCAT(FFT_prefix, _plan_r2r_1d)
#define FFT_destroy_plan CONCAT(FFT_prefix, _destroy_plan)
#define FFT_free CONCAT(FFT_prefix, _free)
#define FFT_execute CONCAT(FFT_prefix, _execute)

enum fft_data_type_t {
    FFT_DATA_TYPE_COMPLEX = 'C',
    FFT_DATA_TYPE_REAL = 'R'
};

typedef enum fft_data_type_t fft_data_type;

struct fft_ctx_t {
    size_t size;
    fft_data_type data_type;

    FFT_FLOAT *real_input;
    FFT_FLOAT *real_output;

    FFT_complex *complex_input;
    FFT_complex *complex_output;

    FFT_plan plan;
};

typedef struct fft_ctx_t fft_ctx;

fft_ctx *fft_init(size_t, int, fft_data_type);

void fft_free(fft_ctx *);

void fft_compute(fft_ctx *ctx);

int fft_complex_compute(fft_ctx *, FFT_FLOAT complex *, FFT_FLOAT complex *);

int fft_real_compute(fft_ctx *, FFT_FLOAT *, FFT_FLOAT *);

int fft_real_compute_int8(fft_ctx *, const int8_t *, int8_t *);

#endif
