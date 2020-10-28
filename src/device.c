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


#include <rtl-sdr.h>

#include "device.h"
#include "cfg.h"
#include "log.h"

rtlsdr_dev_t *device;

extern cfg *conf;

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

//    result = rtlsdr_set_tuner_gain_mode(device, conf->rtlsdr_device_tuner_gain_mode);
//    if (result < 0) {
//        log_error("device", "Failed to set gain mode: %d\n", result);
//        return -1;
//    }

//    result = rtlsdr_set_tuner_gain(device, conf->rtlsdr_device_tuner_gain);
//    if (result < 0) {
//        log_error("device", "Failed to set tuner gain: %d\n", result);
//        return -1;
//    }

//    result = rtlsdr_set_agc_mode(device, conf->rtlsdr_device_agc_mode);
//    if (result < 0)
//        log_error("device", "Failed to disable AGC: %d\n", result);

    rtlsdr_reset_buffer(device);

    return 0;
}

void device_close() {
    rtlsdr_close(device);
}
