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

/* $Header: /Commando/Code/Tools/max2w3d/w3dquat.h 27    2/03/00 4:55p Jason_a $ */
/***************************************************************************
 ***                  Confidential - Westwood Studios                    ***
 ***************************************************************************
 *                                                                         *
 *                 Project Name : Voxel Technology                         *
 *                                                                         *
 *                    File Name : QUAT.H                                   *
 *                                                                         *
 *                   Programmer : Greg Hjelstrom                           *
 *                                                                         *
 *                   Start Date : 02/24/97                                 *
 *                                                                         *
 *                  Last Update : February 24, 1997 [GH]                   *
 *                                                                         *
 *-------------------------------------------------------------------------*
 * Functions:                                                              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifndef QUAT_H
#define QUAT_H

#include "always.h"
#include "wwmath.h"
#include "WWmatrix3.h"
#include "vector3.h"


class Quaternion
{
private:

public:

	// X,Y,Z are the imaginary parts of the quaterion
	// W is the real part
	float X;
	float Y;
	float Z;
	float W;

public:

	Quaternion(void) {};
	explicit Quaternion(bool init) { if (init) { X = 0.0f; Y = 0.0f; Z = 0.0f; W = 1.0f; } }
	explicit Quaternion(float a, float b, float c, float d) { X=a; Y=b; Z=c; W=d; }
	explicit Quaternion(const Vector3 & axis,float angle);
	Quaternion & operator=(const Quaternion & source);

	void		Set(float a = 0.0, float b = 0.0, float c = 0.0, float d = 1.0) { X = a; Y = b; Z = c; W = d; }
	void		Make_Identity(void) { Set(); };
	void		Scale(float s) { X = (float)(s*X); Y = (float)(s*Y); Z = (float)(s*Z); W = (float)(s*W); }

	// Array access
	float &	operator [](int i) { return (&X)[i]; }
	const float &  operator [](int i) const { return (&X)[i]; }

	// Unary operators.
	// Remember that q and -q represent the same 3D rotation.
	Quaternion operator-() const { return(Quaternion(-X,-Y,-Z,-W)); }
	Quaternion operator+() const { return *this; }

	// Every 3D rotation can be expressed by two different quaternions,  This
	// function makes the current quaternion convert itself to the representation
	// which is closer on the 4D unit-hypersphere to the given quaternion.
	Quaternion & Make_Closest(const Quaternion & qto);

	// Square of the magnitude of the quaternion
	float Length2(void) const { return (X*X + Y*Y + Z*Z + W*W); }

	// Magnitude of the quaternion
	float Length(void) const { return WWMath::Sqrt(Length2()); }

	// Make the quaternion unit length
	void Normalize(void);

	// post-concatenate rotations about the coordinate axes
	void	Rotate_X(float theta);
	void	Rotate_Y(float theta);
	void 	Rotate_Z(float theta);

	// initialize this quaternion randomly (creates a random *unit* quaternion)
	void	Randomize(void);

	// transform (rotate) a vector with this quaternion
	Vector3	Rotate_Vector(const Vector3 & v) const;
	void		Rotate_Vector(const Vector3 & v,Vector3 * set_result) const;

	// verify that none of the members of this quaternion are invalid floats
	bool		Is_Valid(void) const;
};

// Inverse of the quaternion (1/q)
inline Quaternion Inverse(const Quaternion & a)
{
	return Quaternion(-a[0],-a[1],-a[2],a[3]);
}

// Conjugate of the quaternion
inline Quaternion Conjugate(const Quaternion & a)
{
	return Quaternion(-a[0],-a[1],-a[2],a[3]);
}

// Add two quaternions
inline Quaternion operator + (const Quaternion & a,const Quaternion & b)
{
	return Quaternion(a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]);
}

// Subract two quaternions
inline Quaternion operator - (const Quaternion & a,const Quaternion & b)
{
	return Quaternion(a[0] - b[0], a[1] - b[1], a[2] - b[2], a[3] - b[3]);
}

// Multiply a quaternion by a scalar:
inline Quaternion operator * (float scl, const Quaternion & a)
{
	return Quaternion(scl*a[0], scl*a[1], scl*a[2], scl*a[3]);
}

// Multiply a quaternion by a scalar
inline Quaternion operator * (const Quaternion & a, float scl)
{
	return scl*a;
}

// Multiply two quaternions
inline Quaternion operator * (const Quaternion & a,const Quaternion & b)
{
	return Quaternion
	(
		a.W*b.X + b.W*a.X + (a.Y*b.Z - b.Y*a.Z),
		a.W*b.Y + b.W*a.Y - (a.X*b.Z - b.X*a.Z),
		a.W*b.Z + b.W*a.Z + (a.X*b.Y - b.X*a.Y),
		a.W * b.W - (a.X * b.X + a.Y * b.Y + a.Z * b.Z)
	);
}

// Divide two quaternions
inline Quaternion operator / (const Quaternion & a,const Quaternion & b)
{
	return a * Inverse(b);
}

// Normalized version of the quaternion
inline Quaternion Normalize(const Quaternion & a)
{
	float mag = a.Length();
	if (0.0f == mag) {
		return a;
	} else {
		float oomag = 1.0f / mag;
		return Quaternion(a[0] * oomag, a[1] * oomag, a[2] * oomag, a[3] * oomag);
	}
}

// This function computes a quaternion based on an axis
// (defined by the given Vector a) and an angle about
// which to rotate.  The angle is expressed in radians.
Quaternion Axis_To_Quat(const Vector3 &a, float angle);

// Pass the x and y coordinates of the last and current position
// of the mouse, scaled so they are from -1.0 to 1.0
// The quaternion is the computed as the rotation of a trackball
// between the two points projected onto a sphere.  This can
// be used to implement an intuitive viewing control system.
Quaternion Trackball(float x0, float y0, float x1, float y1, float sphsize);

// Spherical Linear interpolation of quaternions
Quaternion Slerp(const Quaternion & a,const Quaternion & b,float t);

// Convert a rotation matrix into a quaternion
Quaternion Build_Quaternion(const Matrix3 & matrix);
Quaternion Build_Quaternion(const Matrix3D & matrix);
Quaternion Build_Quaternion(const Matrix4 & matrix);

// Convert a quaternion into a rotation matrix
Matrix3	Build_Matrix3(const Quaternion & quat);
Matrix3D Build_Matrix3D(const Quaternion & quat);
Matrix4  Build_Matrix4(const Quaternion & quat);


// Some values can be cached if you are performing multiple slerps
// between the same two quaternions...
struct SlerpInfoStruct
{
	float		SinT;
	float		Theta;
	bool		Flip;
	bool		Linear;
};

// Cached slerp implementation
void Slerp_Setup(const Quaternion & p,const Quaternion & q,SlerpInfoStruct * slerpinfo);
void Cached_Slerp(const Quaternion & p,const Quaternion & q,float alpha,SlerpInfoStruct * slerpinfo,Quaternion * set_q);
Quaternion Cached_Slerp(const Quaternion & p,const Quaternion & q,float alpha,SlerpInfoStruct * slerpinfo);

inline Vector3 Quaternion::Rotate_Vector(const Vector3 & v) const
{
	float x = W*v.X + (Y*v.Z - v.Y*Z);
	float y = W*v.Y - (X*v.Z - v.X*Z);
	float z = W*v.Z + (X*v.Y - v.X*Y);
	float w = -(X*v.X + Y*v.Y + Z*v.Z);

	return Vector3
	(
		w*(-X) + W*x + (y*(-Z) - (-Y)*z),
		w*(-Y) + W*y - (x*(-Z) - (-X)*z),
		w*(-Z) + W*z + (x*(-Y) - (-X)*y)
	);
}

inline void Quaternion::Rotate_Vector(const Vector3 & v,Vector3 * result) const
{
	assert(result != NULL);

	float x = W*v.X + (Y*v.Z - v.Y*Z);
	float y = W*v.Y - (X*v.Z - v.X*Z);
	float z = W*v.Z + (X*v.Y - v.X*Y);
	float w = -(X*v.X + Y*v.Y + Z*v.Z);

	result->X = w*(-X) + W*x + (y*(-Z) - (-Y)*z);
	result->Y = w*(-Y) + W*y - (x*(-Z) - (-X)*z);
	result->Z = w*(-Z) + W*z + (x*(-Y) - (-X)*y);
}

inline bool Quaternion::Is_Valid(void) const
{
	return (	WWMath::Is_Valid_Float(X) &&
				WWMath::Is_Valid_Float(Y) &&
				WWMath::Is_Valid_Float(Z) &&
				WWMath::Is_Valid_Float(W) );
}

#endif /* QUAT_H */



