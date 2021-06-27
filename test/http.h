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

#define URL_PREFIX "http://127.0.0.1:8000"
#define URL_API_TEST "/api/publiv/v1/test"

#define URL_TEST_1_INPUT "/test"
#define URL_TEST_1_EXPECTED URL_PREFIX URL_TEST_1_INPUT

#define BODY_EMPTY "{}"

static int test_http_group_setup(void **);

static int test_http_group_teardown(void **);

static int test_http_setup(void **);

static int test_http_teardown(void **);

void test_http_url_concat(void **);

void test_http_init(void **);

#endif
