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


#include <malloc.h>
#include <string.h>

#include "frame.h"
#include "log.h"

frame *frame_init(size_t size_iq, size_t size_sample, size_t size_pcm) {
    frame *f;

    log_debug("Initializing frame");

    log_trace("Allocating frame");
    f = (frame *) malloc(sizeof(frame));
    if (f == NULL) {
        log_error("Unable to allocate frame");
        return NULL;
    }

    f->number = 0;

    log_trace("Setting IQ buffer_int16 samples_size");
    f->size_iq = size_iq;

    log_trace("Allocating IQ buffer_int16");
    f->iq = (uint8_t *) calloc(size_iq, sizeof(uint8_t));
    if (f->iq == NULL) {
        log_error("Unable to allocate IQ buffer_int16");
        frame_free(f);
        return NULL;
    }

    log_trace("Setting Samples buffer_int16 samples_size");
    f->size_sample = size_sample;

    log_trace("Allocating Samples buffer_int16");
    f->samples = (FP_FLOAT complex *) calloc(size_sample, sizeof(FP_FLOAT complex));
    if (f->samples == NULL) {
        log_error("Unable to allocate Samples buffer_int16");
        frame_free(f);
        return NULL;
    }

    log_trace("Allocating Demod buffer_int16");
    f->demod = (int8_t *) calloc(size_sample, sizeof(int8_t));
    if (f->demod == NULL) {
        log_error("Unable to allocate Demod buffer_int16");
        frame_free(f);
        return NULL;
    }

    log_trace("Allocating Filtered buffer_int16");
    f->filtered = (int8_t *) calloc(size_sample, sizeof(int8_t));
    if (f->filtered == NULL) {
        log_error("Unable to allocate Filtered buffer_int16");
        frame_free(f);
        return NULL;
    }

    log_trace("Setting PCM buffer_int16 samples_size");
    f->size_pcm = size_pcm;

    log_trace("Allocating PCM buffer_int16");
    f->pcm = (int8_t *) calloc(size_pcm, sizeof(int8_t));
    if (f->pcm == NULL) {
        log_error("Unable to allocate PCM buffer_int16");
        frame_free(f);
        return NULL;
    }

    frame_clear(f, 0);

    return f;
}

void frame_clear(frame *f, uint64_t number) {
    size_t i;

    log_debug("Cleaning frame");

    log_trace("Clearing number");
    f->number = number;

    log_trace("Setting timestamp");
    timespec_get(&f->ts, TIME_UTC);

    log_trace("Zeroing IQ buffer_int16");
    for (i = 0; i < f->size_iq; i++)
        f->iq[i] = 0;

    log_trace("Zeroing Samples, Demod and Filtered buffers");
    for (i = 0; i < f->size_sample; i++) {
        f->samples[i] = 0 + 0 * I;
        f->demod[i] = 0;
        f->filtered[i] = 0;
    }

    log_trace("Zeroing PCM buffer_int16");
    for (i = 0; i < f->size_pcm; i++)
        f->pcm[i] = 0;

    log_trace("Zeroing RMS");
    f->rms = 0;
}

void frame_free(frame *f) {
    log_trace("Freeing frame");

    log_trace("Freeing RMS");
    if (f->pcm != NULL)
        free(f->pcm);

    log_trace("Freeing Filtered buffer_int16");
    if (f->filtered != NULL)
        free(f->filtered);

    log_trace("Freeing Demod buffer_int16");
    if (f->demod != NULL)
        free(f->demod);

    log_trace("Freeing Samples buffer_int16");
    if (f->samples != NULL)
        free(f->samples);

    log_trace("Freeing IQ buffers");
    if (f->iq != NULL)
        free(f->iq);

    log_trace("Freeing frame");
    free(f);
}
