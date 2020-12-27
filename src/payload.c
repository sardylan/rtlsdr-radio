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
#include <malloc.h>
#include <string.h>

#include "payload.h"
#include "log.h"
#include "utils.h"

payload *payload_init() {
    payload *p;

    log_info("Initializing payload");

    log_debug("Allocating payload");
    p = (payload *) malloc(sizeof(payload));
    if (p == NULL) {
        log_error("Unable to allocate payload");
        return NULL;
    }

    log_debug("Setting initial values");

    p->receiver = 0;
    p->number = 0;

    p->timestamp = 0;

    p->channel = 0;
    p->frequency = 0;

    p->data_size = 0;
    p->data = NULL;

    return p;
}

void payload_free(payload *p) {
    log_info("Freeing payload");

    log_debug("Freeing data buffer");
    if (p->data != NULL)
        free(p->data);

    log_debug("Freeing payload");
    free(p);
}

int payload_set_numbers(payload *p, uint32_t receiver, uint64_t number) {
    log_info("Setting numbers");

    p->receiver = receiver;
    p->number = number;

    return EXIT_SUCCESS;
}

int payload_set_timestamp(payload *p, struct timespec *ts) {
    uint64_t timestamp;

    log_info("Setting timestamp");

    timestamp = ts->tv_sec * 1000;
    timestamp += ts->tv_nsec / 1000000;
    p->timestamp = timestamp;

    return EXIT_SUCCESS;
}

int payload_set_channel_frequency(payload *p, uint32_t channel, uint32_t frequency) {
    log_info("Setting channel and frequency");

    p->channel = channel;
    p->frequency = frequency;

    return EXIT_SUCCESS;
}

int payload_set_data(payload *p, uint8_t *data, uint32_t data_size) {
    log_info("Setting data");

    log_debug("Allocating data");
    p->data = (uint8_t *) realloc(p->data, data_size * sizeof(uint8_t));
    if (p == NULL) {
        log_error("Unable to allocate payload");
        return EXIT_FAILURE;
    }

    log_debug("Copying data");
    memcpy(p->data, data, data_size);

    log_debug("Setting data size");
    p->data_size = data_size;

    return EXIT_SUCCESS;
}

size_t payload_get_size(payload *p) {
    size_t ln;

    log_info("Getting payload size");

    ln = 0;

    ln += sizeof(PAYLOAD_HEADER);

    ln += sizeof(uint64_t);
    ln += sizeof(uint64_t);

    ln += sizeof(uint64_t);

    ln += sizeof(uint32_t);
    ln += sizeof(uint32_t);

    ln += sizeof(uint32_t);
    ln += p->data_size;

    return ln;
}

int payload_serialize(payload *p, uint8_t *buffer, size_t buffer_size, size_t *bytes_written) {
    size_t ln;

    log_info("Serializing payload");

    ln = payload_get_size(p);
    if (buffer_size < ln) {
        log_error("Not enough space for payload serialization");
        return EXIT_FAILURE;
    }

    ln = 0;

    log_debug("Adds header");
    strcpy((char *) buffer + ln, PAYLOAD_HEADER);
    ln += strlen(PAYLOAD_HEADER);

    log_debug("Adds receiver number");
    utils_uint64_to_be(buffer + ln, p->receiver);
    ln += sizeof(uint64_t);

    log_debug("Adds payload number");
    utils_uint64_to_be(buffer + ln, p->number);
    ln += sizeof(uint64_t);

    log_debug("Adds timestamp");
    utils_uint64_to_be(buffer + ln, p->timestamp);
    ln += sizeof(uint64_t);

    log_debug("Adds channel");
    utils_uint32_to_be(buffer + ln, p->channel);
    ln += sizeof(uint32_t);

    log_debug("Adds frequency");
    utils_uint32_to_be(buffer + ln, p->frequency);
    ln += sizeof(uint32_t);

    log_debug("Adds data size");
    utils_uint32_to_be(buffer + ln, p->data_size);
    ln += sizeof(uint32_t);

    log_debug("Adds data");
    memcpy(buffer + ln, p->data, p->data_size);
    ln += p->data_size;

    log_debug("Sets bytes_written");
    *bytes_written = ln;

    return EXIT_SUCCESS;
}
