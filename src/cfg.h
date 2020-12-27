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


#ifndef __RTLSDR_RADIO__CFG__H
#define __RTLSDR_RADIO__CFG__H

#include <stddef.h>
#include <stdint.h>

enum work_mode_t {
    MODE_VERSION = 'v',
    MODE_HELP = 'h',
    MODE_RX = 'r',
    MODE_INFO = 'i'
};

typedef enum work_mode_t work_mode;

enum modulation_type_t {
    MOD_TYPE_AM = 0,
    MOD_TYPE_FM = 1
};

typedef enum modulation_type_t modulation_type;

enum filter_mode_t {
    FILTER_MODE_NONE = 0,
    FILTER_MODE_FIR_SW = 1,
    FILTER_MODE_FFT_SW = 2
};

typedef enum filter_mode_t filter_mode;

struct cfg_t {
    int ui_log_level;
    int file_log_level;
    char *file_log_name;

    int debug;

    work_mode mode;

    uint32_t rtlsdr_device_id;
    uint32_t rtlsdr_device_sample_rate;
    uint32_t rtlsdr_device_center_freq;
    int rtlsdr_device_freq_correction;
    int rtlsdr_device_tuner_gain_mode;
    int rtlsdr_device_tuner_gain;
    int rtlsdr_device_agc_mode;
    size_t rtlsdr_samples;

    modulation_type modulation;

    filter_mode filter;

    int filter_fir;

    uint32_t audio_sample_rate;

    uint32_t codec_opus_bitrate;

    char *network_server;
    uint16_t network_port;
};

typedef struct cfg_t cfg;

void cfg_init();

void cfg_free();

void cfg_print();

int cfg_parse(int, char **);

int cfg_parse_flag(int);

const char *cfg_tochar_bool(int);

const char *cfg_tochar_log_level(int);

const char *cfg_tochar_work_mode(work_mode);

const char *cfg_tochar_modulation(modulation_type);

const char *cfg_tochar_filter_mode(filter_mode);

#endif
