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


#include <string.h>
#include <malloc.h>

#include "circbuf2.h"
#include "log.h"
#include "ui.h"

circbuf2_ctx *circbuf2_init(const char *name, size_t item_size, size_t payload_size, size_t size) {
    circbuf2_ctx *ctx;
    int result;
    size_t ln;

    log_info("Circular buffer init");

    log_debug("Allocating circbuf context");
    ctx = (circbuf2_ctx *) malloc(sizeof(circbuf2_ctx));
    if (ctx == NULL) {
        log_error("Unable to allocate circbuf context");
        return NULL;
    }

    log_debug("Setting circbuf name");
    ln = strlen(name);
    ctx->name = calloc(ln + 1, sizeof(char));
    if (ctx->name == NULL) {
        log_error("Error allocating name buffer");
        circbuf2_free(ctx);
        return NULL;
    }

    strcpy(ctx->name, name);

    log_debug("Setting samples_size");
    ctx->size = size;
    ctx->payload_size = payload_size;
    ctx->item_size = item_size;

    ctx->shift = ctx->payload_size * ctx->item_size;

    log_debug("Allocating data: %zu payloads with %zu items of %zu bytes - Total %zu bytes",
              ctx->size, ctx->payload_size, ctx->item_size, ctx->size * ctx->shift);
    ctx->data = (void *) calloc(ctx->size, ctx->shift);
    if (ctx->data == NULL) {
        log_error("Error allocating data buffer");
        circbuf2_free(ctx);
        return NULL;
    }

    log_debug("Initializing internal vars");
    ctx->head = 0;
    ctx->tail = 0;

    ctx->free = size;

    ctx->busy_head = 0;
    ctx->busy_tail = 0;

    ctx->keep_running = 1;

    log_debug("Initializing mutex");
    result = pthread_mutex_init(&ctx->mutex, NULL);
    if (result != 0) {
        log_error("Error initializing mutex: %d", result);
        circbuf2_free(ctx);
        return NULL;
    }

    log_debug("Initializing condition");
    result = pthread_cond_init(&ctx->cond, NULL);
    if (result != 0) {
        log_error("Error initializing condition: %d", result);
        circbuf2_free(ctx);
        return NULL;
    }

    return ctx;
}

void circbuf2_free(circbuf2_ctx *ctx) {
    log_info("Circular buffer free");

    if (ctx == NULL)
        return;

    log_debug("Destroying mutex");
    pthread_mutex_destroy(&ctx->mutex);

    log_debug("Destroying cond");
    pthread_cond_destroy(&ctx->cond);

    log_debug("Freeing data pointer");
    if (ctx->data != NULL)
        free(ctx->data);

    log_debug("Freeing name");
    if (ctx->name != NULL)
        free(ctx->name);

    log_debug("Freeing circbuf");
    free(ctx);
}

void circbuf2_status(circbuf2_ctx *ctx) {
    int dimension;

    dimension = 100 - (int) ((FP_FLOAT) (ctx->free) * 100 / (FP_FLOAT) ctx->size);
    ui_message("Circular buffer %s: %d% (%zu/%zu)\n", ctx->name, dimension, ctx->free, ctx->size);
}

void circbuf2_stop(circbuf2_ctx *ctx) {
    log_info("Circular buffer stop");

    ctx->keep_running = 0;
}

void *circbuf2_head_acquire(circbuf2_ctx *ctx) {
    void *ptr;
    size_t pos;

    log_debug("Circular buffer head acquire");

    ptr = NULL;

    log_trace("Acquiring lock");
    pthread_mutex_lock(&ctx->mutex);

    if (ctx->busy_head == 0) {
        if (ctx->free > 0) {
            ctx->busy_head = 1;
            pos = ctx->head * ctx->shift;
            ptr = (uint8_t *) ctx->data + pos;
        } else {
            log_warn("No free space in %s buffer", ctx->name);
        }
    } else {
        log_warn("Head busy in %s buffer", ctx->name);
    }

    pthread_mutex_unlock(&ctx->mutex);
    log_trace("Releasing lock");

    return ptr;
}

void circbuf2_head_release(circbuf2_ctx *ctx) {
    log_debug("Circular buffer head release");

    log_trace("Acquiring lock");
    pthread_mutex_lock(&ctx->mutex);

    if (ctx->busy_head != 0) {
        ctx->busy_head = 0;

        ctx->head++;
        if (ctx->head >= ctx->size)
            ctx->head = 0;

        ctx->free--;
        if (ctx->free == 0)
            log_warn("Buffer %s full", ctx->name);
    }

    pthread_mutex_unlock(&ctx->mutex);
    log_trace("Releasing lock");

    log_debug("Signaling condition");
    pthread_cond_signal(&ctx->cond);
}

void *circbuf2_tail_acquire(circbuf2_ctx *ctx) {
    void *ptr;
    size_t pos;

    log_debug("Circular buffer tail acquire");

    ptr = NULL;

    log_trace("Acquiring lock");
    pthread_mutex_lock(&ctx->mutex);

    if (ctx->busy_tail == 0 && ctx->free > 0) {
        while (ctx->keep_running == 1 && ctx->free == ctx->size) {
            log_trace("Data not available yet, waiting");
            pthread_cond_wait(&ctx->cond, &ctx->mutex);
        }
        if (ctx->keep_running == 1) {
            ctx->busy_tail = 1;
            pos = ctx->tail * ctx->shift;
            ptr = (uint8_t *) ctx->data + pos;
        }
    } else {
        log_warn("Head busy");
    }

    pthread_mutex_unlock(&ctx->mutex);
    log_trace("Releasing lock");

    return ptr;
}

void circbuf2_tail_release(circbuf2_ctx *ctx) {
    log_debug("Circular buffer tail release");

    log_trace("Acquiring lock");
    pthread_mutex_lock(&ctx->mutex);

    if (ctx->busy_tail != 0) {
        ctx->busy_tail = 0;

        ctx->tail++;
        if (ctx->tail >= ctx->size)
            ctx->tail = 0;

        ctx->free++;
    }

    pthread_mutex_unlock(&ctx->mutex);
    log_trace("Releasing lock");
}
