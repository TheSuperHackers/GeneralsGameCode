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

#include <Utility/STLUtils.h>

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
	ArchiveFile* m_archiveFile; // The archive file that contains this file

	ArchivedFileInfo()
		: m_offset(0)
		, m_size(0)
		, m_archiveFile(nullptr)
	{
	}
};

// TheSuperHackers @refactor All archived files are held in one map that is sorted by the path key of the file.
// Files of a directory are therefore neighbors. So are files with the same path in different archive files,
// which are ordered by priority. The first one is file instance 0.
typedef rts::string_key<AsciiString> ArchivedFileKey;
typedef std::multimap<ArchivedFileKey, ArchivedFileInfo, rts::string_key_less> ArchivedFileMap; // Path key of archived file to archived file info


class ArchiveFileSystem : public SubsystemInterface
{
public:
	ArchiveFileSystem();
	virtual ~ArchiveFileSystem() override;

	virtual void postProcessLoad() override = 0;

	// ArchiveFile operations
	virtual ArchiveFile*	openArchiveFile( const Char *filename, Bool overwrite = FALSE ) = 0;		///< Create new Archive file from file name and add its files. Its files take priority over the same files of other Archive files if overwrite is set.
	virtual void					closeArchiveFile( const Char *filename ) = 0;		///< Close the one specified big file.
	virtual void					closeAllArchiveFiles() = 0;								///< Close all Archive files currently open

	// File operations
	virtual File*					openFile( const Char *filename, Int access = 0, FileInstance instance = 0);	///< Search Archive files for specified file name and open it if found
	virtual void					closeAllFiles() = 0;									///< Close all files associated with Archive files
	virtual Bool					doesFileExist(const Char *filename, FileInstance instance = 0) const;	///< return true if that file exists in an archive file somewhere.

	void					getFileListInDirectory(const AsciiString& currentDirectory, const AsciiString& originalDirectory, const AsciiString& searchName, FilenameList &filenameList, Bool searchSubdirectories) const; ///< search the given directory for files matching the searchName (egs. *.ini, *.rep).  Possibly search subdirectories.  Scans each Archive file.
	Bool					getFileInfo(const AsciiString& filename, FileInfo *fileInfo, FileInstance instance = 0) const; ///< see FileSystem.h

	virtual Bool	loadArchiveFilesFromDirectory(AsciiString dir, AsciiString fileMask, Bool overwrite = FALSE) = 0;

	virtual void loadMods() = 0;

	ArchivedDirectoryInfo* friend_getArchivedDirectoryInfo(const Char* directory);
	stl::range<ArchivedFileMap> friend_getArchivedFiles(const Char* directory); ///< returns all archived files in the directory and its subdirectories.

protected:
	struct ArchivedDirectoryInfoResult
	{
		ArchivedDirectoryInfoResult() : dirInfo(nullptr) {}
		Bool valid() const { return dirInfo != nullptr; }

		ArchivedDirectoryInfo* dirInfo;
		AsciiString lastToken; ///< Synonymous for file name if the search directory was a file path
	};

	ArchiveFile* getArchiveFile(const AsciiString& filename, FileInstance instance = 0) const;
	ArchivedDirectoryInfoResult getArchivedDirectoryInfo(const Char* directory);

	virtual void loadIntoDirectoryTree(ArchiveFile *archiveFile, Bool overwrite = FALSE);	///< load the archive file's header information and apply it to the global archive directory tree.

	void addFile(ArchiveFile *archiveFile, const AsciiString& path, const ArchivedFileInfo& fileInfo, Bool overwrite); ///< add a file of the Archive file. path is the full path of the file inside the Archive file.
	void removeFiles(const ArchiveFile *archiveFile); ///< remove all files of the Archive file.
	const ArchivedFileInfo* findFile(const Char *filename, FileInstance instance) const;

	ArchiveFileMap m_archiveFileMap;
	ArchivedDirectoryInfo m_rootDirectory;
	ArchivedFileMap m_files;
};


extern ArchiveFileSystem *TheArchiveFileSystem;

//----------------------------------------------------------------------------
//           Inlining
//----------------------------------------------------------------------------
