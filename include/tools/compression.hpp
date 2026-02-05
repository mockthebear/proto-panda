#pragma once
#include "config.hpp"
#include <stdint.h>

class Compression{
    public:
        static int Compress(uint16_t *m_compressionBuffer, uint16_t* originalData, int dataLen);
        static int Decompress(uint16_t *m_decompressionBuffer, int decompressedSize, uint16_t* originalData, int compressedSize);        
};