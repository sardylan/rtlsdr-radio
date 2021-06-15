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


#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "cfg.h"
#include "default.h"
#include "ui.h"
#include "log.h"
#include "utils.h"

extern cfg *conf;

void cfg_init() {
    size_t ln;

    conf = (cfg *) malloc(sizeof(cfg));

    conf->ui_log_level = CONFIG_UI_LOG_LEVEL_DEFAULT;
    conf->file_log_level = CONFIG_FILE_LOG_LEVEL_DEFAULT;

    ln = strlen(CONFIG_FILE_LOG_NAME_DEFAULT) + 1;
    conf->file_log_name = (char *) calloc(sizeof(char), ln);
    strcpy(conf->file_log_name, CONFIG_FILE_LOG_NAME_DEFAULT);

    conf->source = CONFIG_SOURCE_DEFAULT;
    conf->mode = CONFIG_MODE_DEFAULT;
    conf->debug = CONFIG_DEBUG_DEFAULT;

    ln = strlen(CONFIG_FILE_RAWIQ_FILE_PATH_DEFAULT) + 1;
    conf->rawiq_file_path = (char *) calloc(sizeof(char), ln);
    strcpy(conf->rawiq_file_path, CONFIG_FILE_RAWIQ_FILE_PATH_DEFAULT);

    conf->rtlsdr_device_id = CONFIG_RTLSDR_DEVICE_ID_DEFAULT;
    conf->rtlsdr_device_sample_rate = CONFIG_RTLSDR_DEVICE_SAMPLE_RATE_DEFAULT;
    conf->rtlsdr_device_center_freq = CONFIG_RTLSDR_DEVICE_CENTER_FREQ_DEFAULT;
    conf->rtlsdr_device_freq_correction = CONFIG_RTLSDR_DEVICE_FREQ_CORRECTION_DEFAULT;
    conf->rtlsdr_device_tuner_gain_mode = CONFIG_RTLSDR_DEVICE_TUNER_GAIN_MODE_DEFAULT;
    conf->rtlsdr_device_tuner_gain = CONFIG_RTLSDR_DEVICE_TUNER_GAIN_DEFAULT;
    conf->rtlsdr_device_agc_mode = CONFIG_RTLSDR_DEVICE_AGC_MODEDEFAULT;
    conf->rtlsdr_samples = CONFIG_RTLSDR_SAMPLES_DEFAULT;

    conf->modulation = CONFIG_MODULATION_DEFAULT;

    conf->filter = CONFIG_FILTER_DEFAULT;
    conf->filter_fir = CONFIG_FILTER_FIR_DEFAULT;

    conf->audio_frames_per_period = CONFIG_AUDIO_FRAME_PER_PERIOD_DEFAULT;
    conf->audio_sample_rate = CONFIG_AUDIO_SAMPLE_RATE_DEFAULT;

    conf->audio_file_enabled = CONFIG_AUDIO_FILE_ENABLED_DEFAULT;

    ln = strlen(CONFIG_AUDIO_FILE_PATH_DEFAULT) + 1;
    conf->audio_file_path = (char *) calloc(sizeof(char), ln);
    strcpy(conf->audio_file_path, CONFIG_AUDIO_FILE_PATH_DEFAULT);

    conf->audio_monitor_enabled = CONFIG_AUDIO_MONITOR_ENABLED_DEFAULT;

    ln = strlen(CONFIG_AUDIO_MONITOR_DEVICE_DEFAULT) + 1;
    conf->audio_monitor_device = (char *) calloc(sizeof(char), ln);
    strcpy(conf->audio_monitor_device, CONFIG_AUDIO_MONITOR_DEVICE_DEFAULT);

    conf->audio_stdout = CONFIG_AUDIO_STDOUT_DEFAULT;

    conf->codec_opus_bitrate = CONFIG_CODEC_OPUS_BITRATE_DEFAULT;

    ln = strlen(CONFIG_NETWORK_SERVER_DEFAULT) + 1;
    conf->network_server = (char *) calloc(sizeof(char), ln);
    strcpy(conf->network_server, CONFIG_NETWORK_SERVER_DEFAULT);

    conf->network_port = CONFIG_NETWORK_PORT_DEFAULT;
}

void cfg_free() {
    free(conf->file_log_name);
    free(conf->rawiq_file_path);
    free(conf->audio_file_path);
    free(conf->audio_monitor_device);
    free(conf->network_server);

    free(conf);
}

void cfg_print() {
    ui_message("\n");
    ui_message("ui_log_level:                  %s\n", cfg_tochar_log_level(conf->ui_log_level));
    ui_message("file_log_level:                %s\n", cfg_tochar_log_level(conf->file_log_level));
    ui_message("file_log_name:                 %s\n", conf->file_log_name);
    ui_message("\n");
    ui_message("debug:                         %s\n", cfg_tochar_bool(conf->debug));
    ui_message("\n");
    ui_message("source:                        %s\n", cfg_tochar_source_type(conf->source));
    ui_message("mode:                          %s\n", cfg_tochar_work_mode(conf->mode));
    ui_message("\n");
    ui_message("rawiq_file_path:               %s\n", conf->rawiq_file_path);
    ui_message("\n");
    ui_message("rtlsdr_device_id:              %u\n", conf->rtlsdr_device_id);
    ui_message("rtlsdr_device_sample_rate:     %u (Hz)\n", conf->rtlsdr_device_sample_rate);
    ui_message("rtlsdr_device_center_freq:     %u (Hz)\n", conf->rtlsdr_device_center_freq);
    ui_message("rtlsdr_device_freq_correction: %u (ppm)\n", conf->rtlsdr_device_freq_correction);
    ui_message("rtlsdr_device_tuner_gain_mode: %s\n", cfg_tochar_bool(conf->rtlsdr_device_tuner_gain_mode));
    ui_message("rtlsdr_device_tuner_gain:      %u (10e-1 dB)\n", conf->rtlsdr_device_tuner_gain);
    ui_message("rtlsdr_device_agc_mode:        %s\n", cfg_tochar_bool(conf->rtlsdr_device_agc_mode));
    ui_message("rtlsdr_samples:                %zu\n", conf->rtlsdr_samples);
    ui_message("\n");
    ui_message("modulation:                    %s\n", cfg_tochar_modulation(conf->modulation));
    ui_message("\n");
    ui_message("filter:                        %s\n", cfg_tochar_filter_mode(conf->filter));
    ui_message("filter_fir:                    %d\n", conf->filter_fir);
    ui_message("\n");
    ui_message("audio_frames_per_period:       %u\n", conf->audio_frames_per_period);
    ui_message("audio_sample_rate:             %u (Hz)\n", conf->audio_sample_rate);
    ui_message("\n");
    ui_message("audio_file_enabled:            %s\n", cfg_tochar_bool(conf->audio_file_enabled));
    ui_message("audio_file_path:               %s\n", conf->audio_file_path);
    ui_message("\n");
    ui_message("audio_monitor_enabled:         %s\n", cfg_tochar_bool(conf->audio_monitor_enabled));
    ui_message("audio_monitor_device:          %s\n", conf->audio_monitor_device);
    ui_message("\n");
    ui_message("audio_stdout:                  %s\n", cfg_tochar_bool(conf->audio_stdout));
    ui_message("\n");
    ui_message("codec_opus_bitrate:            %u (b/s)\n", conf->codec_opus_bitrate);
    ui_message("\n");
    ui_message("network_server:                %s\n", conf->network_server);
    ui_message("network_port:                  %u\n", conf->network_port);
    ui_message("\n");
}

int cfg_parse(int argc, char **argv) {
    int ret;
    int option_index = 0;
    int c;
    size_t ln;
    char *config_filename;
    char *endptr;

    config_filename = NULL;

    ret = EXIT_SUCCESS;

    static struct option long_options[] = {
            {"config",                required_argument, 0, 'c'},

            {"help",                  no_argument,       0, 'h'},
            {"version",               no_argument,       0, 'V'},

            {"quiet",                 no_argument,       0, 'q'},
            {"verbose",               no_argument,       0, 'v'},
            {"ui-log-level",          required_argument, 0, 'd'},
            {"file-log-level",        required_argument, 0, 'l'},
            {"file-log-name",         required_argument, 0, 'L'},

            {"debug",                 no_argument,       0, 'D'},

            {"source",                required_argument, 0, 's'},
            {"mode",                  required_argument, 0, 'm'},

            {"audio-monitor-enabled", no_argument,       0, 'a'},
            {"audio-file-enabled",    no_argument,       0, 'w'},
            {"audio-stdout",          no_argument,       0, 'o'},

            {0, 0,                                       0, 0}
    };

    config_filename = (char *) malloc(sizeof(char));
    *config_filename = '\0';

    while (1) {
        c = getopt_long(argc, argv, "c:hVqvd:l:L:Ds:m:awo", long_options, &option_index);

        if (c == -1) {
            break;
        }

        if (c == '?') {
            conf->mode = MODE_HELP;
            break;
        }

        if (c == 'c') {
            ln = strlen(optarg) + 1;
            config_filename = (char *) realloc(config_filename, ln * sizeof(char));
            strcpy(config_filename, optarg);

            ret = cfg_parse_file(config_filename);
            continue;
        }

        if (c == 'h') {
            conf->mode = MODE_HELP;
            break;
        }

        if (c == 'V') {
            conf->mode = MODE_VERSION;
            break;
        }

        if (c == 'q') {
            conf->ui_log_level = LOG_LEVEL_OFF;
            continue;
        }

        if (c == 'v') {
            conf->ui_log_level = LOG_LEVEL_DEBUG;
            continue;
        }

        if (c == 'd') {
            conf->ui_log_level = (int) strtol(optarg, &endptr, 10);
            continue;
        }

        if (c == 'l') {
            conf->file_log_level = (int) strtol(optarg, &endptr, 10);
            continue;
        }

        if (c == 'L') {
            ln = strlen(optarg) + 1;
            conf->file_log_name = (char *) realloc((void *) conf->file_log_name, sizeof(char) * ln);
            strcpy(conf->file_log_name, optarg);
            continue;
        }

        if (c == 'D') {
            conf->debug = FLAG_TRUE;
            continue;
        }

        if (c == 's') {
            if (cfg_parse_source_type(&conf->source, optarg) != EXIT_SUCCESS) {
                ret = EXIT_FAILURE;
                break;
            }

            continue;
        }

        if (c == 'm') {
            if (cfg_parse_work_mode(&conf->mode, optarg) != EXIT_SUCCESS) {
                ret = EXIT_FAILURE;
                break;
            }

            continue;
        }

        if (c == 'a') {
            conf->audio_monitor_enabled = FLAG_TRUE;
            continue;
        }

        if (c == 'w') {
            conf->audio_file_enabled = FLAG_TRUE;
            continue;
        }

        if (c == 'o') {
            conf->audio_stdout = FLAG_TRUE;
            continue;
        }
    }

    free(config_filename);

    return ret;
}

int cfg_parse_file(char *config_filename) {
    int ret;

    FILE *fd;
    size_t line_size;
    size_t line_num;
    char *line;
    char *line_trimmed;
    char *save_ptr;
    char *endptr;

    char *param;
    char *value;

    line_size = 0;
    line = NULL;
    line_trimmed = NULL;
    save_ptr = NULL;

    param = NULL;
    value = NULL;
    size_t ln;

    ret = EXIT_SUCCESS;

    fd = fopen(config_filename, "r");
    if (fd == NULL) {
        ui_message("Unable to open config file \"%s\" with error %d: %s\n", config_filename, errno, strerror(errno));
        return EXIT_FAILURE;
    }

    rewind(fd);

    line_num = 0;

    while (getline(&line, &line_size, fd) >= 0) {
        line_num++;

        line_trimmed = (char *) realloc(line_trimmed, sizeof(char) * (line_size + 1));
        if (utils_trim(line_trimmed, line, line_size + 1) != EXIT_SUCCESS) {
            ui_message("Unable to read line %zu\n", line_num);
            continue;
        }

        if (line_trimmed[0] == '#')
            continue;

        param = strtok_r(line, " ", &save_ptr);
        if (param == NULL)
            continue;

        value = (char *) realloc(value, sizeof(char) * (strlen(save_ptr) + 1));
        if (utils_trim(value, save_ptr, strlen(save_ptr) + 1) != EXIT_SUCCESS) {
            ui_message("Unable to read value on line %zu\n", line_num);
            continue;
        }

        if (strcmp(param, "ui_log_level") == 0) {
            conf->ui_log_level = (int) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "file_log_level") == 0) {
            conf->file_log_level = (int) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "file_log_name") == 0) {
            ln = strlen(value) + 1;
            conf->file_log_name = (char *) realloc((void *) conf->file_log_name, sizeof(char) * ln);
            strcpy(conf->file_log_name, value);
            continue;
        }

        if (strcmp(param, "debug") == 0) {
            conf->debug = cfg_parse_flag(value);
            continue;
        }

        if (strcmp(param, "source") == 0) {
            if (cfg_parse_source_type(&conf->source, value) != EXIT_SUCCESS) {
                log_error("Config file error in line %zu", line_num);
                ret = EXIT_FAILURE;
                break;
            }

            continue;
        }

        if (strcmp(param, "mode") == 0) {
            if (cfg_parse_work_mode(&conf->mode, value) != EXIT_SUCCESS) {
                log_error("Config file error in line %zu", line_num);
                ret = EXIT_FAILURE;
                break;
            }

            continue;
        }

        if (strcmp(param, "rtlsdr_device_id") == 0) {
            conf->rtlsdr_device_id = (uint32_t) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "rtlsdr_device_sample_rate") == 0) {
            conf->rtlsdr_device_sample_rate = (uint32_t) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "rtlsdr_device_center_freq") == 0) {
            conf->rtlsdr_device_center_freq = (uint32_t) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "rtlsdr_device_freq_correction") == 0) {
            conf->rtlsdr_device_freq_correction = (int) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "rtlsdr_device_tuner_gain_mode") == 0) {
            conf->rtlsdr_device_tuner_gain_mode = (int) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "rtlsdr_device_tuner_gain") == 0) {
            conf->rtlsdr_device_tuner_gain = (int) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "rtlsdr_device_agc_mode") == 0) {
            conf->rtlsdr_device_agc_mode = (int) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "rtlsdr_samples") == 0) {
            conf->rtlsdr_samples = (size_t) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "modulation") == 0) {
            if (cfg_parse_modulation(&conf->modulation, value) != EXIT_SUCCESS) {
                log_error("Config file error in line %zu", line_num);
                ret = EXIT_FAILURE;
                break;
            }

            continue;
        }

        if (strcmp(param, "filter") == 0) {
            if (cfg_parse_filter_mode(&conf->filter, value) != EXIT_SUCCESS) {
                log_error("Config file error in line %zu", line_num);
                ret = EXIT_FAILURE;
                break;
            }

            continue;
        }

        if (strcmp(param, "filter_fir") == 0) {
            conf->filter_fir = (int) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "audio_frames_per_period") == 0) {
            conf->audio_frames_per_period = (uint64_t) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "audio_sample_rate") == 0) {
            conf->audio_sample_rate = (uint32_t) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "audio_file_enabled") == 0) {
            conf->audio_file_enabled = cfg_parse_flag(value);
            continue;
        }

        if (strcmp(param, "audio_file_path") == 0) {
            ln = strlen(value) + 1;
            conf->audio_file_path = (char *) realloc((void *) conf->audio_file_path, sizeof(char) * ln);
            strcpy(conf->audio_file_path, value);
            continue;
        }

        if (strcmp(param, "audio_monitor_enabled") == 0) {
            conf->audio_monitor_enabled = cfg_parse_flag(value);
            continue;
        }

        if (strcmp(param, "audio_monitor_device") == 0) {
            ln = strlen(value) + 1;
            conf->audio_monitor_device = (char *) realloc((void *) conf->audio_monitor_device, sizeof(char) * ln);
            strcpy(conf->audio_monitor_device, value);
            continue;
        }

        if (strcmp(param, "audio_monitor_enabled") == 0) {
            conf->audio_stdout = cfg_parse_flag(value);
            continue;
        }

        if (strcmp(param, "codec_opus_bitrate") == 0) {
            conf->codec_opus_bitrate = (uint32_t) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "network_server") == 0) {
            ln = strlen(value) + 1;
            conf->network_server = (char *) realloc((void *) conf->network_server, sizeof(char) * ln);
            strcpy(conf->network_server, value);
            continue;
        }

        if (strcmp(param, "network_port") == 0) {
            conf->network_port = (uint16_t) strtol(value, &endptr, 10);
            continue;
        }

        log_debug("Line: %zu - Param: \"%s\" - Value: \"%s\"", line_num, param, value);
    }

    fclose(fd);

    free(line);
    free(line_trimmed);

    return ret;
}

bool_flag cfg_parse_flag(const char *flag) {
    if (utils_stricmp(flag, "y") == 0
        || utils_stricmp(flag, "yes") == 0
        || utils_stricmp(flag, "t") == 0
        || utils_stricmp(flag, "true") == 0
        || utils_stricmp(flag, "1") == 0)
        return FLAG_TRUE;
    else
        return FLAG_FALSE;
}

int cfg_parse_source_type(source_type *source, char *value) {
    int ret;

    ret = EXIT_SUCCESS;

    if (strcmp(value, "rtlsdr") == 0)
        *source = SOURCE_RTLSDR;
    else if (strcmp(value, "file") == 0)
        *source = SOURCE_FILE;
    else {
        log_error("Wrong source: %s", value);
        ret = EXIT_FAILURE;
    }

    return ret;
}

int cfg_parse_work_mode(work_mode *source, char *value) {
    int ret;

    ret = EXIT_SUCCESS;

    if (strcmp(value, "rx") == 0)
        *source = MODE_RX;
    else if (strcmp(value, "info") == 0)
        *source = MODE_INFO;
    else {
        log_error("Wrong source: %s", value);
        ret = EXIT_FAILURE;
    }

    return ret;
}

int cfg_parse_modulation(modulation_type *mod, char *value) {
    int ret;

    ret = EXIT_SUCCESS;

    if (utils_stricmp(value, "AM") == 0)
        *mod = MOD_TYPE_AM;
    else if (utils_stricmp(value, "FM") == 0)
        *mod = MOD_TYPE_FM;
    else {
        log_error("Wrong mode: %s", value);
        ret = EXIT_FAILURE;
    }

    return ret;
}

int cfg_parse_filter_mode(filter_mode *filter, char *value) {
    int ret;

    ret = EXIT_SUCCESS;

    if (strcmp(value, "none") == 0)
        *filter = FILTER_MODE_NONE;
    else if (strcmp(value, "fir_sw") == 0)
        *filter = FILTER_MODE_FIR_SW;
    else if (strcmp(value, "fft_sw") == 0)
        *filter = FILTER_MODE_FFT_SW;
    else {
        log_error("Wrong mode: %s", value);
        ret = EXIT_FAILURE;
    }

    return ret;
}

const char *cfg_tochar_bool(bool_flag value) {
    switch (value) {
        case FLAG_FALSE:
            return "NO";
        case FLAG_TRUE:
            return "YES";
        default:
            return "";
    }
}

const char *cfg_tochar_log_level(int value) {
    switch (value) {
        case LOG_LEVEL_OFF:
            return " OFF ";
        case LOG_LEVEL_ERROR:
            return "ERROR";
        case LOG_LEVEL_WARNING:
            return "WARN ";
        case LOG_LEVEL_INFO:
            return "INFO ";
        case LOG_LEVEL_DEBUG:
            return "DEBUG";
        case LOG_LEVEL_TRACE:
            return "TRACE";
        default:
            return "     ";
    }
}

const char *cfg_tochar_source_type(source_type source) {
    switch (source) {
        case SOURCE_RTLSDR:
            return "RTL-SDR device";
        case SOURCE_FILE:
            return "Raw IQ file";
        default:
            return "";
    }
}

const char *cfg_tochar_work_mode(work_mode value) {
    switch (value) {
        case MODE_VERSION:
            return "VERSION";
        case MODE_HELP:
            return "HELP";
        case MODE_RX:
            return "RX";
        case MODE_INFO:
            return "INFO";
        default:
            return "";
    }
}

const char *cfg_tochar_modulation(modulation_type value) {
    switch (value) {
        case MOD_TYPE_AM:
            return "AM";
        case MOD_TYPE_FM:
            return "FM";
        default:
            return "";
    }
}

const char *cfg_tochar_filter_mode(filter_mode value) {
    switch (value) {
        case FILTER_MODE_NONE:
            return "None (bypass)";
        case FILTER_MODE_FIR_SW:
            return "FIR Software (Finite Impulse Response with no optimizations)";
        case FILTER_MODE_FFT_SW:
            return "Fast Fourier Transform (based on libfftw3)";
        default:
            return "";
    }
}
