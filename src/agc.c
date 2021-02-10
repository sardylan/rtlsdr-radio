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
#include <math.h>

#include "agc.h"
#include "log.h"
#include "dsp.h"

int agc_limiter(int8_t *data, size_t data_size) {
    size_t i;
    int8_t abs_value;
    int8_t max;
    FP_FLOAT rms;
    FP_FLOAT gain;

    log_info("Applying limiter");

    rms = dsp_rms(data, data_size);
    gain = pow(2, rms + 4);

    max = 0;

    for (i = 0; i < data_size; i++) {
        abs_value = abs(data[i]);
        if (abs_value > max)
            max = abs_value;
    }

    if (max * gain > 127)
        gain = 127.0 / max;

    for (i = 0; i < data_size; i++)
        data[i] *= gain;

    return EXIT_SUCCESS;
}
