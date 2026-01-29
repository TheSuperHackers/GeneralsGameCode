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

/* $Header: /Commando/Code/wwmath/vector4.h 16    5/11/01 7:11p Jani_p $ */
/***************************************************************************
 ***                  Confidential - Westwood Studios                    ***
 ***************************************************************************
 *                                                                         *
 *                 Project Name : Westwood 3D                              *
 *                                                                         *
 *                    File Name : VECTOR4.h                                *
 *                                                                         *
 *                   Programmer : Greg Hjelstrom                           *
 *                                                                         *
 *                   Start Date : 02/24/97                                 *
 *                                                                         *
 *                  Last Update : June 2, 1997 [GH]                        *
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 *   Scalar Division Operator -- Divide a vector by a scalar               *
 *   Scalar Multiply Operator -- Multiply a vector by a scalar             *
 *   Vector Addition Operator -- Add two vectors                           *
 *   Vector Subtraction Operator -- Subtract two vectors                   *
 *   Vector Inner Product Operator -- Compute the inner or dot product     *
 *   Vector Equality Operator -- Detemine if two vectors are identical     *
 *   Vector Inequality Operator -- Detemine if two vectors are identical   *
 *   Swap -- swap two Vector4's                                            *
 *   Vector4::Is_Valid -- Vector4::Is_Valid                                *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "wwmath.h"
#include <math.h>

/*
** Vector4 - 4 dimensional vectors
*/
class Vector4
{

public:

	float X;
	float Y;
	float Z;
	float W;


	// Constructors
	WWINLINE Vector4(void) {};
	WWINLINE Vector4(const Vector4 & v) { X = v.X; Y = v.Y; Z = v.Z; W = v.W; }
	WWINLINE Vector4(float x, float y, float z, float w) { X = x; Y = y; Z = z; W = w; }
	WWINLINE Vector4(const float vector[4]) { X = vector[0]; Y = vector[1]; Z = vector[2]; W = vector[3]; }

	// Assignment
	WWINLINE Vector4 & operator = (const Vector4 & v) { X = v.X; Y = v.Y; Z = v.Z; W = v.W; return *this; }
	WWINLINE void	Set(float x, float y, float z, float w) { X = x; Y = y; Z = z; W = w; }

	// Array access
	WWINLINE float &	operator [](int i) { return (&X)[i]; }
	WWINLINE const float &  operator [](int i) const { return (&X)[i]; }

	// normalize, compute length
	void	Normalize(void);
	WWINLINE float	Length(void) const;
	WWINLINE float Length2(void) const;

	// unary operators
	WWINLINE Vector4 operator-() const { return(Vector4(-X,-Y,-Z,-W)); }
	WWINLINE Vector4 operator+() const { return *this; }

	WWINLINE Vector4 & operator += (const Vector4 & v) { X += v.X; Y += v.Y; Z += v.Z; W += v.W; return *this; }
	WWINLINE Vector4 & operator -= (const Vector4 & v) { X -= v.X; Y -= v.Y; Z -= v.Z; W += v.W; return *this; }
	WWINLINE Vector4 & operator *= (float k) { X = X*k; Y=Y*k; Z=Z*k; W=W*k; return *this; }
	WWINLINE Vector4 & operator /= (float k) { k=1.0f/k; X = X*k; Y=Y*k; Z=Z*k; W=W*k; return *this; }

	// scalar multiplication, division
	WWINLINE friend Vector4 operator * (const Vector4 &a,float k);
	WWINLINE friend Vector4 operator * (float k,const Vector4 &a);
	WWINLINE friend Vector4 operator / (const Vector4 &a,float k);

	// vector addition,subtraction
	WWINLINE friend Vector4 operator + (const Vector4 &a,const Vector4 &b);
	WWINLINE friend Vector4 operator - (const Vector4 &a,const Vector4 &b);

	// dot product / inner product
	WWINLINE friend float operator * (const Vector4 &a,const Vector4 &b);
	static WWINLINE float Dot_Product(const Vector4 &a,const Vector4 &b);

	// Equality operators
	friend bool operator == (const Vector4 &a,const Vector4 &b);
	friend bool operator != (const Vector4 &a,const Vector4 &b);

	// Linearly interpolate between two Vector4's
	static Vector4 Lerp(const Vector4 & a, const Vector4 & b, float alpha);
	static void Lerp(const Vector4 & a, const Vector4 & b, float alpha,Vector4 * set_result);

	// verify that none of the members of this vector are invalid floats
	WWINLINE bool Is_Valid(void) const;
};

/**************************************************************************
 * Scalar Multiply Operator -- Multiply a vector by a scalar              *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   02/24/1997 GH  : Created.                                            *
 *========================================================================*/
WWINLINE Vector4 operator * (const Vector4 &a,float k)
{
	return Vector4((a.X * k),(a.Y * k),(a.Z * k),(a.W * k));
}

WWINLINE Vector4 operator * (float k, const Vector4 &a)
{
	return a*k;
}

/**************************************************************************
 * Scalar Division Operator -- Divide a vector by a scalar                *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *========================================================================*/
WWINLINE Vector4 operator / (const Vector4 &a,float k)
{
	float ook=1.0f/k;
	return Vector4((a[0] * ook),(a[1] * ook),(a[2] * ook),(a[3] * ook));
}

/**************************************************************************
 * Vector Addition Operator -- Add two vectors                            *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   02/24/1997 GH  : Created.                                            *
 *========================================================================*/
WWINLINE Vector4 operator + (const Vector4 &a,const Vector4 &b)
{
	return Vector4(
							a[0]+b[0],
							a[1]+b[1],
							a[2]+b[2],
							a[3]+b[3]
						);
}

/**************************************************************************
 * Vector Subtraction Operator -- Subtract two vectors                    *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *   02/24/1997 GH  : Created.                                            *
 *========================================================================*/
WWINLINE Vector4 operator - (const Vector4 &a,const Vector4 &b)
{
	return Vector4(
							a[0]-b[0],
							a[1]-b[1],
							a[2]-b[2],
							a[3]-b[3]
						);
}

/**************************************************************************
 * Vector Inner Product -- Compute the inner or dot product of two vector *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *========================================================================*/
WWINLINE float operator * (const Vector4 &a,const Vector4 &b)
{
	return	a[0]*b[0] +
				a[1]*b[1] +
				a[2]*b[2] +
				a[3]*b[3];
}

WWINLINE float Vector4::Dot_Product(const Vector4 &a,const Vector4 &b)
{
	return a*b;
}


/**************************************************************************
 * Vector Equality Operator -- Detemine if two vectors are identical      *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *========================================================================*/
WWINLINE bool operator == (const Vector4 &a,const Vector4 &b)
{
	return ( (a[0] == b[0]) && (a[1] == b[1]) && (a[2] == b[2]) && (a[3] == b[3]));
}

/**************************************************************************
 * Vector Inequality Operator -- Detemine if two vectors are identical    *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *========================================================================*/
WWINLINE bool operator != (const Vector4 &a,const Vector4 &b)
{
	return ( (a[0] != b[0]) || (a[1] != b[1]) || (a[2] != b[2]) || (a[3] != b[3]));
}

/**************************************************************************
 * Vector4::Normalize -- Normalizes the vector.                           *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *========================================================================*/
WWINLINE void Vector4::Normalize()
{
	float len2 = Length2();
	if (len2 != 0.0f) {
		float oolen = WWMath::Inv_Sqrt(len2);
		X *= oolen;
		Y *= oolen;
		Z *= oolen;
		W *= oolen;
	}
}

WWINLINE Vector4 Normalize(const Vector4 & vec)
{
	float len2 = vec.Length2();
	if (len2 != 0.0f) {
		float oolen = WWMath::Inv_Sqrt(len2);
		return vec * oolen;
	}
	return Vector4(0.0f,0.0f,0.0f,0.0f);
}

/**************************************************************************
 * Vector4::Length -- Returns the length of the vector                    *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *========================================================================*/
WWINLINE float Vector4::Length() const
{
	return WWMath::Sqrt(Length2());
}

/**************************************************************************
 * Vector4::Length -- Returns the square of the length of the vector      *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *========================================================================*/
WWINLINE float Vector4::Length2() const
{
	return X*X + Y*Y + Z*Z + W*W;
}


/***********************************************************************************************
 * Swap -- swap two Vector4's                                                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/02/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void Swap(Vector4 & a,Vector4 & b)
{
	Vector4 tmp(a);
	a = b;
	b = tmp;
}

/***********************************************************************************************
 * Lerp -- linearly interpolate two Vector4's by an interpolation factor.                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS: No checking is done to ensure that alpha is between 0 and 1.                      *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/14/1999 NH  : Created.                                                                 *
 *=============================================================================================*/
WWINLINE Vector4 Lerp(const Vector4 & a, const Vector4 & b, float alpha)
{
   return Vector4(
      (a.X + (b.X - a.X)*alpha),
      (a.Y + (b.Y - a.Y)*alpha),
      (a.Z + (b.Z - a.Z)*alpha),
      (a.W + (b.W - a.W)*alpha)
   );
}

WWINLINE Vector4 Vector4::Lerp(const Vector4 & a, const Vector4 & b, float alpha)
{
   return Vector4(
      (a.X + (b.X - a.X)*alpha),
      (a.Y + (b.Y - a.Y)*alpha),
      (a.Z + (b.Z - a.Z)*alpha),
      (a.W + (b.W - a.W)*alpha)
   );
}

WWINLINE void Vector4::Lerp(const Vector4 & a, const Vector4 & b, float alpha,Vector4 * set_result)
{
   set_result->X = (a.X + (b.X - a.X)*alpha);
	set_result->Y = (a.Y + (b.Y - a.Y)*alpha);
	set_result->Z = (a.Z + (b.Z - a.Z)*alpha);
	set_result->X = (a.W + (b.W - a.W)*alpha);
}

/***********************************************************************************************
 * Vector4::Is_Valid -- Vector4::Is_Valid                                                      *
 *                                                                                             *
 *    verifies that all members of this vector are valid floats                                *
 *                                                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   10/18/99   gth : Created.                                                                 *
 *=============================================================================================*/
WWINLINE bool Vector4::Is_Valid(void) const
{
	return (WWMath::Is_Valid_Float(X) && WWMath::Is_Valid_Float(Y) && WWMath::Is_Valid_Float(Z) && WWMath::Is_Valid_Float(W));
}
