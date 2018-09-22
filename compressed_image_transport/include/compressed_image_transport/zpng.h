#ifndef ZPNG_H
#define ZPNG_H
#endif

#include <stddef.h>   /* size_t */

size_t compressGreyscalePng(size_t width, size_t height, uint16_t bits, 
        const uint8_t *rawBuff, size_t rawSize, uint8_t *pngBuff, size_t pngCapacity,
        int level);

size_t readGreyscalePng(size_t *width, size_t *height, uint16_t *bits,
        const uint8_t *pngBuff, size_t *idatSize, size_t *rawSize,
        const uint8_t **idatBuff);

size_t decompressGreyscalePng(const uint8_t *idatBuff, size_t idatSize,
        uint8_t *rawBuff, size_t rawBuffCapacity);
