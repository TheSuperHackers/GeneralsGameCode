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

/* $Header: /Commando/Code/wwlib/chunkio.cpp 11    7/12/01 7:44p Jani_p $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Tiberian Sun / Commando / G Library                          *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/chunkio.cpp                            $*
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/10/01 7:38p                                               $*
 *                                                                                             *
 *                    $Revision:: 11                                                          $*
 *                                                                                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "chunkio.h"
#include <assert.h>


ChunkSaveClass::ChunkSaveClass(FileClass * file) :
	File(file),
	StackIndex(0),
	InMicroChunk(false),
	MicroChunkPosition(0)
{
	memset(PositionStack,0,sizeof(PositionStack));
	memset(HeaderStack,0,sizeof(HeaderStack));
	memset(&MCHeader,0,sizeof(MCHeader));
}


bool ChunkSaveClass::Begin_Chunk(uint32 id)
{
	ChunkHeader	chunkh;
	int 			filepos;

	// If we have a parent chunk, set its 'Contains_Chunks' flag
	if (StackIndex > 0) {
		HeaderStack[StackIndex-1].Set_Sub_Chunk_Flag(true);
	}

	// Save the current file position and chunk header
	// for the call to End_Chunk.
	chunkh.Set_Type(id);
	chunkh.Set_Size(0);
	filepos = File->Seek(0);

	PositionStack[StackIndex] = filepos;
	HeaderStack[StackIndex] = chunkh;
	StackIndex++;

	// write a temporary chunk header (size = 0)
	if (File->Write(&chunkh,sizeof(chunkh)) != sizeof(chunkh)) {
		return false;
	}
	return true;
}


// Computes the size and adds it to the header.
bool ChunkSaveClass::End_Chunk()
{
	// If the user didn't close his micro chunks bad things are gonna happen
	assert(!InMicroChunk);

	// Save the current position
	int curpos = File->Seek(0);

	// Pop the position and chunk header off the stacks
	StackIndex--;
	int chunkpos = PositionStack[StackIndex];
	ChunkHeader chunkh = HeaderStack[StackIndex];

	// write the completed header
	File->Seek(chunkpos,SEEK_SET);
	if (File->Write(&chunkh,sizeof(chunkh)) != sizeof(chunkh)) {
		return false;
	}

	// Add the total bytes written to any encompasing chunk
	if (StackIndex != 0) {
		HeaderStack[StackIndex-1].Add_Size(chunkh.Get_Size() + sizeof(chunkh));
	}

	// Go back to the end of the file
	File->Seek(curpos,SEEK_SET);

	return true;
}


// Micro chunks are used to wrap individual variables. They aren't hierarchical so if you attempt to
// open a micro chunk while already in one, an assert will occur.
// WARNING: Id is asserted to be between 0 and 255.
bool ChunkSaveClass::Begin_Micro_Chunk(uint32 id)
{
	assert(id < 256);
	assert(!InMicroChunk);

	// Save the current file position and chunk header
	// for the call to End_Micro_Chunk.
	MCHeader.Set_Type(id);
	MCHeader.Set_Size(0);
	MicroChunkPosition = File->Seek(0);

	// Write a temporary chunk header
	// NOTE: I'm calling the ChunkSaveClass::Write method so that the bytes for
	// this header are tracked in the wrapping chunk.  This is because micro-chunks
	// are simply data inside the normal chunks...
	if (Write(&MCHeader,sizeof(MCHeader)) != sizeof(MCHeader)) {
		return false;
	}

	InMicroChunk = true;
	return true;
}


bool ChunkSaveClass::End_Micro_Chunk()
{
	assert(InMicroChunk);

	// Save the current position
	int curpos = File->Seek(0);

	// Seek back and write the micro chunk header
	File->Seek(MicroChunkPosition,SEEK_SET);
	if (File->Write(&MCHeader,sizeof(MCHeader)) != sizeof(MCHeader)) {
		return false;
	}

	// Go back to the end of the file
	File->Seek(curpos,SEEK_SET);
	InMicroChunk = false;
	return true;
}

uint32 ChunkSaveClass::Write(const void * buf, uint32 nbytes)
{
	// If this assert hits, you mixed data and chunks within the same chunk NO NO!
	assert(HeaderStack[StackIndex-1].Get_Sub_Chunk_Flag() == 0);

	// If this assert hits, you didnt open any chunks yet
	assert(StackIndex > 0);

	// write the bytes into the file
	if (File->Write(buf,nbytes) != (int)nbytes) return 0;

	// track them in the wrapping chunk
	HeaderStack[StackIndex-1].Add_Size(nbytes);

	// track them if you are using a micro-chunk too.
	if (InMicroChunk) {
		assert(MCHeader.Get_Size() < 255 - nbytes);	// micro chunks can only be 255 bytes
		MCHeader.Add_Size(nbytes);
	}

	return nbytes;
}


uint32 ChunkSaveClass::Write(const IOVector2Struct & v)
{
	return Write(&v,sizeof(v));
}


uint32 ChunkSaveClass::Write(const IOVector3Struct & v)
{
	return Write(&v,sizeof(v));
}


uint32 ChunkSaveClass::Write(const IOVector4Struct & v)
{
	return Write(&v,sizeof(v));
}

uint32 ChunkSaveClass::Write(const IOQuaternionStruct & q)
{
	return Write(&q,sizeof(q));
}

int ChunkSaveClass::Cur_Chunk_Depth()
{
	return StackIndex;
}


ChunkLoadClass::ChunkLoadClass(FileClass * file) :
	File(file),
	StackIndex(0),
	InMicroChunk(false),
	MicroChunkPosition(0)
{
	memset(PositionStack,0,sizeof(PositionStack));
	memset(HeaderStack,0,sizeof(HeaderStack));
	memset(&MCHeader,0,sizeof(MCHeader));
}


// Reads in the chunk header.
bool ChunkLoadClass::Open_Chunk()
{
	// if user didn't close any micro chunks that he opened, bad things could happen
	assert(InMicroChunk == false);

	// check for stack overflow
	assert(StackIndex < MAX_STACK_DEPTH-1);

	// if the parent chunk has been completely eaten, return false
	if ((StackIndex > 0) && (PositionStack[StackIndex-1] == HeaderStack[StackIndex-1].Get_Size())) {
		return false;
	}

	// read the chunk header
	if (File->Read(&HeaderStack[StackIndex],sizeof(ChunkHeader)) != sizeof(ChunkHeader)) {
		return false;
	}

	PositionStack[StackIndex] = 0;
	StackIndex++;
	return true;
}


// Seeks to the end if needed.
bool ChunkLoadClass::Close_Chunk()
{
	// if user didn't close any micro chunks that he opened, bad things could happen
	assert(InMicroChunk == false);

	// check for stack overflow
	assert(StackIndex > 0);

	int csize = HeaderStack[StackIndex-1].Get_Size();
	int pos = PositionStack[StackIndex-1];

	if (pos < csize) {
		File->Seek(csize - pos,SEEK_CUR);
	}

	StackIndex--;
	if (StackIndex > 0) {
		PositionStack[StackIndex - 1] += csize + sizeof(ChunkHeader);
	}

	return true;
}


uint32 ChunkLoadClass::Cur_Chunk_ID()
{
	assert(StackIndex >= 1);
	return HeaderStack[StackIndex-1].Get_Type();
}


uint32 ChunkLoadClass::Cur_Chunk_Length()
{
	assert(StackIndex >= 1);
	return HeaderStack[StackIndex-1].Get_Size();
}


int ChunkLoadClass::Cur_Chunk_Depth()
{
	return StackIndex;
}


// Test whether the current chunk contains chunks (or data).
int ChunkLoadClass::Contains_Chunks()
{
	return HeaderStack[StackIndex-1].Get_Sub_Chunk_Flag();
}

// Reads in a micro-chunk header.
bool ChunkLoadClass::Open_Micro_Chunk()
{
	assert(!InMicroChunk);

	// read the chunk header
	// calling the ChunkLoadClass::Read fn so that if we exhaust the chunk, the read will fail
	if (Read(&MCHeader,sizeof(MCHeader)) != sizeof(MCHeader)) {
		return false;
	}

	InMicroChunk = true;
	MicroChunkPosition = 0;
	return true;
}


// Seeks to the end of the micro-chunk.
bool ChunkLoadClass::Close_Micro_Chunk()
{
	assert(InMicroChunk);
	InMicroChunk = false;

	int csize = MCHeader.Get_Size();
	int pos = MicroChunkPosition;

	// seek the file past this micro chunk
	if (pos < csize) {

		File->Seek(csize - pos,SEEK_CUR);

		// update the tracking variables for where we are in the normal chunk.
		if (StackIndex > 0) {
			PositionStack[StackIndex-1] += csize - pos;
		}
	}

	return true;
}


// Micro chunks have an id between 0 and 255.
// WARNING: Asserts if you are not currently inside a micro-chunk.
uint32 ChunkLoadClass::Cur_Micro_Chunk_ID()
{
	assert(InMicroChunk);
	return MCHeader.Get_Type();
}


// Micro chunks have a maximum size of 255 bytes.
// WARNING: Asserts if you are not currently inside a micro-chunk.
uint32 ChunkLoadClass::Cur_Micro_Chunk_Length()
{
	assert(InMicroChunk);
	return MCHeader.Get_Size();
}

// Seek over nbytes in the stream
uint32 ChunkLoadClass::Seek(uint32 nbytes)
{
	assert(StackIndex >= 1);

	// Don't seek if we would go past the end of the current chunk
	if (PositionStack[StackIndex-1] + nbytes > (int)HeaderStack[StackIndex-1].Get_Size()) {
		return 0;
	}

	// Don't read if we are in a micro chunk and would go past the end of it
	if (InMicroChunk && MicroChunkPosition + nbytes > MCHeader.Get_Size()) {
		return 0;
	}

	uint32 curpos=File->Tell();
	if (File->Seek(nbytes,SEEK_CUR)-curpos != (int)nbytes) {
		return 0;
	}

	// Update our position in the chunk
	PositionStack[StackIndex-1] += nbytes;

	// Update our position in the micro chunk if we are in one
	if (InMicroChunk) {
		MicroChunkPosition += nbytes;
	}

	return nbytes;
}

uint32 ChunkLoadClass::Read(void * buf,uint32 nbytes)
{
	assert(StackIndex >= 1);

	// Don't read if we would go past the end of the current chunk
	if (PositionStack[StackIndex-1] + nbytes > (int)HeaderStack[StackIndex-1].Get_Size()) {
		return 0;
	}

	// Don't read if we are in a micro chunk and would go past the end of it
	if (InMicroChunk && MicroChunkPosition + nbytes > MCHeader.Get_Size()) {
		return 0;
	}

	if (File->Read(buf,nbytes) != (int)nbytes) {
		return 0;
	}

	// Update our position in the chunk
	PositionStack[StackIndex-1] += nbytes;

	// Update our position in the micro chunk if we are in one
	if (InMicroChunk) {
		MicroChunkPosition += nbytes;
	}

	return nbytes;
}


uint32 ChunkLoadClass::Read(IOVector2Struct * v)
{
	assert(v != nullptr);
	return Read(v,sizeof(v));
}


uint32 ChunkLoadClass::Read(IOVector3Struct * v)
{
	assert(v != nullptr);
	return Read(v,sizeof(v));
}


uint32 ChunkLoadClass::Read(IOVector4Struct * v)
{
	assert(v != nullptr);
	return Read(v,sizeof(v));
}


uint32 ChunkLoadClass::Read(IOQuaternionStruct * q)
{
	assert(q != nullptr);
	return Read(q,sizeof(q));
}

