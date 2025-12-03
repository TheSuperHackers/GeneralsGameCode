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
//                Copyright(C) 2001 - All Rights Reserved
//
//----------------------------------------------------------------------------
//
// Project:   WSYS Library
//
// Module:    IO
//
// File name: IO_FileSystem.cpp
//
// Created:   4/23/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//         Includes
//----------------------------------------------------------------------------

#include "PreRTS.h"
#include "Common/file.h"
#include "Common/FileSystem.h"

#include "Common/ArchiveFileSystem.h"
#include "Common/CDManager.h"
#include "Common/GameAudio.h"
#include "Common/LocalFileSystem.h"
#include "Common/PerfTimer.h"


DECLARE_PERF_TIMER(FileSystem)

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

//===============================
// TheFileSystem
//===============================
/**
  *	This is the FileSystem's singleton class. All file access
	* should be through TheFileSystem, unless code needs to use an explicit
	* File or FileSystem derivative.
	*
	* Using TheFileSystem->open and File exclusively for file access, particularly
	* in library or modular code, allows applications to transparently implement
	* file access as they see fit. This is particularly important for code that
	* needs to be shared between applications, such as games and tools.
	*/
//===============================

FileSystem	*TheFileSystem = NULL;

//----------------------------------------------------------------------------
//         Private Prototypes
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//         Private Functions
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//         Public Functions
//----------------------------------------------------------------------------


//============================================================================
// FileSystem::FileSystem
//============================================================================

FileSystem::FileSystem()
{

}

//============================================================================
// FileSystem::~FileSystem
//============================================================================

FileSystem::~FileSystem()
{

}

//============================================================================
// FileSystem::init
//============================================================================

void		FileSystem::init( void )
{
	TheLocalFileSystem->init();
	TheArchiveFileSystem->init();
}

//============================================================================
// FileSystem::update
//============================================================================

void		FileSystem::update( void )
{
	USE_PERF_TIMER(FileSystem)
	TheLocalFileSystem->update();
	TheArchiveFileSystem->update();
}

//============================================================================
// FileSystem::reset
//============================================================================

void		FileSystem::reset( void )
{
	USE_PERF_TIMER(FileSystem)
	TheLocalFileSystem->reset();
	TheArchiveFileSystem->reset();
}

//============================================================================
// FileSystem::open
//============================================================================

File*		FileSystem::openFile( const Char *filename, Int access, size_t bufferSize, FileInstance instance )
{
	USE_PERF_TIMER(FileSystem)
	File *file = NULL;

	if ( TheLocalFileSystem != NULL )
	{
		if (instance != 0)
		{
			if (TheLocalFileSystem->doesFileExist(filename))
			{
				--instance;
			}
		}
		else
		{
			file = TheLocalFileSystem->openFile( filename, access, bufferSize );

#if ENABLE_FILESYSTEM_EXISTENCE_CACHE
			if (file != NULL && (file->getAccess() & File::CREATE))
			{
				FastCriticalSectionClass::LockClass lock(m_fileExistMutex);
				FileExistMap::iterator it = m_fileExist.find(FileExistMap::key_type::temporary(filename));
				if (it != m_fileExist.end())
				{
					++it->second.instanceExists;
					if (it->second.instanceDoesNotExist != ~FileInstance(0))
						++it->second.instanceDoesNotExist;
				}
				else
				{
					m_fileExist[filename];
				}
			}
#endif
		}
	}

	if ( (TheArchiveFileSystem != NULL) && (file == NULL) )
	{
		// TheSuperHackers @todo Pass 'access' here?
		file = TheArchiveFileSystem->openFile( filename, 0, instance );
	}

	return file;
}

//============================================================================
// FileSystem::doesFileExist
//============================================================================

Bool FileSystem::doesFileExist(const Char *filename, FileInstance instance) const
{
	USE_PERF_TIMER(FileSystem)

#if ENABLE_FILESYSTEM_EXISTENCE_CACHE
	{
		FastCriticalSectionClass::LockClass lock(m_fileExistMutex);
		FileExistMap::const_iterator it = m_fileExist.find(FileExistMap::key_type::temporary(filename));
		if (it != m_fileExist.end())
		{
			// Must test instanceDoesNotExist first!
			if (instance >= it->second.instanceDoesNotExist)
				return FALSE;
			if (instance <= it->second.instanceExists)
				return TRUE;
		}
	}
#endif

	if (TheLocalFileSystem->doesFileExist(filename))
	{
		if (instance == 0)
		{
#if ENABLE_FILESYSTEM_EXISTENCE_CACHE
			{
				FastCriticalSectionClass::LockClass lock(m_fileExistMutex);
				m_fileExist[filename];
			}
#endif
			return TRUE;
		}

		--instance;
	}

	if (TheArchiveFileSystem->doesFileExist(filename, instance))
	{
#if ENABLE_FILESYSTEM_EXISTENCE_CACHE
		{
			FastCriticalSectionClass::LockClass lock(m_fileExistMutex);
			FileExistMap::mapped_type& value = m_fileExist[filename];
			value.instanceExists = max(value.instanceExists, instance);
		}
#endif
		return TRUE;
	}

#if ENABLE_FILESYSTEM_EXISTENCE_CACHE
	{
		FastCriticalSectionClass::LockClass lock(m_fileExistMutex);
		FileExistMap::mapped_type& value = m_fileExist[filename];
		value.instanceDoesNotExist = min(value.instanceDoesNotExist, instance);
	}
#endif
	return FALSE;
}

//============================================================================
// FileSystem::getFileListInDirectory
//============================================================================
void FileSystem::getFileListInDirectory(const AsciiString& directory, const AsciiString& searchName, FilenameList &filenameList, Bool searchSubdirectories) const
{
	USE_PERF_TIMER(FileSystem)
	TheLocalFileSystem->getFileListInDirectory(AsciiString::TheEmptyString, directory, searchName, filenameList, searchSubdirectories);
	TheArchiveFileSystem->getFileListInDirectory(AsciiString::TheEmptyString, directory, searchName, filenameList, searchSubdirectories);
}

//============================================================================
// FileSystem::getFileInfo
//============================================================================
Bool FileSystem::getFileInfo(const AsciiString& filename, FileInfo *fileInfo, FileInstance instance) const
{
	USE_PERF_TIMER(FileSystem)

	// TheSuperHackers @todo Add file info cache?

	if (fileInfo == NULL) {
		return FALSE;
	}
	memset(fileInfo, 0, sizeof(*fileInfo));

	if (TheLocalFileSystem->getFileInfo(filename, fileInfo)) {
		if (instance == 0) {
			return TRUE;
		}

		--instance;
	}

	if (TheArchiveFileSystem->getFileInfo(filename, fileInfo, instance)) {
		return TRUE;
	}

	return FALSE;
}

//============================================================================
// FileSystem::createDirectory
//============================================================================
Bool FileSystem::createDirectory(AsciiString directory)
{
	USE_PERF_TIMER(FileSystem)
	if (TheLocalFileSystem != NULL) {
		return TheLocalFileSystem->createDirectory(directory);
	}
	return FALSE;
}

//============================================================================
// FileSystem::areMusicFilesOnCD
//============================================================================
Bool FileSystem::areMusicFilesOnCD()
{
#if 1
	return TRUE;
#else
	if (!TheCDManager) {
		DEBUG_LOG(("FileSystem::areMusicFilesOnCD() - No CD Manager; returning false"));
		return FALSE;
	}

	AsciiString cdRoot;
	Int dc = TheCDManager->driveCount();
	for (Int i = 0; i < dc; ++i) {
		DEBUG_LOG(("FileSystem::areMusicFilesOnCD() - checking drive %d", i));
		CDDriveInterface *cdi = TheCDManager->getDrive(i);
		if (!cdi) {
			continue;
		}

		cdRoot = cdi->getPath();
		if (!cdRoot.endsWith("\\"))
			cdRoot.concat("\\");
#if RTS_GENERALS
		cdRoot.concat("gensec.big");
#elif RTS_ZEROHOUR
		cdRoot.concat("genseczh.big");
#endif
		DEBUG_LOG(("FileSystem::areMusicFilesOnCD() - checking for %s", cdRoot.str()));
		File *musicBig = TheLocalFileSystem->openFile(cdRoot.str());
		if (musicBig)
		{
			DEBUG_LOG(("FileSystem::areMusicFilesOnCD() - found it!"));
			musicBig->close();
			return TRUE;
		}
	}
	return FALSE;
#endif
}
//============================================================================
// FileSystem::loadMusicFilesFromCD
//============================================================================
void FileSystem::loadMusicFilesFromCD()
{
	if (!TheCDManager) {
		return;
	}

	AsciiString cdRoot;
	Int dc = TheCDManager->driveCount();
	for (Int i = 0; i < dc; ++i) {
		CDDriveInterface *cdi = TheCDManager->getDrive(i);
		if (!cdi) {
			continue;
		}

		cdRoot = cdi->getPath();
		if (TheArchiveFileSystem->loadBigFilesFromDirectory(cdRoot, MUSIC_BIG)) {
			break;
		}
	}
}

//============================================================================
// FileSystem::unloadMusicFilesFromCD
//============================================================================
void FileSystem::unloadMusicFilesFromCD()
{
	if (!(TheAudio && TheAudio->isMusicPlayingFromCD())) {
		return;
	}

	TheArchiveFileSystem->closeArchiveFile( MUSIC_BIG );
}

//============================================================================
// FileSystem::normalizePath
//============================================================================
AsciiString FileSystem::normalizePath(const AsciiString& path)
{
	return TheLocalFileSystem->normalizePath(path);
}

//============================================================================
// FileSystem::isPathInDirectory
//============================================================================
Bool FileSystem::isPathInDirectory(const AsciiString& testPath, const AsciiString& basePath)
{
	AsciiString testPathNormalized = TheFileSystem->normalizePath(testPath);
	AsciiString basePathNormalized = TheFileSystem->normalizePath(basePath);

	if (basePathNormalized.isEmpty())
	{
		DEBUG_CRASH(("Unable to normalize base directory path '%s'.", basePath.str()));
		return false;
	}
	else if (testPathNormalized.isEmpty())
	{
		DEBUG_CRASH(("Unable to normalize file path '%s'.", testPath.str()));
		return false;
	}

#ifdef _WIN32
	const char* pathSep = "\\";
#else
	const char* pathSep = "/";
#endif

	if (!basePathNormalized.endsWith(pathSep))
	{
		basePathNormalized.concat(pathSep);
	}

#ifdef _WIN32
	if (!testPathNormalized.startsWithNoCase(basePathNormalized))
#else
	if (!testPathNormalized.startsWith(basePathNormalized))
#endif
	{
		return false;
	}

	return true;
}

//============================================================================
// FileSystem::hasValidTransferFileContent
//============================================================================
// TheSuperHackers @security bobtista 06/11/2025
// Validates file content format during map transfer operations.
// Checks file headers, magic bytes, size limits, and basic structure to prevent
// malformed or malicious files from being processed.
//============================================================================
Bool FileSystem::hasValidTransferFileContent(const AsciiString& filePath)
{
	File* file = TheLocalFileSystem->openFile(filePath.str(), File::READ | File::BINARY);
	if (file == NULL)
	{
		DEBUG_LOG(("Cannot open file '%s' for content validation.", filePath.str()));
		return false;
	}

	const Int fileSize = file->size();
	Bool isValid = false;

	const char* lastDot = strrchr(filePath.str(), '.');
	if (lastDot == NULL)
	{
		file->close();
		return false;
	}

	const Int MAX_MAP_SIZE = 50 * 1024 * 1024;
	const Int MAX_INI_SIZE = 10 * 1024 * 1024;
	const Int MAX_STR_SIZE = 5 * 1024 * 1024;
	const Int MAX_TGA_SIZE = 20 * 1024 * 1024;
	const Int MAX_TXT_SIZE = 5 * 1024 * 1024;
	const Int MAX_WAK_SIZE = 10 * 1024 * 1024;

#ifdef _WIN32
	if (_stricmp(lastDot, ".map") == 0)
#else
	if (strcasecmp(lastDot, ".map") == 0)
#endif
	{
		if (fileSize > MAX_MAP_SIZE)
		{
			DEBUG_LOG(("Map file '%s' exceeds maximum size (%d bytes).", filePath.str(), fileSize));
			isValid = false;
		}
		else
		{
			UnsignedByte header[4];
			file->read(header, 4);
			if (header[0] == 'C' && header[1] == 'k' && header[2] == 'M' && header[3] == 'p')
			{
				isValid = true;
			}
			else
			{
				DEBUG_LOG(("Map file '%s' has invalid magic bytes.", filePath.str()));
				isValid = false;
			}
		}
	}
#ifdef _WIN32
	else if (_stricmp(lastDot, ".ini") == 0)
#else
	else if (strcasecmp(lastDot, ".ini") == 0)
#endif
	{
		if (fileSize > MAX_INI_SIZE)
		{
			DEBUG_LOG(("INI file '%s' exceeds maximum size (%d bytes).", filePath.str(), fileSize));
			isValid = false;
		}
		else
		{
			UnsignedByte sample[512];
			Int bytesToRead = fileSize < 512 ? fileSize : 512;
			file->read(sample, bytesToRead);

			Bool hasNullBytes = false;
			for (Int i = 0; i < bytesToRead; ++i)
			{
				if (sample[i] == 0)
				{
					hasNullBytes = true;
					break;
				}
			}

			if (hasNullBytes)
			{
				DEBUG_LOG(("INI file '%s' contains null bytes (likely binary).", filePath.str()));
				isValid = false;
			}
			else
			{
				isValid = true;
			}
		}
	}
#ifdef _WIN32
	else if (_stricmp(lastDot, ".str") == 0 || _stricmp(lastDot, ".txt") == 0)
#else
	else if (strcasecmp(lastDot, ".str") == 0 || strcasecmp(lastDot, ".txt") == 0)
#endif
	{
		Int maxSize = MAX_STR_SIZE;
#ifdef _WIN32
		if (_stricmp(lastDot, ".txt") == 0)
#else
		if (strcasecmp(lastDot, ".txt") == 0)
#endif
		{
			maxSize = MAX_TXT_SIZE;
		}

		if (fileSize > maxSize)
		{
			DEBUG_LOG(("Text file '%s' exceeds maximum size (%d bytes).", filePath.str(), fileSize));
			isValid = false;
		}
		else
		{
			isValid = true;
		}
	}
#ifdef _WIN32
	else if (_stricmp(lastDot, ".tga") == 0)
#else
	else if (strcasecmp(lastDot, ".tga") == 0)
#endif
	{
		if (fileSize > MAX_TGA_SIZE)
		{
			DEBUG_LOG(("TGA file '%s' exceeds maximum size (%d bytes).", filePath.str(), fileSize));
			isValid = false;
		}
		else if (fileSize < 18)
		{
			DEBUG_LOG(("TGA file '%s' is too small to be valid (minimum 18 bytes).", filePath.str()));
			isValid = false;
		}
		else
		{
			isValid = true;
		}
	}
#ifdef _WIN32
	else if (_stricmp(lastDot, ".wak") == 0)
#else
	else if (strcasecmp(lastDot, ".wak") == 0)
#endif
	{
		if (fileSize > MAX_WAK_SIZE)
		{
			DEBUG_LOG(("WAK file '%s' exceeds maximum size (%d bytes).", filePath.str(), fileSize));
			isValid = false;
		}
		else
		{
			isValid = true;
		}
	}
	else
	{
		DEBUG_LOG(("File '%s' has unrecognized extension for content validation.", filePath.str()));
		isValid = false;
	}

	file->close();
	return isValid;
}
