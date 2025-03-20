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

//*****************************************************************************
//       C O N F I D E N T I A L -- W E S T W O O D   S T U D I O S       
//*****************************************************************************
//
//	Project name:		Blade Runner CD-ROM Windows 95
//
// File name:			WND_FILE.CPP
//
// Functions:			WND_FILE.H
//
// Compatibility:		Microsoft Visual C++ 4.0
//					 	Borland C++ 5.0
//					 	Watcom C++ 10.6
//
//	Start Date:			See comments in version control log
//	Last Update:		See comments in version control log
//
// Programmer(s):		Michael Legg
//					 	Mike Grayford
//
//*****************************************************************************

//-----------------------------------------------------------------------------
// include files...
//-----------------------------------------------------------------------------
#include "windows.h"
#include <stdio.h>
#include <sys/stat.h>
#include "Wnd_File.h"
#include "WinFix.H"
//#include "autorun.h"


//-----------------------------------------------------------------------------
// private defines...
//-----------------------------------------------------------------------------
#define DEBUG_FILE 		"DebugAutorun.txt"

// TC? This is non-Westwood Library 32-bit file access!
#define MAX_FILES_OPEN_AT_A_TIME		25   	// includes .MIX files

//-----------------------------------------------------------------------------
// private data...
//-----------------------------------------------------------------------------
char HD_Path	[ MAX_PATH ] = { '\0' };
char CD_Path	[ MAX_PATH ] = { '\0' };
char DebugFile	[ MAX_PATH ] = { '\0' };

// HANDLE Windows_File_Handles[ MAX_FILES_OPEN_AT_A_TIME ];
// #if( SUPPORT_STREAMS )
//	 FILE *Windows_File_Streams[ MAX_FILES_OPEN_AT_A_TIME ];
// #endif


//-----------------------------------------------------------------------------
// non-class private functions in this module...
//-----------------------------------------------------------------------------
// int Get_Internal_File_Handle( void );
// #if( SUPPORT_STREAMS )
//	 FILE *Get_Internal_File_Stream( void );
// #endif


//-----------------------------------------------------------------------------
// public file class functions...
//-----------------------------------------------------------------------------
#if(0)
#ifndef _DEBUG
void __cdecl Msg( int, char *, char *, ... ) { };	// line, file, fmt
void 	Delete_Msg_File ( void )  { };
#endif
#endif


#ifdef _DEBUG

/****************************************************************************
 * MSG -- Write Message to Debug file with line and filename.				*
 *																			*
 * INPUT:		int line       --	line where message originated.			*
 *				char *filename -- file where message originated.			*
 *				char *fmt      -- variable argument list.					*
 *																			*
 * OUTPUT:		none.														*
 *																			*
 * WARNINGS:	none.														*
 *																			*
 * HISTORY:																	*
 *   08/19/1998   MML : Created.											*
 *==========================================================================*/

void __cdecl Msg( int line, char *filename, char *fmt, ... )
{
	char     szBuffer1[ MAX_PATH * 3 ];
	char     szBuffer2[ MAX_PATH * 2 ];
	char     szFile[ MAX_PATH ];
	va_list  va;
	DWORD    nBytes;
	StandardFileClass file;
   
	//----------------------------------------------------------------------
	// Variable Arguments
	//----------------------------------------------------------------------
	va_start( va, fmt );

	if ( DebugFile[0] == '\0' ) {
		return;
	}
	if ( filename[0] == '\0' ) {
		return;
	}

	//----------------------------------------------------------------------
	// Make filename.
	//----------------------------------------------------------------------
	char *temp = strrchr( filename, '\\' );
	if ( temp != NULL || temp[0] != '\0' ) {
		temp++;
		strcpy( szFile, temp );
	}

	//----------------------------------------------------------------------
	// format message with header
	//----------------------------------------------------------------------
	memset( szBuffer1, '\0', MAX_PATH * 3 );
	memset( szBuffer2, '\0', MAX_PATH * 2 );
	wvsprintf( szBuffer2, fmt, va );
	wsprintf( szBuffer1, "%4d %14s %s\n",   line, szFile, szBuffer2 );
   
	//----------------------------------------------------------------------
	// Open debug file and write to it.
	//----------------------------------------------------------------------
	file.Open( DebugFile, MODE_WRITE_APPEND );
	if ( file.Query_Open( )) {

		int length = strlen( szBuffer1 );
   		nBytes = file.Write( szBuffer1, length );
		if ( nBytes != strlen( szBuffer1 )) {
		}
		file.Close();
	}

	//----------------------------------------------------------------------
	// To the debugger unless we need to be quiet
	//----------------------------------------------------------------------
	OutputDebugString( szBuffer1 );
   
} /* Msg */


/****************************************************************************
 * MSG -- Write Message to Debug file with line and filename.				*
 *																			*
 * INPUT:		int		line		--	line where message originated.	   	*
 *				wchar_t *filename	-- file where message originated.	   	*
 *				wchar_t *fmt		-- variable argument list. 				*
 *																			*
 * OUTPUT:		none.														*
 *																			*
 * WARNINGS:	none.														*
 *																			*
 * HISTORY:																	*
 *   08/19/1998   MML : Created.											*
 *==========================================================================*/

void __cdecl Msg( int line, char *filename, wchar_t *fmt, UINT codepage, ... )
{
	wchar_t		szBuffer1[ MAX_PATH * 3 ];
	wchar_t		szBuffer2[ MAX_PATH * 3 ];
	char		szBuffer3[ MAX_PATH * 3 ];
	wchar_t		szFile[ MAX_PATH ];
	wchar_t		szArgs[ MAX_PATH ];
	va_list		va;
	int			length;
	DWORD		nBytes;
	StandardFileClass file;
   
	//----------------------------------------------------------------------
	// Variable Arguments
	//----------------------------------------------------------------------
//	va_start( va, fmt );
	va_start( va, codepage );
	memset( szArgs,		'\0', MAX_PATH );
	memset( szFile,		'\0', MAX_PATH );
	memset( szBuffer1,	'\0', MAX_PATH * 3 );
	memset( szBuffer2,	'\0', MAX_PATH * 2 );

	if ( DebugFile == NULL ) {
		return;
	}
	if ( filename == NULL ) {
		return;
	}

	//----------------------------------------------------------------------
	// Make filename.
	//----------------------------------------------------------------------
	char *temp = strrchr( filename, '\\' );
	if ( temp != NULL || temp[0] != '\0' ) {
		temp++;
		length = strlen( temp );
		mbstowcs( szFile, temp, length );
	}

	//----------------------------------------------------------------------
	// format message with header
	//----------------------------------------------------------------------
	vswprintf( szBuffer2, fmt, va );
	swprintf( szBuffer1, L"%4d %14s %s\n", line, szFile, szBuffer2 );
   
	//----------------------------------------------------------------------
	// Open debug file and write to it.
	//----------------------------------------------------------------------
	file.Open( DebugFile, MODE_WRITE_APPEND );

	if ( file.Query_Open( )) {

		//---------------------------------------------------------------------
		//	Identifier		Meaning 
		//	932				Japan 
		//	949				Korean 
		//	950				Chinese (Taiwan; Hong Kong SAR, PRC)  
		//	1252			Windows 3.1 Latin 1 (US, Western Europe) 
		//---------------------------------------------------------------------
		WideCharToMultiByte( codepage, 0, szBuffer1, -1, szBuffer3, MAX_PATH*3, NULL, NULL );

		length = strlen( szBuffer3 );
   		nBytes = file.Write( szBuffer3, length );
		if ( nBytes != strlen( szBuffer3 )) {
		}
		file.Close();
	}

	//----------------------------------------------------------------------
	// To the debugger unless we need to be quiet
	//----------------------------------------------------------------------
	OutputDebugString( szBuffer3 );
   
} /* Msg */


/***************************************************************************
 * DELETE_MSG_FILE -- Delete the Debug file.							   *
 *                                                                         *
 * INPUT:         none.													   *
 *                                                                         *
 * OUTPUT:        none.													   *
 *                                                                         *
 * WARNINGS:      none.                                                    *
 *                                                                         *
 * HISTORY:                                                                *
 *   08/19/1998   MML : Created.                                           *
 *=========================================================================*/

//----------------------------------------------------------------------
// Delete_Msg_File
//----------------------------------------------------------------------

void Delete_Msg_File ( void )
{
	DWORD	nBytes;
	char	buff	[ 300 ];
	char	date	[ 50 ];
	char	time	[ 30 ];
	StandardFileClass file;

	//----------------------------------------------------------------------
	// Make path to debug file.
	//----------------------------------------------------------------------
//	strcat( strcpy( DebugFile, ".\\" ), DEBUG_FILE );
	GetWindowsDirectory( DebugFile, MAX_PATH );
	if ( DebugFile[ strlen( DebugFile )-1 ] != '\\' ) {
		strcat( DebugFile, "\\" );
	}		
	strcat( DebugFile, DEBUG_FILE );

	//--------------------------------------------------------------------------
	// Delete previous file.
	//--------------------------------------------------------------------------
	DeleteFile( DebugFile );
	
	//--------------------------------------------------------------------------
	// Create/Open new file.
	//--------------------------------------------------------------------------
	file.Open( DebugFile, MODE_WRITE_TRUNCATE );
	if ( file.Query_Open( )) {

		wsprintf( buff, "===========================================================\r\n" );
		nBytes = file.Write( buff, strlen( buff ));

		GetDateFormat( LOCALE_USER_DEFAULT, DATE_SHORTDATE, NULL, NULL, date, 50 );
//		GetTimeFormat( LOCALE_USER_DEFAULT, TIME_NOSECONDS, NULL, NULL, time, 30 );
		GetTimeFormat( LOCALE_USER_DEFAULT, NULL, NULL, "hh':'mm':'ss tt", time, 30 );
		wsprintf( buff, "SETUP:  File: %s  Date: %s  Time: %s.\r\n", DebugFile, date, time );
		nBytes = file.Write( buff, strlen( buff ));

		wsprintf( buff, "===========================================================\r\n\r\n" );
		nBytes = file.Write( buff, strlen( buff ));

		file.Close();
	}
}

#endif


//------------------------------------------------------------------------------
// StandardFileClass::StandardFileClass
//------------------------------------------------------------------------------

StandardFileClass::StandardFileClass( void )
{
	//
	// reset all internal data
	//
	Reset();
}

//------------------------------------------------------------------------------
// StandardFileClass::~StandardFileClass
//------------------------------------------------------------------------------

StandardFileClass::~StandardFileClass( void )
{
	//
	// make sure this file got shut down before we destruct
	//
	#if( SUPPORT_HANDLES )
		ASSERT( File_Handle == INVALID_FILE_HANDLE );
	#endif
	#if( SUPPORT_STREAMS )
   		ASSERT( File_Stream_Ptr == NULL );
	#endif
	ASSERT( Currently_Open == FALSE );

	//
	// reset all internal data
	//
	Reset();
}

//------------------------------------------------------------------------------
// bool StandardFileClass::Open
//------------------------------------------------------------------------------

bool StandardFileClass::Open( const char *no_path_file_name, int open_mode )
{
	int test;
	char *attributes;
	char pathed_file_name[ MAX_PATH_SIZE ];

	//
	// debug checks...
	//
	ASSERT( no_path_file_name != NULL );
	ASSERT( Currently_Open == FALSE );
	ASSERT( strlen( no_path_file_name ) < MAX_PATH );
	ASSERT(	open_mode == MODE_READ_ONLY ||
			open_mode == MODE_WRITE_ONLY ||
			open_mode == MODE_READ_AND_WRITE || 
			open_mode == MODE_WRITE_TRUNCATE ||
			open_mode == MODE_WRITE_APPEND );

	//
	// open the file
	//
	#if( SUPPORT_HANDLES )

		ASSERT( File_Handle == INVALID_FILE_HANDLE );

		//
		// try HD open
		//
		strcpy( pathed_file_name, HD_Path );
		strcat( pathed_file_name, no_path_file_name );
		File_Handle = Open_File( pathed_file_name, open_mode );
                            
		//
		// if not success with HD open, try CD
		//
		if ( File_Handle == INVALID_FILE_HANDLE ) {

			//
			// try CD open
			//
			strcpy( pathed_file_name, CD_Path );
			strcat( pathed_file_name, no_path_file_name );
	   		File_Handle = Open_File( pathed_file_name, open_mode );
		}

   	//
   	// not successful HD or CD open?
   	//
   	if ( File_Handle == INVALID_FILE_HANDLE ) {
		ASSERT( FALSE );
   		return( FALSE );
   	}

	#endif

	#if( SUPPORT_STREAMS )

		ASSERT( File_Stream_Ptr == NULL );
      
		//
		// "r"  - open existing file for reading.
		// "w"  - create new file, or truncate existing one, for output.
		// "a"  - create new file, or append to if existing, for output.
		// "r+" - open existing file for read and write, starting at beginning of file. File must exist.
		// "w+" - create new file, or truncate if existing, for read and write.
		// "a+" - create new file, or append to existing, for read and write.
		//
		// add "b" to any string for binary instead of text
		//
		if ( open_mode == MODE_READ_ONLY ) {
			//
			// open existing file for input (binary)
			//
			attributes = "rb";
		}
		else if ( open_mode == MODE_WRITE_ONLY || open_mode == MODE_WRITE_TRUNCATE ) {
			//
			// create new or open/truncate existing file for output (binary)
			//
			attributes = "wb";
		}
		else if ( open_mode == MODE_READ_AND_WRITE ) {
			//
			// open existing for for read and write, starting at beginning of file
			//
			attributes = "r+b";
		}
		else if ( open_mode == MODE_WRITE_UPDATE ) {
			//
			// Create a new file for update (reading and writing). If a file by 
			// that name already exists, it will be overwritten.
			//
			attributes = "w+b";
		}
		else if ( open_mode == MODE_WRITE_APPEND ) {
			//
			// append to existing file for output (binary)
			//
			attributes = "a";
		}
		else {
			ASSERT( FALSE );
		}

		//
		// try HD open
		//
		strcpy( pathed_file_name, HD_Path );
		strcat( pathed_file_name, no_path_file_name );
		File_Stream_Ptr = fopen( pathed_file_name, attributes );

		//
		// if not success with HD open, try CD
		//
		if ( File_Stream_Ptr == NULL ) {

			//
			// try CD open
			//
			strcpy( pathed_file_name, CD_Path );
			strcat( pathed_file_name, no_path_file_name );
	   		File_Stream_Ptr = fopen( pathed_file_name, attributes );
		}
                            
	//
   	// not successful?
   	//
   	if ( File_Stream_Ptr == NULL ) {
   		return( FALSE );
   	}

	//
	// get file stats
	//
	test = stat( pathed_file_name, &File_Statistics );
	ASSERT( test == 0 );

   #endif
   
	//
	// successful, set name and open status
	//
	strncpy( File_Name, pathed_file_name, MAX_PATH_SIZE-1 );
	Currently_Open = TRUE;

	//
	// success!
	//
	return( TRUE );
}

//------------------------------------------------------------------------------
// bool StandardFileClass::Close
//------------------------------------------------------------------------------

bool StandardFileClass::Close( void )
{
	int status;
	
	#if( SUPPORT_HANDLES )
	   bool success;
	#endif

	//
	// debug checks...
	//
	ASSERT( Currently_Open == TRUE );

	#if( SUPPORT_HANDLES )

	ASSERT( File_Handle > INVALID_FILE_HANDLE );
      
	//
   	// error?
   	//
   	if ( File_Handle == INVALID_FILE_HANDLE || Currently_Open == FALSE ) {
    	//
     	// no success
     	//
     	ASSERT( FALSE );
    	return( FALSE );
	}

   	//
   	// close file
   	//
   	// status = Close_File( File_Handle );
   	success = Close_File( File_Handle );
   	ASSERT( success == TRUE );

	//
   	// reset file data
   	//   		
	File_Handle = INVALID_FILE_HANDLE;
	Currently_Open = FALSE;

   	//
   	// error on close?
   	//
   	// if ( status == INVALID_FILE_HANDLE ) {
   	// 		return( FALSE );
   	// }
	return( success );
      
	#endif
   
	#if( SUPPORT_STREAMS )

	ASSERT( File_Stream_Ptr != NULL );
      
   	//
   	// error?
   	//
   	if ( File_Stream_Ptr == NULL || Currently_Open == FALSE ) {
       		//
         	// no success
         	//
         	ASSERT( FALSE );
       		return( FALSE );
	}

   	//
   	// close file
   	//
   	status = fclose( File_Stream_Ptr );
   	ASSERT( status == 0 );

	//
   	// reset file data
   	//   		
	File_Stream_Ptr = NULL;
	Currently_Open = FALSE;

   	//
   	// error on close?
   	//
   	if ( status != 0 ) {
   		return( FALSE );
   	}
	#endif

	//
	// success!
	//
	return( TRUE );
}

//------------------------------------------------------------------------------
// int StandardFileClass::Read
//------------------------------------------------------------------------------
int StandardFileClass::Read( void *buffer, unsigned long int bytes_to_read )
{
	int bytes_read;
	int items_read;

	//
	// debug checks ( Fails if condition is FALSE ).
	//
	ASSERT( buffer != NULL );
	ASSERT( bytes_to_read > 0 );
	ASSERT( Currently_Open == TRUE );

#if( SUPPORT_HANDLES )
   
	ASSERT( File_Handle != INVALID_FILE_HANDLE );
	//
	// error?
	//
	if ( File_Handle == INVALID_FILE_HANDLE || Currently_Open == FALSE ) {
		//
		// nothing read
		//
		return( 0 );
	}

	//
	// read in the bytes
	//
	bytes_read = Read_File( File_Handle, buffer, bytes_to_read );
#endif

#if( SUPPORT_STREAMS )
   
	ASSERT( File_Stream_Ptr != NULL );
	//
	// error?
	//
	if ( File_Stream_Ptr == NULL || Currently_Open == FALSE ) {
   		//
		// nothing read
		//
   		return( 0 );
	}

	//
	// read in the bytes
	//
	items_read = fread( buffer, bytes_to_read, 1, File_Stream_Ptr );
//	Msg( __LINE__, __FILE__, "Read --- bytes_to_read = %d, items_read = %d.", bytes_to_read, items_read );

	//
	// &&& we should leave this enabled!
	//
	// The TRR system causes an error when we load strings into RAM
	// IF the strings are kept on disk, then no error occurs!
	//
	ASSERT( items_read == 1 );

	bytes_read = items_read * bytes_to_read;                          
#endif

	//
	// return how many bytes we read
	//   
	return( bytes_read );
}

//------------------------------------------------------------------------------
// int StandardFileClass::Write
//------------------------------------------------------------------------------

int StandardFileClass::Write( void *buffer, unsigned long int bytes_to_write )
{
	int items_written;
	int bytes_written;

	//
	// debug checks
	//
	ASSERT( buffer != NULL );
	ASSERT( bytes_to_write > 0 );
	ASSERT( Currently_Open == TRUE );

	if ( buffer == NULL ) {
		return( 0 );
	}
	if ( bytes_to_write < 1 ) {
		return( 0 );
	}

	#if( SUPPORT_HANDLES )
   
	ASSERT( File_Handle != INVALID_FILE_HANDLE );

   	//
   	// error?
   	//
   	if ( File_Handle == INVALID_FILE_HANDLE || Currently_Open == FALSE ) {
       		//
         	// nothing written
         	//
       		return( 0 );
	}

   	//
   	// write out the bytes
   	//
   	bytes_written = Write_File( File_Handle, buffer, bytes_to_write );
	ASSERT( bytes_written == bytes_to_write );
	#endif

	#if( SUPPORT_STREAMS )
   
	ASSERT( File_Stream_Ptr != NULL );
   	//
   	// error?
   	//
   	if ( File_Stream_Ptr == NULL || Currently_Open == FALSE ) {
       		//
         	// nothing written
         	//
      		return( 0 );
	}

   	//
   	// write out the bytes
   	//
   	items_written = fwrite( buffer, bytes_to_write, 1, File_Stream_Ptr );
	ASSERT( items_written == 1 );
    bytes_written = items_written * bytes_to_write;
	#endif

	//
	// return how many bytes we wrote
	//   
	return( bytes_written );
}

//------------------------------------------------------------------------------
// bool StandardFileClass::Seek
//------------------------------------------------------------------------------

bool StandardFileClass::Seek( int distance, int seek_file_position )
{
	//
	// debug checks...
	//
	ASSERT( Currently_Open == TRUE );
	ASSERT( seek_file_position == SEEK_SET ||
   			seek_file_position == SEEK_CUR ||
			seek_file_position == SEEK_END );

	#if( SUPPORT_HANDLES )

	bool success;

	ASSERT( File_Handle != INVALID_FILE_HANDLE );
   	//
   	// error?
   	//
   	if ( File_Handle == INVALID_FILE_HANDLE || Currently_Open == FALSE ) {
       		//
         	// error
         	//
       		return( FALSE );
	}

	//
   	// do the seek!
   	//   
   	success = Seek_File( File_Handle, distance, seek_file_position );
	ASSERT( success == TRUE );
	return( success );                        
	#endif

	#if( SUPPORT_STREAMS )

	ASSERT( File_Stream_Ptr != NULL );

   	//
   	// error?
   	//
   	if ( File_Stream_Ptr == NULL || Currently_Open == FALSE ) {
       		//
         	// error
         	//
      		return( FALSE );
	}

	//
   	// do the seek!
   	//   
   	int result = fseek( File_Stream_Ptr, distance, seek_file_position );

	ASSERT( result == 0 );
	if ( ! result ) {
      	return( TRUE );
	}
	return( FALSE );
	#endif
}

//------------------------------------------------------------------------------
// int StandardFileClass::Tell
//------------------------------------------------------------------------------
//
// return file position
//
int StandardFileClass::Tell( void )
{
	int file_pos;

	//
	// debug checks...
	//
	ASSERT( Currently_Open == TRUE );

	#if( SUPPORT_HANDLES )

	ASSERT( File_Handle != INVALID_FILE_HANDLE );
   	//
   	// error?
   	//
   	if ( File_Handle == INVALID_FILE_HANDLE || Currently_Open == FALSE ) {
       		//
         	// error
         	//
       		return( -1 );
	}

	//
   	// do the tell
   	//   
   	file_pos = Tell_File( File_Handle );

	ASSERT( file_pos != -1 );
	return( file_pos );                        

	#endif

	#if( SUPPORT_STREAMS )

	ASSERT( File_Stream_Ptr != NULL );
   	//
   	// error?
   	//
   	if ( File_Stream_Ptr == NULL || Currently_Open == FALSE ) {
       		//
         	// error
         	//
       		return( -1 );
	}

	//
   	// do the tell!
   	//   
   	file_pos = ftell( File_Stream_Ptr );

	ASSERT( file_pos != -1 );
	return( file_pos );

	#endif
}

//------------------------------------------------------------------------------
// int StandardFileClass::Query_Size
//------------------------------------------------------------------------------

int StandardFileClass::Query_Size( void )
{
	int size;

	//
	// debug checks...
	//
	ASSERT( Currently_Open == TRUE );

	#if( SUPPORT_HANDLES )

	ASSERT( File_Handle != INVALID_FILE_HANDLE );
   	//
   	// error?
   	//
   	if ( File_Handle == INVALID_FILE_HANDLE || Currently_Open == FALSE ) {
       		//
         	// error
         	//
       		return( -1 );
	}

	size = File_Size( File_Handle );
   	ASSERT( size > -1 );
	#endif

	#if( SUPPORT_STREAMS )
	ASSERT( File_Stream_Ptr != NULL );
   	//
   	// error?
   	//
   	if ( File_Stream_Ptr == NULL || Currently_Open == FALSE ) {
       		//
         	// error
         	//
       		return( -1 );
	}

	size = File_Statistics.st_size;
   	ASSERT( size > -1 );
	#endif
   
	return( size );   
}

//------------------------------------------------------------------------------
// int StandardFileClass::Query_Size
//------------------------------------------------------------------------------

bool StandardFileClass::Query_Open( void )
{
	return( Currently_Open );
}

//------------------------------------------------------------------------------
// char *StandardFileClass::Query_Name_String
//------------------------------------------------------------------------------

char *StandardFileClass::Query_Name_String( void )
{
	return( File_Name );
}


#if( SUPPORT_STREAMS )

//------------------------------------------------------------------------------
// FILE *StandardFileClass::Query_File_Stream_Pointer
//------------------------------------------------------------------------------

FILE *StandardFileClass::Query_File_Stream_Pointer( void )
{
	return( File_Stream_Ptr );
}

#endif

//------------------------------------------------------------------------------
// private file class functions...
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
// void StandardFileClass::Reset
//------------------------------------------------------------------------------

void StandardFileClass::Reset( void )
{
	//
	// reset all internal data
	//
	#if( SUPPORT_HANDLES )
	File_Handle = INVALID_FILE_HANDLE;
	#endif
	#if( SUPPORT_STREAMS )
	File_Stream_Ptr = NULL;
	#endif
	Currently_Open = FALSE;
	File_Name[ 0 ] = '\0';
}


int StandardFileClass::End_Of_File	( void )
{
	#if( SUPPORT_HANDLES )
   	return( TRUE );
	#endif

	#if( SUPPORT_STREAMS )
	ASSERT( File_Stream_Ptr != NULL );
   	if ( File_Stream_Ptr == NULL || Currently_Open == FALSE ) {
    	return( -1 );
	}
   	return( feof( File_Stream_Ptr ));
	#endif
}

int StandardFileClass::Flush ( void )
{
	#if( SUPPORT_STREAMS )
	ASSERT( File_Stream_Ptr != NULL );
   	if ( File_Stream_Ptr == NULL || Currently_Open == FALSE ) {
    	return( -1 );
	}
   	return( fflush( File_Stream_Ptr ));
	#endif
}


//------------------------------------------------------------------------------
// non-class public functions from wnd_file.h
//------------------------------------------------------------------------------

#if( SUPPORT_HANDLES )

//------------------------------------------------------------------------------
// int Open_File
//------------------------------------------------------------------------------

// &&& - if enabled, must handle read and write combined

HANDLE Open_File( char const *file_name, int mode )
{
	HANDLE windows_file_handle;
	DWORD access;
	DWORD creation;
	DWORD share;
	// int fh;
	
	//
	// debug checks...
	//
	ASSERT( file_name != NULL );
	// ASSERT( mode == READ || mode == WRITE );
	ASSERT( mode == MODE_READ_ONLY	|| 
			mode == MODE_WRITE_ONLY	||
			mode == MODE_READ_AND_WRITE );

	//
	// get an available file handle
	//
	// fh = Get_Internal_File_Handle();
	// ASSERT( fh > INVALID_FILE_HANDLE );
	// if ( fh == INVALID_FILE_HANDLE ) {
	// 	return( INVALID_FILE_HANDLE );
	// }

	//
	// set the attributes based on read or write for the open
	//
	// if ( mode == READ ) {
	if ( mode == MODE_READ_ONLY ) {
   		access = GENERIC_READ;
		share = FILE_SHARE_READ;
		creation = OPEN_EXISTING;
	}
	// else if ( mode == WRITE ) {
	else if ( mode == MODE_WRITE_ONLY ) {
   		access = GENERIC_WRITE;
    	share = 0;
   		creation = CREATE_ALWAYS;
	}
	else if ( mode == MODE_READ_AND_WRITE ) {
		//
		// &&& are these correct?
		//
		access =	GENERIC_READ | GENERIC_WRITE;
		share = FILE_SHARE_READ | FILE_SHARE_WRITE;
		creation = OPEN_EXISTING;
	}
	else {
		//
		// error;
		//
		ASSERT( FALSE );
	}

	//
	// 32-bit open file
	//
	windows_file_handle = CreateFile( file_name,
                                     access,
                                     share,			
                                     NULL,			
                                     creation,
                                     FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                     NULL );		
	//
    // error?
    // 
    // we don't want to assert because we may be looking for a file
    // to just see if it is there...
    //
    //	ASSERT( windows_file_handle != INVALID_HANDLE_VALUE );
    //
    // error!
    //
    if ( windows_file_handle == INVALID_HANDLE_VALUE ) {
		//  #if( _DEBUG )
		//	Debug_Printf( "%s: Create file error is %d\r\n", file_name, GetLastError());
		//  #endif
        return( INVALID_FILE_HANDLE );
    }

	//
	// store the windows handle
	// 
	// Windows_File_Handles[ fh ] = windows_file_handle;
	//
	// return our internal file handle
	//
	// return( fh );

	//
	// &&& this should be HANDLE, not int
	//	
	// return( windows_file_handle );
	return( windows_file_handle );
}

//------------------------------------------------------------------------------
// int Close_File
//------------------------------------------------------------------------------

bool Close_File( HANDLE handle )
{
	bool success;

	//
	// debug checks...
	//
	// ASSERT( handle > INVALID_FILE_HANDLE );
	// ASSERT( Windows_File_Handles[ handle ] != NULL );
	ASSERT( handle != INVALID_FILE_HANDLE );

	//
	// close the file
	//
	// success = CloseHandle( Windows_File_Handles[ handle ] );
	//
	// &&& - this should be an actual HANDLE
	//
	success = CloseHandle( (HANDLE) handle );
	ASSERT( success == TRUE );

	// Debug_Printf( "File %d closed.\r\n", handle );

	//
	// free the entry
	//
	// Windows_File_Handles[ handle ] = NULL;

	//
	// return success or not
	//   
	// if ( success == TRUE ) {
	// 		//
	//		// return the invalid handle that closed
	//		//
	// 		return( handle );   	
	// }
	// //
	// // error
	// //
	// return( INVALID_FILE_HANDLE );
	return( success );
}

//------------------------------------------------------------------------------
// int Read_File
//------------------------------------------------------------------------------

int Read_File( HANDLE handle, void *buffer, unsigned long int bytes_to_read )
{
	bool success;
	DWORD bytes_actually_read;

	//
	// debug checks...
	//
	ASSERT( handle > INVALID_FILE_HANDLE );
	ASSERT( buffer != NULL );
	// ASSERT( bytes_to_read > 0 );
	// ASSERT( Windows_File_Handles[ handle ] != NULL );

	// Debug_Printf( "Reading file %d\r\n", handle );
   
	// success = ReadFile( Windows_File_Handles[ handle ],
	//                     (void *) buffer,
	//                     (DWORD) bytes_to_read,
	//                     (DWORD *) &bytes_actually_read,
	//                     NULL );

	//
	// &&& use real HANDLE
	//
	success = ReadFile( (HANDLE) handle,
                       (void *) buffer,
                       (DWORD) bytes_to_read,
                       (DWORD *) &bytes_actually_read,
                       NULL );

	ASSERT( success == TRUE );
   
	if ( success == FALSE ) {
   		//
		// no bytes read
		//
   		return( 0 );
	}                       

   return( bytes_actually_read );
}

//------------------------------------------------------------------------------
// int Write_File
//------------------------------------------------------------------------------

int Write_File( HANDLE handle, void const *buffer, unsigned long int bytes_to_write )
{
	bool success;
	DWORD bytes_actually_written;

	//
	// debug checks...
	//
	ASSERT( handle != INVALID_FILE_HANDLE );
	ASSERT( buffer != NULL );
	// ASSERT( bytes_to_write > 0 );
	// ASSERT( Windows_File_Handles[ handle ] != NULL );

	// Debug_Printf( "Writing file %d\r\n", handle );
   
	// success = WriteFile( Windows_File_Handles[ handle ],
	//                      buffer,
	//                      (DWORD) bytes_to_write,
	//                      (DWORD *) &bytes_actually_written,
	//                      NULL );

	//
	// &&& make this a real handle
	//
	success = WriteFile( (HANDLE) handle,
                        buffer,
                        (DWORD) bytes_to_write,
                        (DWORD *) &bytes_actually_written,
                        NULL );

	ASSERT( success == TRUE );
	ASSERT( bytes_actually_written == bytes_to_write );
   
	if ( success == FALSE ) {
   		//
		// no bytes written
		//
   		return( 0 );
	}                       

	return( bytes_actually_written );
}

//------------------------------------------------------------------------------
// bool Seek_File
//------------------------------------------------------------------------------

bool Seek_File( HANDLE handle, int distance, int seek_file_location )
{
	DWORD success;
	int move_method;

	//
	// debug checks...
	//
	ASSERT( handle != INVALID_FILE_HANDLE );
	// ASSERT( distance >= 0 );
	ASSERT( seek_file_location == SEEK_SET || 
   			seek_file_location == SEEK_CUR || 
			seek_file_location == SEEK_END );
	// ASSERT( Windows_File_Handles[ handle ] != NULL );

	//
	// set the seek movement method
	//
	if ( seek_file_location == SEEK_SET ) {
   		move_method = FILE_BEGIN;
	}
	else if ( seek_file_location == SEEK_CUR ) {
		move_method = FILE_CURRENT;
	}
	else if ( seek_file_location == SEEK_END ) {
		move_method = FILE_END;
	}
   
	// success = SetFilePointer( Windows_File_Handles[ handle ],
	//                          distance,
	//                          NULL,
	//                          move_method );

	//
	// make this a real handle
	//                            
	success = SetFilePointer( (HANDLE) handle,
                             distance,
                             NULL,
                             move_method );
                            
	if ( success == 0xFFFFFFFF ) {
   		return( FALSE );
	}                            
	return( TRUE );   	           
}

//------------------------------------------------------------------------------
// int Tell_File
//------------------------------------------------------------------------------

int Tell_File( HANDLE handle )
{
	int move_method;
	int pos;

	//
	// debug checks...
	//
	ASSERT( handle != INVALID_FILE_HANDLE );
	// ASSERT( Windows_File_Handles[ handle ] != NULL );

	//
	// set the seek movement method
	//
	move_method = FILE_CURRENT;

	//
	// move nowhere
	//   
	pos = SetFilePointer( handle,
                         0, // distance to move
                         NULL,
                         move_method );
                            
	if ( pos == 0xFFFFFFFF ) {
   		return( -1 );
	}                            
	return( pos );   	           
}

//------------------------------------------------------------------------------
// int File_Size
//------------------------------------------------------------------------------

int File_Size( HANDLE handle )
{
	DWORD file_size;

	//
	// debug checks...
	//
	ASSERT( handle != INVALID_FILE_HANDLE );
	// ASSERT( Windows_File_Handles[ handle ] != NULL );

	file_size = GetFileSize( handle, NULL );
	ASSERT( file_size != 0xFFFFFFFF );                            

	//
	// error!
	//
	if ( file_size == 0xFFFFFFFF ) {
   		return( -1 );
	}

	//
	// return size
	//   	
	return( (int) file_size );
}

//------------------------------------------------------------------------------
// bool Full_Path_File_Exists
//------------------------------------------------------------------------------

bool Full_Path_File_Exists( char const *file_name )
{
	HANDLE fh;

	//
	// debug checks...
	//
	ASSERT( file_name != NULL );

	//
	// if we can open the file for read, it exists...
	//
	fh = Open_File( file_name, MODE_READ_ONLY );
	//
	// don't assert, because the we might be checking for a file
	// that actually does not exist!
	//
	// ASSERT( fh > INVALID_FILE_HANDLE );

	//
	// close the file and return success if opened
	//   
	if ( fh != INVALID_FILE_HANDLE ) {
   		Close_File( fh );
		return( TRUE );
	}

	//
	// no success if file was not opened
	//
	return( FALSE );
}

//------------------------------------------------------------------------------
// bool HD_File_Exists
//------------------------------------------------------------------------------

bool HD_File_Exists( char const *file_name )
{
	HANDLE fh;
	char full_path[ MAX_PATH_SIZE ];

	//
	// debug checks...
	//
	ASSERT( file_name != NULL );
	strcpy( full_path, HD_Path );
	strcat( full_path, file_name );

	//
	// if we can open the file for read, it exists...
	//
	fh = Open_File( full_path, MODE_READ_ONLY );
	//
	// don't assert, because the we might be checking for a file
	// that actually does not exist!
	//

	//
	// close the file and return success if opened
	//   
	if ( fh != INVALID_FILE_HANDLE ) {
   		Close_File( fh );
		return( TRUE );
	}

	//
	// no success if file was not opened
	//
	return( FALSE );
}

//------------------------------------------------------------------------------
// bool CD_File_Exists
//------------------------------------------------------------------------------

bool CD_File_Exists( char const *file_name )
{
	HANDLE fh;
	char full_path[ MAX_PATH_SIZE ];

	//
	// debug checks...
	//
	ASSERT( file_name != NULL );
	strcpy( full_path, CD_Path );
	strcat( full_path, file_name );

	//
	// if we can open the file for read, it exists...
	//
	fh = Open_File( full_path, MODE_READ_ONLY );
	//
	// don't assert, because the we might be checking for a file
	// that actually does not exist!
	//

	//
	// close the file and return success if opened
	//   
	if ( fh != INVALID_FILE_HANDLE ) {
   		Close_File( fh );
		return( TRUE );
	}

	//
	// no success if file was not opened
	//
	return( FALSE );
}

#if( 0 )
//------------------------------------------------------------------------------
// bool Find_File
//------------------------------------------------------------------------------
bool Find_File( char const *file_name )
{
	return( File_Exists( file_name ) );
}
#endif

#if( 0 )
//------------------------------------------------------------------------------
// int Get_Internal_File_Handle
//------------------------------------------------------------------------------
//
// private...
//
int Get_Internal_File_Handle( void )
{
	static bool _initialized = FALSE;
	int i;

	//
	// initialize file handle table once!
	//
	if ( ! _initialized ) {
   		for ( i = 0; i < MAX_FILES_OPEN_AT_A_TIME; i ++ ) {
			Windows_File_Handles[ i ] = NULL;      	
		}
		_initialized = TRUE;
	}

	//
	// look for free slot
	//
  	for ( i = 0; i < MAX_FILES_OPEN_AT_A_TIME; i ++ ) {
   		if ( Windows_File_Handles[ i ] == NULL ) {
      		return( i );
		}
	}

	//
	// no free slot found
	//   
	ASSERT( FALSE );
	return( INVALID_FILE_HANDLE );
}
#endif


#endif // SUPPORT_HANDLES

#if( SUPPORT_STREAMS )

//------------------------------------------------------------------------------
// bool Full_Path_File_Exists
//------------------------------------------------------------------------------

bool Full_Path_File_Exists( char const *file_name )
{
	FILE *file_stream_ptr;

	file_stream_ptr = fopen( file_name, "rb" );
	if ( file_stream_ptr != NULL ) {
   		fclose( file_stream_ptr );
		return( TRUE );
	}
	return( FALSE );
}

//------------------------------------------------------------------------------
// bool HD_File_Exists
//------------------------------------------------------------------------------

bool HD_File_Exists( char const *file_name )
{
	FILE *file_stream_ptr;
	char full_path[ MAX_PATH_SIZE ];

	//
	// debug checks...
	//
	ASSERT( file_name != NULL );

	strcpy( full_path, HD_Path );
	strcat( full_path, file_name );

	file_stream_ptr = fopen( full_path, "rb" );
	if ( file_stream_ptr != NULL ) {
   		fclose( file_stream_ptr );
		return( TRUE );
	}
	return( FALSE );
}

//------------------------------------------------------------------------------
// bool CD_File_Exists
//------------------------------------------------------------------------------

bool CD_File_Exists( char const *file_name )
{
	FILE *file_stream_ptr;
	char full_path[ MAX_PATH_SIZE ];

	//
	// debug checks...
	//
	ASSERT( file_name != NULL );

	strcpy( full_path, CD_Path );
	strcat( full_path, file_name );

	file_stream_ptr = fopen( full_path, "rb" );
	if ( file_stream_ptr != NULL ) {
   		fclose( file_stream_ptr );
		return( TRUE );
	}
	return( FALSE );
}


#if( 0 )   
//------------------------------------------------------------------------------
// bool Find_File
//------------------------------------------------------------------------------
bool Find_File( char const *file_name )
{
	return( File_Exists( file_name ) );
}
#endif

#endif // SUPPORT_STREAMS

