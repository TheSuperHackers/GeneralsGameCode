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

/* $Header: /Commando/Code/wwmath/matrix3d.h 55    5/11/01 7:11p Jani_p $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Voxel Technology                                             * 
 *                                                                                             * 
 *                    File Name : MATRIX3D.H                                                   * 
 *                                                                                             * 
 *                Org Programmer : Greg Hjelstrom                                               * 
 *                                                                                             * 
 *                   Programmer : Kenny Mitchell                          * 
 *                                                                                             * 
 *                   Start Date : 06/02/97                                                     * 
 *                                                                         * 
 *                  Last Update : June 6, 2002 [KM]                                            * 
 *                                                                                             * 
 * 06/26/02 KM Matrix name change to avoid MAX conflicts                                       *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   Matrix3D::Matrix3D -- Constructors for Matrix3D                                           * 
 *   Matrix3D::Set -- init a Matrix3D from an arrray of 12 floats                              *
 *   Matrix3D::Set -- Init a Matrix3D from 12 individual floats                                *
 *   Matrix3D::Set -- Init a matrix from 3 axis vectors and a position                         *
 *   Matrix3D::Set -- init a matrix to be a rotation about the given axis                      *
 *   Matrix3D::Set -- init a matrix to be a rotation about the given axis                      *
 *   Matrix3D::Set -- Init a matrix to be a pure translation                                   *
 *   Matrix3D::Make_Identity -- Initializes the matrix to be the identity matrix               * 
 *   Matrix3D::Translate -- Post-Multiplies by a Translation Matrix                            * 
 *   Matrix3D::Translate -- Post-Multiplies the matrix by a translation matrix                 * 
 *   M3DC::Translate_X -- Post-Multiplies the matrix by a translation matrix with X only       * 
 *   M3DC::Translate_Y -- Post-Multiplies the matrix by a translation matrix with Y only       * 
 *   M3DC::Translate_Z -- Post-Multiplies the matrix by a translation matrix with Z only       * 
 *   Matrix3D::Rotate_X -- Post-Multiplies the matrix by a rotation about the X axis           * 
 *   Matrix3D::Rotate_X -- Post-Multiplies the matrix by a rotation about the X axis           * 
 *   Matrix3D::Rotate_Y -- Post-multiplies the matrix by a rotation about the Y axis           * 
 *   Matrix3D::Rotate_Y -- Post-Multiplies the matrix by a rotation about Y                    * 
 *   Matrix3D::Rotate_Z -- Post-multiplies the matrix by a rotation about Z                    * 
 *   Matrix3D::Rotate_Z -- Post-multiplies the matrix by a rotation about Z                    * 
 *   Matrix3D::Scale		-- Scales Matrix  !!! WARNING DO NOT USE, INV FUNCTION WILL FAIL       * 
 *   Matrix3D::Scale		-- Scales Matrix  !!! WARNING DO NOT USE, INV FUNCTION WILL FAIL       * 
 *   Matrix3D::Scale		-- Scales Matrix  !!! WARNING DO NOT USE, INV FUNCTION WILL FAIL       * 
 *   M3DC::Pre_Rotate_X -- Pre-multiplies the matrix by a rotation about X                     * 
 *   M3DC::Pre_Rotate_Y -- Pre-multiplies the matrix by a rotation about Y                     * 
 *   M3DC::Pre_Rotate_Z -- Pre-multiplies the matrix by a rotation about Z                     * 
 *   M3DC::Pre_Rotate_X -- Pre-multiplies the matrix by a rotation about X                     * 
 *   M3DC::Pre_Rotate_Y -- Pre-multiplies the matrix by a rotation about Y                     * 
 *   M3DC::Pre_Rotate_Z -- Pre-multiplies the matrix by a rotation about Z                     * 
 *   M3DC::In_Place_Pre_Rotate_X -- Pre-multiplies rotation part of matrix by rotation about X * 
 *   M3DC::In_Place_Pre_Rotate_Y -- Pre-multiplies rotation part of matrix by rotation about Y * 
 *   M3DC::In_Place_Pre_Rotate_Z -- Pre-multiplies rotation part of matrix by rotation about Z * 
 *   M3DC::In_Place_Pre_Rotate_X -- Pre-multiplies rotation part of matrix by rotation about X * 
 *   M3DC::In_Place_Pre_Rotate_Y -- Pre-multiplies rotation part of matrix by rotation about Y * 
 *   M3DC::In_Place_Pre_Rotate_Z -- Pre-multiplies rotation part of matrix by rotation about Z * 
 *   operator * -- Matrix multiplication                                                       * 
 *   operator * -- Matrix - vector multiplication                                              * 
 *   operator == -- Matrix equality operator                                                   * 
 *   operator != -- Matrix inequality operator                                                 * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#define NO_ALLOW_TEMPORARIES

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef MATRIX3D_H
#define MATRIX3D_H

#ifdef _UNIX
#include "osdep.h"
#endif

#include "always.h"
#include <assert.h>
#include "vector2.h"
#include "vector3.h"
#include "vector4.h"
#ifdef _UNIX
#include "osdep.h"
#endif


class Matrix3x3;
class Matrix4x4;
class Quaternion;

/*******************************************************************************

	Matrix3D

	Three important notes:
	- I use *column-vectors*
	- I use a *right-handed* coordinate system
	- These matrices are *orthogonal*
	  
	3D Transformation matrices.  This class is really a 4x4 homogeneous 
	matrix where the last row is assumed to always be 0 0 0 1.  However,
	since I don't store the last row, you cant do some things that you can
	do with a real 4x4 homogeneous matrix.
	
	I use column-vectors so normally transformations are post-multiplied
	and camera transformations should be pre-multiplied.  The methods of
	this class called Translate, Rotate_X, etc. all perform post-multiplication
	with the current matix.  These methods (Translate, Rotate_X, etc) also
	have been hand-coded to only perform the necessary arithmetic.  The
	* operator can be used for general purpose matrix multiplication or to
	transform a vector by a matrix.

	Some operations in this class assume that the matrix is orthogonal.

*********************************************************************************/

class Matrix3D
{
public:

	// Constructors
	WWINLINE Matrix3D(void) {}

	WWINLINE explicit Matrix3D(bool init) { if (init) Make_Identity(); }

	WWINLINE explicit Matrix3D(float m[12]);

	WWINLINE explicit Matrix3D(
		float m11,float m12,float m13,float m14,
		float m21,float m22,float m23,float m24,
		float m31,float m32,float m33,float m34  
	);
	
	WWINLINE explicit Matrix3D(
		const Vector3	&x,		// x-axis unit vector
		const Vector3	&y,		// y-axis unit vector
		const Vector3	&z,		// z-axis unit vector
		const Vector3	&pos		// position
	);

	WWINLINE explicit Matrix3D(
		const Vector3	&axis,
		float angle
	);

	WWINLINE explicit Matrix3D(
		const Vector3	&axis,
		float sine,
		float cosine
	);

	WWINLINE explicit Matrix3D(
		const Matrix3x3 & rotation,
		const Vector3 & position
	);

	WWINLINE explicit Matrix3D(
		const Quaternion & rotation,
		const Vector3 & position
	);

	// Creates identity rotation w. given position.
	WWINLINE explicit Matrix3D(const Vector3 & position);

	// Copy constructor
	WWINLINE Matrix3D(const Matrix3D & m);

	// Assignment operator
	WWINLINE Matrix3D & operator = (const Matrix3D & m);

	// Init functions
	WWINLINE void Set(float m[12]);

	WWINLINE void Set(
		float m11,float m12,float m13,float m14,
		float m21,float m22,float m23,float m24,
		float m31,float m32,float m33,float m34  
	);
	
	WWINLINE void Set(
		const Vector3	&x,		// x-axis unit vector
		const Vector3	&y,		// y-axis unit vector
		const Vector3	&z,		// z-axis unit vector
		const Vector3	&pos		// position
	);

	WWINLINE void Set(const Vector3 & axis,float angle);

	WWINLINE void Set(const Vector3 & axis,float sine,float cosine);

	void Set(const Matrix3x3 & rotation,const Vector3 & position);

	void Set(const Quaternion & rotation,const Vector3 & position);

	// Creates identity rotation w. given position.
	WWINLINE void Set(const Vector3 & position);
	
	// access functions:
 	WWINLINE Vector4 & operator [] (int i) { return Row[i]; }
	WWINLINE const Vector4 & operator [] (int i) const { return Row[i]; }

	WWINLINE Vector3 Get_Translation(void) const { return Vector3(Row[0][3],Row[1][3],Row[2][3]); }
	WWINLINE void Get_Translation(Vector3 * set) const { set->X = Row[0][3]; set->Y = Row[1][3]; set->Z = Row[2][3]; }
	WWINLINE void Set_Translation(const Vector3 & t)  { Row[0][3] = t[0]; Row[1][3] = t[1];Row[2][3] = t[2]; }

	void Set_Rotation(const Matrix3x3 & m);
	void Set_Rotation(const Quaternion & q);

	WWINLINE float Get_X_Translation(void) const { return Row[0][3]; };
	WWINLINE float Get_Y_Translation(void) const { return Row[1][3]; };
	WWINLINE float Get_Z_Translation(void) const { return Row[2][3]; };

	WWINLINE void Set_X_Translation(float x) { Row[0][3] = x; };
	WWINLINE void Set_Y_Translation(float y) { Row[1][3] = y; };
	WWINLINE void Set_Z_Translation(float z) { Row[2][3] = z; };

	WWINLINE void Adjust_Translation(const Vector3 & t) { Row[0][3] += t[0]; Row[1][3] += t[1]; Row[2][3] += t[2]; };
	WWINLINE void Adjust_X_Translation(float x) { Row[0][3] += x; };
	WWINLINE void Adjust_Y_Translation(float y) { Row[1][3] += y; };
	WWINLINE void Adjust_Z_Translation(float z) { Row[2][3] += z; };

	// These functions will give you the approximate amount that the
	// matrix has been rotated about a given axis.  These functions
	// cannot be used to re-build a matrx.  Use the EulerAnglesClass
	// to convert a matrix into a set of three Euler angles.
	float Get_X_Rotation(void) const;
	float Get_Y_Rotation(void) const;
	float Get_Z_Rotation(void) const;
	
	// Each of the transformation methods performs an
	// "optimized" post-multiplication with the current matrix.
	// All angles are assumed to be radians.
	WWINLINE void	Make_Identity(void);
	void	Translate(float x,float y,float z);
	void	Translate(const Vector3 &t);
   void  Translate_X(float x);
   void  Translate_Y(float y);
   void  Translate_Z(float z);
	void	Rotate_X(float theta);
	void	Rotate_Y(float theta);
	void 	Rotate_Z(float theta);
	void	Rotate_X(float s,float c);
	void	Rotate_Y(float s,float c);
	void	Rotate_Z(float s,float c);

	// !!
	// !! Use Scale methods with Extreme Caution
	// !! The Matrix Inverse function, only works
	// !! with Orthogonal Matrices, for optimization purposes
	// !!
	WWINLINE void	Scale(float scale);					 // uniform scale all 3 axis
	WWINLINE void	Scale(float x, float y, float z); // separate input for each axis
	WWINLINE void	Scale(Vector3 &scale);				 // scale each axis


	// Each of these performs an "optimized" pre-multiplication with the
	// current matrix. All angles are assumed to be radians. The "In_Place"
	// versions do not affect the translation part of the matrix,
	WWINLINE void	Pre_Rotate_X(float theta);
	WWINLINE void	Pre_Rotate_Y(float theta);
	WWINLINE void 	Pre_Rotate_Z(float theta);
	WWINLINE void	Pre_Rotate_X(float s,float c);
	WWINLINE void	Pre_Rotate_Y(float s,float c);
	WWINLINE void	Pre_Rotate_Z(float s,float c);
	WWINLINE void	In_Place_Pre_Rotate_X(float theta);
	WWINLINE void	In_Place_Pre_Rotate_Y(float theta);
	WWINLINE void 	In_Place_Pre_Rotate_Z(float theta);
	WWINLINE void	In_Place_Pre_Rotate_X(float s,float c);
	WWINLINE void	In_Place_Pre_Rotate_Y(float s,float c);
	WWINLINE void	In_Place_Pre_Rotate_Z(float s,float c);

	// Points the negative Z axis at the target t.  Assumes that
	// the "world" uses x-y as the ground and z as altitude.
	// Used for pointing cameras at targets.
	void	Look_At(const Vector3 &p,const Vector3 &t,float roll);
	
	// Previous look_at function follows the camera coordinate convention.
	// This one follows the object convention used in Commando and G.  I
	// special cased this convention since it is used so much by us rather
	// than supporting every one of the 24(?) possible conventions...
	void	Obj_Look_At(const Vector3 &p,const Vector3 &t,float roll);

	// Create a matrix given a position and a direction (x axis will point in direction)
	void buildTransformMatrix( const Vector3 &pos, const Vector3 &dir );

	// use the 3x3 sub-matrix only (ignore translation)
	Vector3 Rotate_Vector(const Vector3 &vect) const;
	Vector3 Inverse_Rotate_Vector(const Vector3 &vect) const;

	// these get the a vector representing the direction an axis is pointing
	WWINLINE Vector3 Get_X_Vector() const { return Vector3(Row[0][0], Row[1][0], Row[2][0]); }
	WWINLINE Vector3 Get_Y_Vector() const { return Vector3(Row[0][1], Row[1][1], Row[2][1]); }
	WWINLINE Vector3 Get_Z_Vector() const { return Vector3(Row[0][2], Row[1][2], Row[2][2]); }
	WWINLINE void Get_X_Vector(Vector3 * set_x) const { set_x->Set(Row[0][0], Row[1][0], Row[2][0]); }
	WWINLINE void Get_Y_Vector(Vector3 * set_y) const { set_y->Set(Row[0][1], Row[1][1], Row[2][1]); }
	WWINLINE void Get_Z_Vector(Vector3 * set_z) const { set_z->Set(Row[0][2], Row[1][2], Row[2][2]); }
	
	// Get the inverse of the matrix. 
	// TODO: currently the "intended-to-be" general inverse function just calls
	// the special case Orthogonal inverse functions.  Also, when we implement
	// general case, check where we were using Get_Inverse since usually it should
	// be changed to Get_Orthogonal_Inverse...
	void Get_Inverse(Matrix3D & set_inverse) const;
	void Get_Orthogonal_Inverse(Matrix3D & set_inverse) const;
	
	// used for importing SurRender matrices
	void Copy_3x3_Matrix(float matrix[3][3]);

	// Optimized Axis-Aligned Box transforms.  One for each of the common forms of
	// axis aligned box: min,max vectors and center,extent vectors.
	void Transform_Min_Max_AABox(const Vector3 & min,const Vector3 & max,Vector3 * set_min,Vector3 * set_max) const;
	void Transform_Center_Extent_AABox(const Vector3 & center,const Vector3 & extent,Vector3 * set_center,Vector3 * set_extent) const;

	// matrix multiplication without temporaries
	static void	Multiply(const Matrix3D &A,const Matrix3D &B,Matrix3D * set_result);
	static WWINLINE void	Transform_Vector(const Matrix3D & tm,const Vector3 & in,Vector3 * out);
	static WWINLINE void	Rotate_Vector(const Matrix3D & tm,const Vector3 & in,Vector3 * out);

	// transform a vector by the inverse of this matrix (note: assumes the matrix
	// is orthogonal; if you've manually scaled or sheared the matrix this function
	// will not give correct results)
	static WWINLINE void	Inverse_Transform_Vector(const Matrix3D & tm,const Vector3 & in,Vector3 * out);
	static WWINLINE void	Inverse_Rotate_Vector(const Matrix3D & tm,const Vector3 & in,Vector3 * out);

	// Solve a linear system of 3 equations and 3 unknowns.  The result of this function is
	// that the 3x3 portion of the matrix is inverted and the final column is your answer...
	static bool Solve_Linear_System(Matrix3D & system);

	// Check whether a matrix is orthogonal or FORCE it to be :-)
	int	Is_Orthogonal(void) const; 
	void	Re_Orthogonalize(void);

	static void Lerp(const Matrix3D &A, const Matrix3D &B, float factor, Matrix3D& result);
	
#ifdef ALLOW_TEMPORARIES
	// nothing
#else

	// does "this = that * this"
	void preMul(const Matrix3D& that);

	// does "this = this * that"
	void postMul(const Matrix3D& that);

	// does "this = a * b"
	void mul(const Matrix3D& a, const Matrix3D& b);

	void mulVector3(const Vector3& in, Vector3& out) const;
	void mulVector3(Vector3& inout) const { mulVector3(inout, inout); }

#endif

	void mulVector3Array(const Vector3* in, Vector3* out, int count) const;
	void mulVector3Array(Vector3* inout, int count) const;

	// some static matrices which are sometimes useful
	static const Matrix3D		Identity;
	static const Matrix3D		RotateX90;
	static const Matrix3D		RotateX180;
	static const Matrix3D		RotateX270;
	static const Matrix3D		RotateY90;
	static const Matrix3D		RotateY180;
	static const Matrix3D		RotateY270;
	static const Matrix3D		RotateZ90;
	static const Matrix3D		RotateZ180;
	static const Matrix3D		RotateZ270;

protected:

	Vector4			Row[3];

#ifdef ALLOW_TEMPORARIES
	WWINLINE friend Vector3 operator * (const Matrix3D &A,const Vector3 &a);
#endif
};



/* ---------------------------------------------------------------
	Vector Transformation, Matrix concatenation
--------------------------------------------------------------- */
#ifdef ALLOW_TEMPORARIES
Vector3  operator * (const Matrix3D &A,const Vector3 &v);
Matrix3D operator * (const Matrix3D &A,const Matrix3D &B);
#endif

/* ---------------------------------------------------------------
   Equality and inequality operators
--------------------------------------------------------------- */
bool operator == (const Matrix3D &A, const Matrix3D &B);
bool operator != (const Matrix3D &A, const Matrix3D &B);


/*********************************************************************************************** 
 * M3DC::Matrix3D -- Constructors for Matrix3D										                    * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/24/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
WWINLINE Matrix3D::Matrix3D(float m[12])
{
	Row[0].Set(m[0],m[1],m[2],m[3]);
	Row[1].Set(m[4],m[5],m[6],m[7]);
	Row[2].Set(m[8],m[9],m[10],m[11]);
}

WWINLINE Matrix3D::Matrix3D
(
	float m11,float m12,float m13,float m14,
	float m21,float m22,float m23,float m24,
	float m31,float m32,float m33,float m34  
)
{
	Row[0].Set(m11,m12,m13,m14);
	Row[1].Set(m21,m22,m23,m24);
	Row[2].Set(m31,m32,m33,m34);
}

WWINLINE Matrix3D::Matrix3D
(
	const Vector3	&x,		// x-axis unit vector
	const Vector3	&y,		// y-axis unit vector
	const Vector3	&z,		// z-axis unit vector
	const Vector3	&pos		// position
)
{
	Set(x,y,z,pos);
}

WWINLINE Matrix3D::Matrix3D(const Vector3 & axis,float angle)
{
	Set(axis,angle);
}

WWINLINE Matrix3D::Matrix3D(const Vector3 & axis,float sine,float cosine)
{
	Set(axis,sine,cosine);
}

WWINLINE Matrix3D::Matrix3D(const Matrix3x3 & rot,const Vector3 & pos)
{
	Set(rot,pos);
}

WWINLINE Matrix3D::Matrix3D(const Quaternion & rot,const Vector3 & pos)
{
	Set(rot,pos);
}

WWINLINE Matrix3D::Matrix3D(const Vector3 & position)
{
	Set(position);
}

// Copy Constructor
WWINLINE Matrix3D::Matrix3D(const Matrix3D & m)
{
	Row[0] = m.Row[0];
	Row[1] = m.Row[1];
	Row[2] = m.Row[2];
}

// Assignment operator
WWINLINE Matrix3D & Matrix3D::operator = (const Matrix3D & m)
{
	Row[0] = m.Row[0];
	Row[1] = m.Row[1];
	Row[2] = m.Row[2];
	return *this; 
}


/***********************************************************************************************
 * Matrix3D::Set -- init a Matrix3D from an arrray of 12 floats                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void Matrix3D::Set(float m[12])
{
	Row[0].Set(m[0],m[1],m[2],m[3]);
	Row[1].Set(m[4],m[5],m[6],m[7]);
	Row[2].Set(m[8],m[9],m[10],m[11]);
}


/***********************************************************************************************
 * Matrix3D::Set -- Init a Matrix3D from 12 individual floats                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void Matrix3D::Set(		float m11,float m12,float m13,float m14,
										float m21,float m22,float m23,float m24,
										float m31,float m32,float m33,float m34)
{
	Row[0].Set(m11,m12,m13,m14);
	Row[1].Set(m21,m22,m23,m24);
	Row[2].Set(m31,m32,m33,m34);
}


/***********************************************************************************************
 * Matrix3D::Set -- Init a matrix from 3 axis vectors and a position                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void Matrix3D::Set(		const Vector3	&x,		// x-axis unit vector
										const Vector3	&y,		// y-axis unit vector
										const Vector3	&z,		// z-axis unit vector
										const Vector3	&pos)		// position
{
	Row[0].Set(x[0],y[0],z[0],pos[0]);
	Row[1].Set(x[1],y[1],z[1],pos[1]);
	Row[2].Set(x[2],y[2],z[2],pos[2]);
}


/***********************************************************************************************
 * Matrix3D::Set -- init a matrix to be a rotation about the given axis                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void Matrix3D::Set(const Vector3 & axis,float angle)
{
	float c = cosf(angle);
	float s = sinf(angle);

	Set(axis,s,c);
}

/***********************************************************************************************
 * Matrix3D::Set -- init a matrix to be a rotation about the given axis                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void Matrix3D::Set(const Vector3 & axis,float s,float c)
{
	assert(WWMath::Fabs(axis.Length2() - 1.0f) < 0.001f);

	Row[0].Set(
		(float)(axis[0]*axis[0] + c*(1.0f - axis[0]*axis[0])),
		(float)(axis[0]*axis[1]*(1.0f - c) - axis[2]*s),
		(float)(axis[2]*axis[0]*(1.0f - c) + axis[1]*s),
		0.0f
	);

	Row[1].Set(
		(float)(axis[0]*axis[1]*(1.0f - c) + axis[2]*s),
		(float)(axis[1]*axis[1] + c*(1.0f - axis[1]*axis[1])),
		(float)(axis[1]*axis[2]*(1.0f - c) - axis[0]*s),
		0.0f
	);

	Row[2].Set(
		(float)(axis[2]*axis[0]*(1.0f - c) - axis[1]*s),
		(float)(axis[1]*axis[2]*(1.0f - c) + axis[0]*s),
		(float)(axis[2]*axis[2] + c*(1 - axis[2]*axis[2])),
		0.0f
	);
}


/***********************************************************************************************
 * Matrix3D::Set -- Init a matrix to be a pure translation                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void Matrix3D::Set(const Vector3 & position)
{
	Row[0].Set(1.0f,0.0f,0.0f,position[0]);
	Row[1].Set(0.0f,1.0f,0.0f,position[1]);
	Row[2].Set(0.0f,0.0f,1.0f,position[2]);
}
	

/*********************************************************************************************** 
 * M3DC::Make_Identity -- Initializes the matrix to be the identity matrix                     * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/24/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
WWINLINE void Matrix3D::Make_Identity(void)
{
	Row[0].Set(1.0f,0.0f,0.0f,0.0f);
	Row[1].Set(0.0f,1.0f,0.0f,0.0f);
	Row[2].Set(0.0f,0.0f,1.0f,0.0f);
}


/*********************************************************************************************** 
 * M3DC::Translate -- Post-Multiplies by a Translation Matrix                                  * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/24/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
WWINLINE void Matrix3D::Translate(float x,float y,float z)
{
	Row[0][3]  += (float)(Row[0][0]*x + Row[0][1]*y + Row[0][2]*z);
	Row[1][3]  += (float)(Row[1][0]*x + Row[1][1]*y + Row[1][2]*z);
	Row[2][3]  += (float)(Row[2][0]*x + Row[2][1]*y + Row[2][2]*z);
}


/*********************************************************************************************** 
 * M3DC::Translate -- Post-Multiplies the matrix by a translation matrix                       * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/24/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
WWINLINE void Matrix3D::Translate(const Vector3 &t)
{
	Row[0][3]  += Row[0][0]*t[0] + Row[0][1]*t[1] + Row[0][2]*t[2];
	Row[1][3]  += Row[1][0]*t[0] + Row[1][1]*t[1] + Row[1][2]*t[2];
	Row[2][3]  += Row[2][0]*t[0] + Row[2][1]*t[1] + Row[2][2]*t[2];
}


/*********************************************************************************************** 
 * M3DC::Translate_X -- Post-Multiplies the matrix by a translation matrix with X only         * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/06/1998 NH  : Created.                                                                 * 
 *=============================================================================================*/
WWINLINE void Matrix3D::Translate_X(float x)
{
	Row[0][3]  += (float)(Row[0][0]*x);
	Row[1][3]  += (float)(Row[1][0]*x);
	Row[2][3]  += (float)(Row[2][0]*x);
}


/*********************************************************************************************** 
 * M3DC::Translate_Y -- Post-Multiplies the matrix by a translation matrix with Y only         * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/06/1998 NH  : Created.                                                                 * 
 *=============================================================================================*/
WWINLINE void Matrix3D::Translate_Y(float y)
{
	Row[0][3]  += (float)(Row[0][1]*y);
	Row[1][3]  += (float)(Row[1][1]*y);
	Row[2][3]  += (float)(Row[2][1]*y);
}


/*********************************************************************************************** 
 * M3DC::Translate_Z -- Post-Multiplies the matrix by a translation matrix with Z only         * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/06/1998 NH  : Created.                                                                 * 
 *=============================================================================================*/
WWINLINE void Matrix3D::Translate_Z(float z)
{
	Row[0][3]  += (float)(Row[0][2]*z);
	Row[1][3]  += (float)(Row[1][2]*z);
	Row[2][3]  += (float)(Row[2][2]*z);
}


/*********************************************************************************************** 
 * M3DC::Rotate_X -- Post-Multiplies the matrix by a rotation about the X axis                 * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/24/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
WWINLINE void Matrix3D::Rotate_X(float theta)
{
	float tmp1,tmp2;
	float s,c;

	s = sinf(theta);
	c = cosf(theta);

	tmp1 = Row[0][1]; tmp2 = Row[0][2];
	Row[0][1] = (float)( c*tmp1 + s*tmp2);
	Row[0][2] = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[1][1]; tmp2 = Row[1][2];
	Row[1][1] = (float)( c*tmp1 + s*tmp2);
	Row[1][2] = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[2][1]; tmp2 = Row[2][2];
	Row[2][1] = (float)( c*tmp1 + s*tmp2);
	Row[2][2] = (float)(-s*tmp1 + c*tmp2);

}


/*********************************************************************************************** 
 * M3DC::Rotate_X -- Post-Multiplies the matrix by a rotation about the X axis                 * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 * s - sine of the angle                                                                       * 
 * c - cosine of the angle                                                                     * 
 * 											                                                           * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/24/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
WWINLINE void Matrix3D::Rotate_X(float s,float c)
{
	float tmp1,tmp2;

	tmp1 = Row[0][1]; tmp2 = Row[0][2];
	Row[0][1] = (float)( c*tmp1 + s*tmp2);
	Row[0][2] = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[1][1]; tmp2 = Row[1][2];
	Row[1][1] = (float)( c*tmp1 + s*tmp2);
	Row[1][2] = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[2][1]; tmp2 = Row[2][2];
	Row[2][1] = (float)( c*tmp1 + s*tmp2);
	Row[2][2] = (float)(-s*tmp1 + c*tmp2);
}


/*********************************************************************************************** 
 * M3DC::Rotate_Y -- Post-multiplies the matrix by a rotation about the Y axis                 * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 * theta - angle (in radians) to rotate                                                        * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/24/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
WWINLINE void Matrix3D::Rotate_Y(float theta)
{
	float tmp1,tmp2;
	float s,c;

	s = sinf(theta);
	c = cosf(theta);

	tmp1 = Row[0][0]; tmp2 = Row[0][2];
	Row[0][0] = (float)(c*tmp1 - s*tmp2);
	Row[0][2] = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[1][0]; tmp2 = Row[1][2];
	Row[1][0] = (float)(c*tmp1 - s*tmp2);
	Row[1][2] = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[2][0]; tmp2 = Row[2][2];
	Row[2][0] = (float)(c*tmp1 - s*tmp2);
	Row[2][2] = (float)(s*tmp1 + c*tmp2);
}


/*********************************************************************************************** 
 * M3DC::Rotate_Y -- Post-Multiplies the matrix by a rotation about Y                          * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 * s - sine of the angle                                                                       * 
 * c - cosine of the angle                                                                     * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/24/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
WWINLINE void Matrix3D::Rotate_Y(float s,float c)
{
	float tmp1,tmp2;

	tmp1 = Row[0][0]; tmp2 = Row[0][2];
	Row[0][0] = (float)(c*tmp1 - s*tmp2);
	Row[0][2] = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[1][0]; tmp2 = Row[1][2];
	Row[1][0] = (float)(c*tmp1 - s*tmp2);
	Row[1][2] = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[2][0]; tmp2 = Row[2][2];
	Row[2][0] = (float)(c*tmp1 - s*tmp2);
	Row[2][2] = (float)(s*tmp1 + c*tmp2);
}


/*********************************************************************************************** 
 * M3DC::Rotate_Z -- Post-multiplies the matrix by a rotation about Z                          * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 * theta - angle (in radians) to rotate                                                        * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/24/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
WWINLINE void Matrix3D::Rotate_Z(float theta)
{
	float tmp1,tmp2;
	float c,s;

	c = cosf(theta);
	s = sinf(theta);

	tmp1 = Row[0][0]; tmp2 = Row[0][1];
	Row[0][0] = (float)( c*tmp1 + s*tmp2);
	Row[0][1] = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[1][0]; tmp2 = Row[1][1];
	Row[1][0] = (float)( c*tmp1 + s*tmp2);
	Row[1][1] = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[2][0]; tmp2 = Row[2][1];
	Row[2][0] = (float)( c*tmp1 + s*tmp2);
	Row[2][1] = (float)(-s*tmp1 + c*tmp2);
}


/*********************************************************************************************** 
 * M3DC::Rotate_Z -- Post-multiplies the matrix by a rotation about Z                          * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 * s - sine of the angle to rotate                                                             * 
 * c - cosine of the angle to rotate                                                           * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/24/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
WWINLINE void Matrix3D::Rotate_Z(float s,float c)
{
	float tmp1,tmp2;

	tmp1 = Row[0][0]; tmp2 = Row[0][1];
	Row[0][0] = (float)( c*tmp1 + s*tmp2);
	Row[0][1] = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[1][0]; tmp2 = Row[1][1];
	Row[1][0] = (float)( c*tmp1 + s*tmp2);
	Row[1][1] = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[2][0]; tmp2 = Row[2][1];
	Row[2][0] = (float)( c*tmp1 + s*tmp2);
	Row[2][1] = (float)(-s*tmp1 + c*tmp2);
}

/*********************************************************************************************** 
 * Matrix3D::Scale -- Scales each Axis											                          * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   03/07/2000 jga  : Created.                                                                * 
 *=============================================================================================*/
// !!
// !! Use Scale methods with Extreme Caution
// !! The Matrix Inverse function, only works
// !! with Orthogonal Matrices, for optimization purposes
// !!
WWINLINE void	Matrix3D::Scale(float scale)
{	// uniform scale all 3 axis
	// X
	Row[0][0] *= scale;
	Row[1][0] *= scale;
	Row[2][0] *= scale;
	// Y
	Row[0][1] *= scale;
	Row[1][1] *= scale;
	Row[2][1] *= scale;
	// Z
	Row[0][2] *= scale;
	Row[1][2] *= scale;
	Row[2][2] *= scale;
}
/*********************************************************************************************** 
 * Matrix3D::Scale -- Scales each Axis											                          * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   03/07/2000 jga  : Created.                                                                * 
 *=============================================================================================*/
// !!
// !! Use Scale methods with Extreme Caution
// !! The Matrix Inverse function, only works
// !! with Orthogonal Matrices, for optimization purposes
// !!
WWINLINE void	Matrix3D::Scale(float x, float y, float z)
{ // separate input for each axis
	// X
	Row[0][0] *= x;
	Row[1][0] *= x;
	Row[2][0] *= x;
	// Y
	Row[0][1] *= y;
	Row[1][1] *= y;
	Row[2][1] *= y;
	// Z
	Row[0][2] *= z;
	Row[1][2] *= z;
	Row[2][2] *= z;
}
/*********************************************************************************************** 
 * Matrix3D::Scale -- Scales each Axis											                          * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   03/07/2000 jga  : Created.                                                                * 
 *=============================================================================================*/
// !!
// !! Use Scale methods with Extreme Caution
// !! The Matrix Inverse function, only works
// !! with Orthogonal Matrices, for optimization purposes
// !!
WWINLINE void	Matrix3D::Scale(Vector3 &scale)
{ // scale each axis
	Scale(scale.X, scale.Y, scale.Z);
}

/*********************************************************************************************** 
 * M3DC::Pre_Rotate_X -- Pre-multiplies the matrix by a rotation about X                       * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 * theta - angle (in radians) to rotate                                                        * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/1/1999 NH  : Created.                                                                  * 
 *=============================================================================================*/
WWINLINE void Matrix3D::Pre_Rotate_X(float theta)
{
	float tmp1,tmp2;
	float c,s;

	c = cosf(theta);
	s = sinf(theta);

	tmp1 = Row[1][0]; tmp2 = Row[2][0];
	Row[1][0] = (float)(c*tmp1 - s*tmp2);
	Row[2][0] = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[1][1]; tmp2 = Row[2][1];
	Row[1][1] = (float)(c*tmp1 - s*tmp2);
	Row[2][1] = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[1][2]; tmp2 = Row[2][2];
	Row[1][2] = (float)(c*tmp1 - s*tmp2);
	Row[2][2] = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[1][3]; tmp2 = Row[2][3];
	Row[1][3] = (float)(c*tmp1 - s*tmp2);
	Row[2][3] = (float)(s*tmp1 + c*tmp2);
}

/*********************************************************************************************** 
 * M3DC::Pre_Rotate_Y -- Pre-multiplies the matrix by a rotation about Y                       * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 * theta - angle (in radians) to rotate                                                        * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/1/1999 NH  : Created.                                                                  * 
 *=============================================================================================*/
WWINLINE void Matrix3D::Pre_Rotate_Y(float theta)
{
	float tmp1,tmp2;
	float c,s;

	c = cosf(theta);
	s = sinf(theta);

	tmp1 = Row[0][0]; tmp2 = Row[2][0];
	Row[0][0] = (float)( c*tmp1 + s*tmp2);
	Row[2][0] = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[0][1]; tmp2 = Row[2][1];
	Row[0][1] = (float)( c*tmp1 + s*tmp2);
	Row[2][1] = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[0][2]; tmp2 = Row[2][2];
	Row[0][2] = (float)( c*tmp1 + s*tmp2);
	Row[2][2] = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[0][3]; tmp2 = Row[2][3];
	Row[0][3] = (float)( c*tmp1 + s*tmp2);
	Row[2][3] = (float)(-s*tmp1 + c*tmp2);
}

/*********************************************************************************************** 
 * M3DC::Pre_Rotate_Z -- Pre-multiplies the matrix by a rotation about Z                       * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 * theta - angle (in radians) to rotate                                                        * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/1/1999 NH  : Created.                                                                  * 
 *=============================================================================================*/
WWINLINE void Matrix3D::Pre_Rotate_Z(float theta)
{
	float tmp1,tmp2;
	float c,s;

	c = cosf(theta);
	s = sinf(theta);

	tmp1 = Row[0][0]; tmp2 = Row[1][0];
	Row[0][0] = (float)(c*tmp1 - s*tmp2);
	Row[1][0] = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[0][1]; tmp2 = Row[1][1];
	Row[0][1] = (float)(c*tmp1 - s*tmp2);
	Row[1][1] = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[0][2]; tmp2 = Row[1][2];
	Row[0][2] = (float)(c*tmp1 - s*tmp2);
	Row[1][2] = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[0][3]; tmp2 = Row[1][3];
	Row[0][3] = (float)(c*tmp1 - s*tmp2);
	Row[1][3] = (float)(s*tmp1 + c*tmp2);
}

/*********************************************************************************************** 
 * M3DC::Pre_Rotate_X -- Pre-multiplies the matrix by a rotation about X                       * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 * s - sine of the angle to rotate                                                             * 
 * c - cosine of the angle to rotate                                                           * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/1/1999 NH  : Created.                                                                  * 
 *=============================================================================================*/
WWINLINE void Matrix3D::Pre_Rotate_X(float s,float c)
{
	float tmp1,tmp2;

	tmp1 = Row[1][0]; tmp2 = Row[2][0];
	Row[1][0] = (float)(c*tmp1 - s*tmp2);
	Row[2][0] = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[1][1]; tmp2 = Row[2][1];
	Row[1][1] = (float)(c*tmp1 - s*tmp2);
	Row[2][1] = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[1][2]; tmp2 = Row[2][2];
	Row[1][2] = (float)(c*tmp1 - s*tmp2);
	Row[2][2] = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[1][3]; tmp2 = Row[2][3];
	Row[1][3] = (float)(c*tmp1 - s*tmp2);
	Row[2][3] = (float)(s*tmp1 + c*tmp2);
}

/*********************************************************************************************** 
 * M3DC::Pre_Rotate_Y -- Pre-multiplies the matrix by a rotation about Y                       * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 * s - sine of the angle to rotate                                                             * 
 * c - cosine of the angle to rotate                                                           * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/1/1999 NH  : Created.                                                                  * 
 *=============================================================================================*/
WWINLINE void Matrix3D::Pre_Rotate_Y(float s,float c)
{
	float tmp1,tmp2;

	tmp1 = Row[0][0]; tmp2 = Row[2][0];
	Row[0][0] = (float)( c*tmp1 + s*tmp2);
	Row[2][0] = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[0][1]; tmp2 = Row[2][1];
	Row[0][1] = (float)( c*tmp1 + s*tmp2);
	Row[2][1] = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[0][2]; tmp2 = Row[2][2];
	Row[0][2] = (float)( c*tmp1 + s*tmp2);
	Row[2][2] = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[0][3]; tmp2 = Row[2][3];
	Row[0][3] = (float)( c*tmp1 + s*tmp2);
	Row[2][3] = (float)(-s*tmp1 + c*tmp2);
}

/*********************************************************************************************** 
 * M3DC::Pre_Rotate_Z -- Pre-multiplies the matrix by a rotation about Z                       * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 * s - sine of the angle to rotate                                                             * 
 * c - cosine of the angle to rotate                                                           * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/1/1999 NH  : Created.                                                                  * 
 *=============================================================================================*/
WWINLINE void Matrix3D::Pre_Rotate_Z(float s,float c)
{
	float tmp1,tmp2;

	tmp1 = Row[0][0]; tmp2 = Row[1][0];
	Row[0][0] = (float)(c*tmp1 - s*tmp2);
	Row[1][0] = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[0][1]; tmp2 = Row[1][1];
	Row[0][1] = (float)(c*tmp1 - s*tmp2);
	Row[1][1] = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[0][2]; tmp2 = Row[1][2];
	Row[0][2] = (float)(c*tmp1 - s*tmp2);
	Row[1][2] = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[0][3]; tmp2 = Row[1][3];
	Row[0][3] = (float)(c*tmp1 - s*tmp2);
	Row[1][3] = (float)(s*tmp1 + c*tmp2);
}

/*********************************************************************************************** 
 * M3DC::In_Place_Pre_Rotate_X -- Pre-multiplies rotation part of matrix by a rotation about X * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 * theta - angle (in radians) to rotate                                                        * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/1/1999 NH  : Created.                                                                  * 
 *=============================================================================================*/
WWINLINE void Matrix3D::In_Place_Pre_Rotate_X(float theta)
{
	float tmp1,tmp2;
	float c,s;

	c = cosf(theta);
	s = sinf(theta);

	tmp1 = Row[1][0]; tmp2 = Row[2][0];
	Row[1][0] = (float)(c*tmp1 - s*tmp2);
	Row[2][0] = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[1][1]; tmp2 = Row[2][1];
	Row[1][1] = (float)(c*tmp1 - s*tmp2);
	Row[2][1] = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[1][2]; tmp2 = Row[2][2];
	Row[1][2] = (float)(c*tmp1 - s*tmp2);
	Row[2][2] = (float)(s*tmp1 + c*tmp2);
}

/*********************************************************************************************** 
 * M3DC::In_Place_Pre_Rotate_Y -- Pre-multiplies rotation part of matrix by a rotation about Y * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 * theta - angle (in radians) to rotate                                                        * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/1/1999 NH  : Created.                                                                  * 
 *=============================================================================================*/
WWINLINE void Matrix3D::In_Place_Pre_Rotate_Y(float theta)
{
	float tmp1,tmp2;
	float c,s;

	c = cosf(theta);
	s = sinf(theta);

	tmp1 = Row[0][0]; tmp2 = Row[2][0];
	Row[0][0] = (float)( c*tmp1 + s*tmp2);
	Row[2][0] = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[0][1]; tmp2 = Row[2][1];
	Row[0][1] = (float)( c*tmp1 + s*tmp2);
	Row[2][1] = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[0][2]; tmp2 = Row[2][2];
	Row[0][2] = (float)( c*tmp1 + s*tmp2);
	Row[2][2] = (float)(-s*tmp1 + c*tmp2);
}

/*********************************************************************************************** 
 * M3DC::In_Place_Pre_Rotate_Z -- Pre-multiplies rotation part of matrix by a rotation about Z * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 * theta - angle (in radians) to rotate                                                        * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/1/1999 NH  : Created.                                                                  * 
 *=============================================================================================*/
WWINLINE void Matrix3D::In_Place_Pre_Rotate_Z(float theta)
{
	float tmp1,tmp2;
	float c,s;

	c = cosf(theta);
	s = sinf(theta);

	tmp1 = Row[0][0]; tmp2 = Row[1][0];
	Row[0][0] = (float)(c*tmp1 - s*tmp2);
	Row[1][0] = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[0][1]; tmp2 = Row[1][1];
	Row[0][1] = (float)(c*tmp1 - s*tmp2);
	Row[1][1] = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[0][2]; tmp2 = Row[1][2];
	Row[0][2] = (float)(c*tmp1 - s*tmp2);
	Row[1][2] = (float)(s*tmp1 + c*tmp2);
}

/*********************************************************************************************** 
 * M3DC::In_Place_Pre_Rotate_X -- Pre-multiplies rotation part of matrix by a rotation about X * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 * s - sine of the angle to rotate                                                             * 
 * c - cosine of the angle to rotate                                                           * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/1/1999 NH  : Created.                                                                  * 
 *=============================================================================================*/
WWINLINE void Matrix3D::In_Place_Pre_Rotate_X(float s,float c)
{
	float tmp1,tmp2;

	tmp1 = Row[1][0]; tmp2 = Row[2][0];
	Row[1][0] = (float)(c*tmp1 - s*tmp2);
	Row[2][0] = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[1][1]; tmp2 = Row[2][1];
	Row[1][1] = (float)(c*tmp1 - s*tmp2);
	Row[2][1] = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[1][2]; tmp2 = Row[2][2];
	Row[1][2] = (float)(c*tmp1 - s*tmp2);
	Row[2][2] = (float)(s*tmp1 + c*tmp2);
}

/*********************************************************************************************** 
 * M3DC::In_Place_Pre_Rotate_Y -- Pre-multiplies rotation part of matrix by a rotation about Y * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 * s - sine of the angle to rotate                                                             * 
 * c - cosine of the angle to rotate                                                           * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/1/1999 NH  : Created.                                                                  * 
 *=============================================================================================*/
WWINLINE void Matrix3D::In_Place_Pre_Rotate_Y(float s,float c)
{
	float tmp1,tmp2;

	tmp1 = Row[0][0]; tmp2 = Row[2][0];
	Row[0][0] = (float)( c*tmp1 + s*tmp2);
	Row[2][0] = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[0][1]; tmp2 = Row[2][1];
	Row[0][1] = (float)( c*tmp1 + s*tmp2);
	Row[2][1] = (float)(-s*tmp1 + c*tmp2);

	tmp1 = Row[0][2]; tmp2 = Row[2][2];
	Row[0][2] = (float)( c*tmp1 + s*tmp2);
	Row[2][2] = (float)(-s*tmp1 + c*tmp2);
}

/*********************************************************************************************** 
 * M3DC::In_Place_Pre_Rotate_Z -- Pre-multiplies rotation part of matrix by a rotation about Z * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 * s - sine of the angle to rotate                                                             * 
 * c - cosine of the angle to rotate                                                           * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   07/1/1999 NH  : Created.                                                                  * 
 *=============================================================================================*/
WWINLINE void Matrix3D::In_Place_Pre_Rotate_Z(float s,float c)
{
	float tmp1,tmp2;

	tmp1 = Row[0][0]; tmp2 = Row[1][0];
	Row[0][0] = (float)(c*tmp1 - s*tmp2);
	Row[1][0] = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[0][1]; tmp2 = Row[1][1];
	Row[0][1] = (float)(c*tmp1 - s*tmp2);
	Row[1][1] = (float)(s*tmp1 + c*tmp2);

	tmp1 = Row[0][2]; tmp2 = Row[1][2];
	Row[0][2] = (float)(c*tmp1 - s*tmp2);
	Row[1][2] = (float)(s*tmp1 + c*tmp2);
}

#ifdef ALLOW_TEMPORARIES
/*********************************************************************************************** 
 * operator * -- Matrix multiplication                                                         * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/24/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
WWINLINE Matrix3D operator * (const Matrix3D &A,const Matrix3D &B)
{
	Matrix3D C;
	float tmp1,tmp2,tmp3;

	tmp1 = B[0][0];
	tmp2 = B[1][0];
	tmp3 = B[2][0];

	C[0][0] = (float)(A[0][0]*tmp1 + A[0][1]*tmp2 + A[0][2]*tmp3);
	C[1][0] = (float)(A[1][0]*tmp1 + A[1][1]*tmp2 + A[1][2]*tmp3);
	C[2][0] = (float)(A[2][0]*tmp1 + A[2][1]*tmp2 + A[2][2]*tmp3);

	tmp1 = B[0][1];
	tmp2 = B[1][1];
	tmp3 = B[2][1];

	C[0][1] = (float)(A[0][0]*tmp1 + A[0][1]*tmp2 + A[0][2]*tmp3);
	C[1][1] = (float)(A[1][0]*tmp1 + A[1][1]*tmp2 + A[1][2]*tmp3);
	C[2][1] = (float)(A[2][0]*tmp1 + A[2][1]*tmp2 + A[2][2]*tmp3);

	tmp1 = B[0][2];
	tmp2 = B[1][2];
	tmp3 = B[2][2];

	C[0][2] = (float)(A[0][0]*tmp1 + A[0][1]*tmp2 + A[0][2]*tmp3);
	C[1][2] = (float)(A[1][0]*tmp1 + A[1][1]*tmp2 + A[1][2]*tmp3);
	C[2][2] = (float)(A[2][0]*tmp1 + A[2][1]*tmp2 + A[2][2]*tmp3);

	tmp1 = B[0][3];
	tmp2 = B[1][3];
	tmp3 = B[2][3];

	C[0][3] = (float)(A[0][0]*tmp1 + A[0][1]*tmp2 + A[0][2]*tmp3 + A[0][3]);
	C[1][3] = (float)(A[1][0]*tmp1 + A[1][1]*tmp2 + A[1][2]*tmp3 + A[1][3]);
	C[2][3] = (float)(A[2][0]*tmp1 + A[2][1]*tmp2 + A[2][2]*tmp3 + A[2][3]);

	return C;
}

#else

WWINLINE float submul(const Vector4& row, float tmp1, float tmp2, float tmp3)
{
  return row.X * tmp1 + row.Y * tmp2 + row.Z * tmp3;
}

// does "this = that * this"
WWINLINE void Matrix3D::preMul(const Matrix3D& that)
{
	assert(this != &that);

	// yes, this is ok. see the comments for 'mul'
	this->mul(that, *this);
}

// does "this = this * that"
WWINLINE void Matrix3D::postMul(const Matrix3D& that)
{
	assert(this != &that);

#define AVOID_TEMP_IN_POSTMUL
#ifdef AVOID_TEMP_IN_POSTMUL
  float tmpX, tmpY, tmpZ, tmpW;

	tmpX = submul(this->Row[0], that.Row[0].X, that.Row[1].X, that.Row[2].X);
	tmpY = submul(this->Row[0], that.Row[0].Y, that.Row[1].Y, that.Row[2].Y);
	tmpZ = submul(this->Row[0], that.Row[0].Z, that.Row[1].Z, that.Row[2].Z);
	tmpW = submul(this->Row[0], that.Row[0].W, that.Row[1].W, that.Row[2].W);

	this->Row[0].X = tmpX;
	this->Row[0].Y = tmpY;
	this->Row[0].Z = tmpZ;
	this->Row[0].W += tmpW;

	tmpX = submul(this->Row[1], that.Row[0].X, that.Row[1].X, that.Row[2].X);
	tmpY = submul(this->Row[1], that.Row[0].Y, that.Row[1].Y, that.Row[2].Y);
	tmpZ = submul(this->Row[1], that.Row[0].Z, that.Row[1].Z, that.Row[2].Z);
	tmpW = submul(this->Row[1], that.Row[0].W, that.Row[1].W, that.Row[2].W);

	this->Row[1].X = tmpX;
	this->Row[1].Y = tmpY;
	this->Row[1].Z = tmpZ;
	this->Row[1].W += tmpW;

	tmpX = submul(this->Row[2], that.Row[0].X, that.Row[1].X, that.Row[2].X);
	tmpY = submul(this->Row[2], that.Row[0].Y, that.Row[1].Y, that.Row[2].Y);
	tmpZ = submul(this->Row[2], that.Row[0].Z, that.Row[1].Z, that.Row[2].Z);
	tmpW = submul(this->Row[2], that.Row[0].W, that.Row[1].W, that.Row[2].W);

	this->Row[2].X = tmpX;
	this->Row[2].Y = tmpY;
	this->Row[2].Z = tmpZ;
	this->Row[2].W += tmpW;
#else
	Matrix3D tmp = *this;
	this->mul(tmp, that);
#endif
}

// does "this = A * B"
WWINLINE void Matrix3D::mul(const Matrix3D& A, const Matrix3D& B)
{
	assert(this != &A);
	//assert(this != &B);
	// nope, this is actually ok. (srj)
	//assert(this != &B);

	float tmp1,tmp2,tmp3;

	tmp1 = B.Row[0].X;
	tmp2 = B.Row[1].X;
	tmp3 = B.Row[2].X;

	this->Row[0].X = submul(A.Row[0], tmp1, tmp2, tmp3);
	this->Row[1].X = submul(A.Row[1], tmp1, tmp2, tmp3);
	this->Row[2].X = submul(A.Row[2], tmp1, tmp2, tmp3);

	tmp1 = B.Row[0].Y;
	tmp2 = B.Row[1].Y;
	tmp3 = B.Row[2].Y;

	this->Row[0].Y = submul(A.Row[0], tmp1, tmp2, tmp3);
	this->Row[1].Y = submul(A.Row[1], tmp1, tmp2, tmp3);
	this->Row[2].Y = submul(A.Row[2], tmp1, tmp2, tmp3);

	tmp1 = B.Row[0].Z;
	tmp2 = B.Row[1].Z;
	tmp3 = B.Row[2].Z;

	this->Row[0].Z = submul(A.Row[0], tmp1, tmp2, tmp3);
	this->Row[1].Z = submul(A.Row[1], tmp1, tmp2, tmp3);
	this->Row[2].Z = submul(A.Row[2], tmp1, tmp2, tmp3);

	tmp1 = B.Row[0].W;
	tmp2 = B.Row[1].W;
	tmp3 = B.Row[2].W;

	this->Row[0].W = submul(A.Row[0], tmp1, tmp2, tmp3) + A.Row[0].W;
	this->Row[1].W = submul(A.Row[1], tmp1, tmp2, tmp3) + A.Row[1].W;
	this->Row[2].W = submul(A.Row[2], tmp1, tmp2, tmp3) + A.Row[2].W;
}

#endif

#ifdef ALLOW_TEMPORARIES
/*********************************************************************************************** 
 * operator * -- Matrix - vector multiplication                                                * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   02/24/1997 GH  : Created.                                                                 * 
 *=============================================================================================*/
WWINLINE Vector3 operator * (const Matrix3D &A,const Vector3 &a)
{
#if 0
	return Vector3
	(
		(A[0][0]*a[0] + A[0][1]*a[1] + A[0][2]*a[2] + A[0][3]),
		(A[1][0]*a[0] + A[1][1]*a[1] + A[1][2]*a[2] + A[1][3]),
		(A[2][0]*a[0] + A[2][1]*a[1] + A[2][2]*a[2] + A[2][3])
	);
#else
	return Vector3
	(
		(A.Row[0].X*a.X + A.Row[0].Y*a.Y + A.Row[0].Z*a.Z + A.Row[0].W),
		(A.Row[1].X*a.X + A.Row[1].Y*a.Y + A.Row[1].Z*a.Z + A.Row[1].W),
		(A.Row[2].X*a.X + A.Row[2].Y*a.Y + A.Row[2].Z*a.Z + A.Row[2].W)
	);
#endif
}

#else

WWINLINE void Matrix3D::mulVector3(const Vector3& in, Vector3& out) const
{
	// note, it's ok for in==out here.
	float x = (Row[0].X * in.X + Row[0].Y * in.Y + Row[0].Z * in.Z + Row[0].W);
	float y = (Row[1].X * in.X + Row[1].Y * in.Y + Row[1].Z * in.Z + Row[1].W);
	float z = (Row[2].X * in.X + Row[2].Y * in.Y + Row[2].Z * in.Z + Row[2].W);
	out.Set(x, y, z);
}

#endif

WWINLINE void Matrix3D::mulVector3Array(const Vector3* in, Vector3* out, int count) const
{
	assert(in != out);
#ifdef ALLOW_TEMPORARIES
	for (i=0; i<count; i++)
	{
		out[i] = (*this) * in[i];
	}
#else	
	while (count--)
	{
		// we know that in != out, so no temps needed
		out->X = (Row[0].X * in->X + Row[0].Y * in->Y + Row[0].Z * in->Z + Row[0].W);
		out->Y = (Row[1].X * in->X + Row[1].Y * in->Y + Row[1].Z * in->Z + Row[1].W);
		out->Z = (Row[2].X * in->X + Row[2].Y * in->Y + Row[2].Z * in->Z + Row[2].W);
		++in;
		++out;
	}
#endif
}

WWINLINE void Matrix3D::mulVector3Array(Vector3* inout, int count) const
{
#ifdef ALLOW_TEMPORARIES
	for (i=0; i<count; i++)
	{
		inout[i] = (*this) * inout[i];
	}
#else	
	while (count--)
	{
		float x = (Row[0].X * inout->X + Row[0].Y * inout->Y + Row[0].Z * inout->Z + Row[0].W);
		float y = (Row[1].X * inout->X + Row[1].Y * inout->Y + Row[1].Z * inout->Z + Row[1].W);
		float z = (Row[2].X * inout->X + Row[2].Y * inout->Y + Row[2].Z * inout->Z + Row[2].W);
		inout->Set(x, y, z);
		++inout;
	}
#endif
}

/*********************************************************************************************** 
 * operator == -- Matrix equality operator                                                     * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   04/29/1998 NH  : Created.                                                                 * 
 *=============================================================================================*/
WWINLINE bool operator == (const Matrix3D &A, const Matrix3D &B)
{
   for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 4; j++) {
         if (A[i][j] != B[i][j]) return false;
      }
   }
   return true;
}


/*********************************************************************************************** 
 * operator != -- Matrix inequality operator                                                   * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   04/29/1998 NH  : Created.                                                                 * 
 *=============================================================================================*/
WWINLINE bool operator != (const Matrix3D &A, const Matrix3D &B)
{
   return !(A == B);
}


WWINLINE void Matrix3D::Transform_Vector(const Matrix3D & A,const Vector3 & in,Vector3 * out)
{
	Vector3 tmp;
	Vector3 * v;

	// check for aliased parameters
	if (out == &in) {
		tmp = in;
		v = &tmp;
	} else {
		v = (Vector3 *)&in;		// whats the right way to do this...
	}

	out->X = (A[0][0] * v->X + A[0][1] * v->Y + A[0][2] * v->Z + A[0][3]);
	out->Y = (A[1][0] * v->X + A[1][1] * v->Y + A[1][2] * v->Z + A[1][3]);
	out->Z = (A[2][0] * v->X + A[2][1] * v->Y + A[2][2] * v->Z + A[2][3]);
}

WWINLINE void	Matrix3D::Rotate_Vector(const Matrix3D & A,const Vector3 & in,Vector3 * out)
{
	Vector3 tmp;
	Vector3 * v;

	// check for aliased parameters
	if (out == &in) {
		tmp = in;
		v = &tmp;
	} else {
		v = (Vector3 *)&in;
	}

	out->X = (A[0][0] * v->X + A[0][1] * v->Y + A[0][2] * v->Z);
	out->Y = (A[1][0] * v->X + A[1][1] * v->Y + A[1][2] * v->Z);
	out->Z = (A[2][0] * v->X + A[2][1] * v->Y + A[2][2] * v->Z);
}

WWINLINE void	Matrix3D::Inverse_Transform_Vector(const Matrix3D & A,const Vector3 & in,Vector3 * out)
{
	Vector3 tmp;
	Vector3 * v;

	// check for aliased parameters
	if (out == &in) {
		tmp = in;
		v = &tmp;
	} else {
		v = (Vector3 *)&in;		// whats the right way to do this...
	}

	Vector3 diff(v->X - A[0][3], v->Y - A[1][3], v->Z - A[2][3]);
	Matrix3D::Inverse_Rotate_Vector(A, diff, out);
}

WWINLINE void	Matrix3D::Inverse_Rotate_Vector(const Matrix3D & A,const Vector3 & in,Vector3 * out)
{
	Vector3 tmp;
	Vector3 * v;

	// check for aliased parameters
	if (out == &in) {
		tmp = in;
		v = &tmp;
	} else {
		v = (Vector3 *)&in;
	}
	out->X = (A[0][0] * v->X + A[1][0] * v->Y + A[2][0] * v->Z);
	out->Y = (A[0][1] * v->X + A[1][1] * v->Y + A[2][1] * v->Z);
	out->Z = (A[0][2] * v->X + A[1][2] * v->Y + A[2][2] * v->Z);
}

class DynamicMatrix3D : public W3DMPO
{
	W3DMPO_GLUE(DynamicMatrix3D)
public:
	Matrix3D Mat;
};
#endif /* MATRIX3D_H */
