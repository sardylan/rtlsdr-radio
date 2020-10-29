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

#include "log.h"
#include "cfg.h"
#include "ui.h"

extern cfg *conf;

void log_message(const int level, const char *element, const char *message, ...) {
    va_list args;
    char datetime[20];
    time_t rawtime;
    struct tm *timeinfo;
    char content[131072];
    char prefix[1025];
    char *level_string;

    if (level <= conf->ui_log_level || level <= conf->file_log_level) {
        rawtime = time(NULL);
        timeinfo = localtime(&rawtime);
        strftime(datetime, 20, "%Y-%m-%d %H:%M:%S", timeinfo);

        level_string = log_level_to_char(level);
        sprintf(prefix, "%s [%s] {%s} ", datetime, level_string, element);

        fprintf(UI_MESSAGES_OUTPUT, "%s", prefix);

        memset(content, '\0', sizeof(content));

        va_start(args, message);
        vsprintf(content, message, args);
        va_end(args);

        if (level <= conf->ui_log_level)
            fprintf(UI_MESSAGES_OUTPUT, "%s\n", content);
    }

//    log_file_message(level, prefix, content);
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