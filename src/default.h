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

#include "log.h"
#include "cfg.h"

#define CONFIG_UI_LOG_LEVEL_DEFAULT LOG_LEVEL_TRACE
#define CONFIG_FILE_LOG_LEVEL_DEFAULT LOG_LEVEL_INFO
#define CONFIG_FILE_LOG_NAME_DEFAULT "rtlsdr_radio.log"

#define CONFIG_MODE_DEFAULT MODE_RX2
#define CONFIG_DEBUG_DEFAULT 0

#define CONFIG_RTLSDR_DEVICE_ID_DEFAULT 0
#define CONFIG_RTLSDR_DEVICE_SAMPLE_RATE_DEFAULT 256000
#define CONFIG_RTLSDR_DEVICE_CENTER_FREQ_DEFAULT 93600000
#define CONFIG_RTLSDR_DEVICE_FREQ_CORRECTION_DEFAULT 0
#define CONFIG_RTLSDR_DEVICE_TUNER_GAIN_MODE_DEFAULT 1
#define CONFIG_RTLSDR_DEVICE_TUNER_GAIN_DEFAULT 197
#define CONFIG_RTLSDR_DEVICE_AGC_MODEDEFAULT 0
#define CONFIG_RTLSDR_SAMPLES_DEFAULT 1024

#define CONFIG_MODULATION_DEFAULT MOD_TYPE_FM

#define CONFIG_FILTER_DEFAULT FILTER_MODE_FFT_SW
#define CONFIG_FILTER_FIR_DEFAULT 1

#define CONFIG_AUDIO_SAMPLE_RATE_DEFAULT 8000

#define CONFIG_AUDIO_MONITOR_ENABLED_DEFAULT 0
#define CONFIG_AUDIO_MONITOR_DEVICE_DEFAULT "default"

#define CONFIG_CODEC_OPUS_BITRATE_DEFAULT 16384

#define CONFIG_NETWORK_SERVER_DEFAULT "127.0.0.1"
#define CONFIG_NETWORK_PORT_DEFAULT 64123

#endif
