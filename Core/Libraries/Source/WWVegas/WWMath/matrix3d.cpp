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

/* $Header: /Commando/Code/wwmath/matrix3d.cpp 41    5/09/01 2:38p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Voxel Technology                                             *
 *                                                                                             *
 *                    File Name : MATRIX3D.cpp                                                 *
 *                                                                                             *
 *                Org Programmer : Greg Hjelstrom                                               *
 *                                                                                             *
 *                   Programmer : Kenny Mitchell                          *
 *                                                                                             *
 *                   Start Date : 02/24/97                                                     *
 *                                                                         *
 *                  Last Update : June 6, 2002 [KM]                                            *
 *                                                                                             *
 * 06/26/02 KM Matrix name change to avoid MAX conflicts                                       *
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
 *   Matrix3D::Solve_Linear_System -- 3x3 Gauss-Jordan elimination                             *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "matrix3d.h"

#include <math.h>
#include <assert.h>
#include <stdlib.h>
#include "vector3.h"
#include "matrix3.h"
#include "matrix4.h"
#include "quat.h"

#include "WWLib/win.h"
#include <d3d8types.h>
#include <d3dx8math.h>

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
void Matrix3D::Set(const Matrix3x3 & rot,const Vector3 & pos)
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
void Matrix3D::Set_Rotation(const Matrix3x3 & m)
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
	return WWMath::Atan2(Row[2][1], Row[1][1]);
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
	return WWMath::Atan2(Row[0][2], Row[2][2]);
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
	return WWMath::Atan2(Row[1][0], Row[0][0]);
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

// Create a matrix given a position and a direction (x axis will point in direction)
// Make sure you pass in UNITIZED direction!!!
void Matrix3D::buildTransformMatrix( const Vector3 &pos, const Vector3 &dir )
{
	float sinp, cosp;	// sine and cosine of the pitch ("up-down" tilt about y)
	float siny, cosy;	// sine and cosine of the yaw ("left-right"tilt about z)

	float len2 = (float)sqrt( (dir.X * dir.X) + (dir.Y * dir.Y) );

	sinp = dir.Z;
	cosp = len2;

	if( len2 != 0.0f )
	{
		siny = dir.Y / len2;
		cosy = dir.X / len2;
	}
	else
	{
		siny = 0.0f;
		cosy = 1.0f;
	}

	Make_Identity();
	Translate( pos );

	// Yaw rotation to projection of target in x-y plane
	Rotate_Z( siny, cosy );

	// Pitch rotation
	Rotate_Y( -sinp, cosp );
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
 *   01/03/2026 TheSuperHackers : Implemented.                                                 *
 *=============================================================================================*/
Matrix3D * Matrix3D::Get_Inverse(Matrix3D * out, float * detOut, const Matrix3D * m)
{
	// Read linear + translation elements

	const float m00 = m->Row[0][0], m01 = m->Row[1][0], m02 = m->Row[2][0];
	const float m10 = m->Row[0][1], m11 = m->Row[1][1], m12 = m->Row[2][1];
	const float m20 = m->Row[0][2], m21 = m->Row[1][2], m22 = m->Row[2][2];

	const float tx = m->Row[0][3];
	const float ty = m->Row[1][3];
	const float tz = m->Row[2][3];

	// Compute 2x2 sub-determinants (minors) for cofactor expansion
	// These correspond to minors of the 4x4 extended matrix (with last row 0,0,0,1)

	const float s0 = m00 * m11 - m10 * m01;
	const float s1 = m00 * m12 - m10 * m02;
	const float s3 = m01 * m12 - m11 * m02;

	const float c5 = m22;
	const float c4 = m21;
	const float c2 = m20;

	const float c3 = m21 * tz - ty * m22;
	const float c1 = m20 * tz - tx * m22;
	const float c0 = m20 * ty - tx * m21;

	// Compute determinant (matches 4x4 extended determinant)

	const float det = s0 * c5 - s1 * c4 + s3 * c2;

	if (detOut)
			*detOut = det;

	if (fabsf(det) < 1e-8f)
			return NULL;

	const float invDet = 1.0f / det;

	// Compute inverse using adjugate / determinant
	// Adjugate = transpose of cofactor matrix
	// Multiplies each cofactor by 1/det to get the inverse
	// Writes in column-major order to match engine conventions

	out->Row[0][0] = ( m11 * c5 - m12 * c4) * invDet;
	out->Row[1][0] = (-m01 * c5 + m02 * c4) * invDet;
	out->Row[2][0] = (                  s3) * invDet;

	out->Row[0][1] = (-m10 * c5 + m12 * c2) * invDet;
	out->Row[1][1] = ( m00 * c5 - m02 * c2) * invDet;
	out->Row[2][1] = (                 -s1) * invDet;

	out->Row[0][2] = ( m10 * c4 - m11 * c2) * invDet;
	out->Row[1][2] = (-m00 * c4 + m01 * c2) * invDet;
	out->Row[2][2] = (                  s0) * invDet;

	// Translation (still from 4x4 cofactors)

	out->Row[0][3] = (-m10 * c3 + m11 * c1 - m12 * c0) * invDet;
	out->Row[1][3] = ( m00 * c3 - m01 * c1 + m02 * c0) * invDet;
	out->Row[2][3] = (-tx  * s3 + ty  * s1 - tz  * s0) * invDet;

	return out;
}

void Matrix3D::Get_Inverse(Matrix3D & inv) const
{
	Get_Inverse(&inv, NULL, this);
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
	assert(set_res != nullptr);

	Matrix3D tmp;
	const Matrix3D * Aptr;

	// Check for aliased parameters, copy the 'A' matrix into a temporary if the
	// result is going into 'A'. (in this case, this function is no better than
	// the overloaded C++ operator...)
	if (set_res == &A) {
		tmp = A;
		Aptr = &tmp;
	} else {
		Aptr = &A;
	}

#ifdef ALLOW_TEMPORARIES
	float tmp1,tmp2,tmp3;

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
#else
	set_res->mul(*Aptr, B);
#endif
}

#if 0
void Matrix3D::Multiply(const Matrix3D & A,const Matrix3D & B,Matrix3D * set_res)
{
	assert(set_res != nullptr);

	float tmp[12];
// Check for aliased parameters, copy the 'A' matrix into a temporary if the
// result is going into 'A'. (in this case, this function is no better than
// the overloaded C++ operator...)

/*	if (set_res == &A)
	{
		Matrix3D tmp;
		Matrix3D * Aptr;
		float tmp1,tmp2,tmp3;
		tmp = A;
		Aptr = &tmp;

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

		return;
	}
*/

	__asm {
		mov	ecx,B
		fld	dword ptr [ecx+32]	// B[2][0]
		mov	edx,A
		lea	ebx,tmp
		mov	eax,set_res
		cmp	eax,edx
		jne	not_equal
		mov	eax,ebx
not_equal:
		fld	dword ptr [ecx+16]	//	B[1][0]
		fld	dword ptr [ecx]		//	B[0][0]

//		tmp1 = B[0][0];
//		tmp2 = B[1][0];
//		tmp3 = B[2][0];

//		(*set_res)[0][0] = (*Aptr)[0][0]*tmp1 + (*Aptr)[0][1]*tmp2 + (*Aptr)[0][2]*tmp3;
		fld			dword ptr [edx+8]		// A[0][2]
		fmul			st(0),st(3)	// A[0][2] * B[2][0]
		fld			dword ptr [edx+4]		// A[0][1]
		fmul			st(0),st(3)	// A[0][1] * B[1][0]
		fld			dword ptr [edx]			// A[0][0]
		fmul			st(0),st(3)	// A[0][2] * B[0][0]
		faddp			st(1),st(0)
		faddp			st(1),st(0)
		fstp			dword ptr [eax]			// res[0][0]

//		(*set_res)[1][0] = A[1][0]*tmp1 + A[1][1]*tmp2 + A[1][2]*tmp3;
		fld			dword ptr [edx+24]		// A[1][2]
		fmul			st(0),st(3)	// A[1][2] * B[2][0]
		fld			dword ptr [edx+20]		//	A[1][1]
		fmul			st(0),st(3)	// A[1][1] * B[1][0]
		fld			dword ptr [edx+16]		//	A[1][0]
		fmul			st(0),st(3)	// A[1][0] * B[0][0]
		faddp			st(1),st(0)
		faddp			st(1),st(0)
		fstp			dword ptr [eax+16]		//	res[1][0]

//		(*set_res)[2][0] = A[2][0]*tmp1 + A[2][1]*tmp2 + A[2][2]*tmp3;

		fld			dword ptr [edx+40]		//	A[2][2]
		fmul			st(0),st(3)	// A[2][2] * B[2][0]
		fld			dword ptr [edx+36]		//	A[2][1]
		fmul			st(0),st(3)	// A[2][1] * B[1][0]
		fld			dword ptr [edx+32]		// A[2][0]
		fmul			st(0),st(3)	// A[2][0] * B[0][0]
		faddp			st(1),st(0)
		faddp			st(1),st(0)
		fstp			dword ptr [eax+32]		//	res[2][0]

//		tmp1 = B[0][1];
//		tmp2 = B[1][1];
//		tmp3 = B[2][1];
		fstp			st(0)
		fstp			st(0)
		fstp			st(0)
		fld			dword ptr [ecx+36]			//	B[2][1]
		fld			dword ptr [ecx+20]			//	B[1][1]
		fld			dword ptr [ecx+4]			//	B[0][1]

//		(*set_res)[0][1] = (*Aptr)[0][0]*tmp1 + (*Aptr)[0][1]*tmp2 + (*Aptr)[0][2]*tmp3;
		fld			dword ptr [edx+8]			//	A[0][2]
		fmul			st(0),st(3)		// A[0][2] * B[2][1]
		fld			dword ptr [edx+4]			//	A[0][1]
		fmul			st(0),st(3)		// A[0][1] * B[1][1]
		fld			dword ptr [edx]				//	A[0][0]
		fmul			st(0),st(3)		// A[0][2] * B[0][1]
		faddp			st(1),st(0)
		faddp			st(1),st(0)
		fstp			dword ptr [eax+4]			//	res[0][1]

//		(*set_res)[1][1] = A[1][0]*tmp1 + A[1][1]*tmp2 + A[1][2]*tmp3;
		fld			dword ptr [edx+24]			//	A[1][2]
		fmul			st(0),st(3)		// A[1][2] * B[2][1]
		fld			dword ptr [edx+20]			//	A[1][1]
		fmul			st(0),st(3)		// A[1][1] * B[1][1]
		fld			dword ptr [edx+16]			//	A[1][0]
		fmul			st(0),st(3)		// A[1][0] * B[0][1]
		faddp			st(1),st(0)
		faddp			st(1),st(0)
		fstp			dword ptr [eax+20]			//	res[1][1]

//		(*set_res)[2][1] = A[2][0]*tmp1 + A[2][1]*tmp2 + A[2][2]*tmp3;

		fld			dword ptr [edx+40]			//	A[2][2]
		fmul			st(0),st(3)		// A[2][2] * B[2][1]
		fld			dword ptr [edx+36]			//	A[2][1]
		fmul			st(0),st(3)		// A[2][1] * B[1][1]
		fld			dword ptr [edx+32]			//	A[2][0]
		fmul			st(0),st(3)		// A[2][0] * B[0][1]
		faddp			st(1),st(0)
		faddp			st(1),st(0)
		fstp			dword ptr [eax+36]			//	res[2][1]

//		tmp1 = B[0][2];
//		tmp2 = B[1][2];
//		tmp3 = B[2][2];
		fstp			st(0)
		fstp			st(0)
		fstp			st(0)
		fld			dword ptr [ecx+40]			//	B[2][2]
		fld			dword ptr [ecx+24]			//	B[1][2]
		fld			dword ptr [ecx+8]			//	B[0][2]

//		(*set_res)[0][2] = (*Aptr)[0][0]*tmp1 + (*Aptr)[0][1]*tmp2 + (*Aptr)[0][2]*tmp3;
		fld			dword ptr [edx+8]			//	A[0][2]
		fmul			st(0),st(3)		// A[0][2] * B[2][2]
		fld			dword ptr [edx+4]			//	A[0][1]
		fmul			st(0),st(3)		// A[0][1] * B[1][2]
		fld			dword ptr [edx]				//	A[0][0]
		fmul			st(0),st(3)		// A[0][2] * B[0][2]
		faddp			st(1),st(0)
		faddp			st(1),st(0)
		fstp			dword ptr [eax+8]			//	res[0][2]

//		(*set_res)[1][2] = A[1][0]*tmp1 + A[1][1]*tmp2 + A[1][2]*tmp3;
		fld			dword ptr [edx+24]			//	A[1][2]
		fmul			st(0),st(3)		// A[1][2] * B[2][2]
		fld			dword ptr [edx+20]			//	A[1][1]
		fmul			st(0),st(3)		// A[1][1] * B[1][2]
		fld			dword ptr [edx+16]			//	A[1][0]
		fmul			st(0),st(3)		// A[1][0] * B[0][2]
		faddp			st(1),st(0)
		faddp			st(1),st(0)
		fstp			dword ptr [eax+24]			//	res[1][2]

//		(*set_res)[2][2] = A[2][0]*tmp1 + A[2][1]*tmp2 + A[2][2]*tmp3;

		fld			dword ptr [edx+40]			//	A[2][2]
		fmul			st(0),st(3)		// A[2][2] * B[2][2]
		fld			dword ptr [edx+36]			//	A[2][1]
		fmul			st(0),st(3)		// A[2][1] * B[1][2]
		fld			dword ptr [edx+32]			//	A[2][0]
		fmul			st(0),st(3)		// A[2][0] * B[0][2]
		faddp			st(1),st(0)
		faddp			st(1),st(0)
		fstp			dword ptr [eax+40]			//	res[2][2]

// -------------------------------
//		tmp1 = B[0][3];
//		tmp2 = B[1][3];
//		tmp3 = B[2][3];
		fstp			st(0)
		fstp			st(0)
		fstp			st(0)
		fld			dword ptr [ecx+44]			//	B[2][3]
		fld			dword ptr [ecx+28]			//	B[1][3]
		fld			dword ptr [ecx+12]			//	B[0][3]

//		(*set_res)[0][3] = A[0][0]*tmp1 + A[0][1]*tmp2 + A[0][2]*tmp3 + A[0][3];
		fld			dword ptr [edx+8]			//	A[0][2]
		fmul			st(0),st(3)		// A[0][2] * B[2][3]
		fld			dword ptr [edx+4]			//	A[0][1]
		fmul			st(0),st(3)		// A[0][1] * B[1][3]
		fld			dword ptr [edx]				//	A[0][0]
		fmul			st(0),st(3)		// A[0][2] * B[0][3]
		faddp			st(1),st(0)
		faddp			st(1),st(0)
		fadd			dword ptr [edx+12]			// + A[0][3]
		fstp			dword ptr [eax+12]			//	res[0][3]

//		(*set_res)[1][3] = A[1][0]*tmp1 + A[1][1]*tmp2 + A[1][2]*tmp3 + A[1][3];
		fld			dword ptr [edx+24]			//	A[1][2]
		fmul			st(0),st(3)		// A[1][2] * B[2][3]
		fld			dword ptr [edx+20]			//	A[1][1]
		fmul			st(0),st(3)		// A[1][1] * B[1][3]
		fld			dword ptr [edx+16]			//	A[1][0]
		fmul			st(0),st(3)		// A[1][0] * B[0][3]
		faddp			st(1),st(0)
		faddp			st(1),st(0)
		fadd			dword ptr [edx+28]			//	+ A[1][3]
		fstp			dword ptr [eax+28]			//	res[1][3]

//		(*set_res)[2][3] = A[2][0]*tmp1 + A[2][1]*tmp2 + A[2][2]*tmp3 + A[2][3];

		fld			dword ptr [edx+40]			//	A[2][2]
		fmul			st(0),st(3)		// A[2][2] * B[2][3]
		fld			dword ptr [edx+36]			//	A[2][1]
		fmul			st(0),st(3)		// A[2][1] * B[1][3]
		fld			dword ptr [edx+32]			//	A[2][0]
		fmul			st(0),st(3)		// A[2][0] * B[0][3]
		faddp			st(1),st(0)
		faddp			st(1),st(0)
		fadd			dword ptr [edx+44]			//	+ A[2][3]
		fstp			dword ptr [eax+44]			//	res[2][3]
		fstp			st(0)
		fstp			st(0)
		fstp			st(0)

		cmp	eax,ebx
		jne	not_equal2
		mov	ecx,12		// copy 12 dwords
		mov	esi,eax		// set source to tmp
		mov	edi,set_res	// set destination to set_res
		rep	movsd			// copy
not_equal2:

	}
/*
	WWDEBUG_SAY(("{%2.2f, %2.2f, %2.2f, %2.2f}, {%2.2f, %2.2f, %2.2f, %2.2f}, {%2.2f, %2.2f, %2.2f, %2.2f}"
		,res[0][0],res[0][1],res[0][2],res[0][3]
		,res[1][0],res[1][1],res[1][2],res[1][3]
		,res[2][0],res[2][1],res[2][2],res[2][3]));
	WWDEBUG_SAY(("{%2.2f, %2.2f, %2.2f, %2.2f}, {%2.2f, %2.2f, %2.2f, %2.2f}, {%2.2f, %2.2f, %2.2f, %2.2f}"
		,res2[0][0],res2[0][1],res2[0][2],res2[0][3]
		,res2[1][0],res2[1][1],res2[1][2],res2[1][3]
		,res2[2][0],res2[2][1],res2[2][2],res2[2][3]));
//	res[2][3]=res2[2][3];
//	res=res2;
*/
/*	for (int y=0;y<3;++y) {
		for (int x=0;x<4;++x) {
			if (fabs(res2[y][x]-res[y][x])>0.001f) {
				WWDEBUG_SAY(("x: %d, y: %d, %f != %f",x,y,res2[y][x],res[y][x]));
				__asm nop
			}
		}
	}
*/
/*	WWASSERT(res2[0][0]==res[0][0]);
	WWASSERT(res2[0][1]==res[0][1]);
	WWASSERT(res2[0][2]==res[0][2]);
	WWASSERT(res2[0][3]==res[0][3]);
	WWASSERT(res2[1][0]==res[1][0]);
	WWASSERT(res2[1][1]==res[1][1]);
	WWASSERT(res2[1][2]==res[1][2]);
	WWASSERT(res2[1][3]==res[1][3]);
	WWASSERT(res2[2][0]==res[2][0]);
	WWASSERT(res2[2][1]==res[2][1]);
	WWASSERT(res2[2][2]==res[2][2]);
	WWASSERT(res2[2][3]==res[2][3]);
*/
}
#endif

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
	WWASSERT(set_min != &min);
	WWASSERT(set_max != &max);

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
	WWASSERT(set_center != &center);
	WWASSERT(set_extent != &extent);

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

	if (WWMath::Fabs(x.Length2() - 1.0f) > WWMATH_EPSILON) return 0;
	if (WWMath::Fabs(y.Length2() - 1.0f) > WWMATH_EPSILON) return 0;
	if (WWMath::Fabs(z.Length2() - 1.0f) > WWMATH_EPSILON) return 0;

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
void Matrix3D::Lerp(const Matrix3D &A, const Matrix3D &B, float factor, Matrix3D& result)
{
   assert(factor >= 0.0f);
   assert(factor <= 1.0f);

	// Lerp position
#ifdef ALLOW_TEMPORARIES
  Vector3 pos = Vector3::Lerp(A.Get_Translation(), B.Get_Translation(), factor);
#else
	Vector3 pos;
	Vector3::Lerp(A.Get_Translation(), B.Get_Translation(), factor, &pos);
#endif
	Quaternion rot;
	Slerp(rot,Build_Quaternion(A), Build_Quaternion(B), factor);
	result.Set(rot, pos);
}


/***********************************************************************************************
 * Matrix3D::Solve_Linear_System -- 3x3 Gauss-Jordan elimination                               *
 *                                                                                             *
 *    The result of this function will be that the 3x3 sub-matrix is inverted and the          *
 *    final column will contain the result.  False is returned if the system cannot be solved. *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/18/2001  gth : Created.                                                                 *
 *=============================================================================================*/
bool Matrix3D::Solve_Linear_System(Matrix3D & system)
{
	/*
	** Gauss-Jordan Elimination
	** We repeatedly replace rows in the matrix with a linear combination of itself and
	** another row in the system in order to reduce the matrix to the identity matrix.
	** TODO: optimize away all unnecessary math operations!
	*/
	if (system[0][0] == 0.0f) return false;
	system[0] *= 1.0f / system[0][0];				// (0,0) now equals 1.0  (row,col)
	system[1] -= system[1][0] * system[0];			// (1,0) now equals 0.0
	system[2] -= system[2][0] * system[0];			// (2,0) now equals 0.0

	if (system[1][1] == 0.0f) return false;
	system[1] *= 1.0f / system[1][1];				// (1,1) now equals 1.0
	system[2] -= system[2][1] * system[1];			// (2,1) now equals 0.0

	if (system[2][2] == 0.0f) return false;
	system[2] *= 1.0f / system[2][2];				// (2,2) now equals 1.0, and we already have one answer

	system[1] -= system[1][2] * system[2];			// (1,2) now equals 0.0, and we have another answer
	system[0] -= system[0][2] * system[2];			// (0,2) now equals 0.0

	system[0] -= system[0][1] * system[1];			// (0,1) now equals 0.0, and we are done!

	return true;
}


void To_D3DMATRIX(_D3DMATRIX& dxm, const Matrix3D& m)
{
	dxm.m[0][0] = m[0][0];
	dxm.m[0][1] = m[1][0];
	dxm.m[0][2] = m[2][0];
	dxm.m[0][3] = 0.0f;

	dxm.m[1][0] = m[0][1];
	dxm.m[1][1] = m[1][1];
	dxm.m[1][2] = m[2][1];
	dxm.m[1][3] = 0.0f;

	dxm.m[2][0] = m[0][2];
	dxm.m[2][1] = m[1][2];
	dxm.m[2][2] = m[2][2];
	dxm.m[2][3] = 0.0f;

	dxm.m[3][0] = m[0][3];
	dxm.m[3][1] = m[1][3];
	dxm.m[3][2] = m[2][3];
	dxm.m[3][3] = 1.0f;
}

_D3DMATRIX To_D3DMATRIX(const Matrix3D& m)
{
	_D3DMATRIX dxm;
	To_D3DMATRIX(dxm, m);
	return dxm;
}

D3DXMATRIX To_D3DXMATRIX(const Matrix3D& m)
{
	D3DXMATRIX dxm;
	To_D3DMATRIX(dxm, m);
	return dxm;
}
