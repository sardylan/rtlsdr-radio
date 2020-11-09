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


#include <malloc.h>

#include "fft.h"
#include "log.h"

fft_ctx *fft_init(size_t size) {
    fft_ctx *ctx;

    ctx = (fft_ctx *) malloc(sizeof(fft_ctx));
    if (ctx == NULL) {
        log_error("Unable to allocate FFT contest");
        return NULL;
    }

    ctx->size = size;

    ctx->input = (fftw_complex *) fftw_malloc(sizeof(double) * ctx->size);
    ctx->output = (fftw_complex *) fftw_malloc(sizeof(double) * ctx->size);

    ctx->plan = fftw_plan_dft_c2r((int) ctx->size, FFT::input, FFT::output, FFTW_R2HC, FFTW_ESTIMATE);

    return ctx;
}

void fft_free(fft_ctx *ctx) {
    free(ctx);
}
