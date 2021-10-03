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


#ifndef __RTLSDR_RADIO__HTTP__H__TEST
#define __RTLSDR_RADIO__HTTP__H__TEST

#include "../src/http.h"

#define URL_PREFIX "http://127.0.0.1:8000/app"
#define URL_API_TEST "/api/public/v1/test"

#define TEST_1_URL_INPUT "/test"
#define TEST_1_URL_EXPECTED URL_PREFIX TEST_1_URL_INPUT

#define TEST_REQUEST_BODY_EMPTY "{}"
#define TEST_REQUEST_BODY_PARAM_VALUE "{\"param\": \"value\"}"

#define TEST_RESPONSE_BODY_GET "{\"test\": \"test\"}"

static int test_http_group_setup(void **);

static int test_http_group_teardown(void **);

static int test_http_setup(void **);

static int test_http_teardown(void **);

void test_http_url_concat(void **);

void test_http_get(void **);

void test_http_post_json_empty(void **);

void test_http_post_json_param_value(void **);

#endif
