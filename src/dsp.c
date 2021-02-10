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


#include <math.h>

#include "dsp.h"

FP_FLOAT dsp_complex_rms(FP_FLOAT complex *data, size_t data_size) {
    size_t i;
    FP_FLOAT sum;

    sum = 0;
    for (i = 0; i < data_size; i++)
        sum += cabs(data[i]);

    sum /= (data_size * M_SQRT2);

    return sqrt(sum);
}

FP_FLOAT dsp_rms(int8_t *data, size_t data_size) {
    size_t i;
    FP_FLOAT sum;

    sum = 0;
    for (i = 0; i < data_size; i++)
        sum += pow(data[i], 2);

    sum /= (data_size * M_SQRT2);

    return sqrt(sum);
}

void dsp_remove_dc_offset(int8_t *buffer, size_t buffer_size) {
    size_t i;
    FP_FLOAT sum;
    int8_t offset;

    sum = 0;

    for (i = 0; i < buffer_size; i++)
        sum += buffer[i];

    offset = sum / buffer_size;

    for (i = 0; i < buffer_size; i++)
        buffer[i] -= offset;
}
