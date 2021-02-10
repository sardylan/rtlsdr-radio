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
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

#define BUFFER_SIZE 16

const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_stricmp),
        cmocka_unit_test(test_utils_ltrim),
        cmocka_unit_test(test_utils_rtrim),
        cmocka_unit_test(test_utils_trim),
};

int main() {
    return cmocka_run_group_tests(tests, NULL, NULL);
}

void test_stricmp(void **state) {
    (void) state;

    assert_true(utils_stricmp(NULL, NULL) == 0);
    assert_true(utils_stricmp(NULL, "") < 0);
    assert_true(utils_stricmp(NULL, "a") < 0);
    assert_true(utils_stricmp("", NULL) > 0);
    assert_true(utils_stricmp("", "") == 0);
    assert_true(utils_stricmp("", "a") < 0);
    assert_true(utils_stricmp("a", NULL) > 0);
    assert_true(utils_stricmp("a", "") > 0);
    assert_true(utils_stricmp("a", "a") == 0);

    assert_int_equal(0, utils_stricmp("test", "test"));
    assert_int_equal(0, utils_stricmp("test", "tesT"));
    assert_int_equal(0, utils_stricmp("test", "teSt"));
    assert_int_equal(0, utils_stricmp("test", "teST"));
    assert_int_equal(0, utils_stricmp("test", "tEst"));
    assert_int_equal(0, utils_stricmp("test", "tEsT"));
    assert_int_equal(0, utils_stricmp("test", "tESt"));
    assert_int_equal(0, utils_stricmp("test", "tEST"));
    assert_int_equal(0, utils_stricmp("test", "Test"));
    assert_int_equal(0, utils_stricmp("test", "TesT"));
    assert_int_equal(0, utils_stricmp("test", "TeSt"));
    assert_int_equal(0, utils_stricmp("test", "TeST"));
    assert_int_equal(0, utils_stricmp("test", "TEst"));
    assert_int_equal(0, utils_stricmp("test", "TEsT"));
    assert_int_equal(0, utils_stricmp("test", "TESt"));
    assert_int_equal(0, utils_stricmp("test", "TEST"));

    assert_int_equal(0, utils_stricmp("test", "test"));
    assert_int_equal(0, utils_stricmp("tesT", "test"));
    assert_int_equal(0, utils_stricmp("teSt", "test"));
    assert_int_equal(0, utils_stricmp("teST", "test"));
    assert_int_equal(0, utils_stricmp("tEst", "test"));
    assert_int_equal(0, utils_stricmp("tEsT", "test"));
    assert_int_equal(0, utils_stricmp("tESt", "test"));
    assert_int_equal(0, utils_stricmp("tEST", "test"));
    assert_int_equal(0, utils_stricmp("Test", "test"));
    assert_int_equal(0, utils_stricmp("TesT", "test"));
    assert_int_equal(0, utils_stricmp("TeSt", "test"));
    assert_int_equal(0, utils_stricmp("TeST", "test"));
    assert_int_equal(0, utils_stricmp("TEst", "test"));
    assert_int_equal(0, utils_stricmp("TEsT", "test"));
    assert_int_equal(0, utils_stricmp("TESt", "test"));
    assert_int_equal(0, utils_stricmp("TEST", "test"));

    assert_true(utils_stricmp("abcd", "abce") < 0);
    assert_true(utils_stricmp("abcd", "abcc") > 0);

    assert_true(utils_stricmp("test", "tes") > 0);
    assert_true(utils_stricmp("tes", "test") < 0);
}

void test_utils_ltrim(void **state) {
    (void) state;

    char input[BUFFER_SIZE];
    char expected[BUFFER_SIZE];
    char actual[BUFFER_SIZE];

    bzero(input, BUFFER_SIZE);
    bzero(expected, BUFFER_SIZE);
    bzero(actual, BUFFER_SIZE);

    assert_int_equal(EXIT_FAILURE, utils_ltrim(NULL, NULL, 0));
    assert_int_equal(EXIT_FAILURE, utils_ltrim(NULL, input, 0));
    assert_int_equal(EXIT_FAILURE, utils_ltrim(actual, NULL, 0));
    assert_int_equal(EXIT_FAILURE, utils_ltrim(actual, input, 0));

    assert_int_equal(EXIT_FAILURE, utils_ltrim(NULL, NULL, BUFFER_SIZE));
    assert_int_equal(EXIT_FAILURE, utils_ltrim(NULL, input, BUFFER_SIZE));
    assert_int_equal(EXIT_FAILURE, utils_ltrim(actual, NULL, BUFFER_SIZE));

    strcpy(input, "");
    strcpy(expected, "");
    assert_int_equal(EXIT_SUCCESS, utils_ltrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, " ");
    strcpy(expected, "");
    assert_int_equal(EXIT_SUCCESS, utils_ltrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, "a");
    strcpy(expected, "a");
    assert_int_equal(EXIT_SUCCESS, utils_ltrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, "a ");
    strcpy(expected, "a ");
    assert_int_equal(EXIT_SUCCESS, utils_ltrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, " a");
    strcpy(expected, "a");
    assert_int_equal(EXIT_SUCCESS, utils_ltrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, " a ");
    strcpy(expected, "a ");
    assert_int_equal(EXIT_SUCCESS, utils_ltrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, "ab");
    strcpy(expected, "ab");
    assert_int_equal(EXIT_SUCCESS, utils_ltrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, "ab ");
    strcpy(expected, "ab ");
    assert_int_equal(EXIT_SUCCESS, utils_ltrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, " ab");
    strcpy(expected, "ab");
    assert_int_equal(EXIT_SUCCESS, utils_ltrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, " ab ");
    strcpy(expected, "ab ");
    assert_int_equal(EXIT_SUCCESS, utils_ltrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, "a b");
    strcpy(expected, "a b");
    assert_int_equal(EXIT_SUCCESS, utils_ltrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, "a b ");
    strcpy(expected, "a b ");
    assert_int_equal(EXIT_SUCCESS, utils_ltrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, " a b");
    strcpy(expected, "a b");
    assert_int_equal(EXIT_SUCCESS, utils_ltrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, " a b ");
    strcpy(expected, "a b ");
    assert_int_equal(EXIT_SUCCESS, utils_ltrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);
}

void test_utils_rtrim(void **state) {
    (void) state;

    char input[BUFFER_SIZE];
    char expected[BUFFER_SIZE];
    char actual[BUFFER_SIZE];

    bzero(input, BUFFER_SIZE);
    bzero(expected, BUFFER_SIZE);
    bzero(actual, BUFFER_SIZE);

    assert_int_equal(EXIT_FAILURE, utils_rtrim(NULL, NULL, 0));
    assert_int_equal(EXIT_FAILURE, utils_rtrim(NULL, input, 0));
    assert_int_equal(EXIT_FAILURE, utils_rtrim(actual, NULL, 0));
    assert_int_equal(EXIT_FAILURE, utils_rtrim(actual, input, 0));

    assert_int_equal(EXIT_FAILURE, utils_rtrim(NULL, NULL, BUFFER_SIZE));
    assert_int_equal(EXIT_FAILURE, utils_rtrim(NULL, input, BUFFER_SIZE));
    assert_int_equal(EXIT_FAILURE, utils_rtrim(actual, NULL, BUFFER_SIZE));

    strcpy(input, "");
    strcpy(expected, "");
    assert_int_equal(EXIT_SUCCESS, utils_rtrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, " ");
    strcpy(expected, "");
    assert_int_equal(EXIT_SUCCESS, utils_rtrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, "a");
    strcpy(expected, "a");
    assert_int_equal(EXIT_SUCCESS, utils_rtrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, "a ");
    strcpy(expected, "a");
    assert_int_equal(EXIT_SUCCESS, utils_rtrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, " a");
    strcpy(expected, " a");
    assert_int_equal(EXIT_SUCCESS, utils_rtrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, " a ");
    strcpy(expected, " a");
    assert_int_equal(EXIT_SUCCESS, utils_rtrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, "ab");
    strcpy(expected, "ab");
    assert_int_equal(EXIT_SUCCESS, utils_rtrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, "ab ");
    strcpy(expected, "ab");
    assert_int_equal(EXIT_SUCCESS, utils_rtrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, " ab");
    strcpy(expected, " ab");
    assert_int_equal(EXIT_SUCCESS, utils_rtrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, " ab ");
    strcpy(expected, " ab");
    assert_int_equal(EXIT_SUCCESS, utils_rtrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, "a b");
    strcpy(expected, "a b");
    assert_int_equal(EXIT_SUCCESS, utils_rtrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, "a b ");
    strcpy(expected, "a b");
    assert_int_equal(EXIT_SUCCESS, utils_rtrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, " a b");
    strcpy(expected, " a b");
    assert_int_equal(EXIT_SUCCESS, utils_rtrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, " a b ");
    strcpy(expected, " a b");
    assert_int_equal(EXIT_SUCCESS, utils_rtrim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);
}

void test_utils_trim(void **state) {
    (void) state;

    char input[BUFFER_SIZE];
    char expected[BUFFER_SIZE];
    char actual[BUFFER_SIZE];

    bzero(input, BUFFER_SIZE);
    bzero(expected, BUFFER_SIZE);
    bzero(actual, BUFFER_SIZE);

    assert_int_equal(EXIT_FAILURE, utils_trim(NULL, NULL, 0));
    assert_int_equal(EXIT_FAILURE, utils_trim(NULL, input, 0));
    assert_int_equal(EXIT_FAILURE, utils_trim(actual, NULL, 0));
    assert_int_equal(EXIT_FAILURE, utils_trim(actual, input, 0));

    assert_int_equal(EXIT_FAILURE, utils_trim(NULL, NULL, BUFFER_SIZE));
    assert_int_equal(EXIT_FAILURE, utils_trim(NULL, input, BUFFER_SIZE));
    assert_int_equal(EXIT_FAILURE, utils_trim(actual, NULL, BUFFER_SIZE));

    strcpy(input, "");
    strcpy(expected, "");
    assert_int_equal(EXIT_SUCCESS, utils_trim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, " ");
    strcpy(expected, "");
    assert_int_equal(EXIT_SUCCESS, utils_trim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, "a");
    strcpy(expected, "a");
    assert_int_equal(EXIT_SUCCESS, utils_trim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, "a ");
    strcpy(expected, "a");
    assert_int_equal(EXIT_SUCCESS, utils_trim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, " a");
    strcpy(expected, "a");
    assert_int_equal(EXIT_SUCCESS, utils_trim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, " a ");
    strcpy(expected, "a");
    assert_int_equal(EXIT_SUCCESS, utils_trim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, "ab");
    strcpy(expected, "ab");
    assert_int_equal(EXIT_SUCCESS, utils_trim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, "ab ");
    strcpy(expected, "ab");
    assert_int_equal(EXIT_SUCCESS, utils_trim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, " ab");
    strcpy(expected, "ab");
    assert_int_equal(EXIT_SUCCESS, utils_trim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, " ab ");
    strcpy(expected, "ab");
    assert_int_equal(EXIT_SUCCESS, utils_trim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, "a b");
    strcpy(expected, "a b");
    assert_int_equal(EXIT_SUCCESS, utils_trim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, "a b ");
    strcpy(expected, "a b");
    assert_int_equal(EXIT_SUCCESS, utils_trim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, " a b");
    strcpy(expected, "a b");
    assert_int_equal(EXIT_SUCCESS, utils_trim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);

    strcpy(input, " a b ");
    strcpy(expected, "a b");
    assert_int_equal(EXIT_SUCCESS, utils_trim(actual, input, BUFFER_SIZE));
    assert_string_equal(expected, actual);
}
