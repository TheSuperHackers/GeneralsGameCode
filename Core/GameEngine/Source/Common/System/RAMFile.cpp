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

#include "PreRTS.h"

#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include <sys/stat.h>

#include "Common/AsciiString.h"
#include "Common/FileSystem.h"
#include "Common/RAMFile.h"
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
// RAMFile::RAMFile
//=================================================================

RAMFile::RAMFile()
: m_size(0),
	m_data(NULL),
//Added By Sadullah Nader
//Initializtion(s) inserted
	m_pos(0)
//
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
	closeFile();
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

//DECLARE_PERF_TIMER(RAMFile)
Bool RAMFile::open( const Char *filename, Int access )
{
	//USE_PERF_TIMER(RAMFile)
	File *file = TheFileSystem->openFile( filename, access );

	if ( file == NULL )
	{
		return FALSE;
	}	

	Bool result = open( file );

	file->close();

	return result;
}

//============================================================================
// RAMFile::open
//============================================================================

Bool RAMFile::open( File *file )
{
	//USE_PERF_TIMER(RAMFile)
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
	m_data = MSGNEW("RAMFILE") char [ m_size ];	// pool[]ify

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

//============================================================================
// RAMFile::openFromArchive
//============================================================================
Bool RAMFile::openFromArchive(File *archiveFile, const AsciiString& filename, Int offset, Int size) 
{
	//USE_PERF_TIMER(RAMFile)
	if (archiveFile == NULL) {
		return FALSE;
	}

	if (File::open(filename.str(), File::READ | File::BINARY) == FALSE) {
		return FALSE;
	}

	if (m_data != NULL) {
		delete[] m_data;
		m_data = NULL;
	}
	m_data = MSGNEW("RAMFILE") Char [size];	// pool[]ify
	m_size = size;

	if (archiveFile->seek(offset, File::START) != offset) {
		return FALSE;
	}
	if (archiveFile->read(m_data, size) != size) {
		return FALSE;
	}
	m_nameStr = filename;

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
	closeFile();
	File::close();
}

//=================================================================

void RAMFile::closeFile()
{
	delete [] m_data;
	m_data = NULL;
}

//=================================================================
// RAMFile::read 
//=================================================================
// if buffer is null, just advance the current position by 'bytes'
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

	if (( bytes > 0 ) && ( buffer != NULL ))
	{
		memcpy ( buffer, &m_data[m_pos], bytes );
	}

	m_pos += bytes;

	return bytes;
}

//=================================================================
// RAMFile::write 
//=================================================================

Int RAMFile::write( const void *buffer, Int bytes )
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
			DEBUG_ASSERTCRASH(pos <= 0, ("RAMFile::seek - position should be <= 0 for a seek starting from the end."));
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

	m_pos = newPos;

	return m_pos;

}

//=================================================================
// RAMFile::scanInt
//=================================================================
Bool RAMFile::scanInt(Int &newInt) 
{
	newInt = 0;
	AsciiString tempstr;

	while ((m_pos < m_size) && ((m_data[m_pos] < '0') || (m_data[m_pos] > '9')) && (m_data[m_pos] != '-')) {
		++m_pos;
	}

	if (m_pos >= m_size) {
		m_pos = m_size;
		return FALSE;
	}

	do {
		tempstr.concat(m_data[m_pos]);
		++m_pos;
	} while ((m_pos < m_size) && ((m_data[m_pos] >= '0') && (m_data[m_pos] <= '9')));

//	if (m_pos < m_size) {
//		--m_pos;
//	}

	newInt = atoi(tempstr.str());
	return TRUE;
}

//=================================================================
// RAMFile::scanInt
//=================================================================
Bool RAMFile::scanReal(Real &newReal) 
{
	newReal = 0.0;
	AsciiString tempstr;
	Bool sawDec = FALSE;

	while ((m_pos < m_size) && ((m_data[m_pos] < '0') || (m_data[m_pos] > '9')) && (m_data[m_pos] != '-') && (m_data[m_pos] != '.')) {
		++m_pos;
	}

	if (m_pos >= m_size) {
		m_pos = m_size;
		return FALSE;
	}

	do {
		tempstr.concat(m_data[m_pos]);
		if (m_data[m_pos] == '.') {
			sawDec = TRUE;
		}
		++m_pos;
	} while ((m_pos < m_size) && (((m_data[m_pos] >= '0') && (m_data[m_pos] <= '9')) || ((m_data[m_pos] == '.') && !sawDec)));

//	if (m_pos < m_size) {
//		--m_pos;
//	}

	newReal = atof(tempstr.str());
	return TRUE;
}

//=================================================================
// RAMFile::scanString
//=================================================================
Bool RAMFile::scanString(AsciiString &newString) 
{
	newString.clear();

	while ((m_pos < m_size) && isspace(m_data[m_pos])) {
		++m_pos;
	}

	if (m_pos >= m_size) {
		m_pos = m_size;
		return FALSE;
	}

	do {
		newString.concat(m_data[m_pos]);
		++m_pos;
	} while ((m_pos < m_size) && (!isspace(m_data[m_pos])));

	return TRUE;
}

//=================================================================
// RAMFile::nextLine
//=================================================================
void RAMFile::nextLine(Char *buf, Int bufSize) 
{
	Int i = 0;
	// seek to the next new-line character
	while ((m_pos < m_size) && (m_data[m_pos] != '\n')) {
		if ((buf != NULL) && (i < (bufSize-1))) {
			buf[i] = m_data[m_pos];
			++i;
		}
		++m_pos;
	}

	// we got to the new-line character, now go one past it.
	if (m_pos < m_size) {
		if ((buf != NULL) && (i < bufSize)) {
			buf[i] = m_data[m_pos];
			++i;
		}
		++m_pos;
	}
	if (buf != NULL) {
		if (i < bufSize) {
			buf[i] = 0;
		} else {
			buf[bufSize] = 0;
		}
	}
	if (m_pos >= m_size) {
		m_pos = m_size;
	}
}

//=================================================================
// RAMFile::nextLine
//=================================================================
Bool RAMFile::copyDataToFile(File *localFile) 
{
	if (localFile == NULL) {
		return FALSE;
	}

	if (localFile->write(m_data, m_size) == m_size) {
		return TRUE;
	}


	return FALSE;
}

//=================================================================
//=================================================================
File* RAMFile::convertToRAMFile()
{
	return this;
}

//=================================================================
// RAMFile::readEntireAndClose
//=================================================================
/**
	Allocate a buffer large enough to hold entire file, read 
	the entire file into the buffer, then close the file.
	the buffer is owned by the caller, who is responsible
	for freeing is (via delete[]). This is a Good Thing to
	use because it minimizes memory copies for BIG files.
*/
char* RAMFile::readEntireAndClose()
{

	if (m_data == NULL)
	{
		DEBUG_CRASH(("m_data is NULL in RAMFile::readEntireAndClose -- should not happen!"));
		return NEW char[1];	// just to avoid crashing...
	}

	char* tmp = m_data;
	m_data = NULL;	// will belong to our caller!

	close();

	return tmp;
}
