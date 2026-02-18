#ifndef LZHL_H
#define LZHL_H

#include <stddef.h>

typedef void* LZHL_DHANDLE;
typedef void* LZHL_CHANDLE;

#ifdef __cplusplus
extern "C" {
#endif

LZHL_DHANDLE LZHLCreateDecompressor(void);
int LZHLDecompress(LZHL_DHANDLE handle, void* dst, size_t* dstSz, const void* src, size_t* srcSz);
void LZHLDestroyDecompressor(LZHL_DHANDLE handle);

LZHL_CHANDLE LZHLCreateCompressor(void);
unsigned int LZHLCompress(LZHL_CHANDLE handle, void* dst, const void* src, unsigned int srcLen);
void LZHLDestroyCompressor(LZHL_CHANDLE handle);

unsigned int LZHLCompressorCalcMaxBuf(unsigned int rawSize);

#ifdef __cplusplus
}
#endif

#endif // LZHL_H
