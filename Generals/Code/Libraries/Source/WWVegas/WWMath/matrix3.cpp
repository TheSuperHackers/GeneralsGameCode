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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WWMath                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwmath/matrix3.cpp                           $*
 *                                                                                             *
 *                   Org Author:: Greg_h                                                       *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               * 
 *                                                                                             *
 *                     $Modtime:: 06/26/02 4:04p                                             $*
 *                                                                                             *
 *                    $Revision:: 17                                                          $*
 *                                                                                             *
 * 06/26/02 KM Matrix name change to avoid MAX conflicts                                       *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "matrix3.h"
#include "matrix3d.h"
#include "matrix4.h"
#include "quat.h"


/*
** Some pre-initialized Matrix3x3's
*/
const Matrix3x3 Matrix3x3::Identity
(
	1.0,	0.0,	0.0,
	0.0,	1.0,	0.0,
	0.0,	0.0,	1.0
);

const Matrix3x3 Matrix3x3::RotateX90
(
	1.0,	0.0,	0.0,
	0.0,	0.0, -1.0,
	0.0,	1.0,	0.0
);

const Matrix3x3 Matrix3x3::RotateX180
(
	1.0,	0.0,	0.0,
	0.0, -1.0,	0.0,
	0.0,	0.0, -1.0
);

const Matrix3x3 Matrix3x3::RotateX270
(
	1.0,	0.0,	0.0,
	0.0,	0.0,	1.0,
	0.0, -1.0,	0.0
);

const Matrix3x3 Matrix3x3::RotateY90
(
	0.0,	0.0,	1.0,
	0.0,	1.0,	0.0,
  -1.0,	0.0,	0.0
);

const Matrix3x3 Matrix3x3::RotateY180
(
  -1.0,	0.0,	0.0,
	0.0,	1.0,	0.0,
	0.0,	0.0, -1.0
);

const Matrix3x3 Matrix3x3::RotateY270
(
	0.0,	0.0, -1.0,
	0.0,	1.0,	0.0,
	1.0,	0.0,	0.0
);

const Matrix3x3 Matrix3x3::RotateZ90
(
	0.0, -1.0,	0.0,
	1.0,	0.0,	0.0,
	0.0,	0.0,	1.0
);

const Matrix3x3 Matrix3x3::RotateZ180
(
  -1.0,	0.0,	0.0,
	0.0, -1.0,	0.0,
	0.0,	0.0,	1.0
);

const Matrix3x3 Matrix3x3::RotateZ270
(
	0.0,	1.0,	0.0,
  -1.0,	0.0,	0.0,
	0.0,	0.0,	1.0
);



/*********************************************************************************************** 
 * Matrix3x3::Matrix3x3 -- Convert a Matrix3D (fake 4x4) to a Matrix3x3                              * 
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
Matrix3x3::Matrix3x3(const Matrix3D & m)
{
	Row[0].Set(m[0][0],m[0][1],m[0][2]);
	Row[1].Set(m[1][0],m[1][1],m[1][2]);
	Row[2].Set(m[2][0],m[2][1],m[2][2]);
}

Matrix3x3::Matrix3x3(const Matrix4x4 & m)
{
	Row[0].Set(m[0][0],m[0][1],m[0][2]);
	Row[1].Set(m[1][0],m[1][1],m[1][2]);
	Row[2].Set(m[2][0],m[2][1],m[2][2]);
}

void Matrix3x3::Set(const Matrix3D & m)
{
	Row[0].Set(m[0][0],m[0][1],m[0][2]);
	Row[1].Set(m[1][0],m[1][1],m[1][2]);
	Row[2].Set(m[2][0],m[2][1],m[2][2]);
}

void Matrix3x3::Set(const Matrix4x4 & m)
{
	Row[0].Set(m[0][0],m[0][1],m[0][2]);
	Row[1].Set(m[1][0],m[1][1],m[1][2]);
	Row[2].Set(m[2][0],m[2][1],m[2][2]);
}

void Matrix3x3::Set(const Quaternion & q)
{
	Row[0][0] = (float)(1.0 - 2.0 * (q[1] * q[1] + q[2] * q[2]));
	Row[0][1] = (float)(2.0 * (q[0] * q[1] - q[2] * q[3]));
	Row[0][2] = (float)(2.0 * (q[2] * q[0] + q[1] * q[3]));

	Row[1][0] = (float)(2.0 * (q[0] * q[1] + q[2] * q[3]));
	Row[1][1] = (float)(1.0 - 2.0f * (q[2] * q[2] + q[0] * q[0]));
	Row[1][2] = (float)(2.0 * (q[1] * q[2] - q[0] * q[3]));

	Row[2][0] = (float)(2.0 * (q[2] * q[0] - q[1] * q[3]));
	Row[2][1] = (float)(2.0 * (q[1] * q[2] + q[0] * q[3]));
	Row[2][2] =(float)(1.0 - 2.0 * (q[1] * q[1] + q[0] * q[0]));
}


Matrix3x3 & Matrix3x3::operator = (const Matrix3D & m)
{
	Row[0].Set(m[0][0],m[0][1],m[0][2]);
	Row[1].Set(m[1][0],m[1][1],m[1][2]);
	Row[2].Set(m[2][0],m[2][1],m[2][2]);
	return *this; 
}

Matrix3x3 & Matrix3x3::operator = (const Matrix4x4 & m)
{
	Row[0].Set(m[0][0],m[0][1],m[0][2]);
	Row[1].Set(m[1][0],m[1][1],m[1][2]);
	Row[2].Set(m[2][0],m[2][1],m[2][2]);
	return *this; 
}

void Matrix3x3::Multiply(const Matrix3D & a, const Matrix3x3 & b,Matrix3x3 * res)
{
	#define ROWCOL(i,j) a[i][0]*b[0][j] + a[i][1]*b[1][j] + a[i][2]*b[2][j]
    
	(*res)[0][0] = ROWCOL(0,0);
	(*res)[0][1] = ROWCOL(0,1);
	(*res)[0][2] = ROWCOL(0,2);

	(*res)[1][0] = ROWCOL(1,0);
	(*res)[1][1] = ROWCOL(1,1);
	(*res)[1][2] = ROWCOL(1,2);

	(*res)[2][0] = ROWCOL(2,0);
	(*res)[2][1] = ROWCOL(2,1);
	(*res)[2][2] = ROWCOL(2,2);

	#undef ROWCOL
}

void Matrix3x3::Multiply(const Matrix3x3 & a, const Matrix3D & b,Matrix3x3 * res)
{
	#define ROWCOL(i,j) a[i][0]*b[0][j] + a[i][1]*b[1][j] + a[i][2]*b[2][j]
    
	(*res)[0][0] = ROWCOL(0,0);
	(*res)[0][1] = ROWCOL(0,1);
	(*res)[0][2] = ROWCOL(0,2);

	(*res)[1][0] = ROWCOL(1,0);
	(*res)[1][1] = ROWCOL(1,1);
	(*res)[1][2] = ROWCOL(1,2);

	(*res)[2][0] = ROWCOL(2,0);
	(*res)[2][1] = ROWCOL(2,1);
	(*res)[2][2] = ROWCOL(2,2);

	#undef ROWCOL
}

Matrix3x3 operator * (const Matrix3D & a, const Matrix3x3 & b)
{
	#define ROWCOL(i,j) a[i][0]*b[0][j] + a[i][1]*b[1][j] + a[i][2]*b[2][j]
    
	return Matrix3x3(
			Vector3(ROWCOL(0,0), ROWCOL(0,1), ROWCOL(0,2) ),
			Vector3(ROWCOL(1,0), ROWCOL(1,1), ROWCOL(1,2) ),
			Vector3(ROWCOL(2,0), ROWCOL(2,1), ROWCOL(2,2) )
	);
	
	#undef ROWCOL
}

Matrix3x3 operator * (const Matrix3x3 & a, const Matrix3D & b)
{
	#define ROWCOL(i,j) a[i][0]*b[0][j] + a[i][1]*b[1][j] + a[i][2]*b[2][j]
    
	return Matrix3x3(
			Vector3(ROWCOL(0,0), ROWCOL(0,1), ROWCOL(0,2) ),
			Vector3(ROWCOL(1,0), ROWCOL(1,1), ROWCOL(1,2) ),
			Vector3(ROWCOL(2,0), ROWCOL(2,1), ROWCOL(2,2) )
	);
	
	#undef ROWCOL
}


#if 0

void Matrix3x3::Compute_Jacobi_Rotation(int i,int j,Matrix3x3 * r,Matrix3x3 * rinv)
{

}

void Matrix3x3::Symmetric_Eigen_Solve(void)
{
	Matrix3x3 eigen_vals = *this;
	Matrix3x3 eigen_vecs(1);

	Matrix3x3 jr,jrinv;

	while (!done) {
		eigen_vals.Compute_Jacobi_Rotation(i,j,&jr,&jrinv);
		eigen_vals = jr * (eigenvals) * jrinv;
		eigen_vecs = eigen_vecs * jr;
	}

	/*
	** Done!  Eigen values are the diagonals of
	** the eigen_vals matrix and the eigen vectors
	** are the columns of the eigen_vecs matrix
	*/

}

#endif


void Matrix3x3::Multiply(const Matrix3x3 & A,const Matrix3x3 & B,Matrix3x3 * set_res)
{
	Matrix3x3 tmp;
	Matrix3x3 * Aptr;
	float tmp1,tmp2,tmp3;

	// Check for aliased parameters, copy the 'A' matrix into a temporary if the 
	// result is going into 'A'. (in this case, this function is no better than 
	// the overloaded C++ operator...)
	if (set_res == &A) {
		tmp = A;
		Aptr = &tmp;
	} else {
		Aptr = (Matrix3x3 *)&A;	
	}

	tmp1 = B[0][0];
	tmp2 = B[1][0];
	tmp3 = B[2][0];

	(*set_res)[0][0] = (float)((*Aptr)[0][0]*tmp1 + (*Aptr)[0][1]*tmp2 + (*Aptr)[0][2]*tmp3);
	(*set_res)[1][0] = (float)((*Aptr)[1][0]*tmp1 + (*Aptr)[1][1]*tmp2 + (*Aptr)[1][2]*tmp3);
	(*set_res)[2][0] = (float)((*Aptr)[2][0]*tmp1 + (*Aptr)[2][1]*tmp2 + (*Aptr)[2][2]*tmp3);

	tmp1 = B[0][1];
	tmp2 = B[1][1];
	tmp3 = B[2][1];

	(*set_res)[0][1] = (float)((*Aptr)[0][0]*tmp1 + (*Aptr)[0][1]*tmp2 + (*Aptr)[0][2]*tmp3);
	(*set_res)[1][1] = (float)((*Aptr)[1][0]*tmp1 + (*Aptr)[1][1]*tmp2 + (*Aptr)[1][2]*tmp3);
	(*set_res)[2][1] = (float)((*Aptr)[2][0]*tmp1 + (*Aptr)[2][1]*tmp2 + (*Aptr)[2][2]*tmp3);

	tmp1 = B[0][2];
	tmp2 = B[1][2];
	tmp3 = B[2][2];

	(*set_res)[0][2] = (float)((*Aptr)[0][0]*tmp1 + (*Aptr)[0][1]*tmp2 + (*Aptr)[0][2]*tmp3);
	(*set_res)[1][2] = (float)((*Aptr)[1][0]*tmp1 + (*Aptr)[1][1]*tmp2 + (*Aptr)[1][2]*tmp3);
	(*set_res)[2][2] = (float)((*Aptr)[2][0]*tmp1 + (*Aptr)[2][1]*tmp2 + (*Aptr)[2][2]*tmp3);
}

int Matrix3x3::Is_Orthogonal(void) const
{
	Vector3 x(Row[0].X,Row[0].Y,Row[0].Z);
	Vector3 y(Row[1].X,Row[1].Y,Row[1].Z);
	Vector3 z(Row[2].X,Row[2].Y,Row[2].Z);
	
	if (Vector3::Dot_Product(x,y) > WWMATH_EPSILON) return 0;
	if (Vector3::Dot_Product(y,z) > WWMATH_EPSILON) return 0;
	if (Vector3::Dot_Product(z,x) > WWMATH_EPSILON) return 0;

	if (WWMath::Fabs(x.Length() - 1.0f) > WWMATH_EPSILON) return 0;
	if (WWMath::Fabs(y.Length() - 1.0f) > WWMATH_EPSILON) return 0;
	if (WWMath::Fabs(z.Length() - 1.0f) > WWMATH_EPSILON) return 0;

	return 1;
}

void Matrix3x3::Re_Orthogonalize(void)
{
	Vector3 x(Row[0][0],Row[0][1],Row[0][2]);
	Vector3 y(Row[1][0],Row[1][1],Row[1][2]);
	Vector3 z;

	Vector3::Cross_Product(x,y,&z);
	Vector3::Cross_Product(z,x,&y);

	float len = x.Length();
	if (len < WWMATH_EPSILON) {
		Make_Identity();
		return;
	} else {
		x /= len;
	}

	len = y.Length();
	if (len < WWMATH_EPSILON) {
		Make_Identity();
		return;
	} else {
		y /= len;
	}

	len = z.Length();
	if (len < WWMATH_EPSILON) {
		Make_Identity();
		return;
	} else {
		z /= len;
	}

	Row[0][0] = x.X;
	Row[0][1] = x.Y;
	Row[0][2] = x.Z;

	Row[1][0] = y.X;
	Row[1][1] = y.Y;
	Row[1][2] = y.Z;
	
	Row[2][0] = z.X;
	Row[2][1] = z.Y;
	Row[2][2] = z.Z;
}

