/*
**	Command & Conquer Generals(tm)
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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: Compression.cpp /////////////////////////////////////////////////////
// Author: Matthew D. Campbell
//////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"
#include "Compression.h"

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
/////  Performance Testing  ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

//#define TEST_COMPRESSION
#ifdef TEST_COMPRESSION

#include "GameClient/MapUtil.h"
#include "Common/FileSystem.h"
#include "Common/file.h"

#include "Common/PerfTimer.h"
enum { NUM_TIMES = 1 };

struct CompData
{
public:
	Int origSize;
	Int compressedSize[COMPRESSION_MAX+1];
};

#define TEST_COMPRESSION_MIN COMPRESSION_BTREE
#define TEST_COMPRESSION_MAX COMPRESSION_MAX

void DoCompressTest( void )
{
	
	Int i;

	/*
	PerfGather *s_compressGathers[TEST_COMPRESSION_MAX+1];
	PerfGather *s_decompressGathers[TEST_COMPRESSION_MAX+1];
	for (i = TEST_COMPRESSION_MIN; i < TEST_COMPRESSION_MAX+1; ++i)
	{
		s_compressGathers[i] = new PerfGather(CompressionManager::getCompressionNameByType((CompressionType)i));
		s_decompressGathers[i] = new PerfGather(CompressionManager::getDecompressionNameByType((CompressionType)i));
	}
	*/

	std::map<AsciiString, CompData> s_sizes;

	std::map<AsciiString, MapMetaData>::const_iterator it = TheMapCache->find("userdata\\maps\\_usa01\\_usa01.map");
	if (it != TheMapCache->end())
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

			for (i=TEST_COMPRESSION_MIN; i<=TEST_COMPRESSION_MAX; ++i)
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
					//s_compressGathers[i]->startTimer();
					compressedLen = CompressionManager::compressData((CompressionType)i, buf, origSize, compressedBuf, maxCompressedSize);
					//s_compressGathers[i]->stopTimer();
					//s_decompressGathers[i]->startTimer();
					decompressedLen = CompressionManager::decompressData(compressedBuf, compressedLen, uncompressedBuf, origSize);
					//s_decompressGathers[i]->stopTimer();
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

	for (i=TEST_COMPRESSION_MIN; i<=TEST_COMPRESSION_MAX; ++i)
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

	/*
	PerfGather::dumpAll(10000);
	PerfGather::resetAll();
	CopyFile( "AAAPerfStats.csv", "AAACompressPerfStats.csv", FALSE );

	for (i = TEST_COMPRESSION_MIN; i < TEST_COMPRESSION_MAX+1; ++i) 
	{
		delete s_compressGathers[i];
		s_compressGathers[i] = NULL;

		delete s_decompressGathers[i];
		s_decompressGathers[i] = NULL;
	}
	*/

}

#endif // TEST_COMPRESSION
