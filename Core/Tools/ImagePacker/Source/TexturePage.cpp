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

// FILE: TexturePage.cpp //////////////////////////////////////////////////////
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
// File name:  TexturePage.cpp
//
// Created:    Colin Day, August 2001
//
// Desc:			 This class represents a texture that contains packed
//						 images.
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/Debug.h"
#include "TexturePage.h"
#include "ImagePacker.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// TexturePage::extendToRowIfOpen =============================================
/** If the pixel at location 'row' is "open", then extend the pixel
	* at 'src' to that location
	*
	* NOTE this assumes 'src' and 'row' are pointers into the same
	* buffer and the bits per pixel (buffBPP) are treated as the
	* same for both */
//=============================================================================
void TexturePage::extendToRowIfOpen( char *src,
																		 Int buffWidth,
																		 Int buffBPP,
																		 Bool extendAlpha,
																		 Int imageHeight,
																		 UnsignedInt fitBits,
																		 Int srcX, Int srcY )
{
	char otherAlpha;
	char otherColor[ 3 ];
	char *row = NULL;

	// sanity
	if( src == NULL )
		return;

	//
	// try to extend pixel up and down a row as well, we try to extend
	// up if we're in the top half of the image down if we're
	// in the bottom half of the image.  Note we're not
	// allowed to extend outside the image region if we don't have
	// a border to extend into
	//
	if( srcY < imageHeight / 2 &&
			(srcY != 0 || BitIsSet( fitBits,	ImageInfo::FIT_YBORDER_TOP )) )
	{

		// try to extend pixel "up" if that pixel is "open"
		row = src + (buffWidth * buffBPP);


	}  // end if
	else if( srcY >= imageHeight / 2 &&
					 (srcY != imageHeight - 1 ||
						BitIsSet( fitBits, ImageInfo::FIT_YBORDER_BOTTOM )) )
	{

		// try to extend pixel "down" if that pixel is "open"
		row = src - (buffWidth * buffBPP);

	}  // end else

	//
	// if a 'row' is available, try to extend the current pixel
	// into that location if it's open
	//
	if( row )
	{

		// read the pixel at 'row'
		if( buffBPP == 4 )
		{

			otherAlpha = row[ 0 ];
			otherColor[ 0 ] = row[ 1 ];
			otherColor[ 1 ] = row[ 2 ];
			otherColor[ 2 ] = row[ 3 ];

		}  // end if
		else
		{

			otherColor[ 0 ] = row[ 0 ];
			otherColor[ 1 ] = row[ 1 ];
			otherColor[ 2 ] = row[ 2 ];

		}  // end else

		//
		// see if this pixel is "open", again we prefer to check the
		// alpha channel if present, otherwise we say black is "open"
		//
		Bool otherOpen = FALSE;
		if( buffBPP == 4 )
		{

			if( otherAlpha == 0 )
				otherOpen = TRUE;

		}  // end if
		else
		{

			if( otherColor[ 0 ] == 0 &&
					otherColor[ 1 ] == 0 &&
					otherColor[ 2 ] == 0 )
				otherOpen = TRUE;

		}  // end else

		// copy pixel data from 'src' to 'row' if 'row' is "open"
		if( otherOpen == TRUE )
		{
			char alpha;
			char color[ 3 ];

			// read the pixel data at 'src'
			if( buffBPP == 4 )
			{

				alpha = src[ 0 ];
				color[ 0 ] = src[ 1 ];
				color[ 1 ] = src[ 2 ];
				color[ 2 ] = src[ 3 ];

			}  // end if
			else
			{

				color[ 0 ] = src[ 0 ];
				color[ 1 ] = src[ 1 ];
				color[ 2 ] = src[ 2 ];

			}  // end else

			// copy the pixel to 'row'
			if( buffBPP == 4 )
			{

				if( extendAlpha )
					row[ 0 ] = alpha;
				row[ 1 ] = color[ 0 ];
				row[ 2 ] = color[ 1 ];
				row[ 3 ] = color[ 2 ];

			}  // end if
			else
			{

				row[ 0 ] = color[ 0 ];
				row[ 1 ] = color[ 1 ];
				row[ 2 ] = color[ 2 ];

			}  // end else

		}  // end if, other spot is open, copy it

	}  // end if, row

}  // end extendToRowIfOpen

// TexturePage::extendImageEdges ==============================================
/** We want to extend the image data in destBuffer at the location region
	* of image->m_pagePos in "outward" directions, effectively bleeding
	* the edges outward.
	*
	* Note we will not extend outward from the image region UNLESS we
	* have a border present on that side (described in the image->m_fitBits
	* for the region image->m_pagePos).
	*/
//=============================================================================
void TexturePage::extendImageEdges( Byte *destBuffer,
																	  Int destWidth,
																	  Int destHeight,
																	  Int destBPP,
																	  ImageInfo *image,
																		Bool extendAlpha )
{

	// sanity
	if( destBuffer == NULL || image == NULL )
		return;

	//
	// get the extents that we will loop through on the destination surface,
	// those extents are the size of the image, but we have to take into
	// account whether or not the destination image was rotated or not
	//
	Int imageWidth, imageHeight;
	if( BitIsSet( image->m_status, ImageInfo::ROTATED90C ) )
	{

		imageWidth = image->m_size.y;
		imageHeight = image->m_size.x;

	}  // end if
	else
	{

		imageWidth = image->m_size.x;
		imageHeight = image->m_size.y;

	}  // end else

	Int x, y;
	char *ptr;
	char color[ 3 ];
	char alpha;
	Bool prevPixel, currPixel;
	for( y = 0; y < imageHeight; y++ )
	{

		// compute beginning of destination row
		ptr = destBuffer +
				  ((destHeight - 1 - (image->m_pagePos.lo.y + y)) * destWidth * destBPP ) +
				  (image->m_pagePos.lo.x * destBPP);

		prevPixel = FALSE;
		for( x = 0; x < imageWidth; x++ )
		{

			// read the pixel
			if( destBPP == 4 )
			{

				alpha = ptr[ 0 ];
				color[ 0 ] = ptr[ 1 ];
				color[ 1 ] = ptr[ 2 ];
				color[ 2 ] = ptr[ 3 ];

			}  // end if
			else
			{

				color[ 0 ] = ptr[ 0 ];
				color[ 1 ] = ptr[ 1 ];
				color[ 2 ] = ptr[ 2 ];

			}  // end else

			//
			// see wheter or not we have data at this pixel, if we have alpha
			// we will use just the mask comparison, if not we will compare the
			// colors with black (0,0,0) being an "empty" pixel
			//
			currPixel = FALSE;
			if( destBPP == 4 )
			{

				if( alpha != 0 )
					currPixel = TRUE;

			}  // end if
			else
			{

				if( color[ 0 ] != 0 &&
						color[ 1 ] != 0 &&
						color[ 2 ] != 0 )
					currPixel = TRUE;

			}  // end else

			//
			// if we're at the right edge we will extend this pixel off the
			// image to the right border if present, we dont' have to worry about
			// the top and bottom edges because they are attempted to be
			// extended into when we detect an edge change moving across x
			//
			if( currPixel == TRUE && x == imageWidth - 1 &&
					BitIsSet( image->m_fitBits, ImageInfo::FIT_XBORDER_RIGHT ) )
			{

				//
				// we are at the right side of the image, we have a pixel here,
				// AND we have a border to extend that pixel into
				//
				if( destBPP == 4 )
				{

					if( extendAlpha )
						*(ptr + 4) = alpha;
					*(ptr + 5) = color[ 0 ];
					*(ptr + 6) = color[ 1 ];
					*(ptr + 7) = color[ 2 ];

				}  // end if
				else
				{

					*(ptr + 3) = color[ 0 ];
					*(ptr + 4) = color[ 1 ];
					*(ptr + 5) = color[ 2 ];

				}  // end else

			}  // end if

			//
			// if we have a pixel here, attempt to extend it to the above
			// or below row if that spot is empty
			//
			if( currPixel == TRUE )
				extendToRowIfOpen( ptr, destWidth, destBPP, extendAlpha,
													 imageHeight, image->m_fitBits, x, y );

			//
			// if we've crossed from empty<->filled either extend that pixel
			// left or right
			//
			if( prevPixel == FALSE && currPixel == TRUE )
			{

				//
				// we've crossed from empty to filled, copy the color of current
				// pixel to the position of previous pixel.  Note this is not allowed
				// when we're at the left edge and we DON'T have a border to copy into
				//
				if( x != 0 || BitIsSet( image->m_fitBits, ImageInfo::FIT_XBORDER_LEFT ) )
				{

					// extend our current pixel to the previous location
					if( destBPP == 4 && extendAlpha )
							*(ptr - 4) = alpha;
					*(ptr - 3) = color[ 0 ];
					*(ptr - 2) = color[ 1 ];
					*(ptr - 1) = color[ 2 ];

				}  // end if

			}  // end if
			else if( prevPixel == TRUE && currPixel == FALSE )
			{

				//
				// we've crossed from filled to empty, copy the color of previous
				// pixel to the current pixel position.
				//

				//
				// this assert should never happen because if x were 0, we are on the
				// first column in this image, and the prevPixel should be FALSE since
				// previous would be "off" the image which is by definition "open"
				//
				DEBUG_ASSERTCRASH( x != 0, ("Coming from off image and detecting right edge!") );

				// extend the previous pixel to this location
				if( destBPP == 4 )
				{

					if( extendAlpha )
						ptr[ 0 ] = *(ptr - 4);
					ptr[ 1 ] = *(ptr - 3);
					ptr[ 2 ] = *(ptr - 2);
					ptr[ 3 ] = *(ptr - 1);

				}  // end if
				else
				{

					ptr[ 0 ] = *(ptr - 3);
					ptr[ 1 ] = *(ptr - 2);
					ptr[ 2 ] = *(ptr - 1);

				}  // end else

			}  // end else if

			//
			// one more time now for a special case in the corners of the extended
			// image.  since this algorithm goes across extending pixels left, right
			// up, and down we must check to see if we have a pixel when our
			// source location is in the 4 corners of the image ... if so we will
			// extend that pixel DIAGONALLY out to make a complete extended rectangle
			//
			if( currPixel == TRUE )
			{
				char *dst = NULL;

				// top left corner
				if( x == 0 && y == 0 &&
						BitIsSet( image->m_fitBits, ImageInfo::FIT_XBORDER_LEFT ) &&
						BitIsSet( image->m_fitBits, ImageInfo::FIT_YBORDER_TOP ) )
					dst = (ptr + (destWidth * destBPP)) - destBPP;

				// top right corner
				else if( x == imageWidth - 1 && y == 0 &&
								 BitIsSet( image->m_fitBits, ImageInfo::FIT_XBORDER_RIGHT ) &&
								 BitIsSet( image->m_fitBits, ImageInfo::FIT_YBORDER_TOP ) )
					dst = (ptr + (destWidth * destBPP)) + destBPP;

				// bottom right corner
				else if( x == imageWidth - 1 && y == imageHeight - 1 &&
								 BitIsSet( image->m_fitBits, ImageInfo::FIT_XBORDER_RIGHT ) &&
								 BitIsSet( image->m_fitBits, ImageInfo::FIT_YBORDER_BOTTOM ) )
					dst = (ptr - (destWidth * destBPP)) + destBPP;

				// bottom left corner
				else if( x == 0 && y == imageHeight - 1 &&
								 BitIsSet( image->m_fitBits, ImageInfo::FIT_XBORDER_LEFT ) &&
								 BitIsSet( image->m_fitBits, ImageInfo::FIT_YBORDER_BOTTOM ) )
					dst = (ptr - (destWidth * destBPP)) - destBPP;

				// copy the pixel at 'ptr' to 'dst' for the diagonal extend
				if( dst )
				{

					if( destBPP == 4 )
					{

						if( extendAlpha )
							dst[ 0 ] = alpha;
						dst[ 1 ] = color[ 0 ];
						dst[ 2 ] = color[ 1 ];
						dst[ 3 ] = color[ 2 ];

					}  // end if
					else
					{

						dst[ 0 ] = color[ 0 ];
						dst[ 1 ] = color[ 1 ];
						dst[ 2 ] = color[ 2 ];

					}  // end else

				}  // end if dst

			}  // end if

			// move to the next pixel
			ptr += destBPP;

			//
			// the state of our current pixel (on/off) becomes the state of the
			// previous pixel now
			//
			prevPixel = currPixel;

		}  // end for x

	}  // end for y

}  // end extendImageEdges

// TexturePage::addImageData ==================================================
/** Add the actual image data from 'image' to the destination buffer
	* at the coordinates specified in the 'image' ... this puts the
	* actual packed image data on the final texture page
	*
	* NOTE that we have created our texture page regions with the
	* assumption that we were packing images with an upper left
	* corner at (0,0), but the targa files have the origin in the
	* lower left corner ... thus the translation here to shift source
	* images into the right positions
	*/
//=============================================================================
Bool TexturePage::addImageData( Byte *destBuffer,
																Int destWidth,
																Int destHeight,
																Int destBPP,
																ImageInfo *image )
{

	// sanity
	if( destBuffer == NULL || image == NULL )
		return FALSE;

	// load the real image data for the source
	Targa source;
	if( source.Load( image->m_path, TGAF_IMAGE, FALSE ) != 0 )
	{
		char buffer[ _MAX_PATH + 32 ];

		sprintf( buffer, "Error loading source file '%s'\n", image->m_path );
		DEBUG_ASSERTCRASH( 0, (buffer) );
		MessageBox( NULL, buffer, "Cannot Load Source File", MB_OK | MB_ICONERROR );
		return FALSE;

	}  // end if

	// get the source image buffer
	char *sourceBuffer = source.GetImage();
	DEBUG_ASSERTCRASH( sourceBuffer, ("No Source buffer for source image") );

	// get the source bytes per pixel
	Int sourceBPP = TGA_BytesPerPixel( source.Header.PixelDepth );

	//
	// the loaded targas are all laid out flat with no encoding, copy
	// all the rows in the source to the destination buffer at the coords
	// specified in the images' page location
	//
	char *src, *dest;
	Int count;
	Int x, y;

	if( BitIsSet( image->m_status, ImageInfo::ROTATED90C ) == FALSE )
	{

		//
		// normal copy, image was not rotated
		//

		// do all rows
		for( y = 0; y < image->m_size.y; y++ )
		{

			// compute source location
			src = sourceBuffer + ( (image->m_size.y - 1 - y) * image->m_size.x * sourceBPP);

			// compute destination location
			dest = destBuffer +
						 ((destHeight - 1 - (image->m_pagePos.lo.y + y)) * destWidth * destBPP ) +
						 (image->m_pagePos.lo.x * destBPP);

			// copy a row from source to destination
			count = image->m_pagePos.hi.x - image->m_pagePos.lo.x + 1;
			for( x = 0; x < count; x++ )
			{

				// check the target and source formats
				if( destBPP == 4 )
				{

					// copy the rgb
					dest[ 3 ] = src[ 0 ];
					dest[ 2 ] = src[ 1 ];
					dest[ 1 ] = src[ 2 ];

					// copy the alpha if present in the source
					if( sourceBPP == 4 )
						dest[ 0 ] = src[ 3 ];
					else
						dest[ 0 ] = (char)0xFF;  // solid alpha

				}  // end if
				else
				{

					// copy the rgb
					dest[ 2 ] = src[ 0 ];
					dest[ 1 ] = src[ 1 ];
					dest[ 0 ] = src[ 2 ];

				}  // end else

				// skip past all these pixels
				dest += destBPP;
				src += sourceBPP;

			}  // end for x

		}  // end for y

	}  // end if, not rotated
	else
	{

		//
		// image was rotated, perform a 90 degrees rotation clockwise when we
		// copy over the image data
		//
		for( y = 0; y < image->m_size.y; y++ )
		{


			// compute beginning of source row to copy from
			src = sourceBuffer + ((image->m_size.y - 1 - y) * image->m_size.x * sourceBPP);

			// for each pixel in source put it in dest rotated
			for( x = 0; x < image->m_size.x; x++ )
			{

				// compute destination location
				dest = destBuffer +
							 ( ( (destHeight - 1) - (image->m_pagePos.lo.y + x) ) * destWidth * destBPP ) +
							 ((image->m_pagePos.lo.x + (image->m_size.y - 1 - y)) * destBPP);

				// copy this pixel, checking target and source formats
				if( destBPP == 4 )
				{

					// copy the rgb
					dest[ 3 ] = src[ 0 ];
					dest[ 2 ] = src[ 1 ];
					dest[ 1 ] = src[ 2 ];

					// copy the alpha if present in the source
					if( sourceBPP == 4 )
						dest[ 0 ] = src[ 3 ];
					else
						dest[ 0 ] = (char)0xFF;  // solid alpha

				}  // end if
				else
				{

					// copy the rgb
					dest[ 2 ] = src[ 0 ];
					dest[ 1 ] = src[ 1 ];
					dest[ 0 ] = src[ 2 ];

				}  // end else

				// skip past all these pixels
				dest += destBPP;
				src += sourceBPP;

			}  // end for x

		}  // end for y

	}  // end else

	//
	// if we have the option to extend the RGB edges on we now need to process
	// the image we just copied into the texture page and "bleed" the edges outward
	// and if a border is present, into the border
	//
	if( BitIsSet( TheImagePacker->getGapMethod(),
							 ImagePacker::GAP_METHOD_EXTEND_RGB ) )
		extendImageEdges( destBuffer,
											destWidth,
											destHeight,
											destBPP,
											image,
											FALSE );

	return TRUE;  // all done

}  // end addImageData

// TexturePage::spotUsed ======================================================
/** Is this spot in the texture page open? */
//=============================================================================
Bool TexturePage::spotUsed( Int x, Int y )
{

	return m_canvas[ y * m_size.y + x ];

}  // end spotUsed

// TexturePage::lineUsed ======================================================
/** Is there ANY spot in the line specified that is used */
//=============================================================================
Bool TexturePage::lineUsed( Int sx, Int sy, Int ex, Int ey )
{
	Int x, y;
	UnsignedByte *ptr;

	for( y = sy; y <= ey; y++ )
	{

		// compute start of row
		ptr = m_canvas + (y * m_size.y + sx);

		// scan the row
		for( x = sx; x <= ex; x++, ptr++ )
			if( *ptr == USED )
				return USED;

	}  // end for y

	return FALSE;  // it's open!

}  // end lineUsed

// TexturePage::markRegionUsed ================================================
/** Mark this region as used */
//=============================================================================
void TexturePage::markRegionUsed( IRegion2D *region )
{
	UnsignedByte *ptr;
	Int y;
	Int count;

	// loop through y
	for( y = region->lo.y; y <= region->hi.y; y++ )
	{

		// compute start of row
		ptr = m_canvas + (y * m_size.y + region->lo.x);

		// fill this row
		count = (region->hi.x - region->lo.x) + 1;
		memset( ptr, USED, count );

	}  // end for

}  // end markRegionUsed

// TexturePage::buildFitRegion ================================================
/** Build an image region to try to fit into the page based on the location
	* given, with the image size, the gutter sizes, and the all sides border
	* size
	*
	* Note that x and y Gutter sizes can be changed as a result of this
	* method
	*
	* Returns a set of "fit bits" that describe what the components
	* have been included in the region constructed
	*/
//=============================================================================
UnsignedInt TexturePage::buildFitRegion( IRegion2D *region,
																				 Int startX, Int startY,
																				 Int imageWidth, Int imageHeight,
																				 Int *xGutter, Int *yGutter,
																				 Bool allSidesBorder )
{

	// sanity
	if( region == NULL || xGutter == NULL || yGutter == NULL )
		return 0;

	//
	// create border size, if we have an 'allSidesBorder' then we need to
	// add two pixels to width and height
	//
	Int xBorder = 0, yBorder = 0;
	if( allSidesBorder )
	{

		xBorder = 2;
		yBorder = 2;

	}  // end if

	//
	// when the image size exactly matches the target size of the texture
	// page the region will not include any gutter or border sizes cause
	// we can fit the image exactly as it is on the page
	//
	if( imageWidth == m_size.x )
	{

		*xGutter = 0;
		xBorder = 0;

	}  // end if
	if( imageHeight == m_size.y )
	{

		*yGutter = 0;
		yBorder = 0;

	}  // end if

	//
	// when an image is 1 pixel smaller than the destination texture
	// page we can eliminate some borders used to stretch the RGB values
	// at the edges because we will be hitting one side of the texture
	// anyway.  We will say these borders that are only on one side
	// will be on the right and bottom (as described in the fit bits
	// returned below)
	//
	if( imageWidth == m_size.x - 1 )
		xBorder = 1;
	if( imageHeight == m_size.y - 1 )
		yBorder = 1;

	// build the region
	region->lo.x = startX;
	region->lo.y = startY;
	region->hi.x = startX + imageWidth - 1 + *xGutter + xBorder;  // -1 for zero based
	region->hi.y = startY + imageHeight - 1 + *yGutter + yBorder;  // -1 for zero based

	//
	// build a set of region bit flags that tell what this region ACTUALLY
	// used in its construction. Note that when we stripped off only ONE
	// pixel off one dimension when talking about borders, we say that the
	// border that we added was either on the right side of the image,
	// or on the bottom of the image
	//
	UnsignedInt fitBits = 0;
	if( *xGutter != 0 )
		BitSet( fitBits, ImageInfo::FIT_XGUTTER );
	if( *yGutter != 0 )
		BitSet( fitBits, ImageInfo::FIT_YGUTTER );
	if( xBorder >= 1 )
		BitSet( fitBits, ImageInfo::FIT_XBORDER_RIGHT );
	if( xBorder == 2 )
		BitSet( fitBits, ImageInfo::FIT_XBORDER_LEFT );
	if( yBorder >= 1 )
		BitSet( fitBits, ImageInfo::FIT_YBORDER_BOTTOM );
	if( yBorder == 2 )
		BitSet( fitBits, ImageInfo::FIT_YBORDER_TOP );

	return fitBits;

}  // end buildFitRegion

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// TexturePage::TexturePage ===================================================
/** */
//============================================================================
TexturePage::TexturePage( Int width, Int height )
{
	Int canvasSize;

	m_id = -1;
	m_next = NULL;
	m_prev = NULL;
	m_size.x = width;
	m_size.y = height;
	m_packedImage = NULL;
	m_targa = NULL;

	// create a "canvas" to represent used and unused areas
	canvasSize = m_size.x * m_size.y;
	m_canvas = new UnsignedByte[ canvasSize ];
	DEBUG_ASSERTCRASH( m_canvas, ("Cannot allocate canvas for texture page") );
	memset( m_canvas, FREE, sizeof( UnsignedByte ) * canvasSize );

}  // end TexturePage

// TexturePage::~TexturePage ==================================================
/**  */
//=============================================================================
TexturePage::~TexturePage( void )
{

	// delete the canvas
	if( m_canvas )
		delete [] m_canvas;

	// delete targa if present, this will NOT delete a user assigned image buffer
	if( m_targa )
		delete m_targa;

	// delete the final image buffer if present
	if( m_packedImage )
		delete [] m_packedImage;

}  // end ~TexturePage

// TexturePage::addImage ======================================================
/** If this image will fit on this page, add it */
//=============================================================================
Bool TexturePage::addImage( ImageInfo *image )
{
	IRegion2D region;

	// santiy
	if( image == NULL )
	{

		DEBUG_ASSERTCRASH( image, ("TexturePage::addImage: NULL image!") );
		return TRUE;  // say it was added

	}  // end if

	// get our options for fitting
	Bool useGutter, useRGBExtend;
	useGutter = BitIsSet( TheImagePacker->getGapMethod(),
											 ImagePacker::GAP_METHOD_GUTTER );
	useRGBExtend = BitIsSet( TheImagePacker->getGapMethod(),
													ImagePacker::GAP_METHOD_EXTEND_RGB );

	//
	// try to fit this image in this page ... we have two tries, once
	// normally, and once with the image rotated 90 degrees clockwise
	//
	Int triesLeft = 2;
	Bool tryRotate = FALSE;
	while( triesLeft )
	{

		// try the rotated image the second time around
		if( triesLeft == 1 )
			tryRotate = TRUE;

		// we've now 'used up' a try
		triesLeft--;

		// find a free upper left corner to put the image in
		Int x, y;
		Int xGutter = 0, yGutter = 0;
		Int imageWidth, imageHeight;
		UnsignedInt fitBits = 0;
		for( y = 0; y < m_size.y; y++ )
		{

			for( x = 0; x < m_size.x; x++ )
			{

				// get the gutter size
				if( useGutter )
				{

					xGutter = TheImagePacker->getGutter();
					yGutter = TheImagePacker->getGutter();

				}  // end if
				else
				{

					xGutter = 0;
					yGutter = 0;

				}  // end else

				//
				// compute the region of the image at this location, the region that will
				// be used will take up the image region AND the gutter.  UNLESS the
				// image is as big as the texture page, in that case there is no reason
				// to use a gutter size.  Also note that if we're trying to fit
				// a rotated image this time we have to swap the coords
				// around a little bit
				//
				if( tryRotate == FALSE )
				{

					// normal, non-rotated image
					imageWidth = image->m_size.x;
					imageHeight = image->m_size.y;

				}  // end if
				else
				{

					//
					// build region for rotation 90 degrees clockwise
					//
					// 1------------2
					// |            |
					// 3------------4
					//
					//     becomes
					//
					//      3--1
					//      |  |
					//      |  |
					//      |  |
					//      |  |
					//      |  |
					//      4--2
					//

					imageWidth = image->m_size.y;
					imageHeight = image->m_size.x;

				}  // end else

				// build the region
				fitBits = buildFitRegion( &region, x, y,
																	imageWidth, imageHeight,
																	&xGutter, &yGutter,
																	useRGBExtend );

				//
				// if the image region plus the gutter goes off the image page, BUT
				// the image itself stays on the page, adjust the gutter to only
				// be as big as from the end of the image to the end of the page,
				// this technically doesn't fill the requirements of making a gutter
				// around every image, but it's OK since that space will be
				// designated as filled, and at that will be filled with
				// transparent alpha - nothingness!
				//
				if( region.hi.x >= m_size.x )
				{

					//
					// attempt to shrink x gutter if image can still fit with a
					// smaller gutter to the right
					//
					if( region.hi.x - xGutter < m_size.x )
						xGutter = m_size.x - imageWidth;

					// rebuild region with new gutter size
					fitBits = buildFitRegion( &region, x, y,
																		imageWidth, imageHeight,
																		&xGutter, &yGutter,
																		useRGBExtend );


				}  // end if
				if( region.hi.y >= m_size.y )
				{

					//
					// attempt to shrink y gutter if image can still fit with
					// a smaller gutter below
					//
					if( region.hi.y - yGutter < m_size.y )
						yGutter = m_size.y - imageHeight;

					// rebuild region with new gutter size
					fitBits = buildFitRegion( &region, x, y,
																		imageWidth, imageHeight,
																		&xGutter, &yGutter,
																		useRGBExtend );

				}  // end if

				// reject this location if the hi region goes off the texture page
				if( region.hi.y >= m_size.y )
				{

					y = m_size.y;  // skip to end, this isn't gonna work
					continue;

				}  // end if
				if( region.hi.x >= m_size.x )
				{

					x = m_size.x;  // skip to end of row to try next row
					continue;

				}  // end if

				//
				// reject this location if any of the corners are in used spots,
				// note since we're trying to fit things by "sliding" them
				// horizontally ... we cut out more checks if we see if the horizontal
				// bounds are full at the right side first ... if they are we
				// move our "anchor" point to try to fit again to after that
				// point (saving the checking of between the current upper
				// left anchor point and the right side point that is used anyway
				//

				// upper right and lower right
				if( spotUsed( region.hi.x, region.lo.y ) ||  // upper right
						spotUsed( region.hi.x, region.hi.y ) )   // lower right
				{

					x = region.hi.x;  // next anchor spot will be to the right of here
					continue;

				}  // end if

				// upper left and lower left
				if( spotUsed( region.lo.x, region.lo.y ) ||  // upper left
						spotUsed( region.lo.x, region.hi.y ) )	 // lower left
					continue;

				//
				// reject this location if we cross any used locations between
				// the 4 corners
				//
				if( lineUsed( region.lo.x, region.lo.y,
											region.hi.x, region.lo.y ) || // top side
						lineUsed( region.hi.x, region.lo.y,
											region.hi.x, region.hi.y ) || // right side
						lineUsed( region.lo.x, region.hi.y,
											region.hi.x, region.hi.y ) || // bottom side
						lineUsed( region.lo.x, region.lo.y,
											region.lo.x, region.hi.y ) )  // left side
					continue;

				//
				// we passed all tests, take up this spot
				//
				markRegionUsed( &region );  // marks region AND gutter used
				BitClear( image->m_status, ImageInfo::TOOBIG );
				BitClear( image->m_status, ImageInfo::UNPACKED );
				BitSet( image->m_status, ImageInfo::PACKED );
				image->m_page = this;

				//
				// store the properties of the region that was used to fit this
				// image
				//
				image->m_fitBits = fitBits;

				// store the gutter sizes used in fitting this image
				image->m_gutterUsed.x = xGutter;
				image->m_gutterUsed.y = yGutter;

				//
				// if we packed this image rotated, set a flag telling us we
				// need to swap the size dimension in the image structure
				// when copying the image data
				//
				if( tryRotate == TRUE )
					BitSet( image->m_status, ImageInfo::ROTATED90C );

				//
				// save the page position of this image, but do not include
				// the gutter or padding borders which is incorporated into the region,
				// we're interested in just the bounding rectangle of the image itself
				// on the texture page
				//
				image->m_pagePos = region;
				if( BitIsSet( fitBits, ImageInfo::FIT_XBORDER_LEFT ) )
					image->m_pagePos.lo.x++;
				if( BitIsSet( fitBits, ImageInfo::FIT_YBORDER_TOP ) )
					image->m_pagePos.lo.y++;
				if( BitIsSet( fitBits, ImageInfo::FIT_XBORDER_RIGHT ) )
					image->m_pagePos.hi.x--;
				if( BitIsSet( fitBits, ImageInfo::FIT_YBORDER_BOTTOM ) )
					image->m_pagePos.hi.y--;
				if( BitIsSet( fitBits, ImageInfo::FIT_XGUTTER ) )
					image->m_pagePos.hi.x -= xGutter;
				if( BitIsSet( fitBits, ImageInfo::FIT_YGUTTER ) )
					image->m_pagePos.hi.y -= yGutter;

				// link this image to the texture page
				image->m_prevPageImage = NULL;
				image->m_nextPageImage = m_imageList;
				if( m_imageList )
					m_imageList->m_prevPageImage = image;
				m_imageList = image;

				return TRUE;  // success

			}  // end for x

		}  // end for y

	}  // end while, triesLeft

	// no space
	return FALSE;

}  // end addImage

// TexturePage::generateTexture ===============================================
/** Generate the final packed texture given all the images that have
	* already been assigned to this page */
//=============================================================================
Bool TexturePage::generateTexture( void )
{

	// sanity
	if( m_imageList == NULL )
		return FALSE;

	// sanity
	DEBUG_ASSERTCRASH( m_packedImage == NULL, ("The packed image list must be NULL before generating texture") );
	DEBUG_ASSERTCRASH( m_targa == NULL, ("The targa must be NULL before generating a new texture") );

	// allocate targa to help us generate the final texture
	m_targa = new Targa;
	if( m_targa == NULL )
	{
		char buffer[ 128 ];

		sprintf( buffer, "Unable to allocate new targa to generate texture\n" );
		DEBUG_ASSERTCRASH( m_targa, (buffer) );
		MessageBox( NULL, buffer, "Internal Error", MB_OK | MB_ICONERROR );
		return FALSE;

	}  // end if

	Bool outputAlpha = TheImagePacker->getOutputAlpha();
	Int depth, bpp;

	//
	// if we're outputting an alpha channel we will use 32 bit color depth,
	// if we're not we will use 24 bit
	//
	if( outputAlpha )
		depth = 32;
	else
		depth = 24;

	// how many bytes per pixel for the targa file format
	bpp = TGA_BytesPerPixel( depth );

	// allocate a buffer for our final image
	Int bufferSize = m_size.x * m_size.y * bpp;
	m_packedImage = new Byte[ bufferSize ];
	if( m_packedImage == NULL )
	{
		char buffer[ 128 ];

		sprintf( buffer, "Unable to allocate final packed image buffer\n" );
		DEBUG_ASSERTCRASH( m_packedImage, (buffer) );
		MessageBox( NULL, buffer, "Internal Error", MB_OK | MB_ICONERROR );
		BitSet( m_status, PAGE_ERROR );
		BitSet( m_status, CANT_ALLOCATE_PACKED_IMAGE );
		return FALSE;

	}  // end if

	// zero the packed image to all zero
	memset( m_packedImage, 0, sizeof( Byte ) * bufferSize );

	// setup the targa header
	m_targa->Header.ImageType = TGA_TRUECOLOR;
	m_targa->Header.Width = m_size.x;
	m_targa->Header.Height = m_size.y;
	m_targa->Header.PixelDepth = depth;

	// add all the images to the final packed buffer
	ImageInfo *image;
	for( image = m_imageList; image; image = image->m_nextPageImage )
	{

		if( addImageData( m_packedImage, m_size.x, m_size.y,
											bpp, image ) == FALSE )
		{

			BitSet( m_status, PAGE_ERROR );
			BitSet( m_status, CANT_ADD_IMAGE_DATA );
			return FALSE;

		}  // end if

	}  // end for image

	// set this data into the targa structure
	m_targa->SetImage( m_packedImage );

	return TRUE;  // success

}  // end generateTexture

// TexturePage::writeFile =====================================================
/** Write the texture data that has already been generated to a file
	* starting with the baseName passed in with the texture id of
	* this texture page appended to the end of it */
//=============================================================================
Bool TexturePage::writeFile( char *baseFilename )
{

	// sanity
	if( baseFilename == NULL || m_targa == NULL )
	{

		BitSet( m_status, PAGE_ERROR );
		BitSet( m_status, NO_TEXTURE_DATA );
		return FALSE;

	}  // end if

	// construct filename
	char filePath[ _MAX_PATH ];
	sprintf( filePath, "%s%s_%03d.tga", TheImagePacker->getOutputDirectory(),
					 baseFilename, getID() );

	// write the file
	Bool error = FALSE;
	long flags = TGAF_IMAGE;
	if( TheImagePacker->getCompressTextures() == TRUE )
		BitSet( flags, TGAF_COMPRESS );
	error = m_targa->Save( filePath, flags , FALSE );

	if( error != 0 )
	{

		// there was an error, set a status bit
		BitSet( m_status, PAGE_ERROR );
		BitSet( m_status, ERROR_DURING_SAVE );

	}  // end if

	// return success or not
	return !error;

}  // end writeFile

// TexturePage::getPixel ======================================================
/** Get the RGB pixel stored at location (x,y) (where (0,0) is the upper
	* left corner of the image ... even though the internal targa
	* isn't stored that way */
//=============================================================================
void TexturePage::getPixel( Int x, Int y, Byte *r, Byte *g, Byte *b, Byte *a )
{

	// do nothing if we have no image data
	if( m_packedImage == NULL )
		return;

	// how many bytes per pixel for the targa file format
	Int depth = m_targa->Header.PixelDepth;
	Int bpp = TGA_BytesPerPixel( depth );

	// compute location into buffer
	Byte *buf;
	buf = m_packedImage + ((m_size.y - 1 - y) * m_size.x * bpp) + (x * bpp);

	// read the pixel data
	if( bpp == 4 )
	{

		if( a )
			*a = buf[ 0 ];

		*r = buf[ 1 ];
		*g = buf[ 2 ];
		*b = buf[ 3 ];

	}  // end if
	else
	{

		if( a )
			*a = (char)0xff;  // no data, just return solid alpha

		*r = buf[ 0 ];
		*g = buf[ 1 ];
		*b = buf[ 2 ];

	}  // end else

}  // end getPixel
