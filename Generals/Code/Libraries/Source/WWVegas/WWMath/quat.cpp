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

/* $Header: /Commando/Code/wwmath/quat.cpp 38    8/28/01 10:26a Greg_h $ */
/*********************************************************************************************** 
 ***                            Confidential - Westwood Studios                              *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Voxel Technology                                             * 
 *                                                                                             * 
 *                    File Name : QUAT.CPP                                                     * 
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


#include "quat.h"
#include "matrix3d.h"
#include "matrix4.h"
#include "wwmath.h"

#include <stdio.h>
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
	float s = WWMath::Sin(angle/2);
	float c = WWMath::Cos(angle/2);
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
	float len2=X * X + Y * Y + Z * Z + W * W;
	if (0.0f == len2) {
		return;
	} else {
		float inv_mag = WWMath::Inv_Sqrt(len2);

		X *= inv_mag;
		Y *= inv_mag;
		Z *= inv_mag;
		W *= inv_mag;
	}
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
	phi = 2.0f * WWMath::Asin(t);

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

	q.Scale(WWMath::Sin(phi / 2.0f));
	q[3] =  WWMath::Cos(phi / 2.0f);

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

#if 0
#pragma warning (disable : 4725)

#define ARC_TABLE_SIZE_MASK 1023
#define SIN_TABLE_SIZE_MASK 1023

void __cdecl Fast_Slerp(Quaternion& res, const Quaternion & p,const Quaternion & q,float alpha)
{
	float float_epsilon2=WWMATH_EPSILON * WWMATH_EPSILON;
	float HalfOfArcTableSize=float(ARC_TABLE_SIZE/2);
	float HalfOfSinTableSize=float(SIN_TABLE_SIZE/2);
	const unsigned ARC_TABLE_SIZE_PER_2=ARC_TABLE_SIZE/2;

	float beta;		// complementary interploation parameter
	float theta;	// angle between p and q

	__asm {
		mov		esi, p
		mov		edi, q
		fld1										// we'll need 1.0 and 0.0 later

// ----------------------------------------------------------------------------
//	cos theta = dot product of p and q
//	cos_t = p.X * q.X + p.Y * q.Y + p.Z * q.Z + p.W * q.W;
// if q is on opposite hemisphere from A, use -B instead
// if (cos_t < 0.0) {
//		cos_t = -cos_t;
//		qflip = true;
//	}
//	else {
//		qflip = false;
//	}
// ----------------------------------------------------------------------------

		fld      dword ptr [esi]			// p.X
		fmul     dword ptr [edi]			//	p.X*q.X
		fld      dword ptr [esi+08h]		//	p.Y
		fmul     dword ptr [edi+08h]		// p.Y*q.Y
		fld      dword ptr [esi+04h]		// p.Z
		fmul     dword ptr [edi+04h]		// p.Z*q.Z
		fld      dword ptr [edi+0ch]		// p.W
		fmul     dword ptr [esi+0ch]		// p.W*q.W
		faddp		st(2), st(0)				// y+=w
		faddp		st(2), st(0)				// x+=z
		faddp		st(1),st(0)					// x+z + y+w
		fst		beta
		fabs
		mov		ebx,beta
		and		ebx,0x80000000

// ----------------------------------------------------------------------------
// if q is very close to p, just linearly interpolate
// between the two.
//	if (1.0 - cos_t < WWMATH_EPSILON * WWMATH_EPSILON) {
//		beta = 1.0 - alpha;
// }
// ----------------------------------------------------------------------------

		fld		st(0)						// duplicate st(0), which contains cos_t
		fsubr		st(0),st(2)				// st(2) contains 1.0
		fcomp    float_epsilon2
		fnstsw   ax
		test     ah, 01h
		je       normal_slerp

		fld		alpha
		fsubr		st(0),st(1)				// st(1) contains 1.0
 		fstp     beta
		jmp      done_slerp

normal_slerp:
// ----------------------------------------------------------------------------
// normal slerp!
//	else {
//		theta = WWMath::Acos(cos_t);
//		sin_t = WWMath::Sin(theta);
//		oo_sin_t = 1.0 / sin_t;
//		beta = WWMath::Sin(theta - alpha*theta) * oo_sin_t;
//		alpha = WWMath::Sin(alpha*theta) * oo_sin_t;
// }
//	if (qflip) {
//		alpha = -alpha;
//	}
// ----------------------------------------------------------------------------

		fld		HalfOfSinTableSize
		fld		HalfOfArcTableSize
		fmul		st(0),st(2)					// cos_t * (ARC_TABLE_SIZE/2)
		fistp		theta							// convert to integer
		mov		eax,theta
		add		eax,ARC_TABLE_SIZE_PER_2
		jns		no_neg
		xor		eax,eax
		jmp		contin
no_neg:
		cmp		eax,ARC_TABLE_SIZE
		jl			contin					// Note: Use Setcc/Movcc here!!!
		mov		eax,ARC_TABLE_SIZE_MASK
contin:
		fld		dword ptr[_FastAcosTable+eax*4]
		fst		theta

		fmul		st(0),st(1)				// theta * (sin_table_size/2)
		fadd		st(0),st(1)				// theta * (sin_table_size/2) + (sin_table_size/2)
		fistp		beta						// conver to integer
		mov		ecx,SIN_TABLE_SIZE_MASK
		mov		eax,beta
		and		eax,ecx					// & SIN_TABLE_SIZE_MASK
		fld		dword ptr[_FastInvSinTable+eax*4]	// 1.0f/sin(theta)

		fld      theta
		fmul     alpha						// theta*alpha
		fld		st(0)						// duplicate stack head as we need theta*alpha later
		fsubr		theta						// theta-theta*alpha

		fmul		st(0),st(3)				//	(theta-theta*alpha)*HalfOfSinTableSize
		fadd		st(0),st(3)				// (theta-theta*alpha)*HalfOfSinTableSize+HalfOfSinTableSize
		fistp		beta						// convert to integer
		mov		eax,beta
		and		eax,ecx					// & SIN_TABLE_SIZE_MASK
		fld		dword ptr[_FastSinTable+eax*4]	// sin(theta-theta*alpha)

		fmul     st(0),st(2)				// sin(theta-theta*alpha) * oo_sin_t
		fstp     beta

		fmul		st(0),st(2)				// (theta*alpha)*HalfOfSinTableSize
		fadd		st(0),st(2)				// (theta*alpha)*HalfOfSinTableSize+HalfOfSinTableSize
		fistp		theta						// convert to integer
		mov		eax,theta
		and		eax,ecx					// & SIN_TABLE_SIZE_MASK
		fld		dword ptr[_FastSinTable+eax*4]	// sin(theta*alpha)

		fmul     st(0),st(1)				//	oo_sin_t
		fstp     alpha
		fstp st(0)							// pop oo_sin_t
		fstp	st(0)							// pop HalfOfSinTableSize
done_slerp:
		test     ebx, ebx
		je       no_negative
		fld      alpha
		fchs
		fstp     alpha

no_negative:
// ----------------------------------------------------------------------------
//	res.X = beta*p.X + alpha*q.X;
//	res.Y = beta*p.Y + alpha*q.Y;
//	res.Z = beta*p.Z + alpha*q.Z;
//	res.W = beta*p.W + alpha*q.W;
// ----------------------------------------------------------------------------

		fstp		st(0)						// pop cos_t
		fstp		st(0)						// pop 1.0

		fld		alpha
		fld		dword ptr [edi+4]		// q.Y
		fmul		st(0),st(1)				// alpha*q.Y
		fld		dword ptr [edi+8]		// q.Z
		fmul		st(0),st(2)				// alpha*q.Z
		fld		dword ptr [edi+12]	// q.W
		fmul		st(0),st(3)				// alpha*q.W
		fld		dword ptr [edi]		// q.X
		fmulp		st(4),st					// alpha*q.X

		fld		beta
		fld		dword ptr [esi+4]		// p.Y
		fmul		st(0),st(1)				// beta*p.Y
		fld		dword ptr [esi+8]		// p.Z
		fmul		st(0),st(2)				// beta*p.Z
		fld		dword ptr [esi]		// p.X
		fmul		st(0),st(3)				// beta*p.X
		fxch		st(3)						// move beta to top of stack
		fmul		dword ptr [esi+12]	// beta*p.W

		faddp		st(4),st					// w
		faddp		st(4),st					// z
		faddp		st(4),st					// y
		faddp		st(4),st					// x

		mov		ecx, res
		fstp		[ecx+12]					// w
		fstp		[ecx+8]					//	z
		fstp		[ecx+4]					// y
		fstp		[ecx]						// x
	}
}
#else
void __cdecl Fast_Slerp(Quaternion& res, const Quaternion & p,const Quaternion & q,float alpha)
{		
	float beta;			// complementary interploation parameter
	float theta;		// angle between p and q
	float cos_t; 		// sine, cosine of theta
	float oo_sin_t;
	int qflip;			// use flip of q?
	
	// cos theta = dot product of p and q
	cos_t = p.X * q.X + p.Y * q.Y + p.Z * q.Z + p.W * q.W;

	// if q is on opposite hemisphere from A, use -B instead
	if (cos_t < 0.0f) {
		cos_t = -cos_t;
		qflip = true;
	} else {
		qflip = false;
	}

	if (1.0f - cos_t < WWMATH_EPSILON * WWMATH_EPSILON) {

		// if q is very close to p, just linearly interpolate
		// between the two.
		beta = 1.0f - alpha;

	} else {

		theta = WWMath::Fast_Acos(cos_t);
		float sin_t = WWMath::Fast_Sin(theta);
		oo_sin_t = 1.0f / sin_t;
		beta = WWMath::Fast_Sin(theta - alpha*theta) * oo_sin_t;
		alpha = WWMath::Fast_Sin(alpha*theta) * oo_sin_t;
	}

	if (qflip) {
		alpha = -alpha;
	}

	res.X = beta*p.X + alpha*q.X;
	res.Y = beta*p.Y + alpha*q.Y;
	res.Z = beta*p.Z + alpha*q.Z;
	res.W = beta*p.W + alpha*q.W;
}

#endif	// MSC_VER

void Slerp(Quaternion& res, const Quaternion & p,const Quaternion & q,float alpha)
{		
	float beta;				// complementary interploation parameter
	float theta;				// angle between p and q
	//float sin_t
	float cos_t; 		// sine, cosine of theta
	float oo_sin_t;
	int qflip;					// use flip of q?
	
	// cos theta = dot product of p and q
	cos_t = p.X * q.X + p.Y * q.Y + p.Z * q.Z + p.W * q.W;

	// if q is on opposite hemisphere from A, use -B instead
	if (cos_t < 0.0f) {
		cos_t = -cos_t;
		qflip = true;
	} else {
		qflip = false;
	}

	if (1.0f - cos_t < WWMATH_EPSILON * WWMATH_EPSILON) {

		// if q is very close to p, just linearly interpolate
		// between the two.
		beta = 1.0f - alpha;

	} else {

		// normal slerp!
		theta = WWMath::Acos(cos_t);
		float sin_t = WWMath::Sin(theta);
		oo_sin_t = 1.0f / sin_t;
		beta = WWMath::Sin(theta - alpha*theta) * oo_sin_t;
		alpha = WWMath::Sin(alpha*theta) * oo_sin_t;
	}

	if (qflip) {
		alpha = -alpha;
	}

	res.X = beta*p.X + alpha*q.X;
	res.Y = beta*p.Y + alpha*q.Y;
	res.Z = beta*p.Z + alpha*q.Z;
	res.W = beta*p.W + alpha*q.W;
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
	if (cos_t < 0.0f) {
		cos_t = -cos_t;
		slerpinfo->Flip = true;
	} else {
		slerpinfo->Flip = false;
	}

	if (1.0f - cos_t < SLERP_EPSILON) {

		slerpinfo->Linear = true;
		slerpinfo->Theta = 0.0f;
		slerpinfo->SinT = 0.0f;

	} else {

		slerpinfo->Linear = false;
		slerpinfo->Theta = WWMath::Acos(cos_t);
		slerpinfo->SinT = WWMath::Sin(slerpinfo->Theta);
	
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
		beta = 1.0f - alpha;

	} else {

		// normal slerp!
		oo_sin_t = 1.0f / slerpinfo->Theta;
		beta = WWMath::Sin(slerpinfo->Theta - alpha*slerpinfo->Theta) * oo_sin_t;
		alpha = WWMath::Sin(alpha*slerpinfo->Theta) * oo_sin_t;
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
		beta = 1.0f - alpha;

	} else {

		// normal slerp!
		oo_sin_t = 1.0f / slerpinfo->Theta;
		beta = WWMath::Sin(slerpinfo->Theta - alpha*slerpinfo->Theta) * oo_sin_t;
		alpha = WWMath::Sin(alpha*slerpinfo->Theta) * oo_sin_t;
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

Quaternion Build_Quaternion(const Matrix3x3 & mat)
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

Quaternion Build_Quaternion(const Matrix4x4 & mat)
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
Matrix3x3 Build_Matrix3(const Quaternion & q)
{
	Matrix3x3 m;

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

Matrix4x4 Build_Matrix4(const Quaternion & q)
{
	Matrix4x4 m;

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


