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
#include <stdarg.h>

#include "ui.h"
#include "version.h"
#include "cfg.h"
#include "default.h"
#include "log.h"

extern cfg *conf;
extern const char *main_program_name;

void ui_message(const char *format, ...) {
    va_list args;

    va_start(args, format);
    vfprintf(UI_MESSAGES_OUTPUT, format, args);
    va_end(args);
}

void ui_message_out(const char *format, ...) {
    va_list args;

    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
}

void ui_header() {
    ui_message("%s %s\n\n", APPLICATION_NAME, APPLICATION_VERSION);
}

void ui_usage() {
    ui_message("Usage:\n");
    ui_message("    %s [<option> <value> ...]\n", main_program_name);
}

void ui_help() {
    ui_usage();

    ui_message("\n");
    ui_message("\n");
    ui_message("Options list with default values:\n");
    ui_message("\n");
    ui_message("    -c | --config            Config file (overwrite all other options)\n");
    ui_message("\n");
    ui_message("    -h | --help              This message\n");
    ui_message("    -V | --version           Print Version and exit\n");
    ui_message("\n");
    ui_message("    -q | --quiet             Disable output (debug level 0)\n");
    ui_message("    -v | --verbose           Verbose output (debug level 4)\n");
    ui_message("\n");
    ui_message("    -d | --ui-log-level      UI log level (%d)\n", CONFIG_UI_LOG_LEVEL_DEFAULT);
    ui_message("                             - 0 DISABLE (quiet)\n");
    ui_message("                             - 1 ERROR\n");
    ui_message("                             - 2 WARNING\n");
    ui_message("                             - 3 INFO\n");
    ui_message("                             - 4 DEBUG (verbose)\n");
    ui_message("\n");
    ui_message("    -l | --file-log-level    File log level (%d)\n", CONFIG_FILE_LOG_LEVEL_DEFAULT);
    ui_message("                             - 0 DISABLE)\n");
    ui_message("                             - 1 ERROR\n");
    ui_message("                             - 2 WARNING\n");
    ui_message("                             - 3 INFO\n");
    ui_message("                             - 4 DEBUG\n");
    ui_message("\n");
    ui_message("    -L | --file-log-name     Log file name and path (%s)\n", CONFIG_FILE_LOG_NAME_DEFAULT);
    ui_message("\n");
    ui_message("    -D | --debug             Enable detailed debug messages (%d)\n", CONFIG_DEBUG_DEFAULT);
    ui_message("                                          - 0 Disabled\n");
    ui_message("                                          - 1 Enabled\n");
    ui_message("\n");
    ui_message("    -m | --mode              Working mode (%s)\n", log_mode_to_char(CONFIG_MODE_DEFAULT));
    ui_message("                             - rx (Receiver)\n");
    ui_message("                             - info (Devices info)\n");
    ui_message("\n");
    ui_message("\n");
    ui_message("\n");
    ui_message("    -i | --rtlsdr-device-id               Device ID (%zu)\n", CONFIG_RTLSDR_DEVICE_ID_DEFAULT);
    ui_message("    -f | --rtlsdr-device-center-freq      Center frequency in Hz (%zu)\n",
               CONFIG_RTLSDR_DEVICE_CENTER_FREQ_DEFAULT);
    ui_message("    -p | --rtlsdr-device-freq-correction  Frequency correction in ppm (%d)\n",
               CONFIG_RTLSDR_DEVICE_FREQ_CORRECTION_DEFAULT);
    ui_message("\n");
    ui_message("    -G | --rtlsdr-device-tuner-gain-mode  Tuner gain mode (%d)\n",
               CONFIG_RTLSDR_DEVICE_TUNER_GAIN_MODE_DEFAULT);
    ui_message("                                          - 0 Automatic\n");
    ui_message("                                          - 1 Manual\n");
    ui_message("\n");
    ui_message("    -g | --rtlsdr-device-tuner-gain       Tune gain in tenths of dB Hz (%d)\n",
               CONFIG_RTLSDR_DEVICE_TUNER_GAIN_DEFAULT);
    ui_message("\n");
    ui_message("    -a | --rtlsdr-device-agc-mode         AGC Mode (%d)\n", CONFIG_RTLSDR_DEVICE_AGC_MODEDEFAULT);
    ui_message("                                          - 0 Disabled\n");
    ui_message("                                          - 1 Enabled\n");
    ui_message("\n");
    ui_message("All outputs are on Standard Error.\n");
}
