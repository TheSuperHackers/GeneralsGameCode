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

/* $Header: /Commando/Code/Tools/max2w3d/EULER.cpp 5     12/02/97 10:14p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Commando / G 3D Engine                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/Tools/max2w3d/EULER.cpp                      $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 11/13/97 7:16p                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "EULER.h"
#include <float.h>


/*********************************************************************

	There are 24 possible conventions for Euler angles.  They can
	be designated by:

		EulerAxis		= Axis used initially
		EulerParity		= parity of axis permutation (even = x,y,z)
		EulerRepeat		= is last axis a repeat of the initial axis?
		EulerFrame		= frame from which axes are taken (rotating or static)

*********************************************************************/

#define EULER_FRAME_STATIC			0x00000000
#define EULER_FRAME_ROTATING		0x00000001
#define EULER_FRAME(order)			((unsigned)(order) & 1)

#define EULER_REPEAT_NO				0x00000000
#define EULER_REPEAT_YES			0x00000001
#define EULER_REPEAT(order)		(((unsigned)(order) >> 1) & 1)

#define EULER_PARITY_EVEN			0x00000000
#define EULER_PARITY_ODD			0x00000001
#define EULER_PARITY(order)		(((unsigned)(order) >> 2) & 1)

#define EULER_BUILD_ORDER(i,p,r,f) (((((((i) << 1) + (p)) << 1) + (r)) << 1) + (f))


/* static axes */
int	EulerOrderXYZs = EULER_BUILD_ORDER(0, EULER_PARITY_EVEN, EULER_REPEAT_NO,  EULER_FRAME_STATIC);
int	EulerOrderXYXs = EULER_BUILD_ORDER(0, EULER_PARITY_EVEN, EULER_REPEAT_YES, EULER_FRAME_STATIC);
int	EulerOrderXZYs = EULER_BUILD_ORDER(0, EULER_PARITY_ODD,  EULER_REPEAT_NO,  EULER_FRAME_STATIC);
int	EulerOrderXZXs = EULER_BUILD_ORDER(0, EULER_PARITY_ODD,  EULER_REPEAT_YES, EULER_FRAME_STATIC);
int	EulerOrderYZXs = EULER_BUILD_ORDER(1, EULER_PARITY_EVEN, EULER_REPEAT_NO,  EULER_FRAME_STATIC);
int	EulerOrderYZYs = EULER_BUILD_ORDER(1, EULER_PARITY_EVEN, EULER_REPEAT_YES, EULER_FRAME_STATIC);
int	EulerOrderYXZs = EULER_BUILD_ORDER(1, EULER_PARITY_ODD,  EULER_REPEAT_NO,  EULER_FRAME_STATIC);
int	EulerOrderYXYs = EULER_BUILD_ORDER(1, EULER_PARITY_ODD,  EULER_REPEAT_YES, EULER_FRAME_STATIC);
int	EulerOrderZXYs = EULER_BUILD_ORDER(2, EULER_PARITY_EVEN, EULER_REPEAT_NO,  EULER_FRAME_STATIC);
int	EulerOrderZXZs = EULER_BUILD_ORDER(2, EULER_PARITY_EVEN, EULER_REPEAT_YES, EULER_FRAME_STATIC);
int	EulerOrderZYXs = EULER_BUILD_ORDER(2, EULER_PARITY_ODD,  EULER_REPEAT_NO,  EULER_FRAME_STATIC);
int	EulerOrderZYZs = EULER_BUILD_ORDER(2, EULER_PARITY_ODD,  EULER_REPEAT_YES, EULER_FRAME_STATIC);

/* rotating axes */
int	EulerOrderZYXr = EULER_BUILD_ORDER(0, EULER_PARITY_EVEN, EULER_REPEAT_NO,  EULER_FRAME_ROTATING);
int	EulerOrderXYXr = EULER_BUILD_ORDER(0, EULER_PARITY_EVEN, EULER_REPEAT_YES, EULER_FRAME_ROTATING);
int	EulerOrderYZXr = EULER_BUILD_ORDER(0, EULER_PARITY_ODD,  EULER_REPEAT_NO,  EULER_FRAME_ROTATING);
int	EulerOrderXZXr = EULER_BUILD_ORDER(0, EULER_PARITY_ODD,  EULER_REPEAT_YES, EULER_FRAME_ROTATING);
int	EulerOrderXZYr = EULER_BUILD_ORDER(1, EULER_PARITY_EVEN, EULER_REPEAT_NO,  EULER_FRAME_ROTATING);
int	EulerOrderYZYr = EULER_BUILD_ORDER(1, EULER_PARITY_EVEN, EULER_REPEAT_YES, EULER_FRAME_ROTATING);
int	EulerOrderZXYr = EULER_BUILD_ORDER(1, EULER_PARITY_ODD,  EULER_REPEAT_NO,  EULER_FRAME_ROTATING);
int	EulerOrderYXYr = EULER_BUILD_ORDER(1, EULER_PARITY_ODD,  EULER_REPEAT_YES, EULER_FRAME_ROTATING);
int	EulerOrderYXZr = EULER_BUILD_ORDER(2, EULER_PARITY_EVEN, EULER_REPEAT_NO,  EULER_FRAME_ROTATING);
int	EulerOrderZXZr = EULER_BUILD_ORDER(2, EULER_PARITY_EVEN, EULER_REPEAT_YES, EULER_FRAME_ROTATING);
int	EulerOrderXYZr = EULER_BUILD_ORDER(2, EULER_PARITY_ODD,  EULER_REPEAT_NO,  EULER_FRAME_ROTATING);
int	EulerOrderZYZr = EULER_BUILD_ORDER(2, EULER_PARITY_ODD,  EULER_REPEAT_YES, EULER_FRAME_ROTATING);

/* local functions */
static void _euler_unpack_order(int order,int &i,int &j,int &k,int &h,int &n,int &s,int &f);
static int _euler_axis_i(int order);
static int _euler_axis_j(int order);
static int _euler_axis_k(int order);
static int _euler_axis_h(int order);
static void _mat_to_array(const Matrix3 & tm, float M[3][4]);
static void _array_to_mat(float M[3][4], Matrix3 & tm);



EulerAnglesClass::EulerAnglesClass(const Matrix3 & M,int order)
{
	this->From_Matrix(M,order);
}

double EulerAnglesClass::Get_Angle(int i)
{
	return Angle[i];
}

void EulerAnglesClass::From_Matrix(const Matrix3 & tm, int order)
{
	float M[3][4];
	_mat_to_array(tm,M);

	int i,j,k,h,n,s,f;

	Order = order;
	_euler_unpack_order(order,i,j,k,h,n,s,f);

	if (s == EULER_REPEAT_YES) {
		double sy = sqrt(M[i][j]*M[i][j] + M[i][k]*M[i][k]);

		if (sy > 16*FLT_EPSILON) {

			Angle[0] = atan2(M[i][j],M[i][k]);
			Angle[1] = atan2(sy,M[i][i]);
			Angle[2] = atan2(M[j][i],-M[k][i]);

		} else {

			Angle[0] = atan2(-M[j][k],M[j][j]);
			Angle[1] = atan2(sy,M[i][i]);
			Angle[2] = 0.0;
		}

	} else {

		double cy = sqrt(M[i][i]*M[i][i] + M[j][i]*M[j][i]);

		if (cy > 16*FLT_EPSILON) {

			Angle[0] = atan2(M[k][j],M[k][k]);
			Angle[1] = atan2(-M[k][i],cy);
			Angle[2] = atan2(M[j][i],M[i][i]);

		} else {

			Angle[0] = atan2(-M[j][k],M[j][j]);
			Angle[1] = atan2(-M[k][i],cy);
			Angle[2] = 0;
		}
	}

	if (n==EULER_PARITY_ODD) { Angle[0] = -Angle[0]; Angle[1] = -Angle[1]; Angle[2] = -Angle[2]; }
	if (f==EULER_FRAME_ROTATING) { double t = Angle[0]; Angle[0] = Angle[2]; Angle[2] = t; }

	// Trying to "clean" up the eulers, special cased for XYZr
	if (order == EulerOrderXYZr) {

		double x2 = PI + Angle[0];
		double y2 = PI - Angle[1];
		double z2 = PI + Angle[2];

		if (x2 > PI) {
			x2 = x2 - 2*PI;
		}

		if (y2 > PI) {
			y2 = y2 - 2*PI;
		}

		if (z2 > PI) {
			z2 = z2 - 2*PI;
		}

		double mag0 = Angle[0]*Angle[0] + Angle[1]*Angle[1] + Angle[2]*Angle[2];
		double mag1 = x2*x2 + y2*y2 + z2*z2;

		if (mag1 < mag0) {
			Angle[0] = x2;
			Angle[1] = y2;
			Angle[2] = z2;
		}
	}
}



void EulerAnglesClass::To_Matrix(Matrix3 & tm)
{
	float M[3][4] = {
		{ 1.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f, 0.0f }
	};

	double a0,a1,a2;
	double ti,tj,th,ci,cj,ch,si,sj,sh,cc,cs,sc,ss;
	int i,j,k,h,n,s,f;

	a0 = Angle[0];
	a1 = Angle[1];
	a2 = Angle[2];

	_euler_unpack_order(Order,i,j,k,h,n,s,f);
	if (f == EULER_FRAME_ROTATING) {
		double t = a0; a0 = a2; a2 = t;
	}

	if (n == EULER_PARITY_ODD) {
		a0 = -a0; a1 = -a1; a2 = -a2;
	}

	ti = a0;				tj = a1;				th = a2;
	ci = cos(ti);		cj = cos(tj);		ch = cos(th);
	si = sin(ti);		sj = sin(tj);		sh = sin(th);

	cc = ci*ch;
	cs = ci*sh;
	sc = si*ch;
	ss = si*sh;

	if (s == EULER_REPEAT_YES) {

		M[i][i] = (float)(cj);			M[i][j] = (float)(sj*si);			M[i][k] = (float)(sj*ci);
		M[j][i] = (float)(sj*sh);		M[j][j] = (float)(-cj*ss+cc);		M[j][k] = (float)(-cj*cs-sc);
		M[k][i] = (float)(-sj*ch);		M[k][j] = (float)(cj*sc+cs);		M[k][k] = (float)(cj*cc-ss);

	} else {

		M[i][i] = (float)(cj*ch);		M[i][j] = (float)(sj*sc-cs);		M[i][k] = (float)(sj*cc+ss);
		M[j][i] = (float)(cj*sh);		M[j][j] = (float)(sj*ss+cc);		M[j][k] = (float)(sj*cs-sc);
		M[k][i] = (float)(-sj);			M[k][j] = (float)(cj*si);			M[k][k] = (float)(cj*ci);

	}
	_array_to_mat(M,tm);
}


static int _euler_safe[] = { 0,1,2,0 };
static int _euler_next[] = { 1,2,0,1 };

int _euler_axis_i(int order)
{
	return _euler_safe[ (order>>3) & 3 ];
}

int _euler_axis_j(int order)
{
	int index = _euler_axis_i(order);
	if (EULER_PARITY(order) == 1) {
		index++;
	}

	return _euler_next[ index ];
}

int _euler_axis_k(int order)
{
	int index = _euler_axis_i(order);
	if (EULER_PARITY(order) != 1) {
		index++;
	}

	return _euler_next[ index ];
}

int _euler_axis_h(int order)
{
	if (EULER_REPEAT(order) == 1) {
		return _euler_axis_k(order);
	} else {
		return _euler_axis_i(order);
	}
}

void _euler_unpack_order(int order,int &i,int &j,int &k,int &h,int &n,int &s,int &f)
{

	f = order & 1;
	order >>= 1;

	s = order & 1;
	order >>= 1;

	n = order & 1;
	order >>= 1;

	i = _euler_safe[order & 3];
	j = _euler_next[i+n];
	k = _euler_next[i+1-n];
	h = (s ? k : i);
}

void _mat_to_array(const Matrix3 & tm, float M[3][4])
{
	// Translation vector
	Point3 trans = tm.GetRow(3);
	M[0][3] = trans.x;
	M[1][3] = trans.y;
	M[2][3] = trans.z;

	// Rotation matrix
	for (int k=0; k<3; k++) {
		Point3 v = tm.GetRow(k);
		M[0][k] = v.x;
		M[1][k] = v.y;
		M[2][k] = v.z;
	}
}

void _array_to_mat(float M[3][4], Matrix3 & tm)
{
	// translation
	Point3 row(M[3][0],M[3][1],M[3][2]);
	tm.SetRow(3, row);

	// rotation
	for (int k=0; k<3; k++) {
		row = Point3(M[0][k],M[1][k],M[2][k]);
		tm.SetRow(k, row);
	}
}


