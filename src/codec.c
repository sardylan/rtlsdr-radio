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


#include <malloc.h>

#include "codec.h"
#include "log.h"

codec_ctx *codec_init(int mode) {
    codec_ctx *ctx;

    log_info("Initializing context");

    log_debug("Allocating context");
    ctx = (codec_ctx *) malloc(sizeof(codec_ctx));
    if (ctx == NULL) {
        log_error("Unable to allocate context");
        return NULL;
    }

    log_debug("Setting sample rate");
    ctx->codec2 = codec2_create(mode);
    if (ctx->codec2 == NULL) {
        log_error("Unable to allocate context");
        codec_free(ctx);
        return NULL;
    }

    ctx->pcm_size = codec2_samples_per_frame(ctx->codec2);
    ctx->data_size = (codec2_bits_per_frame(ctx->codec2) + 7) / 8;

    return ctx;
}

void codec_free(codec_ctx *ctx) {
    log_info("Freeing context");

    if (ctx == NULL)
        return;

    if (ctx->codec2 != NULL)
        codec2_destroy(ctx->codec2);

    free(ctx);
}

size_t codec_get_pcm_size(codec_ctx *ctx) {
    return ctx->pcm_size;
}

size_t codec_get_data_size(codec_ctx *ctx) {
    return ctx->data_size;
}

void codec_encode(codec_ctx *ctx, int16_t *pcm, uint8_t *output) {
    log_debug("Encoding");

    codec2_encode(ctx->codec2, output, pcm);
}

void codec_decode(codec_ctx *ctx, uint8_t *input, int16_t *pcm) {
    log_debug("Decoding");

    codec2_decode(ctx->codec2, pcm, input);
}
