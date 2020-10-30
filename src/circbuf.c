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
#include "log.h"

int circbuf_init(circbuf_ctx *ctx) {
    int result;

    log_info("circbuf", "Circular buffer init");

    log_debug("circbuf", "Allocating %zu bytes for data", CIRCBUF_INITIAL_SIZE);
    ctx->pointer = (void *) calloc(CIRCBUF_INITIAL_SIZE, sizeof(uint8_t));
    if (ctx->pointer == NULL) {
        free(ctx);
        return EXIT_FAILURE;
    }

    log_debug("circbuf", "Setting internal references and counters");
    ctx->head = 0;
    ctx->tail = 0;

    ctx->size = CIRCBUF_INITIAL_SIZE;
    ctx->free = CIRCBUF_INITIAL_SIZE;

    log_debug("circbuf", "Initializing mutex");
    result = pthread_mutex_init(&ctx->mutex, NULL);
    if (result != 0) {
        log_error("circbuf", "Error initializing mutex: %d", result);
        return EXIT_FAILURE;
    }

    log_debug("circbuf", "Initializing condition");
    result = pthread_cond_init(&ctx->cond, NULL);
    if (result != 0) {
        log_error("circbuf", "Error initializing condition: %d", result);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void circbuf_free(circbuf_ctx *ctx) {
    log_info("circbuf", "Circular buffer free");

    log_debug("circbuf", "Destroying mutex");
    pthread_mutex_destroy(&ctx->mutex);

    log_debug("circbuf", "Destroying cond");
    pthread_cond_destroy(&ctx->cond);

    log_debug("circbuf", "Freeing data pointer");
    free(ctx->pointer);

    log_debug("circbuf", "Freeing circbuf");
    free(ctx);
}

int circbuf_put(circbuf_ctx *ctx, void *data, size_t len) {
    int ret;
    void *begin;
    size_t front_avail;
    size_t ln;

    log_info("circbuf", "Putting %zu bytes", len);

    ret = EXIT_SUCCESS;

    log_debug("circbuf", "Locking mutex");
    pthread_mutex_lock(&ctx->mutex);

    log_debug("circbuf", "Adding data");
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

    log_debug("circbuf", "Unlocking mutex");
    pthread_mutex_unlock(&ctx->mutex);

    log_debug("circbuf", "Signaling condition");
    pthread_cond_signal(&ctx->cond);

    return ret;
}

int circbuf_get(circbuf_ctx *ctx, void *data, size_t len) {
    int ret;
    size_t used;
    void *begin;
    size_t front_avail;
    size_t ln;

    log_info("circbuf", "Getting %zu bytes", len);

    ret = EXIT_SUCCESS;

    log_debug("circbuf", "Locking mutex");
    pthread_mutex_lock(&ctx->mutex);

    while ((used = ctx->size - ctx->free) < len) {
        log_trace("circbuf", "Data not available yet, waiting");
        pthread_cond_wait(&ctx->cond, &ctx->mutex);
    }

    log_debug("circbuf", "Getting data");
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

    log_debug("circbuf", "Unlocking mutex");
    pthread_mutex_unlock(&ctx->mutex);

    return ret;
}

size_t circbuf_size(circbuf_ctx *ctx) {
    return ctx->size;
}