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

/////// LocalFileSystem.h ////////////////////////////////
// Bryan Cleveland, August 2002
//////////////////////////////////////////////////////////

#pragma once

#include "Common/SubsystemInterface.h"
#include "FileSystem.h" // for typedefs, etc.

class LocalFileSystem : public SubsystemInterface
{
	struct IgnoreFileData
	{
	};

	typedef std::hash_map<
		rts::string_key<AsciiString>, IgnoreFileData,
		rts::string_key_hash_path,
		rts::string_key_equal_to_path> IgnoreFileHashMap;

protected:
	typedef Int IgnoreFileTestFlags;
	enum IgnoreFileTestFlags_
	{
		IgnoreFileTestFlags_SkipParentDirectories = 1 << 0,
	};

public:
	virtual ~LocalFileSystem() override {}

	virtual File * openFile(const Char *filename, Int access = File::NONE, size_t bufferSize = File::BUFFERSIZE) = 0;
	virtual Bool doesFileExist(const Char *filename) const = 0;
	virtual void getFileListInDirectory(const AsciiString& currentDirectory, const AsciiString& originalDirectory, const AsciiString& searchName, FilenameList &filenameList, Bool searchSubdirectories) const = 0; ///< search the given directory for files matching the searchName (egs. *.ini, *.rep).  Possibly search subdirectories.
	virtual Bool getFileInfo(const AsciiString& filename, FileInfo *fileInfo) const = 0; ///< see FileSystem.h
	virtual Bool createDirectory(AsciiString directory) = 0; ///< see FileSystem.h
	virtual AsciiString normalizePath(const AsciiString& filePath) const = 0;	///< see FileSystem.h

	Bool ignoreFile(const AsciiString& filename, Bool ignore = true); ///< Ignore this file on the disk.
	Bool ignoreDirectory(const AsciiString& directory, Bool ignore = true); ///< Ignore this directory and all its contents on the disk.

protected:
	Bool isFileIgnored(const Char* filename, IgnoreFileTestFlags flags = 0) const; ///< Whether the given file or its parent directories are ignored.
	Bool isDirectoryIgnored(const Char* directory, IgnoreFileTestFlags flags = 0) const; ///< Whether the given directory or its parent directories are ignored.

private:
	Bool isDirectoryIgnoredRecursive(AsciiString& directory, IgnoreFileTestFlags flags) const;
	Bool isFileIgnoredInternal(const Char* filename) const;
	Bool isDirectoryIgnoredInternal(const Char* directory) const;
	Bool hasIgnoredFile() const;
	Bool hasIgnoredDirectory() const;

	static void trimLastPathSegmentInplace(AsciiString& path); ///< Removes trailing path segment until and including the last slash.
	static void trimTrailingSlash(AsciiString& path); ///< Remove trailing forward or backward slashes.

	IgnoreFileHashMap m_ignoreFileHashMap;
	IgnoreFileHashMap m_ignoreDirectoryHashMap;
	mutable FastCriticalSectionClass m_ignoreFileMutex;
	mutable FastCriticalSectionClass m_ignoreDirectoryMutex;
};

extern LocalFileSystem *TheLocalFileSystem;
