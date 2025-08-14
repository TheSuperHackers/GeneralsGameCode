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

///// StdLocalFileSystem.h //////////////////////////////////
// Stephan Vedder, April 2025
///////////////////////////////////////////////////////////////

#pragma once

#ifndef __STDLOCALFILESYSTEM_H
#define __STDLOCALFILESYSTEM_H
#include "Common/LocalFileSystem.h"

class StdLocalFileSystem : public LocalFileSystem
{
public:
	StdLocalFileSystem();
	virtual ~StdLocalFileSystem();

	virtual void init();
	virtual void reset();
	virtual void update();

	virtual File * openFile(const Char *filename, Int access = File::NONE, size_t bufferSize = File::BUFFERSIZE);	///< open the given file.
	virtual Bool doesFileExist(const Char *filename) const;								///< does the given file exist?

	virtual void getFileListInDirectory(const AsciiString& currentDirectory, const AsciiString& originalDirectory, const AsciiString& searchName, FilenameList &filenameList, Bool searchSubdirectories) const; ///< search the given directory for files matching the searchName (egs. *.ini, *.rep).  Possibly search subdirectories.
	virtual Bool getFileInfo(const AsciiString& filename, FileInfo *fileInfo) const;

	virtual Bool createDirectory(AsciiString directory);
	virtual AsciiString normalizePath(const AsciiString& filePath) const;

protected:
};

#endif // __STDLOCALFILESYSTEM_H
