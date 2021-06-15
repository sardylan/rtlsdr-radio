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
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include <libgen.h>

#include "log.h"
#include "cfg.h"
#include "ui.h"

pthread_mutex_t log_lock;
FILE *log_fp;

extern cfg *conf;

void log_init() {
    pthread_mutex_init(&log_lock, NULL);

    log_fp = NULL;
}

void log_free() {
    pthread_mutex_destroy(&log_lock);
}

void log_message(const int level, const char *element, char *filename, const int row, const char *message, ...) {
    va_list args;
    char thread_name[16];
    char datetime[27];
    struct tm *timeinfo;
    struct timespec ts;
    char content[LOG_BUFFER];
    char prefix[1025];

    size_t ln;
    size_t i;

    if (level > conf->ui_log_level && level > conf->file_log_level)
        return;

    /*
     * TODO: Temporary block unuseful logs
     *       To be removed
     */

    if (strncmp(element, "circbuf_", 8) == 0 && level >= LOG_LEVEL_DEBUG) return;
    if (strncmp(element, "greatbuf_", 9) == 0 && level >= LOG_LEVEL_DEBUG) return;
    if (strncmp(element, "fft_", 4) == 0 && level >= LOG_LEVEL_DEBUG) return;
    if (strncmp(element, "resample_", 9) == 0 && level >= LOG_LEVEL_DEBUG) return;
    if (strncmp(element, "wav_", 4) == 0 && level >= LOG_LEVEL_DEBUG) return;

    if (strcmp(element, "thread_rx_read") == 0 && level >= LOG_LEVEL_TRACE) return;
    if (strcmp(element, "thread_rx_samples") == 0 && level >= LOG_LEVEL_DEBUG) return;
    if (strcmp(element, "thread_rx_demod") == 0 && level >= LOG_LEVEL_DEBUG) return;
    if (strcmp(element, "thread_rx_filter") == 0 && level >= LOG_LEVEL_DEBUG) return;
    if (strcmp(element, "thread_rx_resample") == 0 && level >= LOG_LEVEL_DEBUG) return;
    if (strcmp(element, "thread_rx_audio") == 0 && level >= LOG_LEVEL_DEBUG) return;

    pthread_mutex_lock(&log_lock);

    timespec_get(&ts, TIME_UTC);

    timeinfo = localtime(&ts.tv_sec);
    strftime(datetime, 20, "%Y-%m-%d %H:%M:%S", timeinfo);
    sprintf(datetime + 19, ".%06lu", ts.tv_nsec / 1000);

    prctl(PR_GET_NAME, thread_name);
    ln = strlen(thread_name);
    for (i = ln; i < 16; i++)
        thread_name[i] = ' ';
    thread_name[15] = '\0';

    sprintf(prefix, "%s (%lx [%s]) [%s] {%s} (%s:%d) ",
            datetime,
            pthread_self(),
            thread_name,
            cfg_tochar_log_level(level),
            element,
            basename(filename),
            row);

    memset(content, '\0', sizeof(content));

    va_start(args, message);
    vsnprintf(content, LOG_BUFFER, message, args);
    va_end(args);

    if (level <= conf->ui_log_level)
        ui_message("%s%s\n", prefix, content);

    if (log_fp != NULL && level <= conf->file_log_level)
        fprintf(log_fp, "%s%s\n", prefix, content);

    pthread_mutex_unlock(&log_lock);
}

void log_start() {
    pthread_mutex_lock(&log_lock);

    ui_message("------- START -------\n");

    log_start_file();

    pthread_mutex_unlock(&log_lock);
}

void log_stop() {
    pthread_mutex_lock(&log_lock);

    log_stop_file();

    ui_message("-------- END --------\n");

    pthread_mutex_unlock(&log_lock);
}

void log_start_file() {
    char *dir_name;
    struct stat dir_stat;
    int dir_res;

    if (conf->file_log_name == NULL || strlen(conf->file_log_name) <= 0) {
        ui_message("Invalid log file name!\n");
        return;
    }

    dir_name = dirname(conf->file_log_name);
    stat(dir_name, &dir_stat);

    if (S_ISDIR(dir_stat.st_mode) == 0) {
        ui_message("Invalid log directory!\n");
        return;
    }

    dir_res = access(dir_name, R_OK | W_OK | X_OK);
    if (dir_res != 0) {
        ui_message("Invalid permissions on log directory!\n");
        return;
    }

    log_fp = fopen(conf->file_log_name, "a");
    if (log_fp == NULL) {
        ui_message("Unable to open log file!\n");
        return;
    }

    fprintf(log_fp, "\n\n------- START -------\n");
}

void log_stop_file() {
    if (log_fp != NULL) {
        fprintf(log_fp, "-------- END --------\n");
        fclose(log_fp);
    }
}
