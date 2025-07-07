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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright(C) 2001 - All Rights Reserved                  
//                                                                          
//----------------------------------------------------------------------------
//
// Project:   RTS
//
// Module:    IO
//
// File name: StreamingArchiveFile.cpp
//
// Created:   12/06/02
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//         Includes                                                      
//----------------------------------------------------------------------------

#include "PreRTS.h"

#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include <sys/stat.h>

#include "Common/AsciiString.h"
#include "Common/FileSystem.h"
#include "Common/StreamingArchiveFile.h"
#include "Common/PerfTimer.h"
									

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
// StreamingArchiveFile::StreamingArchiveFile
//=================================================================

StreamingArchiveFile::StreamingArchiveFile()
: m_file(NULL), 
	m_startingPos(0), 
	m_size(0), 
	m_curPos(0)
{

}


//----------------------------------------------------------------------------
//         Public Functions                                                
//----------------------------------------------------------------------------


//=================================================================
// StreamingArchiveFile::~StreamingArchiveFile	
//=================================================================

StreamingArchiveFile::~StreamingArchiveFile()
{
	File::close();
}

//=================================================================
// StreamingArchiveFile::open	
//=================================================================
/**
  *	This function opens a file using the standard C open() call. Access flags
	* are mapped to the appropriate open flags. Returns true if file was opened
	* successfully.
	*/
//=================================================================

//DECLARE_PERF_TIMER(StreamingArchiveFile)
Bool StreamingArchiveFile::open( const Char *filename, Int access )
{
	//USE_PERF_TIMER(StreamingArchiveFile)
	File *file = TheFileSystem->openFile( filename, access );

	if ( file == NULL )
	{
		return FALSE;
	}	

	return (open( file ) != NULL);
}

//============================================================================
// StreamingArchiveFile::open
//============================================================================

Bool StreamingArchiveFile::open( File *file )
{
	return TRUE;
}

//============================================================================
// StreamingArchiveFile::openFromArchive
//============================================================================
Bool StreamingArchiveFile::openFromArchive(File *archiveFile, const AsciiString& filename, Int offset, Int size) 
{
	//USE_PERF_TIMER(StreamingArchiveFile)
	if (archiveFile == NULL) {
		return FALSE;
	}

	if (File::open(filename.str(), File::READ | File::BINARY | File::STREAMING) == FALSE) {
		return FALSE;
	}

	m_file = archiveFile;
	m_startingPos = offset;
	m_size = size;
	m_curPos = 0;

	if (m_file->seek(offset, File::START) != offset) {
		return FALSE;
	}
	
	if (m_file->seek(size) != m_startingPos + size) {
		return FALSE;
	}

	// We know this will succeed.
	m_file->seek(offset, File::START);

	m_nameStr = filename;

	return TRUE;
}

//=================================================================
// StreamingArchiveFile::close 	
//=================================================================
/**
	* Closes the current file if it is open.
  * Must call StreamingArchiveFile::close() for each successful StreamingArchiveFile::open() call.
	*/
//=================================================================

void StreamingArchiveFile::close( void )
{
	File::close();
}

//=================================================================
// StreamingArchiveFile::read 
//=================================================================
// if buffer is null, just advance the current position by 'bytes'
Int StreamingArchiveFile::read( void *buffer, Int bytes )
{
	if (!m_file) {
		return 0;
	}

	// There shouldn't be a way that this can fail, because we've already verified that the file 
	// contains at least this many bits.
	m_file->seek(m_startingPos + m_curPos, File::START);

	if (bytes + m_curPos > m_size) 
		bytes = m_size - m_curPos;

	Int bytesRead = m_file->read(buffer, bytes);

	m_curPos += bytesRead;

	return bytesRead;
}

//=================================================================
// StreamingArchiveFile::write 
//=================================================================

Int StreamingArchiveFile::write( const void *buffer, Int bytes )
{
	DEBUG_CRASH(("Cannot write to streaming files."));
	return -1;
}

//=================================================================
// StreamingArchiveFile::seek 
//=================================================================

Int StreamingArchiveFile::seek( Int pos, seekMode mode)
{
	Int newPos;

	switch( mode )
	{
		case START:
			newPos = pos;
			break;
		case CURRENT:
			newPos = m_curPos + pos;
			break;
		case END:
			DEBUG_ASSERTCRASH(pos <= 0, ("StreamingArchiveFile::seek - position should be <= 0 for a seek starting from the end."));
			newPos = m_size + pos;
			break;
		default:
			// bad seek mode
			return -1;
	}

	if ( newPos < 0 )
	{
		newPos = 0;
	}
	else if ( newPos > m_size )
	{
		newPos = m_size;
	}

	m_curPos = newPos;

	return m_curPos;

}

