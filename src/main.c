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
#include <signal.h>
#include <string.h>

#include "main.h"
#include "main_rx.h"
#include "main_info.h"
#include "ui.h"
#include "cfg.h"
#include "log.h"

const char *program_name;

volatile int keep_running;
cfg *conf;

int main(int argc, char **argv) {
    int result;

    program_name = argv[0];
    keep_running = 1;

    signal(SIGINT, signal_handler);

    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    ui_header();

    log_init();
    log_start();

    cfg_init();

    result = cfg_parse(argc, argv);

    if (conf->debug)
        cfg_print();

    if (result == 0) {
        switch (conf->mode) {
            case MODE_VERSION:
                result = EXIT_SUCCESS;
                break;

            case MODE_HELP:
                ui_help();
                result = EXIT_SUCCESS;
                break;

            case MODE_RX:
                result = main_rx();
                break;

            case MODE_INFO:
                result = main_info();
                break;

            default:
                log_error("Mode not implemented");
                result = EXIT_FAILURE;
        }
    } else {
        ui_usage();
    }

    cfg_free();

    log_stop();
    log_free();

    return result;
}

void signal_handler(int signum) {
    if (signum == SIGINT)
        main_stop();
}

void main_stop() {
    keep_running = 0;
}
