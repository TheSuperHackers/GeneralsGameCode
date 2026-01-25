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
 *                     $Archive:: /Commando/Code/wwmath/tri.h                                 $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 5/04/01 8:35p                                               $*
 *                                                                                             *
 *                    $Revision:: 14                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "vector4.h"
#include "vector3.h"
#include "vector2.h"
#include <assert.h>


/**
** TriClass
** When the math library needs to deal with triangles, this will be the form used.
** The initial reason for this class is for Commando's collision detection code.  Initially
** I wrote custom code inside the render object for the terrain to perform collision
** detection. Moving the low-level geometrical collision code into the math library makes it
** more re-usable and independent from changes in the rendering code.
*/
class TriClass
{
public:

	const Vector3 *	N;
	const Vector3 *	V[3];

	void Compute_Normal()
	{
		assert(N);
		assert(V[0]);
		assert(V[1]);
		assert(V[2]);

		Vector3::Cross_Product(*(V[1])-*(V[0]),*(V[2])-*(V[0]),(Vector3 *)N);
		((Vector3 *)N)->Normalize();
	}


	bool Contains_Point(const Vector3 & ipoint) const;
	void Find_Dominant_Plane(int * axis1,int * axis2) const;
};


/*
** Utility functions:
** Functions which have to do with triangles but not neccessarily with TriClass - usually used
** by TriClass but also available for use elsewhere.
*/

// Enums for raycast flags (currently used only for semi-infinite axis-aligned rays)
enum
{
	TRI_RAYCAST_FLAG_NONE			= 0x00,
	TRI_RAYCAST_FLAG_HIT_EDGE		= 0x01,
	TRI_RAYCAST_FLAG_START_IN_TRI	= 0x02
};

// This function does a pure 2D point-in-triangle test. We pass in 3D points both for the
// triangle and test points, but we also pass in the two axes to use (the third axis is ignored,
// so we are actually testing the projection of the four points onto one of the axis planes). The
// triangle points are not assumed to be in any particular winding order (that is checked for
// internally). It is used internally by TriClass::Contains_Point(), and may be used elsewhere.
// If the ray intersects the camera at an edge we also count it as an intersection. We set a bit
// in 'flags' to true in this case (some users of this function need this extra information and
// it is very cheap to compute). We do not modify 'flags' if no edges are hit, so it must be
// initialized outside this function if its value is to be used.
inline bool Point_In_Triangle_2D(const Vector3 &tri_point0, const Vector3 &tri_point1,
	const Vector3 &tri_point2, const Vector3 &test_point, int axis_1, int axis_2,
	unsigned char &flags)
{
	// The function is based on checking signs of determinants, or in a more visually intuitive
	// sense, checking on which side of a line a point lies. For example, if the points run in
	// counter-clockwise order, the interior of the triangle is the intersection of the three
	// half-planes to the left of the directed infinite lines along P0 to P1, P1 to P2, P2 to P0.
	// Therefore the test point is in the triangle iff it is to the left of all three lines (if
	// the points are in clockwise order, we check if it is to the right of the lines).
	Vector2 p0p1(tri_point1[axis_1] - tri_point0[axis_1], tri_point1[axis_2] - tri_point0[axis_2]);
	Vector2 p1p2(tri_point2[axis_1] - tri_point1[axis_1], tri_point2[axis_2] - tri_point1[axis_2]);
	Vector2 p2p0(tri_point0[axis_1] - tri_point2[axis_1], tri_point0[axis_2] - tri_point2[axis_2]);

	// Check which side P2 is relative to P0P1. The sign of this test must equal the sign of all
	// three tests between the lines and the test point for the test point to be inside the
	// triangle. (this test will also tell us if the three points are colinear - if the triangle
	// is degenerate).
	Vector2 p0p2(tri_point2[axis_1] - tri_point0[axis_1], tri_point2[axis_2] - tri_point0[axis_2]);
	float p0p1p2 = Vector2::Perp_Dot_Product(p0p1, p0p2);
	if (p0p1p2 != 0.0f) {

		// The triangle is not degenerate - test three sides
		float side_factor = p0p1p2 > 0.0f ? 1.0f : -1.0f;
		float factors[3];

		// Now perform tests
		Vector2 p0pT(test_point[axis_1] - tri_point0[axis_1], test_point[axis_2] - tri_point0[axis_2]);
		factors[0] = Vector2::Perp_Dot_Product(p0p1, p0pT);
		if (factors[0] * side_factor < 0.0f) {
			return false;
		}
		Vector2 p1pT(test_point[axis_1] - tri_point1[axis_1], test_point[axis_2] - tri_point1[axis_2]);
		factors[1] = Vector2::Perp_Dot_Product(p1p2, p1pT);
		if (factors[1] * side_factor < 0.0f) {
			return false;
		}
		Vector2 p2pT(test_point[axis_1] - tri_point2[axis_1], test_point[axis_2] - tri_point2[axis_2]);
		factors[2] = Vector2::Perp_Dot_Product(p2p0, p2pT);
		if (factors[2] * side_factor < 0.0f) {
			return false;
		}
		if ((factors[0] == 0.0f) || (factors[1] == 0.0f) || (factors[2] == 0.0f)) flags |= TRI_RAYCAST_FLAG_HIT_EDGE;
		return true;

	} else {

		// The triangle is degenerate. This should be a rare case, so it does not matter much if it
		// is a little slower than the non-colinear case.

		// Find the two outer points along the triangle's line ('start' and 'end' points)
		float p0p1dist2 = p0p1.Length2();
		float p1p2dist2 = p1p2.Length2();
		float p2p0dist2 = p1p2.Length2();
		float max_dist2;
		Vector2 pSpE, pSpT;	// 'end' point, test point - both in 'start' points' frame
		if (p0p1dist2 > p1p2dist2) {
			if (p0p1dist2 > p2p0dist2) {
				// points 0 and 1 are the 'outer' points. 0 is 'start' and 1 is 'end'.
				pSpE = p0p1;
				pSpT.Set(test_point[axis_1] - tri_point0[axis_1], test_point[axis_2] - tri_point0[axis_2]);
				max_dist2 = p0p1dist2;
			} else {
				// points 0 and 2 are the 'outer' points. 2 is 'start' and 0 is 'end'.
				pSpE = p2p0;
				pSpT.Set(test_point[axis_1] - tri_point2[axis_1], test_point[axis_2] - tri_point2[axis_2]);
				max_dist2 = p2p0dist2;
			}
		} else {
			if (p1p2dist2 > p2p0dist2) {
				// points 1 and 2 are the 'outer' points. 1 is 'start' and 2 is 'end'.
				pSpE = p1p2;
				pSpT.Set(test_point[axis_1] - tri_point1[axis_1], test_point[axis_2] - tri_point1[axis_2]);
				max_dist2 = p1p2dist2;
			} else {
				// points 0 and 2 are the 'outer' points. 2 is 'start' and 0 is 'end'.
				pSpE = p2p0;
				pSpT.Set(test_point[axis_1] - tri_point2[axis_1], test_point[axis_2] - tri_point2[axis_2]);
				max_dist2 = p2p0dist2;
			}
		}

		if (max_dist2 != 0.0f) {
			// Triangle is line segment, check if test point is colinear with it
			if (Vector2::Perp_Dot_Product(pSpE, pSpT)) {
				// Not colinear
				return false;
			} else {
				// Colinear - is test point's distance from start and end <= segment length?
				Vector2 pEpT = pSpT - pSpE;
				if (pSpT.Length2() <= max_dist2 && pEpT.Length2() <= max_dist2) {
					flags |= TRI_RAYCAST_FLAG_HIT_EDGE;
					return true;
				} else {
					return false;
				}
			}
		} else {
			// All triangle points coincide, check if test point coincides with them
			if (pSpT.Length2() == 0.0f) {
				flags |= TRI_RAYCAST_FLAG_HIT_EDGE;
				return true;
			} else {
				return false;
			}
		}
	}
}


// This function tests a semi-infinite axis-aligned ray vs. a triangle.
// The inputs are blah blah blah
inline bool Cast_Semi_Infinite_Axis_Aligned_Ray_To_Triangle(const Vector3 &tri_point0,
	const Vector3 &tri_point1, const Vector3 &tri_point2, const Vector4 &tri_plane,
	const Vector3 &ray_start, int axis_r, int axis_1, int axis_2, int direction,
	unsigned char & flags)
{
	bool retval = false;

	// First check infinite ray vs. triangle (2D check)
	unsigned char flags_2d = TRI_RAYCAST_FLAG_NONE;
	if (Point_In_Triangle_2D(tri_point0, tri_point1, tri_point2, ray_start, axis_1, axis_2, flags_2d)) {

		// NOTE: SR plane equations, unlike WWMath's PlaneClass, use the Ax+By+Cz+D = 0
		// representation. It can also be viewed as C0x+C1y+C2z+C3 = dist where dist is the
		// signed distance from the plane (and therefore the plane is defined as those points
		// where dist = 0). Now that we know that the projection along the ray is inside the
		// triangle, determining whether the ray hits the triangle is a matter of determining
		// whether the start point is on the triangle plane's "anti-rayward side" (the side
		// which the ray points away from).
		// To determine this, we will use C[axis_r] (the plane coefficient of the ray axis).
		// This coefficient is positive if the positive direction of the ray axis points into
		// the planes' positive halfspace and negative if it points into the planes' negative
		// halfspace (it is zero if the ray axis is parallel to the triangle plane). If we
		// multiply this by 'sign' which is defined to be -1.0 if 'direction' equals 0 and
		// 1.0 if 'direction' equals 1, we will get a number which is positive or negative
		// depending on which halfspace the ray itself (as opposed to the rays axis) points
		// towards. If we further multiply this by dist(start point) - the result of plugging
		// the start point into the plane equation - we will get a number which is positive
		// if the start point is on the 'rayward side' (ray does not intersect the triangle)
		// and is negative if the start point is on the 'anti-rayward side' (ray does
		// intersect triangle). In either of these two cases we are done.
		// (see below for what happens when the result is zero - more checks need to be made).
		static const float sign[2] = {-1.0f, 1.0f };
		float result = tri_plane[axis_r] * sign[direction] * (tri_plane.X * ray_start.X +
			tri_plane.Y * ray_start.Y + tri_plane.Z * ray_start.Z + tri_plane.W);
		if (result < 0.0f) {
			// Intersection!
			flags |= (flags_2d & TRI_RAYCAST_FLAG_HIT_EDGE);
			retval = true;
		} else {
			if (result == 0.0f) {
				// If the result is 0, this means either the ray is parallel to the triangle
				// plane or the start point is embedded in the triangle plane. Note that since
				// the start point passed the 2D check, then if the ray is parallel the start
				// point must also be embedded in the triangle plane. This leaves us with two
				// cases:
				// A) The ray is not parallel to the plane - in this case the start point is
				// embedded in the triangle. We report an intersection, bitwise OR the edge
				// result from the 2D check into the 'edge hit' flag and set the 'start in tri'
				// flag.
				// B) The ray is parallel to the plane. In this case the result of the 2D test
				// tells us that the infinite line intersects the triangle (actually is
				// embedded in the triangle along part of its length), but we do not know
				// whether the semi-infinite ray also does so. We simplify things by not
				// counting such an 'intersection'. There are four reasons behind this:
				// 1. It differs from a 'normal' intersection (which has one intersecting
				// point) - there are infinitely many intersecting points.
				// 2. Moving the plane by an infinitesimally small amount to either side will
				// cause the ray to no longer touch the plane.
				// 3. This will not affect results for the known uses of this function.
				// 4. By doing so we avoid having to code up a bunch of complex tests.
				// Therefore in case B) we just report no intersection. We still need to find
				// out whether the point is embedded in the triangle (for setting the flag) so
				// we do another simple 2D test on the dominant plane.
				if (tri_plane[axis_r]) {
					// Case A)
					flags |= (flags_2d & TRI_RAYCAST_FLAG_HIT_EDGE);
					flags |= TRI_RAYCAST_FLAG_START_IN_TRI;
					retval = true;
				} else {
					// Case B) - test if point in tri (we know it is in plane, so we can use
					// TriClass function)
					TriClass tri;
					tri.V[0] = &tri_point0;
					tri.V[1] = &tri_point1;
					tri.V[2] = &tri_point2;
					tri.N = (Vector3*)&tri_plane;
					if (tri.Contains_Point(ray_start)) {
						flags |= TRI_RAYCAST_FLAG_START_IN_TRI;
					}
				}
			}
		}
	}

	return retval;
}
