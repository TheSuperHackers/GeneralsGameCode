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

/////// ArchiveFile.h ////////////////////
// Bryan Cleveland, August 2002
//////////////////////////////////////////

#pragma once

#include "Common/AsciiString.h"
#include "Common/ArchiveFileSystem.h"

class File;

/**
  *	An archive file is itself a collection of sub files. Each file inside the archive file
	* has a unique name by which it can be accessed. The ArchiveFile object class is the
	* runtime interface to the mix file and the sub files. Each file inside the mix
	* file can be accessed by the openFile().
	*
	* ArchiveFile interfaces can be created by the TheArchiveFileSystem object.
	*/
//===============================

class ArchiveFile
{
public:
	ArchiveFile(AsciiString name, AsciiString path);
	~ArchiveFile();

	Bool					getFileInfo( const ArchivedFileInfo& archivedFileInfo, FileInfo *fileInfo) const;	///< fill in the fileInfo struct with info about the file requested.
	File*					openFile( const ArchivedFileInfo& archivedFileInfo, const Char *filename, Int access = 0);	///< Open the specified file within the archive file
	void					closeAllFiles();									///< Close all file opened in this archive file
	AsciiString		getName();												///< Returns the name of the archive file
	AsciiString		getPath();												///< Returns full path and name of archive file
	void					setSearchPriority( Int new_priority );	///< Set this archive file's search priority
	void					close();													///< Close this archive file
	void					attachFile(File *file);

protected:
	File *m_file; ///< file pointer to the archive file on disk.  Kept open so we don't have to continuously open and close the file all the time.
	FileInfo m_fileInfo; ///< file info of the archive file on disk.
	AsciiString m_name; ///< archive file name
	AsciiString m_path; ///< archive file path
};
