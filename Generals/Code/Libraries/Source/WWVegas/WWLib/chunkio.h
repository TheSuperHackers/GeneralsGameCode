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

/* $Header: /Commando/Code/wwlib/chunkio.h 21    7/31/01 6:41p Patrick $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Tiberian Sun / Commando / G Library                          * 
 *                                                                                             * 
 *                     $Archive:: /Commando/Code/wwlib/chunkio.h                              $* 
 *                                                                                             * 
 *                      $Author:: Patrick                                                     $* 
 *                                                                                             * 
 *                     $Modtime:: 7/27/01 2:47p                                               $* 
 *                                                                                             * 
 *                    $Revision:: 21                                                          $* 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef CHUNKIO_H
#define CHUNKIO_H

#ifndef ALWAYS_H
#include "always.h"
#endif

#ifndef BITTYPE_H
#include "bittype.h"
#endif

#ifndef WWFILE_H
#include "WWFILE.H"
#endif

#ifndef IOSTRUCT_H
#include "iostruct.h"
#endif


/************************************************************************************

	ChunkIO

	(gth) This module provides classes for reading and writing chunk-based files.
	For example, all of the w3d files are stored in a hierarchical-chunk format.
	Basically the format is similar to IFF.  All data in the file has chunk headers
	wrapped around it.  A chunk header contains an ID, and a Size.  The size
	is the number of bytes in the chunk (not including the header).  The
	contents of a chunk may be either: more "sub-chunks" or raw data.  These classes
	will automatically keep track of your positions within all of the sub and parent
	chunks (to some maximum recursion depth).

	Sept 3, 1999
	(gth) Adding the new concept of "micro-chunks".  Instead of filling the contents of a
	chunk with data, you can fill it with "micro-chunks" which contain a single byte
	id and a single byte size.  Micro-chunks are used for storing simple variables
	in a form that can survive revisions to the file format without paying the price
	for a full chunk header.  You CANNOT recursively embed micro-chunks due to their
	size limitations....

	Sept 24, 1999
	(gth) Using the MSB of the chunksize to indicate whether a chunk contains other
	chunks or pure data.  If the MSB is 0, the chunk contains data (so that the reader
	I'm going to write doesn't break on older files) and if it is 1 then it is
	assumed to contain other chunks.  This does not apply to micro-chunks as they
	are considered data.

**************************************************************************************/

struct ChunkHeader
{
	// Functions.
	ChunkHeader() : ChunkType(0), ChunkSize(0) {}
	ChunkHeader(uint32 type, uint32 size) {ChunkType = type; ChunkSize = size;}

	// Use these accessors to ensure you correctly deal with the data in the chunk header
	void		Set_Type(uint32 type)					{ ChunkType = type; }
	uint32	Get_Type(void)								{ return ChunkType; }
	void		Set_Size(uint32 size)					{ ChunkSize &= 0x80000000; ChunkSize |= (size & 0x7FFFFFFF); }
	void		Add_Size(uint32 add)						{ Set_Size(Get_Size() + add); }
	uint32	Get_Size(void)								{ return (ChunkSize & 0x7FFFFFFF); }
	void		Set_Sub_Chunk_Flag(bool onoff)		{ if (onoff) { ChunkSize |= 0x80000000; } else { ChunkSize &= 0x7FFFFFFF; } }
	int		Get_Sub_Chunk_Flag(void)				{ return (ChunkSize & 0x80000000); }

	// Chunk type and size.
	// Note: MSB of ChunkSize is used to indicate whether this chunk
	// contains other chunks or data.
	uint32 ChunkType;
	uint32 ChunkSize;
};

struct MicroChunkHeader
{
	MicroChunkHeader() {}
	MicroChunkHeader(uint8 type, uint8 size) { ChunkType = type, ChunkSize = size; }

	void		Set_Type(uint8 type)						{ ChunkType = type; }
	uint8		Get_Type(void)								{ return ChunkType; }
	void		Set_Size(uint8 size)						{ ChunkSize = size; }
	void		Add_Size(uint8 add)						{ Set_Size(Get_Size() + add); }
	uint8		Get_Size(void)								{ return ChunkSize; }

	uint8	ChunkType;
	uint8	ChunkSize;
};


/**************************************************************************************
**
** ChunkSaveClass
** Wrap an instance of this class around an opened file for easy chunk
** creation.
**
**************************************************************************************/
class ChunkSaveClass
{
public:
	ChunkSaveClass(FileClass * file);

	// Chunk methods
	bool					Begin_Chunk(uint32 id);
	bool					End_Chunk();
	int					Cur_Chunk_Depth();

	// Micro chunk methods
	bool					Begin_Micro_Chunk(uint32 id);
	bool					End_Micro_Chunk();

	// Write data into the file
	uint32				Write(const void *buf, uint32 nbytes);
	uint32				Write(const IOVector2Struct & v);
	uint32				Write(const IOVector3Struct & v);
	uint32				Write(const IOVector4Struct & v);
	uint32				Write(const IOQuaternionStruct & q);

private:

	enum { MAX_STACK_DEPTH = 256 };

	FileClass *			File;

	// Chunk building support
	int					StackIndex;
	int					PositionStack[MAX_STACK_DEPTH];
	ChunkHeader			HeaderStack[MAX_STACK_DEPTH];

	// MicroChunk building support
	bool					InMicroChunk;
	int					MicroChunkPosition;
	MicroChunkHeader	MCHeader;
};


/**************************************************************************************
**
** ChunkLoadClass
** wrap an instance of one of these objects around an opened file
** to easily parse the chunks in the file
**
**************************************************************************************/
class ChunkLoadClass
{
public:

	ChunkLoadClass(FileClass * file);

	// Chunk methods
	bool					Open_Chunk();
	bool					Close_Chunk();
	uint32				Cur_Chunk_ID();
	uint32				Cur_Chunk_Length();
	int					Cur_Chunk_Depth();
	int					Contains_Chunks();

	// Micro Chunk methods
	bool					Open_Micro_Chunk();
	bool					Close_Micro_Chunk();
	uint32				Cur_Micro_Chunk_ID();
	uint32				Cur_Micro_Chunk_Length();

	// Read a block of bytes from the output stream.
	uint32				Read(void *buf, uint32 nbytes);
	uint32				Read(IOVector2Struct * v);
	uint32				Read(IOVector3Struct * v);
	uint32				Read(IOVector4Struct * v);
	uint32				Read(IOQuaternionStruct * q);

	// Seek over a block of bytes in the stream (same as Read but don't copy the data to a buffer)
	uint32				Seek(uint32 nbytes);

private:

	enum { MAX_STACK_DEPTH = 256 };

	FileClass *			File;

	// Chunk reading support
	int					StackIndex;
	uint32				PositionStack[MAX_STACK_DEPTH];
	ChunkHeader			HeaderStack[MAX_STACK_DEPTH];

	// Micro-chunk reading support
	bool					InMicroChunk;
	int					MicroChunkPosition;
	MicroChunkHeader	MCHeader;

};

/*
** WRITE_WWSTRING_CHUNK	- use this one-line macro to easily create a chunk to save a potentially
** long string.  Note:  This macro does NOT create a micro chunk...
** Example:
**
**	csave.Begin_Chunk(CHUNKID_PARENT);
**		ParentClass::Save (csave);
**	csave.End_Chunk();
**
**	WRITE_WWSTRING_CHUNK(csave, CHUNKID_NAME, string);
**	WRITE_WIDESTRING_CHUNK(csave, CHUNKID_WIDE_NAME, wide_string);
**
**	csave.Begin_Chunk(PHYSGRID_CHUNK_VARIABLES);
**	WRITE_MICRO_CHUNK(csave,PHYSGRID_VARIABLE_VERSION,version);
**	WRITE_MICRO_CHUNK(csave,PHYSGRID_VARIABLE_DUMMYVISID,DummyVisId);
**	WRITE_MICRO_CHUNK(csave,PHYSGRID_VARIABLE_BASEVISID,BaseVisId);
**	csave.End_Chunk();
**
*/
#define WRITE_WWSTRING_CHUNK(csave,id,var) { \
	csave.Begin_Chunk(id); \
	csave.Write((const TCHAR *)var, var.Get_Length () + 1); \
	csave.End_Chunk(); }

#define WRITE_WIDESTRING_CHUNK(csave,id,var) { \
	csave.Begin_Chunk(id); \
	csave.Write((const WCHAR *)var, (var.Get_Length () + 1) * 2); \
	csave.End_Chunk(); }


/*
** READ_WWSTRING_CHUNK	- use this macro in a switch statement to read the contents
**	of a chunk into a string object.
** Example:
**
**	while (cload.Open_Chunk()) {
**
**		switch(cload.Cur_Chunk_ID()) {
**			READ_WWSTRING_CHUNK(cload,CHUNKID_NAME,string);
**			READ_WIDESTRING_CHUNK(cload,CHUNKID_WIDE_NAME,wide_string);
**		}
**		cload.Close_Chunk();
**	}
**
*/
#define READ_WWSTRING_CHUNK(cload,id,var)		\
	case (id):	cload.Read(var.Get_Buffer(cload.Cur_Chunk_Length()),cload.Cur_Chunk_Length()); break;	\

#define READ_WIDESTRING_CHUNK(cload,id,var)		\
	case (id):	cload.Read(var.Get_Buffer((cload.Cur_Chunk_Length()+1)/2),cload.Cur_Chunk_Length()); break;	\


/*
** WRITE_MICRO_CHUNK	- use this one-line macro to easily make a micro chunk for an individual variable.
** Note that you should always wrap your micro-chunks inside a normal chunk.
** Example:
**
**	csave.Begin_Chunk(PHYSGRID_CHUNK_VARIABLES);
**	WRITE_MICRO_CHUNK(csave,PHYSGRID_VARIABLE_VERSION,version);
**	WRITE_MICRO_CHUNK(csave,PHYSGRID_VARIABLE_DUMMYVISID,DummyVisId);
**	WRITE_MICRO_CHUNK(csave,PHYSGRID_VARIABLE_BASEVISID,BaseVisId);
**	csave.End_Chunk();
*/
#define WRITE_MICRO_CHUNK(csave,id,var) { \
	csave.Begin_Micro_Chunk(id); \
	csave.Write(&var,sizeof(var)); \
	csave.End_Micro_Chunk(); }

#define WRITE_SAFE_MICRO_CHUNK(csave,id,var,type) { \
	csave.Begin_Micro_Chunk(id);		\
	type data = (type)var;				\
	csave.Write(&data,sizeof(data)); \
	csave.End_Micro_Chunk(); }

#define WRITE_MICRO_CHUNK_STRING(csave,id,var) { \
	csave.Begin_Micro_Chunk(id); \
	csave.Write(var, strlen(var) + 1); \
	csave.End_Micro_Chunk(); }

#define WRITE_MICRO_CHUNK_WWSTRING(csave,id,var) { \
	csave.Begin_Micro_Chunk(id); \
	csave.Write((const TCHAR *)var, var.Get_Length () + 1); \
	csave.End_Micro_Chunk(); }

#define WRITE_MICRO_CHUNK_WIDESTRING(csave,id,var) { \
	csave.Begin_Micro_Chunk(id); \
	csave.Write((const WCHAR *)var, (var.Get_Length () + 1) * 2); \
	csave.End_Micro_Chunk(); }


/*
** READ_MICRO_CHUNK - use this macro in a switch statement to read a micro chunk into a variable
** Example:
**
**	while (cload.Open_Micro_Chunk()) {
**
**		switch(cload.Cur_Micro_Chunk_ID()) {
**			READ_MICRO_CHUNK(cload,PHYSGRID_VARIABLE_VERSION,version);
**			READ_MICRO_CHUNK(cload,PHYSGRID_VARIABLE_DUMMYVISID,DummyVisId);
**			READ_MICRO_CHUNK(cload,PHYSGRID_VARIABLE_BASEVISID,BaseVisId);
**		}
**		cload.Close_Micro_Chunk();
**	}
*/
#define READ_MICRO_CHUNK(cload,id,var)						\
	case (id):	cload.Read(&var,sizeof(var)); break;	\

/*
** Like READ_MICRO_CHUNK but reads items straight into the data safe.
*/
#define READ_SAFE_MICRO_CHUNK(cload,id,var,type)								\
	case (id):	{                                                     \
		void *temp_read_buffer_on_the_stack = _alloca(sizeof(var));		\
		cload.Read(temp_read_buffer_on_the_stack, sizeof(var));        \
		var = *((type*)temp_read_buffer_on_the_stack);                 \
		break;                                                         \
	}

#define READ_MICRO_CHUNK_STRING(cload,id,var,size)		\
	case (id):	WWASSERT(cload.Cur_Micro_Chunk_Length() <= size); cload.Read(var,cload.Cur_Micro_Chunk_Length()); break;	\

#define READ_MICRO_CHUNK_WWSTRING(cload,id,var)		\
	case (id):	cload.Read(var.Get_Buffer(cload.Cur_Micro_Chunk_Length()),cload.Cur_Micro_Chunk_Length()); break;	\

#define READ_MICRO_CHUNK_WIDESTRING(cload,id,var)		\
	case (id):	cload.Read(var.Get_Buffer((cload.Cur_Micro_Chunk_Length()+1)/2),cload.Cur_Micro_Chunk_Length()); break;	\

/*
** These load macros make it easier to add extra code to a specifc case
*/
#define LOAD_MICRO_CHUNK(cload,var)						\
	cload.Read(&var,sizeof(var)); \

#define LOAD_MICRO_CHUNK_WWSTRING(cload,var)		\
	cload.Read(var.Get_Buffer(cload.Cur_Micro_Chunk_Length()),cload.Cur_Micro_Chunk_Length());	\

#define LOAD_MICRO_CHUNK_WIDESTRING(cload,var)		\
	cload.Read(var.Get_Buffer((cload.Cur_Micro_Chunk_Length()+1)/2),cload.Cur_Micro_Chunk_Length());	\


/*
** OBSOLETE_MICRO_CHUNK - use this macro in a switch statement when you want your code
** to skip a given micro chunk but not fall through to your 'default' case statement which
** prints an "unrecognized chunk" warning message.
*/
#define OBSOLETE_MICRO_CHUNK(id) \
	case (id): break;



#endif // CHUNKIO_H
