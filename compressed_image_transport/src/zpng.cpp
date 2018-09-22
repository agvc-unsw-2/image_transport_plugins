#include "zstd/zstd.h"

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <arpa/inet.h>

#include <stdio.h>
    
uint8_t pngHeader[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

/* Table of CRCs of all 8-bit messages. */
unsigned long crc_table[256];

/* Flag: has the table been computed? Initially false. */
int crc_table_computed = 0;

/* Make the table for a fast CRC. */
void make_crc_table(void)
{
    unsigned long c;
    int n, k;

    for (n = 0; n < 256; n++) {
        c = (unsigned long) n;
        for (k = 0; k < 8; k++) {
            if (c & 1)
                c = 0xedb88320L ^ (c >> 1);
            else
                c = c >> 1;
        }
        crc_table[n] = c;
    }
    crc_table_computed = 1;
}

/* Update a running CRC with the bytes buf[0..len-1]--the CRC
 *       should be initialized to all 1's, and the transmitted value
 *             is the 1's complement of the final running CRC (see the
 *                   crc() routine below)). */

unsigned long update_crc(unsigned long crc, unsigned char *buf,
        int len)
{
    unsigned long c = crc;
    int n;

    if (!crc_table_computed)
        make_crc_table();
    for (n = 0; n < len; n++) {
        c = crc_table[(c ^ buf[n]) & 0xff] ^ (c >> 8);
    }
    return c;
}

/* Return the CRC of the bytes buf[0..len-1]. */
unsigned long crc(unsigned char *buf, int len)
{
    return update_crc(0xffffffffL, buf, len) ^ 0xffffffffL;
}

size_t addChunk(uint8_t *buf, uint32_t size, const char *type, uint8_t *data)
{
    // Length, 0-3
    *buf++ = (uint8_t)(size >> 24);
    *buf++ = (uint8_t)(size >> 16);
    *buf++ = (uint8_t)(size >> 8);
    *buf++ = (uint8_t)(size);

    // Type, 4-7
    memcpy(buf, type, 4);
    buf += 4;

    // Data, 8-n-4
    if (data != buf) {
        memcpy(buf, data, size);
    }
    buf += size;

    // CRC
    uint32_t sum = crc(buf - size - 4, size + 4);
    *buf++ = (uint8_t)(sum >> 24);
    *buf++ = (uint8_t)(sum >> 16);
    *buf++ = (uint8_t)(sum >> 8);
    *buf++ = (uint8_t)(sum);

    return size + 12;
}

uint32_t readChunk(const uint8_t *buf, char *type, uint8_t *data, uint32_t maxSize)
{
    uint32_t size = 0;
    size |= (uint32_t)(*buf++) << 24;
    size |= (uint32_t)(*buf++) << 16;
    size |= (uint32_t)(*buf++) << 8;
    size |= (uint32_t)(*buf++);

    if (type) {
        memcpy(type, buf, 4);
    }
    buf += 4;

    if (data && maxSize >= size) {
        memcpy(data, buf, size);
    }

    return size + 12;
}

struct __attribute__((packed)) pngIHDR {
    uint32_t width;
    uint32_t height;
    uint8_t depth;
    uint8_t colour;
    uint8_t compression;
    uint8_t filter;
    uint8_t interlace;
};

size_t compressGreyscalePng(size_t width, size_t height, uint16_t bits, 
        const uint8_t *rawBuff, size_t rawSize, uint8_t *pngBuff, size_t pngCapacity,
        int level)
{
    uint8_t *wrBuff = pngBuff;

    memcpy(wrBuff, pngHeader, sizeof(pngHeader));
    wrBuff += sizeof(pngHeader);

    // Setup image header
    struct pngIHDR ihdr;
    ihdr.width = htonl(width);
    ihdr.height = htonl(height);
    ihdr.depth = bits;
    ihdr.colour = 0; // Greyscale
    ihdr.compression = 0; // Deflate compression
    ihdr.filter = 0; // No filter
    ihdr.interlace = 0; // No interlace

    wrBuff += addChunk(wrBuff, sizeof(ihdr), "IHDR", (uint8_t *)&ihdr);

    // Perform compression
    size_t compressedBytes = ZSTD_compress(wrBuff + 8,
            pngCapacity - (wrBuff - pngBuff) - 12 - 12,
            rawBuff, rawSize, level);
    if (ZSTD_isError(compressedBytes)) {
        return 0;
    }
    wrBuff += addChunk(wrBuff, compressedBytes, "IDAT", wrBuff + 8);

    // End marker
    wrBuff += addChunk(wrBuff, 0, "IEND", NULL);

    return wrBuff - pngBuff;
}

size_t readGreyscalePng(size_t *width, size_t *height, uint16_t *bits,
        const uint8_t *pngBuff, size_t *idatSize, const uint8_t **idatBuff)
{
    const uint8_t *rdBuff = pngBuff;

    if (memcmp(rdBuff, pngHeader, sizeof(pngHeader))) {
        // Header bad
        return 0;
    }
    rdBuff += sizeof(pngHeader);

    struct pngIHDR ihdr;
    rdBuff += readChunk(rdBuff, NULL, (uint8_t *)&ihdr, sizeof(ihdr));

    *width = ntohl(ihdr.width);
    *height = ntohl(ihdr.height);
    *bits = ihdr.depth;
    size_t rawSize = *width * *height * ihdr.depth / 8;

    *idatSize = readChunk(rdBuff, NULL, NULL, 0) - 12;
    *idatBuff = rdBuff + 8;

    return rawSize;
}

size_t decompressGreyscalePng(const uint8_t *idatBuff, size_t idatSize,
        uint8_t *rawBuff, size_t rawBuffCapacity)
{
    size_t rawSize = ZSTD_decompress(rawBuff, rawBuffCapacity, idatBuff, idatSize);

    if (ZSTD_isError(rawSize)) {
        return 0;
    } else {
        return rawSize;
    }
}
