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


#ifndef __RTLSDR_RADIO__LOG__H
#define __RTLSDR_RADIO__LOG__H

#include <stddef.h>

#ifndef __RTLSDR__TESTS

#include "cfg.h"
#include "buildflags.h"

#endif

#define LOG_LEVEL_OFF 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_DEBUG 4
#define LOG_LEVEL_TRACE 5

#define LOG_BUFFER 131072

#ifdef RTLSDR_RADIO_LOG_ERROR_ENABLED
#define log_error(message, ...) log_message(LOG_LEVEL_ERROR, __func__, __FILENAME__, __LINE__, message, ##__VA_ARGS__)
#else
#define log_error(message, ...)
#endif

#ifdef RTLSDR_RADIO_LOG_WARN_ENABLED
#define log_warn(message, ...) log_message(LOG_LEVEL_WARNING, __func__, __FILENAME__, __LINE__, message, ##__VA_ARGS__)
#else
#define log_warn(message, ...)
#endif

#ifdef RTLSDR_RADIO_LOG_INFO_ENABLED
#define log_info(message, ...) log_message(LOG_LEVEL_INFO, __func__, __FILENAME__, __LINE__, message, ##__VA_ARGS__)
#else
#define log_info(message, ...)
#endif

#ifdef RTLSDR_RADIO_LOG_DEBUG_ENABLED
#define log_debug(message, ...) log_message(LOG_LEVEL_DEBUG, __func__, __FILENAME__, __LINE__, message, ##__VA_ARGS__)
#else
#define log_debug(message, ...)
#endif

#ifdef RTLSDR_RADIO_LOG_TRACE_ENABLED
#define log_trace(message, ...) log_message(LOG_LEVEL_TRACE, __func__, __FILENAME__, __LINE__, message, ##__VA_ARGS__)
#else
#define log_trace(message, ...)
#endif

#ifndef __RTLSDR__TESTS

void log_init();

void log_free();

void log_message(int, const char *, char *, int, const char *, ...);

void log_start();

void log_stop();

void log_start_file();

void log_stop_file();

#endif

#endif
