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

struct greatbuf_ctx_t {
    size_t iq_size;
    size_t samples_size;
    size_t demod_size;
    size_t filtered_size;
    size_t pcm_size;

    size_t size;
    struct greatbuf_item_t *buffer;

    uint64_t counter;

    size_t pos_iq_head;
    size_t pos_iq_tail;

    size_t pos_samples_head;
    size_t pos_samples_tail;

    size_t pos_demod_head;
    size_t pos_demod_tail;

    size_t pos_filtered_head;
    size_t pos_filtered_tail;

    size_t pos_pcm_head;
    size_t pos_pcm_tail;

    pthread_mutex_t mutex_iq;
    pthread_cond_t cond_iq;

    pthread_mutex_t mutex_samples;
    pthread_cond_t cond_samples;

    pthread_mutex_t mutex_demod;
    pthread_cond_t cond_demod;

    pthread_mutex_t mutex_filtered;
    pthread_cond_t cond_filtered;

    pthread_mutex_t mutex_pcm;
    pthread_cond_t cond_pcm;
};

struct greatbuf_item_t {
    uint64_t number;
    struct timespec ts;
    uint8_t *iq;
    FP_FLOAT complex *samples;
    FP_FLOAT *demod;
    FP_FLOAT *filtered;
    int16_t *pcm;
};

typedef struct greatbuf_ctx_t greatbuf_ctx;
typedef struct greatbuf_item_t greatbuf_item;

greatbuf_ctx *greatbuf_init(size_t, size_t, size_t);

void greatbuf_free(greatbuf_ctx *);

size_t greatbuf_iq_head_start(greatbuf_ctx *);

void greatbuf_iq_head_stop(greatbuf_ctx *);

size_t greatbuf_iq_tail(greatbuf_ctx *);

size_t greatbuf_samples_head_start(greatbuf_ctx *);

void greatbuf_samples_head_stop(greatbuf_ctx *);

size_t greatbuf_samples_tail(greatbuf_ctx *);

size_t greatbuf_demod_head_start(greatbuf_ctx *);

void greatbuf_demod_head_stop(greatbuf_ctx *);

size_t greatbuf_demod_tail(greatbuf_ctx *);

size_t greatbuf_filtered_head_start(greatbuf_ctx *);

void greatbuf_filtered_head_stop(greatbuf_ctx *);

size_t greatbuf_filtered_tail(greatbuf_ctx *);

size_t greatbuf_pcm_head_start(greatbuf_ctx *);

void greatbuf_pcm_head_stop(greatbuf_ctx *);

size_t greatbuf_pcm_tail(greatbuf_ctx *);

void greatbuf_item_init(greatbuf_ctx *, size_t);

uint64_t greatbuf_item_number_get(greatbuf_ctx *, size_t);

struct timespec *greatbuf_item_ts_get(greatbuf_ctx *, size_t);

uint8_t *greatbuf_item_iq_get(greatbuf_ctx *, size_t);

FP_FLOAT complex *greatbuf_item_samples_get(greatbuf_ctx *, size_t);

FP_FLOAT *greatbuf_item_demod_get(greatbuf_ctx *, size_t);

FP_FLOAT *greatbuf_item_filtered_get(greatbuf_ctx *, size_t);

int16_t *greatbuf_item_pcm_get(greatbuf_ctx *, size_t);

#endif
