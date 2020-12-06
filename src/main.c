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
#include <pthread.h>
#include <string.h>

#include "main.h"
#include "ui.h"
#include "cfg.h"
#include "device.h"
#include "log.h"
#include "circbuf.h"
#include "resample.h"
#include "fft.h"
#include "dsp.h"
#include "fir_lpf.h"
#include "agc.h"
#include "network.h"

const char *main_program_name;
static volatile int keep_running = 1;
circbuf_ctx *buffer_samples;
circbuf_ctx *buffer_demod;
circbuf_ctx *buffer_filtered;

pthread_t rx_device_thread;
pthread_t rx_demod_thread;
pthread_t rx_lpf_thread;
pthread_t rx_resample_thread;
pthread_t rx_network_thread;

pthread_mutex_t rx_ready_mutex;
pthread_cond_t rx_ready_cond;
int rx_device_ready;
int rx_demod_ready;
int rx_lpf_ready;
int rx_resample_ready;
int rx_network_ready;

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

    log_debug("Init IQ circbuf");
    buffer_samples = circbuf_init(sizeof(double complex), 2097152);
    if (buffer_samples == NULL) {
        log_error("Unable to init IQ circular buffer");
        return EXIT_FAILURE;
    }

    log_debug("Init demod circbuf");
    buffer_demod = circbuf_init(sizeof(int8_t), 2097152);
    if (buffer_demod == NULL) {
        log_error("Unable to init demod circular buffer");
        return EXIT_FAILURE;
    }

    log_debug("Init filtered circbuf");
    buffer_filtered = circbuf_init(sizeof(int8_t), 2097152);
    if (buffer_filtered == NULL) {
        log_error("Unable to allocate lpf circular buffer");
        return EXIT_FAILURE;
    }

    log_debug("Opening RTL-SDR device");
    result = device_open();
    if (result == EXIT_FAILURE) {
        log_error("Unable to open RTL-SDR device");
        return EXIT_FAILURE;
    }

    rx_device_ready = 0;
    rx_demod_ready = 0;
    rx_lpf_ready = 0;
    rx_resample_ready = 0;

    log_debug("Initializing mutex");
    result = pthread_mutex_init(&rx_ready_mutex, NULL);
    if (result != 0) {
        log_error("Error initializing mutex: %d", result);
        return EXIT_FAILURE;
    }

    log_debug("Initializing condition");
    result = pthread_cond_init(&rx_ready_cond, NULL);
    if (result != 0) {
        log_error("Error initializing condition: %d", result);
        return EXIT_FAILURE;
    }

    log_debug("Setting thread attributes");
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

//    log_debug("Starting RX device read thread");
//    pthread_create(&rx_device_thread, &attr, thread_rx_device_read, NULL);
//
//    log_debug("Starting RX demod thread");
//    pthread_create(&rx_demod_thread, &attr, thread_rx_demod, NULL);
//
//    log_debug("Starting RX lpf thread");
//    pthread_create(&rx_lpf_thread, &attr, thread_rx_lpf, NULL);
//
//    log_debug("Starting RX resample thread");
//    pthread_create(&rx_resample_thread, &attr, thread_rx_resample, NULL);

    log_debug("Starting RX network thread");
    pthread_create(&rx_network_thread, &attr, thread_rx_network, NULL);

    sleep_req.tv_sec = 2;
    sleep_req.tv_nsec = 0;

    log_debug("Printing device infos");
    while (keep_running) {
        device_info();
        nanosleep(&sleep_req, &sleep_rem);
    }

    log_debug("Joining threads");
//    pthread_join(rx_device_thread, NULL);
//    pthread_join(rx_demod_thread, NULL);
//    pthread_join(rx_lpf_thread, NULL);
//    pthread_join(rx_resample_thread, NULL);
    pthread_join(rx_network_thread, NULL);

    log_debug("Freeing buffers");
    circbuf_free(buffer_samples);
    circbuf_free(buffer_demod);
    circbuf_free(buffer_filtered);

    log_debug("Destroying mutex");
    pthread_mutex_destroy(&rx_ready_mutex);

    log_debug("Destroying cond");
    pthread_cond_destroy(&rx_ready_cond);

    return EXIT_SUCCESS;
}

int main_program_mode_info() {
    device_list();

    return EXIT_SUCCESS;
}

void main_stop() {
    keep_running = 0;
}

void main_program_mode_rx_wait_init() {
    log_debug("Locking mutex");
    pthread_mutex_lock(&rx_ready_mutex);

    if (rx_device_ready == 0
        || rx_demod_ready == 0
        || rx_resample_ready == 0
        || rx_lpf_ready == 0)
        pthread_cond_wait(&rx_ready_cond, &rx_ready_mutex);

    log_debug("Unlocking mutex");
    pthread_mutex_unlock(&rx_ready_mutex);

    pthread_cond_broadcast(&rx_ready_cond);
}

void *thread_rx_device_read(void *data) {
    uint8_t *input_buffer;
    double complex *samples;

    size_t rtlsdr_buffer_size;
    int bytes;
    int result;
    struct timespec ts;

    log_info("Thread start");

    rtlsdr_buffer_size = conf->rtlsdr_samples * 2;

    log_debug("Allocating buffers");
    input_buffer = (uint8_t *) calloc(rtlsdr_buffer_size, sizeof(uint8_t));
    samples = (double complex *) calloc(conf->rtlsdr_samples, sizeof(double complex));

    log_debug("Opening RTL-SDR device");
    result = device_open();
    if (result == EXIT_FAILURE) {
        log_error("Unable to open RTL-SDR device");
        main_stop();
        return (void *) EXIT_FAILURE;
    }

    log_debug("Waiting for other threads to init");
    rx_device_ready = 1;
    main_program_mode_rx_wait_init();

    log_debug("Starting read loop");
    while (keep_running) {
        result = rtlsdr_read_sync(device, input_buffer, rtlsdr_buffer_size, &bytes);
        if (result != 0) {
            log_error("Error reading data from RTL-SDR device: %d", result);
            break;
        }

        log_trace("Read %zu bytes", bytes);

        timespec_get(&ts, TIME_UTC);

        log_trace("Converting to complex samples");
        device_buffer_to_samples(input_buffer, samples, rtlsdr_buffer_size);

        log_trace("Put samples in circbuf");
        result = circbuf_put(buffer_samples, &ts, samples, conf->rtlsdr_samples);
        if (result != EXIT_SUCCESS) {
            log_error("Unable to put data to circbuf");
            break;
        }
    }

    log_debug("Closing RTL-SDR device");
    device_close();

    free(input_buffer);

    log_info("Thread end");

    main_stop();

    return (void *) EXIT_SUCCESS;
}

void *thread_rx_demod(void *data) {
    double complex *samples;
    int8_t *output_buffer;

    double complex product;
    double complex prev_sample;

    double value;
    int8_t elem;
    int j;
    struct timespec ts;
    int result;

    double rms;

    log_info("Thread start");

    log_debug("Allocating buffers");
    samples = (double complex *) calloc(conf->rtlsdr_samples, sizeof(double complex));
    output_buffer = (int8_t *) calloc(conf->rtlsdr_samples, sizeof(int8_t));

    prev_sample = 0 + 0 * I;

    log_debug("Waiting for other threads to init");
    rx_demod_ready = 1;
    main_program_mode_rx_wait_init();

    log_debug("Starting demod loop");
    while (keep_running) {
        log_trace("Reading %zu samples", conf->rtlsdr_samples);
        result = circbuf_get(buffer_samples, &ts, samples, conf->rtlsdr_samples);
        if (result != EXIT_SUCCESS) {
            log_error("Unable to get data from circbuf");
            break;
        }

        log_trace("Computing RMS");
        rms = dsp_complex_rms(samples, conf->rtlsdr_samples);

        ui_message("RMS: %.02f\n", rms);

        log_trace("Demodulating");
        for (j = 0; j < conf->rtlsdr_samples; j++) {
            switch (conf->modulation) {
                case MOD_TYPE_FM:
                    product = samples[j] * conj(prev_sample);
                    value = atan2(cimag(product), creal(product)) / M_PI;
                    break;

                case MOD_TYPE_AM:
                    value = cabs(samples[j]);
                    break;

                default:
                    value = 0;
            }

            elem = (int8_t) (value * 127);
            output_buffer[j] = elem;

            prev_sample = samples[j];
        }

        log_trace("Removing DC offset");
        dsp_remove_dc_offset(output_buffer, conf->rtlsdr_samples);

        result = circbuf_put(buffer_demod, &ts, output_buffer, conf->rtlsdr_samples);
        if (result != EXIT_SUCCESS) {
            log_error("Unable to put data to circbuf");
            break;
        }
    }

    free(samples);
    free(output_buffer);

    log_info("Thread end");

    main_stop();

    return (void *) EXIT_SUCCESS;
}

void *thread_rx_lpf(void *data) {
    int8_t *input_buffer;
    int8_t *output_buffer;

    fir_ctx *lpf_ctx;
    int result;
    struct timespec ts;

    log_info("Thread start");

    log_debug("Allocating input buffer");
    input_buffer = (int8_t *) calloc(conf->rtlsdr_samples, sizeof(int8_t));
    if (input_buffer == NULL) {
        log_error("Unable to allocate input buffer");
        main_stop();
        return (void *) EXIT_FAILURE;
    }

    log_debug("Allocating output buffer");
    output_buffer = (int8_t *) calloc(conf->rtlsdr_samples, sizeof(int8_t));
    if (output_buffer == NULL) {
        log_error("Unable to allocate resample context");
        free(input_buffer);
        main_stop();
        return (void *) EXIT_FAILURE;
    }

    log_debug("Initializing FIR low-pass filter context");
    lpf_ctx = fir_init_lpf1();
    if (lpf_ctx == NULL) {
        log_error("Unable to allocate FIR low-pass filter context");
        main_stop();
        return (void *) EXIT_FAILURE;
    }

    log_debug("Waiting for other threads to init");
    rx_lpf_ready = 1;
    main_program_mode_rx_wait_init();

    log_debug("Starting FIR low-pass filter loop");
    while (keep_running) {
        log_trace("Reading %zu bytes", conf->rtlsdr_samples);
        result = circbuf_get(buffer_demod, &ts, input_buffer, conf->rtlsdr_samples);
        if (result != EXIT_SUCCESS) {
            log_error("Unable to get data from circbuf");
            break;
        }

        log_trace("Filtering");
        fir_convolve(lpf_ctx, output_buffer, input_buffer, conf->rtlsdr_samples);
//        memcpy(output_buffer, input_buffer, conf->rtlsdr_samples);

        log_trace("Output %zu bytes", conf->rtlsdr_samples);
        result = circbuf_put(buffer_filtered, &ts, output_buffer, conf->rtlsdr_samples);
        if (result != EXIT_SUCCESS) {
            log_error("Unable to put data to circbuf");
            break;
        }
    }

    log_debug("Freeing resample context");
    fir_free(lpf_ctx);

    log_debug("Freeing buffers");
    free(input_buffer);
    free(output_buffer);

    log_info("Thread end");

    main_stop();

    return (void *) EXIT_SUCCESS;
}

void *thread_rx_resample(void *data) {
    size_t audio_num;
    int8_t *input_buffer;
    int8_t *output_buffer;
    double complex *fft_buffer;
    resample_ctx *res_ctx;
    int result;
    struct timespec ts;

    log_info("Thread start");

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
    input_buffer = (int8_t *) calloc(conf->rtlsdr_samples, sizeof(int8_t));

    if (input_buffer == NULL) {
        log_error("Unable to allocate input buffer");
        main_stop();
        return (void *) EXIT_FAILURE;
    }

    fft_buffer = (double complex *) calloc(conf->rtlsdr_samples, sizeof(double complex));
    if (fft_buffer == NULL) {
        log_error("Unable to allocate input buffer");
        free(input_buffer);
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

    log_debug("Waiting for other threads to init");
    rx_resample_ready = 1;
    main_program_mode_rx_wait_init();

    log_debug("Starting resample loop");
    while (keep_running) {
        log_trace("Reading %zu bytes", conf->rtlsdr_samples);
        result = circbuf_get(buffer_filtered, &ts, input_buffer, conf->rtlsdr_samples);
        if (result != EXIT_SUCCESS) {
            log_error("Unable to get data from circbuf");
            break;
        }

        log_trace("Resampling %zu bytes in %zu bytes", conf->rtlsdr_samples, audio_num);
        resample_do(res_ctx, input_buffer, conf->rtlsdr_samples, output_buffer, audio_num);

        log_trace("Applying limiter");
        agc_limiter(output_buffer, audio_num);

        log_trace("Output %zu bytes", audio_num);
        fwrite(output_buffer, sizeof(int8_t), audio_num, stdout);
    }

    log_debug("Freeing resample context");
    resample_free(res_ctx);

    log_debug("Freeing buffers");
    free(input_buffer);
    free(output_buffer);

    log_info("Thread end");

    main_stop();

    return (void *) EXIT_SUCCESS;
}

void *thread_rx_network(void *data) {
    network_ctx *ctx;
    int result;

    log_info("Thread start");

    log_debug("Initializing network context");
    ctx = network_init(conf->network_server, conf->network_port);
    if (ctx == NULL) {
        log_error("Unable to allocate network context");
        main_stop();
        return (void *) EXIT_FAILURE;
    }

    result = network_socket_open(ctx);
    if (result != EXIT_SUCCESS) {
        log_error("Unable to open socket");
        main_stop();
        return (void *) EXIT_FAILURE;
    }

    log_info("Thread end");

    main_stop();

    return (void *) EXIT_SUCCESS;
}
