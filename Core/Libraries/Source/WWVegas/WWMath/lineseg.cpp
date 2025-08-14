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
 *                 Project Name : WWMath                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwmath/lineseg.cpp                           $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 3/16/00 3:16p                                               $*
 *                                                                                             *
 *                    $Revision:: 25                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   LineSegClass::Set -- Initialize this 'lineseg' by transforming another 'lineseg'          *
 *   LineSegClass::Set_Random -- create a random linesegment within the given space            *
 *   LineSegClass::Find_Point_Closest_To -- Finds point on line closest to point supplied.     *
 *   LineSegClass::Find_Intersection -- Finds the closest points on the two lines				  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "lineseg.h"
//#include <stdlib.h>

#include "matrix3d.h"

/***********************************************************************************************
 * LineSegClass::Set -- Initialize this 'lineseg' by transforming another 'lineseg'            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/17/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void LineSegClass::Set(const LineSegClass & that,const Matrix3D & tm)
{
	/*
	** Transform P0 and P1
	*/
	Matrix3D::Transform_Vector(tm,that.P0,&P0);
	Matrix3D::Transform_Vector(tm,that.P1,&P1);

	/*
	** Just calculate DP
	*/
	DP = P1 - P0;

	/*
	** Rotate the direction vector
	*/
	Matrix3D::Rotate_Vector(tm,that.Dir,&Dir);

	/*
	** Length should be un-changed
	*/
	Length = that.Length;
}

/***********************************************************************************************
 * LineSegClass::Set_Random -- create a random linesegment within the given space              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/21/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void LineSegClass::Set_Random(const Vector3 & min,const Vector3 & max)
{
	float frac;

	frac = WWMath::Random_Float();
	P0.X = min.X + frac * (max.X - min.X);
	frac = WWMath::Random_Float();
	P0.Y = min.Y + frac * (max.Y - min.Y);
	frac = WWMath::Random_Float();
	P0.Z = min.Z + frac * (max.Z - min.Z);

	frac = WWMath::Random_Float();
	P1.X = min.X + frac * (max.X - min.X);
	frac = WWMath::Random_Float();
	P1.Y = min.Y + frac * (max.Y - min.Y);
	frac = WWMath::Random_Float();
	P1.Z = min.Z + frac * (max.Z - min.Z);

	DP = P1 - P0;
	Dir = DP;
	Dir.Normalize();
	Length = DP.Length();
}




/***********************************************************************************************
 * LineSegClass::Find_Point_Closest_To -- Finds point on line closest to point supplied.       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   07/26/1999 SKB : Created.                                                                 *
 *=============================================================================================*/
Vector3 LineSegClass::Find_Point_Closest_To(const Vector3 &pos) const
{
	// Get a vector from one line endpoint to point in question.
	Vector3 v_0_pos = (pos - P0);
	float dotprod = Vector3::Dot_Product(Dir, v_0_pos);

	// Check to see if point is past either of the endpoints.
	// (Unable to draw a perpendicular line from the point to the line segment.)
	if (dotprod <= 0.0) {
		return(P0);
	} else if (dotprod >= Length) {
		return(P1);
	}

	// Find point on line seg that is closest to pos passed in.
	Vector3 point = P0 + (dotprod * Dir);
	return(point);
}


/***********************************************************************************************
 * LineSegClass::Find_Intersection -- Finds the closest points on the two lines..				  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/03/2000 PDS : Created.                                                                 *
 *=============================================================================================*/
bool
LineSegClass::Find_Intersection
(
	const LineSegClass &	other_line,
	Vector3 *				p1,
	float	*					fraction1,
	Vector3 *				p2,
	float *					fraction2
) const
{
	bool retval = false;

#ifdef ALLOW_TEMPORARIES
	Vector3 cross1 = Vector3::Cross_Product (Dir, other_line.Dir);
	Vector3 cross2 = Vector3::Cross_Product (other_line.P0 - P0, other_line.Dir);
	float top1		= cross2 * cross1;
	float bottom1	= cross1 * cross1;

	Vector3 cross3 = Vector3::Cross_Product (other_line.Dir, Dir);
	Vector3 cross4 = Vector3::Cross_Product (P0 - other_line.P0, Dir);
	float top2		= cross4 * cross3;
	float bottom2	= cross3 * cross3;
#else
	Vector3 cross1, cross2, cross3, cross4;

	Vector3::Cross_Product(Dir, other_line.Dir, &cross1);
	Vector3::Cross_Product(other_line.P0 - P0, other_line.Dir, &cross2);
	float top1		= Vector3::Dot_Product(cross2, cross1);
	float bottom1	= Vector3::Dot_Product(cross1, cross1);

	Vector3::Cross_Product(other_line.Dir, Dir, &cross3);
	Vector3::Cross_Product(P0 - other_line.P0, Dir, &cross4);
	float top2		= Vector3::Dot_Product(cross4, cross3);
	float bottom2	= Vector3::Dot_Product(cross3, cross3);
#endif

	//
	//	If either of the divisors are 0, then the lines are parallel
	//
	if (bottom1 != 0 && bottom2 != 0) {
		float length1 = top1 / bottom1;
		float length2 = top2 / bottom2;

		//
		//	Calculate the closest points on both lines.
		// Note:  If the points are the same, then the lines intersect.
		//
		(*p1) = P0 + (Dir * length1);
		(*p2) = other_line.P0 + (other_line.Dir * length2);

		//
		//	Return the fractions if they caller wants them
		//
		if (fraction1 != NULL) {
			(*fraction1) = length1 / Length;
		}

		if (fraction2 != NULL) {
			(*fraction2) = length2 / Length;
		}

		retval = true;
	}

	return retval;
}

