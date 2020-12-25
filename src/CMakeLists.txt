configure_file(version.h.in version.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Threads REQUIRED)

pkg_check_modules(fftw3 REQUIRED IMPORTED_TARGET fftw3)
pkg_check_modules(opus REQUIRED IMPORTED_TARGET opus)
pkg_check_modules(rtlsdr REQUIRED IMPORTED_TARGET librtlsdr)

add_executable(rtlsdr_radio
        agc.c agc.h
        audiocodec.c audiocodec.h
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
        -Wall)
