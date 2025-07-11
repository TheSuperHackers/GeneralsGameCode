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

ArchiveFileSystem *TheArchiveFileSystem = NULL;


//----------------------------------------------------------------------------
//         Private Prototypes                                               
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Functions                                               
//----------------------------------------------------------------------------



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
		if (file != NULL) {
			delete file;
			file = NULL;
		}
		iter++;
	}
}

void ArchiveFileSystem::loadIntoDirectoryTree(const ArchiveFile *archiveFile, const AsciiString& archiveFilename, Bool overwrite)
{

	FilenameList filenameList;

	archiveFile->getFileListInDirectory(AsciiString(""), AsciiString(""), AsciiString("*"), filenameList, TRUE);

	FilenameListIter it = filenameList.begin();

	while (it != filenameList.end()) {
		// add this filename to the directory tree.
		AsciiString path = *it;
		path.toLower();
		AsciiString token;
		AsciiString debugpath;

		ArchivedDirectoryInfo *dirInfo = &m_rootDirectory;

		Bool infoInPath;
		infoInPath = path.nextToken(&token, "\\/");

		while (infoInPath && (!token.find('.') || path.find('.'))) {
			ArchivedDirectoryInfoMap::iterator tempiter = dirInfo->m_directories.find(token);
			if (tempiter == dirInfo->m_directories.end()) 
			{
				dirInfo->m_directories[token].clear();
				dirInfo->m_directories[token].m_directoryName = token;
			}

			dirInfo = &(dirInfo->m_directories[token]);
			debugpath.concat(token);
			debugpath.concat('\\');
			infoInPath = path.nextToken(&token, "\\/");
		}

		// token is the filename, and dirInfo is the directory that this file is in.
		if (dirInfo->m_files.find(token) == dirInfo->m_files.end() || overwrite) {
			AsciiString path2;
			path2 = debugpath;
			path2.concat(token);
//			DEBUG_LOG(("ArchiveFileSystem::loadIntoDirectoryTree - adding file %s, archived in %s\n", path2.str(), archiveFilename.str()));
			dirInfo->m_files[token] = archiveFilename;
		}

		it++;
	}
}

void ArchiveFileSystem::loadMods() {
	if (TheGlobalData->m_modBIG.isNotEmpty())
	{
		ArchiveFile *archiveFile = openArchiveFile(TheGlobalData->m_modBIG.str());

		if (archiveFile != NULL) {
			DEBUG_LOG(("ArchiveFileSystem::loadMods - loading %s into the directory tree.\n", TheGlobalData->m_modBIG.str()));
			loadIntoDirectoryTree(archiveFile, TheGlobalData->m_modBIG, TRUE);
			m_archiveFileMap[TheGlobalData->m_modBIG] = archiveFile;
			DEBUG_LOG(("ArchiveFileSystem::loadMods - %s inserted into the archive file map.\n", TheGlobalData->m_modBIG.str()));
		}
		else
		{
			DEBUG_LOG(("ArchiveFileSystem::loadMods - could not openArchiveFile(%s)\n", TheGlobalData->m_modBIG.str()));
		}
	}

	if (TheGlobalData->m_modDir.isNotEmpty())
	{
#ifdef DEBUG_LOGGING
		Bool ret =
#endif
		loadBigFilesFromDirectory(TheGlobalData->m_modDir, "*.big", TRUE);
		DEBUG_ASSERTLOG(ret, ("loadBigFilesFromDirectory(%s) returned FALSE!\n", TheGlobalData->m_modDir.str()));
	}
}

Bool ArchiveFileSystem::doesFileExist(const Char *filename) const
{
	AsciiString path = filename;
	path.toLower();
	AsciiString token;

	const ArchivedDirectoryInfo *dirInfo = &m_rootDirectory;

	path.nextToken(&token, "\\/");

	while (!token.find('.') || path.find('.')) 
	{
		ArchivedDirectoryInfoMap::const_iterator tempiter = dirInfo->m_directories.find(token);
		if (tempiter != dirInfo->m_directories.end()) 
		{
			dirInfo = &tempiter->second;
			path.nextToken(&token, "\\/");
		}
		else
		{
			// the directory doesn't exist, so return false
			return FALSE;
		}
	}

	// token is the filename, and dirInfo is the directory that this file is in.
	if (dirInfo->m_files.find(token) == dirInfo->m_files.end()) {
		return FALSE;
	}
	return TRUE;
}

File * ArchiveFileSystem::openFile(const Char *filename, Int access /* = 0 */) 
{
	AsciiString archiveFilename;
	archiveFilename = getArchiveFilenameForFile(AsciiString(filename));

	if (archiveFilename.getLength() == 0) {
		return NULL;
	}

	return m_archiveFileMap[archiveFilename]->openFile(filename, access);
}

Bool ArchiveFileSystem::getFileInfo(const AsciiString& filename, FileInfo *fileInfo) const
{
	if (fileInfo == NULL) {
		return FALSE;
	}

	if (filename.getLength() <= 0) {
		return FALSE;
	}

	AsciiString archiveFilename = getArchiveFilenameForFile(filename);
	ArchiveFileMap::const_iterator it = m_archiveFileMap.find(archiveFilename);
	if (it != m_archiveFileMap.end())
	{
		return it->second->getFileInfo(filename, fileInfo);
	}
	else
	{
		return FALSE;
	}
}

AsciiString ArchiveFileSystem::getArchiveFilenameForFile(const AsciiString& filename) const
{
	AsciiString path;
	path = filename;
	path.toLower();
	AsciiString token;
	AsciiString debugpath;

	const ArchivedDirectoryInfo *dirInfo = &m_rootDirectory;

	path.nextToken(&token, "\\/");

	while (!token.find('.') || path.find('.')) {

		ArchivedDirectoryInfoMap::const_iterator it = dirInfo->m_directories.find(token);
		if (it != dirInfo->m_directories.end())
		{
			dirInfo = &it->second;
		}
		else
		{
			// the directory doesn't exist, so return NULL

			// dump the directories;
			//DEBUG_LOG(("directory %s not found in %s in archive file system\n", token.str(), debugpath.str()));
			//DEBUG_LOG(("directories in %s in archive file system are:\n", debugpath.str()));
			//ArchivedDirectoryInfoMap::const_iterator it = dirInfo->m_directories.begin();
			//while (it != dirInfo->m_directories.end()) {
			//	DEBUG_LOG(("\t%s\n", it->second.m_directoryName.str()));
			//	it++;
			//}
			//DEBUG_LOG(("end of directory list.\n"));
			return AsciiString::TheEmptyString;
		}

		debugpath.concat(token);
		debugpath.concat('\\');

		path.nextToken(&token, "\\/");
	}

	ArchivedFileLocationMap::const_iterator it = dirInfo->m_files.find(token);
	if (it != dirInfo->m_files.end())
	{
		return it->second;
	}
	else
	{
		return AsciiString::TheEmptyString;
	}

}

void ArchiveFileSystem::getFileListInDirectory(const AsciiString& currentDirectory, const AsciiString& originalDirectory, const AsciiString& searchName, FilenameList &filenameList, Bool searchSubdirectories) const
{
	ArchiveFileMap::const_iterator it = m_archiveFileMap.begin();
	while (it != m_archiveFileMap.end()) {
		it->second->getFileListInDirectory(currentDirectory, originalDirectory, searchName, filenameList, searchSubdirectories);
		it++;
	}
}
