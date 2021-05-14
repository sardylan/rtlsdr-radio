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


#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>

#include "main_rx2.h"
#include "cfg.h"
#include "log.h"
#include "ui.h"

extern volatile int keep_running;
extern cfg *conf;

pthread_t rx2_read_thread;
pthread_t rx2_demod_thread;
pthread_t rx2_lpf_thread;
pthread_t rx2_resample_thread;
pthread_t rx2_codec_thread;
pthread_t rx2_monitor_thread;
pthread_t rx2_network_thread;

pthread_mutex_t rx2_ready_mutex;
pthread_cond_t rx2_ready_cond;

int rx2_read_ready;
int rx2_demod_ready;
int rx2_lpf_ready;
int rx2_resample_ready;
int rx2_codec_ready;
int rx2_monitor_ready;
int rx2_network_ready;

int main_rx2() {
    int result;
    pthread_attr_t attr;
    struct timespec sleep_req;
    struct timespec sleep_rem;

    char datetime[27];
    struct tm *timeinfo;
    struct timespec ts;

    int thread_result;

    log_info("Main program RX 2 mode");

    rx2_read_ready = 0;
    rx2_demod_ready = 0;
    rx2_lpf_ready = 0;
    rx2_resample_ready = 0;
    rx2_codec_ready = 0;
    rx2_monitor_ready = 0;
    rx2_network_ready = 0;

    log_debug("Initializing mutex");
    result = pthread_mutex_init(&rx2_ready_mutex, NULL);
    if (result != 0) {
        log_error("Error initializing mutex: %d", result);
        main_rx2_end();
        return EXIT_FAILURE;
    }

    log_debug("Initializing condition");
    result = pthread_cond_init(&rx2_ready_cond, NULL);
    if (result != 0) {
        log_error("Error initializing condition: %d", result);
        main_rx2_end();
        return EXIT_FAILURE;
    }

    log_debug("Setting thread attributes");
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    log_debug("Starting RX 2 read thread");
    pthread_create(&rx2_read_thread, &attr, thread_rx2_read, NULL);

    log_debug("Starting RX 2 demod thread");
    pthread_create(&rx2_demod_thread, &attr, thread_rx2_demod, NULL);

    log_debug("Starting RX 2 lpf thread");
    pthread_create(&rx2_lpf_thread, &attr, thread_rx2_lpf, NULL);

    log_debug("Starting RX 2 resample thread");
    pthread_create(&rx2_resample_thread, &attr, thread_rx2_resample, NULL);

    log_debug("Starting RX 2 codec thread");
    pthread_create(&rx2_codec_thread, &attr, thread_rx2_codec, NULL);

    log_debug("Starting RX 2 monitor thread");
    pthread_create(&rx2_monitor_thread, &attr, thread_rx2_monitor, NULL);

    log_debug("Starting RX 2 network thread");
    pthread_create(&rx2_network_thread, &attr, thread_rx2_network, NULL);

    sleep_req.tv_sec = 2;
    sleep_req.tv_nsec = 0;

    log_debug("Printing device infos");
    while (keep_running) {
        timespec_get(&ts, TIME_UTC);
        timeinfo = localtime(&ts.tv_sec);
        strftime(datetime, 20, "%Y-%m-%d %H:%M:%S", timeinfo);
        sprintf(datetime + 19, ".%06lu", ts.tv_nsec / 1000);

        ui_message("---\n");
        ui_message("UTC: %s\n", datetime);

        nanosleep(&sleep_req, &sleep_rem);
    }

    log_debug("Joining threads");

    result = EXIT_SUCCESS;

    log_debug("Joining RX 2 read thread");
    pthread_join(rx2_read_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;

    log_debug("Joining RX 2 demod thread");
    pthread_join(rx2_demod_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;

    log_debug("Joining RX 2 lpf thread");
    pthread_join(rx2_lpf_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;

    log_debug("Joining RX 2 resample thread");
    pthread_join(rx2_resample_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;

    log_debug("Joining RX 2 codec thread");
    pthread_join(rx2_codec_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;

    log_debug("Joining RX 2 monitor thread");
    pthread_join(rx2_monitor_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;

    log_debug("Joining RX 2 network thread");
    pthread_join(rx2_network_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;

    main_rx2_end();

    return result;
}

void main_rx2_end() {
//    int i;

    log_info("Main program RX2 mode ending");

//    log_debug("Closing RTL-SDR device");
//    device_close(device);
//
//    log_debug("Freeing frames");
//    if (frames != NULL) {
//        for (i = 0; i < BUFFER_FRAMES; i++)
//            if (frames[i] != NULL)
//                frame_free(frames[i]);
//
//        free(frames);
//    }
//
//    log_debug("Freeing payloads");
//    if (payloads != NULL) {
//        for (i = 0; i < BUFFER_PAYLOADS; i++)
//            if (payloads[i] != NULL)
//                payload_free(payloads[i]);
//
//        free(payloads);
//    }
//
//    log_debug("Freeing buffers");
//    if (buffer_samples != NULL)
//        circbuf_free(buffer_samples);
//    if (buffer_demod != NULL)
//        circbuf_free(buffer_demod);
//    if (buffer_filtered != NULL)
//        circbuf_free(buffer_filtered);
//    if (buffer_codec != NULL)
//        circbuf_free(buffer_codec);

    log_debug("Destroying mutex");
    pthread_mutex_destroy(&rx2_ready_mutex);

    log_debug("Destroying cond");
    pthread_cond_destroy(&rx2_ready_cond);
}

void main_rx2_wait_init() {
    log_debug("Locking mutex");
    pthread_mutex_lock(&rx2_ready_mutex);

    if (rx2_read_ready == 0
        || rx2_demod_ready == 0
        || rx2_lpf_ready == 0
        || rx2_resample_ready == 0
        || rx2_codec_ready == 0
        || rx2_monitor_ready == 0
        || rx2_network_ready == 0)
        pthread_cond_wait(&rx2_ready_cond, &rx2_ready_mutex);

    log_debug("Unlocking mutex");
    pthread_mutex_unlock(&rx2_ready_mutex);

    pthread_cond_broadcast(&rx2_ready_cond);
}

void *thread_rx2_read() {
    int retval;

    log_info("Thread start");

    retval = EXIT_SUCCESS;

    log_debug("Waiting for other threads to init");
    rx2_read_ready = 1;
    main_rx2_wait_init();

    log_debug("Starting read loop");
    while (keep_running) {
    }

    pthread_exit(&retval);
}

void *thread_rx2_demod() {
    int retval;

    log_info("Thread start");

    retval = EXIT_SUCCESS;

    log_debug("Waiting for other threads to init");
    rx2_demod_ready = 1;
    main_rx2_wait_init();

    log_debug("Starting read loop");
    while (keep_running) {
    }

    pthread_exit(&retval);
}

void *thread_rx2_lpf() {
    int retval;

    log_info("Thread start");

    retval = EXIT_SUCCESS;

    log_info("Thread end");

    log_debug("Waiting for other threads to init");
    rx2_lpf_ready = 1;
    main_rx2_wait_init();

    log_debug("Starting read loop");
    while (keep_running) {
    }

    pthread_exit(&retval);
}

void *thread_rx2_resample() {
    int retval;

    log_info("Thread start");

    retval = EXIT_SUCCESS;

    log_info("Thread end");

    log_debug("Waiting for other threads to init");
    rx2_resample_ready = 1;
    main_rx2_wait_init();

    log_debug("Starting read loop");
    while (keep_running) {
    }

    pthread_exit(&retval);
}

void *thread_rx2_codec() {
    int retval;

    log_info("Thread start");

    retval = EXIT_SUCCESS;

    log_info("Thread end");

    log_debug("Waiting for other threads to init");
    rx2_codec_ready = 1;
    main_rx2_wait_init();

    log_debug("Starting read loop");
    while (keep_running) {
    }

    pthread_exit(&retval);
}

void *thread_rx2_monitor() {
    int retval;

    log_info("Thread start");

    retval = EXIT_SUCCESS;

    log_info("Thread end");

    log_debug("Waiting for other threads to init");
    rx2_monitor_ready = 1;
    main_rx2_wait_init();

    log_debug("Starting read loop");
    while (keep_running) {
    }

    pthread_exit(&retval);
}

void *thread_rx2_network() {
    int retval;

    log_info("Thread start");

    retval = EXIT_SUCCESS;

    log_info("Thread end");

    log_debug("Waiting for other threads to init");
    rx2_network_ready = 1;
    main_rx2_wait_init();

    log_debug("Starting read loop");
    while (keep_running) {
    }

    pthread_exit(&retval);
}
