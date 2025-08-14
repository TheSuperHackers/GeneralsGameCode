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

// FILE: ImageDirectory.h /////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:    ImagePacker
//
// File name:  ImageDirectory.h
//
// Created:    Colin Day, August 2001
//
// Desc:       Image directory description for directories containing
//						 image files to pack
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __IMAGEDIRECTORY_H_
#define __IMAGEDIRECTORY_H_

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////

// FORWARD REFERENCES /////////////////////////////////////////////////////////

// TYPE DEFINES ///////////////////////////////////////////////////////////////

// ImageDirectory -------------------------------------------------------------
/** Directory to contain art files */
//-----------------------------------------------------------------------------
class ImageDirectory
{

public:

	ImageDirectory();
	~ImageDirectory();

	char *m_path;  ///< path for directory
	UnsignedInt m_imageCount;  ///< images to consider in this directory
	ImageDirectory *m_next;
	ImageDirectory *m_prev;

};

///////////////////////////////////////////////////////////////////////////////
// INLINING ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
inline ImageDirectory::~ImageDirectory( void ) { delete m_path; }
inline ImageDirectory::ImageDirectory( void )
{
	m_path = NULL;
	m_next = NULL;
	m_prev = NULL;
	m_imageCount = 0;
}

// EXTERNALS //////////////////////////////////////////////////////////////////

#endif // __IMAGEDIRECTORY_H_

