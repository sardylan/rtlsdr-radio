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

#include "fir.h"
#include "log.h"

fir_ctx *fir_init(const double *kernel, size_t kernel_size) {
    fir_ctx *ctx;
    size_t i;

    ctx = (fir_ctx *) malloc(sizeof(fir_ctx));
    if (ctx == NULL) {
        log_error("Unable to allocate context");
        return NULL;
    }

    ctx->kernel_size = kernel_size;

    ctx->kernel = (double *) calloc(ctx->kernel_size, sizeof(double));
    if (ctx->kernel == NULL) {
        log_error("Unable to allocate kernel buffer");
        fir_free(ctx);
        return NULL;
    }

    ctx->prev = (int8_t *) calloc(ctx->kernel_size, sizeof(int8_t));
    if (ctx->prev == NULL) {
        log_error("Unable to allocate previous values buffer");
        fir_free(ctx);
        return NULL;
    }

    for (i = 0; i < ctx->kernel_size; i++)
        ctx->prev[i] = 0;

    memcpy(ctx->kernel, kernel, ctx->kernel_size * sizeof(double));

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
    size_t input_pos;
    int8_t *item;
    double sum;

    for (i = 0; i < size; i++) {
        sum = 0;

        for (j = 0; j < ctx->kernel_size; j++) {
            input_pos = i - j;

            if (input_pos >= 0)
                item = (int8_t *) &input[input_pos];
            else
                item = &ctx->prev[ctx->kernel_size - j];

            sum += (*item) * (ctx->kernel[(ctx->kernel_size - 1) - j]);
        }

        output[i] = sum;
    }

    memcpy(ctx->prev, &output[size - ctx->kernel_size], ctx->kernel_size * sizeof(int8_t));

    return EXIT_SUCCESS;
}
