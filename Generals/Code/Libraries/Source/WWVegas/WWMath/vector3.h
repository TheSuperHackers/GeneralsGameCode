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

/* $Header: /Commando/Code/wwmath/vector3.h 40    5/11/01 7:11p Jani_p $ */
/*********************************************************************************************** 
 ***                  Confidential - Westwood Studios                                        *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Westwood 3D                                                  * 
 *                                                                                             * 
 *                    File Name : VECTOR3.H                                                    *  
 *                                                                                             * 
 *                   Programmer : Greg Hjelstrom                                               * 
 *                                                                                             *  
 *                   Start Date : 02/24/97                                                     * 
 *                                                                                             * 
 *                  Last Update : February 24, 1997 [GH]                                       * 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Scalar Division Operator -- Divide a vector by a scalar                                   *
 *   Scalar Multiply Operator -- Multiply a vector by a scalar                                 *
 *   Vector Addition Operator -- Add two vectors                                               *
 *   Vector Subtraction Operator -- Subract two vectors                                        *
 *   Vector Inner Product Operator -- Compute the inner or dot product                         *
 *   Vector Equality Operator -- Determine if two vectors are identical                        *
 *   Vector Inequality Operator -- Determine if two vectors are identical                      *
 *   Equal_Within_Epsilon -- Determine if two vectors are identical within                     *
 *   Cross_Product -- compute the cross product of two vectors                                 *
 *   Vector3::Normalize -- Normalizes the vector.                                              *
 *   Vector3::Length -- Returns the length of the vector                                       *
 *   Vector3::Length2 -- Returns the square of the length of the vector                        *
 *   Vector3::Quick_Length -- returns a quick approximation of the length                      *
 *   Swap -- swap two Vector3's                                                                *
 *   Lerp -- linearly interpolate two Vector3's by an interpolation factor.                    *
 *   Lerp -- linearly interpolate two Vector3's without return-by-value                        *
 *   Vector3::Add -- Add two vector3's without return-by-value                                 *
 *   Vector3::Subtract -- Subtract two vector3's without return-by-value                       *
 *   Vector3::Update_Min -- sets each component of the vector to the min of this and a         *
 *   Vector3::Update_Max -- Sets each component of the vector to the max of this and a         *
 *   Vector3::Scale -- scale this vector by 3 independent scale factors                        *
 *   Vector3::Rotate_X -- rotates this vector around the X axis                                *
 *   Vector3::Rotate_X -- Rotates this vector around the x axis                                *
 *   Vector3::Rotate_Y -- Rotates this vector around the y axis                                *
 *   Vector3::Rotate_Y -- Rotates this vector around the Y axis                                *
 *   Vector3::Rotate_Z -- Rotates this vector around the Z axis                                *
 *   Vector3::Rotate_Z -- Rotates this vector around the Z axis                                *
 *   Vector3::Is_Valid -- Verifies that each component of this vector is a valid float         *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef VECTOR3_H
#define VECTOR3_H

#include "always.h"
#include "wwmath.h"
#include <assert.h>
#ifdef _UNIX
#include "osdep.h"
#endif


/*
** Vector3 - 3-Dimensional Vectors
*/
class Vector3
{

public:


	float	X;
	float Y;
	float Z;


	// Constructors
	WWINLINE Vector3(void) {};
	WWINLINE Vector3(const Vector3 & v) { X = v.X; Y = v.Y; Z = v.Z; }
	WWINLINE Vector3(float x, float y, float z) { X = x; Y = y; Z = z; }
	WWINLINE Vector3(const float vector[3]) { X = vector[0]; Y = vector[1]; Z = vector[2]; }
	
	// Assignment
	WWINLINE Vector3 & operator = (const Vector3 & v) { X = v.X; Y = v.Y; Z = v.Z; return *this; }	
	WWINLINE void	Set(float x, float y, float z) { X = x; Y = y; Z = z; }
	WWINLINE void	Set(const Vector3 & that) { X = that.X; Y = that.Y; Z = that.Z; }

	// Array access
	WWINLINE float &	operator [](int i) { return (&X)[i]; }     
	WWINLINE const float &  operator [](int i) const { return (&X)[i]; }  

	// normalize, compute length
	void	Normalize(void);
	WWINLINE float	Length(void) const;
	WWINLINE float	Length2(void) const;
	float Quick_Length(void) const;
	void  Scale(const Vector3 & scale);

	// rotation, (warning, modifies this vector!)
	WWINLINE void	Rotate_X(float angle);
	WWINLINE void	Rotate_X(float s_angle,float c_angle);
	WWINLINE void	Rotate_Y(float angle);
	WWINLINE void	Rotate_Y(float s_angle,float c_angle);
	WWINLINE void	Rotate_Z(float angle);
	WWINLINE void	Rotate_Z(float s_angle,float c_angle);

	// unary operators
	WWINLINE Vector3 operator-() const { return(Vector3(-X,-Y,-Z)); } 
	WWINLINE Vector3 operator+() const { return *this; } 

	WWINLINE Vector3 & operator += (const Vector3 & v) { X += v.X; Y += v.Y; Z += v.Z; return *this; }	
	WWINLINE Vector3 & operator -= (const Vector3 & v) { X -= v.X; Y -= v.Y; Z -= v.Z; return *this; }		
	WWINLINE Vector3 & operator *= (float k) { X = X*k; Y=Y*k; Z=Z*k; return *this; }
	WWINLINE Vector3 & operator /= (float k) { float ook=1.0f/k; X=X*ook; Y=Y*ook; Z=Z*ook; return *this; }

	// scalar multiplication, division
	WWINLINE friend Vector3 operator * (const Vector3 &a,float k);
	WWINLINE friend Vector3 operator * (float k,const Vector3 &a);
	WWINLINE friend Vector3 operator / (const Vector3 &a,float k);

	// vector addition,subtraction
	WWINLINE friend Vector3 operator + (const Vector3 &a,const Vector3 &b);
	WWINLINE friend Vector3 operator - (const Vector3 &a,const Vector3 &b);

	// Equality operators
	friend bool operator == (const Vector3 &a,const Vector3 &b);
	friend bool operator != (const Vector3 &a,const Vector3 &b);
   WWINLINE friend bool Equal_Within_Epsilon(const Vector3 &a,const Vector3 &b,float epsilon);

	// dot product / inner product
	//WWINLINE friend float operator * (const Vector3 &a,const Vector3 &b);
	static WWINLINE float Dot_Product(const Vector3 &a,const Vector3 &b);
	
	// cross product / outer product
#ifdef ALLOW_TEMPORARIES
	static WWINLINE Vector3 Cross_Product(const Vector3 &a,const Vector3 &b);
#endif
	static WWINLINE void Cross_Product(const Vector3 &a,const Vector3 &b,Vector3 * result);
	static WWINLINE void Normalized_Cross_Product(const Vector3 &a, const Vector3 &b, Vector3 * result);
	static WWINLINE float Cross_Product_X(const Vector3 &a,const Vector3 &b);
	static WWINLINE float Cross_Product_Y(const Vector3 &a,const Vector3 &b);
	static WWINLINE float Cross_Product_Z(const Vector3 &a,const Vector3 &b);

	// add and subtract without return by value
	static WWINLINE void Add(const Vector3 & a,const Vector3 & b,Vector3 * c);
	static WWINLINE void Subtract(const Vector3 & a,const Vector3 & b,Vector3 * c);
	
	// Line intersection functions.
	static WWINLINE float Find_X_At_Y(float y, const Vector3 &p1, const Vector3 &p2);
	static WWINLINE float Find_X_At_Z(float z, const Vector3 &p1, const Vector3 &p2);
	static WWINLINE float Find_Y_At_X(float x, const Vector3 &p1, const Vector3 &p2);
	static WWINLINE float Find_Y_At_Z(float z, const Vector3 &p1, const Vector3 &p2);
	static WWINLINE float Find_Z_At_X(float x, const Vector3 &p1, const Vector3 &p2);
	static WWINLINE float Find_Z_At_Y(float z, const Vector3 &p1, const Vector3 &p2);

	// make this vector the min or max of itself and the passed vector
	WWINLINE void Update_Min(const Vector3 & a);
	WWINLINE void Update_Max(const Vector3 & a);
	WWINLINE void Cap_Absolute_To(const Vector3 & a);

	// verify that none of the members of this vector are invalid floats
	WWINLINE bool Is_Valid(void) const;

	static WWINLINE float Quick_Distance(const Vector3 &p1, const Vector3 &p2);
	static WWINLINE float Distance(const Vector3 &p1, const Vector3 &p2);

	// Linearly interpolate two Vector3's
	static void Lerp(const Vector3 & a, const Vector3 & b, float alpha,Vector3 * set_result);

	// Color Conversion
	WWINLINE unsigned	long	Convert_To_ABGR( void ) const;
	WWINLINE unsigned	long	Convert_To_ARGB( void ) const;
	WWINLINE unsigned	long	Convert_To_ARGB( float alpha ) const;
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
WWINLINE Vector3 operator * (const Vector3 &a,float k)
{
	return Vector3((a.X * k),(a.Y * k),(a.Z * k));
}

WWINLINE Vector3 operator * (float k, const Vector3 &a)
{
	return Vector3((a.X * k),(a.Y * k),(a.Z * k));
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
WWINLINE Vector3 operator / (const Vector3 &a,float k)
{
	float ook = 1.0f/k;
	return Vector3((a.X * ook),(a.Y * ook),(a.Z * ook));
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
WWINLINE Vector3 operator + (const Vector3 &a,const Vector3 &b)
{
	return Vector3(
							a.X+b.X,
							a.Y+b.Y,
							a.Z+b.Z
						);
}

/************************************************************************** 
 * Vector Subtraction Operator -- Subract two vectors                     * 
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
WWINLINE Vector3 operator - (const Vector3 &a,const Vector3 &b)
{
	return Vector3(
							a.X-b.X,
							a.Y-b.Y,
							a.Z-b.Z
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
//WWINLINE float operator * (const Vector3 &a,const Vector3 &b)
//{
//	return	a.X*b.X + 
//				a.Y*b.Y + 
//				a.Z*b.Z;
//}

WWINLINE float Vector3::Dot_Product(const Vector3 &a,const Vector3 &b)
{
	return a.X*b.X + 
				a.Y*b.Y + 
				a.Z*b.Z;
}


/************************************************************************** 
 * Vector Equality Operator -- Determine if two vectors are identical     * 
 *                                                                        * 
 * INPUT:                                                                 * 
 *                                                                        * 
 * OUTPUT:                                                                * 
 *                                                                        * 
 * WARNINGS:                                                              * 
 *                                                                        * 
 * HISTORY:                                                               * 
 *========================================================================*/
WWINLINE bool operator == (const Vector3 &a,const Vector3 &b)
{
	return ( (a.X == b.X) && (a.Y == b.Y) && (a.Z == b.Z));
}

/************************************************************************** 
 * Vector Inequality Operator -- Determine if two vectors are identical   * 
 *                                                                        * 
 * INPUT:                                                                 * 
 *                                                                        * 
 * OUTPUT:                                                                * 
 *                                                                        * 
 * WARNINGS:                                                              * 
 *                                                                        * 
 * HISTORY:                                                               * 
 *========================================================================*/
WWINLINE bool operator != (const Vector3 &a,const Vector3 &b)
{
	return ( (a.X != b.X) || (a.Y != b.Y) || (a.Z != b.Z));
}

/************************************************************************** 
 * Equal_Within_Epsilon -- Determine if two vectors are identical within e* 
 *                                                                        * 
 * INPUT:                                                                 * 
 *                                                                        * 
 * OUTPUT:                                                                * 
 *                                                                        * 
 * WARNINGS:                                                              * 
 *                                                                        * 
 * HISTORY:                                                               * 
 *========================================================================*/
WWINLINE bool Equal_Within_Epsilon(const Vector3 &a,const Vector3 &b,float epsilon)
{
   return(	(WWMath::Fabs(a.X - b.X) < epsilon) && 
				(WWMath::Fabs(a.Y - b.Y) < epsilon) && 
				(WWMath::Fabs(a.Z - b.Z) < epsilon)	);
}


/************************************************************************** 
 * Cross_Product -- compute the cross product of two vectors              * 
 *                                                                        * 
 * INPUT:                                                                 * 
 *                                                                        * 
 * OUTPUT:                                                                * 
 *                                                                        * 
 * WARNINGS:                                                              * 
 *                                                                        * 
 * HISTORY:                                                               * 
 *========================================================================*/
#ifdef ALLOW_TEMPORARIES
WWINLINE Vector3 Vector3::Cross_Product(const Vector3 &a,const Vector3 &b)
{
	return Vector3(
		(a.Y * b.Z - a.Z * b.Y),
		(a.Z * b.X - a.X * b.Z),
		(a.X * b.Y - a.Y * b.X)
	);
}
#endif

WWINLINE void Vector3::Cross_Product(const Vector3 &a,const Vector3 &b,Vector3 * set_result)
{
	assert(set_result != &a);
	set_result->X = (a.Y * b.Z - a.Z * b.Y);
	set_result->Y = (a.Z * b.X - a.X * b.Z);
	set_result->Z = (a.X * b.Y - a.Y * b.X);
}

WWINLINE void Vector3::Normalized_Cross_Product(const Vector3 &a,const Vector3 &b,Vector3 * set_result)
{
	assert(set_result != &a);
	set_result->X = (a.Y * b.Z - a.Z * b.Y);
	set_result->Y = (a.Z * b.X - a.X * b.Z);
	set_result->Z = (a.X * b.Y - a.Y * b.X);
	set_result->Normalize();
}

WWINLINE float Vector3::Cross_Product_X(const Vector3 &a,const Vector3 &b)
{
   return a.Y * b.Z - a.Z * b.Y;
}

WWINLINE float Vector3::Cross_Product_Y(const Vector3 &a,const Vector3 &b)
{
   return a.Z * b.X - a.X * b.Z;
}

WWINLINE float Vector3::Cross_Product_Z(const Vector3 &a,const Vector3 &b)
{
   return a.X * b.Y - a.Y * b.X;
}

/************************************************************************** 
 * Vector3::Normalize -- Normalizes the vector.                           * 
 *                                                                        * 
 * INPUT:                                                                 * 
 *                                                                        * 
 * OUTPUT:                                                                * 
 *                                                                        * 
 * WARNINGS:                                                              * 
 *                                                                        * 
 * HISTORY:                                                               * 
 *========================================================================*/
WWINLINE void Vector3::Normalize()
{
	float len2 = Length2();
	if (len2 != 0.0f) 
	{
		float oolen = WWMath::Inv_Sqrt(len2);
		X *= oolen;
		Y *= oolen;
		Z *= oolen;
	}
}

#ifdef ALLOW_TEMPORARIES
WWINLINE Vector3 Normalize(const Vector3 & vec)
{
	float len2 = vec.Length2();
	if (len2 != 0.0f) 
	{
		float oolen = WWMath::Inv_Sqrt(len2);
		return vec * oolen;
	}
	return vec;
}
#endif

/************************************************************************** 
 * Vector3::Length -- Returns the length of the vector                    * 
 *                                                                        * 
 * INPUT:                                                                 * 
 *                                                                        * 
 * OUTPUT:                                                                * 
 *                                                                        * 
 * WARNINGS:                                                              * 
 *                                                                        * 
 * HISTORY:                                                               * 
 *========================================================================*/
WWINLINE float Vector3::Length() const
{
	return WWMath::Sqrt(Length2());
}

/************************************************************************** 
 * Vector3::Length2 -- Returns the square of the length of the vector     * 
 *                                                                        * 
 * INPUT:                                                                 * 
 *                                                                        * 
 * OUTPUT:                                                                * 
 *                                                                        * 
 * WARNINGS:                                                              * 
 *                                                                        * 
 * HISTORY:                                                               * 
 *========================================================================*/
WWINLINE float Vector3::Length2() const
{
	return X*X + Y*Y + Z*Z;
}


/***********************************************************************************************
 * Vector3::Quick_Length -- returns a quick approximation of the length                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/15/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WWINLINE float Vector3::Quick_Length(void) const
{
	// this method of approximating the length comes from Graphics Gems 1 and
	// supposedly gives an error of +/- 8%
	float max = WWMath::Fabs(X);
	float mid = WWMath::Fabs(Y);
	float min = WWMath::Fabs(Z);
	float tmp;

	if (max < mid) { tmp = max; max = mid; mid = tmp; }
	if (max < min) { tmp = max; max = min; min = tmp; }
	if (mid < min) { tmp = mid; mid = min; min = mid; }

	return max + (11.0f / 32.0f)*mid + (1.0f / 4.0f)*min;
}


/*********************************************************************************************** 
 * Swap -- swap two Vector3's                                                                  * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   08/11/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
WWINLINE void Swap(Vector3 & a,Vector3 & b)
{
	Vector3 tmp(a);
	a = b;
	b = tmp;
}

/***********************************************************************************************
 * Lerp -- linearly interpolate two Vector3's without return-by-value                          *
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
WWINLINE void Vector3::Lerp(const Vector3 & a, const Vector3 & b, float alpha,Vector3 * set_result)
{
	assert(set_result != NULL);
	set_result->X = (a.X + (b.X - a.X)*alpha);
   set_result->Y = (a.Y + (b.Y - a.Y)*alpha);
   set_result->Z = (a.Z + (b.Z - a.Z)*alpha);
}

/***********************************************************************************************
 * Vector3::Add -- Add two vector3's without return-by-value                                   *
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
WWINLINE void Vector3::Add(const Vector3 &a,const Vector3 &b,Vector3 * set_result)
{
	assert(set_result != NULL);
	set_result->X = a.X + b.X;
	set_result->Y = a.Y + b.Y;
	set_result->Z = a.Z + b.Z;
}


/***********************************************************************************************
 * Vector3::Subtract -- Subtract two vector3's without return-by-value                         *
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
WWINLINE void Vector3::Subtract(const Vector3 &a,const Vector3 &b,Vector3 * set_result)
{
	assert(set_result != NULL);
	set_result->X = a.X - b.X;
	set_result->Y = a.Y - b.Y;
	set_result->Z = a.Z - b.Z;
}


/***********************************************************************************************
 * Vector3::Update_Min -- sets each component of the vector to the min of this and a           *
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
WWINLINE void Vector3::Update_Min(const Vector3 & a)
{	
	if (a.X < X) X = a.X;
	if (a.Y < Y) Y = a.Y;
	if (a.Z < Z) Z = a.Z;
}


/***********************************************************************************************
 * Vector3::Update_Max -- Sets each component of the vector to the max of this and a           *
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
WWINLINE void Vector3::Update_Max(const Vector3 & a)
{	
	if (a.X > X) X = a.X;
	if (a.Y > Y) Y = a.Y;
	if (a.Z > Z) Z = a.Z;
}

/***********************************************************************************************
 * Vector3::Cap_To_Absolute_Of -- Sets each component of the vector to no larger than the -ve or +ve of*
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/29/99   wst : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void Vector3::Cap_Absolute_To(const Vector3 & a)
{	
	if (X > 0)
	{
		if (a.X < X) X = a.X;
	}
	else
	{
		if (-a.X > X) X = -a.X;
	}

	if (Y > 0)
	{
		if (a.Y < Y) Y = a.Y;
	}
	else
	{
		if (-a.Y > Y) Y = -a.Y;
	}

	if (Z > 0)
	{
		if (a.Z < Z) Z = a.Z;
	}
	else
	{
		if (-a.Z > Z) Z = -a.Z;
	}

}

/***********************************************************************************************
 * Vector3::Scale -- scale this vector by 3 independent scale factors                          *
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
WWINLINE void Vector3::Scale(const Vector3 & scale)
{
	X *= scale.X;
	Y *= scale.Y;
	Z *= scale.Z;
}


/***********************************************************************************************
 * Vector3::Rotate_X -- rotates this vector around the X axis                                  *
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
WWINLINE void Vector3::Rotate_X(float angle)
{
	Rotate_X(sinf(angle),cosf(angle));
}


/***********************************************************************************************
 * Vector3::Rotate_X -- Rotates this vector around the x axis                                  *
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
WWINLINE void Vector3::Rotate_X(float s_angle,float c_angle)
{
	float tmp_y = Y;
	float tmp_z = Z;

	Y = c_angle * tmp_y - s_angle * tmp_z;
	Z = s_angle * tmp_y + c_angle * tmp_z;
}


/***********************************************************************************************
 * Vector3::Rotate_Y -- Rotates this vector around the y axis                                  *
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
WWINLINE void Vector3::Rotate_Y(float angle)
{
	Rotate_Y(sinf(angle),cosf(angle));
}


/***********************************************************************************************
 * Vector3::Rotate_Y -- Rotates this vector around the Y axis                                  *
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
WWINLINE void Vector3::Rotate_Y(float s_angle,float c_angle)
{
	float tmp_x = X;
	float tmp_z = Z;

	X = c_angle * tmp_x + s_angle * tmp_z;
	Z = -s_angle * tmp_x + c_angle * tmp_z;
}


/***********************************************************************************************
 * Vector3::Rotate_Z -- Rotates this vector around the Z axis                                  *
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
WWINLINE void Vector3::Rotate_Z(float angle)
{
	Rotate_Z(sinf(angle),cosf(angle));
}


/***********************************************************************************************
 * Vector3::Rotate_Z -- Rotates this vector around the Z axis                                  *
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
WWINLINE void Vector3::Rotate_Z(float s_angle,float c_angle)
{
	float tmp_x = X;
	float tmp_y = Y;

	X = c_angle * tmp_x - s_angle * tmp_y;
	Y = s_angle * tmp_x + c_angle * tmp_y;
}


/***********************************************************************************************
 * Vector3::Is_Valid -- Verifies that each component of this vector is a valid float           *
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
WWINLINE bool Vector3::Is_Valid(void) const
{
	return (WWMath::Is_Valid_Float(X) && WWMath::Is_Valid_Float(Y) && WWMath::Is_Valid_Float(Z));
}

WWINLINE float Vector3::Find_X_At_Y(float y, const Vector3 &p1, const Vector3 &p2) 
{
	return(p1.X + ((y - p1.Y) * ((p2.X - p1.X) / (p2.Y - p1.Y))));
}
WWINLINE float Vector3::Find_X_At_Z(float z, const Vector3 &p1, const Vector3 &p2) 
{
	return(p1.X + ((z - p1.Z) * ((p2.X - p1.X) / (p2.Z - p1.Z))));
}
WWINLINE float Vector3::Find_Y_At_X(float x, const Vector3 &p1, const Vector3 &p2)  
{
	return(p1.Y + ((x - p1.X) * ((p2.Y - p1.Y) / (p2.X - p1.X))));
}
WWINLINE float Vector3::Find_Y_At_Z(float z, const Vector3 &p1, const Vector3 &p2)  
{
	return(p1.Y + ((z - p1.Z) * ((p2.Y - p1.Y) / (p2.Z - p1.Z))));
}
WWINLINE float Vector3::Find_Z_At_X(float x, const Vector3 &p1, const Vector3 &p2)  
{
	return(p1.Z + ((x - p1.X) * ((p2.Z - p1.Z) / (p2.X - p1.X))));
}
WWINLINE float Vector3::Find_Z_At_Y(float y, const Vector3 &p1, const Vector3 &p2) 
{
	return(p1.Z + ((y - p1.Y) * ((p2.Z - p1.Z) / (p2.Y - p1.Y))));
}

/***********************************************************************************************
 * Vector3::Distance -- Accurate distance calculation.                                         *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/29/1999MLL: Created.                                                                   *
 *=============================================================================================*/
WWINLINE float Vector3::Distance(const Vector3 &p1, const Vector3 &p2)
{
	Vector3	temp;
	temp = p1 - p2;
	return (temp.Length());
}

/***********************************************************************************************
 * Vector3::Quick_Distance -- Fast but inaccurate distance calculation.                        *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/29/1999MLL: Created.                                                                   *
 *=============================================================================================*/
WWINLINE float Vector3::Quick_Distance(const Vector3 &p1, const Vector3 &p2)
{
	Vector3	temp;
	temp = p1 - p2;
	return (temp.Quick_Length());
}

/***********************************************************************************************
 * Vector3::Convert_To_ABGR -- Converts to SR packed color				.                         *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/29/1999MLL: Created.                                                                   *
 *=============================================================================================*/
WWINLINE unsigned long	Vector3::Convert_To_ABGR( void ) const 
{
	return (unsigned(255)<<24) | 
			 (unsigned(Z*255.0f)<<16) | 
			 (unsigned(Y*255.0f)<<8) | 
			 (unsigned(X*255.0f));
}

/***********************************************************************************************
 * Vector3::Convert_To_ARGB -- Converts to packed color				.                         *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/29/1999MLL: Created.                                                                   *
 *=============================================================================================*/
WWINLINE unsigned long	Vector3::Convert_To_ARGB( void ) const 
{
	return (unsigned(255)<<24) | 
			 (unsigned(X*255.0f)<<16) | 
			 (unsigned(Y*255.0f)<<8) | 
			 (unsigned(Z*255.0f));
}

WWINLINE unsigned long Vector3::Convert_To_ARGB( float alpha ) const
{
    return (unsigned(alpha * 255)<<24) |
        (unsigned(X*255.0f)<<16) |
        (unsigned(Y*255.0f)<<8) |
        (unsigned(Z*255.0f));
}

#endif /* Vector3_H */

