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


#include <stdlib.h>

#include "resample.h"
#include "log.h"
#include "cfg.h"

resample_ctx *resample_init(uint32_t src_rate, uint32_t dst_rate) {
    resample_ctx *ctx;
    log_info("Initializing resample context");

    log_debug("Create resample context");
    ctx = (resample_ctx *) malloc(sizeof(resample_ctx));
    if (ctx == NULL) {
        log_error("Unable to allocate resample context");
        return NULL;
    }

    log_debug("Setting src and dest sample rate");
    ctx->src_rate = src_rate;
    ctx->dst_rate = dst_rate;

    log_debug("Computing resample ratio");
    ctx->ratio = ctx->src_rate / ctx->dst_rate;

    return ctx;
}

void resample_free(resample_ctx *ctx) {
    free(ctx);
}

size_t resample_compute_output_size(resample_ctx *ctx, size_t input_size) {
    return input_size / ctx->ratio;
}

int resample_do(resample_ctx *ctx, const int8_t *input, size_t input_size, int8_t *output, size_t output_size) {
    size_t i;
    size_t shift;

    log_info("Resampling");

    // TODO:
    //  This function just skips some samples, which is not a real resampling,
    //  but since we applied a low-pass filter before, it could works.

    for (i = 0; i < output_size; i++) {
        shift = ctx->ratio * i;
        if (shift >= input_size)
            break;
        output[i] = input[shift];
    }

    return EXIT_SUCCESS;
}
