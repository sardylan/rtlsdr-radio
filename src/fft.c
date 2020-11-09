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
#include <string.h>

#include "fft.h"
#include "log.h"

fft_ctx *fft_init(size_t size) {
    fft_ctx *ctx;

    log_init("Initializing");

    log_debug("Allocating context");
    ctx = (fft_ctx *) malloc(sizeof(fft_ctx));
    if (ctx == NULL) {
        log_error("Unable to allocate contest");
        return NULL;
    }

    log_debug("Setting size");
    ctx->size = size;

    log_debug("Allocating input and output buffers");
    ctx->input = fftw_alloc_complex(ctx->size);
    ctx->output = fftw_alloc_complex(ctx->size);

    log_debug("Computing FFT plan");
    ctx->plan = fftw_plan_dft_1d((int) ctx->size, ctx->input, ctx->output, FFTW_FORWARD, FFTW_EXHAUSTIVE);

    return ctx;
}

void fft_free(fft_ctx *ctx) {
    log_init("Freeing");

    log_debug("Destroing plan");
    fftw_destroy_plan(ctx->plan);

    log_debug("Deallocationg buffers");
    fftw_free(ctx->input);
    fftw_free(ctx->output);

    log_debug("Deallocationg context");
    free(ctx);
}

void fft_compute(fft_ctx *ctx, double complex *input, double complex *output) {
    log_info("Computing FFT");

    log_debug("Copying input values");
    memcpy(input, ctx->input, ctx->size);

    log_debug("Computing FFT");
    fftw_execute(ctx->plan);

    log_debug("Copying output values");
    memcpy(output, ctx->output, ctx->size);
}
