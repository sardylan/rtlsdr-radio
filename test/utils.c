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

#include "utils.h"

const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_stricmp),
};

int main() {
    return cmocka_run_group_tests(tests, NULL, NULL);
}

static void test_stricmp(void **state) {
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