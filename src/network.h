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


#ifndef __RTLSDR_RADIO__NETWORK__H
#define __RTLSDR_RADIO__NETWORK__H

#include <stddef.h>
#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include <netdb.h>
#include <sys/socket.h>


struct network_ctx_t {
    char *address;
    uint16_t port;

    int sck;

    struct sockaddr sockaddr;
    socklen_t sockaddr_len;
};

typedef struct network_ctx_t network_ctx;

network_ctx *network_init(const char *, uint16_t);

void network_free(network_ctx *);

int network_socket_open(network_ctx *ctx);

void network_socket_close(network_ctx *ctx);

int network_socket_send(network_ctx *ctx, uint8_t *, size_t);

#endif
