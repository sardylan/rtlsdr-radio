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


#ifndef __RTLSDR_RADIO__HTTP__H
#define __RTLSDR_RADIO__HTTP__H

#include <stdlib.h>

#include <curl/curl.h>

#include "version.h"

#define HTTP_TOKEN_DEFAULT ""
#define HTTP_URL_MAX_SIZE 1024
#define HTTP_HEADER_MAX_SIZE 1024

#define HTTP_USER_AGENT() APPLICATION_NAME " " APPLICATION_VERSION " HTTPClient (libcurl)"
#define http_user_agent() HTTP_USER_AGENT()

struct http_ctx_t {
    char *url_prefix;
    char *token;
};

typedef struct http_ctx_t http_ctx;

struct http_data_t {
    char *data;
    size_t size;
};

typedef struct http_data_t http_data;

enum http_method_t {
    HTTP_GET,
    HTTP_POST
};

typedef enum http_method_t http_method;

int http_global_init();

void http_global_cleanup();

http_ctx *http_init(char *);

void http_free(http_ctx *);

http_data *http_data_init();

void http_data_free(http_data *);

int http_url_concat(http_ctx *, char *, size_t, const char *);

char *http_method_string(http_method);

void http_headers_add(struct curl_slist **, char *, ...);

size_t http_callback_write(char *, size_t, size_t, void *);

int http_do_call(http_ctx *, http_method, const char *, const char *, http_data *);

#endif
