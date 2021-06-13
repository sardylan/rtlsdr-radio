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
#include <rtl-sdr.h>
#include <math.h>
#include <unistd.h>

#include "main_rx.h"
#include "main.h"
#include "cfg.h"
#include "log.h"
#include "ui.h"
#include "device.h"
#include "circbuf.h"
#include "greatbuf.h"
#include "fir.h"
#include "fft.h"
#include "resample.h"
#include "audio.h"

extern volatile int keep_running;
extern cfg *conf;

#ifdef MAIN_RX_ENABLE_THREAD_READ
pthread_t rx_read_thread;
#endif

#ifdef MAIN_RX_ENABLE_THREAD_SAMPLES
pthread_t rx_samples_thread;
#endif

#ifdef MAIN_RX_ENABLE_THREAD_DEMOD
pthread_t rx_demod_thread;
#endif

#ifdef MAIN_RX_ENABLE_THREAD_FILTER
pthread_t rx_filter_thread;
#endif

#ifdef MAIN_RX_ENABLE_THREAD_RESAMPLE
pthread_t rx_resample_thread;
#endif

#ifdef MAIN_RX_ENABLE_THREAD_MONITOR
pthread_t rx_monitor_thread;
#endif

#ifdef MAIN_RX_ENABLE_THREAD_CODEC
pthread_t rx_codec_thread;
#endif

#ifdef MAIN_RX_ENABLE_THREAD_NETWORK
pthread_t rx_network_thread;
#endif

pthread_mutex_t rx_ready_mutex;
pthread_cond_t rx_ready_cond;

int rx_read_ready;
int rx_samples_ready;
int rx_demod_ready;
int rx_filter_ready;
int rx_resample_ready;
int rx_monitor_ready;
int rx_codec_ready;
int rx_network_ready;

rtlsdr_dev_t *rx_device;

greatbuf_ctx *greatbuf;

size_t rx_pcm_size;

int main_rx() {
    int result;
    pthread_attr_t attr;
    struct timespec sleep_req;
    struct timespec sleep_rem;

    char datetime[27];
    struct tm *timeinfo;
    struct timespec ts;

    int thread_result;

    FP_FLOAT sample_pcm_ratio;

    log_info("Main program RX 2 mode");

    greatbuf = NULL;

    sample_pcm_ratio = (FP_FLOAT) conf->rtlsdr_device_sample_rate / (FP_FLOAT) conf->audio_sample_rate;
    rx_pcm_size = (size_t) ((FP_FLOAT) conf->rtlsdr_samples / sample_pcm_ratio);

    log_debug("Initializing Great Buffer");
    greatbuf = greatbuf_init(MAIN_RX_BUFFERS_SIZE, conf->rtlsdr_samples, rx_pcm_size);
    if (greatbuf == NULL) {
        log_error("Unable to allocate Greatbuf");
        main_rx_end();
        return EXIT_FAILURE;
    }

#ifdef MAIN_RX_ENABLE_THREAD_READ
    rx_read_ready = 0;
#else
    rx_read_ready = 1;
#endif

#ifdef MAIN_RX_ENABLE_THREAD_SAMPLES
    rx_samples_ready = 0;
#else
    rx_samples_ready = 1;
#endif

#ifdef MAIN_RX_ENABLE_THREAD_DEMOD
    rx_demod_ready = 0;
#else
    rx_demod_ready = 1;
#endif

#ifdef MAIN_RX_ENABLE_THREAD_FILTER
    rx_filter_ready = 0;
#else
    rx_filter_ready = 1;
#endif

#ifdef MAIN_RX_ENABLE_THREAD_RESAMPLE
    rx_resample_ready = 0;
#else
    rx_resample_ready = 1;
#endif

#ifdef MAIN_RX_ENABLE_THREAD_MONITOR
    rx_monitor_ready = 0;
#else
    rx_monitor_ready = 1;
#endif

#ifdef MAIN_RX_ENABLE_THREAD_CODEC
    rx_codec_ready = 0;
#else
    rx_codec_ready = 1;
#endif

#ifdef MAIN_RX_ENABLE_THREAD_NETWORK
    rx_network_ready = 0;
#else
    rx_network_ready = 1;
#endif

    log_debug("Initializing mutex");
    result = pthread_mutex_init(&rx_ready_mutex, NULL);
    if (result != 0) {
        log_error("Error initializing mutex: %d", result);
        main_rx_end();
        return EXIT_FAILURE;
    }

    log_debug("Initializing condition");
    result = pthread_cond_init(&rx_ready_cond, NULL);
    if (result != 0) {
        log_error("Error initializing condition: %d", result);
        main_rx_end();
        return EXIT_FAILURE;
    }

    log_debug("Opening RTL-SDR device");
    result = device_open(&rx_device, conf->rtlsdr_device_id);
    if (result == EXIT_FAILURE) {
        log_error("Unable to open RTL-SDR device");
        main_rx_end();
        return EXIT_FAILURE;
    }

    log_debug("Setting RTL-SDR device params");
    result = device_set_params(
            rx_device,
            conf->rtlsdr_device_sample_rate,
            conf->rtlsdr_device_freq_correction,
            conf->rtlsdr_device_tuner_gain_mode,
            conf->rtlsdr_device_tuner_gain,
            conf->rtlsdr_device_agc_mode
    );
    if (result == EXIT_FAILURE) {
        log_error("Unable to set RTL-SDR device params");
        main_rx_end();
        return EXIT_FAILURE;
    }

    log_debug("Setting thread attributes");
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

#ifdef MAIN_RX_ENABLE_THREAD_READ
    log_debug("Starting RX 2 read thread");
    pthread_create(&rx_read_thread, &attr, thread_rx_read, NULL);
#endif

#ifdef MAIN_RX_ENABLE_THREAD_SAMPLES
    log_debug("Starting RX 2 sample thread");
    pthread_create(&rx_samples_thread, &attr, thread_rx_samples, NULL);
#endif

#ifdef MAIN_RX_ENABLE_THREAD_DEMOD
    log_debug("Starting RX 2 demod thread");
    pthread_create(&rx_demod_thread, &attr, thread_rx_demod, NULL);
#endif

#ifdef MAIN_RX_ENABLE_THREAD_FILTER
    log_debug("Starting RX 2 filter thread");
    pthread_create(&rx_filter_thread, &attr, thread_rx_filter, NULL);
#endif

#ifdef MAIN_RX_ENABLE_THREAD_RESAMPLE
    log_debug("Starting RX 2 resample thread");
    pthread_create(&rx_resample_thread, &attr, thread_rx_resample, NULL);
#endif

#ifdef MAIN_RX_ENABLE_THREAD_MONITOR
    log_debug("Starting RX 2 monitor thread");
    pthread_create(&rx_monitor_thread, &attr, thread_rx_monitor, NULL);
#endif

#ifdef MAIN_RX_ENABLE_THREAD_CODEC
    log_debug("Starting RX 2 codec thread");
    pthread_create(&rx_codec_thread, &attr, thread_rx_codec, NULL);
#endif

#ifdef MAIN_RX_ENABLE_THREAD_NETWORK
    log_debug("Starting RX 2 network thread");
    pthread_create(&rx_network_thread, &attr, thread_rx_network, NULL);
#endif

    log_debug("Waiting for other threads to startup");
    main_rx_wait_init();

    log_debug("Setting RTL-SDR device frequency");
    result = device_set_frequency(rx_device, conf->rtlsdr_device_center_freq);
    if (result == EXIT_FAILURE) {
        log_error("Unable to set RTL-SDR device frequency");
        main_rx_end();
        return EXIT_FAILURE;
    }

    sleep_req.tv_sec = 1;
    sleep_req.tv_nsec = 0;

    log_debug("Printing device infos");
    while (keep_running) {
        timespec_get(&ts, TIME_UTC);
        timeinfo = localtime(&ts.tv_sec);
        strftime(datetime, 20, "%Y-%m-%d %H:%M:%S", timeinfo);
        sprintf(datetime + 19, ".%06lu", ts.tv_nsec / 1000);

        ui_message("---\n");
        ui_message("UTC: %s\n", datetime);

        greatbuf_circbuf_status(greatbuf, GREATBUF_CIRCBUF_READ);
        greatbuf_circbuf_status(greatbuf, GREATBUF_CIRCBUF_SAMPLES);
        greatbuf_circbuf_status(greatbuf, GREATBUF_CIRCBUF_DEMOD);
        greatbuf_circbuf_status(greatbuf, GREATBUF_CIRCBUF_FILTER);
        greatbuf_circbuf_status(greatbuf, GREATBUF_CIRCBUF_RESAMPLE);
        greatbuf_circbuf_status(greatbuf, GREATBUF_CIRCBUF_CODEC);
        greatbuf_circbuf_status(greatbuf, GREATBUF_CIRCBUF_MONITOR);
        greatbuf_circbuf_status(greatbuf, GREATBUF_CIRCBUF_NETWORK);

        nanosleep(&sleep_req, &sleep_rem);
    }

    log_debug("Joining threads");

    result = EXIT_SUCCESS;

#ifdef MAIN_RX_ENABLE_THREAD_READ
    log_debug("Joining RX 2 read thread");
    pthread_join(rx_read_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;
#endif

#ifdef MAIN_RX_ENABLE_THREAD_SAMPLES
    log_debug("Joining RX 2 sample thread");
    pthread_join(rx_samples_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;
#endif

#ifdef MAIN_RX_ENABLE_THREAD_DEMOD
    log_debug("Joining RX 2 demod thread");
    pthread_join(rx_demod_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;
#endif

#ifdef MAIN_RX_ENABLE_THREAD_FILTER
    log_debug("Joining RX 2 filter thread");
    pthread_join(rx_filter_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;
#endif

#ifdef MAIN_RX_ENABLE_THREAD_RESAMPLE
    log_debug("Joining RX 2 resample thread");
    pthread_join(rx_resample_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;
#endif

#ifdef MAIN_RX_ENABLE_THREAD_MONITOR
    log_debug("Joining RX 2 monitor thread");
    pthread_join(rx_monitor_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;
#endif

#ifdef MAIN_RX_ENABLE_THREAD_CODEC
    log_debug("Joining RX 2 codec thread");
    pthread_join(rx_codec_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;
#endif

#ifdef MAIN_RX_ENABLE_THREAD_NETWORK
    log_debug("Joining RX 2 network thread");
    pthread_join(rx_network_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;
#endif

    main_rx_end();

    return result;
}

void main_rx_end() {
    log_info("Main program RX mode ending");

    log_debug("Closing RTL-SDR device");
    device_close(rx_device);

    log_debug("Freeing Great Buffer");
    greatbuf_free(greatbuf);

    log_debug("Destroying mutex");
    pthread_mutex_destroy(&rx_ready_mutex);

    log_debug("Destroying cond");
    pthread_cond_destroy(&rx_ready_cond);
}

void main_rx_wait_init() {
    log_debug("Locking mutex");
    pthread_mutex_lock(&rx_ready_mutex);

    while (rx_read_ready == 0
           || rx_samples_ready == 0
           || rx_demod_ready == 0
           || rx_filter_ready == 0
           || rx_resample_ready == 0
           || rx_monitor_ready == 0
           || rx_codec_ready == 0
           || rx_network_ready == 0)
        pthread_cond_wait(&rx_ready_cond, &rx_ready_mutex);

    log_debug("Unlocking mutex");
    pthread_mutex_unlock(&rx_ready_mutex);

    pthread_cond_broadcast(&rx_ready_cond);
}

#ifdef MAIN_RX_ENABLE_THREAD_READ

void *thread_rx_read() {
    int retval;

    ssize_t pos;
    uint8_t *iq_buffer;
    int len;

    int bytes;
    int result;

    log_info("Thread start");

    retval = EXIT_SUCCESS;

    log_debug("Waiting for other threads to init");
    rx_read_ready = 1;
    main_rx_wait_init();

    len = (int) conf->rtlsdr_samples * 2;

    log_debug("Starting read loop");
    while (keep_running) {
        pos = greatbuf_circbuf_head_acquire(greatbuf, GREATBUF_CIRCBUF_READ);
        if (pos == -1) {
            log_error("Error acquiring IQ buffer head");
            retval = EXIT_FAILURE;
            break;
        }
        iq_buffer = greatbuf_item_get(greatbuf, pos)->iq;
        result = rtlsdr_read_sync(rx_device, (void *) iq_buffer, len, &bytes);
        greatbuf_circbuf_head_release(greatbuf, GREATBUF_CIRCBUF_READ);
        if (result != 0) {
            log_error("Error %d reading data from RTL-SDR device: %s", result, strerror(result));
            retval = EXIT_FAILURE;
            break;
        }

        log_trace("Read %zu bytes from RTL-SDR", bytes);
    }

    log_info("Thread end");

    main_stop();

    pthread_exit(&retval);
}

#endif

#ifdef MAIN_RX_ENABLE_THREAD_SAMPLES

void *thread_rx_samples() {
    int retval;

    ssize_t pos;
    uint8_t *iq_buffer;
    FP_FLOAT complex *samples_buffer;
    int len;

    log_info("Thread start");

    iq_buffer = NULL;
    samples_buffer = NULL;

    retval = EXIT_SUCCESS;

    log_debug("Waiting for other threads to init");
    rx_samples_ready = 1;
    main_rx_wait_init();

    len = (int) conf->rtlsdr_samples * 2;

    log_debug("Starting read loop");
    while (keep_running) {
        pos = greatbuf_circbuf_tail_acquire(greatbuf, GREATBUF_CIRCBUF_READ);
        if (pos == -1) {
            log_error("Error acquiring IQ buffer tail");
            retval = EXIT_FAILURE;
            break;
        }
        iq_buffer = greatbuf_item_get(greatbuf, pos)->iq;

        pos = greatbuf_circbuf_head_acquire(greatbuf, GREATBUF_CIRCBUF_SAMPLES);
        if (pos == -1) {
            log_error("Error acquiring Samples buffer head");
            retval = EXIT_FAILURE;
            break;
        }
        samples_buffer = greatbuf_item_get(greatbuf, pos)->samples;

        log_trace("Converting IQ to complex samples");
        device_buffer_to_samples(iq_buffer, samples_buffer, len);

        greatbuf_circbuf_tail_release(greatbuf, GREATBUF_CIRCBUF_READ);
        greatbuf_circbuf_head_release(greatbuf, GREATBUF_CIRCBUF_SAMPLES);
    }

    log_info("Thread end");

    main_stop();

    pthread_exit(&retval);
}

#endif

#ifdef MAIN_RX_ENABLE_THREAD_DEMOD

void *thread_rx_demod() {
    int retval;

    ssize_t pos;

    FP_FLOAT complex *samples_buffer;
    FP_FLOAT *demod_buffer;

    FP_FLOAT complex product;
    FP_FLOAT complex prev_sample;

    FP_FLOAT real;
    FP_FLOAT imag;

    size_t j;

    log_info("Thread start");

    retval = EXIT_SUCCESS;
    prev_sample = 0 + 0 * I;

    log_debug("Waiting for other threads to init");
    rx_demod_ready = 1;
    main_rx_wait_init();

    log_debug("Starting demod loop");
    while (keep_running) {
        pos = greatbuf_circbuf_tail_acquire(greatbuf, GREATBUF_CIRCBUF_SAMPLES);
        if (pos == -1) {
            log_error("Error acquiring samples buffer tail");
            retval = EXIT_FAILURE;
            break;
        }
        samples_buffer = greatbuf_item_get(greatbuf, pos)->samples;

        pos = greatbuf_circbuf_head_acquire(greatbuf, GREATBUF_CIRCBUF_DEMOD);
        if (pos == -1) {
            log_error("Error acquiring demod buffer head");
            retval = EXIT_FAILURE;
            break;
        }
        demod_buffer = greatbuf_item_get(greatbuf, pos)->demod;

        log_trace("Demodulating samples");
        for (j = 0; j < conf->rtlsdr_samples; j++) {
            switch (conf->modulation) {
                case MOD_TYPE_FM:
                    product = samples_buffer[j] * conj(prev_sample);

#if FP_FLOAT == float
                    real = crealf(product);
                    imag = cimagf(product);

                    if (real != 0 || imag != 0)
                        demod_buffer[j] = atan2f(imag, real) * (float) M_1_PI;
                    else
                        demod_buffer[j] = 0;
#elif FP_FLOAT == double
                    real = creal(product);
                    imag = cimag(product);

                    if (real != 0 || imag != 0)
                        demod_buffer[j] = atan2(imag, real) * M_1_PI;
                    else
                        demod_buffer[j] = 0;
#elif FP_FLOAT == long double
                    real = creall(product);
                    imag = cimagl(product);

                    if (real != 0 || imag != 0)
                        demod_buffer[j] = atan2l(imag, real) * M_1_PI;
                    else
                        demod_buffer[j] = 0;
#else
                    demod_buffer[j] = 0;
#endif
                    prev_sample = samples_buffer[j];
                    break;

                case MOD_TYPE_AM:

#if FP_FLOAT == float
                    demod_buffer[j] = cabsf(samples_buffer[j]) / (float) M_SQRT2;
#elif FP_FLOAT == double
                    demod_buffer[j] = cabs(samples_buffer[j]) / M_SQRT2;
#elif FP_FLOAT == long double
                    demod_buffer[j] = cabsl(samples_buffer[j]) / M_SQRT2;
#else
                    demod_buffer[j] = 0;
#endif
                    break;

                default:
                    demod_buffer[j] = 0;
            }
        }

        greatbuf_circbuf_tail_release(greatbuf, GREATBUF_CIRCBUF_SAMPLES);
        greatbuf_circbuf_head_release(greatbuf, GREATBUF_CIRCBUF_DEMOD);
    }

    log_info("Thread end");

    main_stop();

    pthread_exit(&retval);
}

#endif

#ifdef MAIN_RX_ENABLE_THREAD_FILTER

void *thread_rx_filter() {
    int retval;

    ssize_t pos;

    FP_FLOAT *demod_buffer;
    FP_FLOAT *filtered_buffer;

    fft_ctx *fwd_fft_ctx;
    fft_ctx *bck_fft_ctx;

    size_t i;
    size_t half;
    size_t coeff_truncate;

    log_info("Thread start");

    retval = EXIT_SUCCESS;
    half = conf->rtlsdr_samples / 2;
    coeff_truncate = (conf->audio_sample_rate * conf->rtlsdr_samples) / conf->rtlsdr_device_sample_rate;

    switch (conf->filter) {

        case FILTER_MODE_NONE:
            break;

        case FILTER_MODE_FFT_SW:
            log_debug("Initializing FFT forward context");
            fwd_fft_ctx = fft_init(conf->rtlsdr_samples, FFTW_R2HC, FFT_DATA_TYPE_REAL);
            if (fwd_fft_ctx == NULL) {
                log_error("Unable to allocate FFT forward context");
                retval = EXIT_FAILURE;
                pthread_exit(&retval);
            }

            log_debug("Initializing FFT backward context");
            bck_fft_ctx = fft_init(conf->rtlsdr_samples, FFTW_HC2R, FFT_DATA_TYPE_REAL);
            if (bck_fft_ctx == NULL) {
                log_error("Unable to allocate FFT backward context");
                fft_free(fwd_fft_ctx);
                retval = EXIT_FAILURE;
                pthread_exit(&retval);
            }

            break;

        default:
            log_error("Not implemented");
            retval = EXIT_FAILURE;
            pthread_exit(&retval);
    }

    log_debug("Waiting for other threads to init");
    rx_filter_ready = 1;
    main_rx_wait_init();

    log_debug("Starting filter loop");
    while (keep_running) {
        pos = greatbuf_circbuf_tail_acquire(greatbuf, GREATBUF_CIRCBUF_DEMOD);
        if (pos == -1) {
            log_error("Error acquiring demod buffer tail");
            retval = EXIT_FAILURE;
            break;
        }
        demod_buffer = greatbuf_item_get(greatbuf, pos)->demod;

        pos = greatbuf_circbuf_head_acquire(greatbuf, GREATBUF_CIRCBUF_FILTER);
        if (pos == -1) {
            log_error("Error acquiring filtered buffer head");
            retval = EXIT_FAILURE;
            break;
        }
        filtered_buffer = greatbuf_item_get(greatbuf, pos)->filtered;

        log_trace("Filtering");

        switch (conf->filter) {

            case FILTER_MODE_NONE:
                log_trace("Copying data");
                for (i = 0; i < conf->rtlsdr_samples; i++)
                    filtered_buffer[i] = demod_buffer[i];
                break;

            case FILTER_MODE_FFT_SW:
                log_debug("Copying input values for forward FFT");
                for (i = 0; i < conf->rtlsdr_samples; i++)
                    fwd_fft_ctx->real_input[i] = demod_buffer[i];

                log_trace("Computing forward FFT");
                fft_compute(fwd_fft_ctx);

                log_debug("Copying output values from forward FFT output to input values for backward FFT");
                for (i = 0; i < conf->rtlsdr_samples; i++)
                    bck_fft_ctx->real_input[i] = fwd_fft_ctx->real_output[i];

                log_trace("Adjusting coeffs");
                for (i = coeff_truncate; i < half; i++) {
                    bck_fft_ctx->real_input[i] = 0;
                    bck_fft_ctx->real_input[conf->rtlsdr_samples - i] = 0;
                }

                log_trace("Computing backward FFT");
                fft_compute(bck_fft_ctx);

                log_debug("Copying output values from backward FFT output");
                for (i = 0; i < conf->rtlsdr_samples; i++)
                    filtered_buffer[i] = bck_fft_ctx->real_output[i] / (FP_FLOAT) conf->rtlsdr_samples;

                break;

            default:
                log_error("Not implemented");
                retval = EXIT_FAILURE;
                break;
        }

        greatbuf_circbuf_tail_release(greatbuf, GREATBUF_CIRCBUF_DEMOD);
        greatbuf_circbuf_head_release(greatbuf, GREATBUF_CIRCBUF_FILTER);
    }

    switch (conf->filter) {

        case FILTER_MODE_NONE:
            break;

        case FILTER_MODE_FFT_SW:
            log_debug("Freeing FFT forward context");
            fft_free(fwd_fft_ctx);

            log_debug("Freeing FFT backward context");
            fft_free(bck_fft_ctx);
            break;

        default:
            log_error("Not implemented");
            retval = EXIT_FAILURE;
            break;
    }

    log_info("Thread end");

    main_stop();

    pthread_exit(&retval);
}

#endif

#ifdef MAIN_RX_ENABLE_THREAD_RESAMPLE

void *thread_rx_resample() {
    int retval;

    ssize_t pos;

    resample_ctx *res_ctx;

    FP_FLOAT *filtered_buffer;
    int16_t *pcm_buffer;

    log_info("Thread start");

    retval = EXIT_SUCCESS;

    log_debug("Initializing resample context");
    res_ctx = resample_init(conf->rtlsdr_device_sample_rate, conf->audio_sample_rate);
    if (res_ctx == NULL) {
        log_error("Unable to allocate resample context");
        retval = EXIT_FAILURE;
        pthread_exit(&retval);
    }

    log_debug("Waiting for other threads to init");
    rx_resample_ready = 1;
    main_rx_wait_init();

    log_debug("Starting read loop");
    while (keep_running) {
        pos = greatbuf_circbuf_tail_acquire(greatbuf, GREATBUF_CIRCBUF_FILTER);
        if (pos == -1) {
            log_error("Error acquiring filtered buffer tail");
            retval = EXIT_FAILURE;
            break;
        }
        filtered_buffer = greatbuf_item_get(greatbuf, pos)->filtered;

        pos = greatbuf_circbuf_head_acquire(greatbuf, GREATBUF_CIRCBUF_RESAMPLE);
        if (pos == -1) {
            log_error("Error acquiring pcm buffer head");
            retval = EXIT_FAILURE;
            break;
        }
        pcm_buffer = greatbuf_item_get(greatbuf, pos)->pcm;

        log_trace("Resampling");
        resample_float_to_int16(res_ctx, filtered_buffer, conf->rtlsdr_samples, pcm_buffer, rx_pcm_size);

        greatbuf_circbuf_tail_release(greatbuf, GREATBUF_CIRCBUF_FILTER);
        greatbuf_circbuf_head_release(greatbuf, GREATBUF_CIRCBUF_RESAMPLE);
    }

    resample_free(res_ctx);

    log_info("Thread end");

    main_stop();

    pthread_exit(&retval);
}

#endif

#ifdef MAIN_RX_ENABLE_THREAD_CODEC
void *thread_rx_monitor() {
    int retval;

    int result;

    size_t pcm_pos;
    int16_t *pcm_buffer;

    audio_ctx *ctx;

    log_info("Thread start");

    retval = EXIT_SUCCESS;

    log_debug("Initializing audio context");
    ctx = audio_init(conf->audio_monitor_device, conf->audio_sample_rate, 1, SND_PCM_FORMAT_S16);
    if (ctx == NULL) {
        log_error("Unable to allocate codec context");
        retval = EXIT_FAILURE;
        pthread_exit(&retval);
    }

    log_debug("Waiting for other threads to init");
    rx_monitor_ready = 1;
    main_rx_wait_init();

    log_debug("Starting read loop");
    while (keep_running) {
        pcm_pos = greatbuf_pcm_tail(greatbuf);
        pcm_buffer = greatbuf_item_pcm_get(greatbuf, pcm_pos);

        result = audio_play_int16(ctx, pcm_buffer, greatbuf->rx_pcm_size);
        if (result != EXIT_SUCCESS) {
            log_error("Unable to play buffer");
            retval = EXIT_FAILURE;
            break;
        }
    }

    log_debug("Freeing audio context");
    audio_free(ctx);

    log_info("Thread end");

    main_stop();

    pthread_exit(&retval);
}
#endif

#ifdef MAIN_RX_ENABLE_THREAD_MONITOR

void *thread_rx_monitor() {
    int retval;

    ssize_t pos;

    audio_ctx *ctx;
    int16_t *pcm_buffer;

    int result;

    log_info("Thread start");

    retval = EXIT_SUCCESS;

    log_debug("Initializing audio context");
    ctx = audio_init(conf->audio_monitor_device, conf->audio_sample_rate, 1, SND_PCM_FORMAT_S16_LE);
    if (ctx == NULL) {
        log_error("Unable to allocate codec context");
        retval = EXIT_FAILURE;
        pthread_exit(&retval);
    }

    log_debug("Waiting for other threads to init");
    rx_monitor_ready = 1;
    main_rx_wait_init();

    log_debug("Starting read loop");
    while (keep_running) {
        pos = greatbuf_circbuf_tail_acquire(greatbuf, GREATBUF_CIRCBUF_RESAMPLE);
        if (pos == -1) {
            log_error("Error acquiring filtered buffer tail");
            retval = EXIT_FAILURE;
            break;
        }
        pcm_buffer = greatbuf_item_get(greatbuf, pos)->pcm;

        result = audio_play_int16(ctx, pcm_buffer, rx_pcm_size);
        if (result != EXIT_SUCCESS) {
            log_error("Unable to play buffer");
            retval = EXIT_FAILURE;
            break;
        }

        greatbuf_circbuf_tail_release(greatbuf, GREATBUF_CIRCBUF_RESAMPLE);
    }

    log_debug("Freeing audio context");
    audio_free(ctx);

    log_info("Thread end");

    main_stop();

    pthread_exit(&retval);
}

#endif

#ifdef MAIN_RX_ENABLE_THREAD_NETWORK
void *thread_rx_network() {
    int retval;

    log_info("Thread start");

    retval = EXIT_SUCCESS;

    log_debug("Waiting for other threads to init");
    rx_network_ready = 1;
    main_rx_wait_init();

    log_debug("Starting read loop");
    while (keep_running) {
        sleep(1);
    }

    log_info("Thread end");

    main_stop();

    pthread_exit(&retval);
}
#endif
