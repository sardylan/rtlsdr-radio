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


#ifndef __RTLSDR_RADIO__MAIN_RX__H
#define __RTLSDR_RADIO__MAIN_RX__H

#define MAIN_RX_BUFFERS_SIZE 2048

#define MAIN_RX_ENABLE_THREAD_READ
#define MAIN_RX_ENABLE_THREAD_SAMPLES
#define MAIN_RX_ENABLE_THREAD_DEMOD
#define MAIN_RX_ENABLE_THREAD_FILTER
#define MAIN_RX_ENABLE_THREAD_RESAMPLE
#define MAIN_RX_ENABLE_THREAD_CODEC
//#define MAIN_RX_ENABLE_THREAD_AUDIO
//#define MAIN_RX_ENABLE_THREAD_NETWORK

int main_rx();

void main_rx_end();

void main_rx_wait_init();

#ifdef MAIN_RX_ENABLE_THREAD_READ
void *thread_rx_read();
#endif

#ifdef MAIN_RX_ENABLE_THREAD_SAMPLES
void *thread_rx_samples();
#endif

#ifdef MAIN_RX_ENABLE_THREAD_DEMOD
void *thread_rx_demod();
#endif

#ifdef MAIN_RX_ENABLE_THREAD_FILTER
void *thread_rx_filter();
#endif

#ifdef MAIN_RX_ENABLE_THREAD_RESAMPLE
void *thread_rx_resample();
#endif

#ifdef MAIN_RX_ENABLE_THREAD_CODEC
void *thread_rx_codec();
#endif

#ifdef MAIN_RX_ENABLE_THREAD_AUDIO
void *thread_rx_audio();
#endif

#ifdef MAIN_RX_ENABLE_THREAD_NETWORK
void *thread_rx_network();
#endif

#endif
