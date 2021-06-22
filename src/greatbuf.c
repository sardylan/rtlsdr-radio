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
#include <string.h>

#include "greatbuf.h"
#include "log.h"
#include "ui.h"

greatbuf_circbuf *greatbuf_circbuf_init(const char *name, size_t size) {
    greatbuf_circbuf *circbuf;
    size_t ln;
    int result;

    circbuf = NULL;

    log_info("Greatbuf circbuf init");

    log_debug("Allocating circbuf");
    circbuf = (greatbuf_circbuf *) malloc(sizeof(greatbuf_circbuf));
    if (circbuf == NULL) {
        log_error("Unable to allocate circbuf");
        return NULL;
    }

    log_debug("Setting name");
    ln = strlen(name);
    circbuf->name = (char *) calloc(ln + 1, sizeof(char));
    if (circbuf->name == NULL) {
        log_error("Unable to allocate circbuf name");
        greatbuf_circbuf_free(circbuf);
        return NULL;
    }
    strcpy(circbuf->name, name);

    log_debug("Setting initial values");
    circbuf->head = 0;
    circbuf->tail = 0;

    circbuf->size = size;
    circbuf->free = size;

    circbuf->busy_head = 0;
    circbuf->busy_tail = 0;

    circbuf->keep_running = 1;

    log_debug("Initializing mutex");
    result = pthread_mutex_init(&circbuf->mutex, NULL);
    if (result != 0) {
        log_error("Error initializing mutex: %d", result);
        greatbuf_circbuf_free(circbuf);
        return NULL;
    }

    log_debug("Initializing condition");
    result = pthread_cond_init(&circbuf->cond, NULL);
    if (result != 0) {
        log_error("Error initializing condition: %d", result);
        greatbuf_circbuf_free(circbuf);
        return NULL;
    }

    return circbuf;
}

void greatbuf_circbuf_free(greatbuf_circbuf *circbuf) {
    log_info("Freeing greatbuf circbuf");

    if (circbuf == NULL)
        return;

    if (circbuf->name != NULL)
        free(circbuf->name);

    log_debug("Destroying mutex");
    pthread_mutex_destroy(&circbuf->mutex);

    log_debug("Destroying cond");
    pthread_cond_destroy(&circbuf->cond);

    free(circbuf);
}

greatbuf_item *greatbuf_item_init(size_t samples_size, size_t pcm_size, size_t data_size) {
    greatbuf_item *item;
    size_t i;

    item = NULL;

    log_debug("Greatbuf 2 item init");

    log_trace("Allocating greatbuf 2 item");
    item = (greatbuf_item *) malloc(sizeof(greatbuf_item));
    if (item == NULL) {
        log_error("Unable to allocate greatbuf 2 item");
        return NULL;
    }

    log_trace("Setting samples_size");

    item->samples_size = samples_size;
    item->pcm_size = pcm_size;
    item->data_size = data_size;

    log_trace("Allocating IQ buffer");
    item->iq = (uint8_t *) calloc(item->samples_size * 2, sizeof(uint8_t));
    if (item->iq == NULL) {
        log_error("Unable to allocate IQ buffer");
        greatbuf_item_free(item);
        return NULL;
    }

    log_trace("Allocating samples buffer");
    item->samples = (FP_FLOAT complex *) calloc(item->samples_size, sizeof(FP_FLOAT complex));
    if (item->samples == NULL) {
        log_error("Unable to allocate samples buffer");
        greatbuf_item_free(item);
        return NULL;
    }

    log_trace("Allocating demod buffer");
    item->demod = (FP_FLOAT *) calloc(item->samples_size, sizeof(FP_FLOAT));
    if (item->demod == NULL) {
        log_error("Unable to allocate demod buffer");
        greatbuf_item_free(item);
        return NULL;
    }

    log_trace("Allocating filtered buffer");
    item->filtered = (FP_FLOAT *) calloc(item->samples_size, sizeof(FP_FLOAT));
    if (item->filtered == NULL) {
        log_error("Unable to allocate filtered buffer");
        greatbuf_item_free(item);
        return NULL;
    }

    log_trace("Allocating pcm buffer");
    item->pcm = (int16_t *) calloc(item->pcm_size, sizeof(int16_t));
    if (item->pcm == NULL) {
        log_error("Unable to allocate pcm buffer");
        greatbuf_item_free(item);
        return NULL;
    }

    log_trace("Allocating data buffer");
    item->data = (uint8_t *) calloc(item->data_size, sizeof(uint8_t));
    if (item->data == NULL) {
        log_error("Unable to allocate pcm buffer");
        greatbuf_item_free(item);
        return NULL;
    }

    log_trace("Setting initial values");

    item->number = 0;

    item->ts.tv_sec = 0;
    item->ts.tv_nsec = 0;

    item->delay.tv_sec = 0;
    item->delay.tv_nsec = 0;

    for (i = 0; i < item->samples_size; i++) {
        item->iq[i * 2] = 0;
        item->iq[i * 2 + 1] = 0;
        item->samples[i] = 0 + 0 * I;
        item->demod[i] = 0;
        item->filtered[i] = 0;
    }

    for (i = 0; i < item->pcm_size; i++) {
        item->pcm[i] = 0;
    }

    item->contains_data = 0;

    item->rms = 0;

    return item;
}

void greatbuf_item_free(greatbuf_item *item) {
    log_debug("Freeing circbuff item");

    if (item == NULL)
        return;

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
    if (item->data != NULL)
        free(item->data);

    free(item);
}

greatbuf_ctx *greatbuf_init(size_t size, size_t samples_size, size_t pcm_size, size_t data_size) {
    greatbuf_ctx *ctx;
    size_t i;

    ctx = NULL;

    log_info("Greatbuf init");

    log_debug("Allocating greatbuf ctx");
    ctx = (greatbuf_ctx *) malloc(sizeof(greatbuf_ctx));
    if (ctx == NULL) {
        log_error("Unable to allocate greatbuf 2 item");
        return NULL;
    }

    log_debug("Setting samples_size");
    ctx->size = size;

    log_debug("Allocating items buffer");
    ctx->items = (greatbuf_item **) calloc(ctx->size, sizeof(greatbuf_item *));
    if (ctx->items == NULL) {
        log_error("Unable to allocate items");
        greatbuf_free(ctx);
        return NULL;
    }

    log_debug("Initializing items");
    for (i = 0; i < ctx->size; i++) {
        ctx->items[i] = greatbuf_item_init(samples_size, pcm_size, data_size);
        if (ctx->items[i] == NULL) {
            log_error("Unable to allocate item");
            greatbuf_free(ctx);
            return NULL;
        }
    }

    log_debug("Allocating circbuf read");
    ctx->circbuf_iq = greatbuf_circbuf_init("read", ctx->size);
    if (ctx->circbuf_iq == NULL) {
        log_error("Unable to allocate circbuf read");
        greatbuf_free(ctx);
        return NULL;
    }

    log_debug("Allocating circbuf samples");
    ctx->circbuf_samples = greatbuf_circbuf_init("samples", ctx->size);
    if (ctx->circbuf_samples == NULL) {
        log_error("Unable to allocate circbuf samples");
        greatbuf_free(ctx);
        return NULL;
    }

    log_debug("Allocating circbuf demod");
    ctx->circbuf_demod = greatbuf_circbuf_init("demod", ctx->size);
    if (ctx->circbuf_demod == NULL) {
        log_error("Unable to allocate circbuf demod");
        greatbuf_free(ctx);
        return NULL;
    }

    log_debug("Allocating circbuf lpf");
    ctx->circbuf_filtered = greatbuf_circbuf_init("lpf", ctx->size);
    if (ctx->circbuf_filtered == NULL) {
        log_error("Unable to allocate circbuf lpf");
        greatbuf_free(ctx);
        return NULL;
    }

    log_debug("Allocating circbuf resample");
    ctx->circbuf_pcm = greatbuf_circbuf_init("resample", ctx->size);
    if (ctx->circbuf_pcm == NULL) {
        log_error("Unable to allocate circbuf resample");
        greatbuf_free(ctx);
        return NULL;
    }

    log_debug("Allocating circbuf codec");
    ctx->circbuf_codec = greatbuf_circbuf_init("codec", ctx->size);
    if (ctx->circbuf_codec == NULL) {
        log_error("Unable to allocate circbuf codec");
        greatbuf_free(ctx);
        return NULL;
    }

    log_debug("Allocating circbuf monitor");
    ctx->circbuf_monitor = greatbuf_circbuf_init("monitor", ctx->size);
    if (ctx->circbuf_monitor == NULL) {
        log_error("Unable to allocate circbuf monitor");
        greatbuf_free(ctx);
        return NULL;
    }

    log_debug("Allocating circbuf network");
    ctx->circbuf_network = greatbuf_circbuf_init("network", ctx->size);
    if (ctx->circbuf_network == NULL) {
        log_error("Unable to allocate circbuf network");
        greatbuf_free(ctx);
        return NULL;
    }

    return ctx;
}

void greatbuf_free(greatbuf_ctx *ctx) {
    size_t i;

    log_info("Greatbuf free");

    log_debug("Freeing items");
    for (i = 0; i < ctx->size; i++)
        if (ctx->items[i] != NULL)
            greatbuf_item_free(ctx->items[i]);

    log_debug("Freeing items buffer");
    free(ctx->items);

    log_debug("Freeing circbufs");
    greatbuf_circbuf_free(ctx->circbuf_iq);
    greatbuf_circbuf_free(ctx->circbuf_samples);
    greatbuf_circbuf_free(ctx->circbuf_demod);
    greatbuf_circbuf_free(ctx->circbuf_filtered);
    greatbuf_circbuf_free(ctx->circbuf_pcm);
    greatbuf_circbuf_free(ctx->circbuf_codec);
    greatbuf_circbuf_free(ctx->circbuf_monitor);
    greatbuf_circbuf_free(ctx->circbuf_network);

    log_debug("Freeing context");
    free(ctx);
}

void greatbuf_stop(greatbuf_ctx *ctx) {
    log_info("Stopping greatbuf");

    ctx->circbuf_iq->keep_running = 0;
    ctx->circbuf_samples->keep_running = 0;
    ctx->circbuf_demod->keep_running = 0;
    ctx->circbuf_filtered->keep_running = 0;
    ctx->circbuf_pcm->keep_running = 0;
    ctx->circbuf_codec->keep_running = 0;
    ctx->circbuf_monitor->keep_running = 0;
    ctx->circbuf_network->keep_running = 0;

    log_debug("Signaling condition");
    pthread_cond_signal(&ctx->circbuf_iq->cond);
    pthread_cond_signal(&ctx->circbuf_samples->cond);
    pthread_cond_signal(&ctx->circbuf_demod->cond);
    pthread_cond_signal(&ctx->circbuf_filtered->cond);
    pthread_cond_signal(&ctx->circbuf_pcm->cond);
    pthread_cond_signal(&ctx->circbuf_codec->cond);
    pthread_cond_signal(&ctx->circbuf_monitor->cond);
    pthread_cond_signal(&ctx->circbuf_network->cond);
}

greatbuf_circbuf *greatbuf_circbuf_get(greatbuf_ctx *ctx, int circbuf) {
    switch (circbuf) {
        case GREATBUF_CIRCBUF_IQ:
            return ctx->circbuf_iq;
        case GREATBUF_CIRCBUF_SAMPLES:
            return ctx->circbuf_samples;
        case GREATBUF_CIRCBUF_DEMOD:
            return ctx->circbuf_demod;
        case GREATBUF_CIRCBUF_FILTERED:
            return ctx->circbuf_filtered;
        case GREATBUF_CIRCBUF_PCM:
            return ctx->circbuf_pcm;
        case GREATBUF_CIRCBUF_CODEC:
            return ctx->circbuf_codec;
        case GREATBUF_CIRCBUF_MONITOR:
            return ctx->circbuf_monitor;
        case GREATBUF_CIRCBUF_NETWORK:
            return ctx->circbuf_network;
        default:
            return NULL;
    }
}

#ifndef __RTLSDR__TESTS

void greatbuf_circbuf_status(greatbuf_ctx *ctx, int circbuf_num) {
    greatbuf_circbuf *circbuf;
    int dimension;

    circbuf = greatbuf_circbuf_get(ctx, circbuf_num);

    dimension = 100 - (int) ((FP_FLOAT) (circbuf->free) * 100 / (FP_FLOAT) circbuf->size);
    ui_message("Circular buffer %s: %d% (%zu/%zu)\n", circbuf->name, dimension, circbuf->free, ctx->size);
}

#endif

greatbuf_item *greatbuf_item_get(greatbuf_ctx *ctx, size_t pos) {
    log_debug("Circular buffer item get");

    return ctx->items[pos];
}

ssize_t greatbuf_head_acquire(greatbuf_ctx *ctx, int circbuf_num) {
    greatbuf_circbuf *circbuf;
    ssize_t pos;

    log_debug("Circular buffer head acquire");

    log_debug("Selecting circbuf");
    circbuf = greatbuf_circbuf_get(ctx, circbuf_num);

    pos = greatbuf_circbuf_head_acquire(circbuf);

    return pos;
}

void greatbuf_head_release(greatbuf_ctx *ctx, int circbuf_num) {
    greatbuf_circbuf *circbuf;

    log_debug("Circular buffer head release");

    log_debug("Selecting circbuf");
    circbuf = greatbuf_circbuf_get(ctx, circbuf_num);

    greatbuf_circbuf_head_release(circbuf);
}

ssize_t greatbuf_tail_acquire(greatbuf_ctx *ctx, int circbuf_num) {
    greatbuf_circbuf *circbuf;
    ssize_t pos;

    log_debug("Circular buffer tail acquire");

    log_trace("Selecting circbuf");
    circbuf = greatbuf_circbuf_get(ctx, circbuf_num);

    pos = greatbuf_circbuf_tail_acquire(circbuf);

    return pos;
}

void greatbuf_tail_release(greatbuf_ctx *ctx, int circbuf_num) {
    greatbuf_circbuf *circbuf;

    log_debug("Circular buffer tail release");

    log_trace("Selecting circbuf");
    circbuf = greatbuf_circbuf_get(ctx, circbuf_num);

    greatbuf_circbuf_tail_release(circbuf);
}

ssize_t greatbuf_circbuf_head_acquire(greatbuf_circbuf *circbuf) {
    ssize_t pos;

    pos = -1;

    log_trace("Acquiring lock");
    pthread_mutex_lock(&circbuf->mutex);

    if (circbuf->busy_head == 0) {
        if (circbuf->free > 0) {
            circbuf->busy_head = 1;
            pos = (ssize_t) circbuf->head;
        } else {
            log_warn("No free space in %s buffer", circbuf->name);
        }
    } else {
        log_warn("Head busy in %s buffer", circbuf->name);
    }

    pthread_mutex_unlock(&circbuf->mutex);
    log_trace("Releasing lock");

    return pos;
}

void greatbuf_circbuf_head_release(greatbuf_circbuf *circbuf) {
    log_trace("Acquiring lock");
    pthread_mutex_lock(&circbuf->mutex);

    if (circbuf->busy_head != 0) {
        circbuf->busy_head = 0;

        circbuf->head++;
        if (circbuf->head >= circbuf->size)
            circbuf->head = 0;

        circbuf->free--;
        if (circbuf->free == 0) {
            log_warn("Buffer %s full", circbuf->name);
        }
    }

    pthread_mutex_unlock(&circbuf->mutex);
    log_trace("Releasing lock");

    log_debug("Signaling condition");
    pthread_cond_signal(&circbuf->cond);
}

ssize_t greatbuf_circbuf_tail_acquire(greatbuf_circbuf *circbuf) {
    ssize_t pos;

    pos = -1;

    log_trace("Acquiring lock");
    pthread_mutex_lock(&circbuf->mutex);

    if (circbuf->busy_tail == 0 && circbuf->free > 0) {
        while (circbuf->keep_running == 1 && circbuf->free == circbuf->size) {
            log_trace("Data not available yet, waiting");
            pthread_cond_wait(&circbuf->cond, &circbuf->mutex);
        }

        if (circbuf->keep_running == 1) {
            circbuf->busy_tail = 1;
            pos = (ssize_t) circbuf->tail;
        }
    } else {
        log_warn("Head busy");
    }

    pthread_mutex_unlock(&circbuf->mutex);
    log_trace("Releasing lock");

    if (circbuf->keep_running == 0)
        pos = -2;

    return pos;
}

void greatbuf_circbuf_tail_release(greatbuf_circbuf *circbuf) {
    log_trace("Acquiring lock");
    pthread_mutex_lock(&circbuf->mutex);

    if (circbuf->busy_tail != 0) {
        circbuf->busy_tail = 0;

        circbuf->tail++;
        if (circbuf->tail >= circbuf->size)
            circbuf->tail = 0;

        circbuf->free++;
    }

    pthread_mutex_unlock(&circbuf->mutex);
    log_trace("Releasing lock");
}
