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
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "WWLib/always.h"
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
	WWINLINE Vector4() {};
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
	void	Normalize();
	WWINLINE float	Length() const;
	WWINLINE float Length2() const;

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
	WWINLINE bool Is_Valid() const;
};

WWINLINE Vector4 operator * (const Vector4 &a,float k)
{
	return Vector4((a.X * k),(a.Y * k),(a.Z * k),(a.W * k));
}

WWINLINE Vector4 operator * (float k, const Vector4 &a)
{
	return a*k;
}

WWINLINE Vector4 operator / (const Vector4 &a,float k)
{
	float ook=1.0f/k;
	return Vector4((a[0] * ook),(a[1] * ook),(a[2] * ook),(a[3] * ook));
}

WWINLINE Vector4 operator + (const Vector4 &a,const Vector4 &b)
{
	return Vector4(
							a[0]+b[0],
							a[1]+b[1],
							a[2]+b[2],
							a[3]+b[3]
						);
}

WWINLINE Vector4 operator - (const Vector4 &a,const Vector4 &b)
{
	return Vector4(
							a[0]-b[0],
							a[1]-b[1],
							a[2]-b[2],
							a[3]-b[3]
						);
}

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


WWINLINE bool operator == (const Vector4 &a,const Vector4 &b)
{
	return ( (a[0] == b[0]) && (a[1] == b[1]) && (a[2] == b[2]) && (a[3] == b[3]));
}

WWINLINE bool operator != (const Vector4 &a,const Vector4 &b)
{
	return ( (a[0] != b[0]) || (a[1] != b[1]) || (a[2] != b[2]) || (a[3] != b[3]));
}

WWINLINE void Vector4::Normalize()
{
	float len2 = Length2();
	if (len2 != 0.0f) {
		float oolen = WWMath::Inv_Sqrt_Legacy(len2);
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
		float oolen = WWMath::Inv_Sqrt_Legacy(len2);
		return vec * oolen;
	}
	return Vector4(0.0f,0.0f,0.0f,0.0f);
}

WWINLINE float Vector4::Length() const
{
	return WWMath::Sqrt(Length2());
}

// Returns the square of the length of the vector.
WWINLINE float Vector4::Length2() const
{
	return X*X + Y*Y + Z*Z + W*W;
}


WWINLINE void Swap(Vector4 & a,Vector4 & b)
{
	Vector4 tmp(a);
	a = b;
	b = tmp;
}

// WARNING: No checking is done to ensure that alpha is between 0 and 1.
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

WWINLINE bool Vector4::Is_Valid() const
{
	return (WWMath::Is_Valid_Float(X) && WWMath::Is_Valid_Float(Y) && WWMath::Is_Valid_Float(Z) && WWMath::Is_Valid_Float(W));
}
