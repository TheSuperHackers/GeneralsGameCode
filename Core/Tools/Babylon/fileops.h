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
// File IO support
//

#ifndef __FILEOPS_H
#define __FILEOPS_H

static const int FA_NOFILE = 0;
static const int 	FA_READONLY = 0x00000001;
static const int 	FA_DIRECTORY = 0x00000002;
static const int 	FA_WRITEABLE = 0x00000004;


int							FileExists ( const char *filename );
int					 		FileAttribs ( const char *filename );
void						MakeBackupFile ( const char *filename );
void						RestoreBackupFile ( const char *filename );


#endif		// __FILEIO_H
