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

void ArchiveFileSystem::loadIntoDirectoryTree(ArchiveFile *archiveFile, Bool overwrite)
{

	FilenameList filenameList;

	archiveFile->getFileListInDirectory("", "", "*", filenameList, TRUE);

	FilenameListIter it = filenameList.begin();

	while (it != filenameList.end())
	{
		ArchivedDirectoryInfo *dirInfo = &m_rootDirectory;

		AsciiString path;
		AsciiString token;
		AsciiString tokenizer = *it;
		tokenizer.toLower();
		tokenizer.nextToken(&token, "\\/");

		while (!tokenizer.isEmpty())
		{
			path.concat(token);
			path.concat('\\');

			ArchivedDirectoryInfoMap::iterator tempiter = dirInfo->m_directories.find(token);
			if (tempiter == dirInfo->m_directories.end())
			{
				dirInfo = &(dirInfo->m_directories[token]);
				dirInfo->m_path = path;
				dirInfo->m_directoryName = token;
			}
			else
			{
				dirInfo = &tempiter->second;
			}

			tokenizer.nextToken(&token, "\\/");
		}

		ArchivedFileLocationMap::iterator fileIt;
		if (overwrite)
		{
			// When overwriting, try place the new value at the beginning of the key list.
			fileIt = dirInfo->m_files.find(token);
		}
		else
		{
			// Append to the end of the key list.
			fileIt = dirInfo->m_files.end();
		}

		dirInfo->m_files.insert(fileIt, std::make_pair(token, archiveFile));

#if defined(DEBUG_LOGGING) && ENABLE_FILESYSTEM_LOGGING
		{
			const stl::const_range<ArchivedFileLocationMap> range = stl::get_range(dirInfo->m_files, token, 0);
			if (range.distance() >= 2)
			{
				ArchivedFileLocationMap::const_iterator rangeIt0;
				ArchivedFileLocationMap::const_iterator rangeIt1;

				if (overwrite)
				{
					rangeIt0 = range.begin;
					rangeIt1 = std::next(rangeIt0);

					DEBUG_LOG(("ArchiveFileSystem::loadIntoDirectoryTree - adding file %s, archived in %s, overwriting same file in %s",
						it->str(),
						rangeIt0->second->getName().str(),
						rangeIt1->second->getName().str()
					));
				}
				else
				{
					rangeIt1 = std::prev(range.end);
					rangeIt0 = std::prev(rangeIt1);

					DEBUG_LOG(("ArchiveFileSystem::loadIntoDirectoryTree - adding file %s, archived in %s, overwritten by same file in %s",
						it->str(),
						rangeIt1->second->getName().str(),
						rangeIt0->second->getName().str()
					));
				}
			}
			else
			{
				DEBUG_LOG(("ArchiveFileSystem::loadIntoDirectoryTree - adding file %s, archived in %s", it->str(), archiveFile->getName().str()));
			}
		}
#endif

		it++;
	}
}

Bool ArchiveFileSystem::doesFileExist(const Char *filename, FileInstance instance) const
{
	ArchivedDirectoryInfoResult result = const_cast<ArchiveFileSystem*>(this)->getArchivedDirectoryInfo(filename);

	if (!result.valid())
		return false;

	stl::const_range<ArchivedFileLocationMap> range = stl::get_range(result.dirInfo->m_files, result.lastToken, instance);

	return range.valid();
}

ArchivedDirectoryInfo* ArchiveFileSystem::friend_getArchivedDirectoryInfo(const Char* directory)
{
	ArchivedDirectoryInfoResult result = getArchivedDirectoryInfo(directory);

	return result.dirInfo;
}

ArchiveFileSystem::ArchivedDirectoryInfoResult ArchiveFileSystem::getArchivedDirectoryInfo(const Char* directory)
{
	ArchivedDirectoryInfoResult result;
	ArchivedDirectoryInfo* dirInfo = &m_rootDirectory;

	AsciiString token;
	AsciiString tokenizer = directory;
	tokenizer.toLower();
	tokenizer.nextToken(&token, "\\/");

	while (!tokenizer.isEmpty())
	{
		ArchivedDirectoryInfoMap::iterator tempiter = dirInfo->m_directories.find(token);
		if (tempiter != dirInfo->m_directories.end())
		{
			dirInfo = &tempiter->second;
			tokenizer.nextToken(&token, "\\/");
		}
		else
		{
			// the directory doesn't exist
			result.dirInfo = nullptr;
			result.lastToken = AsciiString::TheEmptyString;
			return result;
		}
	}

	result.dirInfo = dirInfo;
	result.lastToken = token;
	return result;
}

File * ArchiveFileSystem::openFile(const Char *filename, Int access, FileInstance instance)
{
	ArchiveFile* archive = getArchiveFile(filename, instance);

	if (archive == nullptr)
		return nullptr;

	return archive->openFile(filename, access);
}

Bool ArchiveFileSystem::getFileInfo(const AsciiString& filename, FileInfo *fileInfo, FileInstance instance) const
{
	if (fileInfo == nullptr) {
		return FALSE;
	}

	if (filename.isEmpty()) {
		return FALSE;
	}

	ArchiveFile* archive = getArchiveFile(filename, instance);

	if (archive == nullptr)
		return FALSE;

	return archive->getFileInfo(filename, fileInfo);
}

ArchiveFile* ArchiveFileSystem::getArchiveFile(const AsciiString& filename, FileInstance instance) const
{
	ArchivedDirectoryInfoResult result = const_cast<ArchiveFileSystem*>(this)->getArchivedDirectoryInfo(filename.str());

	if (!result.valid())
		return nullptr;

	stl::const_range<ArchivedFileLocationMap> range = stl::get_range(result.dirInfo->m_files, result.lastToken, instance);

	if (!range.valid())
		return nullptr;
	
	return range.get()->second;
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
	ArchiveFileMap::const_iterator it = m_archiveFileMap.begin();
	while (it != m_archiveFileMap.end()) {
		it->second->getFileListInDirectory(currentDirectory, originalDirectory, searchName, filenameList, searchSubdirectories);
		it++;
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
