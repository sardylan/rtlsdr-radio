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

#include "cfg.h"

#define LOG_LEVEL_OFF 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_DEBUG 4
#define LOG_LEVEL_TRACE 5

#define LOG_BUFFER 131072

#define log_error(message, ...) log_message(LOG_LEVEL_ERROR, __FUNCTION__, __FILE__, __LINE__, message, ##__VA_ARGS__)
#define log_warn(message, ...) log_message(LOG_LEVEL_WARNING, __FUNCTION__, __FILE__, __LINE__, message, ##__VA_ARGS__)
#define log_info(message, ...) log_message(LOG_LEVEL_INFO, __FUNCTION__, __FILE__, __LINE__, message, ##__VA_ARGS__)
#define log_debug(message, ...) log_message(LOG_LEVEL_DEBUG, __FUNCTION__, __FILE__, __LINE__, message, ##__VA_ARGS__)
#define log_trace(message, ...) log_message(LOG_LEVEL_TRACE, __FUNCTION__, __FILE__, __LINE__, message, ##__VA_ARGS__)

void log_init();

void log_free();

void log_message(int, const char *, char *, int, const char *, ...);

void log_start();

void log_stop();

char *log_level_to_char(int);

char *log_mode_to_char(work_mode);

char *log_bool_to_char(int);

char *log_modulation_to_char(modulation_type);

#endif
