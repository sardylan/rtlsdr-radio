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


#ifndef __RTLSDR_RADIO__AUDIO__H
#define __RTLSDR_RADIO__AUDIO__H

#include <stddef.h>
#include <stdint.h>
#include <alsa/asoundlib.h>

struct audio_ctx_t {
    char *device_name;
    unsigned int rate;
    unsigned int channels;

    snd_pcm_t *pcm;
    snd_pcm_uframes_t frames_per_period;
};

typedef struct audio_ctx_t audio_ctx;

audio_ctx *audio_init(const char *, unsigned int, unsigned int);

void audio_free(audio_ctx *);

int audio_play(audio_ctx *, uint8_t *, size_t);

#endif
