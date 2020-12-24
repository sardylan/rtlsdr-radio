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


#ifndef __RTLSDR_RADIO__FRAME__H
#define __RTLSDR_RADIO__FRAME__H

#include <complex.h>
#include <time.h>
#include <stdint.h>
#include <stddef.h>

struct frame_t {
    struct timespec ts;

    size_t size_iq;
    uint8_t *iq;

    size_t size_sample;
    double complex *samples;
    int8_t *demod;
    int8_t *filtered;

    size_t size_pcm;
    int8_t *pcm;

    double rms;
};

typedef struct frame_t frame;

frame *frame_init(size_t, size_t, size_t);

void frame_clear(frame *);

void frame_free(frame *);

#endif
