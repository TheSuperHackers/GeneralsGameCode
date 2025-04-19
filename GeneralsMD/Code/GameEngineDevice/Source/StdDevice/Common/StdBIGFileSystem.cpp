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

//////// StdBIGFileSystem.h ///////////////////////////
// Stephan Vedder, April 2025
/////////////////////////////////////////////////////////////

#include "Common/AudioAffect.h"
#include "Common/ArchiveFile.h"
#include "Common/ArchiveFileSystem.h"
#include "Common/File.h"
#include "Common/GameAudio.h"
#include "Common/GameMemory.h"
#include "Common/LocalFileSystem.h"
#include "StdDevice/Common/StdBIGFile.h"
#include "StdDevice/Common/StdBIGFileSystem.h"
#include "Common/Registry.h"

static const char *BIGFileIdentifier = "BIGF";

StdBIGFileSystem::StdBIGFileSystem() : ArchiveFileSystem() {
}

StdBIGFileSystem::~StdBIGFileSystem() {
}

void StdBIGFileSystem::init() {
	DEBUG_ASSERTCRASH(TheLocalFileSystem != NULL, ("TheLocalFileSystem must be initialized before TheArchiveFileSystem."));
	if (TheLocalFileSystem == NULL) {
		return;
	}

	loadBigFilesFromDirectory("", "*.big");

    // load original Generals assets
    AsciiString installPath;
    GetStringFromGeneralsRegistry("", "InstallPath", installPath );
    //@todo this will need to be ramped up to a crash for release
#ifndef _INTERNAL
    // had to make this non-internal only, otherwise we can't autobuild
    // GeneralsZH...
    DEBUG_ASSERTCRASH(installPath != "", ("Be 1337! Go install Generals!"));
#endif
    if (installPath!="")
      loadBigFilesFromDirectory(installPath, "*.big");
}

void StdBIGFileSystem::reset() {
}

void StdBIGFileSystem::update() {
}

void StdBIGFileSystem::postProcessLoad() {
}

ArchiveFile * StdBIGFileSystem::openArchiveFile(const Char *filename) {
	File *fp = TheLocalFileSystem->openFile(filename, File::READ | File::BINARY);
	AsciiString archiveFileName;
	archiveFileName = filename;
	archiveFileName.toLower();
	Int archiveFileSize = 0;
	Int numLittleFiles = 0;

	ArchiveFile *archiveFile = NEW StdBIGFile;

	DEBUG_LOG(("StdBIGFileSystem::openArchiveFile - opening BIG file %s\n", filename));

	if (fp == NULL) {
		DEBUG_CRASH(("Could not open archive file %s for parsing", filename));
		return NULL;
	}

	AsciiString asciibuf;
	char buffer[_MAX_PATH];
	fp->read(buffer, 4); // read the "BIG" at the beginning of the file.
	buffer[4] = 0;
	if (strcmp(buffer, BIGFileIdentifier) != 0) {
		DEBUG_CRASH(("Error reading BIG file identifier in file %s", filename));
		fp->close();
		fp = NULL;
		return NULL;
	}

	// read in the file size.
	fp->read(&archiveFileSize, 4);

	DEBUG_LOG(("StdBIGFileSystem::openArchiveFile - size of archive file is %d bytes\n", archiveFileSize));

//	char t;

	// read in the number of files contained in this BIG file.
	// change the order of the bytes cause the file size is in reverse byte order for some reason.
	fp->read(&numLittleFiles, 4);
	numLittleFiles = ntohl(numLittleFiles);

	DEBUG_LOG(("StdBIGFileSystem::openArchiveFile - %d are contained in archive\n", numLittleFiles));
//	for (Int i = 0; i < 2; ++i) {
//		t = buffer[i];
//		buffer[i] = buffer[(4-i)-1];
//		buffer[(4-i)-1] = t;
//	}

	// seek to the beginning of the directory listing.
	fp->seek(0x10, File::START);
	// read in each directory listing.
	ArchivedFileInfo *fileInfo = NEW ArchivedFileInfo;

	for (Int i = 0; i < numLittleFiles; ++i) {
		Int filesize = 0;
		Int fileOffset = 0;
		fp->read(&fileOffset, 4);
		fp->read(&filesize, 4);

		filesize = ntohl(filesize);
		fileOffset = ntohl(fileOffset);

		fileInfo->m_archiveFilename = archiveFileName;
		fileInfo->m_offset = fileOffset;
		fileInfo->m_size = filesize;
		
		// read in the path name of the file.
		Int pathIndex = -1;
		do {
			++pathIndex;
			fp->read(buffer + pathIndex, 1);
		} while (buffer[pathIndex] != 0);

		Int filenameIndex = pathIndex;
		while ((filenameIndex >= 0) && (buffer[filenameIndex] != '\\') && (buffer[filenameIndex] != '/')) {
			--filenameIndex;
		}

		fileInfo->m_filename = (char *)(buffer + filenameIndex + 1);
		fileInfo->m_filename.toLower();
		buffer[filenameIndex + 1] = 0;

		AsciiString path;
		path = buffer;

		AsciiString debugpath;
		debugpath = path;
		debugpath.concat(fileInfo->m_filename);
//		DEBUG_LOG(("StdBIGFileSystem::openArchiveFile - adding file %s to archive file %s, file number %d\n", debugpath.str(), fileInfo->m_archiveFilename.str(), i));

		archiveFile->addFile(path, fileInfo);
	}

	archiveFile->attachFile(fp);

	delete fileInfo;
	fileInfo = NULL;

	// leave fp open as the archive file will be using it.

	return archiveFile;
}

void StdBIGFileSystem::closeArchiveFile(const Char *filename) {
	// Need to close the specified big file
	ArchiveFileMap::iterator it =  m_archiveFileMap.find(filename);
	if (it == m_archiveFileMap.end()) {
		return;
	}

	if (stricmp(filename, MUSIC_BIG) == 0) {
		// Stop the current audio
		TheAudio->stopAudio(AudioAffect_Music);

		// No need to turn off other audio, as the lookups will just fail.
	}
	DEBUG_ASSERTCRASH(stricmp(filename, MUSIC_BIG) == 0, ("Attempting to close Archive file '%s', need to add code to handle its shutdown correctly.", filename));

	// may need to do some other processing here first.
	
	delete (it->second);
	m_archiveFileMap.erase(it);
}

void StdBIGFileSystem::closeAllArchiveFiles() {
}

void StdBIGFileSystem::closeAllFiles() {
}

Bool StdBIGFileSystem::loadBigFilesFromDirectory(AsciiString dir, AsciiString fileMask, Bool overwrite) {

	FilenameList filenameList;
	TheLocalFileSystem->getFileListInDirectory(dir, AsciiString(""), fileMask, filenameList, TRUE);

	Bool actuallyAdded = FALSE;
	FilenameListIter it = filenameList.begin();
	while (it != filenameList.end()) {
		ArchiveFile *archiveFile = openArchiveFile((*it).str());

		if (archiveFile != NULL) {
			DEBUG_LOG(("StdBIGFileSystem::loadBigFilesFromDirectory - loading %s into the directory tree.\n", (*it).str()));
			loadIntoDirectoryTree(archiveFile, *it, overwrite);
			m_archiveFileMap[(*it)] = archiveFile;
			DEBUG_LOG(("StdBIGFileSystem::loadBigFilesFromDirectory - %s inserted into the archive file map.\n", (*it).str()));
			actuallyAdded = TRUE;
		}

		it++;
	}

	return actuallyAdded;
}
