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


#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <libgen.h>

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

void log_message(const int level, const char *element, char *filename, const int row, const char *message, ...) {
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
    sprintf(datetime + 19, ".%06lu", ts.tv_nsec / 1000);

    sprintf(prefix, "%s (%lx) [%s] {%s} (%s:%d) ",
            datetime,
            pthread_self(),
            cfg_tochar_log_level(level),
            element,
            basename(filename),
            row);

    fprintf(UI_MESSAGES_OUTPUT, "%s", prefix);

    memset(content, '\0', sizeof(content));

    va_start(args, message);
    vsnprintf(content, LOG_BUFFER, message, args);
    va_end(args);

    if (level <= conf->ui_log_level)
        ui_message("%s\n", content);

    pthread_mutex_unlock(&log_lock);
}

void log_start() {
    ui_message("------- START -------\n");
}

void log_stop() {
    ui_message("-------- END --------\n");
}
