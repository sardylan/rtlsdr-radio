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


#ifndef __RTLSDR_RADIO__PAYLOAD__H
#define __RTLSDR_RADIO__PAYLOAD__H

/*

# 0         1         2         3         4         5         6
# 0123456789012345678901234567890123456789012345678901234567890123456789
# GFPrrrrNNNNNNNNttttttttRccccFFFFdddd...

 */

#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <string.h>

#define PAYLOAD_HEADER "GFP"

struct payload_t {
    uint32_t receiver;
    uint64_t number;

    uint64_t timestamp;

    uint8_t rms;

    uint32_t channel;
    uint32_t frequency;

    uint32_t data_size;
    uint8_t *data;
};

typedef struct payload_t payload;

payload *payload_init();

void payload_free(payload *);

int payload_set_numbers(payload *, uint32_t, uint64_t);

int payload_set_timestamp(payload *, struct timespec *);

int payload_set_rms(payload *, FP_FLOAT);

int payload_set_channel_frequency(payload *, uint32_t, uint32_t);

int payload_set_data(payload *, uint8_t *, uint32_t);

size_t payload_get_size(payload *);

int payload_serialize(payload *, uint8_t *, size_t, size_t *);

#endif
