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
#include <rtl-sdr.h>
#include <complex.h>
#include <math.h>

#include "main.h"
#include "ui.h"
#include "cfg.h"
#include "device.h"
#include "log.h"

extern cfg *conf;
extern rtlsdr_dev_t *device;

int keep_running = 1;

int main(int argc, char **argv) {
    int result;

    ui_header();

    cfg_init();

    result = cfg_parse(argc, argv);

    cfg_print();

    if (result == 0) {
        result = main_program();
    } else {
        ui_usage();
    }

    cfg_free();

    return result;
}

int main_program() {
    uint8_t *input_buffer;
    int8_t *output_buffer;

    size_t output_buffer_size;
    int bytes;
    int result;

    double complex sample;
    double complex product;
    double complex prev_sample;

    double value;
    int8_t elem;

    output_buffer_size = conf->rtlsdr_buffer / 2;

    result = device_open();
    if (result == -1)
        return EXIT_FAILURE;

    input_buffer = (uint8_t *) calloc(conf->rtlsdr_buffer, sizeof(uint8_t));
    output_buffer = (int8_t *) calloc(output_buffer_size, sizeof(int8_t));

    prev_sample = 0 + 0 * I;

    while (keep_running) {
        result = rtlsdr_read_sync(device, input_buffer, conf->rtlsdr_buffer, &bytes);
        if (result != 0)
            break;

        for (int i = 0; i < conf->rtlsdr_buffer; i += 2) {
            sample = (input_buffer[i] - 127) + (input_buffer[i + 1] - 127) * I;
            product = sample * conj(prev_sample);
            value = atan2(cimag(product), creal(product)) / M_PI;
            elem = (int8_t) (value * 127);
            output_buffer[i / 2] = elem;
            prev_sample = sample;
        }

        fwrite(output_buffer, sizeof(int8_t), output_buffer_size, stdout);
    }

    free(input_buffer);
    free(output_buffer);

    device_close();

    return EXIT_SUCCESS;
}
