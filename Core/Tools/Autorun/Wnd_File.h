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

//****************************************************************************
//       C O N F I D E N T I A L -- W E S T W O O D   S T U D I O S
//****************************************************************************
//
//	Project name:		Blade Runner CD-ROM Windows 95
//
// File name:			WND_FILE.h
//
// Source code: 		WND_FILE.cpp
//
// Compatibility:		Microsoft Visual C++ 4.0
//							Borland C++ 5.0
//							Watcom C++ 10.6
//
//	Start Date:			See comments in version control log
//	Last Update:		See comments in version control log
//
// Programmer(s):		Michael Legg
//							Mike Grayford
//							James McNeill
//
//****************************************************************************

#pragma once

//------------------------------------------------------------------------------
// include files...
//------------------------------------------------------------------------------

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdlib.h>
#include <assert.h>

#define BOOL						int
#define ASSERT(x)  		assert(x)
#define VERIFY(X)		assert(X)


//------------------------------------------------------------------------------
// defines...
//------------------------------------------------------------------------------

//
// it's one or the other!
//
#define SUPPORT_STREAMS				TRUE		// Normally this!
#define SUPPORT_HANDLES				FALSE		// This is a test!

#define MODE_READ_ONLY				0
#define MODE_WRITE_ONLY				1
#define MODE_READ_AND_WRITE			2

#define MODE_WRITE_TRUNCATE			MODE_WRITE_ONLY
#define MODE_WRITE_APPEND			3
#define MODE_WRITE_UPDATE 			4

#define SEEK_SET				 	0
#define SEEK_CUR				 	1
#define SEEK_END				 	2

// #define INVALID_FILE_HANDLE		-1
#define INVALID_FILE_HANDLE			INVALID_HANDLE_VALUE
#define MAX_PATH_SIZE				_MAX_PATH


#define STRING_IT(a) #a
#define TOKEN_IT(a) STRING_IT(,##a)
#define MESSAGE(a) message (__FILE__ "(" TOKEN_IT(__LINE__) ") : " a)

//#pragma MESSAGE("What does it do?")



#ifdef RTS_DEBUG
void __cdecl Msg( int line, const char *file, const char *fmt, ... );
void __cdecl Msg( int line, const char *filename, const wchar_t *fmt, unsigned int codepage=1252, ... );
void 	Delete_Msg_File( void );
#else
#define Msg
#define Delete_Msg_File()
#endif


//------------------------------------------------------------------------------
// file class definition
//------------------------------------------------------------------------------

class StandardFileClass
{
	public:

		//
		// public class functions...
		//
		StandardFileClass();
		~StandardFileClass();
		bool 	Open 				( const char *file_name, int open_mode );
		bool 	Close				( void );
		int 	Read 				( void *buffer, unsigned long int bytes_to_read );
		int 	Write				( void *buffer, unsigned long int bytes_to_write );
		bool 	Seek 				( int distance, int seek_file_position );
		int 	Tell 				( void );
		int 	Query_Size			( void );
		bool 	Query_Open			( void );
		char *	Query_Name_String	( void );
     	int		End_Of_File			( void );
		int		Flush	  	 		( void );

		#if( SUPPORT_STREAMS )
		FILE *Query_File_Stream_Pointer( void );
		#endif

	private:

		//
		// private class functions...
		//
		void Reset( void );
		//
		// private class data...
		//
		#if( SUPPORT_HANDLES )
			HANDLE File_Handle;
		#endif

		#if( SUPPORT_STREAMS )
			//--------------------------------------------------------------------
			// The _stat structure, defined in SYS\STAT.h, includes these fields.
			//		st_atime  	Time of last access of file ( time_t ).
			//		st_ctime  	Time of creation of file ( time_t ).
			//		st_dev		Drive number of the disk containing the file (same as st_rdev).
			//		st_rdev		Drive number of the disk containing the file (same as st_dev).
			//		st_mode		Bit mask for file-mode information.
			//				  		_S_IFDIR bit is set if path specifies a directory;
			//				  		_S_IFREG bit is set if path specifies an ordinary file or a device.
			//				  	User read/write bits are set according to the file's permission
			//				  	mode; user execute bits are set according to the filename extension.
			//		st_mtime  	Time of last modification of file.
			//		st_nlink  	Always 1 on non-NTFS file systems.
			//		st_size		Size of the file in bytes; a 64-bit integer for _stati64 and _wstati64
			//--------------------------------------------------------------------
			FILE *File_Stream_Ptr;
			struct stat File_Statistics;
		#endif

		char	File_Name[ MAX_PATH_SIZE ];
		bool	Currently_Open;
};

//------------------------------------------------------------------------------
// non-class public functions...
//------------------------------------------------------------------------------

#if( SUPPORT_HANDLES )
	HANDLE 	Open_File( char const *file_name, int mode );
	bool   	Close_File( HANDLE handle );
	int    	Read_File( HANDLE handle,
		   			void *buffer,
		   			unsigned long int bytes_to_read );
	int    	Write_File( HANDLE handle,
		   		 	 void const *buffer,
		   		 	 unsigned long int bytes_to_write );
	bool   	Seek_File( HANDLE handle,
		   		 	 int distance,
		   		 	 int seek_file_location );
	int    	Tell_File( HANDLE handle );
	int    	File_Size( HANDLE handle );
	//
	// include path in name
	//
	bool Full_Path_File_Exists( char const *file_name );
	//
	// don't include path in name
	//
	bool HD_File_Exists( char const *file_name );
	bool CD_File_Exists( char const *file_name );
	// bool Find_File( char const *file_name );
#endif

#if( SUPPORT_STREAMS )
	//
	// include path in name
	//
	bool Full_Path_File_Exists( char const *file_name );
	//
	// don't include path in name
	//
	bool HD_File_Exists( char const *file_name );
	bool CD_File_Exists( char const *file_name );
	// bool Find_File( char const *file_name );
#endif
