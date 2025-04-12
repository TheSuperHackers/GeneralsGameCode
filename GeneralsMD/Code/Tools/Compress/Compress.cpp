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

#include <string>
#include <cstdio>
#include "Lib/BaseType.h"
#include "Compression.h"


// TheSuperHackers @todo Streamline and simplify the logging approach for tools
#define DEBUG_LOG(x) printf x


void dumpHelp(const char *exe)
{
	DEBUG_LOG(("Usage:\n  To print the compression type of an existing file: %s -in infile\n", exe));
	DEBUG_LOG(("  To compress a file: %s -in infile -out outfile <-type compressionmode>\n\n", exe));
	DEBUG_LOG(("Compression modes:\n"));
	for (int i=COMPRESSION_MIN; i<=COMPRESSION_MAX; ++i)
	{
		DEBUG_LOG(("   %s\n", CompressionManager::getCompressionNameByType((CompressionType)i)));
	}
}

int main(int argc, char **argv)
{
	std::string inFile = "";
	std::string outFile = "";
	CompressionType compressType = CompressionManager::getPreferredCompression();

	for (int i=1; i<argc; ++i)
	{
		if ( !stricmp(argv[i], "-help") )
		{
			dumpHelp(argv[0]);
			return EXIT_SUCCESS;
		}

		if ( !strcmp(argv[i], "-in") )
		{
			++i;
			if (i<argc)
			{
				inFile = argv[i];
			}
		}

		if ( !strcmp(argv[i], "-out") )
		{
			++i;
			if (i<argc)
			{
				outFile = argv[i];
			}
		}

		if ( !strcmp(argv[i], "-type") )
		{
			++i;
			if (i<argc)
			{
				for (int j=COMPRESSION_MIN; j<=COMPRESSION_MAX; ++j)
				{
					if ( !stricmp(CompressionManager::getCompressionNameByType((CompressionType)j), argv[i]) )
					{
						compressType = (CompressionType)j;
						break;
					}
				}
			}
		}
	}

	if (inFile.empty())
	{
		dumpHelp(argv[0]);
		return EXIT_SUCCESS;
	}

	DEBUG_LOG(("IN:'%s' OUT:'%s' Compression:'%s'\n",
		inFile.c_str(), outFile.c_str(), CompressionManager::getCompressionNameByType(compressType)));

	if (outFile.empty())
	{
		// just check compression
		FILE *fp = fopen(inFile.c_str(), "rb");
		if (!fp)
		{
			DEBUG_LOG(("Cannot open '%s'\n", inFile.c_str()));
			return EXIT_FAILURE;
		}
		fseek(fp, 0, SEEK_END);
		int size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		char data[8];
		int numRead = fread(data, 1, 8, fp);

		if (numRead != 8)
		{
			DEBUG_LOG(("Cannot read header from '%s'\n", inFile.c_str()));
			return EXIT_FAILURE;
		}

		CompressionType usedType = CompressionManager::getCompressionType(data, 8);
		if (usedType == COMPRESSION_NONE)
		{
			DEBUG_LOG(("No compression on '%s'\n", inFile.c_str()));
			return EXIT_FAILURE;
		}

		int uncompressedSize = CompressionManager::getUncompressedSize(data, 8);

		DEBUG_LOG(("'%s' is compressed using %s, from %d to %d bytes, %g%% of its original size\n",
			inFile.c_str(), CompressionManager::getCompressionNameByType(usedType),
			uncompressedSize, size, size/(double)(uncompressedSize+0.1)*100.0));

		return EXIT_SUCCESS;
	}

	// compress file
	return EXIT_FAILURE;
}
