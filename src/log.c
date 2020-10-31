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


#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>

#include "log.h"
#include "cfg.h"
#include "ui.h"

pthread_mutex_t log_lock;

extern cfg *conf;

void log_init() {
    pthread_mutex_init(&log_lock, NULL);
}

void log_free() {
    pthread_mutex_destroy(&log_lock);
}

void log_message(const int level, const char *element, const char *message, ...) {
    va_list args;
    char datetime[27];
    struct tm *timeinfo;
    struct timespec ts;
    char content[LOG_BUFFER];
    char prefix[1025];

    if (level > conf->ui_log_level && level > conf->file_log_level)
        return;

    pthread_mutex_lock(&log_lock);

    timespec_get(&ts, TIME_UTC);

    timeinfo = localtime(&ts.tv_sec);
    strftime(datetime, 20, "%Y-%m-%d %H:%M:%S", timeinfo);
    sprintf(datetime + 19, ".%06zu", ts.tv_nsec / 1000);

    sprintf(prefix, "%s (%lx) [%s] {%s} ", datetime, pthread_self(), log_level_to_char(level), element);

    fprintf(UI_MESSAGES_OUTPUT, "%s", prefix);

    memset(content, '\0', sizeof(content));

    va_start(args, message);
    vsnprintf(content, LOG_BUFFER, message, args);
    va_end(args);

    if (level <= conf->ui_log_level)
        ui_message("%s\n", content);
//        fprintf(UI_MESSAGES_OUTPUT, "%s\n", content);

    pthread_mutex_unlock(&log_lock);
}

void log_start() {
    ui_message("------- START -------\n");
}

void log_stop() {
    ui_message("-------- END --------\n");
}

char *log_level_to_char(int level) {
    switch (level) {
        case LOG_LEVEL_OFF:
            return " OFF ";
        case LOG_LEVEL_ERROR:
            return "ERROR";
        case LOG_LEVEL_WARNING:
            return "WARN ";
        case LOG_LEVEL_INFO:
            return "INFO ";
        case LOG_LEVEL_DEBUG:
            return "DEBUG";
        case LOG_LEVEL_TRACE:
            return "TRACE";
        default:
            return "     ";
    }
}

char *log_mode_to_char(work_mode mode) {
    switch (mode) {
        case MODE_VERSION:
            return "VERSION";
        case MODE_HELP:
            return "HELP";
        case MODE_RX:
            return "RX";
        case MODE_INFO:
            return "INFO";
        default:
            return "";
    }
}

char *log_bool_to_char(int value) {
    if (value == 0)
        return "NO";
    else
        return "YES";
}

char *log_modulation_to_char(modulation_type value) {
    switch (value) {
        case MOD_TYPE_AM:
            return "AM";
        case MOD_TYPE_FM:
            return "FM";
        default:
            return "";
    }
}