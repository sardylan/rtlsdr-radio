pkg_check_modules(cmocka REQUIRED IMPORTED_TARGET cmocka)

add_compile_definitions(__RTLSDR__TESTS)

add_executable(test_utils utils.c utils.h ../src/utils.c ../src/utils.h)
target_link_libraries(test_utils PkgConfig::cmocka)
target_compile_options(test_utils PRIVATE -Wall -Wextra -Wpedantic)
add_test(TestUtils test_utils)
set_tests_properties(TestUtils PROPERTIES ENVIRONMENT CMOCKA_MESSAGE_OUTPUT=xml)

add_executable(test_greatbuf greatbuf.c greatbuf.h ../src/greatbuf.c ../src/greatbuf.h)
target_link_libraries(test_greatbuf PkgConfig::cmocka)
target_compile_options(test_greatbuf PRIVATE -Wall -Wextra -Wpedantic)
add_test(GreatbufUtils test_greatbuf)
set_tests_properties(GreatbufUtils PROPERTIES ENVIRONMENT CMOCKA_MESSAGE_OUTPUT=xml)
