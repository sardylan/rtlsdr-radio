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


#ifndef __RTLSDR_RADIO__LOG__H
#define __RTLSDR_RADIO__LOG__H

#include <stddef.h>

#define LOG_LEVEL_OFF 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_DEBUG 4

#define log_error(element, message, ...) log_message(LOG_LEVEL_ERROR, element, message, ##__VA_ARGS__)
#define log_warn(element, message, ...) log_message(LOG_LEVEL_WARNING, element, message, ##__VA_ARGS__)
#define log_info(element, message, ...) log_message(LOG_LEVEL_INFO, element, message, ##__VA_ARGS__)
#define log_debug(element, message, ...) log_message(LOG_LEVEL_DEBUG, element, message, ##__VA_ARGS__)

void log_message(int, const char *, const char *, ...);

char *log_level_to_char(int);

#endif
