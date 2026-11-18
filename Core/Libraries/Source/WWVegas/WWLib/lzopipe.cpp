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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Library/LZOPIPE.cpp                               $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include	"always.h"
#include "BUFF.h"
#include	"lzo.h"
#include	"lzopipe.h"
#include	<assert.h>
#include	<string.h>


LZOPipe::LZOPipe(CompControl control, int blocksize) :
		Control(control),
		Counter(0),
		Buffer(nullptr),
		Buffer2(nullptr),
		BlockSize(blocksize)
{
	SafetyMargin = BlockSize;
	Buffer = new char[BlockSize+SafetyMargin];
	Buffer2 = new char[BlockSize+SafetyMargin];
	BlockHeader.CompCount = 0xFFFF;
}


LZOPipe::~LZOPipe()
{
	delete [] Buffer;
	Buffer = nullptr;

	delete [] Buffer2;
	Buffer2 = nullptr;
}


// Accumulates the necessary bytes to make a whole block. Then the block is processed and the entire
// contents are flushed to the next pipe segment in the chain.
// WARNING: The compression process may be slow as well as consuming two buffers.
int LZOPipe::Put(void const * source, int slen)
{
	if (source == nullptr || slen < 1) {
		return(Pipe::Put(source, slen));
	}

	assert(Buffer != nullptr);

	int total = 0;

	/*
	**	Copy as much as can fit into the buffer from the source data supplied.
	*/
	if (Control ==  DECOMPRESS) {

		while (slen > 0) {

			/*
			**	First check to see if we are in the block header accumulation phase.
			**	When a whole block header has been accumulated, only then will the regular
			**	data processing begin for the block.
			*/
			if (BlockHeader.CompCount == 0xFFFF) {
				int len = ((unsigned)slen < (sizeof(BlockHeader)-Counter)) ? slen : (sizeof(BlockHeader)-Counter);
				memmove(&Buffer[Counter], source, len);
				source = ((char *)source) + len;
				slen -= len;
				Counter += len;

				/*
				**	A whole block header has been accumulated. Store it for safekeeping.
				*/
				if (Counter == sizeof(BlockHeader)) {
					memmove(&BlockHeader, Buffer, sizeof(BlockHeader));
					Counter = 0;
				}
			}

			/*
			**	Fill the buffer with compressed data until there is enough to make a whole
			**	data block.
			*/
			if (slen > 0) {
				int len = (slen < (BlockHeader.CompCount-Counter)) ? slen : (BlockHeader.CompCount-Counter);

				memmove(&Buffer[Counter], source, len);
				slen -= len;
				source = ((char *)source) + len;
				Counter += len;

				/*
				**	If an entire block has been accumulated, then uncompress it and feed it
				**	through the pipe.
				*/
				if (Counter == BlockHeader.CompCount) {
					unsigned int length = sizeof (Buffer2);
					lzo1x_decompress ((unsigned char*)Buffer, BlockHeader.CompCount, (unsigned char*)Buffer2, &length, nullptr);
					total += Pipe::Put(Buffer2, BlockHeader.UncompCount);
					Counter = 0;
					BlockHeader.CompCount = 0xFFFF;
				}
			}
		}

	} else {

		/*
		**	If the buffer already contains some data, then any new data must be stored
		**	into the staging buffer until a full set has been accumulated.
		*/
		if (Counter > 0) {
			int tocopy = (slen < (BlockSize-Counter)) ? slen : (BlockSize-Counter);
			memmove(&Buffer[Counter], source, tocopy);
			source = ((char *)source) + tocopy;
			slen -= tocopy;
			Counter += tocopy;

			if (Counter == BlockSize) {
				unsigned int len = sizeof (Buffer2);
				char *dictionary = new char [64*1024];
				lzo1x_1_compress ((unsigned char*)Buffer, BlockSize, (unsigned char*)Buffer2, &len, dictionary);
				delete [] dictionary;
				BlockHeader.CompCount = (unsigned short)len;
				BlockHeader.UncompCount = (unsigned short)BlockSize;
				total += Pipe::Put(&BlockHeader, sizeof(BlockHeader));
				total += Pipe::Put(Buffer2, len);
				Counter = 0;
			}
		}

		/*
		**	Process the source data in whole block chunks until there is insufficient
		**	source data left for a whole data block.
		*/
		while (slen >= BlockSize) {
			unsigned int len = sizeof (Buffer2);
			char *dictionary = new char [64*1024];
			lzo1x_1_compress ((unsigned char*)source, BlockSize, (unsigned char*)Buffer2, &len, dictionary);
			delete [] dictionary;
			source = ((char *)source) + BlockSize;
			slen -= BlockSize;

			BlockHeader.CompCount = (unsigned short)len;
			BlockHeader.UncompCount = (unsigned short)BlockSize;
			total += Pipe::Put(&BlockHeader, sizeof(BlockHeader));
			total += Pipe::Put(Buffer2, len);
		}

		/*
		**	If there is any remaining data, then it is stored into the buffer
		**	until a full data block has been accumulated.
		*/
		if (slen > 0) {
			memmove(Buffer, source, slen);
			Counter = slen;
		}
	}

	return(total);
}


// Flushes any partially accumulated block. For the compression process, this will generate the
// sub-sized compressed block. For decompression, this routine should not have any data in the
// buffer. In such a case, it means that the data source was prematurely truncated, and the
// accumulated data is just dumped through the pipe.
int LZOPipe::Flush()
{
	assert(Buffer != nullptr);

	int total = 0;

	/*
	**	If there is accumulated data, then it must processed.
	*/
	if (Counter > 0) {
		if (Control == DECOMPRESS) {

			/*
			**	If the accumulated data is insufficient to make a block header, then
			**	this means the data has been truncated. Just dump the data through
			**	as if were already decompressed.
			*/
			if (BlockHeader.CompCount == 0xFFFF) {
				total += Pipe::Put(Buffer, Counter);
				Counter = 0;
			}

			/*
			**	There appears to be a partial block accumulated in the buffer. It would
			**	be disastrous to try to decompress the data since there wouldn't be
			**	the special end of data code that LZO decompression needs. In this
			**	case, dump the data out as if it were already decompressed.
			*/
			if (Counter > 0) {
				total += Pipe::Put(&BlockHeader, sizeof(BlockHeader));
				total += Pipe::Put(Buffer, Counter);
				Counter = 0;
				BlockHeader.CompCount = 0xFFFF;
			}

		} else {

			/*
			**	A partial block in the compression process is a normal occurrence. Just
			**	compress the partial block and output normally.
			*/
			unsigned int len = sizeof (Buffer2);
			char *dictionary = new char [64*1024];
			lzo1x_1_compress ((unsigned char*)Buffer, Counter, (unsigned char *)Buffer2, &len, dictionary);
			delete [] dictionary;
			BlockHeader.CompCount = (unsigned short)len;
			BlockHeader.UncompCount = (unsigned short)Counter;
			total += Pipe::Put(&BlockHeader, sizeof(BlockHeader));
			total += Pipe::Put(Buffer2, len);
			Counter = 0;
		}
	}

	total += Pipe::Flush();
	return(total);
}

