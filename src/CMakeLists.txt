if (RTLSDR_RADIO_FP_FLOAT)
    pkg_check_modules(fftw3 REQUIRED IMPORTED_TARGET fftw3f)
elseif (RTLSDR_RADIO_FP_DOUBLE)
    pkg_check_modules(fftw3 REQUIRED IMPORTED_TARGET fftw3)
elseif (RTLSDR_RADIO_FP_LONG_DOUBLE)
    pkg_check_modules(fftw3 REQUIRED IMPORTED_TARGET fftw3l)
endif ()

configure_file(version.h.in version.h @ONLY)
configure_file(buildflags.h.in buildflags.h @ONLY)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(Threads REQUIRED)

add_executable(rtlsdr_radio
        agc.c agc.h
        audio.c audio.h
        codec.c codec.h
        cfg.c cfg.h
        circbuf.c circbuf.h
        default.h
        device.c device.h
        dsp.c dsp.h
        fft.c fft.h
        fir.c fir.h fir_lpf.h
        frame.c frame.h
        greatbuf.c greatbuf.h
        http.c http.h
        log.c log.h
        main.c main.h
        main_info.c main_info.h
        main_rx.c main_rx.h
        network.h network.c
        payload.c payload.h
        resample.c resample.h
        ui.c ui.h
        utils.c utils.h
        wav.c wav.h)

target_link_libraries(rtlsdr_radio
        Threads::Threads
        m
        PkgConfig::rtlsdr
        PkgConfig::fftw3
        PkgConfig::asound2
        PkgConfig::codec2
        PkgConfig::uuid
        PkgConfig::curl)

target_compile_options(rtlsdr_radio
        PRIVATE
        -Wall -Wextra -Wpedantic)

install(TARGETS rtlsdr_radio
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT rtlsdr_radio)
