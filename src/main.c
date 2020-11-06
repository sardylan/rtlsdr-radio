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
#include "resample.h"

const char *main_program_name;
static volatile int keep_running = 1;
circbuf_ctx *buffer_iq;
circbuf_ctx *buffer_demod;

pthread_t rx_device_thread;
pthread_t rx_demod_thread;
pthread_t rx_resample_thread;

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
        main_stop();
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
            log_error("Mode not implemented");
            return EXIT_FAILURE;
    }
}

int main_program_mode_rx() {
    int result;
    pthread_attr_t attr;
    struct timespec sleep_req;
    struct timespec sleep_rem;

    log_info("Main program RX mode");

    log_debug("Allocating IQ circbuf");
    buffer_iq = (circbuf_ctx *) malloc(sizeof(circbuf_ctx));
    if (buffer_iq == NULL) {
        log_error("Unable to allocate IQ circular buffer");
        return EXIT_FAILURE;
    }

    log_debug("Init IQ circbuf");
    result = circbuf_init(buffer_iq);
    if (result == EXIT_FAILURE) {
        log_error("Unable to init IQ circular buffer");
        return EXIT_FAILURE;
    }

    log_debug("Allocating demod circbuf");
    buffer_demod = (circbuf_ctx *) malloc(sizeof(circbuf_ctx));
    if (buffer_demod == NULL) {
        log_error("Unable to allocate demod circular buffer");
        return EXIT_FAILURE;
    }

    log_debug("Init demod circbuf");
    result = circbuf_init(buffer_demod);
    if (result == EXIT_FAILURE) {
        log_error("Unable to init demod circular buffer");
        return EXIT_FAILURE;
    }

    log_debug("Opening RTL-SDR device");
    result = device_open();
    if (result == EXIT_FAILURE) {
        log_error("Unable to open RTL-SDR device");
        return EXIT_FAILURE;
    }

    log_debug("Setting thread attributes");
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    log_debug("Starting RX device read thread");
    pthread_create(&rx_device_thread, &attr, thread_rx_device_read, NULL);

    log_debug("Starting RX demod thread");
    pthread_create(&rx_demod_thread, &attr, thread_rx_demod, NULL);

    log_debug("Starting RX resample thread");
    pthread_create(&rx_resample_thread, &attr, thread_rx_resample, NULL);

    sleep_req.tv_sec = 2;
    sleep_req.tv_nsec = 0;

    log_debug("Printing device infos");
    while (keep_running) {
        device_info();
        nanosleep(&sleep_req, &sleep_rem);
    }

    log_debug("Joining threads");
    pthread_join(rx_device_thread, NULL);
    pthread_join(rx_demod_thread, NULL);
    pthread_join(rx_resample_thread, NULL);

    log_debug("Closing RTL-SDR device");
    device_close();

    log_debug("Freeing buffers");
    circbuf_free(buffer_iq);
    circbuf_free(buffer_demod);

    return EXIT_SUCCESS;
}

int main_program_mode_info() {
    device_list();

    return EXIT_SUCCESS;
}

void main_stop() {
    keep_running = 0;
}

void *thread_rx_device_read(void *data) {
    uint8_t *input_buffer;
    size_t rtlsdr_buffer_size;
    int bytes;
    int result;

    log_info("Thread start");

    rtlsdr_buffer_size = conf->rtlsdr_samples * 2;

    log_debug("Allocating input buffer");
    input_buffer = (uint8_t *) calloc(rtlsdr_buffer_size, sizeof(uint8_t));

    log_debug("Starting read loop");
    while (keep_running) {
        result = rtlsdr_read_sync(device, input_buffer, rtlsdr_buffer_size, &bytes);
        if (result != 0) {
            log_error("Error reading data from RTL-SDR device: %d", result);
            break;
        }

        log_trace("Read %zu bytes", bytes);

        circbuf_put(buffer_iq, input_buffer, bytes);
    }

    free(input_buffer);

    log_info("Thread end");

    return (void *) EXIT_SUCCESS;
}

void *thread_rx_demod(void *data) {
    uint8_t *input_buffer;
    double complex *samples;
    int8_t *output_buffer;

    double complex sample;
    double complex product;
    double complex prev_sample;

    double value;
    int8_t elem;
    size_t rtlsdr_buffer_size;
    int j;

    log_info("Thread start");

    rtlsdr_buffer_size = conf->rtlsdr_samples * 2;

    log_debug("Allocating input, samples and output buffers");
    input_buffer = (uint8_t *) calloc(rtlsdr_buffer_size, sizeof(uint8_t));
    samples = (double complex *) calloc(conf->rtlsdr_samples, sizeof(double complex));
    output_buffer = (int8_t *) calloc(conf->rtlsdr_samples, sizeof(int8_t));

    prev_sample = 0 + 0 * I;

    log_debug("Starting demod loop");
    while (keep_running) {
        log_trace("Reading %zu bytes", rtlsdr_buffer_size);
        circbuf_get(buffer_iq, input_buffer, rtlsdr_buffer_size);

        log_trace("Converting to complex samples", rtlsdr_buffer_size);
        device_buffer_to_samples(input_buffer, samples, rtlsdr_buffer_size);

        for (j = 0; j < conf->rtlsdr_samples; j++) {
            sample = samples[j];
            product = sample * conj(prev_sample);

            switch (conf->modulation) {
                case MOD_TYPE_FM:
                    value = atan2(cimag(product), creal(product)) / M_PI;
                    break;

                case MOD_TYPE_AM:
                    value = cabs(product);
                    break;

                default:
                    value = 0;
            }

            elem = (int8_t) (value * 127);
            output_buffer[j] = elem;

            prev_sample = sample;
        }

        circbuf_put(buffer_demod, output_buffer, conf->rtlsdr_samples);
    }

    free(input_buffer);
    free(samples);
    free(output_buffer);

    log_info("Thread end");

    return (void *) EXIT_SUCCESS;
}

void *thread_rx_resample(void *data) {
    size_t audio_num;
    int8_t *input_buffer;
    int8_t *output_buffer;
    resample_ctx *res_ctx;

    log_debug("Initializing resample context");
    res_ctx = resample_init();
    if (res_ctx == NULL) {
        log_error("Unable to allocate resample context");
        main_stop();
        return (void *) EXIT_FAILURE;
    }

    audio_num = resample_compute_output_size(res_ctx, conf->rtlsdr_samples);
    log_debug("Audio buffer size is %zu", audio_num);

    log_debug("Allocating input buffer");
    input_buffer = (int8_t *) calloc(conf->rtlsdr_samples, sizeof(uint8_t));
    if (input_buffer == NULL) {
        log_error("Unable to allocate input buffer");
        main_stop();
        return (void *) EXIT_FAILURE;
    }

    log_debug("Allocating output buffer");
    output_buffer = (int8_t *) calloc(audio_num, sizeof(int8_t));
    if (res_ctx == NULL) {
        log_error("Unable to allocate resample context");
        main_stop();
        return (void *) EXIT_FAILURE;
    }

    log_debug("Creating resample context");
    res_ctx = resample_init(conf->rtlsdr_device_sample_rate, conf->audio_sample_rate);

    log_debug("Starting demod loop");
    while (keep_running) {
        log_trace("Reading %zu bytes", conf->rtlsdr_samples);
        circbuf_get(buffer_demod, input_buffer, conf->rtlsdr_samples);

        log_trace("Resampling %zu bytes in %zu bytes", conf->rtlsdr_samples, audio_num);
        resample_do(res_ctx, input_buffer, conf->rtlsdr_samples, output_buffer, audio_num);

        log_trace("Output %zu bytes", audio_num);
        fwrite(output_buffer, sizeof(int8_t), audio_num, stdout);
    }

    log_debug("Freeing resample context");
    resample_free(res_ctx);

    log_debug("Freeing buffers");
    free(input_buffer);
    free(output_buffer);

    log_info("Thread end");

    return (void *) EXIT_SUCCESS;
}
