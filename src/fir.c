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


#include <stdlib.h>
#include <malloc.h>
#include <string.h>

#include "fir.h"
#include "fir_lpf.h"
#include "log.h"

fir_ctx *fir_init(const FP_FLOAT *kernel, size_t kernel_size) {
    fir_ctx *ctx;
    size_t i;

    ctx = (fir_ctx *) malloc(sizeof(fir_ctx));
    if (ctx == NULL) {
        log_error("Unable to allocate context");
        return NULL;
    }

    ctx->kernel_size = kernel_size;

    ctx->kernel = (FP_FLOAT *) calloc(ctx->kernel_size, sizeof(FP_FLOAT));
    if (ctx->kernel == NULL) {
        log_error("Unable to allocate kernel buffer");
        fir_free(ctx);
        return NULL;
    }

    ctx->prev = (int8_t *) calloc((ctx->kernel_size - 1), sizeof(int8_t));
    if (ctx->prev == NULL) {
        log_error("Unable to allocate previous values buffer");
        fir_free(ctx);
        return NULL;
    }

    for (i = 0; i < ctx->kernel_size; i++)
        ctx->prev[i] = 0;

    memcpy(ctx->kernel, kernel, ctx->kernel_size * sizeof(FP_FLOAT));

    return ctx;
}

void fir_free(fir_ctx *ctx) {
    if (ctx->prev != NULL)
        free(ctx->prev);

    if (ctx->kernel != NULL)
        free(ctx->kernel);

    free(ctx);
}

int fir_convolve(fir_ctx *ctx, int8_t *output, const int8_t *input, size_t size) {
    size_t i;
    size_t j;
    ssize_t input_pos;
    int8_t *item;
    FP_FLOAT sum;

    for (i = 0; i < size; i++) {
        sum = 0;

        for (j = 0; j < ctx->kernel_size; j++) {
            input_pos = i - j;

            if (input_pos >= 0)
                item = (int8_t *) &input[input_pos];
            else
                item = &ctx->prev[ctx->kernel_size + input_pos];

            sum += (*item) * (ctx->kernel[j]);
        }

        output[i] = (int8_t) sum;
    }

    memcpy(ctx->prev, &input[size - ctx->kernel_size], (ctx->kernel_size - 1) * sizeof(int8_t));

    return EXIT_SUCCESS;
}

fir_ctx *fir_init_lpf(int number) {
    switch (number) {
        case 1:
            return fir_init(fir_kernel_lpf1_filter_taps, FIR_KERNEL_LPF1_TAP_NUM);
        case 2:
            return fir_init(fir_kernel_lpf2_filter_taps, FIR_KERNEL_LPF2_TAP_NUM);
        case 3:
            return fir_init(fir_kernel_lpf3_filter_taps, FIR_KERNEL_LPF3_TAP_NUM);
        case 4:
            return fir_init(fir_kernel_lpf4_filter_taps, FIR_KERNEL_LPF4_TAP_NUM);
        case 5:
            return fir_init(fir_kernel_lpf5_filter_taps, FIR_KERNEL_LPF5_TAP_NUM);
        default:
            return NULL;
    }
}
