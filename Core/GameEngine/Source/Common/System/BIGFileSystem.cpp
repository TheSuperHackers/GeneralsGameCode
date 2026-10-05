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

//////// BIGFileSystem.cpp ///////////////////////////
// Bryan Cleveland, August 2002
/////////////////////////////////////////////////////////////

#include "PreRTS.h"

#include "Common/BIGFileSystem.h"

#include "Common/AudioAffect.h"
#include "Common/ArchiveFile.h"
#include "Common/file.h"
#include "Common/GameAudio.h"
#include "Common/GameMemory.h"
#include "Common/LocalFileSystem.h"

#if RTS_ZEROHOUR
#include "Common/Registry.h"
#endif

#include "Utility/endian_compat.h"


static const char BIGFileIdentifier[] = { 'B', 'I', 'G', 'F' };

struct BIGFileHeader {
	char identifier[sizeof(BIGFileIdentifier)];
	UnsignedInt archiveFileSize; // little endian, retail reads it as Int so 2 GB max
	UnsignedInt numFiles; // big endian
	UnsignedInt firstEntryOffset; // big endian
};

struct BIGFileEntryHeader {
	UnsignedInt fileOffset; // big endian
	UnsignedInt fileSize; // big endian
};

static_assert(sizeof(BIGFileHeader) == 16, "Incorrect BIG header size");
static_assert(sizeof(BIGFileEntryHeader) == 8, "Incorrect BIG entry header size");

// Returns the length of the string, or -1 if it is truncated or does not fit the buffer.
static Int readNullTerminatedString(File *fp, char *buffer, Int bufferSize) {
	for (Int i = 0; i < bufferSize; ++i) {
		if (fp->read(buffer + i, 1) != 1) {
			return -1;
		}
		if (buffer[i] == 0) {
			return i;
		}
	}
	return -1;
}

BIGFileSystem::BIGFileSystem() : ArchiveFileSystem() {
}

BIGFileSystem::~BIGFileSystem() {
}

void BIGFileSystem::init() {
	DEBUG_ASSERTCRASH(TheLocalFileSystem != nullptr, ("TheLocalFileSystem must be initialized before TheArchiveFileSystem."));
	if (TheLocalFileSystem == nullptr) {
		return;
	}

	loadArchiveFilesFromDirectory("", "*.big");

#if RTS_ZEROHOUR
    // load original Generals assets
    AsciiString installPath;
    GetStringFromGeneralsRegistry("", "InstallPath", installPath );
    //@todo this will need to be ramped up to a crash for release
    DEBUG_ASSERTCRASH(!installPath.isEmpty(), ("Be 1337! Go install Generals!"));
    if (!installPath.isEmpty())
      loadArchiveFilesFromDirectory(installPath, "*.big");
#endif
}

void BIGFileSystem::reset() {
}

void BIGFileSystem::update() {
}

void BIGFileSystem::postProcessLoad() {
}

ArchiveFile * BIGFileSystem::openArchiveFile(const AsciiString &filename, Bool overwrite) {
	if (m_archiveFileMap.find(filename) != m_archiveFileMap.end()) {
		DEBUG_CRASH(("Cannot open archive file %s more than once", filename.str()));
		return nullptr;
	}

	File *fp = TheLocalFileSystem->openFile(filename.str(), File::READ | File::BINARY);
	UnsignedInt archiveFileSize = 0;
	UnsignedInt numLittleFiles = 0;
	BIGFileHeader header;

	DEBUG_LOG(("BIGFileSystem::openArchiveFile - opening BIG file %s", filename.str()));

	if (fp == nullptr) {
		DEBUG_CRASH(("Could not open archive file %s for parsing", filename.str()));
		return nullptr;
	}

	char buffer[_MAX_PATH];
	if (fp->read(&header, sizeof(header)) != sizeof(header) || memcmp(header.identifier, BIGFileIdentifier, sizeof(BIGFileIdentifier)) != 0) {
		DEBUG_CRASH(("Error reading BIG file header in file %s", filename.str()));
		fp->close();
		fp = nullptr;
		return nullptr;
	}

	archiveFileSize = letoh(header.archiveFileSize);
	numLittleFiles = betoh(header.numFiles);

	DEBUG_LOG(("BIGFileSystem::openArchiveFile - size of archive file is %u bytes", archiveFileSize));

	DEBUG_LOG(("BIGFileSystem::openArchiveFile - %u are contained in archive", numLittleFiles));

	// read in each directory listing.
	ArchivedFileInfo fileInfo;
	ArchiveFile *archiveFile = NEW ArchiveFile(filename, AsciiString::TheEmptyString);

	Bool corrupt = FALSE;
	for (UnsignedInt i = 0; i < numLittleFiles; ++i) {
		BIGFileEntryHeader entry;
		if (fp->read(&entry, sizeof(entry)) != sizeof(entry)) {
			corrupt = TRUE;
			break;
		}

		fileInfo.m_offset = betoh(entry.fileOffset);
		fileInfo.m_size = betoh(entry.fileSize);

		// read in the path name of the file.
		const Int pathIndex = readNullTerminatedString(fp, buffer, sizeof(buffer));
		if (pathIndex < 0) {
			corrupt = TRUE;
			break;
		}

		char *leafName = getFileName(buffer);

		fileInfo.m_filename = leafName;
		fileInfo.m_filename.toLower();
		*leafName = 0;

		AsciiString path;
		path = buffer;
		path.concat(fileInfo.m_filename);

		addFile(archiveFile, path, fileInfo, overwrite);
	}

	if (corrupt) {
		DEBUG_CRASH(("Truncated or corrupt directory in BIG file %s", filename.str()));
		removeFiles(archiveFile);
		delete archiveFile;
		fp->close();
		fp = nullptr;
		return nullptr;
	}

	archiveFile->attachFile(fp);

	m_archiveFileMap[filename] = archiveFile;

	// leave fp open as the archive file will be using it.

	return archiveFile;
}

void BIGFileSystem::closeArchiveFile(const AsciiString &filename) {
	// Need to close the specified big file
	ArchiveFileMap::iterator it =  m_archiveFileMap.find(filename);
	if (it == m_archiveFileMap.end()) {
		return;
	}

	if (filename.compareNoCase(MUSIC_BIG) == 0) {
		// Stop the current audio
		TheAudio->stopAudio(AudioAffect_Music);

		// No need to turn off other audio, as the lookups will just fail.
	}
	else {
		DEBUG_CRASH(("Attempting to close Archive file '%s', need to add code to handle its shutdown correctly.", filename.str()));
	}

	// may need to do some other processing here first.

	removeFiles(it->second);
	delete (it->second);
	m_archiveFileMap.erase(it);
}

void BIGFileSystem::closeAllArchiveFiles() {
}

void BIGFileSystem::closeAllFiles() {
}

Bool BIGFileSystem::loadArchiveFilesFromDirectory(AsciiString dir, AsciiString fileMask, Bool overwrite) {

	FilenameList filenameList;
	TheLocalFileSystem->getFileListInDirectory(dir, "", fileMask, filenameList, TRUE);

	Bool actuallyAdded = FALSE;
	FilenameListIter it = filenameList.begin();
	while (it != filenameList.end()) {
#if RTS_ZEROHOUR
		// TheSuperHackers @bugfix bobtista 18/11/2025 Skip duplicate INIZH.big in Data\INI to prevent CRC mismatches.
		// English, Chinese, and Korean SKUs shipped with two INIZH.big files (one in Run directory, one in Run\Data\INI).
		// The DeleteFile cleanup doesn't work on EA App/Origin installs because the folder is not writable, so we skip loading it instead.
		if (it->endsWithNoCase("Data\\INI\\INIZH.big") || it->endsWithNoCase("Data/INI/INIZH.big")) {
			it++;
			continue;
		}
#endif

		ArchiveFile *archiveFile = openArchiveFile(*it, overwrite);

		if (archiveFile != nullptr) {
			DEBUG_LOG(("BIGFileSystem::loadArchiveFilesFromDirectory - %s inserted into the archive file map.", (*it).str()));
			actuallyAdded = TRUE;
		}

		it++;
	}

	return actuallyAdded;
}

void BIGFileSystem::loadMods()
{
	if (TheGlobalData->m_modBIG.isNotEmpty())
	{
		ArchiveFile *archiveFile = openArchiveFile(TheGlobalData->m_modBIG, TRUE);

		if (archiveFile != nullptr) {
			DEBUG_LOG(("BIGFileSystem::loadMods - %s inserted into the archive file map.", TheGlobalData->m_modBIG.str()));
		}
		else
		{
			DEBUG_LOG(("BIGFileSystem::loadMods - could not openArchiveFile(%s)", TheGlobalData->m_modBIG.str()));
		}
	}

	if (TheGlobalData->m_modDir.isNotEmpty())
	{
		MAYBE_UNUSED Bool ret = loadArchiveFilesFromDirectory(TheGlobalData->m_modDir, "*.big", TRUE);
		(void)ret;
		DEBUG_ASSERTLOG(ret, ("loadArchiveFilesFromDirectory(%s) returned FALSE!", TheGlobalData->m_modDir.str()));
	}
}
