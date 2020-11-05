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


#ifndef __RTLSDR_RADIO__RESAMPLE__H
#define __RTLSDR_RADIO__RESAMPLE__H

#include <stdint.h>
#include <stddef.h>

#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>

struct resample_ctx_t {
    uint32_t src_rate;
    uint32_t dst_rate;

    uint32_t ratio;

    struct SwrContext *swr_ctx;

    uint8_t **src_data;
    uint8_t **dst_data;

    int src_nb_channels;
    int dst_nb_channels;

    int src_linesize;
    int dst_linesize;

    int src_nb_samples;
    int dst_nb_samples;

    int max_dst_nb_samples;
};

typedef struct resample_ctx_t resample_ctx;

resample_ctx *resample_init();

void resample_free(resample_ctx *);

size_t resample_compute_output_size(resample_ctx *, size_t);

int resample_do(resample_ctx *ctx, const int8_t *, size_t, int8_t *, size_t);

#endif
