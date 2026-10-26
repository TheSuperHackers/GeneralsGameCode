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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------=
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright(C) 2001 - All Rights Reserved
//
//----------------------------------------------------------------------------
//
// Project:    WSYS Library
//
// Module:     IO
//
// File name:  wsys/RAMFile.h
//
// Created:    11/08/01
//
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------
//           Includes
//----------------------------------------------------------------------------

#include "Common/file.h"

//----------------------------------------------------------------------------
//           Forward References
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//           Type Defines
//----------------------------------------------------------------------------

//===============================
// RAMFile
//===============================
/**
 *	File abstraction for standard C file operators: open, close, lseek, read, write
 */
//===============================

class RAMFile : public File
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(RAMFile, "RAMFile")
protected:
	Char* m_data;    ///< File data in memory
	Int m_pos;    ///< current read position
	Int m_size;    ///< size of file in memory

public:
	RAMFile();
	// virtual				~RAMFile();

	virtual Bool open(const Char* filename, Int access = NONE, size_t bufferSize = 0) override;    ///< Open a file for access
	virtual void close() override;    ///< Close the file
	virtual Int read(void* buffer, Int bytes) override;    ///< Read the specified number of bytes in to buffer: See File::read
	virtual Int readChar() override;    ///< Read a character from the file
	virtual Int readWideChar() override;    ///< Read a wide character from the file
	virtual Int write(const void* buffer, Int bytes) override;    ///< Write the specified number of bytes from the buffer: See File::write
	virtual Int writeFormat(const Char* format, ...) override;    ///< Write the formatted string to the file
	virtual Int writeFormat(const WideChar* format, ...) override;    ///< Write the formatted string to the file
	virtual Int writeChar(const Char* character) override;    ///< Write a character to the file
	virtual Int writeChar(const WideChar* character) override;    ///< Write a wide character to the file
	virtual Int seek(Int new_pos, seekMode mode = CURRENT) override;    ///< Set file position: See File::seek
	virtual Bool flush() override;    ///< flush data to disk
	virtual void nextLine(Char* buf = nullptr, Int bufSize = 0) override;    ///< moves current position to after the next new-line

	virtual Bool scanInt(Int& newInt) override;    ///< return what gets read as an integer from the current memory position.
	virtual Bool scanReal(Real& newReal) override;    ///< return what gets read as a float from the current memory position.
	virtual Bool scanString(AsciiString& newString) override;    ///< return what gets read as a string from the current memory position.

	virtual Bool open(File* file);    ///< Open file for fast RAM access
	virtual Bool openFromArchive(File* archiveFile, const AsciiString& filename, Int offset, Int size);    ///< copy file data from the given file at the given offset for the given size.
	virtual Bool copyDataToFile(File* localFile);    ///< write the contents of the RAM file to the given local file.  This could be REALLY slow.

	/**
	  Allocate a buffer large enough to hold entire file, read
	  the entire file into the buffer, then close the file.
	  the buffer is owned by the caller, who is responsible
	  for freeing is (via delete[]). This is a Good Thing to
	  use because it minimizes memory copies for BIG files.
	*/
	virtual char* readEntireAndClose() override;
	virtual File* convertToRAMFile() override;

protected:
	void closeFile();
};

//----------------------------------------------------------------------------
//           Inlining
//----------------------------------------------------------------------------
