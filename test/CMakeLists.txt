pkg_check_modules(cmocka REQUIRED IMPORTED_TARGET cmocka)

configure_file(../src/version.h.in version.h @ONLY)
configure_file(../src/buildflags.h.in buildflags.h @ONLY)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_compile_definitions(__RTLSDR__TESTS)

add_executable(test_utils utils.c utils.h ../src/utils.c ../src/utils.h)
target_link_libraries(test_utils PkgConfig::cmocka)
target_compile_options(test_utils PRIVATE -Wall -Wextra -Wpedantic)
add_test(TestUtils test_utils)
set_tests_properties(TestUtils PROPERTIES ENVIRONMENT CMOCKA_MESSAGE_OUTPUT=xml)

add_executable(test_greatbuf greatbuf.c greatbuf.h ../src/greatbuf.c ../src/greatbuf.h)
target_link_libraries(test_greatbuf PkgConfig::cmocka)
target_compile_options(test_greatbuf PRIVATE -Wall -Wextra -Wpedantic)
add_test(TestGreatbuf test_greatbuf)
set_tests_properties(TestGreatbuf PROPERTIES ENVIRONMENT CMOCKA_MESSAGE_OUTPUT=xml)

add_executable(test_http http.c http.h ../src/http.c ../src/http.h)
target_link_libraries(test_http PkgConfig::cmocka PkgConfig::curl)
target_compile_options(test_http PRIVATE -Wall -Wextra -Wpedantic)
add_test(TestHTTP test_http)
set_tests_properties(TestHTTP PROPERTIES ENVIRONMENT CMOCKA_MESSAGE_OUTPUT=xml)
