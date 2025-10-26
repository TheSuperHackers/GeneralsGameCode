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
 *                     $Archive:: /Commando/Library/HSV.H                                     $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/22/97 11:37a                                              $*
 *                                                                                             *
 *                    $Revision:: 1                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

class RGBClass;
class HSVClass;

/*
**	Each color entry is represented by this class. It holds the values for the color
**	attributes. The values are recorded in a range from 0 to 255 with 255 being the
**	maximum.
*/
class HSVClass
{
	private:
		static HSVClass const BlackColor;

	public:
		HSVClass(void) : Hue(0), Saturation(0), Value(0) {};
		HSVClass(unsigned char hue, unsigned char saturation, unsigned char value) :
				Hue(hue),
				Saturation(saturation),
				Value(value)
			{};
		operator RGBClass (void) const;

		enum {
			MAX_VALUE=255
		};

		void Adjust(int ratio, HSVClass const & hsv);
		int Difference(HSVClass const & hsv) const;
		int Get_Hue(void) const {return(Hue);};
		int Get_Saturation(void) const {return(Saturation);};
		int Get_Value(void) const {return(Value);};
		void Set_Hue(unsigned char value) {Hue = value;}
		void Set_Saturation(unsigned char value) {Saturation = value;}
		void Set_Value(unsigned char value) {Value = value;}

	private:
		unsigned char Hue;
		unsigned char Saturation;
		unsigned char Value;
};
