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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>

#include "network.h"
#include "log.h"

network_ctx *network_init(const char *address, uint16_t port) {
    network_ctx *ctx;
    size_t ln;

    log_info("Initializing context");

    log_debug("Allocating context");
    ctx = (network_ctx *) malloc(sizeof(network_ctx));
    if (ctx == NULL) {
        log_error("Unable to allocate context");
        return NULL;
    }

    log_debug("Setting address");
    ln = strlen(address);
    ctx->address = (char *) calloc(ln + 1, sizeof(char));
    strcpy(ctx->address, address);

    log_debug("Setting port");
    ctx->port = port;

    log_debug("Initializing socket descriptor");
    ctx->sck = -1;

    return ctx;
}

void network_free(network_ctx *ctx) {
    log_info("Freeing");

    log_debug("Deallocating address");
    if (ctx->address != NULL)
        free(ctx->address);

    log_debug("Deallocating context");
    if (ctx != NULL)
        free(ctx);
}

int network_socket_open(network_ctx *ctx) {
    int result;
    char port[6];
    struct addrinfo hints;
    struct addrinfo *addresses_list;
    struct addrinfo *address;

    log_info("Opening socket");

    log_debug("Preparing socket hints");
    memset(&hints, '\0', sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    log_debug("Resolving address");
    sprintf(port, "%u", ctx->port);
    result = getaddrinfo(ctx->address, port, &hints, &addresses_list);
    if (result != 0) {
        log_error("Error %d in getaddrinfo: %s", result, gai_strerror(result));
        return EXIT_FAILURE;
    }

    for (address = addresses_list; address != NULL; address = address->ai_next) {
        log_debug("Creating socket");
        ctx->sck = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
        if (ctx->sck == -1) {
            log_warn("Socket creation failed");
            continue;
        }

        memcpy(&ctx->sockaddr, address->ai_addr, sizeof(struct sockaddr));
        ctx->sockaddr_len = address->ai_addrlen;

        break;
    }

    log_debug("Freeing Address Infos list");
    freeaddrinfo(addresses_list);

    if (address == NULL) {
        log_error("Socket creation failed");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

void network_socket_close(network_ctx *ctx) {
    if (ctx->sck != -1) {
        log_debug("Closing socket");
        close(ctx->sck);
    }
}

int network_socket_send(network_ctx *ctx, uint8_t *data, size_t data_size) {
    ssize_t sent_bytes;

    sent_bytes = sendto(ctx->sck, (void *) data, data_size, 0, &ctx->sockaddr, ctx->sockaddr_len);

    if (sent_bytes != (ssize_t) data_size) {
        log_error("Unable to sent all data to server");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
