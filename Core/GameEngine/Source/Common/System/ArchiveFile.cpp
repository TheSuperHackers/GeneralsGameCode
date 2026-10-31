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

/////////ArchiveFile.cpp ///////////////////////
// Bryan Cleveland, August 2002
////////////////////////////////////////////////

#include "PreRTS.h"

#include "Common/ArchiveFile.h"
#include "Common/ArchiveFileSystem.h"
#include "Common/LocalFileSystem.h"
#include "Common/file.h"
#include "Common/LocalFile.h"
#include "Common/PerfTimer.h"
#include "Common/RAMFile.h"
#include "Common/StreamingArchiveFile.h"


ArchiveFile::~ArchiveFile()
{
	if (m_file != nullptr) {
		m_file->close();
		m_file = nullptr;
	}
}

ArchiveFile::ArchiveFile(AsciiString name, AsciiString path)
	: m_file(nullptr)
	, m_name(name)
	, m_path(path)
{
}

void ArchiveFile::attachFile(File *file)
{
	if (m_file != nullptr) {
		m_file->close();
		m_file = nullptr;
	}

	m_file = file;

	if (m_file != nullptr) {
		if (!TheLocalFileSystem->getFileInfo(m_file->getName(), &m_fileInfo)) {
			DEBUG_CRASH(("ArchiveFile::attachFile - Cannot query file info for '%s' unexpectedly", m_file->getName()));
			m_fileInfo = FileInfo();
		}
	}
	else {
		m_fileInfo = FileInfo();
	}
}

//============================================================================
// ArchiveFile::openFile
//============================================================================

File* ArchiveFile::openFile( const ArchivedFileInfo& archivedFileInfo, const Char *filename, Int access )
{
	RAMFile *ramFile = nullptr;

	if (BitIsSet(access, File::STREAMING))
		ramFile = newInstance( StreamingArchiveFile );
	else
		ramFile = newInstance( RAMFile );

	ramFile->deleteOnClose();
	if (ramFile->openFromArchive(m_file, archivedFileInfo.m_filename, archivedFileInfo.m_offset, archivedFileInfo.m_size) == FALSE) {
		ramFile->close();
		ramFile = nullptr;
		return nullptr;
	}

	if ((access & File::WRITE) == 0) {
		// requesting read only access. Just return the RAM file.
		return ramFile;
	}

	// whoever is opening this file wants write access, so copy the file to the local disk
	// and return that file pointer.

	constexpr size_t bufferSize = 0;
	File *localFile = TheLocalFileSystem->openFile(filename, access, bufferSize);
	if (localFile != nullptr) {
		ramFile->copyDataToFile(localFile);
	}

	ramFile->close();
	ramFile = nullptr;

	return localFile;
}

//============================================================================
// ArchiveFile::closeAllFiles
//============================================================================

void ArchiveFile::closeAllFiles()
{

}

//============================================================================
// ArchiveFile::getName
//============================================================================

AsciiString ArchiveFile::getName()
{
	return m_name;
}

//============================================================================
// ArchiveFile::getPath
//============================================================================

AsciiString ArchiveFile::getPath()
{
	return m_path;
}

//============================================================================
// ArchiveFile::setSearchPriority
//============================================================================

void ArchiveFile::setSearchPriority( Int new_priority )
{

}

//============================================================================
// ArchiveFile::close
//============================================================================

void ArchiveFile::close()
{

}

//============================================================================
// ArchiveFile::getFileInfo
//============================================================================

Bool ArchiveFile::getFileInfo(const ArchivedFileInfo& archivedFileInfo, FileInfo *fileInfo) const
{
	// fill in the size info.  Since the size can't be bigger than a JUNK file, the high int will always be 0.
	fileInfo->sizeHigh = 0;
	fileInfo->sizeLow = archivedFileInfo.m_size;
	// the timestamp is taken from the archive file.
	fileInfo->timestampHigh = m_fileInfo.timestampHigh;
	fileInfo->timestampLow = m_fileInfo.timestampLow;

	return TRUE;
}
