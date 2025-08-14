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

// FILE: ImageInfo.h //////////////////////////////////////////////////////////
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
// File name:  ImageInfo.h
//
// Created:    Colin Day, August 2001
//
// Desc:       Image descriptor for the image packer
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __IMAGEINFO_H_
#define __IMAGEINFO_H_

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Lib/BaseType.h"

///////////////////////////////////////////////////////////////////////////////
// FORWARD REFERENCES /////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
class TexturePage;

// TYPE DEFINES ///////////////////////////////////////////////////////////////

// ImageInfo ------------------------------------------------------------------
/** Image file information */
//-----------------------------------------------------------------------------
class ImageInfo
{

public:

	enum
	{
		UNPACKED					= 0x00000001,  ///< this image has not yet been packed
		PACKED						= 0x00000002,  ///< this image has been packed and is done
		TOOBIG						= 0x00000004,  ///< this image was too big to process
		ROTATED90C				= 0x00000008,	 ///< image is fitted after 90 clockwise rotation
		CANTPROCESS				= 0x00000010,  ///< can't be processed
		INVALIDCOLORDEPTH = 0x00000020,  ///< unsupported source image format
	};

	enum
	{
		FIT_XGUTTER					= 0x00000001,  ///< x gutter is present in fit region
		FIT_YGUTTER					= 0x00000002,  ///< y gutter is present in fit region
		FIT_XBORDER_RIGHT		= 0x00000004,  ///< x border of 1 pixel on right side of region
		FIT_XBORDER_LEFT		= 0x00000008,  ///< x border of 1 pixel on left side of region
		FIT_YBORDER_TOP			= 0x00000010,  ///< y border of 1 pixel on top of region
		FIT_YBORDER_BOTTOM	= 0x00000020,  ///< y border of 1 pixel on bottom of image
	};

public:

	ImageInfo();
	~ImageInfo();

	char *m_path;  ///< path to file
	char *m_filenameOnly;  ///< filename with extension only (no path information)
	char *m_filenameOnlyNoExt;  ///< filename without extension and no path info
	Int m_colorDepth;  ///< bits per pixel
	UnsignedInt m_area;  ///< width and height area
	ICoord2D m_size;  ///< width and height of image
	UnsignedInt m_status;  ///< status bits for image

	TexturePage *m_page;  ///< pointer to page this image is now packed on
	ImageInfo *m_nextPageImage;  ///< next image on texture page
	ImageInfo *m_prevPageImage;  ///< previous image on texture page
	IRegion2D m_pagePos;  /** once placed on a texture page this has the
												coords of the image on the page, it does not include
												any padding from stretching borders and gutters */

	UnsignedInt m_fitBits;  /**< bit flags of the region used to fit this image on
													a page and therefore to create m_pagePos */
	ICoord2D m_gutterUsed;  ///< the gutter size actually used in this image fit

};

// INLINING ///////////////////////////////////////////////////////////////////

// EXTERNALS //////////////////////////////////////////////////////////////////

#endif // __IMAGEINFO_H_

