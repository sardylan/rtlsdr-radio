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


#include <malloc.h>
#include <string.h>

#include "frame.h"
#include "log.h"

frame *frame_init(size_t size_iq, size_t size_sample, size_t size_pcm) {
    frame *f;

    log_init("Initializing frame");

    log_debug("Allocating frame");
    f = (frame *) malloc(sizeof(frame));
    if (f == NULL) {
        log_error("Unable to allocate frame");
        return NULL;
    }

    f->number = 0;

    log_debug("Setting IQ buffer size");
    f->size_iq = size_iq;

    log_debug("Allocating IQ buffer");
    f->iq = (uint8_t *) calloc(size_iq, sizeof(uint8_t));
    if (f->iq == NULL) {
        log_error("Unable to allocate IQ buffer");
        frame_free(f);
        return NULL;
    }

    log_debug("Setting Samples buffer size");
    f->size_sample = size_sample;

    log_debug("Allocating Samples buffer");
    f->samples = (FP_FLOAT complex *) calloc(size_sample, sizeof(FP_FLOAT complex));
    if (f->samples == NULL) {
        log_error("Unable to allocate Samples buffer");
        frame_free(f);
        return NULL;
    }

    log_debug("Allocating Demod buffer");
    f->demod = (int8_t *) calloc(size_sample, sizeof(int8_t));
    if (f->demod == NULL) {
        log_error("Unable to allocate Demod buffer");
        frame_free(f);
        return NULL;
    }

    log_debug("Allocating Filtered buffer");
    f->filtered = (int8_t *) calloc(size_sample, sizeof(int8_t));
    if (f->filtered == NULL) {
        log_error("Unable to allocate Filtered buffer");
        frame_free(f);
        return NULL;
    }

    log_debug("Setting PCM buffer size");
    f->size_pcm = size_pcm;

    log_debug("Allocating PCM buffer");
    f->pcm = (int8_t *) calloc(size_pcm, sizeof(int8_t));
    if (f->pcm == NULL) {
        log_error("Unable to allocate PCM buffer");
        frame_free(f);
        return NULL;
    }

    frame_clear(f, 0);

    return f;
}

void frame_clear(frame *f, uint64_t number) {
    size_t i;

    log_info("Cleaning frame");

    log_debug("Clearing number");
    f->number = number;

    log_debug("Setting timestamp");
    timespec_get(&f->ts, TIME_UTC);

    log_debug("Zeroing IQ buffer");
    for (i = 0; i < f->size_iq; i++)
        f->iq[i] = 0;

    log_debug("Zeroing Samples, Demod and Filtered buffers");
    for (i = 0; i < f->size_sample; i++) {
        f->samples[i] = 0 + 0 * I;
        f->demod[i] = 0;
        f->filtered[i] = 0;
    }

    log_debug("Zeroing PCM buffer");
    for (i = 0; i < f->size_pcm; i++)
        f->pcm[i] = 0;

    log_debug("Zeroing RMS");
    f->rms = 0;
}

void frame_free(frame *f) {
    log_init("Freeing frame");

    log_debug("Freeing RMS");
    if (f->pcm != NULL)
        free(f->pcm);

    log_debug("Freeing Filtered buffer");
    if (f->filtered != NULL)
        free(f->filtered);

    log_debug("Freeing Demod buffer");
    if (f->demod != NULL)
        free(f->demod);

    log_debug("Freeing Samples buffer");
    if (f->samples != NULL)
        free(f->samples);

    log_debug("Freeing IQ buffers");
    if (f->iq != NULL)
        free(f->iq);

    log_debug("Freeing frame");
    free(f);
}
