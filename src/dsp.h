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


#ifndef __RTLSDR_RADIO__DSP__H
#define __RTLSDR_RADIO__DSP__H

#include <stdint.h>
#include <stddef.h>
#include <complex.h>

FP_FLOAT dsp_complex_rms(FP_FLOAT complex *, size_t);

FP_FLOAT dsp_rms(int8_t *, size_t data_size);

void dsp_remove_dc_offset(int8_t *, size_t);

#endif
