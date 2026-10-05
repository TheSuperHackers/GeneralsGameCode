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
// Project:   Generals
//
// Module:    Game Engine Common
//
// File name: ArchiveFileSystem.cpp
//
// Created:   11/26/01 TR
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//         Includes
//----------------------------------------------------------------------------

#include "PreRTS.h"
#include "Common/ArchiveFile.h"
#include "Common/ArchiveFileSystem.h"
#include "Common/AsciiString.h"
#include "Common/PerfTimer.h"

#include "Lib/PathUtil.h"


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

ArchiveFileSystem *TheArchiveFileSystem = nullptr;


//----------------------------------------------------------------------------
//         Private Prototypes
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Functions
//----------------------------------------------------------------------------

// checks to see if str matches searchString.  Search string is done in the
// using * and ? as wildcards. * is used to denote any number of characters,
// and ? is used to denote a single wildcard character.
static Bool SearchStringMatches(AsciiString str, AsciiString searchString)
{
	if (str.isEmpty()) {
		if (searchString.isEmpty()) {
			return TRUE;
		}
		return FALSE;
	}
	if (searchString.isEmpty()) {
		return FALSE;
	}

	const char *c1 = str.str();
	const char *c2 = searchString.str();

	while ((*c1 == *c2) || (*c2 == '?') || (*c2 == '*')) {
		if ((*c1 == *c2) || (*c2 == '?')) {
			++c1;
			++c2;
		} else if (*c2 == '*') {
			++c2;
			if (*c2 == 0) {
				return TRUE;
			}
			while (*c1 != 0) {
				if (SearchStringMatches(AsciiString(c1), AsciiString(c2))) {
					return TRUE;
				}
				++c1;
			}
		}
		if (*c1 == 0) {
			if (*c2 == 0) {
				return TRUE;
			}
			return FALSE;
		}
		if (*c2 == 0) {
			return FALSE;
		}
	}
	return FALSE;
}

// Appends a backslash unless the key is empty or has one. Returns the new length, or -1 if it does not fit.
static Int ensureTrailingArchivePathSeparator(Char* key, size_t keySize)
{
	size_t length = strlen(key);

	if (length == 0 || key[length - 1] == '\\')
		return (Int)length;

	if (length + 1 >= keySize)
		return -1;

	key[length++] = '\\';
	key[length] = '\0';
	return (Int)length;
}

// Writes the path key of an archived file. Returns FALSE if it does not fit.
// Separators in front of a path have no meaning inside an archive file.
static Bool makeArchivedFileKey(Char* key, size_t keySize, const Char* path)
{
	while (isPathSeparator(*path))
		++path;

	return FileSystem::makePathKey(key, keySize, path);
}

// Writes the key that all archived files in a directory and its subdirectories begin with.
// Returns the length of the key, or -1 if it does not fit.
static Int makeArchivedDirectoryKey(Char* key, size_t keySize, const Char* directory)
{
	if (!makeArchivedFileKey(key, keySize, directory))
		return -1;

	return ensureTrailingArchivePathSeparator(key, keySize);
}


//----------------------------------------------------------------------------
//         Public Functions
//----------------------------------------------------------------------------

//------------------------------------------------------
// ArchivedFileInfo
//------------------------------------------------------
ArchiveFileSystem::ArchiveFileSystem()
{
}

ArchiveFileSystem::~ArchiveFileSystem()
{
	ArchiveFileMap::iterator iter = m_archiveFileMap.begin();
	while (iter != m_archiveFileMap.end()) {
		ArchiveFile *file = iter->second;
		delete file;
		iter++;
	}
}

Bool ArchiveFileSystem::doesFileExist(const Char *filename, FileInstance instance) const
{
	return findFile(filename, instance) != nullptr;
}

File * ArchiveFileSystem::openFile(const Char *filename, Int access, FileInstance instance)
{
	const ArchivedFileInfo* archivedFileInfo = findFile(filename, instance);

	if (archivedFileInfo == nullptr)
		return nullptr;

	return archivedFileInfo->m_archiveFile->openFile(*archivedFileInfo, filename, access);
}

Bool ArchiveFileSystem::getFileInfo(const AsciiString& filename, FileInfo *fileInfo, FileInstance instance) const
{
	if (fileInfo == nullptr) {
		return FALSE;
	}

	const ArchivedFileInfo* archivedFileInfo = findFile(filename.str(), instance);

	if (archivedFileInfo == nullptr)
		return FALSE;

	return archivedFileInfo->m_archiveFile->getFileInfo(*archivedFileInfo, fileInfo);
}

stl::range<ArchivedFileMap> ArchiveFileSystem::friend_getArchivedFiles(const Char* directory)
{
	Char directoryKey[_MAX_PATH];
	const Int directoryKeyLength = makeArchivedDirectoryKey(directoryKey, ARRAY_SIZE(directoryKey), directory);

	if (directoryKeyLength < 0)
		return stl::range<ArchivedFileMap>(m_files.end(), m_files.end());

	const ArchivedFileMap::iterator begin = m_files.lower_bound(ArchivedFileKey::temporary(directoryKey));
	ArchivedFileMap::iterator end = begin;

	while (end != m_files.end() && strncmp(end->first.c_str(), directoryKey, directoryKeyLength) == 0)
		++end;

	return stl::range<ArchivedFileMap>(begin, end);
}

void ArchiveFileSystem::getFileListInDirectory(const AsciiString& currentDirectory, const AsciiString& originalDirectory, const AsciiString& searchName, FilenameList &filenameList, Bool searchSubdirectories) const
{
	Char directoryKey[_MAX_PATH];
	const Int directoryKeyLength = makeArchivedDirectoryKey(directoryKey, ARRAY_SIZE(directoryKey), originalDirectory.str());

	if (directoryKeyLength < 0)
		return;

	AsciiString directory = originalDirectory;
	if (directory.isNotEmpty() && !isPathSeparator(directory.back()))
		directory.concat('\\');

	ArchivedFileMap::const_iterator it = m_files.lower_bound(ArchivedFileKey::temporary(directoryKey));

	for (; it != m_files.end(); ++it)
	{
		const Char* path = it->first.c_str();

		if (strncmp(path, directoryKey, directoryKeyLength) != 0)
			break;

		if (SearchStringMatches(getFileName(path), searchName))
		{
			// The path is listed with the directory as the caller wrote it.
			AsciiString filename = directory;
			filename.concat(path + directoryKeyLength);
			filenameList.insert(filename);
		}
	}
}

void ArchiveFileSystem::addFile(ArchiveFile *archiveFile, const AsciiString& path, const ArchivedFileInfo& fileInfo, Bool overwrite)
{
	Char keyBuffer[_MAX_PATH];

	if (!makeArchivedFileKey(keyBuffer, ARRAY_SIZE(keyBuffer), path.str()))
	{
		DEBUG_CRASH(("ArchiveFileSystem::addFile - path of file %s in %s is too long", path.str(), archiveFile->getName().str()));
		return;
	}

	const ArchivedFileKey key(keyBuffer);
	const std::pair<ArchivedFileMap::iterator, ArchivedFileMap::iterator> files = m_files.equal_range(key);

	// An archive file contains unique paths. If it lists the same path again, then the last one counts.
	for (ArchivedFileMap::iterator it = files.first; it != files.second; ++it)
	{
		if (it->second.m_archiveFile == archiveFile)
		{
			it->second = fileInfo;
			it->second.m_archiveFile = archiveFile;
			return;
		}
	}

	// Files with the same path are ordered by priority. Overwriting places the new file in front of the others.
	const ArchivedFileMap::iterator file = m_files.insert(overwrite ? files.first : files.second, ArchivedFileMap::value_type(key, fileInfo));
	file->second.m_archiveFile = archiveFile;

#if defined(DEBUG_LOGGING) && ENABLE_FILESYSTEM_LOGGING
	{
		ArchivedFileMap::const_iterator other = file;

		if (overwrite && ++other != m_files.end() && strcmp(other->first.c_str(), keyBuffer) == 0)
		{
			DEBUG_LOG(("ArchiveFileSystem::addFile - adding file %s, archived in %s, overwriting same file in %s",
				keyBuffer,
				archiveFile->getName().str(),
				other->second.m_archiveFile->getName().str()
			));
		}
		else if (!overwrite && other != m_files.begin() && strcmp((--other)->first.c_str(), keyBuffer) == 0)
		{
			DEBUG_LOG(("ArchiveFileSystem::addFile - adding file %s, archived in %s, overwritten by same file in %s",
				keyBuffer,
				archiveFile->getName().str(),
				other->second.m_archiveFile->getName().str()
			));
		}
		else
		{
			DEBUG_LOG(("ArchiveFileSystem::addFile - adding file %s, archived in %s", keyBuffer, archiveFile->getName().str()));
		}
	}
#endif
}

void ArchiveFileSystem::removeFiles(const ArchiveFile *archiveFile)
{
	ArchivedFileMap::iterator it = m_files.begin();

	while (it != m_files.end())
	{
		ArchivedFileMap::iterator next = it;
		++next;

		if (it->second.m_archiveFile == archiveFile)
			m_files.erase(it);

		it = next;
	}
}

const ArchivedFileInfo* ArchiveFileSystem::findFile(const Char *filename, FileInstance instance) const
{
	Char key[_MAX_PATH];

	if (!makeArchivedFileKey(key, ARRAY_SIZE(key), filename))
		return nullptr;

	const stl::const_range<ArchivedFileMap> range = stl::get_range(m_files, ArchivedFileKey::temporary(key), instance);

	if (!range.valid())
		return nullptr;

	return &range.get()->second;
}
