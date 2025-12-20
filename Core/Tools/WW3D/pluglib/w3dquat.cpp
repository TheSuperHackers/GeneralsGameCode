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

/* $Header: /Commando/Code/Tools/max2w3d/w3dquat.cpp 29    2/03/00 4:55p Jason_a $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Voxel Technology                                             *
 *                                                                                             *
 *                    File Name : QUAT.cpp                                                     *
 *                                                                                             *
 *                   Programmer : Greg Hjelstrom                                               *
 *                                                                                             *
 *                   Start Date : 02/24/97                                                     *
 *                                                                                             *
 *                  Last Update : February 28, 1997 [GH]                                       *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   Quaternion::Quaternion -- constructor                                                     *
 *   Quaternion::Set -- Set the quaternion                                                     *
 *   Quaternion::operator= -- Assignment operator                                              *
 *   Quaternion::Make_Closest -- Use nearest representation to the given quaternion.           *
 *   Trackball -- Computes a "trackball" quaternion given 2D mouse coordinates                 *
 *   Axis_To_Quat -- Creates a quaternion given an axis and angle of rotation                  *
 *   Slerp -- Spherical Linear interpolation!                                                  *
 *   Build_Quaternion -- Creates a quaternion from a Matrix                                    *
 *   Build_Matrix -- Creates a Matrix from a Quaternion                                        *
 *   Normalize -- normalizes a quaternion                                                      *
 *   Quaternion::Quaternion -- constructor                                                     *
 *   Slerp_Setup -- Get ready to call "Cached_Slerp"                                           *
 *   Cached_Slerp -- Quaternion slerping, optimized with cached values                         *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "w3dquat.h"
#include "matrix3d.h"
#include "matrix4.h"
#include "wwmath.h"

#include <stdio.h>
//#include <iostream.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#define SLERP_EPSILON		0.001

static int _nxt[3] = { 1 , 2 , 0 };


// ------------------------------------------------------------
// 	local functions
// ------------------------------------------------------------
static float project_to_sphere(float,float,float);


/***********************************************************************************************
 * Quaternion::Quaternion -- constructor                                                       *
 *                                                                                             *
 * constructs a quaternion from the given axis and angle of rotation (in RADIANS of course)    *
 *                                                                                             *
 * INPUT:                                                                                      *
 * axis - axis of the rotation                                                                 *
 * angle - rotation angle                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/10/97   GTH : Created.                                                                 *
 *=============================================================================================*/
Quaternion::Quaternion(const Vector3 & axis,float angle)
{
	float s = sinf(angle/2);
	float c = cosf(angle/2);
	X = s * axis.X;
	Y = s * axis.Y;
	Z = s * axis.Z;
	W = c;
}


/***********************************************************************************************
 * Quaternion::Normalize -- Normalize to a unit quaternion                                     *
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
void Quaternion::Normalize()
{
	float mag = WWMath::Sqrt(X * X + Y * Y + Z * Z + W * W);

	if (0.0f == mag) {
		return;
	} else {
		X /= mag;
		Y /= mag;
		Z /= mag;
		W /= mag;
	}
}

/***********************************************************************************************
 * Quaternion::operator= -- Assignment operator                                                *
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
Quaternion & Quaternion::operator = (const Quaternion & source)
{
  X = source[0];
  Y = source[1];
  Z = source[2];
  W = source[3];

  return *this;
}

/***********************************************************************************************
 * Q::Make_Closest -- Use nearest representation to the given quaternion.                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/28/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
Quaternion & Quaternion::Make_Closest(const Quaternion & qto)
{
	float cos_t = qto.X * X + qto.Y * Y + qto.Z * Z + qto.W * W;

	// if we are on opposite hemisphere from qto, negate ourselves
	if (cos_t < 0.0) {
		X = -X;
		Y = -Y;
		Z = -Z;
		W = -W;
	}

	return *this;
}

/***********************************************************************************************
 * Trackball -- Computes a "trackball" quaternion given 2D mouse coordinates                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 * x0,y0 - x1,y1 - "normalized" mouse coordinates for the mouse movement                       *
 * sphsize - size of the trackball sphere                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 * a quaternion representing the rotation of a trackball                                       *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/28/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
Quaternion Trackball(float x0, float y0, float x1, float y1, float sphsize)
{
	Vector3	a;
	Vector3	p1;
	Vector3	p2;
	Vector3	d;

	float phi,t;

	if ((x0 == x1) && (y0 == y1)) {
		return Quaternion(0.0f, 0.0f, 0.0f, 1.0f);	// Zero rotation
	}


	// Compute z coordinates for projection of p1 and p2 to
	// deformed sphere
	p1[0] = x0;
	p1[1] = y0;
	p1[2] = project_to_sphere(sphsize, x0, y0);

	p2[0] = x1;
	p2[1] = y1;
	p2[2] = project_to_sphere(sphsize, x1, y1);


	// Find their cross product
	Vector3::Cross_Product(p2,p1,&a);

	// Compute how much to rotate
	d = p1 - p2;
	t = d.Length() / (2.0f * sphsize);

	// Avoid problems with out of control values
	if (t >  1.0f) t =  1.0f;
	if (t < -1.0f) t = -1.0f;
	phi = 2.0f * asin(t);

	return Axis_To_Quat(a, phi);
}


/***********************************************************************************************
 * Axis_To_Quat -- Creates a quaternion given an axis and angle of rotation                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/28/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
Quaternion Axis_To_Quat(const Vector3 &a, float phi)
{
	Quaternion q;
	Vector3 tmp = a;

	tmp.Normalize();
	q[0] = tmp[0];
	q[1] = tmp[1];
	q[2] = tmp[2];

	q.Scale(sinf(phi / 2.0f));
	q[3] =  cosf(phi / 2.0f);

	return q;
}

/***********************************************************************************************
 * Slerp -- Spherical Linear interpolation!                                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *  p - start quaternion                                                                       *
 *  q - end quaternion                                                                         *
 *  alpha - interpolating parameter                                                            *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/28/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
Quaternion Slerp(const Quaternion & p,const Quaternion & q,float alpha)
{
	float beta;				// complementary interploation parameter
	float theta;				// angle between p and q
	float sin_t,cos_t; 		// sine, cosine of theta
	float oo_sin_t;
	int qflip;					// use flip of q?

	// cos theta = dot product of p and q
	cos_t = p.X * q.X + p.Y * q.Y + p.Z * q.Z + p.W * q.W;

	// if q is on opposite hemisphere from A, use -B instead
	if (cos_t < 0.0) {
		cos_t = -cos_t;
		qflip = true;
	} else {
		qflip = false;
	}

	if (1.0 - cos_t < SLERP_EPSILON) {

		// if q is very close to p, just linearly interpolate
		// between the two.
		beta = 1.0 - alpha;

	} else {

		// normal slerp!
		theta = acos(cos_t);
		sin_t = sinf(theta);
		oo_sin_t = 1.0 / sin_t;
		beta = sinf(theta - alpha*theta) * oo_sin_t;
		alpha = sinf(alpha*theta) * oo_sin_t;
	}

	if (qflip) {
		alpha = -alpha;
	}

	Quaternion res;
	res.X = beta*p.X + alpha*q.X;
	res.Y = beta*p.Y + alpha*q.Y;
	res.Z = beta*p.Z + alpha*q.Z;
	res.W = beta*p.W + alpha*q.W;

	return res;
}


/***********************************************************************************************
 * Slerp_Setup -- Get ready to call "Cached_Slerp"                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/27/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void Slerp_Setup(const Quaternion & p,const Quaternion & q,SlerpInfoStruct * slerpinfo)
{
	float cos_t;

	assert(slerpinfo != NULL);

	// cos theta = dot product of p and q
	cos_t = p.X * q.X + p.Y * q.Y + p.Z * q.Z + p.W * q.W;

	// if q is on opposite hemisphere from A, use -B instead
	if (cos_t < 0.0) {
		cos_t = -cos_t;
		slerpinfo->Flip = true;
	} else {
		slerpinfo->Flip = false;
	}

	if (1.0 - cos_t < SLERP_EPSILON) {

		slerpinfo->Linear = true;
		slerpinfo->Theta = 0.0f;
		slerpinfo->SinT = 0.0f;

	} else {

		slerpinfo->Linear = false;
		slerpinfo->Theta = acos(cos_t);
		slerpinfo->SinT = sinf(slerpinfo->Theta);

	}
}

/***********************************************************************************************
 * Cached_Slerp -- Quaternion slerping, optimized with cached values                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/27/98    GTH : Created.                                                                 *
 *=============================================================================================*/
Quaternion Cached_Slerp(const Quaternion & p,const Quaternion & q,float alpha,SlerpInfoStruct * slerpinfo)
{
	float beta;				// complementary interploation parameter
	float oo_sin_t;

	if (slerpinfo->Linear) {

		// if q is very close to p, just linearly interpolate
		// between the two.
		beta = 1.0 - alpha;

	} else {

		// normal slerp!
		oo_sin_t = 1.0 / slerpinfo->Theta;
		beta = sin(slerpinfo->Theta - alpha*slerpinfo->Theta) * oo_sin_t;
		alpha = sin(alpha*slerpinfo->Theta) * oo_sin_t;
	}

	if (slerpinfo->Flip) {
		alpha = -alpha;
	}

	Quaternion res;
	res.X = beta*p.X + alpha*q.X;
	res.Y = beta*p.Y + alpha*q.Y;
	res.Z = beta*p.Z + alpha*q.Z;
	res.W = beta*p.W + alpha*q.W;

	return res;
}

void Cached_Slerp(const Quaternion & p,const Quaternion & q,float alpha,SlerpInfoStruct * slerpinfo,Quaternion * set_q)
{
	float beta;				// complementary interploation parameter
	float oo_sin_t;

	if (slerpinfo->Linear) {

		// if q is very close to p, just linearly interpolate
		// between the two.
		beta = 1.0 - alpha;

	} else {

		// normal slerp!
		oo_sin_t = 1.0 / slerpinfo->Theta;
		beta = sin(slerpinfo->Theta - alpha*slerpinfo->Theta) * oo_sin_t;
		alpha = sin(alpha*slerpinfo->Theta) * oo_sin_t;
	}

	if (slerpinfo->Flip) {
		alpha = -alpha;
	}

	set_q->X = beta*p.X + alpha*q.X;
	set_q->Y = beta*p.Y + alpha*q.Y;
	set_q->Z = beta*p.Z + alpha*q.Z;
	set_q->W = beta*p.W + alpha*q.W;
}

/***********************************************************************************************
 * Build_Quaternion -- Creates a quaternion from a Matrix                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *   Matrix MUST NOT have scaling!                                                             *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/28/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
Quaternion Build_Quaternion(const Matrix3D & mat)
{
	float tr,s;
	int i,j,k;
	Quaternion q;

	// sum the diagonal of the rotation matrix
	tr = mat[0][0] + mat[1][1] + mat[2][2];

	if (tr > 0.0f) {

		s = sqrt(tr + 1.0);
		q[3] = s * 0.5;
		s = 0.5 / s;

		q[0] = (mat[2][1] - mat[1][2]) * s;
		q[1] = (mat[0][2] - mat[2][0]) * s;
		q[2] = (mat[1][0] - mat[0][1]) * s;

	} else {

		i=0;
		if (mat[1][1] > mat[0][0]) i = 1;
		if (mat[2][2] > mat[i][i]) i = 2;
		j = _nxt[i];
		k = _nxt[j];

		s = sqrt((mat[i][i] - (mat[j][j] + mat[k][k])) + 1.0);

		q[i] = s * 0.5;
		if (s != 0.0) {
			s = 0.5 / s;
		}

		q[3] = 	( mat[k][j] - mat[j][k] ) * s;
		q[j] =	( mat[j][i] + mat[i][j] ) * s;
		q[k] =	( mat[k][i] + mat[i][k] ) * s;

	}

	return q;
}

Quaternion Build_Quaternion(const Matrix3 & mat)
{
	float tr,s;
	int i,j,k;
	Quaternion q;

	// sum the diagonal of the rotation matrix
	tr = mat[0][0] + mat[1][1] + mat[2][2];

	if (tr > 0.0) {

		s = sqrt(tr + 1.0);
		q[3] = s * 0.5;
		s = 0.5 / s;

		q[0] = (mat[2][1] - mat[1][2]) * s;
		q[1] = (mat[0][2] - mat[2][0]) * s;
		q[2] = (mat[1][0] - mat[0][1]) * s;

	} else {

		i = 0;
		if (mat[1][1] > mat[0][0]) i = 1;
		if (mat[2][2] > mat[i][i]) i = 2;

		j = _nxt[i];
		k = _nxt[j];

		s = sqrt( (mat[i][i] - (mat[j][j]+mat[k][k])) + 1.0);

		q[i] =	s * 0.5;

		if (s != 0.0) {
			s = 0.5/s;
		}

		q[3] = 	( mat[k][j] - mat[j][k] ) * s;
		q[j] =	( mat[j][i] + mat[i][j] ) * s;
		q[k] =	( mat[k][i] + mat[i][k] ) * s;
	}

	return q;
}

Quaternion Build_Quaternion(const Matrix4 & mat)
{
	float tr,s;
	int i,j,k;
	Quaternion q;

	// sum the diagonal of the rotation matrix
	tr = mat[0][0] + mat[1][1] + mat[2][2];

	if (tr > 0.0) {

		s = sqrt(tr + 1.0);
		q[3] = s * 0.5;
		s = 0.5 / s;

		q[0] = (mat[2][1] - mat[1][2]) * s;
		q[1] = (mat[0][2] - mat[2][0]) * s;
		q[2] = (mat[1][0] - mat[0][1]) * s;

	} else {

		i = 0;
		if (mat[1][1] > mat[0][0]) i = 1;
		if (mat[2][2] > mat[i][i]) i = 2;

		j = _nxt[i];
		k = _nxt[j];

		s = sqrt( (mat[i][i] - (mat[j][j]+mat[k][k])) + 1.0);

		q[i] =	s * 0.5;
		if (s != 0.0) {
			s = 0.5/s;
		}
		q[3] = 	( mat[k][j] - mat[j][k] ) * s;
		q[j] =	( mat[j][i] + mat[i][j] ) * s;
		q[k] =	( mat[k][i] + mat[i][k] ) * s;
	}

	return q;
}


/***********************************************************************************************
 * Build_Matrix -- Creates a Matrix from a Quaternion                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/28/1997 GH  : Created.                                                                 *
 *=============================================================================================*/
Matrix3	Build_Matrix3(const Quaternion & q)
{
	Matrix3 m;

	m[0][0] = (float)(1.0 - 2.0 * (q[1] * q[1] + q[2] * q[2]));
	m[0][1] = (float)(2.0 * (q[0] * q[1] - q[2] * q[3]));
	m[0][2] = (float)(2.0 * (q[2] * q[0] + q[1] * q[3]));

	m[1][0] = (float)(2.0 * (q[0] * q[1] + q[2] * q[3]));
	m[1][1] = (float)(1.0 - 2.0f * (q[2] * q[2] + q[0] * q[0]));
	m[1][2] = (float)(2.0 * (q[1] * q[2] - q[0] * q[3]));

	m[2][0] = (float)(2.0 * (q[2] * q[0] - q[1] * q[3]));
	m[2][1] = (float)(2.0 * (q[1] * q[2] + q[0] * q[3]));
	m[2][2] =(float)(1.0 - 2.0 * (q[1] * q[1] + q[0] * q[0]));

	return m;
}

Matrix3D Build_Matrix3D(const Quaternion & q)
{
	Matrix3D m;

	// initialize the rotation sub-matrix
	m[0][0] = (float)(1.0 - 2.0 * (q[1] * q[1] + q[2] * q[2]));
	m[0][1] = (float)(2.0 * (q[0] * q[1] - q[2] * q[3]));
	m[0][2] = (float)(2.0 * (q[2] * q[0] + q[1] * q[3]));

	m[1][0] = (float)(2.0 * (q[0] * q[1] + q[2] * q[3]));
	m[1][1] = (float)(1.0 - 2.0f * (q[2] * q[2] + q[0] * q[0]));
	m[1][2] = (float)(2.0 * (q[1] * q[2] - q[0] * q[3]));

	m[2][0] = (float)(2.0 * (q[2] * q[0] - q[1] * q[3]));
	m[2][1] = (float)(2.0 * (q[1] * q[2] + q[0] * q[3]));
	m[2][2] =(float)(1.0 - 2.0 * (q[1] * q[1] + q[0] * q[0]));

	// no translation
	m[0][3] = m[1][3] = m[2][3] = 0.0f;

	return m;
}

Matrix4 Build_Matrix4(const Quaternion & q)
{
	Matrix4 m;

	// initialize the rotation sub-matrix
	m[0][0] = (float)(1.0 - 2.0 * (q[1] * q[1] + q[2] * q[2]));
	m[0][1] = (float)(2.0 * (q[0] * q[1] - q[2] * q[3]));
	m[0][2] = (float)(2.0 * (q[2] * q[0] + q[1] * q[3]));

	m[1][0] = (float)(2.0 * (q[0] * q[1] + q[2] * q[3]));
	m[1][1] = (float)(1.0 - 2.0f * (q[2] * q[2] + q[0] * q[0]));
	m[1][2] = (float)(2.0 * (q[1] * q[2] - q[0] * q[3]));

	m[2][0] = (float)(2.0 * (q[2] * q[0] - q[1] * q[3]));
	m[2][1] = (float)(2.0 * (q[1] * q[2] + q[0] * q[3]));
	m[2][2] = (float)(1.0 - 2.0 * (q[1] * q[1] + q[0] * q[0]));

	// no translation
	m[0][3] = m[1][3] = m[2][3] = 0.0f;

	// last row
	m[3][0] = m[3][1] = m[3][2] = 0.0f;
	m[3][3] = 1.0f;
	return m;
}

void Quaternion::Rotate_X(float theta)
{
	// TODO: optimize this
	*this = (*this) * Quaternion(Vector3(1,0,0),theta);
}

void Quaternion::Rotate_Y(float theta)
{
	// TODO: optimize this
	*this = (*this) * Quaternion(Vector3(0,1,0),theta);
}

void Quaternion::Rotate_Z(float theta)
{
	// TODO: optimize this
	*this = (*this) * Quaternion(Vector3(0,0,1),theta);
}

float project_to_sphere(float r, float x, float y)
{
	const float SQRT2 = 1.41421356f;
	float t, z;
	float d = WWMath::Sqrt(x * x + y * y);

	if (d < r * (SQRT2/(2.0f)))			// inside sphere
		z = WWMath::Sqrt(r * r - d * d);
	else {								// on hyperbola
		t = r / SQRT2;
		z = t * t / d;
	}

	return z;
}


void Quaternion::Randomize(void)
{
	X = ((float) (rand() & 0xFFFF)) / 65536.0f;
	Y = ((float) (rand() & 0xFFFF)) / 65536.0f;
	Z = ((float) (rand() & 0xFFFF)) / 65536.0f;
	W = ((float) (rand() & 0xFFFF)) / 65536.0f;

	Normalize();
}


