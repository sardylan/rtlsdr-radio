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


#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <stdarg.h>

#include "http.h"

#include "log.h"
#include "ui.h"

int http_global_init() {
    int result;
    CURLsslset ret;

#ifndef __RTLSDR__TESTS
    char *message;
#endif

    result = EXIT_SUCCESS;

    ret = curl_global_sslset(CURLSSLBACKEND_OPENSSL, NULL, NULL);
    if (ret != CURLSSLSET_OK) {

#ifndef __RTLSDR__TESTS
        switch (ret) {
            case CURLSSLSET_UNKNOWN_BACKEND:
                message = "Unknown backend";
                break;
            case CURLSSLSET_TOO_LATE:
                message = "Too late";
                break;
            case CURLSSLSET_NO_BACKENDS:
                message = "No backends";
                break;
            default:
                message = "";
                break;
        }

        ui_message("Unable to setup SSL for curl: %s\n", message);
#endif

        return EXIT_FAILURE;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);

    return result;
}

void http_global_cleanup() {
    curl_global_cleanup();
}

http_ctx *http_init(const char *url_prefix, const char *api_prefix) {
    http_ctx *ctx;
    size_t ln;

    log_info("Initializing context");

    log_debug("Allocating context");
    ctx = (http_ctx *) malloc(sizeof(http_ctx));
    if (ctx == NULL) {
        log_error("Unable to allocate frame");
        return NULL;
    }

    ctx->url_prefix = NULL;
    ctx->token = NULL;

    log_debug("Allocating URL prefix");
    ln = strlen(api_prefix);
    ctx->api_prefix = (char *) realloc(ctx->api_prefix, (ln + 1) * sizeof(char));
    if (ctx->api_prefix == NULL) {
        log_error("Unable to allocate API prefix");
        http_free(ctx);
        return NULL;
    }
    strcpy(ctx->api_prefix, api_prefix);

    log_debug("Allocating API prefix");
    ln = strlen(url_prefix);
    ctx->url_prefix = (char *) realloc(ctx->url_prefix, (ln + 1) * sizeof(char));
    if (ctx->url_prefix == NULL) {
        log_error("Unable to allocate URL prefix");
        http_free(ctx);
        return NULL;
    }
    strcpy(ctx->url_prefix, url_prefix);

    log_debug("Allocating token");
    ln = strlen(HTTP_TOKEN_DEFAULT);
    ctx->token = (char *) realloc(ctx->token, (ln + 1) * sizeof(char));
    if (ctx->token == NULL) {
        log_error("Unable to allocate token");
        http_free(ctx);
        return NULL;
    }
    strcpy(ctx->token, HTTP_TOKEN_DEFAULT);

    return ctx;
}

void http_free(http_ctx *ctx) {
    log_info("Freeing context");

    if (ctx == NULL)
        return;

    log_debug("Freeing token");
    if (ctx->token != NULL)
        free(ctx->token);

    log_debug("Freeing URL prefix");
    if (ctx->url_prefix != NULL)
        free(ctx->url_prefix);

    log_debug("Freeing API prefix");
    if (ctx->api_prefix != NULL)
        free(ctx->api_prefix);

    log_debug("Freeing context");
    free(ctx);
}

http_data *http_data_init() {
    http_data *data;

    data = (http_data *) malloc(sizeof(http_data));
    if (data == NULL) {
        log_error("Unable to allocate data");
        return NULL;
    }

    data->data = NULL;
    data->size = 0;

    return data;
}

void http_data_free(http_data *data) {
    if (data == NULL)
        return;

    if (data->data != NULL)
        free(data->data);

    free(data);
}

int http_url_concat(http_ctx *ctx, char *url, size_t url_max_size, const char *api_url) {
    size_t ln;

    bzero(url, url_max_size);

    if (api_url == NULL)
        return EXIT_FAILURE;

    ln = strlen(ctx->url_prefix) + strlen(ctx->api_prefix) + strlen(api_url);
    if (ln > url_max_size) {
        log_error("Not enough space to concat url prefix, api prefix and api");
        return EXIT_FAILURE;
    }

    strcat(url, ctx->url_prefix);
    strcat(url, ctx->api_prefix);
    strcat(url, api_url);

    return EXIT_SUCCESS;
}

char *http_method_string(http_method method) {
    switch (method) {
        case HTTP_GET:
            return "GET";
        case HTTP_POST:
            return "POST";
        default:
            return "";
    }
}

void http_headers_add(struct curl_slist **headers, char *format, ...) {
    char header_buffer[HTTP_HEADER_MAX_SIZE];
    va_list args;

    va_start(args, format);
    vsprintf(header_buffer, format, args);
    va_end(args);

    *headers = curl_slist_append(*headers, header_buffer);
}

size_t http_callback_write(char *raw_data, size_t raw_data_size, size_t raw_data_num_members, void *http_data_ptr) {
    http_data *data;
    size_t size_real;
    size_t ln;

    data = (http_data *) http_data_ptr;
    size_real = raw_data_size * raw_data_num_members;
    ln = data->size + size_real + 1;

    data->data = (char *) realloc(data->data, ln);
    if (data->data == NULL) {
        log_error("Unable to reallocate data for storing HTTP response");
        return 0;
    }

    memcpy(data->data + data->size, raw_data, size_real);
    data->size += size_real;

    data->data[data->size] = '\0';

    return size_real;
}

int http_do_call(http_ctx *ctx,
                 const http_method method,
                 const char *api_url,
                 const char *request_body,
                 http_data *response_data) {
    int res;

    CURL *curl;
    char url[HTTP_URL_MAX_SIZE];
    struct curl_slist *headers;
    CURLcode curl_res;
    long status_code;
    size_t request_body_size;

    int result;

    headers = NULL;
    res = -EXIT_FAILURE;

    result = http_url_concat(ctx, url, HTTP_URL_MAX_SIZE, api_url);
    if (result != EXIT_SUCCESS) {
        log_error("Unable to prepare URL");
        return EXIT_FAILURE;
    }

    curl = curl_easy_init();
    if (curl == NULL) {
        log_error("Unable to init curl");
        return EXIT_FAILURE;
    }

    http_headers_add(&headers, "Accept: application/json");

    if (strlen(ctx->token) > 0)
        http_headers_add(&headers, "X-Auth-Token: %s", ctx->token);

    curl_easy_setopt(curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, http_callback_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) response_data);

    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, http_method_string(method));
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, HTTP_USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    if (method == HTTP_POST)
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

    request_body_size = 0;
    if (request_body != NULL) {
        request_body_size = strlen(request_body);
        http_headers_add(&headers, "Content-Type: application/json; Charset=UTF-8");
        http_headers_add(&headers, "Content-Length: %llu", request_body_size);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body);
    }

    curl_res = curl_easy_perform(curl);
    if (curl_res == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
        res = (int) status_code;
        log_debug("HTTP Status code: %zu", status_code);
        log_debug("HTTP Response: %zu bytes", response_data->size);
    } else {
        res = (int) -curl_res;
        log_error("HTTP request error %d: %s", curl_res, curl_easy_strerror(curl_res));
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    return res;
}
