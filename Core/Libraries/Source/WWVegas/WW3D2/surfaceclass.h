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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /VSS_Sync/ww3d2/surfaceclass.h                              $*
 *                                                                                             *
 *              Original Author:: Nathaniel Hoffman                                            *
 *                                                                                             *
 *                      $Author:: Vss_sync                                                    $*
 *                                                                                             *
 *                     $Modtime:: 8/29/01 9:32p                                               $*
 *                                                                                             *
 *                    $Revision:: 17                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef SURFACECLASS_H
#define SURFACECLASS_H

#include "ww3dformat.h"
#include "refcount.h"

struct IDirect3DSurface8;
class Vector2i;
class Vector3;

/*************************************************************************
**                             SurfaceClass
**
** This is our surface class, which wraps IDirect3DSurface8.
**
** Hector Yee 2/12/01 - added in fills, blits etc for font3d class
**
*************************************************************************/
class SurfaceClass : public W3DMPO, public RefCountClass
{
	W3DMPO_GLUE(SurfaceClass)
	public:

		struct SurfaceDescription {
			WW3DFormat		Format;	// Surface format
			unsigned int	Width;	// Surface width in pixels
			unsigned int	Height;	// Surface height in pixels
		};

		// Create surface with desired height, width and format.
		SurfaceClass(unsigned width, unsigned height, WW3DFormat format);

		// Create surface from a file.
		SurfaceClass(const char *filename);

		// Create the surface from a D3D pointer
		SurfaceClass(IDirect3DSurface8 *d3d_surface);

		~SurfaceClass(void);

		// Get surface description
		 void Get_Description(SurfaceDescription &surface_desc);

		// Lock / unlock the surface
		void * Lock(int * pitch);
		void Unlock(void);

		// HY -- The following functions are support functions for font3d
		// zaps the surface memory to zero
		void Clear();

		// copies the contents of one surface to another		
		void Copy(
			unsigned int dstx, unsigned int dsty,
			unsigned int srcx, unsigned int srcy, 
			unsigned int width, unsigned int height,
			const SurfaceClass *other);

		// support for copying from a byte array
		void Copy(const unsigned char *other);

		// support for copying from a byte array
		void Copy(Vector2i &min,Vector2i &max, const unsigned char *other);

		// copies the contents of one surface to another, stretches
		void Stretch_Copy(
			unsigned int dstx, unsigned int dsty,unsigned int dstwidth, unsigned int dstheight,
			unsigned int srcx, unsigned int srcy, unsigned int srcwidth, unsigned int srcheight,
			const SurfaceClass *source);

		// finds the bounding box of non-zero pixels, used in font3d
		void FindBB(Vector2i *min,Vector2i*max);

		// tests a column to see if the alpha is nonzero, used in font3d
		bool Is_Transparent_Column(unsigned int column);		

		// makes a copy of the surface into a byte array
		unsigned char *CreateCopy(int *width,int *height,int*size,bool flip=false);

			// For use by TextureClass:
		IDirect3DSurface8 *Peek_D3D_Surface(void) { return D3DSurface; }

		// Attaching and detaching a surface pointer
		void	Attach (IDirect3DSurface8 *surface);
		void	Detach (void);

		// draws a horizontal line
		void DrawHLine(const unsigned int y,const unsigned int x1, const unsigned int x2, unsigned int color);

		void DrawPixel(const unsigned int x,const unsigned int y, unsigned int color);

		// get pixel function .. to be used infrequently
		void Get_Pixel(Vector3 &rgb, int x,int y);

		void Hue_Shift(const Vector3 &hsv_shift);

		bool Is_Monochrome(void);

		WW3DFormat Get_Surface_Format() const { return SurfaceFormat; }

	private:

		// Direct3D surface object
		IDirect3DSurface8 *D3DSurface;

		WW3DFormat SurfaceFormat;
	friend class TextureClass;	
};

#endif


