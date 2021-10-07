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


#ifndef __RTLSDR_RADIO__DEVICE__H
#define __RTLSDR_RADIO__DEVICE__H

#include <complex.h>
#include <rtl-sdr.h>

#include "buildflags.h"

void device_list(int);

int device_open(rtlsdr_dev_t **, uint32_t);

void device_close(rtlsdr_dev_t *);

void device_info(rtlsdr_dev_t *);

int device_set_params(rtlsdr_dev_t *, uint32_t, int, int, int, int);

int device_set_frequency(rtlsdr_dev_t *, uint32_t);

int device_buffer_to_samples(const uint8_t *, FP_FLOAT complex *, size_t);

char *device_tuner_to_char(enum rtlsdr_tuner);

#endif
