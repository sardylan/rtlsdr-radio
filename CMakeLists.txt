cmake_minimum_required(VERSION 3.5)

project(rtlsdr_radio C)

set(CMAKE_C_STANDARD 11)

set(APPLICATION_NAME "RTL-SDR Radio")

set(APPLICATION_VERSION_MAJOR "0")
set(APPLICATION_VERSION_MINOR "5")
set(APPLICATION_VERSION_PATCH "0")
set(APPLICATION_VERSION "${APPLICATION_VERSION_MAJOR}.${APPLICATION_VERSION_MINOR}.${APPLICATION_VERSION_PATCH}")

add_compile_options(-Wall -Wextra -pedantic -Werror)
add_compile_options("-D__FILENAME__=__FILE__")

include(cmake/rpi.cmake)

find_package(PkgConfig REQUIRED)

set(RTLSDR_RADIO_LOG_ERROR_ENABLED TRUE CACHE BOOL "Enable ERROR level logging")
set(RTLSDR_RADIO_LOG_WARN_ENABLED TRUE CACHE BOOL "Enable WARN level logging")
set(RTLSDR_RADIO_LOG_INFO_ENABLED TRUE CACHE BOOL "Enable INFO level logging")
set(RTLSDR_RADIO_LOG_DEBUG_ENABLED TRUE CACHE BOOL "Enable DEBUG level logging")
set(RTLSDR_RADIO_LOG_TRACE_ENABLED TRUE CACHE BOOL "Enable TRACE level logging")

set(ENABLED_LOGS)

if (RTLSDR_RADIO_LOG_ERROR_ENABLED)
    list(APPEND ENABLED_LOGS "ERROR")
endif ()
if (RTLSDR_RADIO_LOG_WARN_ENABLED)
    list(APPEND ENABLED_LOGS "WARN")
endif ()
if (RTLSDR_RADIO_LOG_INFO_ENABLED)
    list(APPEND ENABLED_LOGS "INFO")
endif ()
if (RTLSDR_RADIO_LOG_DEBUG_ENABLED)
    list(APPEND ENABLED_LOGS "DEBUG")
endif ()
if (RTLSDR_RADIO_LOG_TRACE_ENABLED)
    list(APPEND ENABLED_LOGS "TRACE")
endif ()

message(STATUS "Log enabled: ${ENABLED_LOGS}")

if (NOT RTLSDR_RADIO_FP_PRECISION)
    if (RPI_PLATFORM_ENABLED)
        set(RTLSDR_RADIO_FP_PRECISION_DEFAULT float)
    else ()
        set(RTLSDR_RADIO_FP_PRECISION_DEFAULT double)
    endif ()
endif ()

set(RTLSDR_RADIO_FP_PRECISION "${RTLSDR_RADIO_FP_PRECISION_DEFAULT}" CACHE STRING "Set Floating Point precision changing used data type")
set_property(CACHE RTLSDR_RADIO_FP_PRECISION PROPERTY STRINGS double float)
unset(RTLSDR_RADIO_FP_PRECISION_DEFAULT CACHE)
message(STATUS "Floating Point precision: ${RTLSDR_RADIO_FP_PRECISION}")

if (RTLSDR_RADIO_FP_PRECISION STREQUAL "float")
    set(RTLSDR_RADIO_FP_FLOAT TRUE)
    set(RTLSDR_RADIO_FP_DOUBLE FALSE)
else ()
    set(RTLSDR_RADIO_FP_FLOAT FALSE)
    set(RTLSDR_RADIO_FP_DOUBLE TRUE)
endif ()

add_compile_definitions(FP_FLOAT="${RTLSDR_RADIO_FP_PRECISION}")
add_compile_definitions(PROJECT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

enable_testing()

pkg_check_modules(rtlsdr REQUIRED IMPORTED_TARGET librtlsdr)
pkg_check_modules(asound2 REQUIRED IMPORTED_TARGET alsa)
pkg_check_modules(codec2 REQUIRED IMPORTED_TARGET codec2)
pkg_check_modules(curl REQUIRED IMPORTED_TARGET libcurl)
pkg_check_modules(uuid REQUIRED IMPORTED_TARGET uuid)

add_subdirectory(src)
add_subdirectory(test)
