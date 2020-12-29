include(CheckIncludeFile)

check_include_file(/opt/vc/include/bcm_host.h RPI_BCMHOST_FOUND)

if (RPI_BCMHOST_FOUND)
    set(RPI_PLATFORM_ENABLED TRUE)
    message(STATUS "Detected RaspberryPi platform")
else ()
    set(RPI_PLATFORM_ENABLED FALSE)
endif ()

unset(RPI_BCMHOST_FOUND CACHE)
