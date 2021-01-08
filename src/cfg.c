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

    conf->mode = CONFIG_MODE_DEFAULT;
    conf->debug = CONFIG_DEBUG_DEFAULT;

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

    conf->audio_sample_rate = CONFIG_AUDIO_SAMPLE_RATE_DEFAULT;

    conf->codec_opus_bitrate = CONFIG_CODEC_OPUS_BITRATE_DEFAULT;

    ln = strlen(CONFIG_NETWORK_SERVER_DEFAULT) + 1;
    conf->network_server = (char *) calloc(sizeof(char), ln);
    strcpy(conf->network_server, CONFIG_NETWORK_SERVER_DEFAULT);

    conf->network_port = CONFIG_NETWORK_PORT_DEFAULT;
}

void cfg_free() {
    free(conf->file_log_name);
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
    ui_message("mode:                          %s\n", cfg_tochar_work_mode(conf->mode));
    ui_message("\n");
    ui_message("rtlsdr_device_id:              %u\n", conf->rtlsdr_device_id);
    ui_message("rtlsdr_device_sample_rate:     %u (Hz)\n", conf->rtlsdr_device_sample_rate);
    ui_message("rtlsdr_device_center_freq:     %u (Hz)\n", conf->rtlsdr_device_center_freq);
    ui_message("rtlsdr_device_freq_correction: %u (ppm)\n", conf->rtlsdr_device_freq_correction);
    ui_message("rtlsdr_device_tuner_gain_mode: %u\n", conf->rtlsdr_device_tuner_gain_mode);
    ui_message("rtlsdr_device_tuner_gain:      %u (10e-1 dB)\n", conf->rtlsdr_device_tuner_gain);
    ui_message("rtlsdr_device_agc_mode:        %u\n", conf->rtlsdr_device_agc_mode);
    ui_message("rtlsdr_samples:                %zu\n", conf->rtlsdr_samples);
    ui_message("\n");
    ui_message("modulation:                    %s\n", cfg_tochar_modulation(conf->modulation));
    ui_message("\n");
    ui_message("filter:                        %s\n", cfg_tochar_filter_mode(conf->filter));
    ui_message("filter_fir:                    %d\n", conf->filter_fir);
    ui_message("\n");
    ui_message("audio_sample_rate:             %u (Hz)\n", conf->audio_sample_rate);
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
            {"config",                        required_argument, 0, 'c'},

            {"help",                          no_argument,       0, 'h'},
            {"version",                       no_argument,       0, 'V'},

            {"quiet",                         no_argument,       0, 'q'},
            {"verbose",                       no_argument,       0, 'v'},
            {"ui-log-level",                  required_argument, 0, 'd'},
            {"file-log-level",                required_argument, 0, 'l'},
            {"file-log-name",                 required_argument, 0, 'L'},

            {"debug",                         no_argument,       0, 'D'},

            {"mode",                          required_argument, 0, 'm'},

            {"rtlsdr-device-id",              required_argument, 0, 'i'},
            {"rtlsdr-device-center-freq",     required_argument, 0, 'f'},
            {"rtlsdr-device-freq-correction", required_argument, 0, 'p'},
            {"rtlsdr-device-tuner-gain-mode", required_argument, 0, 'G'},
            {"rtlsdr-device-tuner-gain",      required_argument, 0, 'g'},
            {"rtlsdr-device-agc-mode",        required_argument, 0, 'a'},

            {"modulation",                    required_argument, 0, 'M'},

            {0, 0,                                               0, 0}
    };

    config_filename = (char *) malloc(sizeof(char));
    *config_filename = '\0';

    while (1) {
        c = getopt_long(argc, argv, "c:hVqvd:l:L:Dm:i:f:p:G:g:a:M:", long_options, &option_index);

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
            conf->debug = 1;
            continue;
        }

        if (c == 'm') {
            if (cfg_parse_work_mode(&conf->mode, optarg) != EXIT_SUCCESS) {
                ret = EXIT_FAILURE;
                break;
            }

            continue;
        }

        if (c == 'i') {
            conf->rtlsdr_device_id = (uint32_t) strtol(optarg, &endptr, 10);
            continue;
        }

        if (c == 's') {
            conf->rtlsdr_device_sample_rate = (uint32_t) strtol(optarg, &endptr, 10);
            continue;
        }

        if (c == 'f') {
            conf->rtlsdr_device_center_freq = (uint32_t) strtol(optarg, &endptr, 10);
            continue;
        }

        if (c == 'p') {
            conf->rtlsdr_device_freq_correction = (int) strtol(optarg, &endptr, 10);
            continue;
        }

        if (c == 'G') {
            conf->rtlsdr_device_tuner_gain_mode = cfg_parse_flag((int) strtol(optarg, &endptr, 10));
            continue;
        }

        if (c == 'g') {
            conf->rtlsdr_device_tuner_gain = (int) strtol(optarg, &endptr, 10);
            continue;
        }

        if (c == 'a') {
            conf->rtlsdr_device_agc_mode = cfg_parse_flag((int) strtol(optarg, &endptr, 10));
            continue;
        }

        if (c == 'M') {
            if (cfg_parse_modulation(&conf->modulation, optarg) != EXIT_SUCCESS) {
                log_error("Wrong modulation: %s", optarg);
                ret = EXIT_FAILURE;
                break;
            }

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

    line_num = 1;

    while (getline(&line, &line_size, fd) >= 0) {
        line_trimmed = utils_trim(line);
        if (line_trimmed[0] == '#')
            continue;

        param = strtok_r(line, " ", &save_ptr);
        if (param == NULL)
            continue;

        value = utils_trim(save_ptr);

        if (strcmp(param, "ui-log-level") == 0) {
            conf->ui_log_level = (int) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "file-log-level") == 0) {
            conf->file_log_level = (int) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "file-log-name") == 0) {
            ln = strlen(value) + 1;
            conf->file_log_name = (char *) realloc((void *) conf->file_log_name, sizeof(char) * ln);
            strcpy(conf->file_log_name, value);
            continue;
        }

        if (strcmp(param, "debug") == 0) {
            conf->file_log_level = cfg_parse_flag((int) strtol(value, &endptr, 10));
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

        if (strcmp(param, "rtlsdr-device-id") == 0) {
            conf->rtlsdr_device_id = (uint32_t) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "rtlsdr-device-sample-rate") == 0) {
            conf->rtlsdr_device_sample_rate = (uint32_t) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "rtlsdr-device-center-freq") == 0) {
            conf->rtlsdr_device_center_freq = (uint32_t) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "rtlsdr-device-freq-correction") == 0) {
            conf->rtlsdr_device_freq_correction = (int) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "rtlsdr-device-tuner-gain-mode") == 0) {
            conf->rtlsdr_device_tuner_gain_mode = (int) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "rtlsdr-device-tuner-gain") == 0) {
            conf->rtlsdr_device_tuner_gain = (int) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "rtlsdr-device-agc-mode") == 0) {
            conf->rtlsdr_device_agc_mode = (int) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "rtlsdr-samples") == 0) {
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

        if (strcmp(param, "filter-fir") == 0) {
            conf->filter_fir = (int) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "audio-sample-rate") == 0) {
            conf->audio_sample_rate = (uint32_t) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "codec-opus-bitrate") == 0) {
            conf->codec_opus_bitrate = (uint32_t) strtol(value, &endptr, 10);
            continue;
        }

        if (strcmp(param, "network-server") == 0) {
            ln = strlen(value) + 1;
            conf->network_server = (char *) realloc((void *) conf->network_server, sizeof(char) * ln);
            strcpy(conf->network_server, value);
            continue;
        }

        if (strcmp(param, "network-port") == 0) {
            conf->network_port = (uint16_t) strtol(value, &endptr, 10);
            continue;
        }

        log_debug("Line: %zu - Param: \"%s\" - Value: \"%s\"", line_num, param, value);

        line_num++;
    }

    fclose(fd);

    free(line);

    return ret;
}

int cfg_parse_flag(int flag) {
    if (flag != 0)
        return 1;
    else
        return 0;
}

int cfg_parse_work_mode(work_mode *mode, char *value) {
    int ret;

    ret = EXIT_SUCCESS;

    if (strcmp(value, "rx") == 0)
        *mode = MODE_RX;
    else if (strcmp(value, "info") == 0)
        *mode = MODE_INFO;
    else {
        log_error("Wrong mode: %s", value);
        ret = EXIT_FAILURE;
    }

    return ret;
}

int cfg_parse_modulation(modulation_type *mod, char *value) {
    int ret;

    ret = EXIT_SUCCESS;

    if ((value[0] == 'a' || value[0] == 'A') && (value[1] == 'm' || value[1] == 'M') && value[2] == '\0')
        *mod = MOD_TYPE_AM;
    else if ((value[0] == 'f' || value[0] == 'F') && (value[1] == 'm' || value[1] == 'M') && value[2] == '\0')
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

const char *cfg_tochar_bool(int value) {
    if (value == 0)
        return "NO";
    else
        return "YES";
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
