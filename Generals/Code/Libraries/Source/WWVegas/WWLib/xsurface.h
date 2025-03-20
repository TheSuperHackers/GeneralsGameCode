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
 *                     $Archive:: /G/wwlib/xsurface.h                                         $* 
 *                                                                                             * 
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 4/02/99 12:01p                                              $*
 *                                                                                             * 
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef XSURFACE_H
#define XSURFACE_H

#include	"Surface.h"

/*
**	This is a concrete (mostly) derived class that handles a surface. This layer presumes that
**	pixels are compositied into a contiguous integral (usually a byte) linear array. The 
**	implemented routines do not use hardware assist. They are prime candidates to be converted
**	to assembly language.
*/
class XSurface : public Surface
{
	public:
		XSurface(int width=0, int height=0) : Surface(width, height), LockCount(0) {}
		virtual ~XSurface(void) {}
		
		/*
		**	Copies regions from one surface to another.
		*/
		virtual bool Blit_From(Rect const & dcliprect, Rect const & destrect, Surface const & source, Rect const & scliprect, Rect const & sourcerect, bool trans=false);
		virtual bool Blit_From(Rect const & destrect, Surface const & source, Rect const & sourcerect, bool trans=false);
		virtual bool Blit_From(Surface const & source, bool trans=false);

		/*
		**	Fills a region with a constant color.
		*/
		virtual bool Fill_Rect(Rect const & rect, int color);
		virtual bool Fill_Rect(Rect const & cliprect, Rect const & fillrect, int color);
		virtual bool Fill(int color);

		/*
		**	Fetches and stores a pixel to the display (pixel is in surface format).
		*/
		virtual bool Put_Pixel(Point2D const & point, int color);
		virtual int Get_Pixel(Point2D const & point) const;

		/*
		**	Draws lines onto the surface.
		*/
		virtual bool Draw_Line(Point2D const & startpoint, Point2D const & endpoint, int color);
		virtual bool Draw_Line(Rect const & cliprect, Point2D const & startpoint, Point2D const & endpoint, int color);
		
		/*
		**	Draws rectangles onto the surface.
		*/
		virtual bool Draw_Rect(Rect const & rect, int color);
		virtual bool Draw_Rect(Rect const & cliprect, Rect const & rect, int color);

		/*
		**	Gets and frees a direct pointer to the video memory.
		*/
		virtual void * Lock(Point2D = Point2D(0, 0)) const {LockCount++;return(NULL);}
		virtual bool Unlock(void) const {LockCount--;return(true);}
		virtual bool Is_Locked(void) const {return(LockCount != 0);}

		/*
		**	Queries information about the surface.
		*/
		virtual int Bytes_Per_Pixel(void) const = 0;
		virtual int Stride(void) const = 0;
		
		/*
		**	Hack function to serve the purpose that RTTI was invented for, but since
		**	the Watcom compiler doesn't support RTTI, we must resort to using this
		**	alternative.
		*/
		virtual bool Is_Direct_Draw(void) const {return(false);}

		/*
		**	This routine is handy for preparing to perform some kind of manual blit
		**	operation.
		*/
		static bool Prep_For_Blit(Surface & dest, Rect & drect, Surface const & source, Rect & srect, bool & overlapped, void * & dbuffer, void * & sbuffer);
		static bool Prep_For_Blit(Surface & dest, Rect const & dcliprect, Rect & drect, Surface const & source, Rect const & scliprect, Rect & srect, bool & overlapped, void * & dbuffer, void * & sbuffer);

		/*
		**	These are the exposed manual blit routines. They can be called directly if desired.
		*/
		static bool Blit_Trans(Surface & dest, Rect const & destrect, Surface const & source, Rect const & sourcerect);
		static bool Blit_Plain(Surface & dest, Rect const & destrect, Surface const & source, Rect const & sourcerect);

	protected:
		/*
		**	Records the number of locks on this surface.
		*/
		mutable int LockCount;
};

bool Blit_Clip(Rect & drect, Rect const & dwindow, Rect & srect, Rect const & swindow);


#endif
