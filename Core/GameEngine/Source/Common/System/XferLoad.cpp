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

// FILE: XferLoad.cpp /////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, February 2002
// Desc:   Xfer implemenation for loading from disk
///////////////////////////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine
#include "Common/Debug.h"
#include "Common/GameState.h"
#include "Common/Snapshot.h"
#include "Common/XferLoad.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
XferLoad::XferLoad( void )
{

	m_xferMode = XFER_LOAD;
	m_fileFP = NULL;

}  // end XferLoad

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
XferLoad::~XferLoad( void )
{

	// warn the user if a file was left open
	if( m_fileFP != NULL )
	{

		DEBUG_CRASH(( "Warning: Xfer file '%s' was left open\n", m_identifier.str() ));
		close();

	}  // end if

}  // end ~XferLoad

//-------------------------------------------------------------------------------------------------
/** Open file 'identifier' for reading */
//-------------------------------------------------------------------------------------------------
void XferLoad::open( AsciiString identifier )
{

	// sanity, check to see if we're already open
	if( m_fileFP != NULL )
	{

		DEBUG_CRASH(( "Cannot open file '%s' cause we've already got '%s' open\n",
									identifier.str(), m_identifier.str() ));
		throw XFER_FILE_ALREADY_OPEN;

	}  // end if

	// call base class
	Xfer::open( identifier );

	// open the file
	m_fileFP = fopen( identifier.str(), "rb" );
	if( m_fileFP == NULL )
	{
		
		DEBUG_CRASH(( "File '%s' not found\n", identifier.str() ));
		throw XFER_FILE_NOT_FOUND;

	}  // end if

}  // end open

//-------------------------------------------------------------------------------------------------
/** Close our current file */
//-------------------------------------------------------------------------------------------------
void XferLoad::close( void )
{

	// sanity, if we don't have an open file we can do nothing
	if( m_fileFP == NULL )
	{

		DEBUG_CRASH(( "Xfer close called, but no file was open\n" ));
		throw XFER_FILE_NOT_OPEN;

	}  // end if

	// close the file
	fclose( m_fileFP );
	m_fileFP = NULL;

	// erase the filename
	m_identifier.clear();

}  // end close

//-------------------------------------------------------------------------------------------------
/** Read a block size descriptor from the file at the current position */
//-------------------------------------------------------------------------------------------------
Int XferLoad::beginBlock( void )
{

	// sanity
	DEBUG_ASSERTCRASH( m_fileFP != NULL, ("Xfer begin block - file pointer for '%s' is NULL\n",
										 m_identifier.str()) );

	// read block size
	XferBlockSize blockSize;
	if( fread( &blockSize, sizeof( XferBlockSize ), 1, m_fileFP ) != 1 )
	{
		
		DEBUG_CRASH(( "Xfer - Error reading block size for '%s'\n", m_identifier.str() ));
		return 0;

	}  // end if

	// return the block size
	return blockSize;

}  // end beginBlock

// ------------------------------------------------------------------------------------------------
/** End block ... this does nothing when reading */
// ------------------------------------------------------------------------------------------------
void XferLoad::endBlock( void )
{

}  // end endBlock

//-------------------------------------------------------------------------------------------------
/** Skip forward 'dataSize' bytes in the file */
//-------------------------------------------------------------------------------------------------
void XferLoad::skip( Int dataSize )
{

	// sanity
	DEBUG_ASSERTCRASH( m_fileFP != NULL, ("XferLoad::skip - file pointer for '%s' is NULL\n",
										 m_identifier.str()) );

	// sanity
	DEBUG_ASSERTCRASH( dataSize >=0, ("XferLoad::skip - dataSize '%d' must be greater than 0\n",
										 dataSize) );

	// skip datasize in the file from the current position
	if( fseek( m_fileFP, dataSize, SEEK_CUR ) != 0 )
		throw XFER_SKIP_ERROR;

}  // end skip

// ------------------------------------------------------------------------------------------------
/** Entry point for xfering a snapshot */
// ------------------------------------------------------------------------------------------------
void XferLoad::xferSnapshot( Snapshot *snapshot )
{

	if( snapshot == NULL )
	{

		DEBUG_CRASH(( "XferLoad::xferSnapshot - Invalid parameters\n" ));
		throw XFER_INVALID_PARAMETERS;

	}  // end if

	// run the xfer function of the snapshot
	snapshot->xfer( this );

	// add this snapshot to the game state for later post processing if not restricted
	if( BitIsSet( getOptions(), XO_NO_POST_PROCESSING ) == FALSE )
		TheGameState->addPostProcessSnapshot( snapshot );

}  // end xferSnapshot

// ------------------------------------------------------------------------------------------------
/** Read string from file and store in ascii string */
// ------------------------------------------------------------------------------------------------
void XferLoad::xferAsciiString( AsciiString *asciiStringData )
{
	
	// read bytes of string length to follow
	UnsignedByte len;
	xferUnsignedByte( &len );

	// read all the string data
	const Int MAX_XFER_LOAD_STRING_BUFFER = 1024;
	static Char buffer[ MAX_XFER_LOAD_STRING_BUFFER ];

	if( len > 0 )
		xferUser( buffer, sizeof( Byte ) * len );
	buffer[ len ] = 0;  // terminate

	// save into ascii string
	asciiStringData->set( buffer );

}  // end xferAsciiString

// ------------------------------------------------------------------------------------------------
/** Read string from file and store in unicode string */
// ------------------------------------------------------------------------------------------------
void XferLoad::xferUnicodeString( UnicodeString *unicodeStringData )
{
	
	// read bytes of string length to follow
	UnsignedByte len;
	xferUnsignedByte( &len );

	// read all the string data
	const Int MAX_XFER_LOAD_STRING_BUFFER = 1024;
	static WideChar buffer[ MAX_XFER_LOAD_STRING_BUFFER ];

	if( len > 0 )
		xferUser( buffer, sizeof( WideChar ) * len );
	buffer[ len ] = 0;  // terminate

	// save into unicode string
	unicodeStringData->set( buffer );

}  // end xferUnicodeString

//-------------------------------------------------------------------------------------------------
/** Perform the read operation */
//-------------------------------------------------------------------------------------------------
void XferLoad::xferImplementation( void *data, Int dataSize )
{

	// sanity
	DEBUG_ASSERTCRASH( m_fileFP != NULL, ("XferLoad - file pointer for '%s' is NULL\n",
										 m_identifier.str()) );

	// read data from file
	if( fread( data, dataSize, 1, m_fileFP ) != 1 )
	{

		DEBUG_CRASH(( "XferLoad - Error reading from file '%s'\n", m_identifier.str() ));
		throw XFER_READ_ERROR;

	}  // end if
	
}  // end xferImplementation

