configure_file(version.h.in version.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Threads REQUIRED)
find_package(rtlsdr REQUIRED)
pkg_check_modules(FFTW3 REQUIRED IMPORTED_TARGET fftw3)

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
        PkgConfig::FFTW3)

target_compile_options(rtlsdr_radio
        PRIVATE
        -Wall)
