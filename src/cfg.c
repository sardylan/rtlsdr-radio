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
#include <getopt.h>

#include "cfg.h"
#include "default.h"
#include "ui.h"
#include "log.h"

cfg *conf;

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
    conf->rtlsdr_buffer = CONFIG_RTLSDR_BUFFER_DEFAULT;
}

void cfg_free() {
    free(conf->file_log_name);

    free(conf);
}

void cfg_print() {
    fprintf(UI_MESSAGES_OUTPUT, "ui_log_level:                  %s\n", log_level_to_char(conf->ui_log_level));
    fprintf(UI_MESSAGES_OUTPUT, "file_log_level:                %s\n", log_level_to_char(conf->file_log_level));
    fprintf(UI_MESSAGES_OUTPUT, "file_log_name:                 %s\n", conf->file_log_name);
    fprintf(UI_MESSAGES_OUTPUT, "mode:                          %s\n", log_mode_to_char(conf->mode));
    fprintf(UI_MESSAGES_OUTPUT, "debug:                         %s\n", log_bool_to_char(conf->debug));
    fprintf(UI_MESSAGES_OUTPUT, "rtlsdr_device_id:              %u\n", conf->rtlsdr_device_id);
    fprintf(UI_MESSAGES_OUTPUT, "rtlsdr_device_sample_rate:     %u\n", conf->rtlsdr_device_sample_rate);
    fprintf(UI_MESSAGES_OUTPUT, "rtlsdr_device_center_freq:     %u\n", conf->rtlsdr_device_center_freq);
    fprintf(UI_MESSAGES_OUTPUT, "rtlsdr_device_freq_correction: %u\n", conf->rtlsdr_device_freq_correction);
    fprintf(UI_MESSAGES_OUTPUT, "rtlsdr_device_tuner_gain_mode: %u\n", conf->rtlsdr_device_tuner_gain_mode);
    fprintf(UI_MESSAGES_OUTPUT, "rtlsdr_device_tuner_gain:      %u\n", conf->rtlsdr_device_tuner_gain);
    fprintf(UI_MESSAGES_OUTPUT, "rtlsdr_device_agc_mode:        %u\n", conf->rtlsdr_device_agc_mode);
    fprintf(UI_MESSAGES_OUTPUT, "rtlsdr_buffer:                 %zu\n", conf->rtlsdr_buffer);
    fprintf(UI_MESSAGES_OUTPUT, "\n");
}

int cfg_parse(int argc, char **argv) {
    int ret = EXIT_SUCCESS;
    int option_index = 0;
    int c;
    size_t ln;
    int conf_file = 0;
    char *config_file;
    char *endptr;

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
            {"rtlsdr-device-sample-rate",     required_argument, 0, 's'},
            {"rtlsdr-device-center-freq",     required_argument, 0, 'f'},
            {"rtlsdr-device-freq-correction", required_argument, 0, 'p'},
            {"rtlsdr-device-tuner-gain-mode", required_argument, 0, 'G'},
            {"rtlsdr-device-tuner-gain",      required_argument, 0, 'g'},
            {"rtlsdr-device-agc-mode",        required_argument, 0, 'a'},
            {"rtlsdr-buffer",                 required_argument, 0, 'b'},

            {0, 0,                                               0, 0}
    };

    config_file = (char *) malloc(sizeof(char));
    *config_file = '\0';

    while (1) {
        c = getopt_long(argc, argv, "c:hVqvd:l:L:Dm:i:s:f:p:G:g:a:b:", long_options, &option_index);

        if (c == -1) {
            break;
        }

        if (c == '?') {
            conf->mode = MODE_HELP;
            break;
        }

        if (c == 'c') {
            conf_file = 1;

            ln = strlen(optarg) + 1;
            config_file = (char *) realloc(config_file, ln * sizeof(char));
            strcpy(config_file, optarg);
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
        }

        if (c == 'v') {
            conf->ui_log_level = LOG_LEVEL_DEBUG;
        }

        if (c == 'd') {
            conf->ui_log_level = (int) strtol(optarg, &endptr, 10);
        }

        if (c == 'l') {
            conf->file_log_level = (int) strtol(optarg, &endptr, 10);
        }

        if (c == 'L') {
            ln = strlen(optarg) + 1;
            conf->file_log_name = (char *) realloc((void *) conf->file_log_name, sizeof(char) * ln);
            strcpy(conf->file_log_name, optarg);
        }

        if (c == 'D') {
            conf->debug = 1;
        }

        if (c == 'm') {
            ln = strlen(optarg) + 1;

            if (strcmp(optarg, "rx") == 0)
                conf->mode = MODE_RX;
            else if (strcmp(optarg, "info") == 0)
                conf->mode = MODE_INFO;
            else {
                log_error("conf", "Wrong mode: %s", optarg);
                ret = EXIT_FAILURE;
                break;
            }
        }

        if (c == 'i') {
            conf->rtlsdr_device_id = (uint32_t) strtol(optarg, &endptr, 10);
        }

        if (c == 's') {
            conf->rtlsdr_device_sample_rate = (uint32_t) strtol(optarg, &endptr, 10);
        }

        if (c == 'f') {
            conf->rtlsdr_device_center_freq = (uint32_t) strtol(optarg, &endptr, 10);
        }

        if (c == 'p') {
            conf->rtlsdr_device_freq_correction = (int) strtol(optarg, &endptr, 10);
        }

        if (c == 'G') {
            conf->rtlsdr_device_tuner_gain_mode = cfg_parse_flag((int) strtol(optarg, &endptr, 10));
        }

        if (c == 'g') {
            conf->rtlsdr_device_tuner_gain = (int) strtol(optarg, &endptr, 10);
        }

        if (c == 'a') {
            conf->rtlsdr_device_agc_mode = cfg_parse_flag((int) strtol(optarg, &endptr, 10));
        }

        if (c == 'b') {
            conf->rtlsdr_buffer = (size_t) strtol(optarg, &endptr, 10);
        }
    }

//    if (conf_file == 1)
//        ret = cfg_file_parse(config_file);

    free(config_file);

    return ret;
}

int cfg_parse_flag(int flag) {
    if (flag != 0)
        return 1;
    else
        return 0;
}