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

#include "audiocodec.h"
#include "log.h"

#define AUDIOCODEC_CHANNELS 1
#define AUDIOCODEC_APPLICATION OPUS_APPLICATION_AUDIO

audiocodec_ctx *audiocodec_init(int32_t sample_rate, int32_t bitrate) {
    audiocodec_ctx *ctx;
    int result;

    int channels;
    int application;

    log_info("Initializing context");

    channels = AUDIOCODEC_CHANNELS;
    application = AUDIOCODEC_APPLICATION;

    log_debug("Allocating context");
    ctx = (audiocodec_ctx *) malloc(sizeof(audiocodec_ctx));
    if (ctx == NULL) {
        log_error("Unable to allocate context");
        return NULL;
    }

    log_debug("Setting sample rate");
    ctx->sample_rate = sample_rate;

    log_debug("Creating encoder");
    ctx->encoder = opus_encoder_create(ctx->sample_rate, channels, application, &result);
    if (result < 0) {
        log_error("Unable to create encoder: %s", opus_strerror(result));

        ctx->encoder = NULL;
        audiocodec_free(ctx);

        return NULL;
    }

    log_debug("Setting encoder bitrate");
    ctx->bitrate = bitrate;
    result = opus_encoder_ctl(ctx->encoder, OPUS_SET_BITRATE(ctx->bitrate));
    if (result < 0) {
        log_error("Unable to set bitrate encoder: %s", opus_strerror(result));
        audiocodec_free(ctx);
        return NULL;
    }

    log_debug("Allocation input buffer");
    ctx->input = (opus_int16 *) calloc(AUDIOCODEC_FRAME_SIZE, sizeof(opus_int16));
    if (ctx->input == NULL) {
        log_error("Unable to allocate input buffer");
        audiocodec_free(ctx);
        return NULL;
    }

    return ctx;
}

void audiocodec_free(audiocodec_ctx *ctx) {
    log_info("Freeing context");

    if (ctx->encoder != NULL)
        opus_encoder_destroy(ctx->encoder);

    if (ctx->input != NULL)
        free(ctx->input);

    free(ctx);
}

int audiocodec_encode(audiocodec_ctx *ctx,
                      const int8_t *pcm,
                      uint8_t *output, size_t output_size,
                      size_t *bytes_written) {
    opus_int32 result;
    int i;

    log_info("Encoding");

    for (i = 0; i < AUDIOCODEC_FRAME_SIZE; i++)
        ctx->input[i] = pcm[i] << 8;

    result = opus_encode(ctx->encoder, ctx->input, AUDIOCODEC_FRAME_SIZE, output, output_size);
    if (result != OPUS_OK) {
        log_error("Unable to encode data: %s", opus_strerror(result));
        *bytes_written = 0;
        return EXIT_FAILURE;
    }

    *bytes_written = result;

    return EXIT_SUCCESS;
}
