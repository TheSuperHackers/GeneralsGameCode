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
 *                     $Archive:: /G/wwlib/PALETTE.h                                          $*
 *                                                                                             *
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             *
 *                     $Modtime:: 4/02/99 12:00p                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include	"rgb.h"

/*
**	The palette class is used to manipulate a palette as a whole. All 256 colors are
**	represented by the palette class object.
*/
class PaletteClass
{
	public:
		enum {
			COLOR_COUNT=256			// Number of color indices on the palette.
		};

		PaletteClass(void) {};
		PaletteClass(RGBClass const & rgb);
		PaletteClass(unsigned char *binary_palette);

		RGBClass & operator[] (int index) {return(Palette[index % COLOR_COUNT]);};
		RGBClass const & operator[] (int index) const {return(Palette[index % COLOR_COUNT]);};
		RGBClass & Get_Color(int index) {return(Palette[index % COLOR_COUNT]);};
		RGBClass const & Get_Color(int index) const {return(Palette[index % COLOR_COUNT]);};
		int operator == (PaletteClass const & palette) const;
		int operator != (PaletteClass const & palette) const {return(!(operator ==(palette)));};
		PaletteClass & operator = (PaletteClass const & palette);
		operator const unsigned char * (void) const {return((const unsigned char *)&Palette[0]);};
		operator unsigned char * (void) {return((unsigned char *)&Palette[0]);};

		void Adjust(int ratio);
		void Adjust(int ratio, PaletteClass const & palette);
		void Partial_Adjust(int ratio, char *lut);
		void Partial_Adjust(int ratio, PaletteClass const & palette, char *lut);
		int Closest_Color(RGBClass const & rgb) const;

	protected:
		RGBClass Palette[COLOR_COUNT];
};
