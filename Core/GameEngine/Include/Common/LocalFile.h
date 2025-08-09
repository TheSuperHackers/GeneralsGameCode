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
// File name:  LocalFile.h
//
// Created:    4/23/01
//
//----------------------------------------------------------------------------

#pragma once

#ifndef __LOCALFILE_H
#define __LOCALFILE_H



//----------------------------------------------------------------------------
//           Includes
//----------------------------------------------------------------------------

#include "Common/file.h"

#if USE_BUFFERED_IO
#include "Utility/stdio_adapter.h"
#endif

//----------------------------------------------------------------------------
//           Forward References
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//           Type Defines
//----------------------------------------------------------------------------

//===============================
// LocalFile
//===============================
/**
  *	File abstraction for standard C file operators: open, close, lseek, read, write
	*/
//===============================

class LocalFile : public File
{
	MEMORY_POOL_GLUE_ABC(LocalFile)
	private:

#if USE_BUFFERED_IO
		// srj sez: this was purely an experiment in optimization.
		// at the present time, it doesn't appear to be a good one.
		// TheSuperHackers @info It is a good optimization and will be
		// significantly faster than unbuffered IO with small reads and writes.
		FILE* m_file;
#else
		int m_handle;											///< Local C file handle
#endif

	public:

		LocalFile();
		//virtual				~LocalFile();


		virtual Bool	open( const Char *filename, Int access = NONE, size_t bufferSize = BUFFERSIZE ); ///< Open a file for access
		virtual void	close( void );																			///< Close the file
		virtual Int		read( void *buffer, Int bytes );										///< Read the specified number of bytes in to buffer: See File::read
		virtual Int		readChar();																///< Read a character from the file
		virtual Int		readWideChar();															///< Read a wide character from the file
		virtual Int		write( const void *buffer, Int bytes );							///< Write the specified number of bytes from the buffer: See File::write
		virtual Int		writeFormat( const Char* format, ... );							///< Write an unterminated formatted string to the file
		virtual Int		writeFormat( const WideChar* format, ... );						///< Write an unterminated formatted string to the file
		virtual Int		writeChar( const Char* character );								///< Write a character to the file
		virtual Int		writeChar( const WideChar* character );							///< Write a wide character to the file
		virtual Int		seek( Int new_pos, seekMode mode = CURRENT );				///< Set file position: See File::seek
		virtual Bool	flush();													///< flush data to disk
		virtual void	nextLine(Char *buf = NULL, Int bufSize = 0);				///< moves file position to after the next new-line
		virtual Bool	scanInt(Int &newInt);																///< return what gets read in as an integer at the current file position.
		virtual Bool	scanReal(Real &newReal);														///< return what gets read in as a float at the current file position.
		virtual	Bool	scanString(AsciiString &newString);									///< return what gets read in as a string at the current file position.
		/**
			Allocate a buffer large enough to hold entire file, read
			the entire file into the buffer, then close the file.
			the buffer is owned by the caller, who is responsible
			for freeing is (via delete[]). This is a Good Thing to
			use because it minimizes memory copies for BIG files.
		*/
		virtual char* readEntireAndClose();
		virtual File* convertToRAMFile();

	protected:

		void closeWithoutDelete();
		void closeFile();
};




//----------------------------------------------------------------------------
//           Inlining
//----------------------------------------------------------------------------


#endif // __LOCALFILE_H
