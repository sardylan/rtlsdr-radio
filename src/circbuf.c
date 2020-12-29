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
#include "ui.h"

circbuf_ctx *circbuf_init(const char *name, size_t item_size, size_t initial_size) {
    circbuf_ctx *ctx;
    int result;
    size_t alloc_size;
    size_t ln;

    log_info("Circular buffer init");

    log_debug("Allocating circbuf context");
    ctx = (circbuf_ctx *) malloc(sizeof(circbuf_ctx));
    if (ctx == NULL) {
        log_error("Unable to allocate circbuf context");
        return NULL;
    }

    log_debug("Setting circbuf name");
    ln = strlen(name);
    ctx->name = calloc(ln + 1, sizeof(char));
    if (ctx->name == NULL) {
        log_error("Error allocating name buffer");
        free(ctx);
        return NULL;
    }

    strcpy(ctx->name, name);

    log_debug("Allocating %zu bytes for data", initial_size);
    alloc_size = item_size * initial_size;
    ctx->pointer = (void *) calloc(alloc_size, sizeof(uint8_t));
    if (ctx->pointer == NULL) {
        log_error("Error allocating data buffer");
        free(ctx);
        return NULL;
    }

    log_debug("Setting size");
    ctx->item_size = item_size;

    log_debug("Initializing internal vars");
    ctx->head = 0;
    ctx->tail = 0;
    ctx->size = initial_size;
    ctx->free = initial_size;

    log_debug("Initializing mutex");
    result = pthread_mutex_init(&ctx->mutex, NULL);
    if (result != 0) {
        log_error("Error initializing mutex: %d", result);
        free(ctx);
        return NULL;
    }

    log_debug("Initializing condition");
    result = pthread_cond_init(&ctx->cond, NULL);
    if (result != 0) {
        log_error("Error initializing condition: %d", result);
        free(ctx);
        return NULL;
    }

    return ctx;
}

void circbuf_free(circbuf_ctx *ctx) {
    log_info("Circular buffer free");

    log_debug("Destroying mutex");
    pthread_mutex_destroy(&ctx->mutex);

    log_debug("Destroying cond");
    pthread_cond_destroy(&ctx->cond);

    log_debug("Freeing data pointer");
    if (ctx->pointer != NULL)
        free(ctx->pointer);

    log_debug("Freeing circbuf");
    free(ctx);
}

int circbuf_put_data(circbuf_ctx *ctx, void *data, size_t data_size) {
    uint8_t *begin;
    size_t len;
    size_t front_avail;
    size_t ln;

    len = data_size * ctx->item_size;
    log_debug("Putting %zu bytes", len);

    if (len > ctx->free)
        return EXIT_FAILURE;

    log_debug("Adding data");

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
        memcpy(ctx->pointer, (uint8_t *) data + front_avail, ln);
        ctx->head = ln;
    }

    ctx->free -= len;

    return EXIT_SUCCESS;
}

int circbuf_get_data(circbuf_ctx *ctx, void *data, size_t data_size) {
    size_t used;
    uint8_t *begin;
    size_t front_avail;
    size_t ln;
    size_t len;

    len = data_size * ctx->item_size;
    log_debug("Getting %zu bytes", len);

    while ((used = ctx->size - ctx->free) < len) {
        log_trace("Data not available yet, waiting");
        pthread_cond_wait(&ctx->cond, &ctx->mutex);
    }

    if (len > used)
        return EXIT_FAILURE;

    log_debug("Getting data");

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
        memcpy((uint8_t *) data + front_avail, ctx->pointer, ln);
        ctx->tail = ln;
    }

    ctx->free += len;

    return EXIT_SUCCESS;
}

int circbuf_put(circbuf_ctx *ctx, void *data, size_t data_size) {
    int result;

    log_debug("Locking mutex");
    pthread_mutex_lock(&ctx->mutex);

    result = circbuf_put_data(ctx, data, data_size);
    if (result != EXIT_SUCCESS)
        return EXIT_FAILURE;

    log_debug("Unlocking mutex");
    pthread_mutex_unlock(&ctx->mutex);

    log_debug("Signaling condition");
    pthread_cond_signal(&ctx->cond);

    return result;
}

int circbuf_get(circbuf_ctx *ctx, void *data, size_t data_size) {
    int result;

    log_debug("Locking mutex");
    pthread_mutex_lock(&ctx->mutex);

    result = circbuf_get_data(ctx, data, data_size);
    if (result != EXIT_SUCCESS)
        return EXIT_FAILURE;

    log_debug("Unlocking mutex");
    pthread_mutex_unlock(&ctx->mutex);

    return result;
}

void circbuf_status(circbuf_ctx *ctx) {
    int dimension;

    dimension = (int) ((FP_FLOAT) (ctx->free) * 100 / ctx->size);
    ui_message("%s: %d% (%zu/%zu)\n", ctx->name, dimension, ctx->free, ctx->size);
}
