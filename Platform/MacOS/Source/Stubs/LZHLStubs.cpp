#include "CompLibHeader/lzhl.h"
#include <stdlib.h>

LZHL_DHANDLE LZHLCreateDecompressor(void) { return (LZHL_DHANDLE)1; }
int LZHLDecompress(LZHL_DHANDLE handle, void* dst, size_t* dstSz, const void* src, size_t* srcSz) {
    // Return 0 to break the loop in NoxCompress.cpp
    return 0;
}
void LZHLDestroyDecompressor(LZHL_DHANDLE handle) {}

LZHL_CHANDLE LZHLCreateCompressor(void) { return (LZHL_CHANDLE)1; }
unsigned int LZHLCompress(LZHL_CHANDLE handle, void* dst, const void* src, unsigned int srcLen) {
    return 0;
}
void LZHLDestroyCompressor(LZHL_CHANDLE handle) {}

unsigned int LZHLCompressorCalcMaxBuf(unsigned int rawSize) {
    return rawSize + 1024;
}
