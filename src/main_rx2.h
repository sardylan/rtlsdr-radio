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


#ifndef __RTLSDR_RADIO__MAIN_RX2__H
#define __RTLSDR_RADIO__MAIN_RX2__H

#define MAIN_RX2_BUFFERS_SIZE 2048

#define MAIN_RX2_ENABLE_THREAD_READ
#define MAIN_RX2_ENABLE_THREAD_SAMPLES
#define MAIN_RX2_ENABLE_THREAD_DEMOD
#define MAIN_RX2_ENABLE_THREAD_FILTER
#define MAIN_RX2_ENABLE_THREAD_RESAMPLE
//#define MAIN_RX2_ENABLE_THREAD_CODEC
#define MAIN_RX2_ENABLE_THREAD_MONITOR
//#define MAIN_RX2_ENABLE_THREAD_NETWORK

int main_rx2();

void main_rx2_end();

void main_rx2_wait_init();

#ifdef MAIN_RX2_ENABLE_THREAD_READ
void *thread_rx2_read();
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_SAMPLES
void *thread_rx2_samples();
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_DEMOD
void *thread_rx2_demod();
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_FILTER
void *thread_rx2_filter();
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_RESAMPLE
void *thread_rx2_resample();
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_CODEC
void *thread_rx2_codec();
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_MONITOR
void *thread_rx2_monitor();
#endif

#ifdef MAIN_RX2_ENABLE_THREAD_NETWORK
void *thread_rx2_network();
#endif

#endif
