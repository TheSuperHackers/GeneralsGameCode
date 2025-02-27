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

/* $Header: /Commando/Code/wwmath/vector2.h 19    9/01/00 5:30p Ian_l $ */
/***********************************************************************************************
 ***                  Confidential - Westwood Studios                                        ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Westwood 3D                                                  *
 *                                                                                             *
 *                    File Name : VECTOR2.h                                                    *
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
 *   Vector Equality Operator -- Detemine if two vectors are identical                         *
 *   Equal_Within_Epsilon -- Determine if two vectors are identical within                     *
 *   Vector Inequality Operator -- Detemine if two vectors are identical                       *
 *   Swap -- swap two Vector2's                                                                *
 *   Vector2::Is_Valid -- Verifies that all components are valid floats                        *
 *	  Vector2::Update_Min -- sets each component of the vector to the min of this and a.        *
 *	  Vector2::Update_Max -- sets each component of the vector to the max of this and a.        *
 *   Vector2::Scale -- multiply components of a vector by independant scaling factors.			  *
 *   Vector2::Lerp -- linearly interpolates two Vector2's                                      *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "wwmath.h"
#include <math.h>


/*
** 2-Dimensional Vectors
*/

class Vector2
{

public:

	union {
		float	X;
		float U;
	};

	union {
		float	Y;
		float V;
	};

	// Constructors
	Vector2(void) {};
	Vector2(const Vector2 & v) { X = v.X; Y = v.Y; }
	Vector2(float x, float y) { X = x; Y = y; }
	Vector2(const float vector[2]) { X = vector[0]; Y = vector[1]; }


	// Assignment
	Vector2 & operator = (const Vector2 & v) { X = v[0]; Y = v[1]; return *this; }
	void	Set(float x, float y) { X = x; Y = y; }
	void	Set(const Vector2 & v) { X = v.X; Y = v.Y; }

	// Array access
	float &	operator [](int i) { return (&X)[i]; }
	const float &  operator [](int i) const { return (&X)[i]; }

	// normalize, compute length
	void	Normalize(void);
	float	Length(void) const;
	float	Length2(void) const;

	// unary operators
	Vector2 operator-() const { return Vector2(-X,-Y); }
	Vector2 operator+() const { return *this; }

	Vector2 & operator += (const Vector2 & v) { X += v.X; Y += v.Y; return *this; }
	Vector2 & operator -= (const Vector2 & v) { X -= v.X; Y -= v.Y; return *this; }
	Vector2 & operator *= (float k) { X = (float)(X*k); Y=(float)(Y*k); return *this; }
	Vector2 & operator /= (float k) { k=1.0f/k; X*=k; Y*=k; return *this; }

	// scalar multiplication, division
	friend Vector2 operator * (const Vector2 &a,float k);
	friend Vector2 operator * (float k,const Vector2 &a);
 	friend Vector2 operator / (const Vector2 &a,float k);

	// vector addition,subtraction
	friend Vector2 operator + (const Vector2 &a,const Vector2 &b);
	friend Vector2 operator - (const Vector2 &a,const Vector2 &b);

	// dot product / inner product
	friend float operator * (const Vector2 &a,const Vector2 &b);
	static float Dot_Product(const Vector2 &a,const Vector2 &b);

	// dot product between a and perpendicular vector to b
	static float Perp_Dot_Product(const Vector2 &a,const Vector2 &b);

	// Equality operators
	friend bool operator == (const Vector2 &a,const Vector2 &b);
	friend bool operator != (const Vector2 &a,const Vector2 &b);
   friend bool Equal_Within_Epsilon(const Vector2 &a,const Vector2 &b,float epsilon);

	// Rotation
	void Rotate(float theta);
	void Rotate(float s, float c);
	bool Rotate_Towards_Vector(Vector2 &target, float max_theta, bool & positive_turn);
	bool Rotate_Towards_Vector(Vector2 &target, float max_s, float max_c, bool & positive_turn);

	// verify that none of the members of this vector are invalid floats
	bool Is_Valid(void) const;

	// make this vector the min or max of itself and the passed vector
	void Update_Min (const Vector2 & a);
	void Update_Max (const Vector2 & a);
	void Scale (float a, float b);

	static float Distance(const Vector2 &p1, const Vector2 &p2);
	static float Quick_Distance(const Vector2 &p1, const Vector2 &p2);

	// interpolate between two Vector2's
	static void Lerp(const Vector2 & a,const Vector2 & b,float t,Vector2 * set_result);
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
inline Vector2 operator * (const Vector2 &a,float k)
{
	return Vector2((a[0] * k),(a[1] * k));
}

inline Vector2 operator * (float k, const Vector2 &a)
{
	return Vector2((a[0] * k),(a[1] * k));
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
inline Vector2 operator / (const Vector2 &a,float k)
{
	float ook=1.0f/k;
	return Vector2((a[0] * ook),(a[1] * ook));
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
inline Vector2 operator + (const Vector2 &a,const Vector2 &b)
{
	return Vector2(
							a.X + b.X,
							a.Y + b.Y
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
inline Vector2 operator - (const Vector2 &a,const Vector2 &b)
{
	return Vector2(
							a.X - b.X,
							a.Y - b.Y
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
inline float operator * (const Vector2 &a,const Vector2 &b)
{
	return	a.X*b.X + a.Y*b.Y;
}

inline float Vector2::Dot_Product(const Vector2 &a,const Vector2 &b)
{
	return a*b;
}

inline float Vector2::Perp_Dot_Product(const Vector2 &a,const Vector2 &b)
{
	return a.X * -b.Y + a.Y * b.X;
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
inline bool operator == (const Vector2 &a,const Vector2 &b)
{
	return ( (a[0] == b[0]) && (a[1] == b[1]));
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
inline bool operator != (const Vector2 &a,const Vector2 &b)
{
	return ( (a[0] != b[0]) || (a[1] != b[1]));
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
inline bool Equal_Within_Epsilon(const Vector2 &a,const Vector2 &b,float epsilon)
{
   return( (WWMath::Fabs(a.X - b.X) < epsilon) && (WWMath::Fabs(a.Y - b.Y) < epsilon) );
}

/**************************************************************************
 * Vector2::Normalize -- Normalizes the vector.                           *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *========================================================================*/
inline void Vector2::Normalize()
{
	float len2 = Length2();
	if (len2 != 0.0f) {
		float oolen = WWMath::Inv_Sqrt(len2);
		X *= oolen;
		Y *= oolen;
	}
}

inline Vector2 Normalize(const Vector2 & vec)
{
	float len2 = vec.Length2();
	if (len2 != 0.0f) {
		float oolen = WWMath::Inv_Sqrt(len2);
		return vec / oolen;
	}
}

/**************************************************************************
 * Vector2::Length -- Returns the length of the vector                    *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *========================================================================*/
inline float Vector2::Length() const
{
	return (float)WWMath::Sqrt(Length2());
}

/**************************************************************************
 * Vector2::Length -- Returns the square of the length of the vector      *
 *                                                                        *
 * INPUT:                                                                 *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *========================================================================*/
inline float Vector2::Length2() const
{
	return (X*X + Y*Y);
}

/**************************************************************************
 * Vector2::Rotate -- Rotate vector                                       *
 *                                                                        *
 * INPUT:                                                                 *
 * float theta - angle to rotate                                          *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *========================================================================*/
inline void Vector2::Rotate(float theta)
{
	Rotate(sin(theta), cos(theta));
}

/**************************************************************************
 * Vector2::Rotate -- Rotate vector                                       *
 *                                                                        *
 * INPUT:                                                                 *
 * s - sine of the angle                                                  *
 * c - cosine of the angle                                                *
 *                                                                        *
 * OUTPUT:                                                                *
 *                                                                        *
 * WARNINGS:                                                              *
 *                                                                        *
 * HISTORY:                                                               *
 *========================================================================*/
inline void Vector2::Rotate(float s, float c)
{
	float new_x = X * c + Y * -s;
	float new_y = X * s + Y * c;
	X = new_x;
	Y = new_y;
}

/**************************************************************************
 * Vector2::Rotate -- Rotate towards given vector (stop on reaching it)   *
 *                                                                        *
 * INPUT:                                                                 *
 * float theta - angle to rotate                                          *
 *                                                                        *
 * OUTPUT:                                                                *
 * bool - true if we have reached the desired vector                      *
 *                                                                        *
 * WARNINGS:                                                              *
 * This function assumes both vectors are normalized!                     *
 *                                                                        *
 * HISTORY:                                                               *
 *========================================================================*/
inline bool Vector2::Rotate_Towards_Vector(Vector2 &target, float max_theta, bool & positive_turn)
{
	return Rotate_Towards_Vector(target, sin(max_theta), cos(max_theta), positive_turn);
}

/**************************************************************************
 * Vector2::Rotate -- Rotate towards given vector (stop on reaching it)   *
 *                                                                        *
 * INPUT:                                                                 *
 * s - sine of the angle                                                  *
 * c - cosine of the angle                                                *
 *                                                                        *
 * OUTPUT:                                                                *
 * bool - true if we have reached the desired vector                      *
 *                                                                        *
 * WARNINGS:                                                              *
 * This function assumes both vectors are normalized!                     *
 *                                                                        *
 * HISTORY:                                                               *
 *========================================================================*/
inline bool Vector2::Rotate_Towards_Vector(Vector2 &target, float max_s, float max_c, bool & positive_turn)
{
	bool return_value = false;

	positive_turn = Vector2::Perp_Dot_Product(target, *this) > 0.0f;

	if (Vector2::Dot_Product(*this, target) >= max_c) {
		Set(target);
		return_value = true;
	} else {
		// Determine turn direction and rotate accordingly.
		if (positive_turn) {
			Rotate(max_s, max_c);
		} else {
			Rotate(-max_s, max_c);
		}
	}

	return return_value;
}

/***********************************************************************************************
 * Swap -- swap two Vector2's                                                                  *
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
inline void Swap(Vector2 & a,Vector2 & b)
{
	Vector2 tmp(a);
	a = b;
	b = tmp;
}


/***********************************************************************************************
 * Vector2::Is_Valid -- Verifies that all components are valid floats                          *
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
inline bool Vector2::Is_Valid(void) const
{
	return (WWMath::Is_Valid_Float(X) && WWMath::Is_Valid_Float(Y));
}


/***********************************************************************************************
 * Vector2::Update_Min -- Set each component of the vector to the min of this and a.			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/12/00   IML : Created.                                                                 *
 *=============================================================================================*/
inline void Vector2::Update_Min (const Vector2 & a)
{
	if (a.X < X) X = a.X;
	if (a.Y < Y) Y = a.Y;
}


/***********************************************************************************************
 * Vector2::Update_Max -- Set each component of the vector to the max of this and a.			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/12/00   IML : Created.                                                                 *
 *=============================================================================================*/
inline void Vector2::Update_Max (const Vector2 & a)
{
	if (a.X > X) X = a.X;
	if (a.Y > Y) Y = a.Y;
}


/***********************************************************************************************
 * Vector2::Scale -- multiply components of a vector by independant scaling factors.			  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/19/2000  IML : Created.                                                                 *
 *=============================================================================================*/
inline void Vector2::Scale (float a, float b)
{
	X *= a;
	Y *= b;
}


/***********************************************************************************************
 * Quick_Distance -- Fast but inaccurate 2D distance calculation.                              *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/29/1999MLL: Created.                                                                   *
 *=============================================================================================*/
inline float Quick_Distance(float x1, float y1, float x2, float y2)
{
	// From Graphic Gems I.
	float x_diff = x1 - x2;
	float y_diff = y1 - y2;

	WWMath::Fabs(x_diff);
	WWMath::Fabs(y_diff);

	if (x_diff > y_diff)
	{
		return ((y_diff / 2) + x_diff);
	}
	else
	{
		return ((x_diff / 2) + y_diff);
	}
}

inline float Vector2::Quick_Distance(const Vector2 &a, const Vector2 &b)
{
	return ::Quick_Distance(a.X, a.Y, b.X, b.Y);
}


/***********************************************************************************************
 * Distance -- Accurate distance 2D calculation.                                               *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/29/1999MLL: Created.                                                                   *
 *=============================================================================================*/
inline float Vector2::Distance(const Vector2 &a, const Vector2 &b)
{
	Vector2 temp;
	temp = a - b;

	return (temp.Length());
}

inline float Distance(float x1, float y1, float x2, float y2)
{
	float x_diff = x1 - x2;
	float y_diff = y1 - y2;

	return (WWMath::Sqrt((x_diff * x_diff) + (y_diff * y_diff)));
}


/***********************************************************************************************
 * Vector2::Lerp -- linearly interpolates two Vector2's                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/14/2000  gth : Created.                                                                 *
 *=============================================================================================*/
inline void Vector2::Lerp(const Vector2 & a,const Vector2 & b,float t,Vector2 * set_result)
{
	assert(set_result != NULL);
	set_result->X = (a.X + (b.X - a.X)*t);
   set_result->Y = (a.Y + (b.Y - a.Y)*t);
}
