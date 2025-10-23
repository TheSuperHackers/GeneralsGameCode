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

/* $Header: /Commando/Code/wwmath/matrix3.h 36    5/11/01 7:11p Jani_p $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                    File Name : MATRIX3.H                                                    *
 *                                                                                             *
 *               Org Programmer : Greg Hjelstrom                                               *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               *
 *                                                                                             *
 *                   Start Date : 06/02/97                                                     *
 *                                                                                             *
 *                  Last Update : June 6, 2002 [KM]                                            *
 *                                                                                             *
 * 06/26/02 KM Matrix name change to avoid MAX conflicts                                       *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Matrix3x3::Matrix3x3 -- Constructor, optionally initialize to Identitiy matrix                *
 *   Matrix3x3::Matrix3x3 -- Copy Constructor                                                      *
 *   Matrix3x3::Matrix3x3 -- Convert a Matrix3D (fake 4x4) to a Matrix3x3                            *
 *   Matrix3x3::Matrix3x3 -- Constructor                                                           *
 *   Matrix3x3::Transpose -- Returns transpose of the matrix                                     *
 *   Matrix3x3::Inverse -- returns the inverse of the matrix                                     *
 *   Matrix3x3::Determinant -- returns the determinant of the matrix                             *
 *   Matrix3x3::operator = -- assignment operator                                                *
 *   Matrix3x3::operator += -- "plus equals" operator                                            *
 *   Matrix3x3::operator-= -- "minus equals" operator                                            *
 *   Matrix3x3::operator *= -- "times equals" operator                                           *
 *   Matrix3x3::operator /= -- "divide equals" operator                                          *
 *   Create_X_Rotation_Matrix3 -- creates a matrix3 which is a rotation about X                *
 *   Create_Y_Rotation_Matrix3 -- Creates a Matrix3x3 which is a rotation about Y                *
 *   Create_Z_Rotation_Matrix3 -- Creates a matrix3 which is a rotation about Z                *
 *   Matrix3x3::Rotate_X -- Post-mutiplies an x rotation onto the current matrix                 *
 *   Matrix3x3::Rotate_Y -- Post-multiplies the matrix with a rotation about Y                   *
 *   Matrix3x3::Rotate_Z -- Post-multiplies the matrix with a rotation about Z                   *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "vector3.h"
#include "wwdebug.h"
#ifdef _UNIX
#include "osdep.h"
#endif


class Matrix3D;
class Matrix4x4;
class Quaternion;

class Matrix3x3
{
public:

	/*
	** Constructors
	*/
	WWINLINE Matrix3x3(void) {};
	WWINLINE Matrix3x3(const Matrix3x3 & m);

	WWINLINE explicit Matrix3x3(bool identity);
	WWINLINE explicit Matrix3x3(const Vector3 & v0, const Vector3 & v1, const Vector3 & v2);
	explicit Matrix3x3(const Matrix3D & m);
	explicit Matrix3x3(const Matrix4x4 & m);
	WWINLINE explicit Matrix3x3(
		float m11,float m12,float m13,
		float m21,float m22,float m23,
		float m31,float m32,float m33
	);
	WWINLINE explicit Matrix3x3(const Vector3 & axis,float angle);
	WWINLINE explicit Matrix3x3(const Vector3 & axis,float s_angle,float c_angle);
	WWINLINE explicit Matrix3x3(const Quaternion & q);

	/*
	** 'Set' functions
	*/
	void Set(const Matrix3D & m);
	void Set(const Matrix4x4 & m);
	WWINLINE void Set(const Vector3 & v0, const Vector3 & v1, const Vector3 & v2);
	WWINLINE void Set(
		float m11,float m12,float m13,
		float m21,float m22,float m23,
		float m31,float m32,float m33
	);
	WWINLINE void Set(const Vector3 & axis,float angle);
	WWINLINE void Set(const Vector3 & axis,float s_angle,float c_angle);
	void Set(const Quaternion & q);

	/*
	** Access operators
	*/
	WWINLINE Vector3 & operator [] (int i) { return Row[i]; }
	WWINLINE const Vector3 & operator [] (int i) const { return Row[i]; }

	/*
	** Transpose and Inverse
	*/
	WWINLINE Matrix3x3	Transpose	(void) const;
	WWINLINE Matrix3x3	Inverse		(void) const;
	WWINLINE float		Determinant	(void) const;

	/*
	** Assignment operators
	*/
	WWINLINE Matrix3x3 & operator = (const Matrix3x3 & m);
	Matrix3x3 & operator = (const Matrix3D & m);
	Matrix3x3 & operator = (const Matrix4x4 & m);

	WWINLINE Matrix3x3 & operator += (const Matrix3x3 & m);
	WWINLINE Matrix3x3 & operator -= (const Matrix3x3 & m);
	WWINLINE Matrix3x3 & operator *= (float d);
	WWINLINE Matrix3x3 & operator /= (float d);

	WWINLINE void	Make_Identity(void);

	/*
	** Automatically concatenate a rotation onto the current matrix
	*/
	WWINLINE void Rotate_X(float theta);
	WWINLINE void Rotate_X(float s,float c);

	WWINLINE void Rotate_Y(float theta);
	WWINLINE void Rotate_Y(float s,float c);

	WWINLINE void Rotate_Z(float theta);
	WWINLINE void Rotate_Z(float s,float c);

	/*
	** These functions will give you the approximate amount that the
	** matrix has been rotated about a given axis.  These functions
	** cannot be used to re-build a matrx.  Use the EulerAnglesClass
	** to convert a matrix into a set of three Euler angles.
	*/
	float Get_X_Rotation(void) const;
	float Get_Y_Rotation(void) const;
	float Get_Z_Rotation(void) const;

	/*
	** These functions return a vector representing the direction an
   ** axis is pointing.
   */
	WWINLINE Vector3 Get_X_Vector(void) const;
	WWINLINE Vector3 Get_Y_Vector(void) const;
	WWINLINE Vector3 Get_Z_Vector(void) const;
	WWINLINE void Get_X_Vector(Vector3 * set_x) const;
	WWINLINE void Get_Y_Vector(Vector3 * set_y) const;
	WWINLINE void Get_Z_Vector(Vector3 * set_z) const;

	/*
	** Negation
	*/
	WWINLINE friend Matrix3x3 operator - (const Matrix3x3& a);

	/*
	** Scalar multiplication and division
	*/
	WWINLINE friend Matrix3x3 operator * (const Matrix3x3& a,float d);
	WWINLINE friend Matrix3x3 operator * (float d,const Matrix3x3& a);
	WWINLINE friend Matrix3x3 operator / (const Matrix3x3& a,float d);

	/*
	** matrix addition
	*/
	WWINLINE friend Matrix3x3 operator + (const Matrix3x3& a, const Matrix3x3& b);
	static WWINLINE void Add(const Matrix3x3 & a, const Matrix3x3 & b,Matrix3x3 * res);

	/*
	** matrix subtraction
	*/
	WWINLINE friend Matrix3x3 operator - (const Matrix3x3 & a, const Matrix3x3 & b);
	static WWINLINE void Subtract(const Matrix3x3 & a, const Matrix3x3 & b,Matrix3x3 * res);

	/*
	** matrix multiplication
	*/
	WWINLINE friend Matrix3x3 operator * (const Matrix3x3 & a, const Matrix3x3 & b);
	WWINLINE friend Matrix3x3 operator * (const Matrix3D & a, const Matrix3x3 & b);
	WWINLINE friend Matrix3x3 operator * (const Matrix3x3 & a, const Matrix3D & b);

	static void Multiply(const Matrix3x3 & a, const Matrix3x3 & b,Matrix3x3 * res);
	static void Multiply(const Matrix3D & a, const Matrix3x3 & b,Matrix3x3 * res);
	static void Multiply(const Matrix3x3 & a, const Matrix3D & b,Matrix3x3 * res);

	/*
	** Matrix-Vector multiplication
	*/
	WWINLINE friend Vector3 operator * (const Matrix3x3 & a, const Vector3 & v);
	static WWINLINE void	Rotate_Vector(const Matrix3x3 & tm,const Vector3 & in,Vector3 * out);
	static WWINLINE void	Transpose_Rotate_Vector(const Matrix3x3 & tm,const Vector3 & in,Vector3 * out);

	/*
	** Comparison operators
	*/
	WWINLINE friend int operator == (const Matrix3x3 & a, const Matrix3x3 & b);
	WWINLINE friend int operator != (const Matrix3x3 & a, const Matrix3x3 & b);

	/*
	** Swap two matrices in place
	*/
	WWINLINE friend void Swap(Matrix3x3 & a,Matrix3x3 & b);

	/*
	** Check whether a matrix is orthogonal, make it orthogonal
	*/
	int	Is_Orthogonal(void) const;
	void	Re_Orthogonalize(void);

	/*
	** Miscellaneous
	*/
	WWINLINE void	Rotate_AABox_Extent(const Vector3 & extent,Vector3 * new_extent);

	/*
	** Some useful pre-initialized Matrix3x3's
	*/
	static const Matrix3x3			Identity;
	static const Matrix3x3			RotateX90;
	static const Matrix3x3			RotateX180;
	static const Matrix3x3			RotateX270;
	static const Matrix3x3			RotateY90;
	static const Matrix3x3			RotateY180;
	static const Matrix3x3			RotateY270;
	static const Matrix3x3			RotateZ90;
	static const Matrix3x3			RotateZ180;
	static const Matrix3x3			RotateZ270;

protected:

	Vector3 Row[3];

};


/***********************************************************************************************
 * Matrix3x3::Matrix3x3 -- Constructor, optionally initialize to Identitiy matrix                  *
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
WWINLINE Matrix3x3::Matrix3x3(bool identity)
{
	if (identity) {
		Row[0].Set(1.0,0.0,0.0);
		Row[1].Set(0.0,1.0,0.0);
		Row[2].Set(0.0,0.0,1.0);
	}
}

/***********************************************************************************************
 * Matrix3x3::Matrix3x3 -- Copy Constructor                                                        *
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
WWINLINE Matrix3x3::Matrix3x3(const Matrix3x3 & m)
{
	Row[0] = m.Row[0]; Row[1] = m.Row[1]; Row[2] = m.Row[2];
}

/***********************************************************************************************
 * Matrix3x3::Matrix3x3 -- Constructor                                                             *
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
WWINLINE Matrix3x3::Matrix3x3(const Vector3 & r0, const Vector3 & r1, const Vector3 & r2)
{
	Row[0] = r0;
	Row[1] = r1;
	Row[2] = r2;
}

WWINLINE void Matrix3x3::Set(const Vector3 & r0, const Vector3 & r1, const Vector3 & r2)
{
	Row[0] = r0;
	Row[1] = r1;
	Row[2] = r2;
}

WWINLINE void Matrix3x3::Make_Identity(void)
{
	Row[0].Set(1.0f,0.0f,0.0f);
	Row[1].Set(0.0f,1.0f,0.0f);
	Row[2].Set(0.0f,0.0f,1.0f);
}


WWINLINE Matrix3x3::Matrix3x3
(
	float m11,float m12,float m13,
	float m21,float m22,float m23,
	float m31,float m32,float m33
)
{
	Row[0].Set(m11,m12,m13);
	Row[1].Set(m21,m22,m23);
	Row[2].Set(m31,m32,m33);
}

WWINLINE void Matrix3x3::Set
(
	float m11,float m12,float m13,
	float m21,float m22,float m23,
	float m31,float m32,float m33
)
{
	Row[0].Set(m11,m12,m13);
	Row[1].Set(m21,m22,m23);
	Row[2].Set(m31,m32,m33);
}

WWINLINE Matrix3x3::Matrix3x3(const Vector3 & axis,float angle)
{
	Set(axis,angle);
}

WWINLINE Matrix3x3::Matrix3x3(const Vector3 & axis,float s_angle,float c_angle)
{
	Set(axis,s_angle,c_angle);
}

WWINLINE void Matrix3x3::Set(const Vector3 & axis,float angle)
{
	Set(axis,sinf(angle),cosf(angle));
}

WWINLINE void Matrix3x3::Set(const Vector3 & axis,float s,float c)
{
	WWASSERT(WWMath::Fabs(axis.Length2() - 1.0f) < 0.001f);

	Row[0].Set(
		(float)(axis[0]*axis[0] + c*(1.0f - axis[0]*axis[0])),
		(float)(axis[0]*axis[1]*(1.0f - c) - axis[2]*s),
		(float)(axis[2]*axis[0]*(1.0f - c) + axis[1]*s)
	);

	Row[1].Set(
		(float)(axis[0]*axis[1]*(1.0f - c) + axis[2]*s),
		(float)(axis[1]*axis[1] + c*(1.0f - axis[1]*axis[1])),
		(float)(axis[1]*axis[2]*(1.0f - c) - axis[0]*s)
	);

	Row[2].Set(
		(float)(axis[2]*axis[0]*(1.0f - c) - axis[1]*s),
		(float)(axis[1]*axis[2]*(1.0f - c) + axis[0]*s),
		(float)(axis[2]*axis[2] + c*(1 - axis[2]*axis[2]))
	);
}

WWINLINE Matrix3x3::Matrix3x3(const Quaternion & q)
{
	this->Set(q);
}

/***********************************************************************************************
 * Matrix3x3::Transpose -- Returns transpose of the matrix                                       *
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
WWINLINE Matrix3x3 Matrix3x3::Transpose() const
{
    return Matrix3x3(
			Vector3(Row[0][0], Row[1][0], Row[2][0]),
			Vector3(Row[0][1], Row[1][1], Row[2][1]),
			Vector3(Row[0][2], Row[1][2], Row[2][2])
	);
}

/***********************************************************************************************
 * Matrix3x3::Inverse -- returns the inverse of the matrix                                       *
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
WWINLINE Matrix3x3 Matrix3x3::Inverse() const    // Gauss-Jordan elimination with partial pivoting
{
	Matrix3x3 a(*this);				// As a evolves from original mat into identity
	Matrix3x3 b(true);				// b evolves from identity into inverse(a)
	int i, j, i1;

	// Loop over cols of a from left to right, eliminating above and below diagonal
	for (j=0; j<3; j++) {

		// Find largest pivot in column j among rows j..3
		i1 = j;
		for (i=j+1; i<3; i++) {
			if (WWMath::Fabs(a[i][j]) > WWMath::Fabs(a[i1][j])) {
				i1 = i;
			}
		}

		// Swap rows i1 and j in a and b to put pivot on diagonal
		Swap(a.Row[i1], a.Row[j]);
		Swap(b.Row[i1], b.Row[j]);

		// Scale row j to have a unit diagonal
		if (a[j][j]==0.) {
			//Matrix3x3::inverse: singular matrix; can't invert
		}
		b.Row[j] /= a.Row[j][j];
		a.Row[j] /= a.Row[j][j];

		// Eliminate off-diagonal elems in col j of a, doing identical ops to b
		for (i=0; i<3; i++) {
			if (i != j) {
				b.Row[i] -= a[i][j] * b.Row[j];
				a.Row[i] -= a[i][j] * a.Row[j];
			}
		}
	}
	return b;
}


/***********************************************************************************************
 * Matrix3x3::Determinant -- returns the determinant of the matrix                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/7/20     DRM : Created.                                                                 *
 *=============================================================================================*/
WWINLINE float Matrix3x3::Determinant(void) const
{
	return   Row[0][0] * (Row[1][1] * Row[2][2] - Row[1][2] * Row[2][1])
		    - Row[0][1] * (Row[1][0] * Row[2][2] - Row[1][2] * Row[2][0])
			 - Row[0][2] * (Row[1][0] * Row[2][1] - Row[1][1] * Row[2][0]);
}

/***********************************************************************************************
 * Matrix3x3::operator = -- assignment operator                                                  *
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
WWINLINE Matrix3x3 & Matrix3x3::operator = (const Matrix3x3 & m)
{
	Row[0] = m.Row[0]; Row[1] = m.Row[1]; Row[2] = m.Row[2];
	return *this;
}

/***********************************************************************************************
 * Matrix3x3::operator += -- "plus equals" operator                                              *
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
WWINLINE Matrix3x3& Matrix3x3::operator += (const Matrix3x3 & m)
{
	Row[0] += m.Row[0]; Row[1] += m.Row[1]; Row[2] += m.Row[2];
	return *this;
}

/***********************************************************************************************
 * Matrix3x3::operator-= -- "minus equals" operator                                              *
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
WWINLINE Matrix3x3& Matrix3x3::operator -= (const Matrix3x3 & m)
{
	Row[0] -= m.Row[0]; Row[1] -= m.Row[1]; Row[2] -= m.Row[2];
	return *this;
}

/***********************************************************************************************
 * Matrix3x3::operator *= -- "times equals" operator                                             *
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
WWINLINE Matrix3x3& Matrix3x3::operator *= (float d)
{
	Row[0] *= d; Row[1] *= d; Row[2] *= d;
	return *this;
}

/***********************************************************************************************
 * Matrix3x3::operator /= -- "divide equals" operator                                            *
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
WWINLINE Matrix3x3& Matrix3x3::operator /= (float d)
{
	Row[0] /= d; Row[1] /= d; Row[2] /= d;
	return *this;
}

/***********************************************************************************************
 * Matrix3x3::Get_X_Rotation -- approximates the rotation about the X axis                       *
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
WWINLINE float Matrix3x3::Get_X_Rotation(void) const
{
	Vector3 v = (*this) * Vector3(0.0,1.0,0.0);
	return WWMath::Atan2(v[2], v[1]);
}

/***********************************************************************************************
 * Matrix3x3::Get_Y_Rotation -- approximates the rotation about the Y axis                       *
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
WWINLINE float Matrix3x3::Get_Y_Rotation(void) const
{
	Vector3 v = (*this) * Vector3(0.0,0.0,1.0);
	return WWMath::Atan2(v[0],v[2]);
}

/***********************************************************************************************
 * Matrix3x3::Get_Z_Rotation -- approximates the rotation about the Z axis                       *
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
WWINLINE float Matrix3x3::Get_Z_Rotation(void) const
{
	Vector3 v = (*this) * Vector3(1.0,0.0,0.0);
	return WWMath::Atan2(v[1],v[0]);
}

WWINLINE Vector3 Matrix3x3::Get_X_Vector(void) const
{
   return Vector3(Row[0][0], Row[1][0], Row[2][0]);
}

WWINLINE Vector3 Matrix3x3::Get_Y_Vector(void) const
{
   return Vector3(Row[0][1], Row[1][1], Row[2][1]);
}

WWINLINE Vector3 Matrix3x3::Get_Z_Vector(void) const
{
   return Vector3(Row[0][2], Row[1][2], Row[2][2]);
}

WWINLINE void Matrix3x3::Get_X_Vector(Vector3 * set) const
{
   set->Set(Row[0][0], Row[1][0], Row[2][0]);
}

WWINLINE void Matrix3x3::Get_Y_Vector(Vector3 * set) const
{
   set->Set(Row[0][1], Row[1][1], Row[2][1]);
}

WWINLINE void Matrix3x3::Get_Z_Vector(Vector3 * set) const
{
   set->Set(Row[0][2], Row[1][2], Row[2][2]);
}

WWINLINE Matrix3x3 operator - (const Matrix3x3 & a)
{
	return Matrix3x3(-a.Row[0], -a.Row[1], -a.Row[2]);
}

WWINLINE Matrix3x3 operator * (const Matrix3x3 & a, float d)
{
	return Matrix3x3(a.Row[0] * d, a.Row[1] * d, a.Row[2] * d);
}

WWINLINE Matrix3x3 operator * (float d, const Matrix3x3 & a)
{
	return a*d;
}

WWINLINE Matrix3x3 operator / (const Matrix3x3 & a, float d)
{
	float ood = 1.0f / d;
	return Matrix3x3(a.Row[0] * ood, a.Row[1] * ood, a.Row[2] * ood);
}

/*
** matrix addition
*/
WWINLINE Matrix3x3 operator + (const Matrix3x3 & a, const Matrix3x3 & b)
{
	return Matrix3x3(
				a.Row[0] + b.Row[0],
				a.Row[1] + b.Row[1],
				a.Row[2] + b.Row[2]
	);
}

WWINLINE void Matrix3x3::Add(const Matrix3x3 & a, const Matrix3x3 & b,Matrix3x3 * c)
{
	WWASSERT(c);
	Vector3::Add(a.Row[0],b.Row[0],&(c->Row[0]));
	Vector3::Add(a.Row[1],b.Row[1],&(c->Row[1]));
	Vector3::Add(a.Row[2],b.Row[2],&(c->Row[2]));
}

/*
** matrix subtraction
*/
WWINLINE Matrix3x3 operator - (const Matrix3x3 & a, const Matrix3x3 & b)
{
	return Matrix3x3(
				a.Row[0] - b.Row[0],
				a.Row[1] - b.Row[1],
				a.Row[2] - b.Row[2]
	);
}

WWINLINE void Matrix3x3::Subtract(const Matrix3x3 & a, const Matrix3x3 & b,Matrix3x3 * c)
{
	WWASSERT(c);
	Vector3::Subtract(a.Row[0],b.Row[0],&(c->Row[0]));
	Vector3::Subtract(a.Row[1],b.Row[1],&(c->Row[1]));
	Vector3::Subtract(a.Row[2],b.Row[2],&(c->Row[2]));
}

/*
** matrix multiplication
*/
WWINLINE Matrix3x3 operator * (const Matrix3x3 & a, const Matrix3x3 & b)
{
	#define ROWCOL(i,j) a[i][0]*b[0][j] + a[i][1]*b[1][j] + a[i][2]*b[2][j]

	return Matrix3x3(
			Vector3(ROWCOL(0,0), ROWCOL(0,1), ROWCOL(0,2) ),
			Vector3(ROWCOL(1,0), ROWCOL(1,1), ROWCOL(1,2) ),
			Vector3(ROWCOL(2,0), ROWCOL(2,1), ROWCOL(2,2) )
	);

	#undef ROWCOL
}


/*
** Multiply a Matrix3x3 by a Vector3. Yeilds a Vector3 result
*/
WWINLINE Vector3 operator * (const Matrix3x3 & a, const Vector3 & v)
{
	return Vector3(
		a[0][0] * v[0] + a[0][1] * v[1] + a[0][2] * v[2],
		a[1][0] * v[0] + a[1][1] * v[1] + a[1][2] * v[2],
		a[2][0] * v[0] + a[2][1] * v[1] + a[2][2] * v[2]
	);
}


WWINLINE int operator == (const Matrix3x3 & a, const Matrix3x3 & b)
{
	return ((a [0] == b [0]) && (a [1] == b [1]) && (a [2] == b [2]));
}


WWINLINE int operator != (const Matrix3x3 & a, const Matrix3x3 & b)
{
	return (!(a == b));
}


/***********************************************************************************************
 * Matrix3x3::Rotate_X -- Post-mutiplies an x rotation onto the current matrix                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/26/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void Matrix3x3::Rotate_X(float theta)
{
	Rotate_X(sinf(theta),cosf(theta));
}

WWINLINE void Matrix3x3::Rotate_X(float s,float c)
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
 * Matrix3x3::Rotate_Y -- Post-multiplies the matrix with a rotation about Y                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/26/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void Matrix3x3::Rotate_Y(float theta)
{
	Rotate_Y(sinf(theta),cosf(theta));
}

WWINLINE void Matrix3x3::Rotate_Y(float s,float c)
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
 * Matrix3x3::Rotate_Z -- Post-multiplies the matrix with a rotation about Z                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/26/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void Matrix3x3::Rotate_Z(float theta)
{
	Rotate_Z(sinf(theta),cosf(theta));
}

WWINLINE void Matrix3x3::Rotate_Z(float s,float c)
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
 * Create_X_Rotation_Matrix3 -- creates a matrix3 which is a rotation about X                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/26/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
WWINLINE Matrix3x3 Create_X_Rotation_Matrix3(float s,float c)
{
	Matrix3x3 mat;

	mat[0][0] = 1.0f;
	mat[0][1] = 0.0f;
	mat[0][2] = 0.0f;

	mat[1][0] = 0.0f;
	mat[1][1] = c;
	mat[1][2] = -s;

	mat[2][0] = 0.0f;
	mat[2][1] = s;
	mat[2][2] = c;

	return mat;
}

WWINLINE Matrix3x3 Create_X_Rotation_Matrix3(float rad)
{
	return Create_X_Rotation_Matrix3(sinf(rad),cosf(rad));
}

/***********************************************************************************************
 * Create_Y_Rotation_Matrix3 -- Creates a Matrix3x3 which is a rotation about Y                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/26/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
WWINLINE Matrix3x3 Create_Y_Rotation_Matrix3(float s,float c)
{
	Matrix3x3 mat;

	mat[0][0] = c;
	mat[0][1] = 0.0f;
	mat[0][2] = s;

	mat[1][0] = 0.0f;
	mat[1][1] = 1.0f;
	mat[1][2] = 0.0f;

	mat[2][0] = -s;
	mat[2][1] = 0.0f;
	mat[2][2] = c;

	return mat;
}

WWINLINE Matrix3x3 Create_Y_Rotation_Matrix3(float rad)
{
	return Create_Y_Rotation_Matrix3(sinf(rad),cosf(rad));
}

/***********************************************************************************************
 * Create_Z_Rotation_Matrix3 -- Creates a matrix3 which is a rotation about Z                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   08/26/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
WWINLINE Matrix3x3 Create_Z_Rotation_Matrix3(float s,float c)
{
	Matrix3x3 mat;

	mat[0][0] = c;
	mat[0][1] = -s;
	mat[0][2] = 0.0f;

	mat[1][0] = s;
	mat[1][1] = c;
	mat[1][2] = 0.0f;

	mat[2][0] = 0.0f;
	mat[2][1] = 0.0f;
	mat[2][2] = 1.0f;

	return mat;
}

WWINLINE Matrix3x3 Create_Z_Rotation_Matrix3(float rad)
{
	return Create_Z_Rotation_Matrix3(sinf(rad),cosf(rad));
}

WWINLINE void Matrix3x3::Rotate_Vector(const Matrix3x3 & A,const Vector3 & in,Vector3 * out)
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

	out->X = (A[0][0] * v->X + A[0][1] * v->Y + A[0][2] * v->Z);
	out->Y = (A[1][0] * v->X + A[1][1] * v->Y + A[1][2] * v->Z);
	out->Z = (A[2][0] * v->X + A[2][1] * v->Y + A[2][2] * v->Z);
}

WWINLINE void Matrix3x3::Transpose_Rotate_Vector(const Matrix3x3 & A,const Vector3 & in,Vector3 * out)
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

WWINLINE void Matrix3x3::Rotate_AABox_Extent(const Vector3 & extent,Vector3 * set_extent)
{
	// push each extent out to the projections of the original extents
	for (int i=0; i<3; i++) {

		// start the center out at the translation portion of the matrix
		// and the extent at zero
		(*set_extent)[i] = 0.0f;

		for (int j=0; j<3; j++) {
			(*set_extent)[i] += WWMath::Fabs(Row[i][j] * extent[j]);
		}
	}
}
