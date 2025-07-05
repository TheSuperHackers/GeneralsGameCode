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
#include <Utility/stdio_adapter.h>
#include <cstdarg>
#include "Lib/BaseTypeCore.h"
#include "Compression.h"


// TheSuperHackers @todo Streamline and simplify the logging approach for tools
static void DebugLog(const char* format, ...)
{
	char buffer[1024];
	buffer[0] = 0;
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, 1024, format, args);
	va_end(args);
	printf("%s\n", buffer);
}
#define DEBUG_LOG(x) DebugLog x


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

	// just check compression on the input file if we have no output specified
	if (outFile.empty())
	{
		FILE *fpIn = fopen(inFile.c_str(), "rb");
		if (!fpIn)
		{
			DEBUG_LOG(("Cannot open '%s'\n", inFile.c_str()));
			return EXIT_FAILURE;
		}
		fseek(fpIn, 0, SEEK_END);
		int size = ftell(fpIn);
		fseek(fpIn, 0, SEEK_SET);

		char data[8];
		int numRead = fread(data, 1, 8, fpIn);
		fclose(fpIn);

		if (numRead != 8)
		{
			DEBUG_LOG(("Cannot read header from '%s'\n", inFile.c_str()));
			return EXIT_FAILURE;
		}

		CompressionType usedType = CompressionManager::getCompressionType(data, 8);
		if (usedType == COMPRESSION_NONE)
		{
			DEBUG_LOG(("No compression on '%s'\n", inFile.c_str()));
			return EXIT_SUCCESS;
		}

		int uncompressedSize = CompressionManager::getUncompressedSize(data, 8);

		DEBUG_LOG(("'%s' is compressed using %s, from %d to %d bytes, %g%% of its original size\n",
			inFile.c_str(), CompressionManager::getCompressionNameByType(usedType),
			uncompressedSize, size, size/(double)(uncompressedSize+0.1)*100.0));

		return EXIT_SUCCESS;
	}

	// Open the input file
	FILE *fpIn = fopen(inFile.c_str(), "rb");
	if (!fpIn)
	{
		DEBUG_LOG(("Cannot open input '%s'\n", inFile.c_str()));
		return EXIT_FAILURE;
	}

	// Read the input file
	fseek(fpIn, 0, SEEK_END);
	int inputSize = ftell(fpIn);
	fseek(fpIn, 0, SEEK_SET);

	char *inputData = new char[inputSize];
	int numRead = fread(inputData, 1, inputSize, fpIn);
	fclose(fpIn);
	if (numRead != inputSize)
	{
		DEBUG_LOG(("Cannot read input '%s'\n", inFile.c_str()));
		delete[] inputData;
		return EXIT_FAILURE;
	}

	DEBUG_LOG(("Read %d bytes from '%s'\n", numRead, inFile.c_str()));

	// Open the output file
	FILE *fpOut = fopen(outFile.c_str(), "wb");
	if (!fpOut)
	{
		DEBUG_LOG(("Cannot open output '%s'\n", outFile.c_str()));
		delete[] inputData;
		return EXIT_FAILURE;
	}


	if (compressType == COMPRESSION_NONE)
	{
		DEBUG_LOG(("No compression requested, writing uncompressed data\n"));
		int outSize = CompressionManager::getUncompressedSize(inputData, inputSize);
		char *outData = new char[outSize];
		CompressionManager::decompressData(inputData, inputSize, outData, outSize);

		// Write the output file
		fwrite(outData, 1, outSize, fpOut);
	}
	else 
	{
		DEBUG_LOG(("Compressing data using %s\n", CompressionManager::getCompressionNameByType(compressType)));
		// Allocate the output buffer
		int outSize = CompressionManager::getMaxCompressedSize(inputSize, compressType);
		char *outData = new char[outSize];
		int compressedSize = CompressionManager::compressData(compressType, inputData, inputSize, outData, outSize);

		// Write the output file
		fwrite(outData, 1, compressedSize, fpOut);
		delete[] outData;
	}

	fclose(fpOut);
	delete[] inputData;

	return EXIT_SUCCESS;
}
