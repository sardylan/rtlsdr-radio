/*
 * rtlsdr-radio
 * Copyright (C) 2020  Luca Cireddu (sardylan@gmail.com)
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


#ifndef __RTLSDR_RADIO__RTLSDR_RADIO__H
#define __RTLSDR_RADIO__RTLSDR_RADIO__H

#define BUFFER_FRAMES 8192

#define BUFFER_SAMPLES 8192
#define BUFFER_DEMOD 8192
#define BUFFER_FILTERED 8192
#define BUFFER_CODEC 8192

void signal_handler(int signum);

int main_program();

int main_program_mode_rx();

void main_program_mode_rx_end();

int main_program_mode_info();

void main_stop();

void main_program_mode_rx_wait_init();

void *thread_rx_device_read();

void *thread_rx_demod();

void *thread_rx_lpf();

void *thread_rx_resample();

void *thread_rx_codec();

void *thread_rx_network();

#endif
