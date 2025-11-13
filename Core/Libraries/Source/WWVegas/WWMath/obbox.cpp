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

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : WWMath                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwmath/obbox.cpp                             $*
 *                                                                                             *
 *                    Org Author:: Greg_h                                                       *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               *
 *                                                                                             *
 *                     $Modtime:: 06/26/02 4:04p                                             $*
 *                                                                                             *
 *                    $Revision:: 24                                                          $*
 *                                                                                             *
 * 06/26/02 KM Matrix name change to avoid MAX conflicts                                       *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   OBBoxClass::OBBoxClass -- Constructor that computes the box for a set of point            *
 *   OBBoxClass::Init_From_Box_Points -- Create an OBBox from 8 corners of  a box              *
 *   OBBoxClass::Init_Random -- initalize a random oriented box                                *
 *   Oriented_Boxes_Intersect_On_Axis -- test if two boxes intersect on given axis             *
 *   Oriented_Boxes_Intersect -- test if two oriented boxes intersect                          *
 *   Oriented_Boxes_Collide_On_Axis -- test if two boxes collide on the given axis             *
 *   Oriented_Boxes_Collide -- test if two oriented boxes collide                              *
 *   Oriented_Box_Intersects_Tri_On_Axis -- tests if the box and tri intersect on the axis     *
 *   Oriented_Box_Intersects_Tri -- tests if the given box and tri intersect                   *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "obbox.h"
#include "matrix3.h"
#include "vector3.h"
#include "aabox.h"
#include "tri.h"
#include "plane.h"
#include "quat.h"
#include <assert.h>
//#include <stdlib.h>


/***********************************************************************************************
 * OBBoxClass::OBBoxClass -- Constructor that computes the box for a set of points             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/4/98     GTH : Created.                                                                 *
 *=============================================================================================*/
OBBoxClass::OBBoxClass(const Vector3 * /*points*/, int /*n*/)
{
	// TODO: IMPLEMENT THIS!!!
	assert(0);

#if 0
	int i;

	// compute mean and covariances of points
	float xsum = 0.0f, ysum = 0.0f, zsum = 0.0f;
	float xxsum = 0.0f, xysum = 0.0f, xzsum = 0.0f;
	float yysum = 0.0f, yzsum = 0.0f, zzsum = 0.0f;

	for (i = 0; i < n; i++)
	{
		xsum += points[i].X;
		ysum += points[i].Y;
		zsum += points[i].Z;

		xxsum += points[i].X * points[i].X;
		xysum += points[i].X * points[i].Y;
		xzsum += points[i].X * points[i].Z;

		yysum += points[i].Y * points[i].Y;
		yzsum += points[i].Y * points[i].Z;
		zzsum += points[i].Z * points[i].Z;
	}

	float xmean = xsum/n;
	float ymean = ysum/n;
	float zmean = zsum/n;
	float xxcov = xxsum/n - xmean*xmean;
	float xycov = xysum/n - xmean*ymean;
	float xzcov = xzsum/n - xmean*zmean;
	float yycov = yysum/n - ymean*ymean;
	float yzcov = yzsum/n - ymean*zmean;
	float zzcov = zzsum/n - zmean*zmean;

	// compute eigenvectors for covariance matrix,
	// these will be the axes.
	mgcEigen eig(3);
	eig.Matrix(0,0) = xxcov;
	eig.Matrix(0,1) = xycov;
	eig.Matrix(0,2) = xzcov;
	eig.Matrix(1,0) = xycov;
	eig.Matrix(1,1) = yycov;
	eig.Matrix(1,2) = yzcov;
	eig.Matrix(2,0) = xzcov;
	eig.Matrix(2,1) = yzcov;
	eig.Matrix(2,2) = zzcov;

	eig.EigenStuff3();
	Point3 U =
	{
		eig.Eigenvector(0,0),
		eig.Eigenvector(1,0),
		eig.Eigenvector(2,0)
	};
	Point3 V =
	{
		eig.Eigenvector(0,1),
		eig.Eigenvector(1,1),
		eig.Eigenvector(2,1)
	};
	Point3 W =
	{
		eig.Eigenvector(0,2),
		eig.Eigenvector(1,2),
		eig.Eigenvector(2,2)
	};

	// box center is the mean of the distribution
	box.center.x = xmean;
	box.center.y = ymean;
	box.center.z = zmean;


	// Box axes are the eigenvectors of the covariance matrix with
	// adjusted lengths to enclose the points.  If U, V, and W are the
	// eigenvectors, C is the center of the box, and X is a point in
	// the input list, then X = C + a*U + b*V + c*W.  The box extent is
	// determined by max|a|, max|b|, and max|c|.  The box axes are then
	// defined to be (max|a|)*U and (max|b|)*V.  Note that since U and V
	// are unit length and orthogonal, a = Dot(U,X-C), b = Dot(V,X-C),
	// and c = Dot(W,X-C).
	float amax = 0.0f, bmax = 0.0f, cmax = 0.0f;
	for (i = 0; i < n; i++)
	{
		float dx = pt[i].x - box.center.x;
		float dy = pt[i].y - box.center.y;
		float dz = pt[i].z - box.center.z;
		float absdot = float(WWMath::Fabs(U.x*dx+U.y*dy+U.z*dz));
		if ( absdot > amax )
			amax = absdot;
		absdot = float(WWMath::Fabs(V.x*dx+V.y*dy+V.z*dz));
		if ( absdot > bmax )
			bmax = absdot;
		absdot = float(WWMath::Fabs(W.x*dx+W.y*dy+W.z*dz));
		if ( absdot > cmax )
			cmax = absdot;
	}

	box.axis[0].x = amax*U.x;
	box.axis[0].y = amax*U.y;
	box.axis[0].z = amax*U.z;
	box.axis[1].x = bmax*V.x;
	box.axis[1].y = bmax*V.y;
	box.axis[1].z = bmax*V.z;
	box.axis[2].x = cmax*W.x;
	box.axis[2].y = cmax*W.y;
	box.axis[2].z = cmax*W.z;

#endif
}

/***********************************************************************************************
 * OBBoxClass::Init_From_Box_Points -- Create an OBBox from 8 corners of  a box                *
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
void OBBoxClass::Init_From_Box_Points(Vector3 * points,int num)
{
	int i,j;

	/*
	** This function assumes that you pass in 8 points which are the
	** corners of a rectangular solid.  Bad things will happen if
	** this assumption is not true!!!!
	*/
	assert(num == 8);

	/*
	** Just pick the first point as the preliminary center.  Compute
	** vectors from this point to each of the other points
	*/
	Vector3 dp[8];
	for (i=1;i<num;i++) {
		dp[i] = points[i] - points[0];
	}

	/*
	** Find the shortest two candidate axes.  Then the
	** third axis will be the cross product of these two.
	*/
	for (i=1;i<num;i++) {
		for (j=i+1;j<num;j++) {
			if (dp[j].Length2() < dp[i].Length2()) {
				Swap(dp[j],dp[i]);
			}
		}
	}

	Vector3 axis0,axis1,axis2;
#ifdef ALLOW_TEMPORARIES
	axis0 = Normalize(dp[1]);
	axis1 = Normalize(dp[2]);
#else
	axis0 = dp[1]; axis0.Normalize();
	axis1 = dp[2]; axis1.Normalize();
#endif
	Vector3::Cross_Product(axis0,axis1,&axis2);

	Basis = Matrix3x3(axis0,axis1,axis2);

	/*
	** Center is the average of all of the points
	*/
	Center.Set(0,0,0);
	for (i=0; i<num; i++) {
		Center += points[i];
	}
	Center.X /= num;
	Center.Y /= num;
	Center.Z /= num;

	/*
	** Compute extents along the computed axes.  This is done
	** by projecting each point onto the three axes and keeping
	** the largest projection on each.
	*/
	Extent.Set(0,0,0);

	for (i=0; i<num; i++) {
		float dx = points[i].X - Center.X;
		float dy = points[i].Y - Center.Y;
		float dz = points[i].Z - Center.Z;

		float xprj = float(WWMath::Fabs(axis0.X * dx + axis0.Y * dy + axis0.Z * dz));
		if (xprj > Extent.X) Extent.X = xprj;

		float yprj = float(WWMath::Fabs(axis1.X * dx + axis1.Y * dy + axis1.Z * dz));
		if (yprj > Extent.Y) Extent.Y = yprj;

		float zprj = float(WWMath::Fabs(axis2.X * dx + axis2.Y * dy + axis2.Z * dz));
		if (zprj > Extent.Z) Extent.Z = zprj;
	}
}


/***********************************************************************************************
 * OBBoxClass::Init_Random -- initalize a random oriented box                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/21/98    GTH : Created.                                                                 *
 *=============================================================================================*/
void OBBoxClass::Init_Random(float min_extent,float max_extent)
{
	Center.Set(0,0,0);

	Extent.X = min_extent + WWMath::Random_Float() * (max_extent - min_extent);
	Extent.Y = min_extent + WWMath::Random_Float() * (max_extent - min_extent);
	Extent.Z = min_extent + WWMath::Random_Float() * (max_extent - min_extent);

	Quaternion orient;
	orient.X = WWMath::Random_Float();
	orient.Y = WWMath::Random_Float();
	orient.Z = WWMath::Random_Float();
	orient.W = WWMath::Random_Float();
	orient.Normalize();

	Basis = Build_Matrix3(orient);
}




/***********************************************************************************************
 * Oriented_Boxes_Intersect_On_Axis -- test if two boxes intersect on given axis               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/7/99     GTH : Created.                                                                 *
 *=============================================================================================*/
bool Oriented_Boxes_Intersect_On_Axis
(
	const OBBoxClass & box0,
	const OBBoxClass & box1,
	const Vector3 & axis
)
{
	float ra,rb,rsum;

	if (axis.Length2() < WWMATH_EPSILON) return true;

	ra = box0.Project_To_Axis(axis);
	rb = box1.Project_To_Axis(axis);
	rsum = WWMath::Fabs(ra) + WWMath::Fabs(rb);

	// project the center distance onto the line:
	Vector3 C = box1.Center - box0.Center;
	float cdist = Vector3::Dot_Product(axis,C);

	if ((cdist > rsum) || (cdist < -rsum)) {
		return false;
	}
	return true;
}


/***********************************************************************************************
 * Oriented_Boxes_Intersect -- test if two oriented boxes intersect                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/7/99     GTH : Created.                                                                 *
 *=============================================================================================*/
bool Oriented_Boxes_Intersect
(
	const OBBoxClass & box0,
	const OBBoxClass & box1
)
{
	Vector3 axis;
	Vector3 A[3],B[3];

	// vectors for the axis directions of the two boxes in world space
	A[0].Set(box0.Basis[0][0],box0.Basis[1][0],box0.Basis[2][0]);
	A[1].Set(box0.Basis[0][1],box0.Basis[1][1],box0.Basis[2][1]);
	A[2].Set(box0.Basis[0][2],box0.Basis[1][2],box0.Basis[2][2]);

	B[0].Set(box1.Basis[0][0],box1.Basis[1][0],box1.Basis[2][0]);
	B[1].Set(box1.Basis[0][1],box1.Basis[1][1],box1.Basis[2][1]);
	B[2].Set(box1.Basis[0][2],box1.Basis[1][2],box1.Basis[2][2]);

	/////////////////////////////////////////////////////////////////////////
	// Projecting the two boxes onto Box0's X axis.  If their intervals
	// on this line do not intersect, the boxes are not intersecting.
	// Each of the tests in this function work in a similar way.
	/////////////////////////////////////////////////////////////////////////
	if (!Oriented_Boxes_Intersect_On_Axis(box0,box1,A[0])) return false;
	if (!Oriented_Boxes_Intersect_On_Axis(box0,box1,A[1])) return false;
	if (!Oriented_Boxes_Intersect_On_Axis(box0,box1,A[2])) return false;
	if (!Oriented_Boxes_Intersect_On_Axis(box0,box1,B[0])) return false;
	if (!Oriented_Boxes_Intersect_On_Axis(box0,box1,B[1])) return false;
	if (!Oriented_Boxes_Intersect_On_Axis(box0,box1,B[2])) return false;

	/////////////////////////////////////////////////////////////////////////
	// None of the aligned axes turned out to be separating axes.  Now
	// we check all combinations of cross products of the two boxes axes.
	/////////////////////////////////////////////////////////////////////////
	Vector3::Cross_Product(A[0],B[0],&axis);
	if (!Oriented_Boxes_Intersect_On_Axis(box0,box1,axis)) return false;

	Vector3::Cross_Product(A[0],B[1],&axis);
	if (!Oriented_Boxes_Intersect_On_Axis(box0,box1,axis)) return false;

	Vector3::Cross_Product(A[0],B[2],&axis);
	if (!Oriented_Boxes_Intersect_On_Axis(box0,box1,axis)) return false;

	Vector3::Cross_Product(A[1],B[0],&axis);
	if (!Oriented_Boxes_Intersect_On_Axis(box0,box1,axis)) return false;

	Vector3::Cross_Product(A[1],B[1],&axis);
	if (!Oriented_Boxes_Intersect_On_Axis(box0,box1,axis)) return false;

	Vector3::Cross_Product(A[1],B[2],&axis);
	if (!Oriented_Boxes_Intersect_On_Axis(box0,box1,axis)) return false;

	Vector3::Cross_Product(A[2],B[0],&axis);
	if (!Oriented_Boxes_Intersect_On_Axis(box0,box1,axis)) return false;

	Vector3::Cross_Product(A[2],B[1],&axis);
	if (!Oriented_Boxes_Intersect_On_Axis(box0,box1,axis)) return false;

	Vector3::Cross_Product(A[2],B[2],&axis);
	if (!Oriented_Boxes_Intersect_On_Axis(box0,box1,axis)) return false;

	// None of the above tests separated the two boxes, so they are intersecting
	return true;
}





/***********************************************************************************************
 * Oriented_Boxes_Collide_On_Axis -- test if two boxes collide on the given axis               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/7/99     GTH : Created.                                                                 *
 *=============================================================================================*/
bool Oriented_Boxes_Collide_On_Axis
(
	const OBBoxClass & box0,
	const Vector3 & v0,
	const OBBoxClass & box1,
	const Vector3 & v1,
	const Vector3 & axis,
	float dt
)
{
	float ra,rb,rsum;

	if (axis.Length2() < WWMATH_EPSILON) return true;

	ra = box0.Project_To_Axis(axis);
	rb = box1.Project_To_Axis(axis);
	rsum = WWMath::Fabs(ra) + WWMath::Fabs(rb);

	// project the center distance onto the line:
	Vector3 C = box1.Center - box0.Center;
	Vector3 V = v1 - v0;

	float cdist = Vector3::Dot_Product(axis,C);
	float vdist = cdist + dt * Vector3::Dot_Product(axis,V);

	if ((cdist > rsum && vdist > rsum) || (cdist < -rsum && vdist < -rsum)) {
		return false;
	}
	return true;
}


/***********************************************************************************************
 * Oriented_Boxes_Collide -- test if two oriented boxes collide                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/7/99     GTH : Created.                                                                 *
 *=============================================================================================*/
bool Oriented_Boxes_Collide
(
	const OBBoxClass & box0,
	const Vector3 & v0,
	const OBBoxClass & box1,
	const Vector3 & v1,
	float dt
)
{
	bool intersect = true;

	// variables for holding the separating axis and the projected distances
	Vector3 SepAxis;

	// vectors for the axis directions of the two boxes in world space
	Vector3 A0(box0.Basis[0][0],box0.Basis[1][0],box0.Basis[2][0]);
	Vector3 A1(box0.Basis[0][1],box0.Basis[1][1],box0.Basis[2][1]);
	Vector3 A2(box0.Basis[0][2],box0.Basis[1][2],box0.Basis[2][2]);

	Vector3 B0(box1.Basis[0][0],box1.Basis[1][0],box1.Basis[2][0]);
	Vector3 B1(box1.Basis[0][1],box1.Basis[1][1],box1.Basis[2][1]);
	Vector3 B2(box1.Basis[0][2],box1.Basis[1][2],box1.Basis[2][2]);


	/////////////////////////////////////////////////////////////////////////
	// L = A0
	//
	// Projecting the two boxes onto Box0's X axis.  If their intervals
	// on this line do not intersect, the boxes are not intersecting!
	// Each of the tests in this function work in a similar way.
	/////////////////////////////////////////////////////////////////////////
	SepAxis = A0;

	if (!Oriented_Boxes_Collide_On_Axis(box0,v0,box1,v1,SepAxis,dt)) {
		intersect = false;
		goto exit;
	}

	/////////////////////////////////////////////////////////////////////////
	// L = A1
	// Separating Axis is Box0's Y axis
	/////////////////////////////////////////////////////////////////////////
	SepAxis = A1;

	if (!Oriented_Boxes_Collide_On_Axis(box0,v0,box1,v1,SepAxis,dt)) {
		intersect = false;
		goto exit;
	}

	/////////////////////////////////////////////////////////////////////////
	// L = A2
	// Separating Axis is Box0's Z axis
	/////////////////////////////////////////////////////////////////////////
	SepAxis = A2;

	if (!Oriented_Boxes_Collide_On_Axis(box0,v0,box1,v1,SepAxis,dt)) {
		intersect = false;
		goto exit;
	}

	/////////////////////////////////////////////////////////////////////////
	// L = B0
	// Separating Axis is Box1's X axis
	/////////////////////////////////////////////////////////////////////////
	SepAxis = B0;

	if (!Oriented_Boxes_Collide_On_Axis(box0,v0,box1,v1,SepAxis,dt)) {
		intersect = false;
		goto exit;
	}

	/////////////////////////////////////////////////////////////////////////
	// L = B1
	// Separating Axis is Box1's Y axis
	/////////////////////////////////////////////////////////////////////////
	SepAxis = B1;

	if (!Oriented_Boxes_Collide_On_Axis(box0,v0,box1,v1,SepAxis,dt)) {
		intersect = false;
		goto exit;
	}

	/////////////////////////////////////////////////////////////////////////
	// L = B2
	// Separating Axis is Box1's Z axis
	/////////////////////////////////////////////////////////////////////////
	SepAxis = B2;

	if (!Oriented_Boxes_Collide_On_Axis(box0,v0,box1,v1,SepAxis,dt)) {
		intersect = false;
		goto exit;
	}

	/////////////////////////////////////////////////////////////////////////
	// None of the aligned axes turned out to be separating axes.  Now
	// we check all combinations of cross products of the two boxes axes.
	/////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////////////
	// L = A0xB0
	/////////////////////////////////////////////////////////////////////////
	Vector3::Cross_Product(A0,B0,&SepAxis);

	if (!Oriented_Boxes_Collide_On_Axis(box0,v0,box1,v1,SepAxis,dt)) {
		intersect = false;
		goto exit;
	}

	/////////////////////////////////////////////////////////////////////////
	// L = A0xB1
	/////////////////////////////////////////////////////////////////////////
	Vector3::Cross_Product(A0,B1,&SepAxis);

	if (!Oriented_Boxes_Collide_On_Axis(box0,v0,box1,v1,SepAxis,dt)) {
		intersect = false;
		goto exit;
	}

	/////////////////////////////////////////////////////////////////////////
	// L = A0xB2
	/////////////////////////////////////////////////////////////////////////
	Vector3::Cross_Product(A0,B2,&SepAxis);

	if (!Oriented_Boxes_Collide_On_Axis(box0,v0,box1,v1,SepAxis,dt)) {
		intersect = false;
		goto exit;
	}

	/////////////////////////////////////////////////////////////////////////
	// L = A1xB0
	/////////////////////////////////////////////////////////////////////////
	Vector3::Cross_Product(A1,B0,&SepAxis);

	if (!Oriented_Boxes_Collide_On_Axis(box0,v0,box1,v1,SepAxis,dt)) {
		intersect = false;
		goto exit;
	}

	/////////////////////////////////////////////////////////////////////////
	// L = A1xB1
	/////////////////////////////////////////////////////////////////////////
	Vector3::Cross_Product(A1,B1,&SepAxis);

	if (!Oriented_Boxes_Collide_On_Axis(box0,v0,box1,v1,SepAxis,dt)) {
		intersect = false;
		goto exit;
	}

	/////////////////////////////////////////////////////////////////////////
	// L = A1xB2
	/////////////////////////////////////////////////////////////////////////
	Vector3::Cross_Product(A1,B2,&SepAxis);

	if (!Oriented_Boxes_Collide_On_Axis(box0,v0,box1,v1,SepAxis,dt)) {
		intersect = false;
		goto exit;
	}

	/////////////////////////////////////////////////////////////////////////
	// L = A2xB0
	/////////////////////////////////////////////////////////////////////////
	Vector3::Cross_Product(A2,B0,&SepAxis);

	if (!Oriented_Boxes_Collide_On_Axis(box0,v0,box1,v1,SepAxis,dt)) {
		intersect = false;
		goto exit;
	}

	/////////////////////////////////////////////////////////////////////////
	// L = A2xB1
	/////////////////////////////////////////////////////////////////////////
	Vector3::Cross_Product(A2,B1,&SepAxis);

	if (!Oriented_Boxes_Collide_On_Axis(box0,v0,box1,v1,SepAxis,dt)) {
		intersect = false;
		goto exit;
	}

	/////////////////////////////////////////////////////////////////////////
	// L = A2xB2
	/////////////////////////////////////////////////////////////////////////
	Vector3::Cross_Product(A2,B2,&SepAxis);

	if (!Oriented_Boxes_Collide_On_Axis(box0,v0,box1,v1,SepAxis,dt)) {
		intersect = false;
		goto exit;
	}

exit:

	return intersect;
}



/***********************************************************************************************
 * Oriented_Box_Intersects_Tri_On_Axis -- tests if the box and tri intersect on the axis       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/7/99     GTH : Created.                                                                 *
 *=============================================================================================*/
bool Oriented_Box_Intersects_Tri_On_Axis(const OBBoxClass & box,const TriClass & tri,Vector3 & axis)
{
	float leb;		// leading edge of box (farthest point from center)
	float lep;		// leading edge of poly (closest point to center)
	float dist;		// distance from box center to v0
	float tmp;

	if (axis.Length2() < WWMATH_EPSILON) return true;

	Vector3 D = *(tri.V[0]) - box.Center;
	Vector3 r1 = *(tri.V[1]) - *(tri.V[0]);
	Vector3 r2 = *(tri.V[2]) - *(tri.V[0]);

	// I want the axis to point from box.center to tri.v0
	dist = Vector3::Dot_Product(D,axis);
	if (dist < 0) {
		dist = -dist;
		axis = -axis;
	}

	// compute leading edge of the box
	leb = box.Project_To_Axis(axis);

	// compute the leading edge of the triangle
	lep = 0;
	tmp = Vector3::Dot_Product(r1,axis); if (tmp < lep) lep = tmp;
	tmp = Vector3::Dot_Product(r2,axis); if (tmp < lep) lep = tmp;
	lep += dist;

	if (lep >= leb) {
		return false;
	} else {
		return true;
	}
}


/***********************************************************************************************
 * Oriented_Box_Intersects_Tri -- tests if the given box and tri intersect                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/7/99     GTH : Created.                                                                 *
 *=============================================================================================*/
bool Oriented_Box_Intersects_Tri(const OBBoxClass & box,const TriClass & tri)
{
	Vector3 axis;

	// vectors for the axis directions of the two boxes in world space
	Vector3 A[3];
	Vector3 E[3];
	Vector3 normal = *tri.N;
	A[0].Set(box.Basis[0][0],box.Basis[1][0],box.Basis[2][0]);
	A[1].Set(box.Basis[0][1],box.Basis[1][1],box.Basis[2][1]);
	A[2].Set(box.Basis[0][2],box.Basis[1][2],box.Basis[2][2]);
	E[0] = *(tri.V[1]) - *(tri.V[0]);
	E[1] = *(tri.V[2]) - *(tri.V[1]);
	E[2] = *(tri.V[0]) - *(tri.V[2]);

	if (!Oriented_Box_Intersects_Tri_On_Axis(box,tri,normal)) return false;
	if (!Oriented_Box_Intersects_Tri_On_Axis(box,tri,A[0])) return false;
	if (!Oriented_Box_Intersects_Tri_On_Axis(box,tri,A[1])) return false;
	if (!Oriented_Box_Intersects_Tri_On_Axis(box,tri,A[2])) return false;

	Vector3::Cross_Product(A[0],E[0],&axis);
	if (!Oriented_Box_Intersects_Tri_On_Axis(box,tri,axis)) return false;

	Vector3::Cross_Product(A[0],E[1],&axis);
	if (!Oriented_Box_Intersects_Tri_On_Axis(box,tri,axis)) return false;

	Vector3::Cross_Product(A[0],E[2],&axis);
	if (!Oriented_Box_Intersects_Tri_On_Axis(box,tri,axis)) return false;

	Vector3::Cross_Product(A[1],E[0],&axis);
	if (!Oriented_Box_Intersects_Tri_On_Axis(box,tri,axis)) return false;

	Vector3::Cross_Product(A[1],E[1],&axis);
	if (!Oriented_Box_Intersects_Tri_On_Axis(box,tri,axis)) return false;

	Vector3::Cross_Product(A[1],E[2],&axis);
	if (!Oriented_Box_Intersects_Tri_On_Axis(box,tri,axis)) return false;

	Vector3::Cross_Product(A[2],E[0],&axis);
	if (!Oriented_Box_Intersects_Tri_On_Axis(box,tri,axis)) return false;

	Vector3::Cross_Product(A[2],E[1],&axis);
	if (!Oriented_Box_Intersects_Tri_On_Axis(box,tri,axis)) return false;

	Vector3::Cross_Product(A[2],E[2],&axis);
	if (!Oriented_Box_Intersects_Tri_On_Axis(box,tri,axis)) return false;

	return true;
}
