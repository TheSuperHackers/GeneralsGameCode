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
 *                     $Archive:: /Commando/Library/HSV.CPP                                   $* 
 *                                                                                             * 
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             * 
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   HSVClass::Adjust -- Adjust an HSV color toward specified color.                           *
 *   HSVClass::Difference -- Finds the difference between two HSV color objects.               *
 *   HSVClass::operator RGBClass -- Conversion operator for RGBClass object.                   *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include	"always.h"
#include	"hsv.h"
#include	"RGB.H"

HSVClass const HSVClass::BlackColor(0, 0, 0);


/***********************************************************************************************
 * HSVClass::Adjust -- Adjust an HSV color toward specified color.                             *
 *                                                                                             *
 *    This routine will adjust the HSV color object toward the color of the specified HSV      *
 *    object. Typical users of this would be palette morphing or fading routines.              *
 *                                                                                             *
 * INPUT:   ratio    -- The ratio to move the HSV object toward the color specified. A value   *
 *                      of zero means no movement at all. A value of 255 means move completely *
 *                      toward the specified color (changed completely).                       *
 *                                                                                             *
 *          hsv      -- A reference to the color that the current HSV object is to change      *
 *                      toward.                                                                *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/20/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
void HSVClass::Adjust(int ratio, HSVClass const & hsv)
{
	/*
	**	Ratio conversion is limited to 0 through 100%. This is
	**	the range of 0 to 255.
	*/
	ratio &= 0x00FF;

	/*
	**	Adjust the color guns by the ratio specified toward the
	**	destination color.
	*/
	int value = hsv.Get_Value() - Get_Value();
	Value = (unsigned char)(Get_Value() + (value * ratio) / 256);

	int saturation = hsv.Get_Saturation() - Get_Saturation();
	Saturation = (unsigned char)(Get_Saturation() + (saturation * ratio) / 256);

	int hue = hsv.Get_Hue() - Get_Hue();
	Hue = (unsigned char)(Get_Hue() + (hue * ratio) / 256);
}


/***********************************************************************************************
 * HSVClass::Difference -- Finds the difference between two HSV color objects.                 *
 *                                                                                             *
 *    This routine will determine a color difference between two HSV objects. The difference   *
 *    has no particular meaning other that larger numbers meaning greater difference.          *
 *                                                                                             *
 * INPUT:   hsv   -- The other HSV object to compare this HSV object to.                       *
 *                                                                                             *
 * OUTPUT:  Returns with a relative distance (in arbitrary units) between this HSV object and  *
 *          the HSV object supplied.                                                           *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/20/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
int HSVClass::Difference(HSVClass const & hsv) const
{
	int hue = (int)Hue - (int)hsv.Hue;
	if (hue < 0) hue = -hue;

	int saturation = (int)Saturation - (int)hsv.Saturation;
	if (saturation < 0) saturation = -saturation;

	int value = (int)Value - (int)hsv.Value;
	if (value < 0) value = -value;

	return(hue*hue + saturation*saturation + value*value);
}


/***********************************************************************************************
 * HSVClass::operator RGBClass -- Conversion operator for RGBClass object.                     *
 *                                                                                             *
 *    This conversion operator will convert the HSV object into an RGB object.                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with a reference (implied) of the RGBClass object that most closely        *
 *          matches this HSVClass object.                                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/20/1996 JLB : Created.                                                                 *
 *=============================================================================================*/
HSVClass::operator RGBClass (void) const
{
	unsigned int i;				// Integer part.
	unsigned int f;				// Fractional or remainder part.  f/HSV_BASE gives fraction.
	unsigned int tmp;			// Temporary variable to help with calculations.
	unsigned int values[7];	// Possible rgb values.  Don't use zero.

	int hue = Get_Hue();
	int saturation = Get_Saturation();
	int value = Get_Value();
	int red, green, blue;


	hue *= 6;
	f = hue % 255;

	// Set up possible red, green and blue values.
	values[1] =
	values[2] = value;

	//
	// The following lines of code change
	//	values[3] = (v * (255 - ( (s * f) / 255) )) / 255;
	//	values[4] = values[5] = (v * (255 - s)) / 255;
	// values[6] = (v * (255 - (s * (255 - f)) / 255)) / 255;
	// so that the are rounded divides.
	//

	tmp = (saturation * f) / 255;
	values[3] = (value * (255 - tmp)) / 255;

	values[4] =
	values[5] = (value * (255 - saturation)) / 255;

	tmp = 255 - (saturation * (255 - f)) / 255;
	values[6] = (value * tmp) / 255;


	// This should not be rounded.
	i = hue / 255;

	i += (i > 4) ? -4 : 2;
	red = values[i];

	i += (i > 4) ? -4 : 2;
	blue = values[i];

	i += (i > 4) ? -4 : 2;
	green = values[i];

	RGBClass rgb((unsigned char)red, (unsigned char)green, (unsigned char)blue);
	return(rgb);
}


