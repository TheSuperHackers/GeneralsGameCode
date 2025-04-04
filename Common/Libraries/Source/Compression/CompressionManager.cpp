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

// FILE: Compression.cpp /////////////////////////////////////////////////////
// Author: Matthew D. Campbell
// LZH wrapper taken from Nox, originally from Jeff Brown
//////////////////////////////////////////////////////////////////////////////

#include "Compression.h"
#include "LZHCompress/NoxCompress.h"
extern "C" {
#include "ZLib/zlib.h"
}
#include "EAC/codex.h"
#include "EAC/btreecodex.h"
#include "EAC/huffcodex.h"
#include "EAC/refcodex.h"

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma message("************************************** WARNING, optimization disabled for debugging purposes")
#endif

#define DEBUG_LOG(x) {}

const char *CompressionManager::getCompressionNameByType( CompressionType compType )
{
	static const char *s_compressionNames[COMPRESSION_MAX+1] = {
		"No compression",
		"RefPack",
		/*
		"LZHL",
		"ZLib 1 (fast)",
		"ZLib 2",
		"ZLib 3",
		"ZLib 4",
		"ZLib 5 (default)",
		"ZLib 6",
		"ZLib 7",
		"ZLib 8",
		"ZLib 9 (slow)",
		"BTree",
		"Huff",
		*/
	};
	return s_compressionNames[compType];
}

// For perf timers, so we can have separate ones for compression/decompression
const char *CompressionManager::getDecompressionNameByType( CompressionType compType )
{
	static const char *s_decompressionNames[COMPRESSION_MAX+1] = {
		"d_None",
		"d_RefPack",
		/*
		"d_NoxLZW",
		"d_ZLib1",
		"d_ZLib2",
		"d_ZLib3",
		"d_ZLib4",
		"d_ZLib5",
		"d_ZLib6",
		"d_ZLib7",
		"d_ZLib8",
		"d_ZLib9",
		"d_BTree",
		"d_Huff",
		*/
	};
	return s_decompressionNames[compType];
}

// ---------------------------------------------------------------------------------------

Bool CompressionManager::isDataCompressed( const void *mem, Int len )
{
	CompressionType t = getCompressionType(mem, len);
	return t != COMPRESSION_NONE;
}

CompressionType CompressionManager::getPreferredCompression( void )
{
	return COMPRESSION_REFPACK;
}


CompressionType CompressionManager::getCompressionType( const void *mem, Int len )
{
	if (len < 8)
		return COMPRESSION_NONE;

	if ( memcmp( mem, "NOX\0", 4 ) == 0 )
		return COMPRESSION_NOXLZH;

	if ( memcmp( mem, "ZL1\0", 4 ) == 0 )
		return COMPRESSION_ZLIB1;
	if ( memcmp( mem, "ZL2\0", 4 ) == 0 )
		return COMPRESSION_ZLIB2;
	if ( memcmp( mem, "ZL3\0", 4 ) == 0 )
		return COMPRESSION_ZLIB3;
	if ( memcmp( mem, "ZL4\0", 4 ) == 0 )
		return COMPRESSION_ZLIB4;
	if ( memcmp( mem, "ZL5\0", 4 ) == 0 )
		return COMPRESSION_ZLIB5;
	if ( memcmp( mem, "ZL6\0", 4 ) == 0 )
		return COMPRESSION_ZLIB6;
	if ( memcmp( mem, "ZL7\0", 4 ) == 0 )
		return COMPRESSION_ZLIB7;
	if ( memcmp( mem, "ZL8\0", 4 ) == 0 )
		return COMPRESSION_ZLIB8;
	if ( memcmp( mem, "ZL9\0", 4 ) == 0 )
		return COMPRESSION_ZLIB9;
	if ( memcmp( mem, "EAB\0", 4 ) == 0 )
		return COMPRESSION_BTREE;
	if ( memcmp( mem, "EAH\0", 4 ) == 0 )
		return COMPRESSION_HUFF;
	if ( memcmp( mem, "EAR\0", 4 ) == 0 )
		return COMPRESSION_REFPACK;

	return COMPRESSION_NONE;
}

Int CompressionManager::getMaxCompressedSize( Int uncompressedLen, CompressionType compType )
{
	switch (compType)
	{
		case COMPRESSION_NOXLZH:
			return CalcNewSize(uncompressedLen) + 8;

		case COMPRESSION_BTREE:   // guessing here
		case COMPRESSION_HUFF:    // guessing here
		case COMPRESSION_REFPACK: // guessing here
			return uncompressedLen + 8;

		case COMPRESSION_ZLIB1:
		case COMPRESSION_ZLIB2:
		case COMPRESSION_ZLIB3:
		case COMPRESSION_ZLIB4:
		case COMPRESSION_ZLIB5:
		case COMPRESSION_ZLIB6:
		case COMPRESSION_ZLIB7:
		case COMPRESSION_ZLIB8:
		case COMPRESSION_ZLIB9:
			return (Int)(ceil(uncompressedLen * 1.1 + 12 + 8));
	}

	return 0;
}

Int CompressionManager::getUncompressedSize( const void *mem, Int len )
{
	if (len < 8)
		return len;

	CompressionType compType = getCompressionType( mem, len );
	switch (compType)
	{
		case COMPRESSION_NOXLZH:
		case COMPRESSION_ZLIB1:
		case COMPRESSION_ZLIB2:
		case COMPRESSION_ZLIB3:
		case COMPRESSION_ZLIB4:
		case COMPRESSION_ZLIB5:
		case COMPRESSION_ZLIB6:
		case COMPRESSION_ZLIB7:
		case COMPRESSION_ZLIB8:
		case COMPRESSION_ZLIB9:
		case COMPRESSION_BTREE:
		case COMPRESSION_HUFF:
		case COMPRESSION_REFPACK:
			return *(Int *)(((UnsignedByte *)mem)+4);
	}

	return len;
}

Int CompressionManager::compressData( CompressionType compType, void *srcVoid, Int srcLen, void *destVoid, Int destLen )
{
	if (destLen < 8)
		return 0;

	destLen -= 8;

	UnsignedByte *src = (UnsignedByte *)srcVoid;
	UnsignedByte *dest = (UnsignedByte *)destVoid;

	if (compType == COMPRESSION_BTREE)
	{
		memcpy(dest, "EAB\0", 4);
		*(Int *)(dest+4) = 0;
		Int ret = BTREE_encode(dest+8, src, srcLen);
		if (ret)
		{
			*(Int *)(dest+4) = srcLen;
			return ret + 8;
		}
		else
			return 0;
	}
	if (compType == COMPRESSION_HUFF)
	{
		memcpy(dest, "EAH\0", 4);
		*(Int *)(dest+4) = 0;
		Int ret = HUFF_encode(dest+8, src, srcLen);
		if (ret)
		{
			*(Int *)(dest+4) = srcLen;
			return ret + 8;
		}
		else
			return 0;
	}
	if (compType == COMPRESSION_REFPACK)
	{
		memcpy(dest, "EAR\0", 4);
		*(Int *)(dest+4) = 0;
		Int ret = REF_encode(dest+8, src, srcLen);
		if (ret)
		{
			*(Int *)(dest+4) = srcLen;
			return ret + 8;
		}
		else
			return 0;
	}

	if (compType == COMPRESSION_NOXLZH)
	{
		memcpy(dest, "NOX\0", 4);
		*(Int *)(dest+4) = 0;
		Bool ret = CompressMemory(src, srcLen, dest+8, destLen);
		if (ret)
		{
			*(Int *)(dest+4) = srcLen;
			return destLen + 8;
		}
		else
			return 0;
	}

	if (compType >= COMPRESSION_ZLIB1 && compType <= COMPRESSION_ZLIB9)
	{
		Int level = compType - COMPRESSION_ZLIB1 + 1; // 1-9
		memcpy(dest, "ZL0\0", 4);
		dest[2] = '0' + level;
		*(Int *)(dest+4) = 0;

		unsigned long outLen = destLen;
		Int err = z_compress2( dest+8, &outLen, src, srcLen, level );

		if (err == Z_OK || err == Z_STREAM_END)
		{
			*(Int *)(dest+4) = srcLen;
			return outLen + 8;
		}
		else
		{
			DEBUG_LOG(("ZLib compression error (level is %d, src len is %d) %d\n", level, srcLen, err));
			return 0;
		}
	}

	return 0;
}

Int CompressionManager::decompressData( void *srcVoid, Int srcLen, void *destVoid, Int destLen )
{
	if (srcLen < 8)
		return 0;

	UnsignedByte *src = (UnsignedByte *)srcVoid;
	UnsignedByte *dest = (UnsignedByte *)destVoid;

	CompressionType compType = getCompressionType(src, srcLen);

	if (compType == COMPRESSION_BTREE)
	{
		Int slen = srcLen - 8;
		Int ret = BTREE_decode(dest, src+8, &slen);
		if (ret)
			return ret;
		else
			return 0;
	}
	if (compType == COMPRESSION_HUFF)
	{
		Int slen = srcLen - 8;
		Int ret = HUFF_decode(dest, src+8, &slen);
		if (ret)
			return ret;
		else
			return 0;
	}
	if (compType == COMPRESSION_REFPACK)
	{
		Int slen = srcLen - 8;
		Int ret = REF_decode(dest, src+8, &slen);
		if (ret)
			return ret;
		else
			return 0;
	}

	if (compType == COMPRESSION_NOXLZH)
	{
		Bool ret = DecompressMemory(src+8, srcLen-8, dest, destLen);
		if (ret)
			return destLen;
		else
			return 0;
	}

	if (compType >= COMPRESSION_ZLIB1 && compType <= COMPRESSION_ZLIB9)
	{
#ifdef DEBUG_LOGGING
		Int level = compType - COMPRESSION_ZLIB1 + 1; // 1-9
#endif

		unsigned long outLen = destLen;
		Int err = z_uncompress(dest, &outLen, src+8, srcLen-8);
		if (err == Z_OK || err == Z_STREAM_END)
		{
			return outLen;
		}
		else
		{
			DEBUG_LOG(("ZLib decompression error (src is level %d, %d bytes long) %d\n", level, srcLen, err));
			return 0;
		}
	}

	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
/////  Performance Testing  ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

#ifdef TEST_COMPRESSION

#include "GameClient/MapUtil.h"
#include "Common/FileSystem.h"
#include "Common/file.h"

#include "Common/PerfTimer.h"
enum { NUM_TIMES = 10 };

struct CompData
{
public:
	Int origSize;
	Int compressedSize[COMPRESSION_MAX+1];
};

void DoCompressTest( void )
{
	
	Int i;

	PerfGather *s_compressGathers[COMPRESSION_MAX+1];
	PerfGather *s_decompressGathers[COMPRESSION_MAX+1];
	for (i = 0; i < COMPRESSION_MAX+1; ++i)
	{
		s_compressGathers[i] = new PerfGather(CompressionManager::getCompressionNameByType((CompressionType)i));
		s_decompressGathers[i] = new PerfGather(CompressionManager::getDecompressionNameByType((CompressionType)i));
	}

	std::map<AsciiString, CompData> s_sizes;

	std::map<AsciiString, MapMetaData>::const_iterator it = TheMapCache->begin();
	while (it != TheMapCache->end())
	{
		//if (it->second.m_isOfficial)
		//{
			//++it;
			//continue;
		//}
		//static Int count = 0;
		//if (count++ > 2)
			//break;
		File *f = TheFileSystem->openFile(it->first.str());
		if (f)
		{
			DEBUG_LOG(("***************************\nTesting '%s'\n\n", it->first.str()));
			Int origSize = f->size();
			UnsignedByte *buf = (UnsignedByte *)f->readEntireAndClose();
			UnsignedByte *uncompressedBuf = NEW UnsignedByte[origSize];

			CompData d = s_sizes[it->first];
			d.origSize = origSize;
			d.compressedSize[COMPRESSION_NONE] = origSize;

			for (i=COMPRESSION_MIN; i<=COMPRESSION_MAX; ++i)
			{
				DEBUG_LOG(("=================================================\n"));
				DEBUG_LOG(("Compression Test %d\n", i));

				Int maxCompressedSize = CompressionManager::getMaxCompressedSize( origSize, (CompressionType)i );
				DEBUG_LOG(("Orig size is %d, max compressed size is %d bytes\n", origSize, maxCompressedSize));

				UnsignedByte *compressedBuf = NEW UnsignedByte[maxCompressedSize];
				memset(compressedBuf, 0, maxCompressedSize);
				memset(uncompressedBuf, 0, origSize);

				Int compressedLen, decompressedLen;

				for (Int j=0; j < NUM_TIMES; ++j) 
				{
					s_compressGathers[i]->startTimer();
					compressedLen = CompressionManager::compressData((CompressionType)i, buf, origSize, compressedBuf, maxCompressedSize);
					s_compressGathers[i]->stopTimer();
					s_decompressGathers[i]->startTimer();
					decompressedLen = CompressionManager::decompressData(compressedBuf, compressedLen, uncompressedBuf, origSize);
					s_decompressGathers[i]->stopTimer();
				}
				d.compressedSize[i] = compressedLen;
				DEBUG_LOG(("Compressed len is %d (%g%% of original size)\n", compressedLen, (double)compressedLen/(double)origSize*100.0));
				DEBUG_ASSERTCRASH(compressedLen, ("Failed to compress\n"));
				DEBUG_LOG(("Decompressed len is %d (%g%% of original size)\n", decompressedLen, (double)decompressedLen/(double)origSize*100.0));

				DEBUG_ASSERTCRASH(decompressedLen == origSize, ("orig size does not match compressed+uncompressed output\n"));
				if (decompressedLen == origSize)
				{
					Int ret = memcmp(buf, uncompressedBuf, origSize);
					if (ret != 0)
					{
						DEBUG_CRASH(("orig buffer does not match compressed+uncompressed output - ret was %d\n", ret));
					}
				}

				delete compressedBuf;
				compressedBuf = NULL;
			}

			DEBUG_LOG(("d = %d -> %d\n", d.origSize, d.compressedSize[i]));
			s_sizes[it->first] = d;
			DEBUG_LOG(("s_sizes[%s] = %d -> %d\n", it->first.str(), s_sizes[it->first].origSize, s_sizes[it->first].compressedSize[i]));

			delete[] buf;
			buf = NULL;

			delete[] uncompressedBuf;
			uncompressedBuf = NULL;
		}

		++it;
	}

	for (i=COMPRESSION_MIN; i<=COMPRESSION_MAX; ++i)
	{
		Real maxCompression = 1000.0f;
		Real minCompression = 0.0f;
		Int totalUncompressedBytes = 0;
		Int totalCompressedBytes = 0;
		for (std::map<AsciiString, CompData>::iterator cd = s_sizes.begin(); cd != s_sizes.end(); ++cd)
		{
			CompData d = cd->second;

			Real ratio = d.compressedSize[i]/(Real)d.origSize;
			maxCompression = min(maxCompression, ratio);
			minCompression = max(minCompression, ratio);

			totalUncompressedBytes += d.origSize;
			totalCompressedBytes += d.compressedSize[i];
		}
		DEBUG_LOG(("***************************************************\n"));
		DEBUG_LOG(("Compression method %s:\n", CompressionManager::getCompressionNameByType((CompressionType)i)));
		DEBUG_LOG(("%d bytes compressed to %d (%g%%)\n", totalUncompressedBytes, totalCompressedBytes,
			totalCompressedBytes/(Real)totalUncompressedBytes*100.0f));
		DEBUG_LOG(("Min ratio: %g%%, Max ratio: %g%%\n",
			minCompression*100.0f, maxCompression*100.0f));
		DEBUG_LOG(("\n"));
	}

	PerfGather::dumpAll(10000);
	//PerfGather::displayGraph(TheGameLogic->getFrame());
	PerfGather::resetAll();
	CopyFile( "AAAPerfStats.csv", "AAACompressPerfStats.csv", FALSE );

	for (i = 0; i < COMPRESSION_MAX+1; ++i) 
	{
		delete s_compressGathers[i];
		s_compressGathers[i] = NULL;

		delete s_decompressGathers[i];
		s_decompressGathers[i] = NULL;
	}

}

#endif // TEST_COMPRESSION
