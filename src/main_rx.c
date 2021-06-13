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
#include <rtl-sdr.h>
#include <complex.h>
#include <math.h>
#include <pthread.h>
#include <string.h>

#include "main_rx.h"
#include "main.h"
#include "ui.h"
#include "cfg.h"
#include "device.h"
#include "log.h"
#include "circbuf.h"
#include "fir.h"
#include "fft.h"
#include "resample.h"
#include "dsp.h"
#include "agc.h"
#include "frame.h"
#include "codec.h"
#include "network.h"
#include "payload.h"
#include "audio.h"

frame **frames;
uint64_t frame_number = 0;

payload **payloads;
uint64_t payload_number = 0;

circbuf_ctx *buffer_samples;
circbuf_ctx *buffer_demod;
circbuf_ctx *buffer_filtered;
circbuf_ctx *buffer_codec;
circbuf_ctx *buffer_monitor;
circbuf_ctx *buffer_network;

pthread_t rx_device_thread;
pthread_t rx_demod_thread;
pthread_t rx_lpf_thread;
pthread_t rx_resample_thread;
pthread_t rx_codec_thread;
pthread_t rx_monitor_thread;
pthread_t rx_network_thread;

pthread_mutex_t rx_ready_mutex;
pthread_cond_t rx_ready_cond;

int rx_device_ready;
int rx_demod_ready;
int rx_lpf_ready;
int rx_resample_ready;
int rx_codec_ready;
int rx_monitor_ready;
int rx_network_ready;

rtlsdr_dev_t *device;

extern volatile int keep_running;
extern cfg *conf;

int main_rx() {
    int result;
    pthread_attr_t attr;
    struct timespec sleep_req;
    struct timespec sleep_rem;
    size_t size_iq;
    size_t size_sample;
    size_t size_pcm;
    size_t i;

    char datetime[27];
    struct tm *timeinfo;
    struct timespec ts;

    int thread_result;

    log_info("Main program RX mode");

    frames = NULL;

    buffer_samples = NULL;
    buffer_demod = NULL;
    buffer_filtered = NULL;
    buffer_codec = NULL;
    buffer_monitor = NULL;

    device = NULL;

    log_debug("Allocating frames buffer");
    frames = (frame **) calloc(BUFFER_FRAMES, sizeof(frame *));
    if (frames == NULL) {
        log_error("Unable to allocate frames buffer");
        main_rx_end();
        return EXIT_FAILURE;
    }

    for (i = 0; i < BUFFER_FRAMES; i++)
        frames[i] = NULL;

    log_debug("Preparing frames buffer");
    size_iq = conf->rtlsdr_samples * 2;
    size_sample = conf->rtlsdr_samples;
    size_pcm = (size_t) ((FP_FLOAT) conf->rtlsdr_device_sample_rate / (FP_FLOAT) conf->audio_sample_rate);

    for (i = 0; i < BUFFER_FRAMES; i++) {
        frames[i] = frame_init(size_iq, size_sample, size_pcm);
        if (frames[i] == NULL) {
            log_error("Unable to initialize frames");
            main_rx_end();
            return EXIT_FAILURE;
        }
    }

    log_debug("Allocating payloads buffer");
    payloads = (payload **) calloc(BUFFER_PAYLOADS, sizeof(payload *));
    if (payloads == NULL) {
        log_error("Unable to allocate payloads buffer");
        main_rx_end();
        return EXIT_FAILURE;
    }

    for (i = 0; i < BUFFER_PAYLOADS; i++)
        payloads[i] = NULL;

    log_debug("Preparing payloads buffer");
    for (i = 0; i < BUFFER_PAYLOADS; i++) {
        payloads[i] = payload_init();
        if (payloads[i] == NULL) {
            log_error("Unable to initialize frames");
            main_rx_end();
            return EXIT_FAILURE;
        }
    }

    log_debug("Init samples circbuf");
    buffer_samples = circbuf_init("samples", sizeof(uint64_t), BUFFER_SAMPLES);
    if (buffer_samples == NULL) {
        log_error("Unable to init IQ circular buffer");
        main_rx_end();
        return EXIT_FAILURE;
    }

    log_debug("Init demod circbuf");
    buffer_demod = circbuf_init("demod", sizeof(uint64_t), BUFFER_DEMOD);
    if (buffer_demod == NULL) {
        log_error("Unable to init demod circular buffer");
        main_rx_end();
        return EXIT_FAILURE;
    }

    log_debug("Init filtered circbuf");
    buffer_filtered = circbuf_init("filtered", sizeof(uint64_t), BUFFER_FILTERED);
    if (buffer_filtered == NULL) {
        log_error("Unable to allocate lpf circular buffer");
        main_rx_end();
        return EXIT_FAILURE;
    }

    log_debug("Init codec circbuf");
    buffer_codec = circbuf_init("codec", sizeof(uint64_t), BUFFER_CODEC);
    if (buffer_codec == NULL) {
        log_error("Unable to allocate codec circular buffer");
        main_rx_end();
        return EXIT_FAILURE;
    }

    log_debug("Init monitor circbuf");
    buffer_monitor = circbuf_init("monitor", sizeof(uint64_t), BUFFER_MONITOR);
    if (buffer_monitor == NULL) {
        log_error("Unable to allocate monitor circular buffer");
        main_rx_end();
        return EXIT_FAILURE;
    }

    log_debug("Init network circbuf");
    buffer_network = circbuf_init("network", sizeof(uint64_t), BUFFER_NETWORK);
    if (buffer_network == NULL) {
        log_error("Unable to allocate network circular buffer");
        main_rx_end();
        return EXIT_FAILURE;
    }

    log_debug("Opening RTL-SDR device");
    result = device_open(&device, conf->rtlsdr_device_id);
    if (result == EXIT_FAILURE) {
        log_error("Unable to open RTL-SDR device");
        main_rx_end();
        return EXIT_FAILURE;
    }

    log_debug("Setting RTL-SDR device params");
    result = device_set_params(
            device,
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

    log_debug("Setting RTL-SDR device frequency");
    result = device_set_frequency(device, conf->rtlsdr_device_center_freq);
    if (result == EXIT_FAILURE) {
        log_error("Unable to set RTL-SDR device frequency");
        main_rx_end();
        return EXIT_FAILURE;
    }

    rx_device_ready = 0;
    rx_demod_ready = 0;
    rx_lpf_ready = 0;
    rx_resample_ready = 0;
    rx_codec_ready = 0;
    rx_monitor_ready = 0;
    rx_network_ready = 0;

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

    log_debug("Setting thread attributes");
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    log_debug("Starting RX device read thread");
    pthread_create(&rx_device_thread, &attr, thread_rx_device_read, NULL);

    log_debug("Starting RX demod thread");
    pthread_create(&rx_demod_thread, &attr, thread_rx_demod, NULL);

    log_debug("Starting RX lpf thread");
    pthread_create(&rx_lpf_thread, &attr, thread_rx_lpf, NULL);

    log_debug("Starting RX resample thread");
    pthread_create(&rx_resample_thread, &attr, thread_rx_resample, NULL);

    log_debug("Starting RX codec thread");
    pthread_create(&rx_codec_thread, &attr, thread_rx_codec, NULL);

    log_debug("Starting RX monitor thread");
    pthread_create(&rx_monitor_thread, &attr, thread_rx_monitor, NULL);

    log_debug("Starting RX network thread");
    pthread_create(&rx_network_thread, &attr, thread_rx_network, NULL);

    log_debug("Waiting for other threads to startup");
    main_rx_wait_init();

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

        device_info(device);

        circbuf_status(buffer_samples);
        circbuf_status(buffer_demod);
        circbuf_status(buffer_filtered);
        circbuf_status(buffer_codec);
        circbuf_status(buffer_monitor);
        circbuf_status(buffer_network);

        nanosleep(&sleep_req, &sleep_rem);
    }

    log_debug("Stopping circula buffers");
    circbuf_stop(buffer_samples);
    circbuf_stop(buffer_demod);
    circbuf_stop(buffer_filtered);
    circbuf_stop(buffer_codec);
    circbuf_stop(buffer_monitor);
    circbuf_stop(buffer_network);

    log_debug("Joining threads");

    result = EXIT_SUCCESS;

    pthread_join(rx_device_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;

    pthread_join(rx_demod_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;

    pthread_join(rx_lpf_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;

    pthread_join(rx_resample_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;

    pthread_join(rx_codec_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;

    pthread_join(rx_monitor_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;

    pthread_join(rx_network_thread, (void **) &thread_result);
    if (thread_result != EXIT_SUCCESS)
        result = EXIT_FAILURE;

    main_rx_end();

    return result;
}

void main_rx_end() {
    int i;

    log_info("Main program RX mode ending");

    log_debug("Closing RTL-SDR device");
    device_close(device);

    log_debug("Freeing frames");
    if (frames != NULL) {
        for (i = 0; i < BUFFER_FRAMES; i++)
            if (frames[i] != NULL)
                frame_free(frames[i]);

        free(frames);
    }

    log_debug("Freeing payloads");
    if (payloads != NULL) {
        for (i = 0; i < BUFFER_PAYLOADS; i++)
            if (payloads[i] != NULL)
                payload_free(payloads[i]);

        free(payloads);
    }

    log_debug("Freeing buffers");
    if (buffer_samples != NULL)
        circbuf_free(buffer_samples);
    if (buffer_demod != NULL)
        circbuf_free(buffer_demod);
    if (buffer_filtered != NULL)
        circbuf_free(buffer_filtered);
    if (buffer_codec != NULL)
        circbuf_free(buffer_codec);

    log_debug("Destroying mutex");
    pthread_mutex_destroy(&rx_ready_mutex);

    log_debug("Destroying cond");
    pthread_cond_destroy(&rx_ready_cond);
}

void main_rx_wait_init() {
    log_debug("Locking mutex");
    pthread_mutex_lock(&rx_ready_mutex);

    if (rx_device_ready == 0
        || rx_demod_ready == 0
        || rx_lpf_ready == 0
        || rx_resample_ready == 0
        || rx_codec_ready == 0
        || rx_monitor_ready == 0
        || rx_network_ready == 0)
        pthread_cond_wait(&rx_ready_cond, &rx_ready_mutex);

    log_debug("Unlocking mutex");
    pthread_mutex_unlock(&rx_ready_mutex);

    pthread_cond_broadcast(&rx_ready_cond);
}

void *thread_rx_device_read() {
    int retval;

    size_t frame_pos;
    frame *fr;

    size_t rtlsdr_buffer_size;
    int bytes;
    int result;

    log_info("Thread start");

    retval = EXIT_SUCCESS;

    rtlsdr_buffer_size = conf->rtlsdr_samples * 2;

    log_debug("Waiting for other threads to init");
    rx_device_ready = 1;
    main_rx_wait_init();

    log_debug("Starting read loop");
    while (keep_running) {
        frame_pos = frame_number % BUFFER_FRAMES;
        log_trace("Using frame %zu");
        fr = frames[frame_pos];

        frame_clear(fr, frame_number);
        frame_number++;

        result = rtlsdr_read_sync(device, fr->iq, rtlsdr_buffer_size, &bytes);
        if (result != 0) {
            log_error("Error reading data from RTL-SDR device: %d", result);
            retval = EXIT_FAILURE;
            break;
        }

        log_trace("Read %zu bytes", bytes);

        log_trace("Converting to complex samples");
        device_buffer_to_samples(fr->iq, fr->samples, rtlsdr_buffer_size);

        log_trace("Put samples in circbuf");
        result = circbuf_put(buffer_samples, &fr->number, 1);
        if (result != EXIT_SUCCESS) {
            log_error("Unable to put data to circbuf");
            retval = EXIT_FAILURE;
            break;
        }
    }

    log_info("Thread end");

    main_stop();

    pthread_exit(&retval);
}

void *thread_rx_demod() {
    int retval;

    uint64_t fr_number;
    size_t fr_pos;
    frame *fr;

    FP_FLOAT complex product;
    FP_FLOAT complex prev_sample;

    FP_FLOAT value;
    FP_FLOAT prod_fm;
    FP_FLOAT prod_am;
    int8_t elem;
    size_t j;
    int result;

    log_info("Thread start");

    retval = EXIT_SUCCESS;

    prev_sample = 0 + 0 * I;

    log_debug("Waiting for other threads to init");
    rx_demod_ready = 1;
    main_rx_wait_init();

    prod_fm = (FP_FLOAT) (127 / M_PI);
    prod_am = (FP_FLOAT) ((127 * M_SQRT2) / 2);

    log_debug("Starting demod loop");
    while (keep_running) {
        log_trace("Reading from circbuf");
        result = circbuf_get(buffer_samples, &fr_number, 1);
        if (result != EXIT_SUCCESS) {
            log_error("Unable to get data from circbuf");
            retval = EXIT_FAILURE;
            break;
        }

        fr_pos = fr_number % BUFFER_FRAMES;
        log_trace("Frame number: %zu - Pos: %zu", fr_number, fr_pos);
        fr = frames[fr_pos];

        log_trace("Computing RMS");
        fr->rms = dsp_complex_rms(fr->samples, conf->rtlsdr_samples);

        log_trace("Demodulating");
        for (j = 0; j < conf->rtlsdr_samples; j++) {
            switch (conf->modulation) {
                case MOD_TYPE_FM:
                    product = fr->samples[j] * conj(prev_sample);
                    value = (FP_FLOAT) (atan2(cimag(product), creal(product)) * prod_fm);

                    prev_sample = fr->samples[j];
                    break;

                case MOD_TYPE_AM:
                    value = (FP_FLOAT) (((cabs(fr->samples[j]) / prod_am) - 1) * 127);
                    break;

                default:
                    value = 0;
            }

            elem = (int8_t) value;
            fr->demod[j] = elem;
        }

        log_trace("Removing DC offset");
        dsp_remove_dc_offset(fr->demod, conf->rtlsdr_samples);

        result = circbuf_put(buffer_demod, &fr->number, 1);
        if (result != EXIT_SUCCESS) {
            log_error("Unable to put data to circbuf");
            retval = EXIT_FAILURE;
            break;
        }
    }

    log_info("Thread end");

    main_stop();

    pthread_exit(&retval);
}

void *thread_rx_lpf() {
    int retval;

    uint64_t fr_number;
    size_t fr_pos;
    frame *fr;

    fir_ctx *lpf_ctx;

    fft_ctx *fwd_fft_ctx;
    fft_ctx *bck_fft_ctx;
    FP_FLOAT *freq_domain_coeffs;

    int result;
    size_t i;
    size_t half;

    log_info("Thread start");

    retval = EXIT_SUCCESS;

    if (conf->filter == FILTER_MODE_FIR_SW) {
        log_debug("Initializing FIR low-pass filter context");
        lpf_ctx = fir_init_lpf(conf->filter_fir);
        if (lpf_ctx == NULL) {
            log_error("Unable to allocate FIR low-pass filter context");
            retval = EXIT_FAILURE;
            pthread_exit(&retval);
        }
    } else if (conf->filter == FILTER_MODE_FFT_SW) {
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

        log_debug("Allocating Frequency Domain coefficients buffer");
        freq_domain_coeffs = (FP_FLOAT *) calloc(conf->rtlsdr_samples, sizeof(FP_FLOAT));
        if (freq_domain_coeffs == NULL) {
            log_error("Unable to allocate Frequency Domain coefficients buffer");
            fft_free(fwd_fft_ctx);
            fft_free(bck_fft_ctx);
            retval = EXIT_FAILURE;
            pthread_exit(&retval);
        }
    }

    log_debug("Waiting for other threads to init");
    rx_lpf_ready = 1;
    main_rx_wait_init();

    log_debug("Starting filter loop");

    half = conf->rtlsdr_samples / 2;

    while (keep_running) {
        log_trace("Reading %zu bytes", conf->rtlsdr_samples);
        result = circbuf_get(buffer_demod, &fr_number, 1);
        if (result != EXIT_SUCCESS) {
            log_error("Unable to get data from circbuf");
            retval = EXIT_FAILURE;
            break;
        }

        fr_pos = fr_number % BUFFER_FRAMES;
        log_trace("Frame number: %zu - Pos: %zu", fr_number, fr_pos);
        fr = frames[fr_pos];

        switch (conf->filter) {

            case FILTER_MODE_NONE:
                log_trace("Copying data");
                memcpy(fr->filtered, fr->demod, conf->rtlsdr_samples);
                break;

            case FILTER_MODE_FIR_SW:
                log_trace("Filtering with FIR");
                fir_convolve(lpf_ctx, fr->filtered, fr->demod, conf->rtlsdr_samples);
                break;

            case FILTER_MODE_FFT_SW:
                log_debug("Copying input values for forward FFT");
                for (i = 0; i < conf->rtlsdr_samples; i++)
                    fwd_fft_ctx->real_input[i] = (FP_FLOAT) fr->demod[i];

                log_trace("Computing forward FFT");
                fft_compute(fwd_fft_ctx);

                log_debug("Copying output values from forward FFT output");
                for (i = 0; i < conf->rtlsdr_samples; i++)
                    freq_domain_coeffs[i] = fwd_fft_ctx->real_output[i];

                log_trace("Adjusting coeffs");
                for (i = 64; i < half; i++) {
                    freq_domain_coeffs[i] = 0;
                    freq_domain_coeffs[conf->rtlsdr_samples - i] = 0;
                }

                log_debug("Copying input values for backward FFT");
                for (i = 0; i < conf->rtlsdr_samples; i++)
                    bck_fft_ctx->real_input[i] = freq_domain_coeffs[i];

                log_trace("Computing backward FFT");
                fft_compute(bck_fft_ctx);

                log_debug("Copying output values from backward FFT output");
                for (i = 0; i < conf->rtlsdr_samples; i++)
                    fr->filtered[i] = bck_fft_ctx->real_output[i] / conf->rtlsdr_samples;

                break;

            default:
                log_error("Not implemented");
                retval = EXIT_FAILURE;
                break;
        }

        result = circbuf_put(buffer_filtered, &fr->number, 1);
        if (result != EXIT_SUCCESS) {
            log_error("Unable to put data to circbuf");
            retval = EXIT_FAILURE;
            break;
        }
    }

    if (conf->filter == FILTER_MODE_FIR_SW) {
        log_debug("Freeing resample context");
        fir_free(lpf_ctx);
    }

    if (conf->filter == FILTER_MODE_FFT_SW) {
        log_debug("Freeing FFT forward context");
        fft_free(fwd_fft_ctx);

        log_debug("Freeing FFT backward context");
        fft_free(bck_fft_ctx);

        log_debug("Freeing Frequency Domain coefficients buffer");
        free(freq_domain_coeffs);
    }

    log_info("Thread end");

    main_stop();

    pthread_exit(&retval);
}

void *thread_rx_resample() {
    int retval;

    uint64_t fr_number;
    size_t fr_pos;
    frame *fr;

    size_t audio_num;
    resample_ctx *res_ctx;
    int result;

    log_info("Thread start");

    retval = EXIT_SUCCESS;

    log_debug("Initializing resample context");
    res_ctx = resample_init(conf->rtlsdr_device_sample_rate, conf->audio_sample_rate);
    if (res_ctx == NULL) {
        log_error("Unable to allocate resample context");
        retval = EXIT_FAILURE;
        pthread_exit(&retval);
    }

    audio_num = resample_compute_output_size(res_ctx, conf->rtlsdr_samples);
    log_debug("Audio buffer samples_size is %zu", audio_num);

    log_debug("Waiting for other threads to init");
    rx_resample_ready = 1;
    main_rx_wait_init();

    log_debug("Starting resample loop");
    while (keep_running) {
        log_trace("Reading %zu bytes", conf->rtlsdr_samples);
        result = circbuf_get(buffer_filtered, &fr_number, 1);
        if (result != EXIT_SUCCESS) {
            log_error("Unable to get data from circbuf");
            retval = EXIT_FAILURE;
            break;
        }

        fr_pos = fr_number % BUFFER_FRAMES;
        log_trace("Frame number: %zu - Pos: %zu", fr_number, fr_pos);
        fr = frames[fr_pos];

        log_trace("Resampling %zu bytes in %zu bytes", conf->rtlsdr_samples, audio_num);
        resample_do(res_ctx, fr->filtered, conf->rtlsdr_samples, fr->pcm, audio_num);

//        log_trace("Applying limiter");
//        agc_perform_gain(fr->pcm, audio_num);

        result = circbuf_put(buffer_codec, &fr->number, 1);
        if (result != EXIT_SUCCESS) {
            log_error("Unable to put data to codec circbuf");
            retval = EXIT_FAILURE;
            break;
        }

        result = circbuf_put(buffer_monitor, &fr->number, 1);
        if (result != EXIT_SUCCESS) {
            log_error("Unable to put data to monitor circbuf");
            retval = EXIT_FAILURE;
            break;
        }
    }

    log_debug("Freeing resample context");
    resample_free(res_ctx);

    log_info("Thread end");

    main_stop();

    pthread_exit(&retval);
}

void *thread_rx_codec() {
    int retval;

    codec_ctx *ctx;

    uint64_t fr_number;
    size_t fr_pos;
    frame *fr;

    size_t p_pos;
    payload *p;

    int result;

    int8_t *pcm_buffer;
    size_t pcm_size;

    uint8_t codec_buffer[8192];
    size_t codec_bytes;

    log_info("Thread start");

    retval = EXIT_SUCCESS;

    payload_number = 0;

    log_debug("Initializing codec context");
    ctx = codec_init(conf->audio_sample_rate, conf->codec_opus_bitrate);
    if (ctx == NULL) {
        log_error("Unable to allocate codec context");
        retval = EXIT_FAILURE;
        pthread_exit(&retval);
    }

    log_debug("Preparing PCM buffer");
    pcm_buffer = (int8_t *) calloc(CODEC_FRAME_BUFFER, sizeof(int8_t));
    if (pcm_buffer == NULL) {
        log_error("Unable to allocate PCM buffer");
        codec_free(ctx);
        retval = EXIT_FAILURE;
        pthread_exit(&retval);
    }

    pcm_size = 0;

    log_debug("Waiting for other threads to init");
    rx_codec_ready = 1;
    main_rx_wait_init();

    log_debug("Starting codec loop");

    while (keep_running) {
        log_trace("Reading frame number");
        result = circbuf_get(buffer_codec, &fr_number, 1);
        if (result != EXIT_SUCCESS) {
            log_error("Unable to get data from circbuf");
            retval = EXIT_FAILURE;
            break;
        }

        fr_pos = fr_number % BUFFER_FRAMES;
        fr = frames[fr_pos];
        log_trace("Frame number: %zu - Pos: %zu", fr->number, fr_pos);

        memcpy(pcm_buffer + pcm_size, fr->pcm, fr->size_pcm);
        pcm_size += fr->size_pcm;

        if (pcm_size >= CODEC_FRAME_SIZE) {
            codec_encode(ctx, pcm_buffer, codec_buffer, sizeof(codec_buffer), &codec_bytes);

            p_pos = payload_number % BUFFER_PAYLOADS;
            p = payloads[p_pos];
            log_trace("Payload number: %zu - Pos: %zu", fr->number, fr_pos);

            payload_set_numbers(p, 0, payload_number);
            payload_set_timestamp(p, &fr->ts);
            payload_set_rms(p, fr->rms);
            payload_set_channel_frequency(p, 0, conf->rtlsdr_device_center_freq);
            payload_set_data(p, codec_buffer, codec_bytes);

            result = circbuf_put(buffer_network, &payload_number, 1);
            if (result != EXIT_SUCCESS) {
                log_error("Unable to put data to circbuf");
                retval = EXIT_FAILURE;
                break;
            }

            pcm_size = 0;

            payload_number++;
        }
    }

    log_debug("Freeing codec context");
    codec_free(ctx);

    log_debug("Freeing PCM buffer");
    free(pcm_buffer);

    log_info("Thread end");

    main_stop();

    pthread_exit(&retval);
}

void *thread_rx_monitor() {
    int retval;

    audio_ctx *ctx;

    uint64_t fr_number;
    size_t fr_pos;
    frame *fr;

    int result;

    uint8_t *pcm_buffer;
    size_t pcm_size;
    size_t i;

    log_info("Thread start");

    retval = EXIT_SUCCESS;

    log_debug("Initializing audio context");
    ctx = audio_init(conf->audio_monitor_device, conf->audio_sample_rate, 1, SND_PCM_FORMAT_U8);
    if (ctx == NULL) {
        log_error("Unable to allocate codec context");
        retval = EXIT_FAILURE;
        pthread_exit(&retval);
    }

    log_debug("Preparing PCM buffer");
    pcm_buffer = (uint8_t *) calloc(CODEC_FRAME_BUFFER, sizeof(uint8_t));
    if (pcm_buffer == NULL) {
        log_error("Unable to allocate PCM buffer");
        audio_free(ctx);
        retval = EXIT_FAILURE;
        pthread_exit(&retval);
    }

    pcm_size = 0;

    log_debug("Waiting for other threads to init");
    rx_monitor_ready = 1;
    main_rx_wait_init();

    log_debug("Starting monitor loop");

    while (keep_running) {
        log_trace("Reading frame number");
        result = circbuf_get(buffer_monitor, &fr_number, 1);
        if (result != EXIT_SUCCESS) {
            log_error("Unable to get data from circbuf");
            retval = EXIT_FAILURE;
            break;
        }

        fr_pos = fr_number % BUFFER_FRAMES;
        fr = frames[fr_pos];
        log_trace("Frame number: %zu - Pos: %zu", fr->number, fr_pos);

        for (i = 0; i < fr->size_pcm; i++)
            pcm_buffer[pcm_size + i] = (uint8_t) (127 + fr->pcm[i]);

        pcm_size += fr->size_pcm;

        if (pcm_size >= ctx->frames_per_period) {
            result = audio_play_uint8(ctx, pcm_buffer, pcm_size);
            if (result != EXIT_SUCCESS) {
                log_error("Unable to play buffer");
                retval = EXIT_FAILURE;
                break;
            }

            pcm_size = 0;
        }
    }

    log_debug("Freeing audio context");
    audio_free(ctx);

    log_debug("Freeing PCM buffer");
    free(pcm_buffer);

    log_info("Thread end");

    main_stop();

    pthread_exit(&retval);
}

void *thread_rx_network() {
    int retval;

    network_ctx *ctx;

    uint64_t p_number;
    size_t p_pos;
    payload *p;

    uint8_t network_buffer[8192];
    size_t network_bytes;

    int result;

    log_info("Thread start");

    retval = EXIT_SUCCESS;

    log_debug("Initializing network context");
    ctx = network_init(conf->network_server, conf->network_port);
    if (ctx == NULL) {
        log_error("Unable to allocate network context");
        retval = EXIT_FAILURE;
        pthread_exit(&retval);
    }

    log_debug("Opening socket");
    result = network_socket_open(ctx);
    if (result != EXIT_SUCCESS) {
        log_error("Unable to open network socket");
        network_free(ctx);
        retval = EXIT_FAILURE;
        pthread_exit(&retval);
    }

    log_debug("Waiting for other threads to init");
    rx_network_ready = 1;
    main_rx_wait_init();

    while (keep_running) {
        log_trace("Reading frame number");
        result = circbuf_get(buffer_network, &p_number, 1);
        if (result != EXIT_SUCCESS) {
            log_error("Unable to get data from circbuf");
            retval = EXIT_FAILURE;
            break;
        }

        p_pos = p_number % BUFFER_PAYLOADS;
        p = payloads[p_pos];
        log_trace("Payload number: %zu - Pos: %zu", p_number, p_pos);

        if (p->rms > 10) {
            payload_serialize(p, network_buffer, sizeof(network_buffer), &network_bytes);
            network_socket_send(ctx, network_buffer, network_bytes);
        }
    }

    log_debug("Closing network socket");
    network_socket_close(ctx);

    log_debug("Freeing codec context");
    network_free(ctx);

    log_info("Thread end");

    main_stop();

    pthread_exit(&retval);
}
