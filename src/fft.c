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
#include <stdlib.h>
#include <math.h>

#include "fft.h"
#include "log.h"

fft_ctx *fft_init(size_t size, int sign, fft_data_type data_type) {
    fft_ctx *ctx;

    log_info("Initializing");

    log_debug("Allocating context");
    ctx = (fft_ctx *) malloc(sizeof(fft_ctx));
    if (ctx == NULL) {
        log_error("Unable to allocate context");
        return NULL;
    }

    log_debug("Setting size");
    ctx->size = size;

    log_debug("Setting data type");
    ctx->data_type = data_type;

    switch (ctx->data_type) {

        case FFT_DATA_TYPE_COMPLEX:
            log_debug("Allocating complex buffers");
            ctx->complex_input = fftw_alloc_complex(ctx->size);
            ctx->complex_output = fftw_alloc_complex(ctx->size);

            log_debug("Computing complex FFT plan");
            ctx->plan = fftw_plan_dft_1d((int) ctx->size, ctx->complex_input, ctx->complex_output, sign,
                                         FFTW_EXHAUSTIVE);
            break;

        case FFT_DATA_TYPE_REAL:
            log_debug("Allocating real buffers");
            ctx->real_input = fftw_alloc_real(ctx->size);
            ctx->real_output = fftw_alloc_real(ctx->size);

            log_debug("Computing complex FFT plan");
            ctx->plan = fftw_plan_r2r_1d((int) ctx->size, ctx->real_input, ctx->real_output, sign, FFTW_EXHAUSTIVE);
            break;

        default:
            log_error("Not implemented");
    }

    return ctx;
}

void fft_free(fft_ctx *ctx) {
    log_info("Freeing");

    log_debug("Destroing plan");
    fftw_destroy_plan(ctx->plan);

    log_debug("Deallocating buffers");
    switch (ctx->data_type) {

        case FFT_DATA_TYPE_COMPLEX:
            fftw_free(ctx->complex_input);
            fftw_free(ctx->complex_output);
            break;

        case FFT_DATA_TYPE_REAL:
            fftw_free(ctx->real_input);
            fftw_free(ctx->real_output);
            break;

        default:
            log_error("Not implemented");
    }

    log_debug("Freeing context");
    free(ctx);
}

int fft_complex_compute(fft_ctx *ctx, double complex *input, double complex *output) {
    log_info("Computing complex FFT");

    if (ctx->data_type != FFT_DATA_TYPE_COMPLEX) {
        log_error("Invalid data type");
        return EXIT_FAILURE;
    }

    log_debug("Copying input values");
    memcpy(ctx->complex_input, input, ctx->size * sizeof(double complex));

    log_debug("Computing FFT");
    fftw_execute(ctx->plan);

    log_debug("Copying output values");
    memcpy(output, ctx->complex_output, ctx->size * sizeof(double complex));

    return EXIT_SUCCESS;
}

void fft_real_manual_compute(fft_ctx *ctx) {
    log_debug("Computing FFT");
    fftw_execute(ctx->plan);
}

int fft_real_compute(fft_ctx *ctx, double *input, double *output) {
    log_info("Computing real FFT");

    if (ctx->data_type != FFT_DATA_TYPE_REAL) {
        log_error("Invalid data type");
        return EXIT_FAILURE;
    }

    log_debug("Copying input values");
    memcpy(ctx->real_input, input, ctx->size * sizeof(double));

    log_debug("Computing FFT");
    fft_real_manual_compute(ctx);

    log_debug("Copying output values");
    memcpy(output, ctx->real_output, ctx->size * sizeof(double));

    return EXIT_SUCCESS;
}

int fft_real_compute_int8(fft_ctx *ctx, const int8_t *input, int8_t *output) {
    size_t i;

    log_debug("Copying input values");
    for (i = 0; i < ctx->size; i++)
        ctx->real_input[i] = (double) input[i];

    log_debug("Computing FFT");
    fft_real_manual_compute(ctx);

    log_debug("Copying output values");
    for (i = 0; i < ctx->size; i++)
        output[i] = (int8_t) (ctx->real_output[i] / ctx->size);

    return EXIT_SUCCESS;
}
