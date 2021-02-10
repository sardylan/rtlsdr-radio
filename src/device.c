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


#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <rtl-sdr.h>

#include "device.h"
#include "cfg.h"
#include "log.h"
#include "ui.h"

#define DEVICE_STRINGS_SIZE 257

void device_list(int debug) {
    uint32_t device_count;
    uint32_t i;
    const char *device_name;
    char *manufact;
    char *product;
    char *serial;
    enum rtlsdr_tuner tuner;
    int result;
    rtlsdr_dev_t *device;
    int gains[128];
    int it;

    manufact = (char *) calloc(DEVICE_STRINGS_SIZE, sizeof(char));
    product = (char *) calloc(DEVICE_STRINGS_SIZE, sizeof(char));
    serial = (char *) calloc(DEVICE_STRINGS_SIZE, sizeof(char));

    device_count = rtlsdr_get_device_count();
    ui_message_out("Device count: %zu\n", device_count);

    ui_message("\n");

    for (i = 0; i < device_count; i++) {
        device_name = rtlsdr_get_device_name(i);

        bzero(manufact, DEVICE_STRINGS_SIZE);
        bzero(product, DEVICE_STRINGS_SIZE);
        bzero(serial, DEVICE_STRINGS_SIZE);
        rtlsdr_get_device_usb_strings(i, manufact, product, serial);

        ui_message_out("%zu - %s (%s %s - %s)\n", i, device_name, manufact, product, serial);

        if (!debug)
            continue;

        result = rtlsdr_open(&device, i);
        if (result < 0) {
            log_error("Unable to open RTL-SDR device: %d", result);
            continue;
        }

        tuner = rtlsdr_get_tuner_type(device);
        ui_message_out("    > Tuner: %s\n", device_tuner_to_char(tuner));

        result = rtlsdr_get_tuner_gains(device, &gains[0]);
        if (result < 0) {
            log_error("Failed to get list of gains: %d", result);
            continue;
        }

        ui_message_out("    > Gains: ");
        for (it = 0; it < result; it++)
            ui_message_out("%d ", gains[it]);

        ui_message_out("\n");

        rtlsdr_close(device);
    }

    free(manufact);
    free(product);
    free(serial);
}

int device_open(rtlsdr_dev_t **device, uint32_t device_id) {
    int result;

    result = rtlsdr_open(device, device_id);
    if (result < 0) {
        log_error("Unable to open RTL-SDR device: %d", result);
        return EXIT_FAILURE;
    }

    rtlsdr_reset_buffer(*device);

    return EXIT_SUCCESS;
}

void device_close(rtlsdr_dev_t *device) {
    if (device != NULL)
        rtlsdr_close(device);

    device = NULL;
}

void device_info(rtlsdr_dev_t *device) {
    uint32_t sample_rate;
    uint32_t center_freq;
    int freq_correction;
    int tuner_gain;

    sample_rate = rtlsdr_get_sample_rate(device);
    center_freq = rtlsdr_get_center_freq(device);
    freq_correction = rtlsdr_get_freq_correction(device);
    tuner_gain = rtlsdr_get_tuner_gain(device);

    ui_message("Sample rate: %zu - Center freq: %zu - Freq correction: %d - Tuner gain: %d\n",
             sample_rate, center_freq, freq_correction, tuner_gain);
}

int device_set_params(
        rtlsdr_dev_t *device,
        uint32_t sample_rate,
        int freq_correction,
        int tuner_gain_mode,
        int tuner_gain,
        int agc_mode
) {
    int result;

    result = rtlsdr_set_sample_rate(device, sample_rate);
    if (result < 0) {
        log_error("Failed to set sample rate: %d", result);
        return EXIT_FAILURE;
    }

    result = rtlsdr_set_freq_correction(device, freq_correction);
    if (result < 0 && result != -2) {
        log_error("Failed to set freq correction: %d", result);
        return EXIT_FAILURE;
    }

    result = rtlsdr_set_tuner_gain_mode(device, tuner_gain_mode);
    if (result < 0) {
        log_error("Failed to set gain mode: %d", result);
        return EXIT_FAILURE;
    }

    result = rtlsdr_set_tuner_gain(device, tuner_gain);
    if (result < 0) {
        log_error("Failed to set tuner gain: %d", result);
        return EXIT_FAILURE;
    }

    result = rtlsdr_set_agc_mode(device, agc_mode);
    if (result < 0) {
        log_error("Failed to disable AGC: %d", result);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int device_set_frequency(rtlsdr_dev_t *device, uint32_t frequency) {
    int result;

    result = rtlsdr_set_center_freq(device, frequency);
    if (result < 0) {
        log_error("Failed to set center frequency: %d", result);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int device_buffer_to_samples(const uint8_t *buffer, FP_FLOAT complex *samples, size_t buffer_size) {
    int8_t i;
    int8_t q;
    size_t j;

    for (j = 0; j < buffer_size; j += 2) {
        i = buffer[j] - 128;
        q = buffer[j + 1] - 128;
        samples[j / 2] = i + q * I;
    }

    return EXIT_SUCCESS;
}

char *device_tuner_to_char(enum rtlsdr_tuner tuner) {
    switch (tuner) {

        case RTLSDR_TUNER_UNKNOWN:
            return "UNKNOWN";
        case RTLSDR_TUNER_E4000:
            return "E4000";
        case RTLSDR_TUNER_FC0012:
            return "Fiticomm FC0012";
        case RTLSDR_TUNER_FC0013:
            return "Fiticomm FC0013";
        case RTLSDR_TUNER_FC2580:
            return "FC2580";
        case RTLSDR_TUNER_R820T:
            return "Rafael Microelectronics R820T";
        case RTLSDR_TUNER_R828D:
            return "Rafael Microelectronics R828D";
        default:
            return "NOT SUPPORTED";
    }
}
