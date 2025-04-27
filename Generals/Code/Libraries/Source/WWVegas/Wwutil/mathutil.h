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

//
// Filename:     mathutil.h
// Project:      wwutil
// Author:       Tom Spencer-Smith
// Date:         June 1998
// Description:  static
//
//-----------------------------------------------------------------------------
#if defined(_MSV_VER)
#pragma once
#endif

#ifndef MATHUTIL_H
#define MATHUTIL_H

class cMathUtil
{
	public:
		static void		Angle_To_Vector(double angle, double & dx, double & dy);
      static void		Vector_To_Angle(double dx, double dy, double & angle);
		static double	Simple_Distance(double x1, double y1, double x2, double y2);
      static int		Round(double arg);
      static void		Rotate_Vector(double & vx, double & vy, double angle);

      //
      // Random numbers generated according to simple 
      // Probability Density Functions
      //
      static double  Get_Uniform_Pdf_Double(double lower, double upper);
      static double  Get_Normalized_Uniform_Pdf_Double();
      static int     Get_Uniform_Pdf_Int(int lower, int upper);
      static double  Get_Hat_Pdf_Double(double lower, double upper);
      static double  Get_Normalized_Hat_Pdf_Double();
      static int     Get_Hat_Pdf_Int(int lower, int upper);

		static const double PI_1;
		static const double PI_2;
};

#endif // MATHUTIL_H