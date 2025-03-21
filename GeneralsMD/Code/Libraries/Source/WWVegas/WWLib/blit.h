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
 *                     $Archive:: /Commando/Code/wwlib/blit.h                                 $* 
 *                                                                                             * 
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 5/04/01 7:49p                                               $*
 *                                                                                             * 
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef BLIT_H
#define BLIT_H

#include	"blitter.h"
#include	"BUFF.H"
#include	"trect.h"
#include	"Surface.h"

bool Bit_Blit(Surface & dest, Rect const & destrect, Surface const & source, Rect const & sourcerect, Blitter const & blitter);
bool RLE_Blit(Surface & dest, Rect const & destrect, Surface const & source, Rect const & sourcerect, RLEBlitter const & blitter);

//bool Bit_Blit(SurfaceRect & dest, Rect const & ddrect, SurfaceRect const & source, Rect const & ssrect, Blitter const & blitter);
//bool RLE_Blit(SurfaceRect & dest, Rect const & ddrect, SurfaceRect const & source, Rect const & ssrect, RLEBlitter const & blitter);

bool Bit_Blit(Surface & dest, Rect const & dcliprect, Rect const & ddrect, Surface const & source, Rect const & scliprect, Rect const & ssrect, Blitter const & blitter);
bool RLE_Blit(Surface & dest, Rect const & dcliprect, Rect const & ddrect, Surface const & source, Rect const & scliprect, Rect const & ssrect, RLEBlitter const & blitter);


int Buffer_Size(Surface & surface, int width, int height);
bool To_Buffer(Surface const & surface, Rect const & rect, Buffer & buffer);
bool From_Buffer(Surface & surface, Rect const & rect, Buffer const & buffer);



#endif
