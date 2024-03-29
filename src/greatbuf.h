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


#ifndef __RTLSDR_RADIO__GREATBUF__H
#define __RTLSDR_RADIO__GREATBUF__H

#include <stdint.h>
#include <stddef.h>
#include <complex.h>
#include <time.h>
#include <pthread.h>

#include "buildflags.h"

#define GREATBUF_CIRCBUF_IQ 0
#define GREATBUF_CIRCBUF_SAMPLES 1
#define GREATBUF_CIRCBUF_DEMOD 2
#define GREATBUF_CIRCBUF_FILTERED 3
#define GREATBUF_CIRCBUF_PCM 4
#define GREATBUF_CIRCBUF_CODEC 5
#define GREATBUF_CIRCBUF_MONITOR 6
#define GREATBUF_CIRCBUF_NETWORK 7

struct greatbuf_circbuf_t {
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

struct greatbuf_item_t {
    size_t samples_size;
    size_t pcm_size;
    size_t data_size;

    uint64_t number;

    struct timespec ts;
    struct timespec delay;

    uint8_t *iq;
    FP_FLOAT complex *samples;

    FP_FLOAT *demod;
    FP_FLOAT *filtered;

    int16_t *pcm;
    uint8_t *data;

    int contains_data;

    FP_FLOAT rms;
};

struct greatbuf_ctx_t {
    size_t size;
    struct greatbuf_item_t **items;

    struct greatbuf_circbuf_t *circbuf_iq;
    struct greatbuf_circbuf_t *circbuf_samples;
    struct greatbuf_circbuf_t *circbuf_demod;
    struct greatbuf_circbuf_t *circbuf_filtered;
    struct greatbuf_circbuf_t *circbuf_pcm;
    struct greatbuf_circbuf_t *circbuf_codec;
    struct greatbuf_circbuf_t *circbuf_monitor;
    struct greatbuf_circbuf_t *circbuf_network;
};

typedef struct greatbuf_circbuf_t greatbuf_circbuf;
typedef struct greatbuf_item_t greatbuf_item;
typedef struct greatbuf_ctx_t greatbuf_ctx;

greatbuf_circbuf *greatbuf_circbuf_init(const char *, size_t);

void greatbuf_circbuf_free(greatbuf_circbuf *);

greatbuf_item *greatbuf_item_init(size_t, size_t, size_t);

void greatbuf_item_free(greatbuf_item *);

greatbuf_ctx *greatbuf_init(size_t, size_t, size_t, size_t);

void greatbuf_free(greatbuf_ctx *);

void greatbuf_stop(greatbuf_ctx *);

greatbuf_circbuf *greatbuf_circbuf_get(greatbuf_ctx *, int);

#ifndef __RTLSDR__TESTS

void greatbuf_circbuf_status(greatbuf_ctx *, int);

#endif

greatbuf_item *greatbuf_item_get(greatbuf_ctx *, size_t);

ssize_t greatbuf_head_acquire(greatbuf_ctx *, int);

void greatbuf_head_release(greatbuf_ctx *, int);

ssize_t greatbuf_tail_acquire(greatbuf_ctx *, int);

void greatbuf_tail_release(greatbuf_ctx *, int);

ssize_t greatbuf_circbuf_head_acquire(greatbuf_circbuf *);

void greatbuf_circbuf_head_release(greatbuf_circbuf *);

ssize_t greatbuf_circbuf_tail_acquire(greatbuf_circbuf *);

void greatbuf_circbuf_tail_release(greatbuf_circbuf * );

#endif
