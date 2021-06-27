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


#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "http.h"

#define BUFFER_SIZE 16

const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_http_url_concat, test_http_setup, test_http_teardown),
        cmocka_unit_test_setup_teardown(test_http_init, test_http_setup, test_http_teardown),
};

int main() {
    return cmocka_run_group_tests_name("http", tests, test_http_group_setup, test_http_group_teardown);
}

static int test_http_group_setup(void **state) {
    int result;

    (void) state;

    result = http_global_init();

    return result;
}

static int test_http_group_teardown(void **state) {
    (void) state;

    http_global_cleanup();

    return EXIT_SUCCESS;
}

static int test_http_setup(void **state) {
    http_ctx *ctx;

    ctx = http_init(URL_PREFIX);
    if (ctx == NULL)
        return EXIT_FAILURE;

    *state = (void *) ctx;

    return EXIT_SUCCESS;
}

static int test_http_teardown(void **state) {
    http_ctx *ctx;

    ctx = (http_ctx *) *state;
    if (ctx != NULL)
        http_free(ctx);

    return EXIT_SUCCESS;
}

void test_http_url_concat(void **state) {
    http_ctx *ctx;
    char *input;
    char *expected;
    char actual[HTTP_URL_MAX_SIZE];
    int result;

    ctx = (http_ctx *) *state;

    input = NULL;
    expected = "";
    result = http_url_concat(ctx, actual, HTTP_URL_MAX_SIZE, input);
    assert_int_equal(result, EXIT_FAILURE);
    assert_memory_equal(expected, actual, strlen(expected));

    input = "";
    expected = URL_PREFIX;
    result = http_url_concat(ctx, actual, HTTP_URL_MAX_SIZE, input);
    assert_int_equal(result, EXIT_SUCCESS);
    assert_memory_equal(expected, actual, strlen(expected));

    input = URL_TEST_1_INPUT;
    expected = URL_TEST_1_EXPECTED;
    result = http_url_concat(ctx, actual, HTTP_URL_MAX_SIZE, input);
    assert_int_equal(result, EXIT_SUCCESS);
    assert_memory_equal(expected, actual, strlen(expected));
}

void test_http_init(void **state) {
    http_ctx *ctx;
    int status_code;
    char *request_body;
    http_data *response_data;

    ctx = (http_ctx *) *state;

    request_body = BODY_EMPTY;

    response_data = http_data_init();

    status_code = http_do_call(ctx, HTTP_POST, URL_API_TEST, request_body, response_data);
    assert_true(status_code >= 0);

    assert_int_equal(strlen(BODY_EMPTY), response_data->size);
    assert_memory_equal(BODY_EMPTY, response_data->data, response_data->size);

    http_data_free(response_data);
}
