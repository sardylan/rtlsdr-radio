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

#include "greatbuf.h"

#define BUFFER_SIZE 16

const struct CMUnitTest tests[] = {
        cmocka_unit_test_setup_teardown(test_greatbuf_circbuf_init,
                                        test_greatbuf_circbuf_setup,
                                        test_greatbuf_circbuf_teardown),
        cmocka_unit_test_setup_teardown(test_greatbuf_circbuf_head_acquire,
                                        test_greatbuf_circbuf_setup,
                                        test_greatbuf_circbuf_teardown),
        cmocka_unit_test_setup_teardown(test_greatbuf_circbuf_head_release,
                                        test_greatbuf_circbuf_setup,
                                        test_greatbuf_circbuf_teardown),
        cmocka_unit_test_setup_teardown(test_greatbuf_circbuf_tail_acquire,
                                        test_greatbuf_circbuf_setup,
                                        test_greatbuf_circbuf_teardown),
        cmocka_unit_test_setup_teardown(test_greatbuf_circbuf_tail_release,
                                        test_greatbuf_circbuf_setup,
                                        test_greatbuf_circbuf_teardown),
        cmocka_unit_test_setup_teardown(test_greatbuf_circbuf_multiple_head_acquire,
                                        test_greatbuf_circbuf_setup,
                                        test_greatbuf_circbuf_teardown),
        cmocka_unit_test_setup_teardown(test_greatbuf_circbuf_max_head_acquire,
                                        test_greatbuf_circbuf_setup,
                                        test_greatbuf_circbuf_teardown),
        cmocka_unit_test_setup_teardown(test_greatbuf_circbuf_rotation_head_acquire,
                                        test_greatbuf_circbuf_setup,
                                        test_greatbuf_circbuf_teardown),
        cmocka_unit_test_setup_teardown(test_greatbuf_init, test_greatbuf_setup, test_greatbuf_teardown),
};

int main() {
    return cmocka_run_group_tests_name("greatbuf", tests, NULL, NULL);
}

static int test_greatbuf_circbuf_setup(void **state) {
    greatbuf_circbuf *ctx;

    ctx = greatbuf_circbuf_init(TEST_GREATBUF_CIRCBUF_NAME, TEST_GREATBUF_CIRCBUF_SIZE);
    if (ctx == NULL)
        return EXIT_FAILURE;

    *state = (void *) ctx;

    return EXIT_SUCCESS;
}

static int test_greatbuf_circbuf_teardown(void **state) {
    greatbuf_circbuf *ctx;

    ctx = (greatbuf_circbuf *) *state;
    if (ctx != NULL)
        greatbuf_circbuf_free(ctx);

    return EXIT_SUCCESS;
}

static int test_greatbuf_setup(void **state) {
    test_greatbuf_state *test_state;

    test_state = (test_greatbuf_state *) malloc(sizeof(test_greatbuf_state));
    if (test_state == NULL)
        return EXIT_FAILURE;

    test_state->ctx = greatbuf_init(TEST_GREATBUF_BUFFER_SIZE,
                                    TEST_GREATBUF_RTLSDR_SAMPLES,
                                    TEST_GREATBUF_PCM_SAMPLES,
                                    TEST_GREATBUF_DATA_SIZE);
    if (test_state->ctx == NULL) {
        free(test_state);
        return EXIT_FAILURE;
    }

    *state = (void *) test_state;

    return EXIT_SUCCESS;
}

static int test_greatbuf_teardown(void **state) {
    test_greatbuf_state *test_state;

    test_state = (test_greatbuf_state *) *state;

    if (test_state != NULL) {
        if (test_state->ctx != NULL) {
            greatbuf_free(test_state->ctx);
        }

        free(test_state);
    }

    return EXIT_SUCCESS;
}

void test_greatbuf_circbuf_init(void **state) {
    greatbuf_circbuf *ctx;

    ctx = (greatbuf_circbuf *) *state;

    assert_non_null(ctx);

    assert_memory_equal(TEST_GREATBUF_CIRCBUF_NAME, ctx->name, strlen(TEST_GREATBUF_CIRCBUF_NAME));

    assert_int_equal(0, ctx->head);
    assert_int_equal(0, ctx->tail);

    assert_int_equal(TEST_GREATBUF_BUFFER_SIZE, ctx->size);
    assert_int_equal(TEST_GREATBUF_BUFFER_SIZE, ctx->free);

    assert_int_equal(0, ctx->busy_head);
    assert_int_equal(0, ctx->busy_tail);

    assert_int_equal(1, ctx->keep_running);
}

void test_greatbuf_circbuf_head_acquire(void **state) {
    greatbuf_circbuf *ctx;
    ssize_t pos;

    ctx = (greatbuf_circbuf *) *state;

    pos = greatbuf_circbuf_head_acquire(ctx);

    assert_int_equal(0, pos);

    assert_int_equal(0, ctx->head);
    assert_int_equal(0, ctx->tail);

    assert_int_equal(TEST_GREATBUF_BUFFER_SIZE, ctx->free);

    assert_int_equal(1, ctx->busy_head);
    assert_int_equal(0, ctx->busy_tail);
}

void test_greatbuf_circbuf_head_release(void **state) {
    greatbuf_circbuf *ctx;

    ctx = (greatbuf_circbuf *) *state;

    greatbuf_circbuf_head_acquire(ctx);
    greatbuf_circbuf_head_release(ctx);

    assert_int_equal(1, ctx->head);
    assert_int_equal(0, ctx->tail);

    assert_int_equal(TEST_GREATBUF_BUFFER_SIZE - 1, ctx->free);

    assert_int_equal(0, ctx->busy_head);
    assert_int_equal(0, ctx->busy_tail);
}

void test_greatbuf_circbuf_tail_acquire(void **state) {
    greatbuf_circbuf *ctx;
    ssize_t pos;

    ctx = (greatbuf_circbuf *) *state;

    greatbuf_circbuf_head_acquire(ctx);
    greatbuf_circbuf_head_release(ctx);
    pos = greatbuf_circbuf_tail_acquire(ctx);

    assert_int_equal(0, pos);

    assert_int_equal(1, ctx->head);
    assert_int_equal(0, ctx->tail);

    assert_int_equal(TEST_GREATBUF_BUFFER_SIZE - 1, ctx->free);

    assert_int_equal(0, ctx->busy_head);
    assert_int_equal(1, ctx->busy_tail);
}

void test_greatbuf_circbuf_tail_release(void **state) {
    greatbuf_circbuf *ctx;

    ctx = (greatbuf_circbuf *) *state;

    greatbuf_circbuf_head_acquire(ctx);
    greatbuf_circbuf_head_release(ctx);
    greatbuf_circbuf_tail_acquire(ctx);
    greatbuf_circbuf_tail_release(ctx);

    assert_int_equal(1, ctx->head);
    assert_int_equal(1, ctx->tail);

    assert_int_equal(TEST_GREATBUF_BUFFER_SIZE, ctx->free);

    assert_int_equal(0, ctx->busy_head);
    assert_int_equal(0, ctx->busy_tail);
}

void test_greatbuf_circbuf_multiple_head_acquire(void **state) {
    greatbuf_circbuf *ctx;
    size_t i;

    ctx = (greatbuf_circbuf *) *state;

    for (i = 0; i < TEST_GREATBUF_MULTIPLE_ITERATIONS; i++) {
        greatbuf_circbuf_head_acquire(ctx);
        greatbuf_circbuf_head_release(ctx);
    }

    assert_int_equal(TEST_GREATBUF_MULTIPLE_ITERATIONS, ctx->head);
    assert_int_equal(0, ctx->tail);

    assert_int_equal(TEST_GREATBUF_BUFFER_SIZE - TEST_GREATBUF_MULTIPLE_ITERATIONS, ctx->free);

    assert_int_equal(0, ctx->busy_head);
    assert_int_equal(0, ctx->busy_tail);
}

void test_greatbuf_circbuf_max_head_acquire(void **state) {
    greatbuf_circbuf *ctx;
    size_t i;

    ctx = (greatbuf_circbuf *) *state;

    for (i = 0; i < TEST_GREATBUF_CIRCBUF_SIZE; i++) {
        greatbuf_circbuf_head_acquire(ctx);
        greatbuf_circbuf_head_release(ctx);
    }

    assert_int_equal(0, ctx->head);
    assert_int_equal(0, ctx->tail);

    assert_int_equal(0, ctx->free);

    assert_int_equal(0, ctx->busy_head);
    assert_int_equal(0, ctx->busy_tail);
}

void test_greatbuf_circbuf_rotation_head_acquire(void **state) {
    greatbuf_circbuf *ctx;
    size_t i;

    ctx = (greatbuf_circbuf *) *state;

    for (i = 0; i < TEST_GREATBUF_CIRCBUF_SIZE - TEST_GREATBUF_MULTIPLE_ITERATIONS; i++) {
        greatbuf_circbuf_head_acquire(ctx);
        greatbuf_circbuf_head_release(ctx);
    }

    assert_int_equal(TEST_GREATBUF_CIRCBUF_SIZE - TEST_GREATBUF_MULTIPLE_ITERATIONS, ctx->head);
    assert_int_equal(0, ctx->tail);

    assert_int_equal(TEST_GREATBUF_MULTIPLE_ITERATIONS, ctx->free);

    for (i = 0; i < TEST_GREATBUF_CIRCBUF_SIZE - TEST_GREATBUF_MULTIPLE_ITERATIONS * 2; i++) {
        greatbuf_circbuf_tail_acquire(ctx);
        greatbuf_circbuf_tail_release(ctx);
    }

    assert_int_equal(TEST_GREATBUF_CIRCBUF_SIZE - TEST_GREATBUF_MULTIPLE_ITERATIONS, ctx->head);
    assert_int_equal(TEST_GREATBUF_CIRCBUF_SIZE - TEST_GREATBUF_MULTIPLE_ITERATIONS * 2, ctx->tail);

    assert_int_equal(TEST_GREATBUF_CIRCBUF_SIZE - TEST_GREATBUF_MULTIPLE_ITERATIONS, ctx->free);

    for (i = 0; i < TEST_GREATBUF_MULTIPLE_ITERATIONS * 3; i++) {
        greatbuf_circbuf_head_acquire(ctx);
        greatbuf_circbuf_head_release(ctx);
    }

    assert_int_equal(TEST_GREATBUF_MULTIPLE_ITERATIONS * 2, ctx->head);
    assert_int_equal(TEST_GREATBUF_CIRCBUF_SIZE - TEST_GREATBUF_MULTIPLE_ITERATIONS * 2, ctx->tail);

    assert_int_equal(TEST_GREATBUF_MULTIPLE_ITERATIONS * 4, ctx->free);
}

void test_greatbuf_init(void **state) {
    test_greatbuf_state *test_state;
    greatbuf_ctx *ctx;

    test_state = (test_greatbuf_state *) *state;
    ctx = test_state->ctx;

    assert_non_null(ctx);
    assert_int_equal(TEST_GREATBUF_BUFFER_SIZE, ctx->size);
    assert_non_null(ctx->items);
}
