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
// File name:  wsys/FileSystem.h
//
// Created:    
//
//----------------------------------------------------------------------------

#pragma once

#ifndef __WSYS_FILESYSTEM_H
#define __WSYS_FILESYSTEM_H



//----------------------------------------------------------------------------
//           Includes                                                      
//----------------------------------------------------------------------------

#ifndef __WSYS_FILE_H
#include "WSYS_file.h"
#endif


//----------------------------------------------------------------------------
//           Forward References
//----------------------------------------------------------------------------


//----------------------------------------------------------------------------
//           Type Defines
//----------------------------------------------------------------------------

//===============================
// FileSystem
//===============================
/**
  * FileSystem is an interface class for creating specific FileSystem objects.
  * 
	* A FileSystem object's implemenation decides what derivative of File object needs to be 
	* created when FileSystem::Open() gets called.
	*/
//===============================

class FileSystem
{
	protected:

	public:

		virtual					~FileSystem() {};
		virtual	File*		open( const Char *filename, Int access = 0 ) = NULL ;		///< opens a File interface to the specified file


};

extern FileSystem*	TheFileSystem;



//----------------------------------------------------------------------------
//           Inlining                                                       
//----------------------------------------------------------------------------



#endif // __WSYS_FILESYSTEM_H
