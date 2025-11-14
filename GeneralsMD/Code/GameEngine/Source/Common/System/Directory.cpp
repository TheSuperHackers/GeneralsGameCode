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

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine
#if (0)

#include "Common/Directory.h"

//-------------------------------------------------------------------------------------------------

static void TimetToFileTime( time_t t, FILETIME& ft )
{
	LONGLONG ll = Int32x32To64(t, 10000000) + 116444736000000000;
	ft.dwLowDateTime = (DWORD) ll;
	ft.dwHighDateTime = ll >>32;
}

static time_t FileTimeToTimet( const FILETIME& ft )
{
	LONGLONG ll = (ft.dwHighDateTime << 32) + ft.dwLowDateTime - 116444736000000000;
	ll /= 10000000;
	return (time_t)ll;
}

//-------------------------------------------------------------------------------------------------

void FileInfo::set( const WIN32_FIND_DATA& info )
{
	filename = info.cFileName;
	creationTime = FileTimeToTimet(info.ftCreationTime);
	accessTime = FileTimeToTimet(info.ftLastAccessTime);
	modTime = FileTimeToTimet(info.ftLastWriteTime);
	attributes = info.dwFileAttributes;
	filesize = info.nFileSizeLow;
	//DEBUG_LOG(("FileInfo::set(): fname=%s, size=%d", filename.str(), filesize));
}

//-------------------------------------------------------------------------------------------------

Directory::Directory( const AsciiString& dirPath ) : m_dirPath(dirPath)
{
	WIN32_FIND_DATA			item;  // search item
	HANDLE							hFile;  // handle for search resources
	char								currDir[ MAX_PATH ];

	// sanity
	if( m_dirPath.isEmpty() )
	{
		DEBUG_LOG(( "Empty dirname"));
		return;
	}

	// save the current directory
	GetCurrentDirectory( MAX_PATH, currDir );

	// switch into the directory provided
	if( SetCurrentDirectory( m_dirPath.str() ) == 0 )
	{
		DEBUG_LOG(( "Can't set directory '%s'", m_dirPath.str() ));
		return;
	}

	// go through each item in the output directory
	bool done = false;
	hFile = FindFirstFile( "*", &item);
	if( hFile == INVALID_HANDLE_VALUE )
	{
		DEBUG_LOG(( "Can't search directory '%s'", m_dirPath.str() ));
		done = true;
	}

	FileInfo info;

	while (!done)
	{
		// if this is a subdirectory keep the name around till the end
		if( BitIsSet( item.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY ) )
		{
			if ( strcmp( item.cFileName, "." ) && strcmp( item.cFileName, ".." ) )
			{
				info.set(item);
				m_subdirs.insert( info );
			}
		}
		else
		{
			info.set(item);
			m_files.insert( info );
		}

		if ( FindNextFile( hFile, &item ) == 0 )
		{
			done = true;
		}
	}

	// close search
	FindClose( hFile );

	// restore the working directory to what it was when we started here
	SetCurrentDirectory( currDir );
}

FileInfoSet* Directory::getFiles( void )
{
	return &m_files;
}

FileInfoSet* Directory::getSubdirs( void )
{
	return &m_subdirs;
}

#endif
