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
 *                     $Archive:: /Commando/Code/Library/dib.h                                $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 7/26/97 11:33a                                              $*
 *                                                                                             * 
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef DIB_H
#define DIB_H

#include "always.h"
#include "bsurface.h"
#include "PALETTE.H"
#include "win.h"


class DIB8Class
{
public:

	DIB8Class(HWND hwnd,int width, int height, PaletteClass & pal);
	~DIB8Class(void);

	HBITMAP		Get_Handle()		{ return Handle; }
	int			Get_Width(void)		{ return Width; }
	int			Get_Height(void)	{ return Height; }
	Surface &	Get_Surface(void)	{ return *Surface; }

	void		Clear(unsigned char color);

private:

	bool				IsZombie;	// object constructor failed, its a living-dead object!
	BITMAPINFO *		Info;		// info used in creating the dib + the palette.
	HBITMAP				Handle;		// handle to the actual dib
	unsigned char *		Pixels;		// address of memory containing the pixel data
	int					Width;		// width of the dib
	int					Height;		// height of the dib
	unsigned char *		PixelBase;	// address of upper left pixel (this and DIBPitch abstract up/down DIBS)
	int					Pitch;		// offset from DIBPixelBase to next row (can be negative for bottom-up DIBS)

	BSurface *			Surface;	// Bsurface wrapped around the pixel buffer.
};


#endif /*DIB_H*/