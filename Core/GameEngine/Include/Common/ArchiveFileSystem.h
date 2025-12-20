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

typedef std::map<AsciiString, DetailedArchivedDirectoryInfo> DetailedArchivedDirectoryInfoMap; // Archived directory name to detailed archived directory info
typedef std::map<AsciiString, ArchivedDirectoryInfo> ArchivedDirectoryInfoMap; // Archived directory name to archived directory info
typedef std::map<AsciiString, ArchivedFileInfo> ArchivedFileInfoMap; // Archived file name to archived file info
typedef std::map<AsciiString, ArchiveFile *> ArchiveFileMap; // Archive file name to archive data
typedef std::multimap<AsciiString, ArchiveFile *> ArchivedFileLocationMap; // Archived file name to archive data

class ArchivedDirectoryInfo
{
public:
	AsciiString								m_path; // The full path to this directory
	AsciiString								m_directoryName; // The current directory
	ArchivedDirectoryInfoMap	m_directories; // Contained leaf directories
	ArchivedFileLocationMap		m_files; // Contained files
};

class DetailedArchivedDirectoryInfo
{
public:
	AsciiString												m_directoryName;
	DetailedArchivedDirectoryInfoMap	m_directories;
	ArchivedFileInfoMap								m_files;
};

class ArchivedFileInfo
{
public:
	AsciiString m_filename;
	AsciiString m_archiveFilename;
	UnsignedInt m_offset;
	UnsignedInt m_size;

	ArchivedFileInfo()
		: m_offset(0)
		, m_size(0)
	{
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
	virtual void					closeAllArchiveFiles( void ) = 0;								///< Close all Archive files currently open

	// File operations
	virtual File*					openFile( const Char *filename, Int access = 0, FileInstance instance = 0);	///< Search Archive files for specified file name and open it if found
	virtual void					closeAllFiles( void ) = 0;									///< Close all files associated with Archive files
	virtual Bool					doesFileExist(const Char *filename, FileInstance instance = 0) const;	///< return true if that file exists in an archive file somewhere.

	void					getFileListInDirectory(const AsciiString& currentDirectory, const AsciiString& originalDirectory, const AsciiString& searchName, FilenameList &filenameList, Bool searchSubdirectories) const; ///< search the given directory for files matching the searchName (egs. *.ini, *.rep).  Possibly search subdirectories.  Scans each Archive file.
	Bool					getFileInfo(const AsciiString& filename, FileInfo *fileInfo, FileInstance instance = 0) const; ///< see FileSystem.h

	virtual Bool	loadBigFilesFromDirectory(AsciiString dir, AsciiString fileMask, Bool overwrite = FALSE) = 0;

	// Unprotected this for copy-protection routines
	ArchiveFile* getArchiveFile(const AsciiString& filename, FileInstance instance = 0) const;

	void loadMods( void );

	ArchivedDirectoryInfo* friend_getArchivedDirectoryInfo(const Char* directory);

protected:
	struct ArchivedDirectoryInfoResult
	{
		ArchivedDirectoryInfoResult() : dirInfo(NULL) {}
		Bool valid() const { return dirInfo != NULL; }

		ArchivedDirectoryInfo* dirInfo;
		AsciiString lastToken; ///< Synonymous for file name if the search directory was a file path
	};

	ArchivedDirectoryInfoResult getArchivedDirectoryInfo(const Char* directory);

	virtual void loadIntoDirectoryTree(ArchiveFile *archiveFile, Bool overwrite = FALSE);	///< load the archive file's header information and apply it to the global archive directory tree.

	ArchiveFileMap m_archiveFileMap;
	ArchivedDirectoryInfo m_rootDirectory;
};


extern ArchiveFileSystem *TheArchiveFileSystem;

//----------------------------------------------------------------------------
//           Inlining
//----------------------------------------------------------------------------
