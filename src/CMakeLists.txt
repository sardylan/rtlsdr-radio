find_package(PkgConfig REQUIRED)

configure_file(version.h.in version.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Threads REQUIRED)
find_package(rtlsdr REQUIRED)
pkg_check_modules(FFTW3 REQUIRED IMPORTED_TARGET fftw3)
pkg_check_modules(libavutil REQUIRED IMPORTED_TARGET libavutil)
pkg_check_modules(libswresample REQUIRED IMPORTED_TARGET libswresample)

add_executable(rtlsdr_radio
        main.c main.h
        ui.c ui.h
        log.c log.h
        cfg.c cfg.h
        device.c device.h
        circbuf.c circbuf.h
        resample.c resample.h
        default.h)

target_link_libraries(rtlsdr_radio
        Threads::Threads
        m
        rtlsdr::rtlsdr
        PkgConfig::FFTW3
        PkgConfig::libavutil
        PkgConfig::libswresample)

target_compile_options(rtlsdr_radio
        PRIVATE
        -Wall)
