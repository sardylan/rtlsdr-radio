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

#include "resample.h"
#include "log.h"
#include "cfg.h"

#define src_ch_layout AV_CH_LAYOUT_MONO
#define dst_ch_layout AV_CH_LAYOUT_MONO

#define src_sample_fmt AV_SAMPLE_FMT_U8
#define dst_sample_fmt AV_SAMPLE_FMT_U8

extern cfg *conf;

resample_ctx *resample_init() {
    resample_ctx *ctx;
    int result;

    log_info("Initializing resample context");

    log_debug("Create resample context");
    ctx = (resample_ctx *) malloc(sizeof(resample_ctx));

    log_debug("Setting src and dest sample rate");
    ctx->src_rate = conf->rtlsdr_device_sample_rate;
    ctx->dst_rate = conf->audio_sample_rate;

    ctx->ratio = ctx->src_rate / ctx->dst_rate;

    log_debug("initializing variables");
    ctx->src_nb_channels = 0;
    ctx->dst_nb_channels = 0;

    ctx->src_nb_samples = 1024;

    log_debug("Create libswresample context");
    ctx->swr_ctx = swr_alloc();
    if (!ctx->swr_ctx) {
        log_error("Could not allocate libswresample context\n");
        resample_free(ctx);
        return NULL;
    }

    log_debug("Setting libswresample options");
    av_opt_set_int(ctx->swr_ctx, "in_channel_layout", src_ch_layout, 0);
    av_opt_set_int(ctx->swr_ctx, "in_sample_rate", ctx->src_rate, 0);
    av_opt_set_sample_fmt(ctx->swr_ctx, "in_sample_fmt", src_sample_fmt, 0);
    av_opt_set_int(ctx->swr_ctx, "out_channel_layout", dst_ch_layout, 0);
    av_opt_set_int(ctx->swr_ctx, "out_sample_rate", ctx->dst_rate, 0);
    av_opt_set_sample_fmt(ctx->swr_ctx, "out_sample_fmt", dst_sample_fmt, 0);

    log_debug("Initializing the resampling context");
    result = swr_init(ctx->swr_ctx);
    if (result < 0) {
        log_error("Failed to initialize the resampling context");
        resample_free(ctx);
        return NULL;
    }

    log_debug("Allocate source samples buffers");
    ctx->src_nb_channels = av_get_channel_layout_nb_channels(src_ch_layout);
    result = av_samples_alloc_array_and_samples(&ctx->src_data,
                                                &ctx->src_linesize,
                                                ctx->src_nb_channels,
                                                ctx->src_nb_samples,
                                                src_sample_fmt,
                                                0);
    if (result < 0) {
        log_error("Could not allocate source samples");
        resample_free(ctx);
        return NULL;
    }

    log_debug("Compute the number of converted samples");
    ctx->max_dst_nb_samples = av_rescale_rnd(ctx->src_nb_samples, ctx->dst_rate, ctx->src_rate, AV_ROUND_UP);
    ctx->dst_nb_samples = ctx->max_dst_nb_samples;

    log_debug("Allocate destination samples buffers");
    ctx->dst_nb_channels = av_get_channel_layout_nb_channels(dst_ch_layout);
    result = av_samples_alloc_array_and_samples(&ctx->dst_data,
                                                &ctx->dst_linesize,
                                                ctx->dst_nb_channels,
                                                ctx->dst_nb_samples,
                                                dst_sample_fmt,
                                                0);
    if (result < 0) {
        log_error("Could not allocate destination samples");
        resample_free(ctx);
        return NULL;
    }

    return ctx;
}

void resample_free(resample_ctx *ctx) {
    if (ctx->src_data)
        av_freep(&ctx->src_data[0]);

    av_freep(&ctx->src_data);

    if (ctx->dst_data)
        av_freep(&ctx->dst_data[0]);

    av_freep(&ctx->dst_data);

    swr_free(&ctx->swr_ctx);

    if (ctx->swr_ctx != NULL)
        free(ctx->swr_ctx);

    free(ctx);
}

size_t resample_compute_output_size(resample_ctx *ctx, size_t input_size) {
    return input_size / ctx->ratio;
}

int resample_do(resample_ctx *ctx, const int8_t *input, size_t input_size, int8_t *output, size_t output_size) {
    int result;
    int64_t delay;

    int dst_bufsize;

    log_info("Resampling");

    log_debug("Copy input buffer");
    memcpy(ctx->src_data[0], input, input_size);

    log_debug("Compute destination number of samples");
    delay = swr_get_delay(ctx->swr_ctx, ctx->src_rate) + ctx->src_nb_samples;
    ctx->dst_nb_samples = av_rescale_rnd(delay, ctx->dst_rate, ctx->src_rate, AV_ROUND_UP);

    if (ctx->dst_nb_samples > ctx->max_dst_nb_samples) {
        av_free(ctx->dst_data[0]);
        result = av_samples_alloc(ctx->dst_data,
                                  &ctx->dst_linesize,
                                  ctx->dst_nb_channels,
                                  ctx->dst_nb_samples,
                                  dst_sample_fmt,
                                  1);
        if (result < 0) {
            log_error("Unable to alloc");
            return EXIT_FAILURE;
        }

        ctx->max_dst_nb_samples = ctx->dst_nb_samples;
    }

    log_debug("Resample");
    result = swr_convert(ctx->swr_ctx,
                         ctx->dst_data,
                         ctx->dst_nb_samples,
                         (const uint8_t **) ctx->src_data,
                         ctx->src_nb_samples);
    if (result < 0) {
        log_error("Error while converting");
        return EXIT_FAILURE;
    }

    dst_bufsize = av_samples_get_buffer_size(&ctx->dst_linesize,
                                             ctx->dst_nb_channels,
                                             result,
                                             dst_sample_fmt,
                                             1);

    memcpy(output, ctx->dst_data[0], dst_bufsize);

//    if ((ret = get_format_from_sample_fmt(&fmt, dst_sample_fmt)) < 0)
//        goto end;
//    fprintf(stderr, "Resampling succeeded. Play the output file with the command:\n"
//                    "ffplay -f %s -channel_layout %"PRId64" -channels %d -ar %d %s\n",
//            fmt, dst_ch_layout, dst_nb_channels, dst_rate, dst_filename);

    return EXIT_SUCCESS;
}
