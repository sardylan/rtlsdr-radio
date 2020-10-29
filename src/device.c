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


#include <string.h>
#include <malloc.h>
#include <rtl-sdr.h>

#include "device.h"
#include "cfg.h"
#include "log.h"
#include "ui.h"

#define DEVICE_STRINGS_SIZE 257

rtlsdr_dev_t *device;

extern cfg *conf;

void device_list() {
    uint32_t device_count;
    uint32_t i;
    const char *device_name;
    char *manufact;
    char *product;
    char *serial;
    enum rtlsdr_tuner tuner;
    int result;
    rtlsdr_dev_t *info_device;
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

        if (!conf->debug)
            continue;

        result = rtlsdr_open(&info_device, i);
        if (result < 0) {
            log_error("device", "Unable to open RTL-SDR device: %d\n", result);
            continue;
        }

        tuner = rtlsdr_get_tuner_type(info_device);
        ui_message_out("    > Tuner: %s\n", device_tuner_to_char(tuner));

        result = rtlsdr_get_tuner_gains(info_device, &gains[0]);
        if (result < 0) {
            log_error("device", "Failed to get list of gains: %d\n", result);
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

int device_open() {
    int result;

    result = rtlsdr_open(&device, conf->rtlsdr_device_id);
    if (result < 0) {
        log_error("device", "Unable to open RTL-SDR device: %d\n", result);
        return -1;
    }

    result = rtlsdr_set_sample_rate(device, conf->rtlsdr_device_sample_rate);
    if (result < 0)
        log_error("device", "Failed to set sample rate: %d\n", result);

    result = rtlsdr_set_center_freq(device, conf->rtlsdr_device_center_freq);
    if (result < 0)
        log_error("device", "Failed to set center frequency: %d\n", result);

    result = rtlsdr_set_freq_correction(device, conf->rtlsdr_device_freq_correction);
    if (result < 0 && result != -2)
        log_error("device", "Failed to set freq correction: %d\n", result);

    result = rtlsdr_set_tuner_gain_mode(device, conf->rtlsdr_device_tuner_gain_mode);
    if (result < 0) {
        log_error("device", "Failed to set gain mode: %d\n", result);
        return -1;
    }

    result = rtlsdr_set_tuner_gain(device, conf->rtlsdr_device_tuner_gain);
    if (result < 0) {
        log_error("device", "Failed to set tuner gain: %d\n", result);
        return -1;
    }

    result = rtlsdr_set_agc_mode(device, conf->rtlsdr_device_agc_mode);
    if (result < 0)
        log_error("device", "Failed to disable AGC: %d\n", result);

    rtlsdr_reset_buffer(device);

    return 0;
}

void device_close() {
    rtlsdr_close(device);
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
