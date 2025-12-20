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
// File name:  wsys/File.h
//
// Created:    4/23/01
//
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------
//           Includes
//----------------------------------------------------------------------------

#include "Lib/BaseType.h"

//----------------------------------------------------------------------------
//           Forward References
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//           Type Defines
//----------------------------------------------------------------------------

#define IO_MAX_PATH			(2*1024)				///< Maximum allowable path legnth

//===============================
// File
//===============================
/**
  *	File is an interface class for basic file operations.
	*
	* All code should use the File class and not its derivatives, unless
	* absolutely necessary. Also FS::Open should be used to create File objects and open files.
	*/
//===============================

class File
{
	friend class FileSystem;

	public:

		enum access
		{
			NONE			= 0x00000000,
			READ			= 0x00000001,				///< Access file for reading
			WRITE			= 0x00000002,				///< Access file for writing
			APPEND		= 0x00000004,				///< Seek to end of file on open
			CREATE		= 0x00000008,				///< Create file if it does not exist
			TRUNCATE	= 0x00000010,				///< Delete all data in file when opened
			TEXT			= 0x00000020,				///< Access file as text data
			BINARY		= 0x00000040,				///< Access file as binary data
			READWRITE = (READ | WRITE),
			NEW				= 0x00000080				///< Only create file if it does not exist
		};

		enum seekMode
		{
			START,												///< Seek position is relative to start of file
			CURRENT,											///< Seek position is relative to current file position
			END														///< Seek position is relative from the end of the file
		};

	protected:

		Char		m_name[IO_MAX_PATH+1];		///< Stores file name
		Bool		m_open;										///< Has the file been opened
		Bool		m_deleteOnClose;					///< delete File object on close()
		Int			m_access;									///< How the file was opened


		File();											///< This class can only used as a base class
		virtual				~File();

	public:



		virtual Bool	open( const Char *filename, Int access = 0 );				///< Open a file for access
		virtual void	close( void );																			///< Close the file !!! File object no longer valid after this call !!!

		virtual Int		read( void *buffer, Int bytes ) = NULL ;						/**< Read the specified number of bytes from the file in to the
																																			  *  memory pointed at by buffer. Returns the number of bytes read.
																																			  *  Returns -1 if an error occured.
																																			  */
		virtual Int		write( void *buffer, Int bytes ) = NULL ;						/**< Write the specified number of bytes from the
																																			  *	 memory pointed at by buffer to the file. Returns the number of bytes written.
																																			  *	 Returns -1 if an error occured.
																																			  */
		virtual Int		seek( Int bytes, seekMode mode = CURRENT ) = NULL;	/**< Sets the file position of the next read/write operation. Returns the new file
																																				*  position as the number of bytes from the start of the file.
																																				*  Returns -1 if an error occured.
																																				*
																																				*  seekMode determines how the seek is done:
																																				*
																																				*  START : means seek to the specified number of bytes from the start of the file
																																				*  CURRENT: means seek the specified the number of bytes from the current file position
																																				*  END: means seek the specified number of bytes back from the end of the file
																																				*/
		virtual Bool	printf ( const Char *format, ...);									///< Prints formated string to text file
		virtual Int		size( void );																				///< Returns the size of the file
		virtual Int		position( void );																		///< Returns the current read/write position


		void					setName( const Char *name );												///< Set the name of the file
		Char*					getName( void );																		///< Returns a pointer to the name of the file
		Bool					getName( Char *buffer, Int max );										///< Copies the name of the file to the buffer
		Int						getAccess( void );																	///< Returns file's access flags

		void					deleteOnClose ( void );															///< Causes the File object to delete itself when it closes
};




//----------------------------------------------------------------------------
//           Inlining
//----------------------------------------------------------------------------

inline Char* File::getName( void ) { return m_name;};
inline Int File::getAccess( void ) { return m_access;};
inline void File::deleteOnClose( void ) { m_deleteOnClose = TRUE;};



// include FileSystem.h as it will be used alot with File.h
//#include "wsys/FileSystem.h"
