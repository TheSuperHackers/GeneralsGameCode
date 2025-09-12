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

// FILE: XferSave.cpp /////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, February 2002
// Desc:   Xfer disk write implementation
///////////////////////////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/XferSave.h"
#include "Common/Snapshot.h"
#include "Common/GameMemory.h"

// PRIVATE TYPES //////////////////////////////////////////////////////////////////////////////////
class XferBlockData : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(XferBlockData, "XferBlockData")

public:

	XferFilePos filePos;			///< the file position of this block
	XferBlockData *next;			///< next block on the stack

};
EMPTY_DTOR(XferBlockData)

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC METHDOS /////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
XferSave::XferSave( void )
{

	m_xferMode = XFER_SAVE;
	m_fileFP = NULL;
	m_blockStack = NULL;

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
XferSave::~XferSave( void )
{

	// warn the user if a file was left open
	if( m_fileFP != NULL )
	{

		DEBUG_CRASH(( "Warning: Xfer file '%s' was left open", m_identifier.str() ));
		close();

	}

	//
	// the block stack should be empty, if it's not that means we started blocks but never
	// called enough matching end blocks
	//
	if( m_blockStack != NULL )
	{

		// tell the user there is an error
		DEBUG_CRASH(( "Warning: XferSave::~XferSave - m_blockStack was not NULL!" ));

		// delete the block stack
		XferBlockData *next;
		while( m_blockStack )
		{

			next = m_blockStack->next;
			deleteInstance(m_blockStack);
			m_blockStack = next;

		}

	}

}

//-------------------------------------------------------------------------------------------------
/** Open file 'identifier' for writing */
//-------------------------------------------------------------------------------------------------
void XferSave::open( AsciiString identifier )
{

	// sanity, check to see if we're already open
	if( m_fileFP != NULL )
	{

		DEBUG_CRASH(( "Cannot open file '%s' cause we've already got '%s' open",
									identifier.str(), m_identifier.str() ));
		throw XFER_FILE_ALREADY_OPEN;

	}

	// call base class
	Xfer::open( identifier );

	// open the file
	m_fileFP = fopen( identifier.str(), "w+b" );
	if( m_fileFP == NULL )
	{

		DEBUG_CRASH(( "File '%s' not found", identifier.str() ));
		throw XFER_FILE_NOT_FOUND;

	}

}

//-------------------------------------------------------------------------------------------------
/** Close our current file */
//-------------------------------------------------------------------------------------------------
void XferSave::close( void )
{

	// sanity, if we don't have an open file we can do nothing
	if( m_fileFP == NULL )
	{

		DEBUG_CRASH(( "Xfer close called, but no file was open" ));
		throw XFER_FILE_NOT_OPEN;

	}

	// close the file
	fclose( m_fileFP );
	m_fileFP = NULL;

	// erase the filename
	m_identifier.clear();

}

//-------------------------------------------------------------------------------------------------
/** Write a placeholder at the current location in the file and store this location
	* internally.  The next endBlock that is called will back up to the most recently stored
	* beginBlock and write the difference in file bytes from the endBlock call to the
	* location of this beginBlock.  The current file position will then return to the location
	* at which endBlock was called */
//-------------------------------------------------------------------------------------------------
Int XferSave::beginBlock( void )
{

	// sanity
	DEBUG_ASSERTCRASH( m_fileFP != NULL, ("Xfer begin block - file pointer for '%s' is NULL",
										 m_identifier.str()) );

	// get the current file position so we can back up here for the next end block call
	XferFilePos filePos = ftell( m_fileFP );

	// write a placeholder
	XferBlockSize blockSize = 0;
	if( fwrite( &blockSize, sizeof( XferBlockSize ), 1, m_fileFP ) != 1 )
	{

		DEBUG_CRASH(( "XferSave::beginBlock - Error writing block size in '%s'",
									m_identifier.str() ));
		return XFER_WRITE_ERROR;

	}

	// save this block position on the top of the "stack"
	XferBlockData *top = newInstance(XferBlockData);
// impossible -- exception will be thrown (srj)
//	if( top == NULL )
//	{
//
//		DEBUG_CRASH(( "XferSave - Begin block, out of memory - can't save block stack data" ));
//		return XFER_OUT_OF_MEMORY;
//
//	}  // end if

	top->filePos = filePos;
	top->next = m_blockStack;
	m_blockStack = top;

	return XFER_OK;

}

//-------------------------------------------------------------------------------------------------
/** Do the tail end as described in beginBlock above.  Back up to the last begin block,
	* write the file difference from current position to the last begin position, and put
	* current file position back to where it was */
//-------------------------------------------------------------------------------------------------
void XferSave::endBlock( void )
{

	// sanity
	DEBUG_ASSERTCRASH( m_fileFP != NULL, ("Xfer end block - file pointer for '%s' is NULL",
										 m_identifier.str()) );

	// sanity, make sure we have a block started
	if( m_blockStack == NULL )
	{

		DEBUG_CRASH(( "Xfer end block called, but no matching begin block was found" ));
		throw XFER_BEGIN_END_MISMATCH;

	}

	// save our current file position
	XferFilePos currentFilePos = ftell( m_fileFP );

	// pop the block descriptor off the top of the block stack
	XferBlockData *top = m_blockStack;
	m_blockStack = m_blockStack->next;

	// rewind the file to the block position
	fseek( m_fileFP, top->filePos, SEEK_SET );

	// write the size in bytes between the block position and what is our current file position
	XferBlockSize blockSize = currentFilePos - top->filePos - sizeof( XferBlockSize );
	if( fwrite( &blockSize, sizeof( XferBlockSize ), 1, m_fileFP ) != 1 )
	{

		DEBUG_CRASH(( "Error writing block size to file '%s'", m_identifier.str() ));
		throw XFER_WRITE_ERROR;

	}

	// place the file pointer back to the current position
	fseek( m_fileFP, currentFilePos, SEEK_SET );

	// delete the block data as it's all used up now
	deleteInstance(top);

}

//-------------------------------------------------------------------------------------------------
/** Skip forward 'dataSize' bytes in the file */
//-------------------------------------------------------------------------------------------------
void XferSave::skip( Int dataSize )
{

	// sanity
	DEBUG_ASSERTCRASH( m_fileFP != NULL, ("XferSave - file pointer for '%s' is NULL",
										 m_identifier.str()) );


	// skip forward dataSize bytes
	fseek( m_fileFP, dataSize, SEEK_CUR );

}

// ------------------------------------------------------------------------------------------------
/** Entry point for xfering a snapshot */
// ------------------------------------------------------------------------------------------------
void XferSave::xferSnapshot( Snapshot *snapshot )
{

	if( snapshot == NULL )
	{

		DEBUG_CRASH(( "XferSave::xferSnapshot - Invalid parameters" ));
		throw XFER_INVALID_PARAMETERS;

	}

	// run the xfer function of the snapshot
	snapshot->xfer( this );

}

// ------------------------------------------------------------------------------------------------
/** Save ascii string */
// ------------------------------------------------------------------------------------------------
void XferSave::xferAsciiString( AsciiString *asciiStringData )
{

	// sanity
	if( asciiStringData->getLength() > 255 )
	{

		DEBUG_CRASH(( "XferSave cannot save this unicode string because it's too long.  Change the size of the length header (but be sure to preserve save file compatability" ));
		throw XFER_STRING_ERROR;

	}

	// save length of string to follow
	UnsignedByte len = asciiStringData->getLength();
	xferUnsignedByte( &len );

	// save string data
	if( len > 0 )
		xferUser( (void *)asciiStringData->str(), sizeof( Byte ) * len );

}

// ------------------------------------------------------------------------------------------------
/** Save unicodee string */
// ------------------------------------------------------------------------------------------------
void XferSave::xferUnicodeString( UnicodeString *unicodeStringData )
{

	// sanity
	if( unicodeStringData->getLength() > 255 )
	{

		DEBUG_CRASH(( "XferSave cannot save this unicode string because it's too long.  Change the size of the length header (but be sure to preserve save file compatability" ));
		throw XFER_STRING_ERROR;

	}

	// save length of string to follow
	UnsignedByte len = unicodeStringData->getLength();
	xferUnsignedByte( &len );

	// save string data
	if( len > 0 )
		xferUser( (void *)unicodeStringData->str(), sizeof( WideChar ) * len );

}

//-------------------------------------------------------------------------------------------------
/** Perform the write operation */
//-------------------------------------------------------------------------------------------------
void XferSave::xferImplementation( void *data, Int dataSize )
{

	// sanity
	DEBUG_ASSERTCRASH( m_fileFP != NULL, ("XferSave - file pointer for '%s' is NULL",
										 m_identifier.str()) );

	// write data to file
	if( fwrite( data, dataSize, 1, m_fileFP ) != 1 )
	{

		DEBUG_CRASH(( "XferSave - Error writing to file '%s'", m_identifier.str() ));
		throw XFER_WRITE_ERROR;

	}

}
