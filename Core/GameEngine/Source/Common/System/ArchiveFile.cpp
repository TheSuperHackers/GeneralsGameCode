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

/////////ArchiveFile.cpp ///////////////////////
// Bryan Cleveland, August 2002
////////////////////////////////////////////////

#include "PreRTS.h"

#include "Common/ArchiveFile.h"
#include "Common/ArchiveFileSystem.h"
#include "Common/file.h"
#include "Common/PerfTimer.h"


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

ArchiveFile::~ArchiveFile()
{
	if (m_file != nullptr) {
		m_file->close();
		m_file = nullptr;
	}
}

ArchiveFile::ArchiveFile()
	: m_file(nullptr)
{
}

void ArchiveFile::addFile(const AsciiString& path, const ArchivedFileInfo *fileInfo)
{
	DetailedArchivedDirectoryInfo *dirInfo = &m_rootDirectory;

	AsciiString token;
	AsciiString tokenizer = path;
	tokenizer.toLower();
	tokenizer.nextToken(&token, "\\/");

	while (!token.isEmpty())
	{
		DetailedArchivedDirectoryInfoMap::iterator tempiter = dirInfo->m_directories.find(token);
		if (tempiter == dirInfo->m_directories.end())
		{
			dirInfo = &(dirInfo->m_directories[token]);
			dirInfo->m_directoryName = token;
		}
		else
		{
			dirInfo = &tempiter->second;
		}

		tokenizer.nextToken(&token, "\\/");
	}

	dirInfo->m_files[fileInfo->m_filename] = *fileInfo;
}

void ArchiveFile::getFileListInDirectory(const AsciiString& currentDirectory, const AsciiString& originalDirectory, const AsciiString& searchName, FilenameList &filenameList, Bool searchSubdirectories) const
{
	const DetailedArchivedDirectoryInfo *dirInfo = &m_rootDirectory;

	AsciiString token;
	AsciiString tokenizer = originalDirectory;
	tokenizer.toLower();
	tokenizer.nextToken(&token, "\\/");

	while (!token.isEmpty()) {

		DetailedArchivedDirectoryInfoMap::const_iterator it = dirInfo->m_directories.find(token);
		if (it != dirInfo->m_directories.end())
		{
			dirInfo = &it->second;
		}
		else
		{
			// if the directory doesn't exist, then there aren't any files to be had.
			return;
		}

		tokenizer.nextToken(&token, "\\/");
	}

	getFileListInDirectory(dirInfo, originalDirectory, searchName, filenameList, searchSubdirectories);
}

void ArchiveFile::getFileListInDirectory(const DetailedArchivedDirectoryInfo *dirInfo, const AsciiString& currentDirectory, const AsciiString& searchName, FilenameList &filenameList, Bool searchSubdirectories) const
{
	DetailedArchivedDirectoryInfoMap::const_iterator diriter = dirInfo->m_directories.begin();
	while (diriter != dirInfo->m_directories.end()) {
		const DetailedArchivedDirectoryInfo *tempDirInfo = &(diriter->second);
		AsciiString tempdirname;
		tempdirname = currentDirectory;
		if ((!tempdirname.isEmpty()) && (!tempdirname.endsWith("\\"))) {
			tempdirname.concat('\\');
		}
		tempdirname.concat(tempDirInfo->m_directoryName);
		getFileListInDirectory(tempDirInfo, tempdirname, searchName, filenameList, searchSubdirectories);
		diriter++;
	}

	ArchivedFileInfoMap::const_iterator fileiter = dirInfo->m_files.begin();
	while (fileiter != dirInfo->m_files.end()) {
		if (SearchStringMatches(fileiter->second.m_filename, searchName)) {
			AsciiString tempfilename;
			tempfilename = currentDirectory;
			if ((!tempfilename.isEmpty()) && (!tempfilename.endsWith("\\"))) {
				tempfilename.concat('\\');
			}
			tempfilename.concat(fileiter->second.m_filename);
			if (filenameList.find(tempfilename) == filenameList.end()) {
				// only insert into the list if its not already in there.
				filenameList.insert(tempfilename);
			}
		}
		fileiter++;
	}
}

void ArchiveFile::attachFile(File *file)
{
	if (m_file != nullptr) {
		m_file->close();
		m_file = nullptr;
	}
	m_file = file;
}

const ArchivedFileInfo * ArchiveFile::getArchivedFileInfo(const AsciiString& filename) const
{
	const DetailedArchivedDirectoryInfo *dirInfo = &m_rootDirectory;

	AsciiString token;
	AsciiString tokenizer = filename;
	tokenizer.toLower();
	tokenizer.nextToken(&token, "\\/");

	while (!token.find('.') || tokenizer.find('.'))
	{
		DetailedArchivedDirectoryInfoMap::const_iterator it = dirInfo->m_directories.find(token);
		if (it != dirInfo->m_directories.end())
		{
			dirInfo = &it->second;
		}
		else
		{
			return nullptr;
		}

		tokenizer.nextToken(&token, "\\/");
	}

	ArchivedFileInfoMap::const_iterator it = dirInfo->m_files.find(token);
	if (it != dirInfo->m_files.end())
	{
		return &it->second;
	}
	else
	{
		return nullptr;
	}

}
