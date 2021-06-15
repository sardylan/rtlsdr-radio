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


#ifndef __RTLSDR_RADIO__CFG__H
#define __RTLSDR_RADIO__CFG__H

#include <stddef.h>
#include <stdint.h>

enum bool_flag_t {
    FLAG_FALSE = 0,
    FLAG_TRUE = 1
};

enum source_type_t {
    SOURCE_RTLSDR = 'r',
    SOURCE_FILE = 'f'
};

enum work_mode_t {
    MODE_VERSION = 'v',
    MODE_HELP = 'h',
    MODE_RX = 'r',
    MODE_INFO = 'i'
};

enum modulation_type_t {
    MOD_TYPE_AM = 0,
    MOD_TYPE_FM = 1
};


enum filter_mode_t {
    FILTER_MODE_NONE = 0,
    FILTER_MODE_FIR_SW = 1,
    FILTER_MODE_FFT_SW = 2
};

typedef enum bool_flag_t bool_flag;
typedef enum source_type_t source_type;
typedef enum work_mode_t work_mode;
typedef enum modulation_type_t modulation_type;
typedef enum filter_mode_t filter_mode;

struct cfg_t {
    int ui_log_level;
    int file_log_level;
    char *file_log_name;

    bool_flag debug;

    source_type source;
    work_mode mode;

    char *rawiq_file_path;

    uint32_t rtlsdr_device_id;
    uint32_t rtlsdr_device_sample_rate;
    uint32_t rtlsdr_device_center_freq;
    int rtlsdr_device_freq_correction;
    bool_flag rtlsdr_device_tuner_gain_mode;
    int rtlsdr_device_tuner_gain;
    bool_flag rtlsdr_device_agc_mode;
    size_t rtlsdr_samples;

    modulation_type modulation;

    filter_mode filter;
    int filter_fir;

    uint64_t audio_frames_per_period;
    uint32_t audio_sample_rate;

    bool_flag audio_file_enabled;
    char *audio_file_path;

    bool_flag audio_monitor_enabled;
    char *audio_monitor_device;

    bool_flag audio_stdout;

    uint32_t codec_opus_bitrate;

    char *network_server;
    uint16_t network_port;
};

typedef struct cfg_t cfg;

void cfg_init();

void cfg_free();

void cfg_print();

int cfg_parse(int, char **);

int cfg_parse_file(char *);

bool_flag cfg_parse_flag(const char *);

int cfg_parse_source_type(source_type *, char *);

int cfg_parse_work_mode(work_mode *, char *);

int cfg_parse_modulation(modulation_type *, char *);

int cfg_parse_filter_mode(filter_mode *, char *);

const char *cfg_tochar_bool(bool_flag);

const char *cfg_tochar_log_level(int);

const char *cfg_tochar_source_type(source_type);

const char *cfg_tochar_work_mode(work_mode);

const char *cfg_tochar_modulation(modulation_type);

const char *cfg_tochar_filter_mode(filter_mode);

#endif
