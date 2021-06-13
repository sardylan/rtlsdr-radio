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
#include <math.h>

#include "agc.h"
#include "log.h"
#include "dsp.h"

agc_ctx *agc_init() {
    agc_ctx *ctx;

    log_info("Initializing AGC");

    log_debug("Create AGC context");
    ctx = (agc_ctx *) malloc(sizeof(agc_ctx));
    if (ctx == NULL) {
        log_error("Unable to allocate AGC context");
        return NULL;
    }

    log_debug("Setting default values");
    ctx->gain = 1;
    ctx->dc_offset = 0;

    ctx->gain_factor = 0.05f;
    ctx->dc_offset_factor = 0.01f;

    return ctx;
}

void agc_free(agc_ctx *ctx) {
    free(ctx);
}

int agc_perform_gain(agc_ctx *ctx, FP_FLOAT *data, size_t len) {
    size_t i;

    FP_FLOAT max;
    FP_FLOAT clip_gain;

    log_debug("Performing gain computations");

    max = 0;

    for (i = 0; i < len; i++)
        if (data[i] >= 0 && data[i] > max)
            max = data[i];
        else if (data[i] < 0 && data[i] < -max)
            max = -data[i];

    if (max > 0)
        clip_gain = 1 / max;
    else
        clip_gain = 0;

    if (ctx->gain < clip_gain)
        ctx->gain += ctx->gain_factor;
    else if (ctx->gain * max > 1)
        ctx->gain -= ctx->gain_factor;

    for (i = 0; i < len; i++)
        *(data + i) = *(data + i) * ctx->gain;

    return EXIT_SUCCESS;
}

//int agc_perform_gain(int8_t *data, size_t data_size) {
//    size_t i;
//    int8_t abs_value;
//    int8_t max;
//    FP_FLOAT rms;
//    FP_FLOAT gain;
//
//    log_info("Applying limiter");
//
//    rms = dsp_rms(data, data_size);
//    gain = pow(2, rms + 4);
//
//    max = 0;
//
//    for (i = 0; i < data_size; i++) {
//        abs_value = abs(data[i]);
//        if (abs_value > max)
//            max = abs_value;
//    }
//
//    if (max * gain > 127)
//        gain = 127.0 / max;
//
//    for (i = 0; i < data_size; i++)
//        data[i] *= gain;
//
//    return EXIT_SUCCESS;
//}
