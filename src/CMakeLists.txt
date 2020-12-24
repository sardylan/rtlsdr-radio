configure_file(version.h.in version.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Threads REQUIRED)

pkg_check_modules(rtlsdr REQUIRED IMPORTED_TARGET librtlsdr)
pkg_check_modules(fftw3 REQUIRED IMPORTED_TARGET fftw3)
pkg_check_modules(opus REQUIRED IMPORTED_TARGET opus)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__FILENAME__='\"$(subst ${CMAKE_SOURCE_DIR}/src,,$(abspath $<))\"'")

add_executable(rtlsdr_radio
        main.c main.h
        ui.c ui.h
        log.c log.h
        cfg.c cfg.h
        device.c device.h
        circbuf.c circbuf.h
        resample.c resample.h
        fft.c fft.h
        dsp.c dsp.h
        fir.c fir.h fir_lpf.h
        agc.c agc.h
        network.h network.c
        frame.c frame.h
        default.h)

target_link_libraries(rtlsdr_radio
        Threads::Threads
        m
        PkgConfig::rtlsdr
        PkgConfig::fftw3
        PkgConfig::opus)

target_compile_options(rtlsdr_radio
        PRIVATE
        -Wall)
