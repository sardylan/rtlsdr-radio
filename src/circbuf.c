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


#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "circbuf.h"

int circbuf_init(circbuf_ctx *ctx) {
    ctx = (circbuf_ctx *) malloc(sizeof(circbuf_ctx));
    if (ctx == NULL)
        return EXIT_FAILURE;

    ctx->pointer = (void *) calloc(CIRCBUF_INITIAL_SIZE, sizeof(uint8_t));
    if (ctx->pointer == NULL) {
        free(ctx);
        return EXIT_FAILURE;
    }

    ctx->head = 0;
    ctx->tail = 0;

    ctx->size = CIRCBUF_INITIAL_SIZE;
    ctx->free = CIRCBUF_INITIAL_SIZE;

    ctx->lock = (pthread_mutex_t *) malloc(sizeof(pthread_mutex_t));

    return EXIT_SUCCESS;
}

void circbuf_free(circbuf_ctx *ctx) {
    free(ctx->pointer);
    free(ctx->lock);

    free(ctx);
}

int circbuf_put(circbuf_ctx *ctx, void *data, size_t len) {
    int ret;
    void *begin;
    size_t front_avail;
    size_t ln;

    ret = EXIT_SUCCESS;

    pthread_mutex_lock(ctx->lock);

    if (len <= ctx->free) {
        begin = ctx->pointer + ctx->head;
        front_avail = ctx->size - ctx->head;

        if (len < front_avail) {
            memcpy(begin, data, len);
            ctx->head += len;
        } else if (len == front_avail) {
            memcpy(begin, data, len);
            ctx->head = 0;
        } else {
            ln = len - front_avail;
            memcpy(begin, data, front_avail);
            memcpy(ctx->pointer, data + front_avail, ln);
            ctx->head = ln;
        }

        ctx->free -= len;
    } else {
        ret = EXIT_FAILURE;
    }

    pthread_mutex_unlock(ctx->lock);

    return ret;
}

int circbuf_get(circbuf_ctx *ctx, void *data, size_t len) {
    int ret;
    size_t used;
    void *begin;
    size_t front_avail;
    size_t ln;

    ret = EXIT_SUCCESS;

    pthread_mutex_lock(ctx->lock);

    used = ctx->size - ctx->free;

    if (len <= used) {
        begin = ctx->pointer + ctx->tail;
        front_avail = ctx->size - ctx->tail;

        if (len < front_avail) {
            memcpy(data, begin, len);
            ctx->tail += len;
        } else if (len == front_avail) {
            memcpy(data, begin, len);
            ctx->tail = 0;
        } else {
            ln = len - front_avail;
            memcpy(data, begin, front_avail);
            memcpy(data + front_avail, ctx->pointer, ln);
            ctx->tail = ln;
        }

        ctx->free += len;
    } else {
        ret = EXIT_FAILURE;
    }

    pthread_mutex_unlock(ctx->lock);

    return ret;
}

size_t circbuf_size(circbuf_ctx *ctx) {
    return ctx->size;
}