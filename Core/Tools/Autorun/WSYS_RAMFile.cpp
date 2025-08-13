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

//----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright(C) 2001 - All Rights Reserved
//
//----------------------------------------------------------------------------
//
// Project:   WSYS Library
//
// Module:    IO
//
// File name: WSYS_RAMFile.cpp
//
// Created:   11/08/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//         Includes
//----------------------------------------------------------------------------

#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include <sys/stat.h>

#include "WSYS_FileSystem.h"
#include "WSYS_RAMFile.h"


//----------------------------------------------------------------------------
//         Externals
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Defines
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Types
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Data
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Public Data
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Prototypes
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Functions
//----------------------------------------------------------------------------

//=================================================================
// RAMFile::RAMFile
//=================================================================

RAMFile::RAMFile()
: m_size(0),
	m_data(NULL)
{

}


//----------------------------------------------------------------------------
//         Public Functions
//----------------------------------------------------------------------------


//=================================================================
// RAMFile::~RAMFile
//=================================================================

RAMFile::~RAMFile()
{
	delete [] m_data;

	File::close();

}

//=================================================================
// RAMFile::open
//=================================================================
/**
  *	This function opens a file using the standard C open() call. Access flags
	* are mapped to the appropriate open flags. Returns true if file was opened
	* successfully.
	*/
//=================================================================

Bool RAMFile::open( const Char *filename, Int access )
{
	File *file = TheFileSystem->open( filename, access );

	if ( file == NULL )
	{
		return FALSE;
	}

	Bool result = open( file );

	file->close();

	return result;

	return TRUE;
}

//============================================================================
// RAMFile::open
//============================================================================

Bool RAMFile::open( File *file )
{
	if ( file == NULL )
	{
		return NULL;
	}

	Int access = file->getAccess();

	if ( !File::open( file->getName(), access ))
	{
		return FALSE;
	}

	// read whole file in to memory
	m_size = file->size();
	m_data = new char [ m_size ];

	if ( m_data == NULL )
	{
		return FALSE;
	}

	m_size = file->read( m_data, m_size );

	if ( m_size < 0 )
	{
		delete [] m_data;
		m_data = NULL;
		return FALSE;
	}

	m_pos = 0;

	return TRUE;
}

//=================================================================
// RAMFile::close
//=================================================================
/**
	* Closes the current file if it is open.
  * Must call RAMFile::close() for each successful RAMFile::open() call.
	*/
//=================================================================

void RAMFile::close( void )
{
	if ( m_data )
	{
		delete [] m_data;
		m_data = NULL;
	}

	File::close();
}

//=================================================================
// RAMFile::read
//=================================================================

Int RAMFile::read( void *buffer, Int bytes )
{
	if( m_data == NULL )
	{
		return -1;
	}

	Int bytesLeft = m_size - m_pos ;

	if ( bytes > bytesLeft )
	{
		bytes = bytesLeft;
	}

	if ( bytes > 0 )
	{
		memcpy ( buffer, &m_data[m_pos], bytes );
	}
	else
	{
		bytes = 0;
	}

	m_pos += bytes;

	return bytes;
}

//=================================================================
// RAMFile::write
//=================================================================

Int RAMFile::write( void *buffer, Int bytes )
{
	return -1;
}

//=================================================================
// RAMFile::seek
//=================================================================

Int RAMFile::seek( Int pos, seekMode mode)
{
	Int newPos;

	switch( mode )
	{
		case START:
			newPos = pos;
			break;
		case CURRENT:
			newPos = m_pos + pos;
			break;
		case END:
			newPos = m_size - pos - 1;
			break;
		default:
			// bad seek mode
			return -1;
	}

	if ( newPos < 0 )
	{
		newPos = 0;
	}
	else if ( newPos > m_size - 1 )
	{
		newPos = m_size - 1;
	}

	m_pos = newPos;

	return m_pos;

}

