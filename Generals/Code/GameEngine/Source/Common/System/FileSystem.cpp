/*
**	Command & Conquer Generals(tm)
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

File*		FileSystem::openFile( const Char *filename, Int access ) 
{
	USE_PERF_TIMER(FileSystem)
	File *file = NULL;

	if ( TheLocalFileSystem != NULL )
	{
		file = TheLocalFileSystem->openFile( filename, access );
	}

	if ( (TheArchiveFileSystem != NULL) && (file == NULL) )
	{
		file = TheArchiveFileSystem->openFile( filename );
	}

	return file;
}

//============================================================================
// FileSystem::doesFileExist
//============================================================================

Bool FileSystem::doesFileExist(const Char *filename) const
{
	USE_PERF_TIMER(FileSystem)
	if (TheLocalFileSystem->doesFileExist(filename)) {
		return TRUE;
	}
	if (TheArchiveFileSystem->doesFileExist(filename)) {
		return TRUE;
	}
	return FALSE;
}

//============================================================================
// FileSystem::getFileListInDirectory
//============================================================================
void FileSystem::getFileListInDirectory(const AsciiString& directory, const AsciiString& searchName, FilenameList &filenameList, Bool searchSubdirectories) const
{
	USE_PERF_TIMER(FileSystem)
	TheLocalFileSystem->getFileListInDirectory(AsciiString(""), directory, searchName, filenameList, searchSubdirectories);
	TheArchiveFileSystem->getFileListInDirectory(AsciiString(""), directory, searchName, filenameList, searchSubdirectories);
}

//============================================================================
// FileSystem::getFileInfo
//============================================================================
Bool FileSystem::getFileInfo(const AsciiString& filename, FileInfo *fileInfo) const
{
	USE_PERF_TIMER(FileSystem)
	if (fileInfo == NULL) {
		return FALSE;
	}
	memset(fileInfo, 0, sizeof(*fileInfo));
	
	if (TheLocalFileSystem->getFileInfo(filename, fileInfo)) {
		return TRUE;
	}

	if (TheArchiveFileSystem->getFileInfo(filename, fileInfo)) {
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
		DEBUG_LOG(("FileSystem::areMusicFilesOnCD() - No CD Manager; returning false\n"));
		return FALSE;
	}

	AsciiString cdRoot;
	Int dc = TheCDManager->driveCount();
	for (Int i = 0; i < dc; ++i) {
		DEBUG_LOG(("FileSystem::areMusicFilesOnCD() - checking drive %d\n", i));
		CDDriveInterface *cdi = TheCDManager->getDrive(i);
		if (!cdi) {
			continue;
		}

		cdRoot = cdi->getPath();
		if (!cdRoot.endsWith("\\"))
			cdRoot.concat("\\");
		cdRoot.concat("gensec.big");
		DEBUG_LOG(("FileSystem::areMusicFilesOnCD() - checking for %s\n", cdRoot.str()));
		File *musicBig = TheLocalFileSystem->openFile(cdRoot.str());
		if (musicBig)
		{
			DEBUG_LOG(("FileSystem::areMusicFilesOnCD() - found it!\n"));
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
AsciiString FileSystem::normalizePath(const AsciiString& path) const
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
		DEBUG_CRASH(("Unable to normalize base directory path '%s'.\n", basePath.str()));
		return false;
	}
	else if (testPathNormalized.isEmpty())
	{
		DEBUG_CRASH(("Unable to normalize file path '%s'.\n", testPath.str()));
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