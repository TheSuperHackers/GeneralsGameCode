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

////// StdBIGFile.cpp /////////////////////////
// Stephan Vedder, April 2025
/////////////////////////////////////////////////////

#include "Common/LocalFile.h"
#include "Common/LocalFileSystem.h"
#include "Common/RAMFile.h"
#include "Common/StreamingArchiveFile.h"
#include "Common/GameMemory.h"
#include "Common/PerfTimer.h"
#include "StdDevice/Common/StdBIGFile.h"

//============================================================================
// StdBIGFile::StdBIGFile
//============================================================================

StdBIGFile::StdBIGFile()
{

}

//============================================================================
// StdBIGFile::~StdBIGFile
//============================================================================

StdBIGFile::~StdBIGFile()
{

}

//============================================================================
// StdBIGFile::openFile
//============================================================================

File* StdBIGFile::openFile( const Char *filename, Int access ) 
{
	const ArchivedFileInfo *fileInfo = getArchivedFileInfo(AsciiString(filename));

	if (fileInfo == NULL) {
		return NULL;
	}

	RAMFile *ramFile = NULL;
	
	if (BitIsSet(access, File::STREAMING)) 
		ramFile = newInstance( StreamingArchiveFile );
	else 
		ramFile = newInstance( RAMFile );

	ramFile->deleteOnClose();
	if (ramFile->openFromArchive(m_file, fileInfo->m_filename, fileInfo->m_offset, fileInfo->m_size) == FALSE) {
		ramFile->close();
		ramFile = NULL;
		return NULL;
	}

	if ((access & File::WRITE) == 0) {
		// requesting read only access. Just return the RAM file.
		return ramFile;
	}

	// whoever is opening this file wants write access, so copy the file to the local disk
	// and return that file pointer.

	CONSTEXPR size_t bufferSize = 0;
	File *localFile = TheLocalFileSystem->openFile(filename, access, bufferSize);
	if (localFile != NULL) {
		ramFile->copyDataToFile(localFile);
	}

	ramFile->close();
	ramFile = NULL;

	return localFile;
}

//============================================================================
// StdBIGFile::closeAllFiles
//============================================================================

void StdBIGFile::closeAllFiles( void )
{

}

//============================================================================
// StdBIGFile::getName
//============================================================================

AsciiString StdBIGFile::getName( void )
{
	return m_name;
}

//============================================================================
// StdBIGFile::getPath
//============================================================================

AsciiString StdBIGFile::getPath( void )
{
	return m_path;
}

//============================================================================
// StdBIGFile::setSearchPriority
//============================================================================

void StdBIGFile::setSearchPriority( Int new_priority )
{

}

//============================================================================
// StdBIGFile::close
//============================================================================

void StdBIGFile::close( void )
{

}

//============================================================================
// StdBIGFile::getFileInfo
//============================================================================

Bool StdBIGFile::getFileInfo(const AsciiString& filename, FileInfo *fileInfo) const 
{
	const ArchivedFileInfo *tempFileInfo = getArchivedFileInfo(filename);

	if (tempFileInfo == NULL) {
		return FALSE;
	}

	TheLocalFileSystem->getFileInfo(AsciiString(m_file->getName()), fileInfo);

	// fill in the size info.  Since the size can't be bigger than a JUNK file, the high Int will always be 0.
	fileInfo->sizeHigh = 0;
	fileInfo->sizeLow = tempFileInfo->m_size;

	return TRUE;
}

