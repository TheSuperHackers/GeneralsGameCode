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

// FILE: ImageInfo.cpp ////////////////////////////////////////////////////////
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
// File name:  ImageInfo.cpp
//
// Created:    Colin Day, August 2001
//
// Desc:       Image information struct for images to pack into
//						 the texture pages
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdlib.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "ImageInfo.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// ImageInfo::ImageInfo =======================================================
/** */
//=============================================================================
ImageInfo::ImageInfo( void )
{

	m_area = 0;
	m_colorDepth = 0;
	m_size.x = 0;
	m_size.y = 0;
	m_path = NULL;
	m_filenameOnly = NULL;
	m_filenameOnlyNoExt = NULL;
	m_status = UNPACKED;

	m_page = NULL;
	m_nextPageImage = NULL;
	m_prevPageImage = NULL;
	m_pagePos.lo.x = 0;
	m_pagePos.lo.y = 0;
	m_pagePos.hi.x = 0;
	m_pagePos.hi.y = 0;
	m_fitBits			 = 0;
	m_gutterUsed.x = 0;
	m_gutterUsed.y = 0;

}  // end ImageInfo

// ImageInfo::~ImageInfo ======================================================
/** */
//=============================================================================
ImageInfo::~ImageInfo( void )
{ 
	
	// delete path name
	if( m_path )
		delete [] m_path; 

	if( m_filenameOnly )
		delete [] m_filenameOnly;

	if( m_filenameOnlyNoExt )
		delete [] m_filenameOnlyNoExt;

}  // end ~ImageInfo
