pkg_check_modules(cmocka REQUIRED IMPORTED_TARGET cmocka)

add_executable(test_utils
        utils.c utils.h
        ../src/utils.c ../src/utils.h)

target_link_libraries(test_utils
        PkgConfig::cmocka)

target_compile_options(test_utils
        PRIVATE
        -Wall -Wextra -Wpedantic)
