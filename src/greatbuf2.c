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

#include "greatbuf2.h"
#include "log.h"
#include "ui.h"

greatbuf2_circbuf *greatbuf2_circbuf_init(const char *name, size_t size) {
    greatbuf2_circbuf *circbuf;
    size_t ln;
    int result;

    circbuf = NULL;

    log_info("Greatbuf circbuf init");

    log_debug("Allocating circbuf");
    circbuf = (greatbuf2_circbuf *) malloc(sizeof(greatbuf2_circbuf));
    if (circbuf == NULL) {
        log_error("Unable to allocate circbuf");
        return NULL;
    }

    log_debug("Setting name");
    ln = strlen(name);
    circbuf->name = (char *) calloc(ln + 1, sizeof(char));
    if (circbuf->name == NULL) {
        log_error("Unable to allocate circbuf name");
        greatbuf2_circbuf_free(circbuf);
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
        greatbuf2_circbuf_free(circbuf);
        return NULL;
    }

    log_debug("Initializing condition");
    result = pthread_cond_init(&circbuf->cond, NULL);
    if (result != 0) {
        log_error("Error initializing condition: %d", result);
        greatbuf2_circbuf_free(circbuf);
        return NULL;
    }

    return circbuf;
}

void greatbuf2_circbuf_free(greatbuf2_circbuf *circbuf) {
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

greatbuf2_item *greatbuf2_item_init(size_t samples_size, size_t pcm_size) {
    greatbuf2_item *item;
    size_t i;

    item = NULL;

    log_info("Greatbuf 2 item init");

    log_debug("Allocating greatbuf 2 item");
    item = (greatbuf2_item *) malloc(sizeof(greatbuf2_item));
    if (item == NULL) {
        log_error("Unable to allocate greatbuf 2 item");
        return NULL;
    }

    log_debug("Setting samples_size");

    item->samples_size = samples_size;
    item->pcm_size = pcm_size;

    log_debug("Allocating IQ buffer");
    item->iq = (uint8_t *) calloc(item->samples_size * 2, sizeof(uint8_t));
    if (item->iq == NULL) {
        log_error("Unable to allocate IQ buffer");
        greatbuf2_item_free(item);
        return NULL;
    }

    log_debug("Allocating samples buffer");
    item->samples = (FP_FLOAT complex *) calloc(item->samples_size, sizeof(FP_FLOAT complex));
    if (item->samples == NULL) {
        log_error("Unable to allocate samples buffer");
        greatbuf2_item_free(item);
        return NULL;
    }

    log_debug("Allocating demod buffer");
    item->demod = (FP_FLOAT *) calloc(item->samples_size, sizeof(FP_FLOAT));
    if (item->demod == NULL) {
        log_error("Unable to allocate demod buffer");
        greatbuf2_item_free(item);
        return NULL;
    }

    log_debug("Allocating filtered buffer");
    item->filtered = (FP_FLOAT *) calloc(item->samples_size, sizeof(FP_FLOAT));
    if (item->filtered == NULL) {
        log_error("Unable to allocate filtered buffer");
        greatbuf2_item_free(item);
        return NULL;
    }

    log_debug("Allocating pcm buffer");
    item->pcm = (int16_t *) calloc(item->pcm_size, sizeof(int16_t));
    if (item->pcm == NULL) {
        log_error("Unable to allocate pcm buffer");
        greatbuf2_item_free(item);
        return NULL;
    }

    log_debug("Setting initial values");

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

    item->rms = 0;

    return item;
}

void greatbuf2_item_free(greatbuf2_item *item) {
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

    free(item);
}

greatbuf2_ctx *greatbuf2_init(size_t size, size_t samples_size, size_t pcm_size) {
    greatbuf2_ctx *ctx;
    size_t i;

    ctx = NULL;

    log_info("Greatbuf init");

    log_debug("Allocating greatbuf ctx");
    ctx = (greatbuf2_ctx *) malloc(sizeof(greatbuf2_ctx));
    if (ctx == NULL) {
        log_error("Unable to allocate greatbuf 2 item");
        return NULL;
    }

    log_debug("Setting samples_size");
    ctx->size = size;

    log_debug("Allocating items buffer");
    ctx->items = (greatbuf2_item **) calloc(ctx->size, sizeof(greatbuf2_item *));
    if (ctx->items == NULL) {
        log_error("Unable to allocate items");
        greatbuf2_free(ctx);
        return NULL;
    }

    log_debug("Initializing items");
    for (i = 0; i < ctx->size; i++) {
        ctx->items[i] = greatbuf2_item_init(samples_size, pcm_size);
        if (ctx->items[i] == NULL) {
            log_error("Unable to allocate item");
            greatbuf2_free(ctx);
            return NULL;
        }
    }

    log_debug("Allocating circbuf read");
    ctx->circbuf_read = greatbuf2_circbuf_init("read", ctx->size);
    if (ctx->circbuf_read == NULL) {
        log_error("Unable to allocate circbuf read");
        greatbuf2_free(ctx);
        return NULL;
    }

    log_debug("Allocating circbuf samples");
    ctx->circbuf_samples = greatbuf2_circbuf_init("samples", ctx->size);
    if (ctx->circbuf_samples == NULL) {
        log_error("Unable to allocate circbuf samples");
        greatbuf2_free(ctx);
        return NULL;
    }

    log_debug("Allocating circbuf demod");
    ctx->circbuf_demod = greatbuf2_circbuf_init("demod", ctx->size);
    if (ctx->circbuf_demod == NULL) {
        log_error("Unable to allocate circbuf demod");
        greatbuf2_free(ctx);
        return NULL;
    }

    log_debug("Allocating circbuf lpf");
    ctx->circbuf_lpf = greatbuf2_circbuf_init("lpf", ctx->size);
    if (ctx->circbuf_lpf == NULL) {
        log_error("Unable to allocate circbuf lpf");
        greatbuf2_free(ctx);
        return NULL;
    }

    log_debug("Allocating circbuf resample");
    ctx->circbuf_resample = greatbuf2_circbuf_init("resample", ctx->size);
    if (ctx->circbuf_resample == NULL) {
        log_error("Unable to allocate circbuf resample");
        greatbuf2_free(ctx);
        return NULL;
    }

    log_debug("Allocating circbuf codec");
    ctx->circbuf_codec = greatbuf2_circbuf_init("codec", ctx->size);
    if (ctx->circbuf_codec == NULL) {
        log_error("Unable to allocate circbuf codec");
        greatbuf2_free(ctx);
        return NULL;
    }

    log_debug("Allocating circbuf monitor");
    ctx->circbuf_monitor = greatbuf2_circbuf_init("monitor", ctx->size);
    if (ctx->circbuf_monitor == NULL) {
        log_error("Unable to allocate circbuf monitor");
        greatbuf2_free(ctx);
        return NULL;
    }

    log_debug("Allocating circbuf network");
    ctx->circbuf_network = greatbuf2_circbuf_init("network", ctx->size);
    if (ctx->circbuf_network == NULL) {
        log_error("Unable to allocate circbuf network");
        greatbuf2_free(ctx);
        return NULL;
    }

    return ctx;
}

void greatbuf2_free(greatbuf2_ctx *ctx) {
    size_t i;

    log_info("Greatbuf free");

    log_debug("Freeing items");
    for (i = 0; i < ctx->size; i++)
        if (ctx->items[i] != NULL)
            greatbuf2_item_free(ctx->items[i]);

    log_debug("Freeing items buffer");
    free(ctx->items);

    log_debug("Freeing circbufs");
    greatbuf2_circbuf_free(ctx->circbuf_read);
    greatbuf2_circbuf_free(ctx->circbuf_samples);
    greatbuf2_circbuf_free(ctx->circbuf_demod);
    greatbuf2_circbuf_free(ctx->circbuf_lpf);
    greatbuf2_circbuf_free(ctx->circbuf_resample);
    greatbuf2_circbuf_free(ctx->circbuf_codec);
    greatbuf2_circbuf_free(ctx->circbuf_monitor);
    greatbuf2_circbuf_free(ctx->circbuf_network);

    log_debug("Freeing context");
    free(ctx);
}

void greatbuf2_stop(greatbuf2_ctx *ctx) {
    log_info("Stopping greatbuf");

    ctx->circbuf_read->keep_running = 0;
    ctx->circbuf_samples->keep_running = 0;
    ctx->circbuf_demod->keep_running = 0;
    ctx->circbuf_lpf->keep_running = 0;
    ctx->circbuf_resample->keep_running = 0;
    ctx->circbuf_codec->keep_running = 0;
    ctx->circbuf_monitor->keep_running = 0;
    ctx->circbuf_network->keep_running = 0;
}

greatbuf2_circbuf *greatbuf2_circbuf_get(greatbuf2_ctx *ctx, int circbuf) {
    switch (circbuf) {
        case GREATBUF2_CIRCBUF_READ:
            return ctx->circbuf_read;
        case GREATBUF2_CIRCBUF_SAMPLES:
            return ctx->circbuf_samples;
        case GREATBUF2_CIRCBUF_DEMOD:
            return ctx->circbuf_demod;
        case GREATBUF2_CIRCBUF_FILTER:
            return ctx->circbuf_lpf;
        case GREATBUF2_CIRCBUF_RESAMPLE:
            return ctx->circbuf_resample;
        case GREATBUF2_CIRCBUF_CODEC:
            return ctx->circbuf_codec;
        case GREATBUF2_CIRCBUF_MONITOR:
            return ctx->circbuf_monitor;
        case GREATBUF2_CIRCBUF_NETWORK:
            return ctx->circbuf_network;
        default:
            return NULL;
    }
}

void greatbuf2_circbuf_status(greatbuf2_ctx *ctx, int circbuf_num) {
    greatbuf2_circbuf *circbuf;
    int dimension;

    circbuf = greatbuf2_circbuf_get(ctx, circbuf_num);

    dimension = 100 - (int) ((FP_FLOAT) (circbuf->free) * 100 / (FP_FLOAT) circbuf->size);
    ui_message("Circular buffer %s: %d% (%zu/%zu)\n", circbuf->name, dimension, circbuf->free, ctx->size);
}

greatbuf2_item *greatbuf2_item_get(greatbuf2_ctx *ctx, size_t pos) {
    log_debug("Circular buffer item get");

    return ctx->items[pos];
}

ssize_t greatbuf2_circbuf_head_acquire(greatbuf2_ctx *ctx, int circbuf_num) {
    greatbuf2_circbuf *circbuf;
    ssize_t pos;

    log_debug("Circular buffer head acquire");

    circbuf = NULL;
    pos = -1;

    log_debug("Selecting circbuf");
    circbuf = greatbuf2_circbuf_get(ctx, circbuf_num);

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

void greatbuf2_circbuf_head_release(greatbuf2_ctx *ctx, int circbuf_num) {
    greatbuf2_circbuf *circbuf;

    log_debug("Circular buffer head release");

    circbuf = NULL;

    log_debug("Selecting circbuf");
    circbuf = greatbuf2_circbuf_get(ctx, circbuf_num);

    log_trace("Acquiring lock");
    pthread_mutex_lock(&circbuf->mutex);

    if (circbuf->busy_head != 0) {
        circbuf->busy_head = 0;

        circbuf->head++;
        if (circbuf->head >= circbuf->size)
            circbuf->head = 0;

        circbuf->free--;
        if (circbuf->free == 0)
            log_warn("Buffer %s full", circbuf->name);
    }

    pthread_mutex_unlock(&circbuf->mutex);
    log_trace("Releasing lock");

    log_debug("Signaling condition");
    pthread_cond_signal(&circbuf->cond);
}

ssize_t greatbuf2_circbuf_tail_acquire(greatbuf2_ctx *ctx, int circbuf_num) {
    greatbuf2_circbuf *circbuf;
    ssize_t pos;

    log_debug("Circular buffer tail acquire");

    circbuf = NULL;
    pos = -1;

    log_debug("Selecting circbuf");
    circbuf = greatbuf2_circbuf_get(ctx, circbuf_num);

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

    return pos;
}

void greatbuf2_circbuf_tail_release(greatbuf2_ctx *ctx, int circbuf_num) {
    greatbuf2_circbuf *circbuf;

    log_debug("Circular buffer tail release");

    circbuf = NULL;

    log_debug("Selecting circbuf");
    circbuf = greatbuf2_circbuf_get(ctx, circbuf_num);

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
