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

// FILE: XferCRC.cpp //////////////////////////////////////////////////////////////////////////////
// Author: Matt Campbell, February 2002
// Desc:   Xfer CRC implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/XferCRC.h"
#include "Common/XferDeepCRC.h"
#include "Common/crc.h"
#include "Common/Snapshot.h"
#include "winsock2.h" // for htonl

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
XferCRC::XferCRC( void )
{

	m_xferMode = XFER_CRC;
	//Added By Sadullah Nader
	//Initialization(s) inserted
	m_crc = 0;
	//
}  // end XferCRC

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
XferCRC::~XferCRC( void )
{

}  // end ~XferCRC

//-------------------------------------------------------------------------------------------------
/** Open file 'identifier' for writing */
//-------------------------------------------------------------------------------------------------
void XferCRC::open( AsciiString identifier )
{

	// call base class
	Xfer::open( identifier );

	// initialize CRC to brand new one at zero
	m_crc = 0;

}  // end open

//-------------------------------------------------------------------------------------------------
/** Close our current file */
//-------------------------------------------------------------------------------------------------
void XferCRC::close( void )
{

}  // end close

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Int XferCRC::beginBlock( void )
{

	return 0;

}  // end beginBlock

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void XferCRC::endBlock( void )
{

}  // end endBlock

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void XferCRC::addCRC( UnsignedInt val )
{
	int hibit;

	val = htonl(val);

	if (m_crc & 0x80000000)
	{
		hibit = 1;
	}
	else
	{
		hibit = 0;
	}

	m_crc <<= 1;
	m_crc += val;
	m_crc += hibit;

}  // end addCRC

// ------------------------------------------------------------------------------------------------
/** Entry point for xfering a snapshot */
// ------------------------------------------------------------------------------------------------
void XferCRC::xferSnapshot( Snapshot *snapshot )
{

	if( snapshot == NULL )
	{

		return;

	}  // end if

	// run the crc function of the snapshot
	snapshot->crc( this );

}  // end xferSnapshot

//-------------------------------------------------------------------------------------------------
/** Perform a single CRC operation on the data passed in */
//-------------------------------------------------------------------------------------------------
void XferCRC::xferImplementation( void *data, Int dataSize )
{

	if (!data || dataSize < 1)
	{
		return;
	}

	const UnsignedInt *uintPtr = (const UnsignedInt *) (data);

	for (Int i=0 ; i<dataSize/4 ; i++)
	{
		addCRC (*uintPtr++);
	}

	int leftover = dataSize & 3;
	if (leftover)
	{
		UnsignedInt val = 0;
		const unsigned char *c = (const unsigned char *)uintPtr;
		for (Int i=0; i<leftover; i++)
		{
			val += (c[i] << (i*8));
		}
		val = htonl(val);
		addCRC (val);
	}
	
}  // end xferImplementation

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void XferCRC::skip( Int dataSize )
{

}  // end skip

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UnsignedInt XferCRC::getCRC( void )
{

	return htonl(m_crc);

}  // end skip


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
XferDeepCRC::XferDeepCRC( void )
{

	m_xferMode = XFER_SAVE;
	m_fileFP = NULL;

}  // end XferCRC

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
XferDeepCRC::~XferDeepCRC( void )
{

	// warn the user if a file was left open
	if( m_fileFP != NULL )
	{

		DEBUG_CRASH(( "Warning: Xfer file '%s' was left open", m_identifier.str() ));
		close();

	}  // end if

}  // end ~XferCRC

//-------------------------------------------------------------------------------------------------
/** Open file 'identifier' for writing */
//-------------------------------------------------------------------------------------------------
void XferDeepCRC::open( AsciiString identifier )
{

	m_xferMode = XFER_SAVE;

	// sanity, check to see if we're already open
	if( m_fileFP != NULL )
	{

		DEBUG_CRASH(( "Cannot open file '%s' cause we've already got '%s' open",
									identifier.str(), m_identifier.str() ));
		throw XFER_FILE_ALREADY_OPEN;

	}  // end if

	// call base class
	Xfer::open( identifier );

	// open the file
	m_fileFP = fopen( identifier.str(), "w+b" );
	if( m_fileFP == NULL )
	{
		
		DEBUG_CRASH(( "File '%s' not found", identifier.str() ));
		throw XFER_FILE_NOT_FOUND;

	}  // end if

	// initialize CRC to brand new one at zero
	m_crc = 0;

}  // end open

//-------------------------------------------------------------------------------------------------
/** Close our current file */
//-------------------------------------------------------------------------------------------------
void XferDeepCRC::close( void )
{

	// sanity, if we don't have an open file we can do nothing
	if( m_fileFP == NULL )
	{

		DEBUG_CRASH(( "Xfer close called, but no file was open" ));
		throw XFER_FILE_NOT_OPEN;

	}  // end if

	// close the file
	fclose( m_fileFP );
	m_fileFP = NULL;

	// erase the filename
	m_identifier.clear();

}  // end close

//-------------------------------------------------------------------------------------------------
/** Perform a single CRC operation on the data passed in */
//-------------------------------------------------------------------------------------------------
void XferDeepCRC::xferImplementation( void *data, Int dataSize )
{

	if (!data || dataSize < 1)
	{
		return;
	}

	// sanity
	DEBUG_ASSERTCRASH( m_fileFP != NULL, ("XferSave - file pointer for '%s' is NULL\n",
										 m_identifier.str()) );

	// write data to file
	if( fwrite( data, dataSize, 1, m_fileFP ) != 1 )
	{

		DEBUG_CRASH(( "XferSave - Error writing to file '%s'", m_identifier.str() ));
		throw XFER_WRITE_ERROR;

	}  // end if

	XferCRC::xferImplementation( data, dataSize );

}  // end xferImplementation

// ------------------------------------------------------------------------------------------------
/** Save ascii string */
// ------------------------------------------------------------------------------------------------
void XferDeepCRC::xferMarkerLabel( AsciiString asciiStringData )
{

}  // end xferAsciiString

// ------------------------------------------------------------------------------------------------
/** Save ascii string */
// ------------------------------------------------------------------------------------------------
void XferDeepCRC::xferAsciiString( AsciiString *asciiStringData )
{

	// sanity
	if( asciiStringData->getLength() > 16385 )
	{

		DEBUG_CRASH(( "XferSave cannot save this ascii string because it's too long.  Change the size of the length header (but be sure to preserve save file compatability" ));
		throw XFER_STRING_ERROR;

	}  // end if

	// save length of string to follow
	UnsignedShort len = asciiStringData->getLength();
	xferUnsignedShort( &len );

	// save string data
	if( len > 0 )
		xferUser( (void *)asciiStringData->str(), sizeof( Byte ) * len );

}  // end xferAsciiString

// ------------------------------------------------------------------------------------------------
/** Save unicodee string */
// ------------------------------------------------------------------------------------------------
void XferDeepCRC::xferUnicodeString( UnicodeString *unicodeStringData )
{
	
	// sanity
	if( unicodeStringData->getLength() > 255 )
	{

		DEBUG_CRASH(( "XferSave cannot save this unicode string because it's too long.  Change the size of the length header (but be sure to preserve save file compatability" ));
		throw XFER_STRING_ERROR;

	}  // end if

	// save length of string to follow
	Byte len = unicodeStringData->getLength();
	xferByte( &len );

	// save string data
	if( len > 0 )
		xferUser( (void *)unicodeStringData->str(), sizeof( WideChar ) * len );

}  // end xferUnicodeString
