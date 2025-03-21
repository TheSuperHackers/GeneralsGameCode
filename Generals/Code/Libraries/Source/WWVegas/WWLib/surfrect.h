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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwlib/surfrect.h                             $*
 *                                                                                             *
 *                      $Author:: Byon_g                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/28/00 2:44p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef SURFRECT_H
#define SURFRECT_H

#ifdef NEVER

#include	"Surface.h"
#include	"trect.h"
#include	"Point.h"
#include	<assert.h>


/*
**	This class encapsulates a surface and a clipping rectangle. It is intended to be used as
**	a convenience for certain operations that need both a surface and a rectangle. Be aware that
**	all surfaces are imbued with a rectangle that is the full size of the surface.
*/
class SurfaceRect {
	public:
		SurfaceRect(Surface * surfaceptr = NULL, Rect * rect = NULL) : SurfacePtr(surfaceptr), Point(0, 0) {
			assert(SurfacePtr != NULL);
			if (rect != NULL) Window = *rect; else Window = SurfacePtr->Get_Rect();
		}
		SurfaceRect(Surface & surface, Rect * rect = NULL) : SurfacePtr(&surface), Point(0, 0) {
			if (rect != NULL) Window = *rect; else Window = SurfacePtr->Get_Rect();
		}
		SurfaceRect(Surface & surface, Rect const & rect) : SurfacePtr(&surface), Window(rect), Point(0, 0) {}
		~SurfaceRect(void) {};

		/*
		**	Pointer to the surface represented by this class.
		*/
		Surface * SurfacePtr;

		/*
		**	Clipping rectangle window into the surface.
		*/
		Rect Window;

		/*
		**	Tracking 2D coordinate into the surface.
		*/
		Point2D Point;

	private:
		/*
		**	Ensures that the copy constructor and assignment operator never
		**	get created for this class.
		*/
		SurfaceRect(SurfaceRect const & rvalue);
		SurfaceRect operator = (SurfaceRect const & rvalue);
};

#endif
#endif
