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
    return 0;
}
