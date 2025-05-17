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

/* $Header: /Commando/Code/Tools/max2w3d/matrix3d.cpp 39    2/03/00 4:55p Jason_a $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Voxel Technology                                             * 
 *                                                                                             * 
 *                    File Name : MATRIX3D.CPP                                                 * 
 *                                                                                             * 
 *                   Programmer : Greg Hjelstrom                                               * 
 *                                                                                             * 
 *                   Start Date : 02/24/97                                                     * 
 *                                                                                             * 
 *                  Last Update : February 28, 1997 [GH]                                       * 
 *                                                                                             * 
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   Matrix3D::Set_Rotation -- Sets the rotation part of the matrix                            *
 *   Matrix3D::Set_Rotation -- Sets the rotation part of the matrix                            *
 *   Matrix3D::Set -- Init a matrix3D from a matrix3 and a position                            *
 *   Matrix3D::Set -- Init a matrix3D from a quaternion and a position                         *
 *   Matrix3D::Get_X_Rotation -- approximates the rotation about the X axis                    * 
 *   Matrix3D::Get_Y_Rotation -- approximates the rotation about the Y axis                    * 
 *   Matrix3D::Get_Z_Rotation -- approximates the rotation about the Z axis                    * 
 *   Matrix3D::Multiply -- matrix multiplication without temporaries.                          *
 *   Matrix3D::Inverse_Rotate_Vector -- rotates a vector by the inverse of the 3x3 sub-matrix  *
 *   Matrix3D::Transform_Min_Max_AABox -- compute transformed axis-aligned box                 *
 *   Matrix3D::Transform_Center_Extent_AABox -- compute transformed axis-aligned box           *
 *   Matrix3D::Get_Inverse -- calculate the inverse of this matrix                             *
 *   Matrix3D::Get_Orthogonal_Inverse -- Returns the inverse of the matrix                     *  
 *   Matrix3D::Re_Orthogonalize -- makes this matrix orthogonal.                               *
 *   Matrix3D::Is_Orthogonal -- checks whether this matrix is orthogonal                       *
 *   Lerp - linearly interpolate matrices (orientation is slerped)                             * 
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "matrix3d.h"

#include <math.h>
#include <assert.h>
#include <stdlib.h>
//#include <stdio.h>
#include "vector3.h"
#include "WWmatrix3.h"
#include "matrix4.h"
#include "w3dquat.h"

// some static matrices which are sometimes useful
const Matrix3D Matrix3D::Identity
(
	1.0,	0.0,	0.0,	0.0,
	0.0,	1.0,	0.0,	0.0,
	0.0,	0.0,	1.0,	0.0
);

const Matrix3D Matrix3D::RotateX90
(
	1.0,	0.0,	0.0,	0.0,
	0.0,	0.0, -1.0,	0.0,
	0.0,	1.0,	0.0,	0.0
);

const Matrix3D Matrix3D::RotateX180
(
	1.0,	0.0,	0.0,	0.0,
	0.0, -1.0,	0.0,	0.0,
	0.0,	0.0, -1.0,	0.0
);

const Matrix3D Matrix3D::RotateX270
(
	1.0,	0.0,	0.0,	0.0,
	0.0,	0.0,	1.0,	0.0,
	0.0, -1.0,	0.0,	0.0
);

const Matrix3D Matrix3D::RotateY90
(
	0.0,	0.0,	1.0,	0.0,
	0.0,	1.0,	0.0,	0.0,
  -1.0,	0.0,	0.0,	0.0
);

const Matrix3D Matrix3D::RotateY180
(
  -1.0,	0.0,	0.0,	0.0,
	0.0,	1.0,	0.0,	0.0,
	0.0,	0.0, -1.0,	0.0
);

const Matrix3D Matrix3D::RotateY270
(
	0.0,	0.0, -1.0,	0.0,
	0.0,	1.0,	0.0,	0.0,
	1.0,	0.0,	0.0,	0.0
);

const Matrix3D	Matrix3D::RotateZ90
(
	0.0, -1.0,	0.0,	0.0,
	1.0,	0.0,	0.0,	0.0,
	0.0,	0.0,	1.0,	0.0
);

const Matrix3D Matrix3D::RotateZ180
(
  -1.0,	0.0,	0.0,	0.0,
	0.0, -1.0,	0.0,	0.0,
	0.0,	0.0,	1.0,	0.0
);

const Matrix3D	Matrix3D::RotateZ270
(
	0.0,	1.0,	0.0,	0.0,
  -1.0,	0.0,	0.0,	0.0,
 	0.0,	0.0,	1.0,	0.0
);


/***********************************************************************************************
 * Matrix3D::Set -- Init a matrix3D from a matrix3 and a position                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
void Matrix3D::Set(const Matrix3 & rot,const Vector3 & pos)
{
	Row[0].Set( rot[0][0], rot[0][1], rot[0][2], pos[0]);
	Row[1].Set( rot[1][0], rot[1][1], rot[1][2], pos[1]);
	Row[2].Set( rot[2][0], rot[2][1], rot[2][2], pos[2]);
}


/***********************************************************************************************
 * Matrix3D::Set -- Init a matrix3D from a quaternion and a position                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *=============================================================================================*/
void Matrix3D::Set(const Quaternion & rot,const Vector3 & pos)
{
   Set_Rotation(rot);
   Set_Translation(pos);
}


/***********************************************************************************************
 * Matrix3D::Set_Rotation -- Sets the rotation part of the matrix                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/11/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void Matrix3D::Set_Rotation(const Matrix3 & m)
{
	Row[0][0] = m[0][0];
	Row[0][1] = m[0][1];
	Row[0][2] = m[0][2];
	
	Row[1][0] = m[1][0];
	Row[1][1] = m[1][1];
	Row[1][2] = m[1][2];

	Row[2][0] = m[2][0];
	Row[2][1] = m[2][1];
	Row[2][2] = m[2][2];
}


/***********************************************************************************************
 * Matrix3D::Set_Rotation -- Sets the rotation part of the matrix                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/11/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void Matrix3D::Set_Rotation(const Quaternion & q)
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


/*********************************************************************************************** 
 * Matrix3D::Get_X_Rotation -- approximates the rotation about the X axis                      * 
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
float Matrix3D::Get_X_Rotation(void) const
{
	return atan2(Row[2][1], Row[1][1]);
}


/*********************************************************************************************** 
 * Matrix3D::Get_Y_Rotation -- approximates the rotation about the Y axis                      * 
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
float Matrix3D::Get_Y_Rotation(void) const
{
	return atan2(Row[0][2], Row[2][2]);
}


/*********************************************************************************************** 
 * Matrix3D::Get_Z_Rotation -- approximates the rotation about the Z axis                      * 
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
float Matrix3D::Get_Z_Rotation(void) const
{
	return atan2(Row[1][0], Row[0][0]);
}


/*********************************************************************************************** 
 * M3DC::Rotate_Vector -- Uses the 3x3 sub-matrix to rotate a vector                           * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *=============================================================================================*/
Vector3 Matrix3D::Rotate_Vector(const Vector3 &vect) const
{
	return Vector3(
		(Row[0][0]*vect[0] + Row[0][1]*vect[1] + Row[0][2]*vect[2]),
		(Row[1][0]*vect[0] + Row[1][1]*vect[1] + Row[1][2]*vect[2]),
		(Row[2][0]*vect[0] + Row[2][1]*vect[1] + Row[2][2]*vect[2])
	);
}


/***********************************************************************************************
 * Matrix3D::Inverse_Rotate_Vector -- rotates a vector by the inverse of the 3x3 sub-matrix    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/27/98    GTH : Created.                                                                 *
 *=============================================================================================*/
Vector3 Matrix3D::Inverse_Rotate_Vector(const Vector3 &vect) const
{
	return Vector3(
		(Row[0][0]*vect[0] + Row[1][0]*vect[1] + Row[2][0]*vect[2]),
		(Row[0][1]*vect[0] + Row[1][1]*vect[1] + Row[2][1]*vect[2]),
		(Row[0][2]*vect[0] + Row[1][2]*vect[1] + Row[2][2]*vect[2])
	);
}

/*********************************************************************************************** 
 * M3DC::Look_At -- Creates a "look at" transformation.                                        * 
 *                                                                                             * 
 *	Builds a transformation matrix which positions the origin at p,                             *
 *	points the negative z-axis towards a target t, and rolls about the z-axis                   *
 *	by the angle specified by roll.  														                 *
 *																							                          *
 *	This can be useful for creating a camera matrix, just invert 							           *
 *	the matrix after initializing it with this function...									           *
 *																							                          *
 * INPUT:                                                                                      * 
 * p - position of the coordinate system                                                       * 
 * t - target of the coordinate system                                                         * 
 * roll - roll angle (in radians)                                                              * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 * This function is written assuming the convention that the "ground" is the X-Y plane and     * 
 * Z is altitude.                                                                              *
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *=============================================================================================*/
void Matrix3D::Look_At(const Vector3 &p,const Vector3 &t,float roll)
{
	float	dx,dy,dz;	//vector from p to t
	float	len1,len2;
	float	sinp,cosp;	//sine and cosine of the pitch ("up-down" tilt about x)
	float	siny,cosy;	//sine and cosine of the yaw ("left-right"tilt about z)

	dx = (t[0] - p[0]);
	dy = (t[1] - p[1]);
	dz = (t[2] - p[2]);

	len1 = (float)WWMath::Sqrt(dx*dx + dy*dy + dz*dz);
	len2 = (float)WWMath::Sqrt(dx*dx + dy*dy);

	if (len1 != 0.0f) {
		sinp = dz/len1;
		cosp = len2/len1;
	} else {
		sinp = 0.0f;
		cosp = 1.0f;
	}

	if (len2 != 0.0f) {
		siny = dy/len2;
		cosy = dx/len2;
	} else {
		siny = 0.0f;
		cosy = 1.0f;
	}

	// init the matrix with position p and -z pointing down +x and +y up
	Row[0].X = 0.0f;	Row[0].Y = 0.0f;	Row[0].Z = -1.0f;
	Row[1].X = -1.0f;	Row[1].Y = 0.0f;	Row[1].Z = 0.0f;
	Row[2].X = 0.0f;	Row[2].Y = 1.0f;	Row[2].Z = 0.0f;

	Row[0].W = p.X;
	Row[1].W = p.Y;
	Row[2].W = p.Z;

	// Yaw rotation to make the matrix look at the projection of the target
	// into the x-y plane
	Rotate_Y(siny,cosy);

	// rotate about local x axis to pitch up to the targets position
	Rotate_X(sinp,cosp); 

	// roll about the local z axis (negate since we look down -z)
	Rotate_Z(-roll);
}


/*********************************************************************************************** 
 * M3DC::Obj_Look_At -- Commando Object "look at" transformation.                              * 
 *                                                                                             * 
 *	Builds a transformation matrix which positions the origin at p,                             *
 *	points the positive X axis towards a target t, and rolls about the X axis                   *
 *	by the angle specified by roll.  														                 *
 *																							                          *
 * The object convention used by Commando and G is Forward = +X, Left = +Y, Up = +Z.  The      *
 *	world is basically the x-y plane with z as altitude and +x is the default "forward".        *
 *																							                          *
 * INPUT:                                                                                      * 
 * p - position of the coordinate system                                                       * 
 * t - target of the coordinate system                                                         * 
 * roll - roll angle (in radians)                                                              * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *=============================================================================================*/
void Matrix3D::Obj_Look_At(const Vector3 &p,const Vector3 &t,float roll)
{
	float	dx,dy,dz;	//vector from p to t
	float	len1,len2;
	float	sinp,cosp;	//sine and cosine of the pitch ("up-down" tilt about y)
	float	siny,cosy;	//sine and cosine of the yaw ("left-right"tilt about z)

	dx = (t[0] - p[0]);
	dy = (t[1] - p[1]);
	dz = (t[2] - p[2]);

	len1 = (float)sqrt(dx*dx + dy*dy + dz*dz);
	len2 = (float)sqrt(dx*dx + dy*dy);

	if (len1 != 0.0f) {
		sinp = dz/len1;
		cosp = len2/len1;
	} else {
		sinp = 0.0f;
		cosp = 1.0f;
	}

	if (len2 != 0.0f) {
		siny = dy/len2;
		cosy = dx/len2;
	} else {
		siny = 0.0f;
		cosy = 1.0f;
	}
	
	Make_Identity();
	Translate(p);
	
	// Yaw rotation to projection of target in x-y plane
	Rotate_Z(siny,cosy);

	// Pitch rotation
	Rotate_Y(-sinp,cosp); 

	// Roll rotation
	Rotate_X(roll);
}


/***********************************************************************************************
 * Matrix3D::Get_Inverse -- calculate the inverse of this matrix                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/7/98     GTH : Created.                                                                 *
 *=============================================================================================*/
void Matrix3D::Get_Inverse(Matrix3D & inv) const
{
	// TODO: Implement the general purpose inverse function here (once we need it :-)
	Get_Orthogonal_Inverse(inv);
}

/*********************************************************************************************** 
 * Matrix3D::Get_Orthogonal_Inverse -- Returns the inverse of the matrix                       *  
 *                                                                                             * 
 *	NOTE!!! This only works if the matrix is really ORTHOGONAL!!!						              *
 *																							                          *
 ***********************************************************************************************
 * Inverting an orthogonal Matrix3D																				  *
 *																							                          *
 *  M is the original transform,																	              *
 *  R is rotation submatrix,																                    *
 *  T is translation vector in M.															                 *
 *																							                          *
 *	To build MINV																							           *
 *																							                          *
 *	R' = transpose of R  (inverse of orthogonal 3x3 matrix is transpose)							     *
 *	T' = -R'T																				                       *
 *																							                          *
 *	Build MINV with R'and T'																                    *
 *  MINV is the inverse of M																						  *
 *																							                          *
 ***********************************************************************************************
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *=============================================================================================*/
void Matrix3D::Get_Orthogonal_Inverse(Matrix3D & inv) const
{
	// Transposing the rotation submatrix
	inv.Row[0][0] = Row[0][0];
	inv.Row[0][1] = Row[1][0];
	inv.Row[0][2] = Row[2][0];

	inv.Row[1][0] = Row[0][1];
	inv.Row[1][1] = Row[1][1];
	inv.Row[1][2] = Row[2][1];

	inv.Row[2][0] = Row[0][2];
	inv.Row[2][1] = Row[1][2];
	inv.Row[2][2] = Row[2][2];

	// Now, calculate translation portion of matrix: 
	// T' = -R'T
	Vector3 trans = Get_Translation();
	trans = inv.Rotate_Vector(trans);
	trans = -trans;

	inv.Row[0][3] = trans[0];
	inv.Row[1][3] = trans[1];
	inv.Row[2][3] = trans[2];
}

/***********************************************************************************************
 *  Copy_3x3_Matrix(float *matrix) -- Copies a 3x3 (float[9]) matrix into this matrix          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/16/98    EHC : Created.                                                                 *
 *=============================================================================================*/
void Matrix3D::Copy_3x3_Matrix(float matrix[3][3]) 
{
	Row[0][0] = matrix[0][0];
	Row[0][1] = matrix[0][1];
	Row[0][2] = matrix[0][2];
	Row[0][3] = 0;
	Row[1][0] = matrix[1][0];
	Row[1][1] = matrix[1][1];
	Row[1][2] = matrix[1][2];
	Row[1][3] = 0;
	Row[2][0] = matrix[2][0];
	Row[2][1] = matrix[2][1];
	Row[2][2] = matrix[2][2];
	Row[2][3] = 0;
}



/***********************************************************************************************
 * Matrix3D::Multiply -- matrix multiplication without temporaries.                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/22/98    GTH : Created.                                                                 *
 *=============================================================================================*/
//void print_matrix(const Matrix3D & m);
void Matrix3D::Multiply(const Matrix3D & A,const Matrix3D & B,Matrix3D * set_res)
{
	assert(set_res != NULL);

	Matrix3D tmp;
	Matrix3D * Aptr;
	float tmp1,tmp2,tmp3;

	// Check for aliased parameters, copy the 'A' matrix into a temporary if the 
	// result is going into 'A'. (in this case, this function is no better than 
	// the overloaded C++ operator...)
	if (set_res == &A) {
		tmp = A;
		Aptr = &tmp;
	} else {
		Aptr = (Matrix3D *)&A;	
	}

	tmp1 = B[0][0];
	tmp2 = B[1][0];
	tmp3 = B[2][0];

	(*set_res)[0][0] = (*Aptr)[0][0]*tmp1 + (*Aptr)[0][1]*tmp2 + (*Aptr)[0][2]*tmp3;
	(*set_res)[1][0] = (*Aptr)[1][0]*tmp1 + (*Aptr)[1][1]*tmp2 + (*Aptr)[1][2]*tmp3;
	(*set_res)[2][0] = (*Aptr)[2][0]*tmp1 + (*Aptr)[2][1]*tmp2 + (*Aptr)[2][2]*tmp3;

	tmp1 = B[0][1];
	tmp2 = B[1][1];
	tmp3 = B[2][1];

	(*set_res)[0][1] = (*Aptr)[0][0]*tmp1 + (*Aptr)[0][1]*tmp2 + (*Aptr)[0][2]*tmp3;
	(*set_res)[1][1] = (*Aptr)[1][0]*tmp1 + (*Aptr)[1][1]*tmp2 + (*Aptr)[1][2]*tmp3;
	(*set_res)[2][1] = (*Aptr)[2][0]*tmp1 + (*Aptr)[2][1]*tmp2 + (*Aptr)[2][2]*tmp3;

	tmp1 = B[0][2];
	tmp2 = B[1][2];
	tmp3 = B[2][2];

	(*set_res)[0][2] = (*Aptr)[0][0]*tmp1 + (*Aptr)[0][1]*tmp2 + (*Aptr)[0][2]*tmp3;
	(*set_res)[1][2] = (*Aptr)[1][0]*tmp1 + (*Aptr)[1][1]*tmp2 + (*Aptr)[1][2]*tmp3;
	(*set_res)[2][2] = (*Aptr)[2][0]*tmp1 + (*Aptr)[2][1]*tmp2 + (*Aptr)[2][2]*tmp3;

	tmp1 = B[0][3];
	tmp2 = B[1][3];
	tmp3 = B[2][3];

	(*set_res)[0][3] = (*Aptr)[0][0]*tmp1 + (*Aptr)[0][1]*tmp2 + (*Aptr)[0][2]*tmp3 + (*Aptr)[0][3];
	(*set_res)[1][3] = (*Aptr)[1][0]*tmp1 + (*Aptr)[1][1]*tmp2 + (*Aptr)[1][2]*tmp3 + (*Aptr)[1][3];
	(*set_res)[2][3] = (*Aptr)[2][0]*tmp1 + (*Aptr)[2][1]*tmp2 + (*Aptr)[2][2]*tmp3 + (*Aptr)[2][3];

}


/***********************************************************************************************
 * Matrix3D::Transform_Min_Max_AABox -- compute transformed axis-aligned box                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/17/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void Matrix3D::Transform_Min_Max_AABox
(
	const Vector3 &		min,
	const Vector3 &		max,
	Vector3 *				set_min,
	Vector3 *				set_max
) const
{
	assert(set_min != &min);
	assert(set_max != &max);

	float tmp0,tmp1;
	
	// init the min and max to the translation of the transform
	set_min->X = set_max->X = Row[0][3];
	set_min->Y = set_max->Y = Row[1][3];
	set_min->Z = set_max->Z = Row[2][3];

	// now push them both out by the projections of the original intervals
	for (int i=0; i<3; i++) {

		for (int j=0; j<3; j++) {

			tmp0 = Row[i][j] * min[j];
			tmp1 = Row[i][j] * max[j];

			if (tmp0 < tmp1) {

				(*set_min)[i] += tmp0;
				(*set_max)[i] += tmp1;

			} else {

				(*set_min)[i] += tmp1;
				(*set_max)[i] += tmp0;

			}
		}
	}
}


/***********************************************************************************************
 * Matrix3D::Transform_Center_Extent_AABox -- compute transformed axis-aligned box             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/17/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void Matrix3D::Transform_Center_Extent_AABox
(
	const Vector3 &		center,
	const Vector3 &		extent,
	Vector3 *				set_center,
	Vector3 *				set_extent
) const
{
	assert(set_center != &center);
	assert(set_extent != &extent);

	// push each extent out to the projections of the original extents
	for (int i=0; i<3; i++) {

		// start the center out at the translation portion of the matrix
		// and the extent at zero
		(*set_center)[i] = Row[i][3];
		(*set_extent)[i] = 0.0f;

		for (int j=0; j<3; j++) {

			(*set_center)[i] += Row[i][j] * center[j];
			(*set_extent)[i] += WWMath::Fabs(Row[i][j] * extent[j]);
			
		}
	}
}


/***********************************************************************************************
 * Matrix3D::Is_Orthogonal -- checks whether this matrix is orthogonal                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/16/98    GTH : Created.                                                                 *
 *=============================================================================================*/
int Matrix3D::Is_Orthogonal(void) const
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

/***********************************************************************************************
 * Matrix3D::Re_Orthogonalize -- makes this matrix orthogonal.                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * This function is rather expensive, should only be used if you *know* numerical error is     *
 * killing you.                                                                                *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/16/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void Matrix3D::Re_Orthogonalize(void)
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
		x *= 1.0f/len;
	}

	len = y.Length();
	if (len < WWMATH_EPSILON) {
		Make_Identity();
		return;
	} else {
		y *= 1.0f/len;
	}

	len = z.Length();
	if (len < WWMATH_EPSILON) {
		Make_Identity();
		return;
	} else {
		z *= 1.0f/len;
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


/*********************************************************************************************** 
 * Lerp - linearly interpolate matrices (orientation is slerped)                               * 
 *                                                                                             * 
 * INPUT:                                                                                      * 
 *                                                                                             * 
 * OUTPUT:                                                                                     * 
 *                                                                                             * 
 * WARNINGS:                                                                                   * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   10/05/1998 NH  : Created.                                                                 * 
 *=============================================================================================*/
Matrix3D Lerp(const Matrix3D &A, const Matrix3D &B, float factor)
{
   assert(factor >= 0.0f);
   assert(factor <= 1.0f);

	// Lerp position
   Vector3 pos = Lerp(A.Get_Translation(), B.Get_Translation(), factor);
	Quaternion rot = Slerp(Build_Quaternion(A), Build_Quaternion(B), factor);
   return Matrix3D(rot, pos);
}

