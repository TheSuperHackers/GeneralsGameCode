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

// FILE: Directory.h //////////////////////////////////////////////////////////////////////////////
// Author: Matthew D. Campbell, August 2002
// Desc:	 Directory info class
///////////////////////////////////////////////////////////////////////////////////////////////////
#if (0)

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/AsciiString.h"
#include "Common/STLTypedefs.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////

// TYPES //////////////////////////////////////////////////////////////////////////////////////////

class FileInfo
{
public:
	FileInfo() {}
	~FileInfo() {}

	void set( const WIN32_FIND_DATA& info );

	AsciiString filename;
	time_t creationTime;
	time_t accessTime;
	time_t modTime;
	DWORD attributes;
	DWORD filesize;	// only care about 32 bits for our purposes

protected:
};

struct FileInfoComparator
{
	bool operator()(const FileInfo& a, const FileInfo& b) const
	{
		return a.filename < b.filename;
	}
};

//-------------------------------------------------------------------------------------------------

typedef std::set<FileInfo, FileInfoComparator> FileInfoSet;

//-------------------------------------------------------------------------------------------------

class Directory
{
public:
	Directory(const AsciiString& dirPath);
	~Directory() {}

	FileInfoSet* getFiles( void );
	FileInfoSet* getSubdirs( void );

protected:
	AsciiString m_dirPath;

	FileInfoSet m_files;
	FileInfoSet m_subdirs;
};

//-------------------------------------------------------------------------------------------------

#endif
