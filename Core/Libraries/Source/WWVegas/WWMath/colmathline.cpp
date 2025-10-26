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
 *                     $Archive:: /VSS_Sync/wwmath/colmathline.cpp                            $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 8/29/01 10:25p                                              $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "colmath.h"
#include "aaplane.h"
#include "plane.h"
#include "lineseg.h"
#include "tri.h"
#include "sphere.h"
#include "aabox.h"
#include "obbox.h"
#include "wwdebug.h"


/*
** Structure used in the line->box test.  There was a lot of common code between the axis-
** aligned and oriented box tests so I package all of the truely relevant information into
** this struct and pass it into a function local to this module.  In the case of oriented
** boxes, the ray must be transformed into the box's coordinate system prior to the call
** and the normal is calculated slightly differently.
*/
struct BoxTestStruct
{
	Vector3		Min;
	Vector3		Max;
	Vector3		P0;
	Vector3		DP;
	float			Fraction;
	bool			Inside;
	int			Axis;
	int			Side;
};

/*
** Enumeration which can be used to categorize a point with respect to the projection
** of a box onto an axis.  The point will either be to the negative side of the span, in the
** span, or on the positive side of the span.
*/
enum BoxSideType {
	BOX_SIDE_NEGATIVE = 0,
	BOX_SIDE_POSITIVE = 1,
	BOX_SIDE_MIDDLE = 2
};


/*
** Table of normals for an axis aligned box.
** access like this:
**
** _box_normal[AXIS][SIDE]
**
** <AXIS> is 0,1,2 meaning x,y,z
** <SIDE> is BOX_SIDE_NEGATIVE or BOX_SIDE_POSITIVE
*/
static Vector3 _box_normal[3][2] =
{
	// plane = 0 (x axis)
	{
		Vector3(-1,0,0),	// Left
		Vector3(1,0,0) 	// Right
	},
	// plane = 1 (y axis)
	{
		Vector3(0,-1,0),
		Vector3(0,1,0)
	},
	// plane = 2 (z axis)
	{
		Vector3(0,0,-1),
		Vector3(0,0,1)
	}
};

/*
** Local function prototypes
*/
inline bool Test_Aligned_Box(BoxTestStruct * test);


bool CollisionMath::Collide(const LineSegClass & line,const AAPlaneClass & plane,CastResultStruct * result)
{
	float num,den,t;

	den = line.Get_DP()[plane.Normal];

	/*
	** Check if line is parallel to this plane
	*/
	if (den == 0.0f) {
		return false;
	}

	num = -(line.Get_P0()[plane.Normal] - plane.Dist);
	t = num/den;

	/*
	** If t is not between 0 and 1, the line containing the segment intersects
	** the plane but the segment does not
	*/
	if ((t < 0.0f) || (t > 1.0f)) {
		return false;
	}

	/*
	** Ok, we hit the plane!
	*/
	if (t < result->Fraction) {
		result->Fraction = t;
		result->Normal.Set(0,0,0);
		result->Normal[plane.Normal] = 1.0f;
		return true;
	}
	return false;
}


bool CollisionMath::Collide(const LineSegClass & line,const PlaneClass & plane,CastResultStruct * result)
{
	float num,den,t;
	den = Vector3::Dot_Product(plane.N,line.Get_DP());

	/*
	** If the denominator is zero, the ray is parallel to the plane
	*/
	if (den == 0.0f) {
		return false;
	}

	num = -(Vector3::Dot_Product(plane.N,line.Get_P0()) - plane.D);
	t = num/den;

	/*
	** If t is not between 0 and 1, the line containing the segment intersects
	** the plane but the segment does not
	*/
	if ((t < 0.0f) || (t > 1.0f)) {
		return false;
	}

	/*
	** Ok, we hit the plane!
	*/
	if (t < result->Fraction) {
		result->Fraction = t;
		result->Normal = plane.N;

		/*
		** if user is asking for the point, compute it.
		*/
		if (result->ComputeContactPoint) {
			result->ContactPoint = line.Get_P0() + result->Fraction * line.Get_DP();
		}
		return true;
	}
	return false;
}

bool CollisionMath::Collide(const LineSegClass & line,const TriClass & tri,CastResultStruct * res)
{
	TRACK_COLLISION_RAY_TRI;

	/*
	** Compute intersection of the line with the plane of the polygon
	*/
	PlaneClass plane(*tri.N,*tri.V[0]);
	Vector3 ipoint;
	float num,den,t;

	den = Vector3::Dot_Product(plane.N,line.Get_DP());

	/*
	** If the denominator is zero, the ray is parallel to the plane
	*/
	if (den == 0.0f) {
		return false;
	}
	num = -(Vector3::Dot_Product(plane.N,line.Get_P0()) - plane.D);
	t = num/den;

	/*
	** If t is not between 0 and 1, the line containing the segment intersects
	** the plane but the segment does not
	*/
	if ((t < 0.0f) || (t > 1.0f)) {
		return false;
	}

	ipoint = line.Get_P0() + t*line.Get_DP();

	/*
	** Check if this point is inside the triangle
	*/
	if (!tri.Contains_Point(ipoint)) {
		return false;
	}

	/*
	** Ok, we hit the triangle, set the collision results
	*/
	if (t < res->Fraction) {
		res->Fraction = t;
		res->Normal = plane.N;
		if (res->ComputeContactPoint) {
			res->ContactPoint = line.Get_P0() + res->Fraction * line.Get_DP();
		}
		TRACK_COLLISION_RAY_TRI_HIT;
		return true;
	}
	return false;
}

bool CollisionMath::Collide(const LineSegClass & line,const SphereClass & sphere,CastResultStruct * res)
{
	// this game from graphics gems 1, page 388
	// intersection of a ray with a sphere
	Vector3 dc = sphere.Center - line.Get_P0();
	float clen = Vector3::Dot_Product((dc) , line.Get_Dir());
	float disc = (sphere.Radius * sphere.Radius) - (dc.Length2() - clen*clen);
	if (disc < 0.0f) {
		return false;
	} else {
		float d = WWMath::Sqrt(disc);
		float frac = (clen - d) / line.Get_Length();
		if (frac<0.0f)
			frac = (clen + d) / line.Get_Length();
		if (frac<0.0f) return false;
		if (frac < res->Fraction) {

			res->Fraction = frac;

			Vector3 p = line.Get_P0() + (clen - d)*line.Get_Dir();
			Vector3 norm = p - sphere.Center;
			norm /= sphere.Radius;

			res->Normal = norm;
			if (res->ComputeContactPoint) {
				res->ContactPoint = line.Get_P0() + res->Fraction * line.Get_DP();
			}
			return true;
		}
	}
	return false;
}

bool CollisionMath::Collide(const LineSegClass & line,const AABoxClass & box,CastResultStruct * res)
{
	// set up the test struct
	BoxTestStruct test;
	test.Min = box.Center - box.Extent;
	test.Max = box.Center + box.Extent;
	test.P0 = line.Get_P0();
	test.DP = line.Get_DP();

	// check ray against the box, exit if the ray totally missed the box,
	if (!Test_Aligned_Box(&test)) {
		return false;
	}

	// if ray starts inside the box, note that fact and bail.
	if (test.Inside) {
		res->StartBad = true;
		return true;
	}

	// Now, if this intersection is before any current intersection
	// that we've found, install our intersection
	if (test.Fraction < res->Fraction) {
		res->Fraction = test.Fraction;
		assert(test.Side != BOX_SIDE_MIDDLE);
		res->Normal = _box_normal[test.Axis][test.Side];
		if (res->ComputeContactPoint) {
			res->ContactPoint = line.Get_P0() + res->Fraction * line.Get_DP();
		}
		return true;
	}
	return false;
}

bool CollisionMath::Collide(const LineSegClass & line,const OBBoxClass & box,CastResultStruct * result)
{
	// set up the test struct
	BoxTestStruct test;
	test.Min = box.Center - box.Extent;
	test.Max = box.Center + box.Extent;

	test.P0 = (box.Basis.Transpose() * (line.Get_P0() - box.Center)) + box.Center;
	test.DP = box.Basis.Transpose() * line.Get_DP();

	// check ray against the box, exit if the ray totally missed the box,
	if (!Test_Aligned_Box(&test)) {
		return false;
	}

	// if ray starts inside the box, don't collide
	if (test.Inside) {
		result->StartBad = true;
		return true;
	}

	// Now, if this intersection is before any current intersection
	// that we've found, install our intersection
	if (test.Fraction < result->Fraction) {
		result->Fraction = test.Fraction;
		assert(test.Side != BOX_SIDE_MIDDLE);

		switch (test.Axis) {
			case 0:
				result->Normal.Set(box.Basis[0][0],box.Basis[1][0],box.Basis[2][0]);
				break;

			case 1:
				result->Normal.Set(box.Basis[0][1],box.Basis[1][1],box.Basis[2][1]);
				break;

			case 2:
				result->Normal.Set(box.Basis[0][2],box.Basis[1][2],box.Basis[2][2]);
				break;
		}

		if (test.Side == BOX_SIDE_NEGATIVE) {
			result->Normal = -result->Normal;
		}
		if (result->ComputeContactPoint) {
			result->ContactPoint = line.Get_P0() + result->Fraction * line.Get_DP();
		}
		return true;
	}
	return false;
}



/***********************************************************************************************
 * Test_Aligned_Box -- used as the guts of the Box intersection tests                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/20/98    GTH : Created.                                                                 *
 *=============================================================================================*/
inline bool Test_Aligned_Box(BoxTestStruct * test)
{
	int i;

	// candidate intersection plane distance for each axis
	float candidateplane[3];

	// t value along the ray for each axis
	float maxt[3];

	// intersection point
	Vector3 coord;

	// which "side" of the box for each axis
	int quadrant[3];
	bool inside = true;

	for (i=0; i<3; i++) {
		if (test->P0[i] < test->Min[i]) {
			quadrant[i] = BOX_SIDE_NEGATIVE;
			candidateplane[i] = test->Min[i];
			inside = false;
		} else if (test->P0[i] > test->Max[i]) {
			quadrant[i] = BOX_SIDE_POSITIVE;
			candidateplane[i] = test->Max[i];
			inside = false;
		} else {
			quadrant[i] = BOX_SIDE_MIDDLE;
		}
	}

	/*
	** Ray started out inside the box...
	*/
	if (inside) {
		test->Fraction = 0.0f;
		test->Inside = true;
		return true;
	}

	/*
	** Calculate the 3 distances to the candidate planes
	*/
	for (i=0; i<3; i++) {
		if (quadrant[i] != BOX_SIDE_MIDDLE && test->DP[i] != 0.0f) {
			maxt[i] = (candidateplane[i] - test->P0[i]) / test->DP[i];
		} else {
			maxt[i] = -1.0f;
		}
	}

	/*
	** Get the largest of the maxt's for the final choice of intersection
	*/
	int intersection_plane = 0;

	for (i=1; i<3; i++) {
		if (maxt[i] > maxt[intersection_plane]) {
			intersection_plane = i;
		}
	}

	/*
	** If the ray is "in front" of all of the planes, return
	*/
	if (maxt[intersection_plane] < 0.0f) {
		return false;
	}

	/*
	** Check if the ray is inside the box, i.e. on the two planes which
	** are not the intersection planes, the intersection point should
	** be between the min and max of the box.
	*/
	for (i=0; i<3; i++) {

		if (intersection_plane != i) {

			coord[i] = test->P0[i] + maxt[intersection_plane] * test->DP[i];

			if ((coord[i] < test->Min[i]) || (coord[i] > test->Max[i])) {

				return false;		// ray is outside the box

			}

		} else {

			coord[i] = candidateplane[i];

		}
	}

	/*
	** Fill in the intersection values
	*/
	test->Fraction = maxt[intersection_plane];
	test->Inside = false;
	test->Axis = intersection_plane;
	test->Side = quadrant[intersection_plane];
	return true;
}




