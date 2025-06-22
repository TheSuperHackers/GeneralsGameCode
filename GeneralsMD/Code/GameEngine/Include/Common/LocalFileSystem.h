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

#ifndef __LOCALFILESYSTEM_H
#define __LOCALFILESYSTEM_H

#include "Common/SubsystemInterface.h"
#include "FileSystem.h" // for typedefs, etc.

class File;

class LocalFileSystem : public SubsystemInterface
{
public:
	virtual ~LocalFileSystem() {}

	virtual void init() = 0;
	virtual void reset() = 0;
	virtual void update() = 0;

	virtual File * openFile(const Char *filename, Int access = 0) = 0;
	virtual Bool doesFileExist(const Char *filename) const = 0;
	virtual void getFileListInDirectory(const AsciiString& currentDirectory, const AsciiString& originalDirectory, const AsciiString& searchName, FilenameList &filenameList, Bool searchSubdirectories) const = 0; ///< search the given directory for files matching the searchName (egs. *.ini, *.rep).  Possibly search subdirectories.
	virtual Bool getFileInfo(const AsciiString& filename, FileInfo *fileInfo) const = 0; ///< see FileSystem.h
	virtual Bool createDirectory(AsciiString directory) = 0; ///< see FileSystem.h
	virtual AsciiString normalizePath(const AsciiString& filePath) const = 0;	///< see FileSystem.h

protected:
};

extern LocalFileSystem *TheLocalFileSystem;

#endif // __LOCALFILESYSTEM_H