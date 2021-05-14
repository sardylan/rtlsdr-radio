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

#include "greatbuf.h"
#include "log.h"

greatbuf_ctx *greatbuf_init(size_t size, size_t sample_size, size_t pcm_size) {
    greatbuf_ctx *ctx;
    greatbuf_item *item;
    size_t i;

    log_debug("Initializing Great Buffer");

    log_trace("Allocating Great Buffer");
    ctx = (greatbuf_ctx *) malloc(sizeof(greatbuf_ctx));
    if (ctx == NULL) {
        log_error("Unable to allocate Great Buffer");
        return NULL;
    }

    log_trace("Setting sizes");
    ctx->iq_size = sample_size * 2;
    ctx->samples_size = sample_size;
    ctx->demod_size = sample_size;
    ctx->filtered_size = sample_size;
    ctx->pcm_size = pcm_size;
    ctx->size = size;

    log_trace("Allocating buffer");
    ctx->buffer = (greatbuf_item *) calloc(ctx->size, sizeof(greatbuf_item));
    if (ctx->buffer == NULL) {
        log_error("Unable to allocate buffer");
        greatbuf_free(ctx);
        return NULL;
    }

    log_trace("Initializing buffer");
    for (i = 0; i < ctx->size; i++) {
        item = &ctx->buffer[i];

        item->number = 0;

        item->ts.tv_sec = 0;
        item->ts.tv_nsec = 0;

        item->iq = (uint8_t *) calloc(ctx->size, sizeof(uint8_t));
        if (item->iq == NULL) {
            log_error("Unable to allocate IQ buffer");
            greatbuf_free(ctx);
            return NULL;
        }

        item->samples = (FP_FLOAT complex *) calloc(ctx->size, sizeof(FP_FLOAT complex));
        if (item->samples == NULL) {
            log_error("Unable to allocate samples buffer");
            greatbuf_free(ctx);
            return NULL;
        }

        item->demod = (FP_FLOAT *) calloc(ctx->size, sizeof(FP_FLOAT));
        if (item->demod == NULL) {
            log_error("Unable to allocate demod buffer");
            greatbuf_free(ctx);
            return NULL;
        }

        item->filtered = (FP_FLOAT *) calloc(ctx->size, sizeof(FP_FLOAT));
        if (item->filtered == NULL) {
            log_error("Unable to allocate filtered buffer");
            greatbuf_free(ctx);
            return NULL;
        }

        item->pcm = (int16_t *) calloc(ctx->size, sizeof(int16_t));
        if (item->pcm == NULL) {
            log_error("Unable to allocate pcm buffer");
            greatbuf_free(ctx);
            return NULL;
        }
    }

    log_trace("Setting counter, heads and tails initial values");
    ctx->counter = 0;

    ctx->pos_iq_head = 0;
    ctx->pos_iq_tail = 0;

    ctx->pos_samples_head = 0;
    ctx->pos_samples_tail = 0;

    ctx->pos_demod_head = 0;
    ctx->pos_demod_tail = 0;

    ctx->pos_filtered_head = 0;
    ctx->pos_filtered_tail = 0;

    ctx->pos_pcm_head = 0;
    ctx->pos_pcm_tail = 0;

    return ctx;
}

void greatbuf_free(greatbuf_ctx *ctx) {
    greatbuf_item *item;
    size_t i;

    log_debug("Cleaning Great Buffer");

    if (ctx == NULL)
        return;

    if (ctx->buffer != NULL) {
        log_trace("Freeing buffer items");

        for (i = 0; i < ctx->size; i++) {
            item = &ctx->buffer[i];

            if (item->iq != NULL)
                free(item->iq);

            if (item->samples != NULL)
                free(item->samples);

            if (item->demod != NULL)
                free(item->demod);

            if (item->filtered != NULL)
                free(item->filtered);

            if (item->pcm != NULL)
                free(item->pcm);
        }

        log_trace("Freeing buffer");
        free(ctx->buffer);
    }

    log_trace("Freeing context");
    free(ctx);
}

size_t greatbuf_iq_head(greatbuf_ctx *ctx) {
    size_t pos;

    log_debug("Getting Great Buffer IQ head");

    pos = ctx->pos_iq_head;
    log_trace("Position is %zu", pos);

    log_trace("Incrementing position");
    ctx->pos_iq_head++;
    if (ctx->pos_iq_head > ctx->iq_size)
        ctx->pos_iq_head = 0;

    return pos;
}

size_t greatbuf_iq_tail(greatbuf_ctx *ctx) {
    size_t pos;

    log_debug("Getting Great Buffer IQ tail");

    pos = ctx->pos_iq_tail;
    log_trace("Position is %zu", pos);

    log_trace("Incrementing position");
    ctx->pos_iq_tail++;
    if (ctx->pos_iq_tail > ctx->iq_size)
        ctx->pos_iq_tail = 0;

    return pos;
}

size_t greatbuf_samples_head(greatbuf_ctx *ctx) {
    size_t pos;

    log_debug("Getting Great Buffer samples head");

    pos = ctx->pos_samples_head;
    log_trace("Position is %zu", pos);

    log_trace("Incrementing position");
    ctx->pos_samples_head++;
    if (ctx->pos_samples_head > ctx->samples_size)
        ctx->pos_samples_head = 0;

    return pos;
}

size_t greatbuf_samples_tail(greatbuf_ctx *ctx) {
    size_t pos;

    log_debug("Getting Great Buffer samples tail");

    pos = ctx->pos_samples_tail;
    log_trace("Position is %zu", pos);

    log_trace("Incrementing position");
    ctx->pos_samples_tail++;
    if (ctx->pos_samples_tail > ctx->samples_size)
        ctx->pos_samples_tail = 0;

    return pos;
}

size_t greatbuf_demod_head(greatbuf_ctx *ctx) {
    size_t pos;

    log_debug("Getting Great Buffer demod head");

    pos = ctx->pos_demod_head;
    log_trace("Position is %zu", pos);

    log_trace("Incrementing position");
    ctx->pos_demod_head++;
    if (ctx->pos_demod_head > ctx->demod_size)
        ctx->pos_demod_head = 0;

    return pos;
}

size_t greatbuf_demod_tail(greatbuf_ctx *ctx) {
    size_t pos;

    log_debug("Getting Great Buffer demod tail");

    pos = ctx->pos_demod_tail;
    log_trace("Position is %zu", pos);

    log_trace("Incrementing position");
    ctx->pos_demod_tail++;
    if (ctx->pos_demod_tail > ctx->demod_size)
        ctx->pos_demod_tail = 0;

    return pos;
}

size_t greatbuf_filtered_head(greatbuf_ctx *ctx) {
    size_t pos;

    log_debug("Getting Great Buffer filterd head");

    pos = ctx->pos_filtered_head;
    log_trace("Position is %zu", pos);

    log_trace("Incrementing position");
    ctx->pos_filtered_head++;
    if (ctx->pos_filtered_head > ctx->filtered_size)
        ctx->pos_filtered_head = 0;

    return pos;
}

size_t greatbuf_filtered_tail(greatbuf_ctx *ctx) {
    size_t pos;

    log_debug("Getting Great Buffer filterd tail");

    pos = ctx->pos_filtered_tail;
    log_trace("Position is %zu", pos);

    log_trace("Incrementing position");
    ctx->pos_filtered_tail++;
    if (ctx->pos_filtered_tail > ctx->filtered_size)
        ctx->pos_filtered_tail = 0;

    return pos;
}

size_t greatbuf_pcm_head(greatbuf_ctx *ctx) {
    size_t pos;

    log_debug("Getting Great Buffer pcm head");

    pos = ctx->pos_pcm_head;
    log_trace("Position is %zu", pos);

    log_trace("Incrementing position");
    ctx->pos_pcm_head++;
    if (ctx->pos_pcm_head > ctx->pcm_size)
        ctx->pos_pcm_head = 0;

    return pos;
}

size_t greatbuf_pcm_tail(greatbuf_ctx *ctx) {
    size_t pos;

    log_debug("Getting Great Buffer pcm tail");

    pos = ctx->pos_pcm_tail;
    log_trace("Position is %zu", pos);

    log_trace("Incrementing position");
    ctx->pos_pcm_tail++;
    if (ctx->pos_pcm_tail > ctx->pcm_size)
        ctx->pos_pcm_tail = 0;

    return pos;
}

void greatbuf_item_init(greatbuf_ctx *ctx, size_t pos) {
    greatbuf_item *item;

    item = &ctx->buffer[pos];

    item->number = ctx->counter;
    ctx->counter++;

    clock_gettime(CLOCK_REALTIME, &item->ts);
}

uint64_t greatbuf_item_number_get(greatbuf_ctx *ctx, size_t pos) {
    greatbuf_item *item;

    item = &ctx->buffer[pos];

    return item->number;
}

struct timespec *greatbuf_item_ts_get(greatbuf_ctx *ctx, size_t pos) {
    greatbuf_item *item;

    item = &ctx->buffer[pos];

    return &item->ts;
}

uint8_t *greatbuf_item_iq_get(greatbuf_ctx *ctx, size_t pos) {
    greatbuf_item *item;

    item = &ctx->buffer[pos];

    return item->iq;
}

FP_FLOAT complex *greatbuf_item_samples_get(greatbuf_ctx *ctx, size_t pos) {
    greatbuf_item *item;

    item = &ctx->buffer[pos];

    return item->samples;
}

FP_FLOAT *greatbuf_item_demod_get(greatbuf_ctx *ctx, size_t pos) {
    greatbuf_item *item;

    item = &ctx->buffer[pos];

    return item->demod;
}

FP_FLOAT *greatbuf_item_filtered_get(greatbuf_ctx *ctx, size_t pos) {
    greatbuf_item *item;

    item = &ctx->buffer[pos];

    return item->filtered;
}

int16_t *greatbuf_item_pcm_get(greatbuf_ctx *ctx, size_t pos) {
    greatbuf_item *item;

    item = &ctx->buffer[pos];

    return item->pcm;
}
