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
// File name:  wsys/StdFile.h
//
// Created:    4/23/01
//
//----------------------------------------------------------------------------

#pragma once

#ifndef __WSYS_STDFILE_H
#define __WSYS_STDFILE_H



//----------------------------------------------------------------------------
//           Includes                                                      
//----------------------------------------------------------------------------

#include "WSYS_file.h"

//----------------------------------------------------------------------------
//           Forward References
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//           Type Defines
//----------------------------------------------------------------------------

//===============================
// StdFile
//===============================
/**
  *	File abstraction for standard C file operators: open, close, lseek, read, write
	*/
//===============================

class StdFile : public File
{
	protected:

		int m_handle;											///< Std C file handle
		
	public:
		
		StdFile();										
		virtual				~StdFile();


		virtual Bool	open( const Char *filename, Int access = 0 );				///< Open a fioe for access
		virtual void	close( void );																			///< Close the file
		virtual Int		read( void *buffer, Int bytes );										///< Read the specified number of bytes in to buffer: See File::read
		virtual Int		write( void *buffer, Int bytes );										///< Write the specified number of bytes from the buffer: See File::write
		virtual Int		seek( Int new_pos, seekMode mode = CURRENT );				///< Set file position: See File::seek

};




//----------------------------------------------------------------------------
//           Inlining                                                       
//----------------------------------------------------------------------------


#endif // __WSYS_STDFILE_H
