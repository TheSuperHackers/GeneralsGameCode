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

//----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright(C) 2001 - All Rights Reserved
//
//----------------------------------------------------------------------------
//
// Project:   WSYS Library
//
// Module:    IO
//
// File name: IO_StdFileSystem.cpp
//
// Created:   4/23/01
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//         Includes
//----------------------------------------------------------------------------

#include "WSYS_StdFileSystem.h"
#include "WSYS_StdFile.h"

//----------------------------------------------------------------------------
//         Externals
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Defines
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Types
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Data
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//         Public Data
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Prototypes
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Functions
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Public Functions
//----------------------------------------------------------------------------

//=================================================================
// StdFileSystem::~StdFileSystem
//=================================================================

StdFileSystem::~StdFileSystem()
{

}

//=================================================================
// StdFileSystem::open
//=================================================================
/**
  * This simply creates a StdFile object and calls its open function.
	*/
//=================================================================

File* StdFileSystem::open( const Char *filename, Int access )
{
	StdFile *file = new StdFile();

	if( file->open( filename, access ))
	{
		file->deleteOnClose(); // File object not created by the user so delete it when the user is finished with it
	}
	else
	{
		delete file;
		file = NULL;
	}

	return (File*) file;
}
