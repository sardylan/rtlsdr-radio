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
#include <unistd.h>
#include <signal.h>
#include <rtl-sdr.h>
#include <complex.h>
#include <math.h>

#include "main.h"
#include "ui.h"
#include "cfg.h"
#include "device.h"
#include "log.h"
#include "circbuf.h"

const char *main_program_name;
static volatile int keep_running = 1;
circbuf_ctx *buffer_iq;

pthread_t device_thread;
pthread_t demod_thread;

extern cfg *conf;
extern rtlsdr_dev_t *device;

int main(int argc, char **argv) {
    int result;

    main_program_name = argv[0];

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
        result = main_program();
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
        keep_running = 0;
}

int main_program() {
    switch (conf->mode) {
        case MODE_VERSION:
            return EXIT_SUCCESS;
        case MODE_HELP:
            ui_help();
            return EXIT_SUCCESS;
        case MODE_RX:
            return main_program_mode_rx();
        case MODE_INFO:
            return main_program_mode_info();
        default:
            log_error("main", "Mode not implemented");
            return EXIT_FAILURE;
    }
}

int main_program_mode_rx() {
    int result;
    pthread_attr_t attr;

    log_debug("main", "Allocating circbuf");
    buffer_iq = (circbuf_ctx *) malloc(sizeof(circbuf_ctx));
    if (buffer_iq == NULL) {
        log_error("main", "Unable to allocate circular buffer");
        return EXIT_FAILURE;
    }

    result = circbuf_init(buffer_iq);
    if (result == EXIT_FAILURE) {
        log_error("main", "Unable to init IQ circular buffer");
        return EXIT_FAILURE;
    }

    result = device_open();
    if (result == -1)
        return EXIT_FAILURE;

    log_debug("main", "Setting thread attributes");
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    log_debug("main", "Starting RX device read thread");
    pthread_create(&device_thread, &attr, thread_rx_device_read, NULL);

    log_debug("main", "Starting RX demod thread");
    pthread_create(&demod_thread, &attr, thread_rx_demod, NULL);

    log_debug("main", "Joining threads");
    pthread_join(device_thread, NULL);
    pthread_join(demod_thread, NULL);

    device_close();

    circbuf_free(buffer_iq);

    return EXIT_SUCCESS;
}

int main_program_mode_info() {
    device_list();

    return EXIT_SUCCESS;
}

void *thread_rx_device_read(void *data) {
    uint8_t *input_buffer;
    int bytes;
    int result;

    log_info("thread-device", "Thread start");

    log_debug("thread-device", "Allocating input buffer");
    input_buffer = (uint8_t *) calloc(conf->rtlsdr_buffer, sizeof(uint8_t));

    log_debug("thread-device", "Starting read loop");
    while (keep_running) {
        result = rtlsdr_read_sync(device, input_buffer, conf->rtlsdr_buffer, &bytes);
        if (result != 0) {
            log_error("thread-device", "Error reading data from RTL-SDR device: %d", result);
            break;
        }

        log_trace("thread-device", "Read %zu bytes", bytes);

        circbuf_put(buffer_iq, input_buffer, bytes);
    }

    free(input_buffer);

    log_info("thread-device", "Thread end");

    return (void *) EXIT_SUCCESS;
}

void *thread_rx_demod(void *data) {
    uint8_t *input_buffer;
    int8_t *output_buffer;
    int j;
    uint8_t i;
    uint8_t q;

    size_t output_buffer_size;

    double complex sample;
    double complex product;
    double complex prev_sample;

    double value;
    int8_t elem;

    log_info("thread-demod", "Thread start");

    output_buffer_size = conf->rtlsdr_buffer / 2;
    log_debug("thread-demod", "Output buffer size is %zu", output_buffer_size);

    log_debug("thread-demod", "Allocating input and output buffers");
    input_buffer = (uint8_t *) calloc(conf->rtlsdr_buffer, sizeof(uint8_t));
    output_buffer = (int8_t *) calloc(output_buffer_size, sizeof(int8_t));

    output_buffer_size = conf->rtlsdr_buffer / 2;

    log_debug("thread-demod", "Starting demod loop");
    while (keep_running) {
        circbuf_get(buffer_iq, input_buffer, conf->rtlsdr_buffer);
        log_trace("thread-demod", "Read %zu bytes", conf->rtlsdr_buffer);

        for (j = 0; j < conf->rtlsdr_buffer; j += 2) {
            i = input_buffer[j] - 127;
            q = input_buffer[j + 1] - 127;
            sample = i + q * I;

            product = sample * conj(prev_sample);
            value = atan2(cimag(product), creal(product)) / M_PI;
//            value = cabs(product);

            elem = (int8_t) (value * 127);
            output_buffer[j / 2] = elem;
            prev_sample = sample;
        }

        fwrite(output_buffer, sizeof(int8_t), output_buffer_size, stdout);
    }

    free(input_buffer);

    log_info("thread-demod", "Thread end");

    return (void *) EXIT_SUCCESS;
}
