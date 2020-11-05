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
#include <math.h>

#include "resample.h"
#include "log.h"

resample_ctx *resample_init(int src_rate, int dst_rate) {
    resample_ctx *ctx;
    uint32_t steps;
    int i;

    ctx = (resample_ctx *) malloc(sizeof(resample_ctx));

    ctx->src_rate = src_rate;
    ctx->dst_rate = dst_rate;

    ctx->ratio = src_rate / dst_rate;

    steps = ctx->ratio / 2;
    ctx->start = -steps;
    ctx->stop = steps;

    ctx->coeff = (double *) calloc(steps, sizeof(double));
    for (i = 1; i <= steps; i++)
        ctx->coeff[i] = (double) 1.0 / i;

    return ctx;
}

void resample_free(resample_ctx *ctx) {
    free(ctx->coeff);

    free(ctx);
}

size_t resample_compute_output_size(resample_ctx *ctx, size_t input_size) {
    return input_size / ctx->ratio;
}

int resample_do(resample_ctx *ctx, const int8_t *input, size_t input_size, int8_t *output, size_t output_size) {
    size_t i;
    int j;
    int64_t shift;
    int64_t pos;

    for (i = 0; i < output_size; i++) {
        shift = ctx->ratio * i;
        if (shift >= input_size) {
            break;
        }

        output[i] = 0;

        for (j = ctx->start; j < ctx->stop; j++) {
            if ((shift + j) < 0)
                continue;

            pos = shift + j;

            if (pos > input_size)
                break;

            output[i] += (int8_t) (input[pos] * ctx->coeff[abs(j)]);
        }

        output[i] = (int8_t) ((double) output[i] / ctx->steps);
    }

    return EXIT_SUCCESS;
}
