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


#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "utils.h"

void utils_uint16_to_be(uint8_t *buffer, uint16_t number) {
    buffer[0] = (number >> 8 * 0) & 0xff;
    buffer[1] = (number >> 8 * 1) & 0xff;
}

void utils_uint16_to_le(uint8_t *buffer, uint16_t number) {
    buffer[0] = (number >> 8 * 1) & 0xff;
    buffer[1] = (number >> 8 * 0) & 0xff;
}

void utils_uint32_to_be(uint8_t *buffer, uint32_t number) {
    buffer[0] = (number >> 8 * 0) & 0xff;
    buffer[1] = (number >> 8 * 1) & 0xff;
    buffer[2] = (number >> 8 * 2) & 0xff;
    buffer[3] = (number >> 8 * 3) & 0xff;
}

void utils_uint32_to_le(uint8_t *buffer, uint32_t number) {
    buffer[0] = (number >> 8 * 3) & 0xff;
    buffer[1] = (number >> 8 * 2) & 0xff;
    buffer[2] = (number >> 8 * 1) & 0xff;
    buffer[3] = (number >> 8 * 0) & 0xff;
}

void utils_uint64_to_be(uint8_t *buffer, uint64_t number) {
    buffer[0] = (number >> 8 * 0) & 0xff;
    buffer[1] = (number >> 8 * 1) & 0xff;
    buffer[2] = (number >> 8 * 2) & 0xff;
    buffer[3] = (number >> 8 * 3) & 0xff;
    buffer[4] = (number >> 8 * 4) & 0xff;
    buffer[5] = (number >> 8 * 5) & 0xff;
    buffer[6] = (number >> 8 * 6) & 0xff;
    buffer[7] = (number >> 8 * 7) & 0xff;
}

void utils_uint64_to_le(uint8_t *buffer, uint64_t number) {
    buffer[0] = (number >> 8 * 7) & 0xff;
    buffer[1] = (number >> 8 * 6) & 0xff;
    buffer[2] = (number >> 8 * 5) & 0xff;
    buffer[3] = (number >> 8 * 4) & 0xff;
    buffer[4] = (number >> 8 * 3) & 0xff;
    buffer[5] = (number >> 8 * 2) & 0xff;
    buffer[6] = (number >> 8 * 1) & 0xff;
    buffer[7] = (number >> 8 * 0) & 0xff;
}

void utils_int16_to_be(uint8_t *buffer, int16_t number) {
    buffer[0] = (number >> 8 * 0) & 0xff;
    buffer[1] = (number >> 8 * 1) & 0xff;
}

void utils_int16_to_le(uint8_t *buffer, int16_t number) {
    buffer[0] = (number >> 8 * 1) & 0xff;
    buffer[1] = (number >> 8 * 0) & 0xff;
}

void utils_int32_to_be(uint8_t *buffer, int32_t number) {
    buffer[0] = (number >> 8 * 0) & 0xff;
    buffer[1] = (number >> 8 * 1) & 0xff;
    buffer[2] = (number >> 8 * 2) & 0xff;
    buffer[3] = (number >> 8 * 3) & 0xff;
}

void utils_int32_to_le(uint8_t *buffer, int32_t number) {
    buffer[0] = (number >> 8 * 3) & 0xff;
    buffer[1] = (number >> 8 * 2) & 0xff;
    buffer[2] = (number >> 8 * 1) & 0xff;
    buffer[3] = (number >> 8 * 0) & 0xff;
}

void utils_int64_to_be(uint8_t *buffer, int64_t number) {
    buffer[0] = (number >> 8 * 0) & 0xff;
    buffer[1] = (number >> 8 * 1) & 0xff;
    buffer[2] = (number >> 8 * 2) & 0xff;
    buffer[3] = (number >> 8 * 3) & 0xff;
    buffer[4] = (number >> 8 * 4) & 0xff;
    buffer[5] = (number >> 8 * 5) & 0xff;
    buffer[6] = (number >> 8 * 6) & 0xff;
    buffer[7] = (number >> 8 * 7) & 0xff;
}

void utils_int64_to_le(uint8_t *buffer, int64_t number) {
    buffer[0] = (number >> 8 * 7) & 0xff;
    buffer[1] = (number >> 8 * 6) & 0xff;
    buffer[2] = (number >> 8 * 5) & 0xff;
    buffer[3] = (number >> 8 * 4) & 0xff;
    buffer[4] = (number >> 8 * 3) & 0xff;
    buffer[5] = (number >> 8 * 2) & 0xff;
    buffer[6] = (number >> 8 * 1) & 0xff;
    buffer[7] = (number >> 8 * 0) & 0xff;
}

int utils_ltrim(char *dst, char *src, size_t size) {
    char *s;

    if (dst == NULL || src == NULL || size == 0)
        return EXIT_FAILURE;

    bzero(dst, size);

    s = src;
    while (s != NULL && isspace(*s)) s++;
    strncpy(dst, s, size);
    return EXIT_SUCCESS;
}

int utils_rtrim(char *dst, char *src, size_t size) {
    char *s;

    if (dst == NULL || src == NULL || size == 0)
        return EXIT_FAILURE;

    strncpy(dst, src, size);

    s = dst;
    while (*s != '\0') s++;
    s--;

    while (s != src && isspace(*s)) s--;
    s++;
    *s = '\0';

    return EXIT_SUCCESS;
}

int utils_trim(char *dst, char *src, size_t size) {
    char *temp;

    if (dst == NULL || src == NULL || size == 0)
        return EXIT_FAILURE;

    temp = (char *) calloc(size, sizeof(char));

    if (utils_ltrim(temp, src, size) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    if (utils_rtrim(dst, temp, size) != EXIT_SUCCESS)
        return EXIT_FAILURE;

    free(temp);

    return EXIT_SUCCESS;
}

int utils_stricmp(const char *a, const char *b) {
    if (a == NULL && b == NULL)
        return 0;

    if (a == NULL)
        return -1;
    if (b == NULL)
        return 1;

    while (*a != '\0') {
        if (*a >= 0x41 && *a <= 0x5A) {
            if (*a != *b && *a != (*b - 0x20))
                break;
        } else if (*a >= 0x61 && *a <= 0x7A) {
            if (*a != *b && *a != (*b + 0x20))
                break;
        } else if (*a != *b) {
            break;
        }

        a++;
        b++;
    }

    return *(const unsigned char *) a - *(const unsigned char *) b;
}

int utils_timespec_sub(struct timespec *a, struct timespec *b, struct timespec *result) {
    if (a == NULL || b == NULL || result == NULL)
        return EXIT_FAILURE;

    result->tv_sec = b->tv_sec - a->tv_sec;
    result->tv_nsec = b->tv_nsec - a->tv_nsec;
    if (result->tv_nsec < 0) {
        result->tv_sec--;
        result->tv_nsec += 1000000000L;
    }

    return EXIT_SUCCESS;
}
