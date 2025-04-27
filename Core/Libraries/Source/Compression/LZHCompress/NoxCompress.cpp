/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// compress.c
// Compress interface for packets and files
// Author: Jeff Brown, January 1999

#include <stdio.h>
#include <stdlib.h>
#include "Lib/BaseTypeCore.h"
#include "NoxCompress.h"
#include "CompLibHeader/lzhl.h"

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma message("************************************** WARNING, optimization disabled for debugging purposes")
#endif

#define BLOCKSIZE 500000
#define NoxRead fread
#define DbgMalloc malloc
#define DbgFree free
#define DEBUG_LOG(x) {}

Bool DecompressFile		(char *infile, char *outfile)
{
	UnsignedInt	rawSize = 0, compressedSize = 0;
	FILE *inFilePtr = NULL;
	FILE *outFilePtr= NULL;
	char *inBlock		= NULL;
	char *outBlock	= NULL;
	LZHL_DHANDLE decompress;
	Int ok = 0;
	size_t srcSz, dstSz;

	// Parameter checking
	 
	if (( infile == NULL ) || ( outfile == NULL ))
		return FALSE;

	inFilePtr = fopen( infile, "rb" );
	if ( inFilePtr )
	{
		// Allocate the appropriate amount of memory
		// Get compressed size of file.
		fseek( inFilePtr, 0, SEEK_END );
		compressedSize = ftell( inFilePtr );
		fseek( inFilePtr, 0, SEEK_SET );

		compressedSize -= sizeof(UnsignedInt);

		// Get uncompressed size. Don't worry about endian, 
		// this is always INTEL baby!
		NoxRead(&rawSize, 1, sizeof(UnsignedInt), inFilePtr);

		// This is ick, but allocate a BIIIIG chunk o' memory x 2
		inBlock = (char *) DbgMalloc( compressedSize );
		outBlock= (char *) DbgMalloc( rawSize );

		if (( inBlock == NULL ) || ( outBlock == NULL ))
			return FALSE;

		// Read in a big chunk o file
		NoxRead(inBlock, 1, compressedSize, inFilePtr);

		fclose(inFilePtr);

		// Decompress
		srcSz = compressedSize;
		dstSz = rawSize;
		
		// Just Do it!
		decompress = LZHLCreateDecompressor();

		for (;;)
		{
			ok = LZHLDecompress( decompress, outBlock + rawSize - dstSz, &dstSz, 
																			 inBlock + compressedSize - srcSz, &srcSz);

			if ( !ok )
				break;

			if (srcSz <= 0)
				break;
		}

		DEBUG_LOG(("Decompressed %s to %s, output size = %d\n", infile, outfile, rawSize));

		LZHLDestroyDecompressor(decompress);
		outFilePtr = fopen(outfile, "wb");
		if (outFilePtr)
		{
			fwrite (outBlock, rawSize, 1, outFilePtr);
			fclose(outFilePtr);
		}
		else
			return FALSE;

		// Clean up this mess
		DbgFree(inBlock);
		DbgFree(outBlock);
		return TRUE;

	} // End of if fileptr

	return FALSE;
}


Bool CompressFile			(char *infile, char *outfile)
{
	UnsignedInt	rawSize = 0;
	UnsignedInt compressedSize = 0, compressed = 0, i = 0;
	FILE *inFilePtr = NULL;
	FILE *outFilePtr= NULL;
	char *inBlock		= NULL;
	char *outBlock	= NULL;
	LZHL_CHANDLE compressor;
	UnsignedInt blocklen;

	// Parameter checking
	 
	if (( infile == NULL ) || ( outfile == NULL ))
		return FALSE;

	// Allocate the appropriate amount of memory
	inFilePtr = fopen( infile, "rb" );
	if ( inFilePtr )
	{
		// Get size of file.
		fseek( inFilePtr, 0, SEEK_END );
		rawSize = ftell( inFilePtr );
		fseek( inFilePtr, 0, SEEK_SET );

		// This is ick, but allocate a BIIIIG chunk o' memory x 2
		inBlock = (char *) DbgMalloc(rawSize);
		outBlock= (char *) DbgMalloc( LZHLCompressorCalcMaxBuf( rawSize ));

		if (( inBlock == NULL ) || ( outBlock == NULL ))
			return FALSE;

		// Read in a big chunk o file
		NoxRead(inBlock, 1, rawSize, inFilePtr);

		fclose(inFilePtr);

		// Compress
		compressor = LZHLCreateCompressor();
		for ( i = 0; i < rawSize; i += BLOCKSIZE )
		{
			blocklen = min((UnsignedInt)BLOCKSIZE, rawSize - i);
			compressed = LZHLCompress(compressor, outBlock + compressedSize, inBlock + i, blocklen);
			compressedSize += compressed;
		}

		LZHLDestroyCompressor(compressor);

		outFilePtr = fopen(outfile, "wb");
		if (outFilePtr)
		{
			// write out the uncompressed size first.
			fwrite(&rawSize, sizeof(UnsignedInt), 1, outFilePtr);
			fwrite(outBlock, compressedSize, 1, outFilePtr);
			fclose(outFilePtr);
		}
		else
			return FALSE;

		// Clean up
		DbgFree(inBlock);
		DbgFree(outBlock);
		return TRUE;
	}

	return FALSE;
}

Bool CompressPacket		(char *inPacket, char *outPacket)
{
	// Parameter checking
	 
	if (( inPacket == NULL ) || ( outPacket == NULL ))
		return FALSE;

	return TRUE;
}


Bool DecompressPacket	(char *inPacket, char *outPacket)
{
	// Parameter checking
	 
	if (( inPacket == NULL ) || ( outPacket == NULL ))
		return FALSE;
	return TRUE;
}


UnsignedInt CalcNewSize		(UnsignedInt rawSize)
{
	return LZHLCompressorCalcMaxBuf(rawSize);
}

Bool DecompressMemory		(void *inBufferVoid, Int inSize, void *outBufferVoid, Int& outSize)
{
	UnsignedByte *inBuffer = (UnsignedByte *)inBufferVoid;
	UnsignedByte *outBuffer = (UnsignedByte *)outBufferVoid;
	UnsignedInt	rawSize = 0, compressedSize = 0;
	LZHL_DHANDLE decompress;
	Int ok = 0;
	size_t srcSz, dstSz;

	// Parameter checking
	 
	if (( inBuffer == NULL ) || ( outBuffer == NULL ) || ( inSize < 4 ) || ( outSize == 0 ))
		return FALSE;

	// Get compressed size of file.
	compressedSize = inSize;

	// Get uncompressed size.
	rawSize = outSize;

	// Decompress
	srcSz = compressedSize;
	dstSz = rawSize;
	
	// Just Do it!
	decompress = LZHLCreateDecompressor();

	for (;;)
	{
		ok = LZHLDecompress( decompress, outBuffer + rawSize - dstSz, &dstSz, 
																		 inBuffer + compressedSize - srcSz, &srcSz);

		if ( !ok )
			break;

		if (srcSz <= 0)
			break;
	}

	LZHLDestroyDecompressor(decompress);

	outSize = rawSize;

	return TRUE;

}

Bool CompressMemory			(void *inBufferVoid, Int inSize, void *outBufferVoid, Int& outSize)
{
	UnsignedByte *inBuffer = (UnsignedByte *)inBufferVoid;
	UnsignedByte *outBuffer = (UnsignedByte *)outBufferVoid;
	UnsignedInt	rawSize = 0;
	UnsignedInt compressedSize = 0, compressed = 0, i = 0;
	LZHL_CHANDLE compressor;
	UnsignedInt blocklen;

	// Parameter checking
	 
	if (( inBuffer == NULL ) || ( outBuffer == NULL ) || ( inSize < 4 ) || ( outSize == 0 ))
		return FALSE;

	rawSize = inSize;

	// Compress
	compressor = LZHLCreateCompressor();
	for ( i = 0; i < rawSize; i += BLOCKSIZE )
	{
		blocklen = min((UnsignedInt)BLOCKSIZE, rawSize - i);
		compressed = LZHLCompress(compressor, outBuffer + compressedSize, inBuffer + i, blocklen);
		compressedSize += compressed;
	}

	LZHLDestroyCompressor(compressor);

	outSize = compressedSize;

	return TRUE;
}
