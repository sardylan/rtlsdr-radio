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

#include "main_rx2.h"
#include "main.h"
#include "cfg.h"
#include "log.h"
#include "ui.h"
#include "device.h"
#include "circbuf2.h"
#include "fir.h"
#include "fft.h"
#include "resample.h"
#include "audio.h"

extern volatile int keep_running;
extern cfg *conf;

#ifdef MAIN_RX2_ENABLE_THREAD_READ
pthread_t rx2_read_thread;
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_SAMPLE
pthread_t rx2_sample_thread;
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_DEMOD
pthread_t rx2_demod_thread;
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_LPF
pthread_t rx2_lpf_thread;
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_RESAMPLE
pthread_t rx2_resample_thread;
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_MONITOR
pthread_t rx2_monitor_thread;
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_CODEC
pthread_t rx2_codec_thread;
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_NETWORK
pthread_t rx2_network_thread;
#endif

pthread_mutex_t rx2_ready_mutex;
pthread_cond_t rx2_ready_cond;

int rx2_read_ready;
int rx2_sample_ready;
int rx2_demod_ready;
int rx2_lpf_ready;
int rx2_resample_ready;
int rx2_monitor_ready;
int rx2_codec_ready;
int rx2_network_ready;

rtlsdr_dev_t *rx2_device;

circbuf2_ctx *buf_iq;
circbuf2_ctx *buf_sample;
circbuf2_ctx *buf_demod;
circbuf2_ctx *buf_filtered;
circbuf2_ctx *buf_pcm;

int main_rx2() {
    int result;
    pthread_attr_t attr;
    struct timespec sleep_req;
    struct timespec sleep_rem;
//    size_t pcm_size;

    char datetime[27];
    struct tm *timeinfo;
    struct timespec ts;

    int thread_result;

    log_info("Main program RX 2 mode");

    buf_iq = NULL;
    buf_sample = NULL;
    buf_demod = NULL;
    buf_filtered = NULL;
    buf_pcm = NULL;

//    pcm_size = (size_t) ((FP_FLOAT) conf->rtlsdr_device_sample_rate / (FP_FLOAT) conf->audio_sample_rate);

    log_debug("Initializing IQ Circular Buffer");
    buf_iq = circbuf2_init("iq", sizeof(uint8_t) * 2, MAIN_RX2_PAYLOAD_SIZE, MAIN_RX2_BUFFERS_SIZE);
    if (buf_iq == NULL) {
        log_error("Unable to allocate IQ Circular Buffer");
        main_rx2_end();
        return EXIT_FAILURE;
    }

    log_debug("Initializing Samples Circular Buffer");
    buf_sample = circbuf2_init("sample", sizeof(FP_FLOAT complex), MAIN_RX2_PAYLOAD_SIZE, MAIN_RX2_BUFFERS_SIZE);
    if (buf_sample == NULL) {
        log_error("Unable to allocate Samples Circular Buffer");
        main_rx2_end();
        return EXIT_FAILURE;
    }

    log_debug("Initializing Demod Circular Buffer");
    buf_demod = circbuf2_init("sample", sizeof(FP_FLOAT), MAIN_RX2_PAYLOAD_SIZE, MAIN_RX2_BUFFERS_SIZE);
    if (buf_demod == NULL) {
        log_error("Unable to allocate Demod Circular Buffer");
        main_rx2_end();
        return EXIT_FAILURE;
    }

    log_debug("Initializing Filtered Circular Buffer");
    buf_filtered = circbuf2_init("sample", sizeof(FP_FLOAT), MAIN_RX2_PAYLOAD_SIZE, MAIN_RX2_BUFFERS_SIZE);
    if (buf_filtered == NULL) {
        log_error("Unable to allocate Filtered Circular Buffer");
        main_rx2_end();
        return EXIT_FAILURE;
    }

    log_debug("Initializing PCM Circular Buffer");
    buf_pcm = circbuf2_init("sample", sizeof(int16_t), MAIN_RX2_PAYLOAD_SIZE, MAIN_RX2_BUFFERS_SIZE);
    if (buf_pcm == NULL) {
        log_error("Unable to allocate PCM Circular Buffer");
        main_rx2_end();
        return EXIT_FAILURE;
    }

#ifdef MAIN_RX2_ENABLE_THREAD_READ
    rx2_read_ready = 0;
#else
    rx2_read_ready = 1;
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_SAMPLE
    rx2_sample_ready = 0;
#else
    rx2_sample_ready = 1;
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_DEMOD
    rx2_demod_ready = 0;
#else
    rx2_demod_ready = 1;
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_LPF
    rx2_lpf_ready = 0;
#else
    rx2_lpf_ready = 1;
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_RESAMPLE
    rx2_resample_ready = 0;
#else
    rx2_resample_ready = 1;
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_MONITOR
    rx2_monitor_ready = 0;
#else
    rx2_monitor_ready = 1;
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_CODEC
    rx2_codec_ready = 0;
#else
    rx2_codec_ready = 1;
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_NETWORK
    rx2_network_ready = 0;
#else
    rx2_network_ready = 1;
#endif

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

    log_debug("Opening RTL-SDR device");
    result = device_open(&rx2_device, conf->rtlsdr_device_id);
    if (result == EXIT_FAILURE) {
        log_error("Unable to open RTL-SDR device");
        main_rx2_end();
        return EXIT_FAILURE;
    }

    log_debug("Setting RTL-SDR device params");
    result = device_set_params(
            rx2_device,
            conf->rtlsdr_device_sample_rate,
            conf->rtlsdr_device_freq_correction,
            conf->rtlsdr_device_tuner_gain_mode,
            conf->rtlsdr_device_tuner_gain,
            conf->rtlsdr_device_agc_mode
    );
    if (result == EXIT_FAILURE) {
        log_error("Unable to set RTL-SDR device params");
        main_rx2_end();
        return EXIT_FAILURE;
    }

    log_debug("Setting thread attributes");
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

#ifdef MAIN_RX2_ENABLE_THREAD_READ
    log_debug("Starting RX 2 read thread");
    pthread_create(&rx2_read_thread, &attr, thread_rx2_read, NULL);
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_SAMPLE
    log_debug("Starting RX 2 sample thread");
    pthread_create(&rx2_sample_thread, &attr, thread_rx2_sample, NULL);
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_DEMOD
    log_debug("Starting RX 2 demod thread");
    pthread_create(&rx2_demod_thread, &attr, thread_rx2_demod, NULL);
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_LPF
    log_debug("Starting RX 2 lpf thread");
    pthread_create(&rx2_lpf_thread, &attr, thread_rx2_lpf, NULL);
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_RESAMPLE
    log_debug("Starting RX 2 resample thread");
    pthread_create(&rx2_resample_thread, &attr, thread_rx2_resample, NULL);
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_MONITOR
    log_debug("Starting RX 2 monitor thread");
    pthread_create(&rx2_monitor_thread, &attr, thread_rx2_monitor, NULL);
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_CODEC
    log_debug("Starting RX 2 codec thread");
    pthread_create(&rx2_codec_thread, &attr, thread_rx2_codec, NULL);
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_NETWORK
    log_debug("Starting RX 2 network thread");
    pthread_create(&rx2_network_thread, &attr, thread_rx2_network, NULL);
#endif

    log_debug("Waiting for other threads to startup");
    main_rx2_wait_init();

    log_debug("Setting RTL-SDR device frequency");
    result = device_set_frequency(rx2_device, conf->rtlsdr_device_center_freq);
    if (result == EXIT_FAILURE) {
        log_error("Unable to set RTL-SDR device frequency");
        main_rx2_end();
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

        circbuf2_status(buf_iq);
        circbuf2_status(buf_sample);

        nanosleep(&sleep_req, &sleep_rem);
    }

    log_debug("Joining threads");

    result = EXIT_SUCCESS;

#ifdef MAIN_RX2_ENABLE_THREAD_READ
    log_debug("Joining RX 2 read thread");
    pthread_join(rx2_read_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_SAMPLE
    log_debug("Joining RX 2 sample thread");
    pthread_join(rx2_sample_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_DEMOD
    log_debug("Joining RX 2 demod thread");
    pthread_join(rx2_demod_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_LPF
    log_debug("Joining RX 2 lpf thread");
    pthread_join(rx2_lpf_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_RESAMPLE
    log_debug("Joining RX 2 resample thread");
    pthread_join(rx2_resample_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_MONITOR
    log_debug("Joining RX 2 monitor thread");
    pthread_join(rx2_monitor_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_CODEC
    log_debug("Joining RX 2 codec thread");
    pthread_join(rx2_codec_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_NETWORK
    log_debug("Joining RX 2 network thread");
    pthread_join(rx2_network_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;
#endif

    main_rx2_end();

    return result;
}

void main_rx2_end() {
    log_info("Main program RX2 mode ending");

    log_debug("Closing RTL-SDR device");
    device_close(rx2_device);

    log_debug("Freeing IQ Buffer");
    circbuf2_free(buf_iq);

    log_debug("Freeing Sample Buffer");
    circbuf2_free(buf_sample);

    log_debug("Freeing Demod Buffer");
    circbuf2_free(buf_demod);

    log_debug("Freeing Filtered Buffer");
    circbuf2_free(buf_filtered);

    log_debug("Freeing PCM Buffer");
    circbuf2_free(buf_pcm);

    log_debug("Destroying mutex");
    pthread_mutex_destroy(&rx2_ready_mutex);

    log_debug("Destroying cond");
    pthread_cond_destroy(&rx2_ready_cond);
}

void main_rx2_wait_init() {
    log_debug("Locking mutex");
    pthread_mutex_lock(&rx2_ready_mutex);

    while (rx2_read_ready == 0
           || rx2_sample_ready == 0
           || rx2_demod_ready == 0
           || rx2_lpf_ready == 0
           || rx2_resample_ready == 0
           || rx2_monitor_ready == 0
           || rx2_codec_ready == 0
           || rx2_network_ready == 0)
        pthread_cond_wait(&rx2_ready_cond, &rx2_ready_mutex);

    log_debug("Unlocking mutex");
    pthread_mutex_unlock(&rx2_ready_mutex);

    pthread_cond_broadcast(&rx2_ready_cond);
}

#ifdef MAIN_RX2_ENABLE_THREAD_READ

void *thread_rx2_read() {
    int retval;

    uint8_t *iq_buffer;
    int len;

    int bytes;
    int result;

    log_info("Thread start");

    retval = EXIT_SUCCESS;

    log_debug("Waiting for other threads to init");
    rx2_read_ready = 1;
    main_rx2_wait_init();

    len = MAIN_RX2_PAYLOAD_SIZE * 2;

    log_debug("Starting read loop");
    while (keep_running) {
        iq_buffer = (uint8_t *) circbuf2_head_acquire(buf_iq);
        if (iq_buffer == NULL) {
            log_error("Error acquiring IQ buffer head");
            retval = EXIT_FAILURE;
            break;
        }

        result = rtlsdr_read_sync(rx2_device, (void *) iq_buffer, len, &bytes);
        circbuf2_head_release(buf_iq);
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

#ifdef MAIN_RX2_ENABLE_THREAD_SAMPLE

void *thread_rx2_sample() {
    int retval;

    uint8_t *iq_buffer;
    FP_FLOAT complex *samples_buffer;
    int len;

    log_info("Thread start");

    iq_buffer = NULL;
    samples_buffer = NULL;

    retval = EXIT_SUCCESS;

    log_debug("Waiting for other threads to init");
    rx2_sample_ready = 1;
    main_rx2_wait_init();

    len = MAIN_RX2_PAYLOAD_SIZE * 2;

    log_debug("Starting read loop");
    while (keep_running) {
        iq_buffer = (uint8_t *) circbuf2_tail_acquire(buf_iq);
        if (iq_buffer == NULL) {
            log_error("Error acquiring IQ buffer tail");
            retval = EXIT_FAILURE;
            break;
        }

        samples_buffer = (FP_FLOAT complex *) circbuf2_head_acquire(buf_sample);
        if (samples_buffer == NULL) {
            log_error("Error acquiring samples buffer head");
            retval = EXIT_FAILURE;
            break;
        }

        log_trace("Converting IQ to complex samples");
        device_buffer_to_samples(iq_buffer, samples_buffer, len);

        circbuf2_tail_release(buf_iq);
        circbuf2_head_release(buf_sample);
    }

    log_info("Thread end");

    main_stop();

    pthread_exit(&retval);
}

#endif

#ifdef MAIN_RX2_ENABLE_THREAD_DEMOD
void *thread_rx2_demod() {
    int retval;
    size_t samples_pos;
    FP_FLOAT complex *samples_buffer;

    size_t demod_pos;
    FP_FLOAT *demod_buffer;

    FP_FLOAT complex product;
    FP_FLOAT complex prev_sample;

    FP_FLOAT prod_fm;
    FP_FLOAT prod_am;
    size_t j;

    log_info("Thread start");

    retval = EXIT_SUCCESS;
    prev_sample = 0 + 0 * I;
    prod_fm = (FP_FLOAT) (127 / M_PI);
    prod_am = (FP_FLOAT) ((127 * M_SQRT2) / 2);

    log_debug("Waiting for other threads to init");
    rx2_demod_ready = 1;
    main_rx2_wait_init();

    log_debug("Starting demod loop");
    while (keep_running) {
        samples_pos = greatbuf_samples_tail(greatbuf);
        samples_buffer = greatbuf_item_samples_get(greatbuf, samples_pos);

        demod_pos = greatbuf_demod_head_start(greatbuf);
        demod_buffer = greatbuf_item_demod_get(greatbuf, demod_pos);

        log_trace("Demodulating sample pos %zu to demod pos %zu", samples_pos, demod_pos);
        for (j = 0; j < greatbuf->samples_size; j++) {
            switch (conf->modulation) {
                case MOD_TYPE_FM:
                    product = samples_buffer[j] * conj(prev_sample);
                    demod_buffer[j] = (FP_FLOAT) (atan2(cimag(product), creal(product)) * prod_fm);

                    prev_sample = samples_buffer[j];
                    break;

                case MOD_TYPE_AM:
                    demod_buffer[j] = (FP_FLOAT) (((cabs(samples_buffer[j]) / prod_am) - 1) * 127);
                    break;

                default:
                    demod_buffer[j] = 0;
            }
        }

        greatbuf_demod_head_stop(greatbuf);
    }

    log_info("Thread end");

    pthread_exit(&retval);
}
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_LPF
void *thread_rx2_lpf() {
    int retval;
    size_t ln;

    size_t demod_pos;
    FP_FLOAT *demod_buffer;

    size_t filtered_pos;
    FP_FLOAT *filtered_buffer;

    fft_ctx *fwd_fft_ctx;
    fft_ctx *bck_fft_ctx;

    size_t i;
    size_t half;

    log_info("Thread start");

    retval = EXIT_SUCCESS;
    ln = greatbuf->demod_size;
    half = ln / 2;

    switch (conf->filter) {

        case FILTER_MODE_NONE:
            break;

        case FILTER_MODE_FFT_SW:
            log_debug("Initializing FFT forward context");
            fwd_fft_ctx = fft_init(ln, FFTW_R2HC, FFT_DATA_TYPE_REAL);
            if (fwd_fft_ctx == NULL) {
                log_error("Unable to allocate FFT forward context");
                retval = EXIT_FAILURE;
                pthread_exit(&retval);
            }

            log_debug("Initializing FFT backward context");
            bck_fft_ctx = fft_init(ln, FFTW_HC2R, FFT_DATA_TYPE_REAL);
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
    rx2_lpf_ready = 1;
    main_rx2_wait_init();

    log_debug("Starting lpf loop");
    while (keep_running) {
        demod_pos = greatbuf_demod_tail(greatbuf);
        demod_buffer = greatbuf_item_demod_get(greatbuf, demod_pos);

        filtered_pos = greatbuf_filtered_head_start(greatbuf);
        filtered_buffer = greatbuf_item_filtered_get(greatbuf, filtered_pos);

        log_trace("Filtering");

        switch (conf->filter) {

            case FILTER_MODE_NONE:
                log_trace("Copying data");
                memcpy(demod_buffer, filtered_buffer, ln);
                break;

            case FILTER_MODE_FFT_SW:
                log_debug("Copying input values for forward FFT");
                for (i = 0; i < ln; i++)
                    fwd_fft_ctx->real_input[i] = demod_buffer[i];

                log_trace("Computing forward FFT");
                fft_compute(fwd_fft_ctx);

                log_debug("Copying output values from forward FFT output to input values for backward FFT");
                for (i = 0; i < conf->rtlsdr_samples; i++)
                    bck_fft_ctx->real_input[i] = fwd_fft_ctx->real_output[i];

                log_trace("Adjusting coeffs");
                for (i = 64; i < half; i++) {
                    bck_fft_ctx->real_input[i] = 0;
                    bck_fft_ctx->real_input[conf->rtlsdr_samples - i] = 0;
                }

                log_trace("Computing backward FFT");
                fft_compute(bck_fft_ctx);

                log_debug("Copying output values from backward FFT output");
                for (i = 0; i < conf->rtlsdr_samples; i++)
                    filtered_buffer[i] = bck_fft_ctx->real_output[i] / (FP_FLOAT) ln;

                break;

            default:
                log_error("Not implemented");
                retval = EXIT_FAILURE;
                break;
        }

        greatbuf_demod_head_stop(greatbuf);
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

    pthread_exit(&retval);
}
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_RESAMPLE
void *thread_rx2_resample() {
    int retval;

    resample_ctx *res_ctx;

    size_t filtered_pos;
    FP_FLOAT *filtered_buffer;

    size_t pcm_pos;
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
    rx2_resample_ready = 1;
    main_rx2_wait_init();

    log_debug("Starting read loop");
    while (keep_running) {
        filtered_pos = greatbuf_filtered_tail(greatbuf);
        filtered_buffer = greatbuf_item_filtered_get(greatbuf, filtered_pos);

        pcm_pos = greatbuf_pcm_head_start(greatbuf);
        pcm_buffer = greatbuf_item_pcm_get(greatbuf, pcm_pos);

        log_trace("Resampling %zu bytes in %zu bytes", greatbuf->filtered_size, greatbuf->pcm_size);
        resample_float_to_int16(res_ctx, filtered_buffer, greatbuf->filtered_size, pcm_buffer, greatbuf->pcm_size);

        greatbuf_pcm_head_stop(greatbuf);
    }

    log_info("Thread end");

    pthread_exit(&retval);
}
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_CODEC
void *thread_rx2_monitor() {
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
    rx2_monitor_ready = 1;
    main_rx2_wait_init();

    log_debug("Starting read loop");
    while (keep_running) {
        pcm_pos = greatbuf_pcm_tail(greatbuf);
        pcm_buffer = greatbuf_item_pcm_get(greatbuf, pcm_pos);

        result = audio_play_int16(ctx, pcm_buffer, greatbuf->pcm_size);
        if (result != EXIT_SUCCESS) {
            log_error("Unable to play buffer");
            retval = EXIT_FAILURE;
            break;
        }
    }

    log_debug("Freeing audio context");
    audio_free(ctx);

    log_info("Thread end");

    pthread_exit(&retval);
}
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_MONITOR
void *thread_rx2_codec() {
    int retval;

    size_t pcm_pos;
    int16_t *pcm_buffer;

    log_info("Thread start");

    retval = EXIT_SUCCESS;

    log_debug("Waiting for other threads to init");
    rx2_codec_ready = 1;
    main_rx2_wait_init();

    log_debug("Starting read loop");
    while (keep_running) {
        pcm_pos = greatbuf_pcm_tail(greatbuf);
        pcm_buffer = greatbuf_item_pcm_get(greatbuf, pcm_pos);

        log_trace("PCM Buffer: %d", sizeof(pcm_buffer));
    }

    log_info("Thread end");

    pthread_exit(&retval);
}
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_NETWORK
void *thread_rx2_network() {
    int retval;

    log_info("Thread start");

    retval = EXIT_SUCCESS;

    log_debug("Waiting for other threads to init");
    rx2_network_ready = 1;
    main_rx2_wait_init();

    log_debug("Starting read loop");
    while (keep_running) {
        sleep(1);
    }

    log_info("Thread end");

    pthread_exit(&retval);
}
#endif
