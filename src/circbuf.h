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

#define CIRCBUF_INITIAL_SIZE 2097152

struct circbuf_ctx_t {
    void *pointer;

    size_t head;
    size_t tail;

    size_t size;
    size_t free;

    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

typedef struct circbuf_ctx_t circbuf_ctx;

int circbuf_init(circbuf_ctx *);

void circbuf_free(circbuf_ctx *);

int circbuf_put(circbuf_ctx *, void *, size_t);

int circbuf_get(circbuf_ctx *, void *,size_t);

size_t circbuf_size(circbuf_ctx *);

#endif
