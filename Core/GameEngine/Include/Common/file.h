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
// File name:  wsys/File.h
//
// Created:    4/23/01
//
//----------------------------------------------------------------------------

#pragma once

#ifndef __FILE_H
#define __FILE_H



//----------------------------------------------------------------------------
//           Includes                                                      
//----------------------------------------------------------------------------

#include "Lib/BaseType.h"
#include "Common/AsciiString.h"
#include "Common/GameMemory.h"
// include FileSystem.h as it will be used alot with File.h
//#include "Common/FileSystem.h"

//----------------------------------------------------------------------------
//           Forward References
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//           Type Defines
//----------------------------------------------------------------------------

//===============================
// File
//===============================
/**
  *	File is an interface class for basic file operations.
	*
	* All code should use the File class and not its derivatives, unless
	* absolutely necessary. Also FS::Open should be used to create File objects and open files.
	* 
	* TheSuperHackers @feature Adds LINEBUF and FULLBUF modes and buffer size argument for file open.
	*/
//===============================

class File : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_ABC(File)
// friend doesn't play well with MPO (srj)
//	friend class FileSystem;

	public:
	
		enum access
		{
			NONE			= 0x00000000,				///< Access file. Reading by default

			READ			= 0x00000001,				///< Access file for reading
			WRITE			= 0x00000002,				///< Access file for writing
			READWRITE = (READ | WRITE),

			APPEND		= 0x00000004,				///< Seek to end of file on open
			CREATE		= 0x00000008,				///< Create file if it does not exist
			TRUNCATE	= 0x00000010,				///< Delete all data in file when opened

			// NOTE: accesses file as text data if neither TEXT and BINARY are set
			TEXT			= 0x00000020,				///< Access file as text data
			BINARY		= 0x00000040,				///< Access file as binary data

			ONLYNEW		= 0x00000080,				///< Only create file if it does not exist
			
			// NOTE: STREAMING is Mutually exclusive with WRITE
			STREAMING = 0x00000100,				///< Do not read this file into a ram file, read it as requested.

			// NOTE: accesses file with full buffering if neither LINEBUF and FULLBUF are set
			LINEBUF   = 0x00000200,				///< Access file with line buffering
			FULLBUF   = 0x00000400,				///< Access file with full buffering
		};

		enum seekMode
		{
			START,												///< Seek position is relative to start of file
			CURRENT,											///< Seek position is relative to current file position
			END														///< Seek position is relative from the end of the file
		};

		enum
		{
			BUFFERSIZE = BUFSIZ,
		};

	protected:

		AsciiString	m_nameStr;						///< Stores file name
		Int					m_access;									///< How the file was opened
		Bool				m_open;										///< Has the file been opened
		Bool				m_deleteOnClose;					///< delete File object on close()
		
		
		File();											///< This class can only used as a base class
		//virtual				~File();

		void closeWithoutDelete();

	public:
		

						Bool	eof();
		virtual Bool	open( const Char *filename, Int access = NONE, size_t bufferSize = BUFFERSIZE ); ///< Open a file for access
		virtual void	close( void );																			///< Close the file !!! File object no longer valid after this call !!!

		virtual Int		read( void *buffer, Int bytes ) = 0 ;						/**< Read the specified number of bytes from the file in to the 
																																			  *  memory pointed at by buffer. Returns the number of bytes read.
																																			  *  Returns -1 if an error occurred.
																																			  */
		virtual Int		write( const void *buffer, Int bytes ) = 0 ;						/**< Write the specified number of bytes from the    
																																			  *	 memory pointed at by buffer to the file. Returns the number of bytes written.
																																			  *	 Returns -1 if an error occurred.
																																			  */
		virtual Int		seek( Int bytes, seekMode mode = CURRENT ) = 0;	/**< Sets the file position of the next read/write operation. Returns the new file
																																				*  position as the number of bytes from the start of the file.
																																				*  Returns -1 if an error occurred.
																																				*
																																				*  seekMode determines how the seek is done:
																																				*
																																				*  START : means seek to the specified number of bytes from the start of the file
																																				*  CURRENT: means seek the specified the number of bytes from the current file position
																																				*  END: means seek the specified number of bytes back from the end of the file
																																				*/
		virtual Bool	flush() = 0;											///< flush data to disk
		virtual void	nextLine(Char *buf = NULL, Int bufSize = 0) = 0;		///< reads until it reaches a new-line character

		virtual Bool	scanInt(Int &newInt) = 0;														///< read an integer from the current file position.
		virtual Bool	scanReal(Real &newReal) = 0;												///< read a real number from the current file position.
		virtual Bool	scanString(AsciiString &newString) = 0;							///< read a string from the current file position.

		virtual Bool	print ( const Char *format, ...);										///< Prints formated string to text file
		virtual Int		size( void );																				///< Returns the size of the file
		virtual Int		position( void );																		///< Returns the current read/write position


		void					setName( const char *name );												///< Set the name of the file
		const char*		getName( void ) const;															///< Returns a pointer to the name of the file
		Int						getAccess( void ) const;														///< Returns file's access flags

		void					deleteOnClose ( void );															///< Causes the File object to delete itself when it closes

		/**
			Allocate a buffer large enough to hold entire file, read 
			the entire file into the buffer, then close the file.
			the buffer is owned by the caller, who is responsible
			for freeing is (via delete[]). This is a Good Thing to
			use because it minimizes memory copies for BIG files.
		*/
		virtual char* readEntireAndClose() = 0;
		virtual File* convertToRAMFile() = 0;
};




//----------------------------------------------------------------------------
//           Inlining                                                       
//----------------------------------------------------------------------------

inline const char* File::getName( void ) const { return m_nameStr.str(); }
inline void File::setName( const char *name ) { m_nameStr.set(name); }
inline Int File::getAccess( void ) const { return m_access; }
inline void File::deleteOnClose( void ) { m_deleteOnClose = TRUE; }





#endif // __FILE_H
