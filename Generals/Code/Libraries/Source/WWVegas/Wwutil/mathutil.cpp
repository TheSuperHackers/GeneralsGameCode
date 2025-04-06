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
// Filename:     mathutil.cpp
// Project:      wwutil
// Author:       Tom Spencer-Smith
// Date:         June 1998
// Description:  
//
//-----------------------------------------------------------------------------
#include "mathutil.h" // I WANNA BE FIRST!

#include <math.h>
#include <stdlib.h>
#include "wwmath.h"
#include "miscutil.h"
#include "wwdebug.h"

const double cMathUtil::PI_1   = 3.1415927;
const double cMathUtil::PI_2 = 1.5707963;

//-----------------------------------------------------------------------------
//
// Returns a unit vector
//
void cMathUtil::Angle_To_Vector(double angle, double & dx, double & dy)
{
	WWASSERT(angle > -WWMATH_EPSILON && angle < 360.0 + WWMATH_EPSILON);

	double angleRadians;

	if (angle >= 0 && angle < 90) {
		angleRadians = angle * PI_1 / 180.0;
		dx = WWMath::Sin(angleRadians);
		dy = WWMath::Cos(angleRadians);
	} else if (angle >= 90 && angle < 180) {
		angleRadians = (angle - 90) * PI_1 / 180.0;
		dx = WWMath::Cos(angleRadians);
		dy = -WWMath::Sin(angleRadians);
	} else if (angle >= 180 && angle < 270) {
		angleRadians = (angle - 180) * PI_1 / 180.0;
		dx = -WWMath::Sin(angleRadians);
		dy = -WWMath::Cos(angleRadians);
	} else {
		angleRadians = (angle - 270) * PI_1 / 180.0;
		dx = -WWMath::Cos(angleRadians);
		dy = WWMath::Sin(angleRadians);
	}

   double len;
	len = ::sqrt(dx * dx + dy * dy); 
   WWASSERT(::fabs(len - 1) < 0.0005);

	//
   // Correction for Irish nature of windows y coords
   //
   dy *= -1;
}

//-----------------------------------------------------------------------------
void cMathUtil::Vector_To_Angle(double dx, double dy, double & angle)
{
	double theta;

	if (dx == 0 && dy == 0) {
		theta = 0;
	}

	if (dx == 0) {
		if (dy <= 0) {
			theta = 0;
		} else {
			theta = PI_1;
		}
	} else {
		theta = WWMath::Atan(-dy / dx);
		if (dx < 0) {
			theta += PI_1;
		}
		theta += 3 * PI_2;
		if (theta >= 2 * PI_1) {
			theta -= 2 * PI_1;
		}
		theta = 2 * PI_1 - theta;
		if (theta == 2 * PI_1) {
			theta = 0;
		}
	}

	angle = theta * 180.0 / PI_1;
}

//-----------------------------------------------------------------------------
double cMathUtil::Simple_Distance(double x1, double y1, double x2, double y2)
{
	double dx = x2 - x1;
	double dy = y2 - y1;
	return(::sqrt(dx * dx + dy * dy));
}

//-----------------------------------------------------------------------------
int cMathUtil::Round(double arg)
{
   //return (int)(arg + 0.5);

	if (arg > MISCUTIL_EPSILON) {
		return (int) (arg + 0.5f);
	} else if (arg < -MISCUTIL_EPSILON) {
		return (int) (arg - 0.5f);
	} else {
		return 0;
	}
}

//-----------------------------------------------------------------------------
void cMathUtil::Rotate_Vector(double & vx, double & vy, double angle)
{
   double angle_radians = angle * PI_1 / 180.0;

   double vx1 = vx;
   double vy1 = vy;

   vx = vx1 * ::WWMath::Cos(angle_radians) - vy1 * ::WWMath::Sin(angle_radians);
   vy = vx1 * ::WWMath::Sin(angle_radians) + vy1 * ::WWMath::Cos(angle_radians);
}



//-----------------------------------------------------------------------------
double cMathUtil::Get_Uniform_Pdf_Double(double lower, double upper)
{
   WWASSERT(upper - lower > -MISCUTIL_EPSILON);

   double x = lower + ::rand() / (double) RAND_MAX * (upper - lower);

   WWASSERT(x - lower > -MISCUTIL_EPSILON && upper - x > -MISCUTIL_EPSILON);
   
   return x;
}

//-----------------------------------------------------------------------------
double cMathUtil::Get_Normalized_Uniform_Pdf_Double()
{
   return Get_Uniform_Pdf_Double(0, 1);
}

//-----------------------------------------------------------------------------
int cMathUtil::Get_Uniform_Pdf_Int(int lower, int upper)
{
   WWASSERT(lower <= upper);
   int x = lower + ::rand() % (upper - lower + 1);

   WWASSERT(x >= lower && upper >= x);
   
   return x;
}

//-----------------------------------------------------------------------------
double cMathUtil::Get_Hat_Pdf_Double(double lower, double upper)
{
   WWASSERT(upper - lower > -MISCUTIL_EPSILON);

   double x;
   
   if (::fabs(upper - lower) < MISCUTIL_EPSILON) {
      x = lower;
   } else {

      double dx = (upper - lower) / 2.0f;
      double dy = 1 / dx;
      double m = dy / dx;
      double c = -m * lower;
   
      x = Get_Uniform_Pdf_Double(lower, lower + dx);
      double y = Get_Uniform_Pdf_Double(0, dy);

      if (y > m * x + c) {
         x += dx;
      }
   }

   WWASSERT(x - lower > -MISCUTIL_EPSILON && upper - x > -MISCUTIL_EPSILON);

   return x;
}

//-----------------------------------------------------------------------------
double cMathUtil::Get_Normalized_Hat_Pdf_Double()
{
   return Get_Hat_Pdf_Double(0, 1);
}

//-----------------------------------------------------------------------------
int cMathUtil::Get_Hat_Pdf_Int(int lower, int upper)
{
   return Round(Get_Hat_Pdf_Double(lower, upper));
}











