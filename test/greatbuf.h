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


#ifndef __RTLSDR_RADIO__GREATBUF__H__TEST
#define __RTLSDR_RADIO__GREATBUF__H__TEST

#include "../src/greatbuf.h"

#define TEST_GREATBUF_CIRCBUF_NAME "testbuf"
#define TEST_GREATBUF_CIRCBUF_SIZE 1024

#define TEST_GREATBUF_BUFFER_SIZE 1024
#define TEST_GREATBUF_RTLSDR_SAMPLES 1024
#define TEST_GREATBUF_PCM_SAMPLES 1024
#define TEST_GREATBUF_DATA_SIZE 1024

#define TEST_GREATBUF_MULTIPLE_ITERATIONS 128

struct test_greatbuf_state_t {
    greatbuf_ctx *ctx;
};

typedef struct test_greatbuf_state_t test_greatbuf_state;

static int test_greatbuf_circbuf_setup(void **);

static int test_greatbuf_circbuf_teardown(void **);

static int test_greatbuf_setup(void **);

static int test_greatbuf_teardown(void **);

void test_greatbuf_circbuf_init(void **);

void test_greatbuf_circbuf_head_acquire(void **);

void test_greatbuf_circbuf_head_release(void **);

void test_greatbuf_circbuf_tail_acquire(void **);

void test_greatbuf_circbuf_tail_release(void **);

void test_greatbuf_circbuf_multiple_head_acquire(void **);

void test_greatbuf_circbuf_max_head_acquire(void **);

void test_greatbuf_circbuf_rotation_head_acquire(void **);

void test_greatbuf_init(void **);

#endif
