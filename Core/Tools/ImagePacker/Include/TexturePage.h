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

// FILE: TexturePage.h ////////////////////////////////////////////////////////
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
// File name:  TexturePage.h
//
// Created:    Colin Day, August 2001
//
// Desc:			 This class represents a texture that contains packed
//						 images.
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __TEXTUREPAGE_H_
#define __TEXTUREPAGE_H_

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////
// USER INCLUDES //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#include "WWLib/TARGA.H"
#include "Lib/BaseType.h"
#include "ImageInfo.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// TYPE DEFINES ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// TexturePage ----------------------------------------------------------------
/** A texture page continaing multiple source images */
//-----------------------------------------------------------------------------
class TexturePage
{

public:

	enum
	{
		FREE = 0,		 ///< open pixel in the cavas
		USED = 1,    ///< used pixel in the canvas
	};

	enum
	{
		READY													= 0x00000001,  ///< texture page here and OK
		PAGE_ERROR										= 0x00000002,  ///< error on page somewhere
		CANT_ALLOCATE_PACKED_IMAGE		= 0x00000004,  ///< couldn't generate final image
		CANT_ADD_IMAGE_DATA						= 0x00000008,  ///< couldn't add image data to page
		NO_TEXTURE_DATA								= 0x00000010,  ///< there was no image data to write
		ERROR_DURING_SAVE							= 0x00000020,  ///< couldn't save final file
	};

	TexturePage( Int width, Int height );
	~TexturePage( void );

	Bool addImage( ImageInfo *image );  ///< try to add image to this page

	void setID( Int id );  ///< set page id
	Int getID( void );  ///< get page id

	Bool generateTexture( void );  ///< generate the final packed texture
	Bool writeFile( char *baseFilename );  ///< write generated texture to file

	ImageInfo *getFirstImage( void );  ///< get the first image in the list

	Int getWidth( void );  ///< get width of texture page
	Int getHeight( void );  ///< get height of texture page

	// get rgb from final generated texture (putting this in for quick preview)
	void getPixel( Int x, Int y, Byte *r, Byte *g, Byte *b, Byte *a = NULL );

	TexturePage *m_next;
	TexturePage *m_prev;

	UnsignedInt m_status;  ///< status bits

protected:

	Bool spotUsed( Int x, Int y );  ///< is this spot used
	Bool lineUsed( Int sx, Int sy, Int ex, Int ey );  ///< is any spot on the line used

	/// build a region to try to fit given the position, size, and border options
	UnsignedInt buildFitRegion( IRegion2D *region,
															Int startX, Int startY,
															Int imageWidth, Int imageHeight,
															Int *xGutter, Int *yGutter,
															Bool allSidesBorder );

	void markRegionUsed( IRegion2D *region );  ///< mark this region as used

	/// add the actual image data of 'image' to the destination buffer
	Bool addImageData( Byte *destBuffer,
										 Int destWidth,
										 Int destHeight,
										 Int destBPP,
										 ImageInfo *image );

	/// extend edges of image outward into border if present
	void extendImageEdges( Byte *destBuffer,
												 Int destWidth,
												 Int destHeight,
												 Int destBPP,
												 ImageInfo *image,
												 Bool extendAlpha );

	/// if the pixel at abolve/below row is open, extend pixel at src to its location
	void extendToRowIfOpen( char *src,
													Int buffWidth,
													Int buffBPP,
													Bool extendAlpha,
													Int imageHeight,
													UnsignedInt fitBits,
													Int srcX, Int srcY );


	Int m_id;  ///< texture page ID
	ICoord2D m_size;  ///< dimensions of texture page
	UnsignedByte *m_canvas;  ///< as big as the texture page, a used spot is non zero

	ImageInfo *m_imageList;  ///< list of images packed on this page

	Byte *m_packedImage;  ///< final generated image data
	Targa *m_targa;  ///< final packed image all in a nice little targa file

};

///////////////////////////////////////////////////////////////////////////////
// INLINING ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
inline void TexturePage::setID( Int id ) { m_id = id; }
inline Int TexturePage::getID( void ) { return m_id; }
inline ImageInfo *TexturePage::getFirstImage( void ) { return m_imageList; }
inline Int TexturePage::getWidth( void ) { return m_size.x; }
inline Int TexturePage::getHeight( void ) { return m_size.y; }

// EXTERNALS //////////////////////////////////////////////////////////////////

#endif // __TEXTUREPAGE_H_

