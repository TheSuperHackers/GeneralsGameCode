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
 *                     $Archive:: /Commando/Code/Tools/max2w3d/simpdib.h                      $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 10/14/97 3:07p                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#ifndef SIMPDIB_H
#define SIMPDIB_H

#ifndef ALWAYS_H
#include "always.h"
#endif

#include <max.h>

#ifndef WIN_H
#include "win.h"
#endif

#ifndef PALETTE_H
#include "palette.h"
#endif

class SimpleDIBClass
{
public:

	SimpleDIBClass(HWND hwnd,int width, int height,PaletteClass & pal);
	~SimpleDIBClass(void);

	HBITMAP		Get_Handle()		{ return Handle; }
	int			Get_Width(void)		{ return Width; }
	int			Get_Height(void)	{ return Height; }

	void			Clear(unsigned char color);
	void			Set_Pixel(int i,int j,unsigned char color);

private:

	bool					IsZombie;	// object constructor failed, its a living-dead object!
	BITMAPINFO *		Info;			// info used in creating the dib + the palette.
	HBITMAP				Handle;		// handle to the actual dib
	unsigned char *	Pixels;		// address of memory containing the pixel data
	int					Width;		// width of the dib
	int					Height;		// height of the dib
	unsigned char *	PixelBase;	// address of upper left pixel (this and DIBPitch abstract up/down DIBS)
	int					Pitch;		// offset from DIBPixelBase to next row (can be negative for bottom-up DIBS)

};


#endif /*SIMPDIB_H*/
