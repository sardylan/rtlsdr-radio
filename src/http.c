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
    char *message;

    result = EXIT_SUCCESS;

    ret = curl_global_sslset(CURLSSLBACKEND_OPENSSL, NULL, NULL);
    if (ret != CURLSSLSET_OK) {
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

        return EXIT_FAILURE;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);

    return result;
}

void http_global_cleanup() {
    curl_global_cleanup();
}

http_ctx *http_init(char *url_prefix) {
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
    ln = strlen(url_prefix);
    ctx->url_prefix = (char *) realloc(ctx->url_prefix, (ln + 1) * sizeof(char));
    if (ctx->url_prefix == NULL) {
        log_error("Unable to allocate URL prefix");
        http_free(ctx);
        return NULL;
    }
    strcpy(ctx->url_prefix, url_prefix);

    log_debug("Allocating URL prefix");
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

    log_debug("Freeing context");
    free(ctx);
}

int http_url_concat(http_ctx *ctx, char *url, size_t url_max_size, const char *api_url) {
    size_t ln;

    ln = strlen(ctx->url_prefix) + strlen(api_url);
    if (ln > url_max_size) {
        log_error("Not enough space to concat url prefix and api");
        return EXIT_FAILURE;
    }

    bzero(url, url_max_size);
    strcat(url, ctx->url_prefix);
    strcat(url, api_url);

    return EXIT_SUCCESS;
}

void http_headers_add(struct curl_slist **headers, char *format, ...) {
    char header_buffer[HTTP_HEADER_MAX_SIZE];
    va_list args;

    va_start(args, format);
    vsprintf(header_buffer, format, args);
    va_end(args);

    *headers = curl_slist_append(*headers, header_buffer);
}

int http_post(http_ctx *ctx, const char *api_url, const char *request_body,
              const char *response_body, size_t response_body_size) {
    CURL *curl;
    char url[HTTP_URL_MAX_SIZE];
    struct curl_slist *headers;
    CURLcode res;

    int result;

    headers = NULL;

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

    http_headers_add(&headers, "X-Auth-Token: %s", ctx->token);
    http_headers_add(&headers, "Content-Type: application/json; Charset=UTF-8");
    http_headers_add(&headers, "Accept: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, http_user_agent());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body);

    //TODO: Implement callbacks for reading headers and resposne body

    res = curl_easy_perform(curl);

    response_body = NULL;
    response_body_size = 0;

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    return 0;
}
