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

///// Win32LocalFileSystem.h //////////////////////////////////
// Bryan Cleveland, August 2002
///////////////////////////////////////////////////////////////

#pragma once

#ifndef __WIN32LOCALFILESYSTEM_H
#define __WIN32LOCALFILESYSTEM_H
#include "Common/LocalFileSystem.h"

class Win32LocalFileSystem : public LocalFileSystem
{
public:
	Win32LocalFileSystem();
	virtual ~Win32LocalFileSystem();

	virtual void init();
	virtual void reset();
	virtual void update();

	virtual File * openFile(const Char *filename, Int access = 0);	///< open the given file.
	virtual Bool doesFileExist(const Char *filename) const;								///< does the given file exist?

	virtual void getFileListInDirectory(const AsciiString& currentDirectory, const AsciiString& originalDirectory, const AsciiString& searchName, FilenameList &filenameList, Bool searchSubdirectories) const; ///< search the given directory for files matching the searchName (egs. *.ini, *.rep).  Possibly search subdirectories.
	virtual Bool getFileInfo(const AsciiString& filename, FileInfo *fileInfo) const;

	virtual Bool createDirectory(AsciiString directory);
	virtual AsciiString normalizePath(const AsciiString& filePath) const;

protected:
};

#endif // __WIN32LOCALFILESYSTEM_H