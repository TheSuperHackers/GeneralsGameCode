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

/* $Header: /Commando/Code/Tools/max2w3d/WWmatrix3.h 33    2/03/00 4:55p Jason_a $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                    File Name : MATRIX3.H                                                    *
 *                                                                                             *
 *                   Programmer : Greg Hjelstrom                                               *
 *                                                                                             *
 *                   Start Date : 06/02/97                                                     *
 *                                                                                             *
 *                  Last Update : June 2, 1997 [GH]                                            *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Matrix3::Matrix3 -- Constructor, optionally initialize to Identitiy matrix                *
 *   Matrix3::Matrix3 -- Copy Constructor                                                      *
 *   Matrix3::Matrix3 -- Convert a Matrix3D (fake 4x4) to a Matrix3                            *
 *   Matrix3::Matrix3 -- Constructor                                                           *
 *   Matrix3::Transpose -- Returns transpose of the matrix                                     *
 *   Matrix3::Inverse -- returns the inverse of the matrix                                     *
 *   Matrix3::Determinant -- returns the determinant of the matrix                             *
 *   Matrix3::operator = -- assignment operator                                                *
 *   Matrix3::operator += -- "plus equals" operator                                            *
 *   Matrix3::operator-= -- "minus equals" operator                                            *
 *   Matrix3::operator *= -- "times equals" operator                                           *
 *   Matrix3::operator /= -- "divide equals" operator                                          *
 *   Create_X_Rotation_Matrix3 -- creates a matrix3 which is a rotation about X                *
 *   Create_Y_Rotation_Matrix3 -- Creates a Matrix3 which is a rotation about Y                *
 *   Create_Z_Rotation_Matrix3 -- Creates a matrix3 which is a rotation about Z                *
 *   Matrix3::Rotate_X -- Post-mutiplies an x rotation onto the current matrix                 *
 *   Matrix3::Rotate_Y -- Post-multiplies the matrix with a rotation about Y                   *
 *   Matrix3::Rotate_Z -- Post-multiplies the matrix with a rotation about Z                   *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef Matrix3_H
#define Matrix3_H

#include "always.h"
#include "vector3.h"
//#include "wwdebug.h"


class Matrix3D;
class Matrix4;
class Quaternion;

class Matrix3
{
public:

	/*
	** Constructors
	*/
	Matrix3(void) {};
	Matrix3(const Matrix3 & m);

	explicit Matrix3(bool identity);
	explicit Matrix3(const Vector3 & v0, const Vector3 & v1, const Vector3 & v2);
	explicit Matrix3(const Matrix3D & m);
	explicit Matrix3(const Matrix4 & m);
	explicit Matrix3(
		float m11,float m12,float m13,
		float m21,float m22,float m23,
		float m31,float m32,float m33
	);
	explicit Matrix3(const Vector3 & axis,float angle);
	explicit Matrix3(const Vector3 & axis,float s_angle,float c_angle);
	explicit Matrix3(const Quaternion & q);

	/*
	** 'Set' functions
	*/
	void Set(const Matrix3D & m);
	void Set(const Matrix4 & m);
	void Set(const Vector3 & v0, const Vector3 & v1, const Vector3 & v2);
	void Set(
		float m11,float m12,float m13,
		float m21,float m22,float m23,
		float m31,float m32,float m33
	);
	void Set(const Vector3 & axis,float angle);
	void Set(const Vector3 & axis,float s_angle,float c_angle);
	void Set(const Quaternion & q);

	/*
	** Access operators
	*/
	Vector3 & operator [] (int i) { return Row[i]; }
	const Vector3 & operator [] (int i) const { return Row[i]; }

	/*
	** Transpose and Inverse
	*/
	Matrix3	Transpose	(void) const;
	Matrix3	Inverse		(void) const;
	float		Determinant	(void) const;

	/*
	** Assignment operators
	*/
	Matrix3 & operator = (const Matrix3 & m);
	Matrix3 & operator = (const Matrix3D & m);
	Matrix3 & operator = (const Matrix4 & m);

	Matrix3 & operator += (const Matrix3 & m);
	Matrix3 & operator -= (const Matrix3 & m);
	Matrix3 & operator *= (float d);
	Matrix3 & operator /= (float d);

	void	Make_Identity(void);

	/*
	** Automatically concatenate a rotation onto the current matrix
	*/
	void Rotate_X(float theta);
	void Rotate_X(float s,float c);

	void Rotate_Y(float theta);
	void Rotate_Y(float s,float c);

	void Rotate_Z(float theta);
	void Rotate_Z(float s,float c);

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
	Vector3 Get_X_Vector(void) const;
	Vector3 Get_Y_Vector(void) const;
	Vector3 Get_Z_Vector(void) const;
	void Get_X_Vector(Vector3 * set_x) const;
	void Get_Y_Vector(Vector3 * set_y) const;
	void Get_Z_Vector(Vector3 * set_z) const;

	/*
	** Negation
	*/
	friend Matrix3 operator - (const Matrix3& a);

	/*
	** Scalar multiplication and division
	*/
	friend Matrix3 operator * (const Matrix3& a,float d);
	friend Matrix3 operator * (float d,const Matrix3& a);
	friend Matrix3 operator / (const Matrix3& a,float d);

	/*
	** matrix addition
	*/
	friend Matrix3 operator + (const Matrix3& a, const Matrix3& b);
	static void Add(const Matrix3 & a, const Matrix3 & b,Matrix3 * res);

	/*
	** matrix subtraction
	*/
	friend Matrix3 operator - (const Matrix3 & a, const Matrix3 & b);
	static void Subtract(const Matrix3 & a, const Matrix3 & b,Matrix3 * res);

	/*
	** matrix multiplication
	*/
	friend Matrix3 operator * (const Matrix3 & a, const Matrix3 & b);
	friend Matrix3 operator * (const Matrix3D & a, const Matrix3 & b);
	friend Matrix3 operator * (const Matrix3 & a, const Matrix3D & b);

	static void Multiply(const Matrix3 & a, const Matrix3 & b,Matrix3 * res);
	static void Multiply(const Matrix3D & a, const Matrix3 & b,Matrix3 * res);
	static void Multiply(const Matrix3 & a, const Matrix3D & b,Matrix3 * res);

	/*
	** Matrix-Vector multiplication
	*/
	friend Vector3 operator * (const Matrix3 & a, const Vector3 & v);
	static void	Rotate_Vector(const Matrix3 & tm,const Vector3 & in,Vector3 * out);
	static void	Transpose_Rotate_Vector(const Matrix3 & tm,const Vector3 & in,Vector3 * out);

	/*
	** Comparison operators
	*/
	friend int operator == (const Matrix3 & a, const Matrix3 & b);
	friend int operator != (const Matrix3 & a, const Matrix3 & b);

	/*
	** Swap two matrices in place
	*/
	friend void Swap(Matrix3 & a,Matrix3 & b);

	/*
	** Check whether a matrix is orthogonal, make it orthogonal
	*/
	int	Is_Orthogonal(void) const;
	void	Re_Orthogonalize(void);

	/*
	** Miscellaneous
	*/
	void	Rotate_AABox_Extent(const Vector3 & extent,Vector3 * new_extent);

	/*
	** Some useful pre-initialized Matrix3's
	*/
	static const Matrix3			Identity;
	static const Matrix3			RotateX90;
	static const Matrix3			RotateX180;
	static const Matrix3			RotateX270;
	static const Matrix3			RotateY90;
	static const Matrix3			RotateY180;
	static const Matrix3			RotateY270;
	static const Matrix3			RotateZ90;
	static const Matrix3			RotateZ180;
	static const Matrix3			RotateZ270;

protected:

	Vector3 Row[3];

};


/***********************************************************************************************
 * Matrix3::Matrix3 -- Constructor, optionally initialize to Identitiy matrix                  *
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
inline Matrix3::Matrix3(bool identity)
{
	if (identity) {
		Row[0].Set(1.0,0.0,0.0);
		Row[1].Set(0.0,1.0,0.0);
		Row[2].Set(0.0,0.0,1.0);
	}
}

/***********************************************************************************************
 * Matrix3::Matrix3 -- Copy Constructor                                                        *
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
inline Matrix3::Matrix3(const Matrix3 & m)
{
	Row[0] = m.Row[0]; Row[1] = m.Row[1]; Row[2] = m.Row[2];
}

/***********************************************************************************************
 * Matrix3::Matrix3 -- Constructor                                                             *
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
inline Matrix3::Matrix3(const Vector3 & r0, const Vector3 & r1, const Vector3 & r2)
{
	Row[0] = r0;
	Row[1] = r1;
	Row[2] = r2;
}

inline void Matrix3::Set(const Vector3 & r0, const Vector3 & r1, const Vector3 & r2)
{
	Row[0] = r0;
	Row[1] = r1;
	Row[2] = r2;
}

inline void Matrix3::Make_Identity(void)
{
	Row[0].Set(1.0f,0.0f,0.0f);
	Row[1].Set(0.0f,1.0f,0.0f);
	Row[2].Set(0.0f,0.0f,1.0f);
}


inline Matrix3::Matrix3
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

inline void Matrix3::Set
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

inline Matrix3::Matrix3(const Vector3 & axis,float angle)
{
	Set(axis,angle);
}

inline Matrix3::Matrix3(const Vector3 & axis,float s_angle,float c_angle)
{
	Set(axis,s_angle,c_angle);
}

inline void Matrix3::Set(const Vector3 & axis,float angle)
{
	Set(axis,sinf(angle),cosf(angle));
}

inline void Matrix3::Set(const Vector3 & axis,float s,float c)
{
	assert(WWMath::Fabs(axis.Length2() - 1.0f) < 0.001f);

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

inline Matrix3::Matrix3(const Quaternion & q)
{
	this->Set(q);
}

/***********************************************************************************************
 * Matrix3::Transpose -- Returns transpose of the matrix                                       *
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
inline Matrix3 Matrix3::Transpose() const
{
    return Matrix3(
			Vector3(Row[0][0], Row[1][0], Row[2][0]),
			Vector3(Row[0][1], Row[1][1], Row[2][1]),
			Vector3(Row[0][2], Row[1][2], Row[2][2])
	);
}

/***********************************************************************************************
 * Matrix3::Inverse -- returns the inverse of the matrix                                       *
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
inline Matrix3 Matrix3::Inverse() const    // Gauss-Jordan elimination with partial pivoting
{
	Matrix3 a(*this);				// As a evolves from original mat into identity
	Matrix3 b(true);				// b evolves from identity into inverse(a)
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
			//Matrix3::inverse: singular matrix; can't invert
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
 * Matrix3::Determinant -- returns the determinant of the matrix                               *
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
inline float Matrix3::Determinant(void) const
{
	return   Row[0][0] * (Row[1][1] * Row[2][2] - Row[1][2] * Row[2][1])
		    - Row[0][1] * (Row[1][0] * Row[2][2] - Row[1][2] * Row[2][0])
			 - Row[0][2] * (Row[1][0] * Row[2][1] - Row[1][1] * Row[2][0]);
}

/***********************************************************************************************
 * Matrix3::operator = -- assignment operator                                                  *
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
inline Matrix3 & Matrix3::operator = (const Matrix3 & m)
{
	Row[0] = m.Row[0]; Row[1] = m.Row[1]; Row[2] = m.Row[2];
	return *this;
}

/***********************************************************************************************
 * Matrix3::operator += -- "plus equals" operator                                              *
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
inline Matrix3& Matrix3::operator += (const Matrix3 & m)
{
	Row[0] += m.Row[0]; Row[1] += m.Row[1]; Row[2] += m.Row[2];
	return *this;
}

/***********************************************************************************************
 * Matrix3::operator-= -- "minus equals" operator                                              *
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
inline Matrix3& Matrix3::operator -= (const Matrix3 & m)
{
	Row[0] -= m.Row[0]; Row[1] -= m.Row[1]; Row[2] -= m.Row[2];
	return *this;
}

/***********************************************************************************************
 * Matrix3::operator *= -- "times equals" operator                                             *
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
inline Matrix3& Matrix3::operator *= (float d)
{
	Row[0] *= d; Row[1] *= d; Row[2] *= d;
	return *this;
}

/***********************************************************************************************
 * Matrix3::operator /= -- "divide equals" operator                                            *
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
inline Matrix3& Matrix3::operator /= (float d)
{
	Row[0] /= d; Row[1] /= d; Row[2] /= d;
	return *this;
}

/***********************************************************************************************
 * Matrix3::Get_X_Rotation -- approximates the rotation about the X axis                       *
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
inline float Matrix3::Get_X_Rotation(void) const
{
	Vector3 v = (*this) * Vector3(0.0,1.0,0.0);
	return atan2(v[2], v[1]);
}

/***********************************************************************************************
 * Matrix3::Get_Y_Rotation -- approximates the rotation about the Y axis                       *
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
inline float Matrix3::Get_Y_Rotation(void) const
{
	Vector3 v = (*this) * Vector3(0.0,0.0,1.0);
	return atan2(v[0],v[2]);
}

/***********************************************************************************************
 * Matrix3::Get_Z_Rotation -- approximates the rotation about the Z axis                       *
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
inline float Matrix3::Get_Z_Rotation(void) const
{
	Vector3 v = (*this) * Vector3(1.0,0.0,0.0);
	return atan2(v[1],v[0]);
}

inline Vector3 Matrix3::Get_X_Vector(void) const
{
   return Vector3(Row[0][0], Row[1][0], Row[2][0]);
}

inline Vector3 Matrix3::Get_Y_Vector(void) const
{
   return Vector3(Row[0][1], Row[1][1], Row[2][1]);
}

inline Vector3 Matrix3::Get_Z_Vector(void) const
{
   return Vector3(Row[0][2], Row[1][2], Row[2][2]);
}

inline void Matrix3::Get_X_Vector(Vector3 * set) const
{
   set->Set(Row[0][0], Row[1][0], Row[2][0]);
}

inline void Matrix3::Get_Y_Vector(Vector3 * set) const
{
   set->Set(Row[0][1], Row[1][1], Row[2][1]);
}

inline void Matrix3::Get_Z_Vector(Vector3 * set) const
{
   set->Set(Row[0][2], Row[1][2], Row[2][2]);
}

inline Matrix3 operator - (const Matrix3 & a)
{
	return Matrix3(-a.Row[0], -a.Row[1], -a.Row[2]);
}

inline Matrix3 operator * (const Matrix3 & a, float d)
{
	return Matrix3(a.Row[0] * d, a.Row[1] * d, a.Row[2] * d);
}

inline Matrix3 operator * (float d, const Matrix3 & a)
{
	return a*d;
}

inline Matrix3 operator / (const Matrix3 & a, float d)
{
	float ood = 1.0f / d;
	return Matrix3(a.Row[0] * ood, a.Row[1] * ood, a.Row[2] * ood);
}

/*
** matrix addition
*/
inline Matrix3 operator + (const Matrix3 & a, const Matrix3 & b)
{
	return Matrix3(
				a.Row[0] + b.Row[0],
				a.Row[1] + b.Row[1],
				a.Row[2] + b.Row[2]
	);
}

inline void Matrix3::Add(const Matrix3 & a, const Matrix3 & b,Matrix3 * c)
{
	assert(c);
	Vector3::Add(a.Row[0],b.Row[0],&(c->Row[0]));
	Vector3::Add(a.Row[1],b.Row[1],&(c->Row[1]));
	Vector3::Add(a.Row[2],b.Row[2],&(c->Row[2]));
}

/*
** matrix subtraction
*/
inline Matrix3 operator - (const Matrix3 & a, const Matrix3 & b)
{
	return Matrix3(
				a.Row[0] - b.Row[0],
				a.Row[1] - b.Row[1],
				a.Row[2] - b.Row[2]
	);
}

inline void Matrix3::Subtract(const Matrix3 & a, const Matrix3 & b,Matrix3 * c)
{
	assert(c);
	Vector3::Subtract(a.Row[0],b.Row[0],&(c->Row[0]));
	Vector3::Subtract(a.Row[1],b.Row[1],&(c->Row[1]));
	Vector3::Subtract(a.Row[2],b.Row[2],&(c->Row[2]));
}

/*
** matrix multiplication
*/
inline Matrix3 operator * (const Matrix3 & a, const Matrix3 & b)
{
	#define ROWCOL(i,j) a[i][0]*b[0][j] + a[i][1]*b[1][j] + a[i][2]*b[2][j]

	return Matrix3(
			Vector3(ROWCOL(0,0), ROWCOL(0,1), ROWCOL(0,2) ),
			Vector3(ROWCOL(1,0), ROWCOL(1,1), ROWCOL(1,2) ),
			Vector3(ROWCOL(2,0), ROWCOL(2,1), ROWCOL(2,2) )
	);

	#undef ROWCOL
}


/*
** Multiply a Matrix3 by a Vector3. Yeilds a Vector3 result
*/
inline Vector3 operator * (const Matrix3 & a, const Vector3 & v)
{
	return Vector3(
		a[0][0] * v[0] + a[0][1] * v[1] + a[0][2] * v[2],
		a[1][0] * v[0] + a[1][1] * v[1] + a[1][2] * v[2],
		a[2][0] * v[0] + a[2][1] * v[1] + a[2][2] * v[2]
	);
}


inline int operator == (const Matrix3 & a, const Matrix3 & b)
{
	return ((a [0] == b [0]) && (a [1] == b [1]) && (a [2] == b [2]));
}


inline int operator != (const Matrix3 & a, const Matrix3 & b)
{
	return (!(a == b));
}


/***********************************************************************************************
 * Matrix3::Rotate_X -- Post-mutiplies an x rotation onto the current matrix                   *
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
inline void Matrix3::Rotate_X(float theta)
{
	Rotate_X(sinf(theta),cosf(theta));
}

inline void Matrix3::Rotate_X(float s,float c)
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
 * Matrix3::Rotate_Y -- Post-multiplies the matrix with a rotation about Y                     *
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
inline void Matrix3::Rotate_Y(float theta)
{
	Rotate_Y(sinf(theta),cosf(theta));
}

inline void Matrix3::Rotate_Y(float s,float c)
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
 * Matrix3::Rotate_Z -- Post-multiplies the matrix with a rotation about Z                     *
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
inline void Matrix3::Rotate_Z(float theta)
{
	Rotate_Z(sinf(theta),cosf(theta));
}

inline void Matrix3::Rotate_Z(float s,float c)
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
inline Matrix3 Create_X_Rotation_Matrix3(float s,float c)
{
	Matrix3 mat;

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

inline Matrix3 Create_X_Rotation_Matrix3(float rad)
{
	return Create_X_Rotation_Matrix3(sinf(rad),cosf(rad));
}

/***********************************************************************************************
 * Create_Y_Rotation_Matrix3 -- Creates a Matrix3 which is a rotation about Y                  *
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
inline Matrix3 Create_Y_Rotation_Matrix3(float s,float c)
{
	Matrix3 mat;

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

inline Matrix3 Create_Y_Rotation_Matrix3(float rad)
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
inline Matrix3 Create_Z_Rotation_Matrix3(float s,float c)
{
	Matrix3 mat;

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

inline Matrix3 Create_Z_Rotation_Matrix3(float rad)
{
	return Create_Z_Rotation_Matrix3(sinf(rad),cosf(rad));
}

inline void Matrix3::Rotate_Vector(const Matrix3 & A,const Vector3 & in,Vector3 * out)
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

inline void Matrix3::Transpose_Rotate_Vector(const Matrix3 & A,const Vector3 & in,Vector3 * out)
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

#endif /*Matrix3_H*/
