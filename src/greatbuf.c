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
#include <pthread.h>

#include "greatbuf.h"
#include "log.h"

greatbuf_ctx *greatbuf_init(size_t size, size_t sample_size, size_t pcm_size) {
    greatbuf_ctx *ctx;
    greatbuf_item *item;
    size_t i;
    int result;

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

    log_debug("Initializing IQ mutex");
    result = pthread_mutex_init(&ctx->mutex_iq, NULL);
    if (result != 0) {
        log_error("Error initializing mutex: %d", result);
        greatbuf_free(ctx);
        return NULL;
    }

    log_debug("Initializing IQ condition");
    result = pthread_cond_init(&ctx->cond_iq, NULL);
    if (result != 0) {
        log_error("Error initializing condition: %d", result);
        greatbuf_free(ctx);
        return NULL;
    }

    log_debug("Initializing samples mutex");
    result = pthread_mutex_init(&ctx->mutex_samples, NULL);
    if (result != 0) {
        log_error("Error initializing mutex: %d", result);
        greatbuf_free(ctx);
        return NULL;
    }

    log_debug("Initializing samples condition");
    result = pthread_cond_init(&ctx->cond_samples, NULL);
    if (result != 0) {
        log_error("Error initializing condition: %d", result);
        greatbuf_free(ctx);
        return NULL;
    }

    log_debug("Initializing demod mutex");
    result = pthread_mutex_init(&ctx->mutex_demod, NULL);
    if (result != 0) {
        log_error("Error initializing mutex: %d", result);
        greatbuf_free(ctx);
        return NULL;
    }

    log_debug("Initializing demod condition");
    result = pthread_cond_init(&ctx->cond_demod, NULL);
    if (result != 0) {
        log_error("Error initializing condition: %d", result);
        greatbuf_free(ctx);
        return NULL;
    }

    log_debug("Initializing filtered mutex");
    result = pthread_mutex_init(&ctx->mutex_filtered, NULL);
    if (result != 0) {
        log_error("Error initializing mutex: %d", result);
        greatbuf_free(ctx);
        return NULL;
    }

    log_debug("Initializing filtered condition");
    result = pthread_cond_init(&ctx->cond_filtered, NULL);
    if (result != 0) {
        log_error("Error initializing condition: %d", result);
        greatbuf_free(ctx);
        return NULL;
    }

    log_debug("Initializing pcm mutex");
    result = pthread_mutex_init(&ctx->mutex_pcm, NULL);
    if (result != 0) {
        log_error("Error initializing mutex: %d", result);
        greatbuf_free(ctx);
        return NULL;
    }

    log_debug("Initializing pcm condition");
    result = pthread_cond_init(&ctx->cond_pcm, NULL);
    if (result != 0) {
        log_error("Error initializing condition: %d", result);
        greatbuf_free(ctx);
        return NULL;
    }

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

    log_debug("Destroying mutexes");
    pthread_mutex_destroy(&ctx->mutex_iq);
    pthread_mutex_destroy(&ctx->mutex_samples);
    pthread_mutex_destroy(&ctx->mutex_demod);
    pthread_mutex_destroy(&ctx->mutex_filtered);
    pthread_mutex_destroy(&ctx->mutex_pcm);

    log_debug("Destroying conds");
    pthread_cond_destroy(&ctx->cond_iq);
    pthread_cond_destroy(&ctx->cond_samples);
    pthread_cond_destroy(&ctx->cond_demod);
    pthread_cond_destroy(&ctx->cond_filtered);
    pthread_cond_destroy(&ctx->cond_pcm);

    log_trace("Freeing context");
    free(ctx);
}

size_t greatbuf_iq_head_start(greatbuf_ctx *ctx) {
    size_t pos;

    log_debug("Getting Great Buffer IQ head");

    log_trace("Locking mutex");
    pthread_mutex_lock(&ctx->mutex_iq);

    pos = ctx->pos_iq_head;
    log_trace("Position is %zu", pos);

    log_trace("Unlocking mutex");
    pthread_mutex_unlock(&ctx->mutex_iq);

    return pos;
}

void greatbuf_iq_head_stop(greatbuf_ctx *ctx) {
    log_debug("Incrementing Great Buffer IQ head");

    log_trace("Locking mutex");
    pthread_mutex_lock(&ctx->mutex_iq);

    log_trace("Incrementing position");
    ctx->pos_iq_head++;
    if (ctx->pos_iq_head >= ctx->iq_size)
        ctx->pos_iq_head = 0;

    pthread_cond_signal(&ctx->cond_iq);

    log_trace("Unlocking mutex");
    pthread_mutex_unlock(&ctx->mutex_iq);
}

size_t greatbuf_iq_tail(greatbuf_ctx *ctx) {
    size_t pos;

    log_debug("Getting Great Buffer IQ tail");

    log_trace("Locking mutex");
    pthread_mutex_lock(&ctx->mutex_iq);

    pos = ctx->pos_iq_tail;
    log_trace("Position is %zu", pos);

    while (pos == ctx->pos_iq_head)
        pthread_cond_wait(&ctx->cond_iq, &ctx->mutex_iq);

    log_trace("Incrementing position");
    ctx->pos_iq_tail++;
    if (ctx->pos_iq_tail >= ctx->iq_size)
        ctx->pos_iq_tail = 0;

    log_trace("Unlocking mutex");
    pthread_mutex_unlock(&ctx->mutex_iq);

    return pos;
}

size_t greatbuf_samples_head_start(greatbuf_ctx *ctx) {
    size_t pos;

    log_debug("Getting Great Buffer samples head");

    log_trace("Locking mutex");
    pthread_mutex_lock(&ctx->mutex_samples);

    pos = ctx->pos_samples_head;
    log_trace("Position is %zu", pos);

    log_trace("Unlocking mutex");
    pthread_mutex_unlock(&ctx->mutex_samples);

    return pos;
}

void greatbuf_samples_head_stop(greatbuf_ctx *ctx) {
    log_debug("Incrementing Great Buffer samples head");

    log_trace("Locking mutex");
    pthread_mutex_lock(&ctx->mutex_samples);

    log_trace("Incrementing position");
    ctx->pos_samples_head++;
    if (ctx->pos_samples_head >= ctx->samples_size)
        ctx->pos_samples_head = 0;

    pthread_cond_signal(&ctx->cond_samples);

    log_trace("Unlocking mutex");
    pthread_mutex_unlock(&ctx->mutex_samples);
}

size_t greatbuf_samples_tail(greatbuf_ctx *ctx) {
    size_t pos;

    log_debug("Getting Great Buffer samples tail");

    log_trace("Locking mutex");
    pthread_mutex_lock(&ctx->mutex_samples);

    pos = ctx->pos_samples_tail;
    log_trace("Position is %zu", pos);

    while (pos == ctx->pos_iq_head)
        pthread_cond_wait(&ctx->cond_samples, &ctx->mutex_samples);

    log_trace("Incrementing position");
    ctx->pos_samples_tail++;
    if (ctx->pos_samples_tail >= ctx->samples_size)
        ctx->pos_samples_tail = 0;

    log_trace("Unlocking mutex");
    pthread_mutex_unlock(&ctx->mutex_samples);

    return pos;
}

size_t greatbuf_demod_head_start(greatbuf_ctx *ctx) {
    size_t pos;

    log_debug("Getting Great Buffer demod head");

    log_trace("Locking mutex");
    pthread_mutex_lock(&ctx->mutex_demod);

    pos = ctx->pos_demod_head;
    log_trace("Position is %zu", pos);

    log_trace("Unlocking mutex");
    pthread_mutex_unlock(&ctx->mutex_demod);

    return pos;
}

void greatbuf_demod_head_stop(greatbuf_ctx *ctx) {
    log_debug("Incrementing Great Buffer demod head");

    log_trace("Locking mutex");
    pthread_mutex_lock(&ctx->mutex_demod);

    log_trace("Incrementing position");
    ctx->pos_demod_head++;
    if (ctx->pos_demod_head >= ctx->demod_size)
        ctx->pos_demod_head = 0;

    pthread_cond_signal(&ctx->cond_demod);

    log_trace("Unlocking mutex");
    pthread_mutex_unlock(&ctx->mutex_demod);
}

size_t greatbuf_demod_tail(greatbuf_ctx *ctx) {
    size_t pos;

    log_debug("Getting Great Buffer demod tail");

    log_trace("Locking mutex");
    pthread_mutex_lock(&ctx->mutex_demod);

    pos = ctx->pos_demod_tail;
    log_trace("Position is %zu", pos);

    while (pos == ctx->pos_iq_head)
        pthread_cond_wait(&ctx->cond_demod, &ctx->mutex_demod);

    log_trace("Incrementing position");
    ctx->pos_demod_tail++;
    if (ctx->pos_demod_tail >= ctx->demod_size)
        ctx->pos_demod_tail = 0;

    log_trace("Unlocking mutex");
    pthread_mutex_unlock(&ctx->mutex_demod);

    return pos;
}

size_t greatbuf_filtered_head_start(greatbuf_ctx *ctx) {
    size_t pos;

    log_debug("Getting Great Buffer filterd head");

    log_trace("Locking mutex");
    pthread_mutex_lock(&ctx->mutex_filtered);

    pos = ctx->pos_filtered_head;
    log_trace("Position is %zu", pos);

    log_trace("Unlocking mutex");
    pthread_mutex_unlock(&ctx->mutex_filtered);

    return pos;
}

void greatbuf_filtered_head_stop(greatbuf_ctx *ctx) {
    log_debug("Incrementing Great Buffer filterd head");

    log_trace("Locking mutex");
    pthread_mutex_lock(&ctx->mutex_filtered);

    log_trace("Incrementing position");
    ctx->pos_filtered_head++;
    if (ctx->pos_filtered_head >= ctx->filtered_size)
        ctx->pos_filtered_head = 0;

    pthread_cond_signal(&ctx->cond_filtered);

    log_trace("Unlocking mutex");
    pthread_mutex_unlock(&ctx->mutex_filtered);
}

size_t greatbuf_filtered_tail(greatbuf_ctx *ctx) {
    size_t pos;

    log_debug("Getting Great Buffer filterd tail");

    log_trace("Locking mutex");
    pthread_mutex_lock(&ctx->mutex_filtered);

    pos = ctx->pos_filtered_tail;
    log_trace("Position is %zu", pos);

    while (pos == ctx->pos_iq_head)
        pthread_cond_wait(&ctx->cond_filtered, &ctx->mutex_filtered);

    log_trace("Incrementing position");
    ctx->pos_filtered_tail++;
    if (ctx->pos_filtered_tail >= ctx->filtered_size)
        ctx->pos_filtered_tail = 0;

    log_trace("Unlocking mutex");
    pthread_mutex_unlock(&ctx->mutex_filtered);

    return pos;
}

size_t greatbuf_pcm_head_start(greatbuf_ctx *ctx) {
    size_t pos;

    log_debug("Getting Great Buffer pcm head");

    log_trace("Locking mutex");
    pthread_mutex_lock(&ctx->mutex_pcm);

    pos = ctx->pos_pcm_head;
    log_trace("Position is %zu", pos);

    log_trace("Unlocking mutex");
    pthread_mutex_unlock(&ctx->mutex_pcm);

    return pos;
}

void greatbuf_pcm_head_stop(greatbuf_ctx *ctx) {
    log_debug("Incrementing Great Buffer pcm head");

    log_trace("Locking mutex");
    pthread_mutex_lock(&ctx->mutex_pcm);

    log_trace("Incrementing position");
    ctx->pos_pcm_head++;
    if (ctx->pos_pcm_head >= ctx->pcm_size)
        ctx->pos_pcm_head = 0;

    pthread_cond_signal(&ctx->cond_pcm);

    log_trace("Unlocking mutex");
    pthread_mutex_unlock(&ctx->mutex_pcm);
}

size_t greatbuf_pcm_tail(greatbuf_ctx *ctx) {
    size_t pos;

    log_debug("Getting Great Buffer pcm tail");

    log_trace("Locking mutex");
    pthread_mutex_lock(&ctx->mutex_pcm);

    pos = ctx->pos_pcm_tail;
    log_trace("Position is %zu", pos);

    while (pos == ctx->pos_iq_head)
        pthread_cond_wait(&ctx->cond_pcm, &ctx->mutex_pcm);

    log_trace("Incrementing position");
    ctx->pos_pcm_tail++;
    if (ctx->pos_pcm_tail >= ctx->pcm_size)
        ctx->pos_pcm_tail = 0;

    log_trace("Unlocking mutex");
    pthread_mutex_unlock(&ctx->mutex_pcm);

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
