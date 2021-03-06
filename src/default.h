/*
 * rtlsdr-radio
 * Copyright (C) 2020 - 2021  Luca Cireddu (sardylan@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */


#ifndef __RTLSDR_RADIO__DEFAULT__H
#define __RTLSDR_RADIO__DEFAULT__H

#include <codec2/codec2.h>

#include "log.h"
#include "cfg.h"

#define CONFIG_UI_LOG_LEVEL_DEFAULT LOG_LEVEL_TRACE
#define CONFIG_FILE_LOG_LEVEL_DEFAULT LOG_LEVEL_INFO
#define CONFIG_FILE_LOG_NAME_DEFAULT "rtlsdr_radio.log"

#define CONFIG_SOURCE_DEFAULT SOURCE_RTLSDR
#define CONFIG_MODE_DEFAULT MODE_RX
#define CONFIG_DEBUG_DEFAULT FLAG_FALSE

#define CONFIG_FILE_RAWIQ_FILE_PATH_DEFAULT "/home/sardylan/desktop/incomings/test.rawiq"

#define CONFIG_RTLSDR_DEVICE_ID_DEFAULT 0
#define CONFIG_RTLSDR_DEVICE_SAMPLE_RATE_DEFAULT 256000
#define CONFIG_RTLSDR_DEVICE_CENTER_FREQ_DEFAULT 339450000
#define CONFIG_RTLSDR_DEVICE_FREQ_CORRECTION_DEFAULT 71
#define CONFIG_RTLSDR_DEVICE_TUNER_GAIN_MODE_DEFAULT FLAG_TRUE
#define CONFIG_RTLSDR_DEVICE_TUNER_GAIN_DEFAULT 496
#define CONFIG_RTLSDR_DEVICE_AGC_MODEDEFAULT FLAG_FALSE
#define CONFIG_RTLSDR_SAMPLES_DEFAULT 2048

#define CONFIG_MODULATION_DEFAULT MOD_TYPE_AM

#define CONFIG_FILTER_DEFAULT FILTER_MODE_FFT_SW
#define CONFIG_FILTER_FIR_DEFAULT 1

#define CONFIG_AUDIO_FRAME_PER_PERIOD_DEFAULT 4096
#define CONFIG_AUDIO_SAMPLE_RATE_DEFAULT 8000

#define CONFIG_AUDIO_FILE_ENABLED_DEFAULT FLAG_FALSE
#define CONFIG_AUDIO_FILE_PATH_DEFAULT "audio.wav"

#define CONFIG_AUDIO_MONITOR_ENABLED_DEFAULT FLAG_TRUE
#define CONFIG_AUDIO_MONITOR_DEVICE_DEFAULT "default"

#define CONFIG_AUDIO_STDOUT_DEFAULT FLAG_FALSE

#define CONFIG_CODEC2_MODE_DEFAULT CODEC2_MODE_3200

#define CONFIG_NETWORK_SERVER_DEFAULT "127.0.0.1"
#define CONFIG_NETWORK_PORT_DEFAULT 64123

#endif
