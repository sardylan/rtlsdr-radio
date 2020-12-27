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


#include "utils.h"

void utils_uint16_to_be(uint8_t *buffer, uint16_t number) {
    buffer[2] = number >> 8 * 1;
    buffer[3] = number >> 8 * 0;
}

void utils_uint32_to_be(uint8_t *buffer, uint32_t number) {
    buffer[0] = number >> 8 * 3;
    buffer[1] = number >> 8 * 2;
    buffer[2] = number >> 8 * 1;
    buffer[3] = number >> 8 * 0;
}

void utils_uint64_to_be(uint8_t *buffer, uint64_t number) {
    buffer[0] = number >> 8 * 7;
    buffer[1] = number >> 8 * 6;
    buffer[2] = number >> 8 * 5;
    buffer[3] = number >> 8 * 4;
    buffer[4] = number >> 8 * 3;
    buffer[5] = number >> 8 * 2;
    buffer[6] = number >> 8 * 1;
    buffer[7] = number >> 8 * 0;
}

void utils_int16_to_be(uint8_t *buffer, int16_t number) {
    buffer[0] = (number >> 8) & 0xff;
    buffer[1] = (number >> 0) & 0xff;
}

void utils_int32_to_be(uint8_t *buffer, int32_t number) {
    buffer[0] = (number >> 24) & 0xff;
    buffer[1] = (number >> 16) & 0xff;
    buffer[2] = (number >> 8) & 0xff;
    buffer[3] = (number >> 0) & 0xff;
}

void utils_int64_to_be(uint8_t *buffer, int64_t number) {
    buffer[0] = (number >> 56) & 0xff;
    buffer[1] = (number >> 48) & 0xff;
    buffer[2] = (number >> 40) & 0xff;
    buffer[3] = (number >> 32) & 0xff;
    buffer[4] = (number >> 24) & 0xff;
    buffer[5] = (number >> 16) & 0xff;
    buffer[6] = (number >> 8) & 0xff;
    buffer[7] = (number >> 0) & 0xff;
}
