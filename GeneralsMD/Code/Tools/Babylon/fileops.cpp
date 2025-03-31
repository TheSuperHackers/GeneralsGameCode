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

//
// fileops.cpp
//

#include "StdAfx.h"
#include "fileops.h"




int							FileExists ( const char *filename )
{
	int fa = FileAttribs ( filename );

	return ! ( (fa == FA_NOFILE) || (fa & FA_DIRECTORY ));
}


int					 		FileAttribs ( const char *filename )
{
	WIN32_FIND_DATA 	fi;
	HANDLE handle;
	int	fa = FA_NOFILE;

	handle = FindFirstFile ( filename, &fi );

	if ( handle != INVALID_HANDLE_VALUE )
	{
		if ( fi.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		{
			fa |= FA_DIRECTORY;
		}
		if ( fi.dwFileAttributes & FILE_ATTRIBUTE_READONLY )
		{
			fa |= FA_READONLY;
		}
		else
		{
			fa |= FA_WRITEABLE;
		}

		FindClose ( handle );
	}

	return fa;
}

static void make_bk_name ( char *bkname, const char *filename )
{
	const char *ext;
	char *ext1;

	strcpy ( bkname, filename );
	
	ext = strchr ( filename, '.' );
	ext1 = strchr ( bkname, '.' );

	if ( ext )
	{
		strcpy ( ext1, "_back_up" );
		strcat ( ext1, ext );
	}
	else
	{
		strcat ( bkname, "_back_up" );
	}

}

void	MakeBackupFile ( const char *filename )
{
	char bkname[256];
	
	make_bk_name ( bkname, filename );

	CopyFile ( filename, bkname, FALSE );

}

void	RestoreBackupFile ( const char *filename )
{
	char bkname[256];
	
	make_bk_name ( bkname, filename );

	if ( FileExists ( bkname ))
	{
		CopyFile ( bkname, filename, FALSE );
	}

}