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


#ifndef __RTLSDR_RADIO__CIRCBUF__H
#define __RTLSDR_RADIO__CIRCBUF__H

#include <stddef.h>
#include <pthread.h>
#include <time.h>

struct circbuf_ctx_t {
    void *pointer;

    size_t item_size;

    size_t head;
    size_t tail;

    size_t size;
    size_t free;

    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

typedef struct circbuf_ctx_t circbuf_ctx;

circbuf_ctx * circbuf_init(size_t, size_t);

void circbuf_free(circbuf_ctx *);

int circbuf_put_data(circbuf_ctx *, void *, size_t);

int circbuf_get_data(circbuf_ctx *, void *, size_t);

int circbuf_put(circbuf_ctx *, struct timespec *, void *, size_t);

int circbuf_get(circbuf_ctx *, struct timespec *, void *, size_t);

#endif
