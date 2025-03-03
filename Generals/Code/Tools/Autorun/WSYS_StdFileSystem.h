/*
**	Command & Conquer Generals(tm)
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

//----------------------------------------------------------------------------=
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information					                  
//                Copyright(C) 2001 - All Rights Reserved                  
//                                                                          
//----------------------------------------------------------------------------
//
// Project:    WSYS Library
//
// Module:     IO
//
// File name:  wsys/StdFileSystem.h
//
// Created:    
//
//----------------------------------------------------------------------------

#pragma once

#ifndef __WSYS_STDFILESYSTEM_H
#define __WSYS_STDFILESYSTEM_H



//----------------------------------------------------------------------------
//           Includes                                                      
//----------------------------------------------------------------------------

#ifndef __WSYS_FILE_H
#include "WSYS_file.h"
#endif

#ifndef __WSYS_FILESYSTEM_H
#include "WSYS_FileSystem.h"
#endif


//----------------------------------------------------------------------------
//           Forward References
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//           Type Defines
//----------------------------------------------------------------------------

//===============================
// StdFileSystem
//===============================
/**
  *	FileSystem that maps directly to StdFile files.
	*/
//===============================

class StdFileSystem	: public FileSystem
{

	public:

		virtual					~StdFileSystem();
		virtual	File*		open( const Char *filename, Int access = 0 );		///< Creates a StdFile object and opens the file with it: See FileSystem::open


};

//----------------------------------------------------------------------------
//           Inlining                                                       
//----------------------------------------------------------------------------



#endif // __WSYS_STDFILESYSTEM_H
