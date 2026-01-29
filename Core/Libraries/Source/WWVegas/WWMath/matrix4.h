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

/* $Header: /Commando/Code/wwmath/matrix4.h 20    10/04/01 10:33a Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                    File Name : MATRIX4.h                                                    *
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
 *   Matrix4x4::Matrix4x4 -- Constructor, optionally initialize to Identitiy matrix                *
 *   Matrix4x4::Matrix4x4 -- Copy Constructor                                                      *
 *   Matrix4x4::Matrix4x4 -- Convert a Matrix3D (fake 4x4) to a Matrix4x4                            *
 *   Matrix4x4::Matrix4x4 -- Constructor                                                           *
 *   Matrix4x4::Make_Identity -- Initializes the matrix to Identity                              *
 *   Matrix4x4::Init -- Initializes from the contents of the give Matrix3D                       *
 *   Matrix4x4::Init -- Initializes the rows from the given Vector4s                             *
 *   Matrix4x4::Init -- Initializes the rows from the given 16 floats                            *
 *   Matrix4x4::Init_Ortho -- Initialize to an orthographic projection matrix                    *
 *   Matrix4x4::Init_Perspective -- Initialize to a perspective projection matrix                *
 *   Matrix4x4::Init_Perspective -- Initialize to a perspective projection matrix                *
 *   Matrix4x4::Transpose -- Returns transpose of the matrix                                     *
 *   Matrix4x4::Inverse -- returns the inverse of the matrix                                     *
 *   Matrix4x4::operator = -- assignment operator                                                *
 *   Matrix4x4::operator += -- "plus equals" operator                                            *
 *   Matrix4x4::operator -= -- "minus equals" operator                                            *
 *   Matrix4x4::operator *= -- "times equals" operator                                           *
 *   Matrix4x4::operator /= -- "divide equals" operator                                          *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "vector4.h"
#include "matrix3d.h"
#include "matrix3.h"


class Matrix4x4
{
public:

	/*
	** Constructors
	*/
	Matrix4x4(void) {};
	Matrix4x4(const Matrix4x4 & m);

	WWINLINE explicit Matrix4x4(bool identity);
	WWINLINE explicit Matrix4x4(const Matrix3D & m);
	WWINLINE explicit Matrix4x4(const Matrix3x3 & m);
	WWINLINE explicit Matrix4x4(const Vector4 & v0, const Vector4 & v1, const Vector4 & v2, const Vector4 & v3);
	WWINLINE explicit Matrix4x4(	float m11,float m12,float m13,float m14,
										float m21,float m22,float m23,float m24,
										float m31,float m32,float m33,float m34,
										float m41,float m42,float m43,float m44 );

	WWINLINE void		Make_Identity(void);
	WWINLINE void		Init(const Matrix3D & m);
	WWINLINE void		Init(const Matrix3x3 & m);
	WWINLINE void		Init(const Vector4 & v0, const Vector4 & v1, const Vector4 & v2, const Vector4 & v3);
	WWINLINE void		Init(	float m11,float m12,float m13,float m14,
									float m21,float m22,float m23,float m24,
									float m31,float m32,float m33,float m34,
									float m41,float m42,float m43,float m44 );

	/*
	** Projection matrices.  The znear and zfar parameters are positive values indicating the
	** distance from the camera to the z clipping planes.  See implementations for more info.
	*/
	WWINLINE void		Init_Ortho(float left,float right,float bottom,float top,float znear,float zfar);
	WWINLINE void		Init_Perspective(float hfov,float vfov,float znear,float zfar);
	WWINLINE void		Init_Perspective(float left,float right,float bottom,float top,float znear,float zfar);

	/*
	** Access operators
	*/
	WWINLINE Vector4 & operator [] (int i) { return Row[i]; }
	WWINLINE const Vector4 & operator [] (int i) const { return Row[i]; }

	/*
	** Transpose and Inverse
	*/
	WWINLINE Matrix4x4 Transpose(void) const;
	static WWINLINE Matrix4x4* Inverse(Matrix4x4* out, float* detOut, const Matrix4x4* m);
	WWINLINE Matrix4x4 Inverse(void) const;

	/*
	** Assignment operators
	*/
	WWINLINE Matrix4x4 & operator = (const Matrix4x4 & m);
	WWINLINE Matrix4x4 & operator += (const Matrix4x4 & m);
	WWINLINE Matrix4x4 & operator -= (const Matrix4x4 & m);
	WWINLINE Matrix4x4 & operator *= (float d);
	WWINLINE Matrix4x4 & operator /= (float d);

	/*
	** Negation
	*/
	WWINLINE friend Matrix4x4 operator - (const Matrix4x4& a);

	/*
	** Scalar multiplication and division
	*/
	WWINLINE friend Matrix4x4 operator * (const Matrix4x4& a,float d);
	WWINLINE friend Matrix4x4 operator * (float d,const Matrix4x4& a);
	WWINLINE friend Matrix4x4 operator / (const Matrix4x4& a,float d);

	/*
	** matrix addition
	*/
	WWINLINE friend Matrix4x4 operator + (const Matrix4x4& a, const Matrix4x4& b);
	WWINLINE friend Matrix4x4 Add(const Matrix4x4& a);

	/*
	** matrix subtraction
	*/
	WWINLINE friend Matrix4x4 operator - (const Matrix4x4 & a, const Matrix4x4 & b);
	WWINLINE friend Matrix4x4 Subtract(const Matrix4x4 & a, const Matrix4x4 & b);

	/*
	** matrix multiplication
	*/
	WWINLINE friend Matrix4x4 operator * (const Matrix4x4 & a, const Matrix4x4 & b);
	WWINLINE friend Matrix4x4 Multiply(const Matrix4x4 & a, const Matrix4x4 & b);
	WWINLINE friend Matrix4x4 operator * (const Matrix4x4 & a, const Matrix3D & b);
	WWINLINE friend Matrix4x4 operator * (const Matrix3D & a, const Matrix4x4 & b);

	/*
	** Comparison operators
	*/
	friend int operator == (const Matrix4x4 & a, const Matrix4x4 & b);
	friend int operator != (const Matrix4x4 & a, const Matrix4x4 & b);

	/*
	** Swap two matrices in place
	*/
	WWINLINE friend void Swap(Matrix4x4 & a,Matrix4x4 & b);

	/*
	** Linear Transforms
	*/
	WWINLINE friend Vector4 operator * (const Matrix4x4 & a, const Vector4 & v);
	WWINLINE friend Vector4 operator * (const Matrix4x4 & a, const Vector3 & v);

	/*
	** Matrix multiplication without temporaries...
	*/
	static void	Multiply(const Matrix4x4 &A,const Matrix4x4 &B,Matrix4x4 * set_result);
	static void	Multiply(const Matrix3D &A,const Matrix4x4 &B,Matrix4x4 * set_result);
	static void	Multiply(const Matrix4x4 &A,const Matrix3D &B,Matrix4x4 * set_result);

	static WWINLINE void	Transform_Vector(const Matrix4x4 & tm,const Vector3 & in,Vector3 * out);
	static WWINLINE void	Transform_Vector(const Matrix4x4 & tm,const Vector3 & in,Vector4 * out);
	static WWINLINE void	Transform_Vector(const Matrix4x4 & tm,const Vector4 & in,Vector4 * out);

protected:

	Vector4 Row[4];

};


/***********************************************************************************************
 * Matrix4x4::Matrix4x4 -- Constructor, optionally initialize to Identitiy matrix                  *
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
WWINLINE Matrix4x4::Matrix4x4(bool identity)
{
	if (identity) {
		Make_Identity();
	}
}

/***********************************************************************************************
 * Matrix4x4::Matrix4x4 -- Copy Constructor                                                        *
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
WWINLINE Matrix4x4::Matrix4x4(const Matrix4x4 & m)
{
	Row[0] = m.Row[0]; Row[1] = m.Row[1]; Row[2] = m.Row[2]; Row[3] = m.Row[3];
}

/***********************************************************************************************
 * Matrix4x4::Matrix4x4 -- Convert a Matrix3D (fake 4x4) to a Matrix4x4                              *
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
WWINLINE Matrix4x4::Matrix4x4(const Matrix3D & m)
{
	Init(m);
}

/***********************************************************************************************
 * Matrix4x4::Matrix4x4 -- Constructor                                                             *
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
WWINLINE Matrix4x4::Matrix4x4(const Vector4 & r0, const Vector4 & r1, const Vector4 & r2, const Vector4 & r3)
{
	Init(r0,r1,r2,r3);
}

/***********************************************************************************************
 * Matrix4x4::Matrix4x4 -- Constructor                                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/06/2001 NH  : Created.                                                                 *
 *=============================================================================================*/
WWINLINE Matrix4x4::Matrix4x4(float m11,float m12,float m13,float m14, float m21,float m22,float m23,float m24,
	float m31,float m32,float m33,float m34, float m41,float m42,float m43,float m44 )
{
	Init(m11, m12, m13, m14, m21, m22, m23, m24, m31, m32, m33, m34, m41, m42, m43, m44);
}

/***********************************************************************************************
 * Matrix4x4::Make_Identity -- Initializes the matrix to Identity                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/5/99    gth : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void Matrix4x4::Make_Identity(void)
{
	Row[0].Set(1.0,0.0,0.0,0.0);
	Row[1].Set(0.0,1.0,0.0,0.0);
	Row[2].Set(0.0,0.0,1.0,0.0);
	Row[3].Set(0.0,0.0,0.0,1.0);
}


/***********************************************************************************************
 * Matrix4x4::Init -- Initializes from the contents of the give Matrix3D                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/5/99    gth : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void Matrix4x4::Init(const Matrix3D & m)
{
	Row[0] = m[0]; Row[1] = m[1]; Row[2] = m[2]; Row[3] = Vector4(0.0,0.0,0.0,1.0);
}


/***********************************************************************************************
 * Matrix4x4::Init -- Initializes the rows from the given Vector4s                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/5/99    gth : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void Matrix4x4::Init(const Vector4 & r0, const Vector4 & r1, const Vector4 & r2, const Vector4 & r3)
{
	Row[0] = r0; Row[1] = r1; Row[2] = r2; Row[3] = r3;
}


/***********************************************************************************************
 * Matrix4x4::Init -- Initializes the rows from the given 16 floats                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/6/01    NH : Created.                                                                  *
 *=============================================================================================*/
WWINLINE void Matrix4x4::Init(float m11,float m12,float m13,float m14, float m21,float m22,float m23,float m24,
	float m31,float m32,float m33,float m34, float m41,float m42,float m43,float m44)
{
	Row[0].Set(m11,m12,m13,m14);
	Row[1].Set(m21,m22,m23,m24);
	Row[2].Set(m31,m32,m33,m34);
	Row[3].Set(m41,m42,m43,m44);
}


/***********************************************************************************************
 * Matrix4x4::Init_Ortho -- Initialize to an orthographic projection matrix                      *
 *                                                                                             *
 * You can find the formulas for this in the appendix of the OpenGL programming guide.  Also   *
 * this happens to be the same convention used by Surrender.                                   *
 *                                                                                             *
 * The result of this projection will be that points inside the volume will have all coords    *
 * between -1 and +1.  A point at znear will project to z=-1.  A point at zfar will project    *
 * to z=+1...                                                                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * Note that the znear and zfar parameters are positive distances to the clipping planes       *
 * even though in the camera coordinate system, the clipping planes are at negative z          *
 * coordinates.  This holds for all of the projection initializations and is consistent        *
 * with OpenGL's convention.                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/5/99    gth : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void Matrix4x4::Init_Ortho
(
	float left,
	float right,
	float bottom,
	float top,
	float znear,
	float zfar
)
{
	assert(znear >= 0.0f);
	assert(zfar > znear);

	Make_Identity();
	Row[0][0] = 2.0f / (right - left);
	Row[0][3] = -(right + left) / (right - left);
	Row[1][1] = 2.0f / (top - bottom);
	Row[1][3] = -(top + bottom) / (top - bottom);
	Row[2][2] = -2.0f / (zfar - znear);
	Row[2][3] = -(zfar + znear) / (zfar - znear);
}


/***********************************************************************************************
 * Matrix4x4::Init_Perspective -- Initialize to a perspective projection matrix                  *
 *                                                                                             *
 * You can find the formulas for this matrix in the appendix of the OpenGL programming guide.  *
 * Also, this happens to be the same convention used by Surrender.                             *
 *                                                                                             *
 * The result of this projection will be that points inside the volume will have all coords    *
 * between -1 and +1.  A point at znear will project to z=-1.  A point at zfar will project    *
 * to z=+1...                                                                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 * hfov - horizontal field of view (in radians)                                                *
 * vfov - vertical field of view (in radians)                                                  *
 * znear - distance to near z clipping plane (positive)                                        *
 * zfar - distance to the far z clipping plane (positive)                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * Note that the znear and zfar parameters are positive distances to the clipping planes       *
 * even though in the camera coordinate system, the clipping planes are at negative z          *
 * coordinates.  This holds for all of the projection initializations and is consistent        *
 * with OpenGL's convention.                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/5/99    gth : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void Matrix4x4::Init_Perspective(float hfov,float vfov,float znear,float zfar)
{
	assert(znear > 0.0f);
	assert(zfar > znear);

	Make_Identity();
	Row[0][0] = static_cast<float>(1.0 / tan(hfov*0.5));
	Row[1][1] = static_cast<float>(1.0 / tan(vfov*0.5));
	Row[2][2] = -(zfar + znear) / (zfar - znear);
	Row[2][3] = static_cast<float>(-(2.0*zfar*znear) / (zfar - znear));
	Row[3][2] = -1.0f;
	Row[3][3] = 0.0f;
}


/***********************************************************************************************
 * Matrix4x4::Init_Perspective -- Initialize to a perspective projection matrix                  *
 *                                                                                             *
 * You can find the formulas for this matrix in the appendix of the OpenGL programming guide.  *
 * Also, this happens to be the same convention used by Surrender.                             *
 *                                                                                             *
 * The result of this projection will be that points inside the volume will have all coords    *
 * between -1 and +1.  A point at znear will project to z=-1.  A point at zfar will project    *
 * to z=+1...                                                                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * left - min x coordinate of near clip plane                                                  *
 * right - max x coordinate of near clip plane                                                 *
 * bottom - min y coordinate of near clip plane                                                *
 * top - max y coordinate of near clip plane                                                   *
 * znear - distance to near Z clipping plane                                                   *
 * zfar - distance to far Z clipping plane                                                     *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * Note that the znear and zfar parameters are positive distances to the clipping planes       *
 * even though in the camera coordinate system, the clipping planes are at negative z          *
 * coordinates.  This holds for all of the projection initializations and is consistent        *
 * with OpenGL's convention.                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/5/99    gth : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void Matrix4x4::Init_Perspective
(
	float left,
	float right,
	float bottom,
	float top,
	float znear,
	float zfar
)
{
	assert(znear > 0.0f);
	assert(zfar > 0.0f);

	Make_Identity();
	Row[0][0] = static_cast<float>(2.0*znear / (right - left));
	Row[0][2] = (right + left) / (right - left);
	Row[1][1] = static_cast<float>(2.0*znear / (top - bottom));
	Row[1][2] = (top + bottom) / (top - bottom);
	Row[2][2] = -(zfar + znear) / (zfar - znear);
	Row[2][3] = static_cast<float>(-(2.0*zfar*znear) / (zfar - znear));
	Row[3][2] = -1.0f;
	Row[3][3] = 0.0f;
}

/***********************************************************************************************
 * Matrix4x4::Transpose -- Returns transpose of the matrix                                       *
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
WWINLINE Matrix4x4 Matrix4x4::Transpose() const
{
    return Matrix4x4(
			Vector4(Row[0][0], Row[1][0], Row[2][0], Row[3][0]),
			Vector4(Row[0][1], Row[1][1], Row[2][1], Row[3][1]),
			Vector4(Row[0][2], Row[1][2], Row[2][2], Row[3][2]),
			Vector4(Row[0][3], Row[1][3], Row[2][3], Row[3][3])
	);
}

/***********************************************************************************************
 * Matrix4x4::Inverse -- returns the inverse of the matrix                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/02/1997 GH  : Created.                                                                 *
 *   01/03/2026 TheSuperHackers : Implemented.                                                 *
 *=============================================================================================*/
WWINLINE Matrix4x4* Matrix4x4::Inverse(Matrix4x4* out, float* detOut, const Matrix4x4* m)
{
	// Read matrix elements
	// Uses a column-major, column-vector convention (matches D3DXMatrixInverse)
	// Row[i][j] stores the element at row i, column j.

	const float m00 = m->Row[0][0], m01 = m->Row[1][0], m02 = m->Row[2][0], m03 = m->Row[3][0];
	const float m10 = m->Row[0][1], m11 = m->Row[1][1], m12 = m->Row[2][1], m13 = m->Row[3][1];
	const float m20 = m->Row[0][2], m21 = m->Row[1][2], m22 = m->Row[2][2], m23 = m->Row[3][2];
	const float m30 = m->Row[0][3], m31 = m->Row[1][3], m32 = m->Row[2][3], m33 = m->Row[3][3];

	// Compute 2x2 determinants (minors) used for cofactors
	// s0..s5: sub-determinants of the upper-left 2x2 blocks, used in cofactor expansion

	const float s0 = m00 * m11 - m10 * m01;
	const float s1 = m00 * m12 - m10 * m02;
	const float s2 = m00 * m13 - m10 * m03;
	const float s3 = m01 * m12 - m11 * m02;
	const float s4 = m01 * m13 - m11 * m03;
	const float s5 = m02 * m13 - m12 * m03;

	// c0..c5: sub-determinants of the lower-right 2x2 blocks, used in cofactor expansion

	const float c5 = m22 * m33 - m32 * m23;
	const float c4 = m21 * m33 - m31 * m23;
	const float c3 = m21 * m32 - m31 * m22;
	const float c2 = m20 * m33 - m30 * m23;
	const float c1 = m20 * m32 - m30 * m22;
	const float c0 = m20 * m31 - m30 * m21;

	// Compute determinant of 4x4 matrix
	// Using cofactor expansion along the first row
	// If det is near zero, the matrix is singular and cannot be inverted

	const float det =
				s0 * c5 - s1 * c4 + s2 * c3
			+ s3 * c2 - s4 * c1 + s5 * c0;

	if (detOut)
			*detOut = det;

	if (fabsf(det) < 1e-8f)
			return NULL;

	const float invDet = 1.0f / det;

	// Compute inverse matrix using adjugate / determinant
	// The adjugate matrix is the transpose of the cofactor matrix
	// Multiplies each cofactor by 1/det to get the inverse
	// Writes in column-major order to match engine conventions

	out->Row[0][0] = ( m11 * c5 - m12 * c4 + m13 * c3) * invDet;
	out->Row[1][0] = (-m01 * c5 + m02 * c4 - m03 * c3) * invDet;
	out->Row[2][0] = ( m31 * s5 - m32 * s4 + m33 * s3) * invDet;
	out->Row[3][0] = (-m21 * s5 + m22 * s4 - m23 * s3) * invDet;

	out->Row[0][1] = (-m10 * c5 + m12 * c2 - m13 * c1) * invDet;
	out->Row[1][1] = ( m00 * c5 - m02 * c2 + m03 * c1) * invDet;
	out->Row[2][1] = (-m30 * s5 + m32 * s2 - m33 * s1) * invDet;
	out->Row[3][1] = ( m20 * s5 - m22 * s2 + m23 * s1) * invDet;

	out->Row[0][2] = ( m10 * c4 - m11 * c2 + m13 * c0) * invDet;
	out->Row[1][2] = (-m00 * c4 + m01 * c2 - m03 * c0) * invDet;
	out->Row[2][2] = ( m30 * s4 - m31 * s2 + m33 * s0) * invDet;
	out->Row[3][2] = (-m20 * s4 + m21 * s2 - m23 * s0) * invDet;

	out->Row[0][3] = (-m10 * c3 + m11 * c1 - m12 * c0) * invDet;
	out->Row[1][3] = ( m00 * c3 - m01 * c1 + m02 * c0) * invDet;
	out->Row[2][3] = (-m30 * s3 + m31 * s1 - m32 * s0) * invDet;
	out->Row[3][3] = ( m20 * s3 - m21 * s1 + m22 * s0) * invDet;

	return out;
}

WWINLINE Matrix4x4 Matrix4x4::Inverse() const
{
	Matrix4x4 inv;
	Inverse(&inv, NULL, this);
	return inv;
}

/***********************************************************************************************
 * Matrix4x4::operator = -- assignment operator                                                  *
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
WWINLINE Matrix4x4 & Matrix4x4::operator = (const Matrix4x4 & m)
{
	Row[0] = m.Row[0]; Row[1] = m.Row[1]; Row[2] = m.Row[2]; Row[3] = m.Row[3];
	return *this;
}

/***********************************************************************************************
 * Matrix4x4::operator += -- "plus equals" operator                                              *
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
WWINLINE Matrix4x4& Matrix4x4::operator += (const Matrix4x4 & m)
{
	Row[0] += m.Row[0]; Row[1] += m.Row[1]; Row[2] += m.Row[2]; Row[3] += m.Row[3];
	return *this;
}

/***********************************************************************************************
 * Matrix4x4::operator-= -- "minus equals" operator                                              *
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
WWINLINE Matrix4x4& Matrix4x4::operator -= (const Matrix4x4 & m)
{
	Row[0] -= m.Row[0]; Row[1] -= m.Row[1]; Row[2] -= m.Row[2]; Row[3] -= m.Row[3];
	return *this;
}

/***********************************************************************************************
 * Matrix4x4::operator *= -- "times equals" operator                                             *
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
WWINLINE Matrix4x4& Matrix4x4::operator *= (float d)
{
	Row[0] *= d; Row[1] *= d; Row[2] *= d; Row[3] *= d;
	return *this;
}

/***********************************************************************************************
 * Matrix4x4::operator /= -- "divide equals" operator                                            *
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
WWINLINE Matrix4x4& Matrix4x4::operator /= (float d)
{
	float ood = d;
	Row[0] *= ood; Row[1] *= ood; Row[2] *= ood; Row[3] *= ood;
	return *this;
}

WWINLINE Matrix4x4 operator - (const Matrix4x4 & a)
{
	return Matrix4x4(-a.Row[0], -a.Row[1], -a.Row[2], -a.Row[3]);
}

WWINLINE Matrix4x4 operator * (const Matrix4x4 & a, float d)
{
	return Matrix4x4(a.Row[0] * d, a.Row[1] * d, a.Row[2] * d, a.Row[3] * d);
}

WWINLINE Matrix4x4 operator * (float d, const Matrix4x4 & a)
{
	return a*d;
}

WWINLINE Matrix4x4 operator / (const Matrix4x4 & a, float d)
{
	float ood = 1.0f / d;
	return Matrix4x4(a.Row[0] * ood, a.Row[1] * ood, a.Row[2] * ood, a.Row[3] * ood);
}

/*
** matrix addition
*/
WWINLINE Matrix4x4 operator + (const Matrix4x4 & a, const Matrix4x4 & b)
{
	return Matrix4x4(
				a.Row[0] + b.Row[0],
				a.Row[1] + b.Row[1],
				a.Row[2] + b.Row[2],
				a.Row[3] + b.Row[3]
	);
}

WWINLINE Matrix4x4 Add(const Matrix4x4 & a, const Matrix4x4 & b)
{ return a+b; }

/*
** matrix subtraction
*/
WWINLINE Matrix4x4 operator - (const Matrix4x4 & a, const Matrix4x4 & b)
{
	return Matrix4x4(
				a.Row[0] - b.Row[0],
				a.Row[1] - b.Row[1],
				a.Row[2] - b.Row[2],
				a.Row[3] - b.Row[3]
	);
}

WWINLINE Matrix4x4 Subtract(const Matrix4x4 & a, const Matrix4x4 & b)
{ return a-b; }

/*
** matrix multiplication
*/
WWINLINE Matrix4x4 operator * (const Matrix4x4 & a, const Matrix4x4 & b)
{
	#define ROWCOL(i, j) a[i][0]*b[0][j] + a[i][1]*b[1][j] + a[i][2]*b[2][j] + a[i][3]*b[3][j]

	return Matrix4x4(
		Vector4(ROWCOL(0,0), ROWCOL(0,1), ROWCOL(0,2), ROWCOL(0,3)),
		Vector4(ROWCOL(1,0), ROWCOL(1,1), ROWCOL(1,2), ROWCOL(1,3)),
		Vector4(ROWCOL(2,0), ROWCOL(2,1), ROWCOL(2,2), ROWCOL(2,3)),
		Vector4(ROWCOL(3,0), ROWCOL(3,1), ROWCOL(3,2), ROWCOL(3,3))
	);

	#undef ROWCOL
}

WWINLINE Matrix4x4 Multiply(const Matrix4x4 & a, const Matrix4x4 & b)
{ return a*b; }

WWINLINE Matrix4x4 operator * (const Matrix4x4 & a, const Matrix3D & b)
{
	// This function hand coded to handle the last row of b as 0,0,0,1
	#define ROWCOL(i,j) a[i][0]*b[0][j] + a[i][1]*b[1][j] + a[i][2]*b[2][j]
	#define ROWCOL_LAST(i,j) a[i][0]*b[0][j] + a[i][1]*b[1][j] + a[i][2]*b[2][j] + a[i][3]
	return Matrix4x4(
		Vector4(ROWCOL(0,0), ROWCOL(0,1), ROWCOL(0,2), ROWCOL_LAST(0,3)),
		Vector4(ROWCOL(1,0), ROWCOL(1,1), ROWCOL(1,2), ROWCOL_LAST(1,3)),
		Vector4(ROWCOL(2,0), ROWCOL(2,1), ROWCOL(2,2), ROWCOL_LAST(2,3)),
		Vector4(ROWCOL(3,0), ROWCOL(3,1), ROWCOL(3,2), ROWCOL_LAST(3,3))
	);
	#undef ROWCOL
	#undef ROWCOL_LAST
}

WWINLINE Matrix4x4 operator * (const Matrix3D & a, const Matrix4x4 & b)
{
	// This function hand coded to handle the last row of a as 0,0,0,1
	#define ROWCOL(i,j) a[i][0]*b[0][j] + a[i][1]*b[1][j] + a[i][2]*b[2][j] + a[i][3]*b[3][j]
	return Matrix4x4(
		Vector4(ROWCOL(0,0), ROWCOL(0,1), ROWCOL(0,2), ROWCOL(0,3)),
		Vector4(ROWCOL(1,0), ROWCOL(1,1), ROWCOL(1,2), ROWCOL(1,3)),
		Vector4(ROWCOL(2,0), ROWCOL(2,1), ROWCOL(2,2), ROWCOL(2,3)),
		Vector4(b[3][0], b[3][1], b[3][2], b[3][3])
	);
	#undef ROWCOL
}

/*
** Multiply a Matrix4x4 by a Vector3 (assumes w=1.0!!!). Yeilds a Vector4 result
*/
WWINLINE Vector4 operator * (const Matrix4x4 & a, const Vector3 & v) {
	return Vector4(
		a[0][0] * v[0] + a[0][1] * v[1] + a[0][2] * v[2] + a[0][3] * 1.0f,
		a[1][0] * v[0] + a[1][1] * v[1] + a[1][2] * v[2] + a[1][3] * 1.0f,
		a[2][0] * v[0] + a[2][1] * v[1] + a[2][2] * v[2] + a[2][3] * 1.0f,
		a[3][0] * v[0] + a[3][1] * v[1] + a[3][2] * v[2] + a[3][3] * 1.0f
	);
}

/*
** Multiply a Matrix4x4 by a Vector4
*/
WWINLINE Vector4 operator * (const Matrix4x4 & a, const Vector4 & v) {
	return Vector4(
		a[0][0] * v[0] + a[0][1] * v[1] + a[0][2] * v[2] + a[0][3] * v[3],
		a[1][0] * v[0] + a[1][1] * v[1] + a[1][2] * v[2] + a[1][3] * v[3],
		a[2][0] * v[0] + a[2][1] * v[1] + a[2][2] * v[2] + a[2][3] * v[3],
		a[3][0] * v[0] + a[3][1] * v[1] + a[3][2] * v[2] + a[3][3] * v[3]
	);
}

/*
** Multiply a Matrix4x4 by a Vector4
*/
WWINLINE void Matrix4x4::Transform_Vector(const Matrix4x4 & A,const Vector3 & in,Vector3 * out)
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


WWINLINE void Matrix4x4::Transform_Vector(const Matrix4x4 & A,const Vector3 & in,Vector4 * out)
{
	out->X = (A[0][0] * in.X + A[0][1] * in.Y + A[0][2] * in.Z + A[0][3]);
	out->Y = (A[1][0] * in.X + A[1][1] * in.Y + A[1][2] * in.Z + A[1][3]);
	out->Z = (A[2][0] * in.X + A[2][1] * in.Y + A[2][2] * in.Z + A[2][3]);
	out->W = 1.0f;
}

WWINLINE void	Matrix4x4::Transform_Vector(const Matrix4x4 & A,const Vector4 & in,Vector4 * out)
{
	Vector4 tmp;
	Vector4 * v;

	// check for aliased parameters
	if (out == &in) {
		tmp = in;
		v = &tmp;
	} else {
		v = (Vector4 *)&in;		// whats the right way to do this...
	}

	out->X = (A[0][0] * v->X + A[0][1] * v->Y + A[0][2] * v->Z + A[0][3] * v->W);
	out->Y = (A[1][0] * v->X + A[1][1] * v->Y + A[1][2] * v->Z + A[1][3] * v->W);
	out->Z = (A[2][0] * v->X + A[2][1] * v->Y + A[2][2] * v->Z + A[2][3] * v->W);
	out->W = (A[3][0] * v->X + A[3][1] * v->Y + A[3][2] * v->Z + A[3][3] * v->W);
}


// TheSuperHackers @info Always convert Matrix4x4 to D3DMATRIX or vice versa with the conversion functions below.
// Reason being, D3DMATRIX is row-major, and Matrix4x4 is column-major and therefore copying one matrix to the
// other will always require a transpose.

struct _D3DMATRIX;
struct D3DXMATRIX;

extern void To_D3DMATRIX(_D3DMATRIX& dxm, const Matrix4x4& m);
extern _D3DMATRIX To_D3DMATRIX(const Matrix4x4& m);
extern D3DXMATRIX To_D3DXMATRIX(const Matrix4x4& m);

extern void To_Matrix4x4(Matrix4x4& m, const _D3DMATRIX& dxm);
extern Matrix4x4 To_Matrix4x4(const _D3DMATRIX& dxm);
