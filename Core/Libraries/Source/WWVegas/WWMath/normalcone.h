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
 *                 Project Name : G                                                            *
 *                                                                                             *
 *                     $Archive:: /G/WWMath/normalcone.h                                      $*
 *                                                                                             *
 *                      $Author:: Eric_c                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/01/99 2:44p                                              $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

/*
**	The NormalCone is a class used to represent a cone of unit length. It can be used to
** loosely represent a collection of other normals allowing backface culling to be
** performed at a heirarchial level rather than at a triangle level.
**
**	The term 'NormalCone' is a bit of a misnomer; it is really a circular portion of a sphere.
** -ehc
*/

#pragma once

#include "vector3.h"
#include "matrix3.h"

class NormalCone : public Vector3
{
	public:

		NormalCone() {}

		NormalCone(const Vector3 & normal, float angle = 1.0f)
		:	Vector3(normal),
			Angle(angle)
		{}

		void Set(const Vector3 & normal, float angle = 1.0f)
		{
			Vector3::Set(normal);
			Angle = angle;
		}

		void Set(const NormalCone & src)
		{
			Set(src, src.Angle);
		}

		// return true if this object has degenerated into a sphere.
		inline bool Complete_Sphere()
		{
			return (Angle - WWMATH_EPSILON) <= -1.0f;
		}

		// find the two vectors on the edge of the cone residing on the same plane as the input vector.
		// Note: these two Get_Coplanar functions return floats in an attempt to reduce float/int CPU state changes...
		inline float Get_Coplanar_Normals(const Vector3 & Input, Vector3 & Output1, Vector3 & Output2) const;

		// find the two vectors on the edge of the cone residing on the same plane as the input vector and their dot products with the input.
		inline float Get_Coplanar_Normals_And_Dots(const Vector3 & Input, Vector3 & Output1, Vector3 & Output2, float & dot1, float & dot2) const;

		// evaluate the input vector, expanding the angle of the cone and recalculating the
		// new center vector as needed.
		inline void Merge(const Vector3 & Input);

		// merge the input normal cone's coplanar normals with this object.
		inline void Merge(const NormalCone & Input);

		// this function returns the dot product between the input vector and the nearest coplanar normal
		// contained by the cone.
		// If the input vector is also contained by the cone, the result is always 1.0f.
		// Note that in the case of a complete sphere, the nearest coplanar normal will be pointing in
		// the opposite direction of the input vector.
		inline float Smallest_Dot_Product(const Vector3 & Input);

		// this value is the dot product of the edge of the cone and the center of the cone.
		// A value of 1.0f indicates that it is a degenerate cone which is basically a cone with radius zero.
		// A value of zero indicates the cone is actually hemisphere.
		// A value of -1.0f indicates that it is a complete sphere.
		float Angle;

};


// find the two vectors on the edge of the cone residing on the same plane as the input vector.
inline float NormalCone::Get_Coplanar_Normals(const Vector3 & Input, Vector3 & Output1, Vector3 & Output2) const
{
	// get the cross product of the existing normal and the new one
	Vector3 cross;
	Vector3::Cross_Product(Input, *this, & cross);

	float length = cross.Length2();
	if(length < WWMATH_EPSILON)
		return 0.0f;

	length = sqrt(length);
	cross /= length;

	// Make a matrix3 which uses it as an axis of rotation and
	// rotate this about that axis twice, once +Angle, once -Angle.
	float radians = (1.0f - Angle) * WWMATH_PI * 0.5f;
	Matrix3 m1(cross, radians);
	Matrix3 m2(cross, -radians);

	Matrix3::Rotate_Vector(m1, *this, & Output1);
	Matrix3::Rotate_Vector(m2, *this, & Output2);

	return length;
}

inline float NormalCone::Get_Coplanar_Normals_And_Dots(const Vector3 & Input, Vector3 & Output1, Vector3 & Output2, float & Dot1, float & Dot2) const
{
	float length = Get_Coplanar_Normals(Input, Output1, Output2);

	if(length < WWMATH_EPSILON)
		return 0.0f;

	// get the dot products of the new normal with the two coplanar normals and the current average
	Dot1 = Vector3::Dot_Product(Input, Output1);
	Dot2 = Vector3::Dot_Product(Input, Output2);
	return length;
}

// evaluate the input vector, expanding the angle of the cone and recalculating the
// new center vector as needed.
inline void NormalCone::Merge(const Vector3 & Input)
{
	// early exit if this normal cone has already turned into a complete sphere.
	if(Complete_Sphere())
		return;

	// get the dot of the new vector with the current center vector
	float dot0 = Vector3::Dot_Product(Input, * this) + WWMATH_EPSILON;

	// if the dot value is greater than the existing cone angle, then the new vector fits
	// within the cone, so return.
	if(dot0 >= Angle)
		return;

	// get the two normals found in the cone which are coplanar to the one passed to this function.
	Vector3 normal1, normal2;
	float dot1, dot2;
	if(Get_Coplanar_Normals_And_Dots(Input, normal1, normal2, dot1, dot2) <= WWMATH_EPSILON)
		return;

	// test the case where the current average has a lower dot than either of the coplanar normals.
	// If true, this means that the object now represents a complete sphere with normals facing every
	// direction.
	if((dot0 < dot1) && (dot0 < dot2))
	{
		Angle = -1;
		return;
	}

	// the smaller of the dot values we have is going to indicate which of the two coplanar normals to use
	// for averaging into the new center normal.
	if(dot1 < dot2)
		Set(Input + normal1, dot1);
	else
		Set(Input + normal2, dot2);

	// if the angle is < 0, reverse the direction of the averaged normal since we have constructed
	// something more like a sphere with a cone shape taken out of it (a negative cone).
	if(Angle < WWMATH_EPSILON)
		*this *= -1;

	Normalize();
}


// merge the input normal cone's coplanar normals with this object.
inline void NormalCone::Merge(const NormalCone & Input)
{
	Vector3 n1, n2;

	if(Input.Get_Coplanar_Normals(*this, n1,n2) >= WWMATH_EPSILON)
	{
		Merge(n1);
		Merge(n2);
	}
}

// this function returns the dot product between the input vector and the nearest coplanar normal
// contained by the cone.
// If the input vector is also contained by the cone, the result is always 1.0f.
// Note that in the case of a complete sphere, the nearest coplanar normal will be pointing in
// the opposite direction of the input vector.
inline float NormalCone::Smallest_Dot_Product(const Vector3 & Input)
{
	if(Complete_Sphere())
		return -1.0f;

		// get the dot of the new vector with the current center vector
	float dot0 = Vector3::Dot_Product(Input, * this);

	// if the negative dot value is greater than the existing cone angle, then the new vector is
	// parallel to one of the vectors contained in the cone but in negative
	// direction, so return -1.0f
	if(-dot0 + WWMATH_EPSILON >= Angle)
		return -1.0f;

	// if the dot value is greater than the existing cone angle, then the new vector is
	// parallel to one of the vectors contained in the cone, so return 1.0f
	if(dot0 + WWMATH_EPSILON >= Angle)
		return 1.0f;

	// get the two normals found in the cone which are coplanar to the one passed to this function.
	Vector3 normal1, normal2;
	float dot1, dot2;
	Get_Coplanar_Normals_And_Dots(Input, normal1, normal2, dot1, dot2);

	// return the smaller of the two dot products
	if(dot1 < dot2)
		return dot1;
	return dot2;
}
