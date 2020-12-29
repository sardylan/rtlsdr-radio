if (RTLSDR_RADIO_FP_PRECISION STREQUAL "float")
    set(RTLSDR_RADIO_FP_FLOAT TRUE)
    set(RTLSDR_RADIO_FP_DOUBLE FALSE)
    pkg_check_modules(fftw3 REQUIRED IMPORTED_TARGET fftw3f)
else ()
    set(RTLSDR_RADIO_FP_FLOAT FALSE)
    set(RTLSDR_RADIO_FP_DOUBLE TRUE)
    pkg_check_modules(fftw3 REQUIRED IMPORTED_TARGET fftw3)
endif ()

add_compile_definitions(FP_FLOAT=${RTLSDR_RADIO_FP_PRECISION})

configure_file(version.h.in version.h @ONLY)
configure_file(buildflags.h.in buildflags.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Threads REQUIRED)

pkg_check_modules(opus REQUIRED IMPORTED_TARGET opus)
pkg_check_modules(rtlsdr REQUIRED IMPORTED_TARGET librtlsdr)

add_executable(rtlsdr_radio
        agc.c agc.h
        codec.c codec.h
        cfg.c cfg.h
        circbuf.c circbuf.h
        default.h
        device.c device.h
        dsp.c dsp.h
        fft.c fft.h
        fir.c fir.h fir_lpf.h
        frame.c frame.h
        log.c log.h
        main.c main.h
        main_info.c main_info.h
        main_rx.c main_rx.h
        network.h network.c
        payload.c payload.h
        resample.c resample.h
        ui.c ui.h
        utils.c utils.h)

target_link_libraries(rtlsdr_radio
        Threads::Threads
        m
        PkgConfig::fftw3
        PkgConfig::opus
        PkgConfig::rtlsdr)

target_compile_options(rtlsdr_radio
        PRIVATE
        -Wall -Wextra -Wpedantic)
