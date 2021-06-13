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


#ifndef __RTLSDR_RADIO__GREATBUF2__H
#define __RTLSDR_RADIO__GREATBUF2__H

#include <stdint.h>
#include <stddef.h>
#include <complex.h>
#include <time.h>
#include <pthread.h>

#define GREATBUF2_CIRCBUF_READ 0
#define GREATBUF2_CIRCBUF_SAMPLES 1
#define GREATBUF2_CIRCBUF_DEMOD 2
#define GREATBUF2_CIRCBUF_FILTER 3
#define GREATBUF2_CIRCBUF_RESAMPLE 4
#define GREATBUF2_CIRCBUF_CODEC 5
#define GREATBUF2_CIRCBUF_MONITOR 6
#define GREATBUF2_CIRCBUF_NETWORK 7

struct greatbuf2_circbuf_t {
    char *name;

    size_t head;
    size_t tail;

    size_t size;
    size_t free;

    int busy_head;
    int busy_tail;

    pthread_mutex_t mutex;
    pthread_cond_t cond;

    volatile int keep_running;
};

struct greatbuf2_item_t {
    size_t samples_size;
    size_t pcm_size;

    uint64_t number;
    struct timespec ts;
    struct timespec delay;

    uint8_t *iq;
    FP_FLOAT complex *samples;

    FP_FLOAT *demod;
    FP_FLOAT *filtered;

    int16_t *pcm;

    FP_FLOAT rms;
};

struct greatbuf2_ctx_t {
    uint64_t number;

    size_t size;
    struct greatbuf2_item_t **items;

    struct greatbuf2_circbuf_t *circbuf_read;
    struct greatbuf2_circbuf_t *circbuf_samples;
    struct greatbuf2_circbuf_t *circbuf_demod;
    struct greatbuf2_circbuf_t *circbuf_lpf;
    struct greatbuf2_circbuf_t *circbuf_resample;
    struct greatbuf2_circbuf_t *circbuf_codec;
    struct greatbuf2_circbuf_t *circbuf_monitor;
    struct greatbuf2_circbuf_t *circbuf_network;
};

typedef struct greatbuf2_circbuf_t greatbuf2_circbuf;
typedef struct greatbuf2_item_t greatbuf2_item;
typedef struct greatbuf2_ctx_t greatbuf2_ctx;

greatbuf2_circbuf *greatbuf2_circbuf_init(const char *, size_t);

void greatbuf2_circbuf_free(greatbuf2_circbuf *);

greatbuf2_item *greatbuf2_item_init(size_t, size_t);

void greatbuf2_item_free(greatbuf2_item *);

greatbuf2_ctx *greatbuf2_init(size_t, size_t, size_t);

void greatbuf2_free(greatbuf2_ctx *);

void greatbuf2_stop(greatbuf2_ctx *);

greatbuf2_circbuf *greatbuf2_circbuf_get(greatbuf2_ctx *, int);

void greatbuf2_circbuf_status(greatbuf2_ctx *, int);

greatbuf2_item *greatbuf2_item_get(greatbuf2_ctx *, size_t);

ssize_t greatbuf2_circbuf_head_acquire(greatbuf2_ctx *, int);

void greatbuf2_circbuf_head_release(greatbuf2_ctx *, int);

ssize_t greatbuf2_circbuf_tail_acquire(greatbuf2_ctx *, int);

void greatbuf2_circbuf_tail_release(greatbuf2_ctx *, int);

#endif
