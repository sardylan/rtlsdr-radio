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


#include <malloc.h>
#include <string.h>
#include <stdlib.h>

#include "wav.h"
#include "log.h"
#include "utils.h"

wav_ctx *wav_init(const char *file_path, uint16_t channels, uint32_t sample_rate, uint16_t bit_per_sample) {
    wav_ctx *ctx;
    size_t ln;

    log_info("Initializing");

    log_debug("Allocating context");
    ctx = (wav_ctx *) malloc(sizeof(wav_ctx));
    if (ctx == NULL) {
        log_error("Unable to allocate context");
        return NULL;
    }

    log_debug("Setting file path");
    ln = strlen(file_path);
    ctx->file_path = (char *) calloc(ln + 1, sizeof(char));
    if (ctx->file_path == NULL) {
        log_error("Unable to allocate context");
        wav_free(ctx);
        return NULL;
    }
    strcpy(ctx->file_path, file_path);

    log_debug("Setting default values");
    ctx->channels = channels;
    ctx->sample_rate = sample_rate;
    ctx->bit_per_sample = bit_per_sample;

    ctx->data_size = 0;

    ctx->fp = NULL;

    return ctx;
}

void wav_free(wav_ctx *ctx) {
    log_info("Freeing");

    if (ctx == NULL)
        return;

    log_debug("Closing file");
    if (ctx->fp != NULL) {
        fclose(ctx->fp);
        ctx->fp = NULL;
    }

    log_debug("Freeing file path");
    if (ctx->file_path != NULL)
        free(ctx->file_path);

    log_debug("Freeing context");
    free(ctx);
}


int wav_write_begin(wav_ctx *ctx) {
    uint8_t header[WAV_HEADER_LENGTH];

    log_info("Begin WAV write");

    log_debug("Opening WAV file");
    ctx->fp = fopen(ctx->file_path, "wb");
    if (ctx->fp == NULL) {
        log_error("Unable to open WAV file for writing");
        return EXIT_FAILURE;
    }

    bzero(header, WAV_HEADER_LENGTH);

    header[0] = 'R';
    header[1] = 'I';
    header[2] = 'F';
    header[3] = 'F';

    header[8] = 'W';
    header[9] = 'A';
    header[10] = 'W';
    header[11] = 'E';

    header[12] = 'f';
    header[13] = 'm';
    header[14] = 't';
    header[15] = ' ';

    utils_uint32_to_be(&header[16], 16);
    utils_uint16_to_be(&header[20], 1);
    utils_uint16_to_be(&header[22], ctx->channels);
    utils_uint32_to_be(&header[24], ctx->sample_rate);
    utils_uint32_to_be(&header[28], ctx->sample_rate * ctx->channels * (ctx->bit_per_sample / 8));
    utils_uint16_to_be(&header[32], ctx->channels * (ctx->bit_per_sample / 8));
    utils_uint16_to_be(&header[34], ctx->bit_per_sample);

    header[36] = 'd';
    header[37] = 'a';
    header[38] = 't';
    header[39] = 'a';

    fwrite(header, sizeof(char), WAV_HEADER_LENGTH, ctx->fp);

    return EXIT_SUCCESS;
}

int wav_write_end(wav_ctx *ctx) {
    uint32_t chunk_size;
    uint8_t buff[4];

    log_info("End WAV write");

    log_debug("Writing chunk size");
    bzero(buff, 4);
    chunk_size = 32 + ctx->data_size;
    utils_uint32_to_be(buff, chunk_size);
    fseek(ctx->fp, 4, SEEK_SET);
    fwrite(buff, sizeof(uint8_t), 4, ctx->fp);

    log_debug("Writing data size");
    bzero(buff, 4);
    utils_uint32_to_be(buff, ctx->data_size);
    fseek(ctx->fp, 40, SEEK_SET);
    fwrite(buff, sizeof(uint8_t), 4, ctx->fp);

    log_debug("Closing file");
    fclose(ctx->fp);

    return EXIT_SUCCESS;
}

int wav_write_data_uint8(wav_ctx *ctx, uint8_t *data, size_t size) {
    size_t written;

    log_debug("Write data");
    written = fwrite(data, sizeof(uint8_t), size, ctx->fp);

    log_debug("Update data size");
    ctx->data_size += written;

    return EXIT_SUCCESS;
}

int wav_write_data_int16(wav_ctx *ctx, int16_t *data, size_t size) {
    size_t written;

    log_debug("Write data");
    written = fwrite(data, sizeof(int16_t), size, ctx->fp);

    log_debug("Update data size");
    ctx->data_size += written * 2;

    return EXIT_SUCCESS;
}
