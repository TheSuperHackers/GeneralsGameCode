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
 *                     $Archive:: /Commando/Library/PALETTE.CPP                               $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             * 
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   PaletteClass::Adjust -- Adjusts the palette toward another palette.                       *
 *   PaletteClass::Adjust -- Adjusts this palette toward black.                                *
 *   PaletteClass::Closest_Color -- Finds closest match to color specified.                    *
 *   PaletteClass::PaletteClass -- Constructor that fills palette with color specified.        *
 *   PaletteClass::operator = -- Assignment operator for palette objects.                      *
 *   PaletteClass::operator == -- Equality operator for palette objects.                       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include	"always.h"
#include	"PALETTE.H"
#include	<string.h>


/***********************************************************************************************
 * PaletteClass::PaletteClass -- Constructor that fills palette with color specified.          *
 *                                                                                             *
 *    This constructor will fill the palette with the color specified.                         *
 *                                                                                             *
 * INPUT:   rgb   -- Reference to the color to fill the entire palette with.                   *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/02/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
PaletteClass::PaletteClass(RGBClass const & rgb)
{
	for (int index = 0; index < COLOR_COUNT; index++) {
		Palette[index] = rgb;
	}
}

PaletteClass::PaletteClass(unsigned char *binary_palette)
{
	memcpy(&Palette[0], binary_palette, sizeof(Palette));
}

/***********************************************************************************************
 * PaletteClass::operator == -- Equality operator for palette objects.                         *
 *                                                                                             *
 *    This is the comparison for equality operator. It will compare palette objects to         *
 *    determine if they are identical.                                                         *
 *                                                                                             *
 * INPUT:   palette  -- Reference to the palette to compare to this palette.                   *
 *                                                                                             *
 * OUTPUT:  Are the two palettes identical?                                                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/02/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int PaletteClass::operator == (PaletteClass const & palette) const
{
	if (this == &palette) return(true);
	return(memcmp(&Palette[0], &palette.Palette[0], sizeof(Palette)) == 0);
}


/***********************************************************************************************
 * PaletteClass::operator = -- Assignment operator for palette objects.                        *
 *                                                                                             *
 *    This is the assignment operator for palette objects. Although the default C++ generated  *
 *    assignment operator would function correctly, it would not check for self-assignment     *
 *    and thus this routine can be faster.                                                     *
 *                                                                                             *
 * INPUT:   palette  -- Reference to that palette that will be copied into this palette.       *
 *                                                                                             *
 * OUTPUT:  Returns with a reference to the newly copied to palette.                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/02/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
PaletteClass & PaletteClass::operator = (PaletteClass const & palette)
{
	if (this == &palette) return(*this);

	memcpy(&Palette[0], &palette.Palette[0], sizeof(Palette));
	return(*this);
}


/***********************************************************************************************
 * PaletteClass::Adjust -- Adjusts this palette toward black.                                  *
 *                                                                                             *
 *    This routine is used to adjust this palette toward black. Typical use of this routine    *
 *    is when fading the palette to black.                                                     *
 *                                                                                             *
 * INPUT:   ratio -- The ratio to fade this palette to black. 0 means no fading at all. 255    *
 *                   means 100% faded to black.                                                *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This routine doesn't actually set the palette to the video card. Use the Set()  *
 *             function to achieve that purpose.                                               *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/02/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void PaletteClass::Adjust(int ratio)
{
	for (int index = 0; index < COLOR_COUNT; index++) {
		Palette[index].Adjust(ratio, BlackColor);
	}
}


/***********************************************************************************************
 * PaletteClass::Adjust -- Adjusts the palette toward another palette.                         *
 *                                                                                             *
 *    This routine is used to adjust a palette toward a destination palette by the ratio       *
 *    specified. This is primarily used by the palette fading routines.                        *
 *                                                                                             *
 * INPUT:   palette  -- Reference to the destination palette.                                  *
 *                                                                                             *
 *          ratio    -- The ratio to adjust this palette toward the destination palette. A     *
 *                      value of 0 means no adjustment at all. A value of 255 means 100%       *
 *                      adjustment.                                                            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/02/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void PaletteClass::Adjust(int ratio, PaletteClass const & palette)
{
	for (int index = 0; index < COLOR_COUNT; index++) {
		Palette[index].Adjust(ratio, palette[index]);
	}
}


/***********************************************************************************************
 * PaletteClass::Partial_Adjust -- Adjusts the specified parts of this palette toward black.   *
 *                                                                                             *
 *    This routine is used to adjust this palette toward black. Typical use of this routine    *
 *    is when fading the palette to black. The input lookup table is used to determine         *
 *    which entries should fade and which should stay the same                                 *
 *                                                                                             *
 * INPUT:   ratio -- The ratio to fade this palette to black. 0 means no fading at all. 255    *
 *                   means 100% faded to black.                                                *
 *                                                                                             *
 *          lookup	-- ptr to lookup table                                                    *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   This routine doesn't actually set the palette to the video card. Use the Set()  *
 *             function to achieve that purpose.                                               *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/02/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void PaletteClass::Partial_Adjust(int ratio, char *lut)
{
	for (int index = 0; index < COLOR_COUNT; index++) {
		if (lut[index]) {
			Palette[index].Adjust(ratio, BlackColor);
		}
	}
}


/***********************************************************************************************
 * PaletteClass::Partial_Adjust -- Adjusts the palette toward another palette.                 *
 *                                                                                             *
 *    This routine is used to adjust a palette toward a destination palette by the ratio       *
 *    specified. This is primarily used by the palette fading routines.  The input lookup      *
 *    table is used to determine which entries should fade and which should stay the same      *
 *                                                                                             *
 *                                                                                             *
 * INPUT:   palette  -- Reference to the destination palette.                                  *
 *                                                                                             *
 *          ratio    -- The ratio to adjust this palette toward the destination palette. A     *
 *                      value of 0 means no adjustment at all. A value of 255 means 100%       *
 *                      adjustment.                                                            *
 *                                                                                             *
 *          lookup   -- ptr to lookup table                                                    *
 *                                                                                             *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/02/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void PaletteClass::Partial_Adjust(int ratio, PaletteClass const & palette, char *lut)
{
	for (int index = 0; index < COLOR_COUNT; index++) {
		if (lut[index]) {
			Palette[index].Adjust(ratio, palette[index]);
		}
	}
}


/***********************************************************************************************
 * PaletteClass::Closest_Color -- Finds closest match to color specified.                      *
 *                                                                                             *
 *    This routine will examine the palette and return with the color index number for the     *
 *    color that most closely matches the color specified. Remap operations rely heavily on    *
 *    this routine to allow working with a constant palette.                                   *
 *                                                                                             *
 * INPUT:   rgb   -- Reference to a color to search for in the current palette.                *
 *                                                                                             *
 * OUTPUT:  Returns with a color index value to most closely matches the specified color.      *
 *                                                                                             *
 * WARNINGS:   This routine will quite likely not find an exact match.                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/02/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int PaletteClass::Closest_Color(RGBClass const & rgb) const
{
	int closest = 0;
	int value = -1;

	RGBClass const * ptr = &Palette[0];
	for (int index = 0; index < COLOR_COUNT; index++) {
		int difference = rgb.Difference(*ptr++);
		if (value == -1 || difference < value) {
			value = difference;
			closest = index;
		}
	}
	return(closest);
}
