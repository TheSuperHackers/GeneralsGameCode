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

////// Win32BIGFile.cpp /////////////////////////
// Bryan Cleveland, August 2002
/////////////////////////////////////////////////////

#include "Common/LocalFile.h"
#include "Common/LocalFileSystem.h"
#include "Common/RAMFile.h"
#include "Common/StreamingArchiveFile.h"
#include "Common/GameMemory.h"
#include "Common/PerfTimer.h"
#include "Win32Device/Common/Win32BIGFile.h"

//============================================================================
// Win32BIGFile::Win32BIGFile
//============================================================================

Win32BIGFile::Win32BIGFile()
{

}

//============================================================================
// Win32BIGFile::~Win32BIGFile
//============================================================================

Win32BIGFile::~Win32BIGFile()
{

}

//============================================================================
// Win32BIGFile::openFile
//============================================================================

File* Win32BIGFile::openFile( const Char *filename, Int access ) 
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

	File *localFile = TheLocalFileSystem->openFile(filename, access);
	if (localFile != NULL) {
		ramFile->copyDataToFile(localFile);
	}

	ramFile->close();
	ramFile = NULL;

	return localFile;
}

//============================================================================
// Win32BIGFile::closeAllFiles
//============================================================================

void Win32BIGFile::closeAllFiles( void )
{

}

//============================================================================
// Win32BIGFile::getName
//============================================================================

AsciiString Win32BIGFile::getName( void )
{
	return m_name;
}

//============================================================================
// Win32BIGFile::getPath
//============================================================================

AsciiString Win32BIGFile::getPath( void )
{
	return m_path;
}

//============================================================================
// Win32BIGFile::setSearchPriority
//============================================================================

void Win32BIGFile::setSearchPriority( Int new_priority )
{

}

//============================================================================
// Win32BIGFile::close
//============================================================================

void Win32BIGFile::close( void )
{

}

//============================================================================
// Win32BIGFile::getFileInfo
//============================================================================

Bool Win32BIGFile::getFileInfo(const AsciiString& filename, FileInfo *fileInfo) const 
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

