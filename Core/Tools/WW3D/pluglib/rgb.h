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
 *                     $Archive:: /G/wwlib/RGB.h                                              $*
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

class PaletteClass;
class HSVClass;


/*
**	Each color entry is represented by this class. It holds the values for the color
**	guns. The gun values are recorded in device dependant format, but the interface
**	uses gun values from 0 to 255.
*/
class RGBClass
{
	public:
		RGBClass(void) : Red(0), Green(0), Blue(0) {}
		RGBClass(unsigned char red, unsigned char green, unsigned char blue) : Red(red), Green(green), Blue(blue) {}
		operator HSVClass (void) const;
		RGBClass & operator = (RGBClass const & rgb) {
			if (this == &rgb) return(*this);

			Red = rgb.Red;
			Green = rgb.Green;
			Blue = rgb.Blue;
			return(*this);
		}

		enum {
			MAX_VALUE=255
		};

		void Adjust(int ratio, RGBClass const & rgb);
		int Difference(RGBClass const & rgb) const;
		int Get_Red(void) const {return (Red);}
		int Get_Green(void) const {return(Green);}
		int Get_Blue(void) const {return(Blue);}
		void Set_Red(unsigned char value) {Red = value;}
		void Set_Green(unsigned char value) {Green = value;}
		void Set_Blue(unsigned char value) {Blue = value;}

	private:

		friend class PaletteClass;

		/*
		**	These hold the actual color gun values in machine independant scale. This
		**	means the values range from 0 to 255.
		*/
		unsigned char Red;
		unsigned char Green;
		unsigned char Blue;
};

extern RGBClass const BlackColor;
