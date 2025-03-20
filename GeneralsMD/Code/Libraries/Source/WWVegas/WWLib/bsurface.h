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
 *                 Project Name : Command & Conquer                                            * 
 *                                                                                             * 
 *                     $Archive:: /G/wwlib/bsurface.h                                         $* 
 *                                                                                             * 
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 4/02/99 11:59a                                              $*
 *                                                                                             * 
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#ifndef BSURFACE_H
#define BSURFACE_H

#include	"BUFF.H"
#include	"xsurface.h"

/*
**	This class handles a simple surface that exists in system RAM.
*/
class BSurface : public XSurface
{
	public:
		BSurface(int width, int height, int bbp, void * buffer=NULL) : 
			XSurface(width, height), 
			BBP(bbp), 
			Buff(buffer, width * height * bbp)
		{
		}
		
		/*
		**	Gets and frees a direct pointer to the buffer.
		*/
		virtual void * Lock(Point2D point = Point2D(0, 0)) const 
		{
			XSurface::Lock();
			return(((char*)Buff.Get_Buffer()) + point.Y * Stride() + point.X * Bytes_Per_Pixel());
		}

		/*
		**	Queries information about the surface.
		*/
		virtual int Bytes_Per_Pixel(void) const {return(BBP);}
		virtual int Stride(void) const {return(Get_Width() * BBP);}

	protected:

		/*
		**	Recorded bytes per pixel (used when determining pixel positions).
		*/
		int BBP;

		/*
		**	Tracks the buffer that this surface represents.
		*/
		Buffer Buff;
};

#endif
