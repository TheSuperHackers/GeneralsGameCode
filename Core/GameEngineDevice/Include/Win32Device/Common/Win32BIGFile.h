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

/////// Win32BIGFile.h ////////////////////////////////////
// Bryan Cleveland, August 2002
///////////////////////////////////////////////////////////

#pragma once

#ifndef __WIN32BIGFILE_H
#define __WIN32BIGFILE_H

#include "Common/ArchiveFile.h"
#include "Common/AsciiString.h"
#include "Common/List.h"

class Win32BIGFile : public ArchiveFile
{
	public:
		Win32BIGFile();
		virtual ~Win32BIGFile();

		virtual Bool					getFileInfo(const AsciiString& filename, FileInfo *fileInfo) const;	///< fill in the fileInfo struct with info about the requested file.
		virtual File*					openFile( const Char *filename, Int access = 0 );///< Open the specified file within the BIG file
		virtual void					closeAllFiles( void );									///< Close all file opened in this BIG file
		virtual AsciiString		getName( void );												///< Returns the name of the BIG file
		virtual AsciiString		getPath( void );												///< Returns full path and name of BIG file
		virtual void					setSearchPriority( Int new_priority );	///< Set this BIG file's search priority
		virtual void					close( void );													///< Close this BIG file

	protected:

		AsciiString		m_name;		///< BIG file name
		AsciiString		m_path;		///< BIG file path
};

#endif // __WIN32BIGFILE_H