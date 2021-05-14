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

int main_rx2();

void main_rx2_end();

void main_rx2_wait_init();

void *thread_rx2_read();

void *thread_rx2_demod();

void *thread_rx2_lpf();

void *thread_rx2_resample();

void *thread_rx2_codec();

void *thread_rx2_monitor();

void *thread_rx2_network();

#endif
