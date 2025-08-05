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
//                Copyright (C) 2001 - All Rights Reserved
//
//----------------------------------------------------------------------------
//
// Project:    Generals
//
// Module:     Archive files
//
// File name:  Common/ArchiveFileSystem.h
//
// Created:    11/26/01 TR
//
//----------------------------------------------------------------------------

#pragma once

#ifndef __ARCHIVEFILESYSTEM_H_
#define __ARCHIVEFILESYSTEM_H_

#define MUSIC_BIG "Music.big"

//----------------------------------------------------------------------------
//           Includes
//----------------------------------------------------------------------------

#include "Common/SubsystemInterface.h"
#include "Common/AsciiString.h"
#include "Common/FileSystem.h" // for typedefs, etc.
#include "Common/STLTypedefs.h"

//----------------------------------------------------------------------------
//           Forward References
//----------------------------------------------------------------------------

class File;
class ArchiveFile;

//----------------------------------------------------------------------------
//           Type Defines
//----------------------------------------------------------------------------


//===============================
// ArchiveFileSystem
//===============================
/**
  *	Creates and manages ArchiveFile interfaces. ArchiveFiles can be accessed
	* by calling the openArchiveFile() member. ArchiveFiles can be accessed by
	* name or by File interface.
	*
	* openFile() member searches all Archive files for the specified sub file.
	*/
//===============================
class ArchivedDirectoryInfo;
class DetailedArchivedDirectoryInfo;
class ArchivedFileInfo;

typedef std::map<AsciiString, DetailedArchivedDirectoryInfo> DetailedArchivedDirectoryInfoMap;
typedef std::map<AsciiString, ArchivedDirectoryInfo> ArchivedDirectoryInfoMap;
typedef std::map<AsciiString, ArchivedFileInfo> ArchivedFileInfoMap;
typedef std::map<AsciiString, ArchiveFile *> ArchiveFileMap;
typedef std::map<AsciiString, AsciiString> ArchivedFileLocationMap; // first string is the file name, second one is the archive filename.

class ArchivedDirectoryInfo
{
public:
	AsciiString								m_directoryName;
	ArchivedDirectoryInfoMap	m_directories;
	ArchivedFileLocationMap		m_files;

	void clear()
	{
		m_directoryName.clear();
		m_directories.clear();
		m_files.clear();
	}

};

class DetailedArchivedDirectoryInfo
{
public:
	AsciiString												m_directoryName;
	DetailedArchivedDirectoryInfoMap	m_directories;
	ArchivedFileInfoMap								m_files;

	void clear()
	{
		m_directoryName.clear();
		m_directories.clear();
		m_files.clear();
	}
};

class ArchivedFileInfo
{
public:
	AsciiString m_filename;
	AsciiString m_archiveFilename;
	UnsignedInt m_offset;
	UnsignedInt m_size;

	ArchivedFileInfo()
	{
		clear();
	}

	void clear()
	{
		m_filename.clear();
		m_archiveFilename.clear();
		m_offset = 0;
		m_size = 0;
	}
};


class ArchiveFileSystem : public SubsystemInterface
{
	public:
	ArchiveFileSystem();
	virtual ~ArchiveFileSystem();

	virtual void init( void ) = 0;
	virtual void update( void ) = 0;
	virtual void reset( void ) = 0;
	virtual void postProcessLoad( void ) = 0;

	// ArchiveFile operations
	virtual ArchiveFile*	openArchiveFile( const Char *filename ) = 0;		///< Create new or return existing Archive file from file name
	virtual void					closeArchiveFile( const Char *filename ) = 0;		///< Close the one specified big file.
	virtual void					closeAllArchiveFiles( void ) = 0;								///< Close all Archivefiles currently open

	// File operations
	virtual File*					openFile( const Char *filename, Int access = 0);	///< Search Archive files for specified file name and open it if found
	virtual void					closeAllFiles( void ) = 0;									///< Close all files associated with ArchiveFiles
	virtual Bool					doesFileExist(const Char *filename) const;		///< return true if that file exists in an archive file somewhere.

	void					getFileListInDirectory(const AsciiString& currentDirectory, const AsciiString& originalDirectory, const AsciiString& searchName, FilenameList &filenameList, Bool searchSubdirectories) const; ///< search the given directory for files matching the searchName (egs. *.ini, *.rep).  Possibly search subdirectories.  Scans each Archive file.
	Bool					getFileInfo(const AsciiString& filename, FileInfo *fileInfo) const; ///< see FileSystem.h

	virtual Bool	loadBigFilesFromDirectory(AsciiString dir, AsciiString fileMask, Bool overwrite = FALSE) = 0;

	// Unprotected this for copy-protection routines
	AsciiString						getArchiveFilenameForFile(const AsciiString& filename) const;
	void loadMods( void );

protected:
	virtual void					loadIntoDirectoryTree(const ArchiveFile *archiveFile, const AsciiString& archiveFilename, Bool overwrite = FALSE );	///< load the archive file's header information and apply it to the global archive directory tree.

	ArchiveFileMap m_archiveFileMap;
	ArchivedDirectoryInfo m_rootDirectory;
};


extern ArchiveFileSystem *TheArchiveFileSystem;

//----------------------------------------------------------------------------
//           Inlining
//----------------------------------------------------------------------------

#endif // __ARCHIVEFILESYSTEM_H_
