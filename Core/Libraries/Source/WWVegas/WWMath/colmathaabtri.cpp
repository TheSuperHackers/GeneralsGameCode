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
 *                     $Archive:: /Commando/Code/wwmath/colmathaabtri.cpp                     $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 1/15/02 2:46p                                               $*
 *                                                                                             *
 *                    $Revision:: 19                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   aabtri_separation_test -- test the projected extents for separation                       *
 *   aabtri_check_axis -- project the aab and tri onto an arbitrary axis                       *
 *   aabtri_check_cross_axis -- projects aab and tri onto a "cross" axis                       *
 *   aabtri_check_basis_axis -- projects the aab and tri onto a basis axis                     *
 *   aabtri_check_normal_axis -- project the box and tri onto the tri-normal                   *
 *   eval_side -- returns -1,0,+1 depending on the sign of val and side                        *
 *   aabtri_compute_contact_normal -- computes the normal of the collision                     *
 *   CollisionMath::Collide -- collide an aabox into a triangle                                *
 *   aabtri_intersect_cross_axis -- intersection check for a "cross-product" axis              *
 *   aabtri_intersect_basis_axis -- intersection check for a basis axis                        *
 *   aabtri_intersect_normal_axis -- intersection check for the triangle normal                *
 *   CollisionMath::Intersection_Test -- Intersection check for an AABox and a triangle        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "colmath.h"
#include "aabox.h"
#include "tri.h"
#include "wwdebug.h"


/*
** Separating Axes have to be rejected if their length is smaller than some epsilon.
** Otherwise, erroneous results can be reported.
*/
#define AXISLEN_EPSILON2	WWMATH_EPSILON * WWMATH_EPSILON	// squared length of a separating axis must be larger than this


/*
** Axes used in Box-Tri intersection tests
** The axes of the box are A0,A1,A2.  N is the normal of the triangle,
** E0,E1,E2 are direction vectors for the edges of the triangle.
** (the box axes are labeled A0,A1,A2 as a holdover from the obbox-obbox
** collision code which this was derived from where there are two boxes
** A and B)
*/
enum
{
	INTERSECTION = 0,
	AXIS_N,					// normal of the triangle
	AXIS_A0,					// first basis vector of the box
	AXIS_A1,					// second basis vector of the box
	AXIS_A2,					// third basis vector of the box

	AXIS_A0E0,				// box0 x edge0...
	AXIS_A1E0,
	AXIS_A2E0,
	AXIS_A0E1,
	AXIS_A1E1,
	AXIS_A2E1,
	AXIS_A0E2,
	AXIS_A1E2,
	AXIS_A2E2
};


/******************************************************************************************

	AABox->Triangle collision

   This code is basically a special-case optimization of the OBBox->Triangle collision
	detection code.  There are many dot and cross products that can be simplified due
	to the fact that we know the axes of the boxes are always the same and are aligned
	with the world coordinate axes.

	Each axis test will use the following logic:
	Project D onto the axis being used, it is the separation distance.  If the
	projection of the extent of the box + the projection of the extent of the
	tri is greater than D*axis then the two intersect

	March 13, 2000 - Modified these routines to all use a static instance of
	the BTCollisionStruct.  The compiler was generating lots of extra code for the
	constructor of this object and testing determined that re-using the same static
	struct was slightly faster anyway.
	NOTE: this makes the code not Thread-Safe!!!!


******************************************************************************************/

/*
** BoxTriColStruct
** Scratchpad variables for the AABox-Triangle collision detection functions.  One instance
** of this structure will be used for all of the local variables and its pointer will be
** handed of to various inline functions for the axis tests.
** Note that much of the code needs the un-normalized triangle normal.  For this reason,
** I have to compute N rather than copying it from the triangle.  (commenting this to
** avoid re-generating a difficult to find bug that I had)
*/
struct BTCollisionStruct
{
	BTCollisionStruct(void) {}

	void Init(const AABoxClass &box,const Vector3 &move,const TriClass &tri,const Vector3 &trimove)
	{
		StartBad = true;			// true until an axis clears it
		MaxFrac = -0.01f;			// maximum move allowed so far
		AxisId = INTERSECTION;	// axis that allowed the longest move
		Point = 0;					// index of triangle point that was closest to the box
		Side = 0;					// side of the interval
		Box = &box;
		Tri = &tri;
		BoxMove = &move;
		TriMove = &trimove;

		Vector3::Subtract(*tri.V[0],box.Center,&D);				// vector from center of box to vertex 0
		Vector3::Subtract(move,trimove,&Move);						// move vector relative to stationary triangle

		Vector3::Subtract(*tri.V[1],*tri.V[0],&E[0]);
		Vector3::Subtract(*tri.V[2],*tri.V[0],&E[1]);
		Vector3::Subtract(E[1],E[0],&E[2]);

		Vector3::Cross_Product(E[0],E[1],&N);
	}

	bool						StartBad;			// Inital configuration is intersecting?
	float						MaxFrac;				// Longest move allowed so far

	int						AxisId;				// Last separating axis
	int						Side;					// which side of the interval
	int						Point;				// Index of the "closest" triangle point (or one of them)

	int						TestAxisId;			// Axis 'id' we're working on
	int						TestSide;			// Was the axis we're working on flipped
	int						TestPoint;			// Index of the closest vertex
	Vector3					TestAxis;			// Axis we're working on

	Vector3					D;						// Vector from the center of the box to v0
	Vector3					Move;					// Move vector relative to stationary triangle
	float						AE[3][3];			// Dot products of the Basis vectors and edges
	float						AN[3];				// Dot products of the Basis vectors and the normal
	Vector3					AxE[3][3];			// Cross products of the Basis vectors and edges

	Vector3					E[3];					// edge vectors for the triangle
	Vector3					N;						// normal (NOT normalized!!!)
	Vector3					FinalD;				// Vector from center of box to v0 at end of move

	const AABoxClass *	Box;
	const TriClass *		Tri;
	const Vector3 *		BoxMove;
	const Vector3 *		TriMove;

private:

	// not implemented
	BTCollisionStruct(const BTCollisionStruct &);
	BTCollisionStruct & operator = (const BTCollisionStruct &);
};

static BTCollisionStruct CollisionContext;

/***********************************************************************************************
 * aabtri_separation_test -- test the projected extents for separation                         *
 *                                                                                             *
 * Once the extents are projected onto the axis, this function contains                        *
 * the logic that determines the allowed fraction.                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/8/99     GTH : Created.                                                                 *
 *   7/12/99    GTH : Converted original OBBox code to AABox                                   *
 *=============================================================================================*/
static inline bool aabtri_separation_test
(
	float lp,float leb0,float leb1
)
{
	/*
	** - If (I'm no more than 'EPSILON' embedded in the wall)
	**   - not startbad
   **   - If (I'm moving towards)
	**     - fraction = How far I can move before embedding (can be negative if started embedded)
	**     - If (I can take entire move)
	**       - accept entire move
	**     - Else If (I can move more than I could have before; *negative always fails!)
	**       - update fraction
	**   - Else
	**     - Accept entire move since I'm not moving towards
	*/
	float eps = 0.0f;
	if (lp - leb0 <= 0.0f) {
		eps = COLLISION_EPSILON * CollisionContext.TestAxis.Length();	// trying to only compute epsilon if I have to
	}

	if (lp - leb0 > -eps) {
		CollisionContext.StartBad = false;
		if (leb1 - leb0 > 0.0f) {
			float frac = (lp-leb0)/(leb1-leb0);
			if (frac >= 1.0f) {
				/* moving toward but not hitting triangle */
				CollisionContext.AxisId = CollisionContext.TestAxisId;
				CollisionContext.MaxFrac = 1.0f;
				return true;
			} else {
				/* moving toward, hitting triangle */
				if (frac > CollisionContext.MaxFrac) {
					CollisionContext.MaxFrac = frac;
					CollisionContext.AxisId = CollisionContext.TestAxisId;
					CollisionContext.Side = CollisionContext.TestSide;
					CollisionContext.Point = CollisionContext.TestPoint;
				}
			}
		} else {
			/* moving away or not moving */
			CollisionContext.AxisId = CollisionContext.TestAxisId;
			CollisionContext.MaxFrac = 1.0f;
			return true;
		}
	}
	return false;
}


/***********************************************************************************************
 * aabtri_check_axis -- project the aab and tri onto an arbitrary axis                         *
 *                                                                                             *
 * projects the box and the triangle onto the given axis and calls                             *
 * obbtri_separation_test.  return true if separation was detected                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/8/99     GTH : Created.                                                                 *
 *   7/12/99    GTH : converted to AABox                                                       *
 *=============================================================================================*/
static inline bool aabtri_check_axis(void)
{
	float		dist;						// separation along the axis
	float		axismove;				// size of the move along the axis.
	float		leb0;						// initial coordinate of the leading edge of the box
	float		leb1;						// final coordinate of the leading edge of the box
	float		lp;						// leading edge of the polygon.
	float		tmp;						// temporary

	dist = Vector3::Dot_Product(CollisionContext.D,CollisionContext.TestAxis);
	axismove = Vector3::Dot_Product(CollisionContext.Move,CollisionContext.TestAxis);

	// I want the axis centered at the box, pointing towards the triangle
	if (dist < 0) {
		dist = -dist;
		axismove = -axismove;
		CollisionContext.TestAxis = -CollisionContext.TestAxis;
		CollisionContext.TestSide = -1.0f;
	} else {
		CollisionContext.TestSide = 1.0f;
	}

	// compute coordinates of the leading edge of the box at t0 and t1
	leb0 =	CollisionContext.Box->Extent.X * WWMath::Fabs(CollisionContext.TestAxis.X) +
				CollisionContext.Box->Extent.Y * WWMath::Fabs(CollisionContext.TestAxis.Y) +
				CollisionContext.Box->Extent.Z * WWMath::Fabs(CollisionContext.TestAxis.Z);
	leb1 = leb0 + axismove;

	// compute coordinate of "leading edge of the triangle" relative to the box center.
	lp = 0;
	tmp = Vector3::Dot_Product(CollisionContext.E[0],CollisionContext.TestAxis); if (tmp < lp) lp = tmp;
	tmp = Vector3::Dot_Product(CollisionContext.E[1],CollisionContext.TestAxis); if (tmp < lp) lp = tmp;
	lp = dist + lp;

	return aabtri_separation_test(/*CollisionContext,*/lp,leb0,leb1);
}


/***********************************************************************************************
 * aabtri_check_cross_axis -- projects aab and tri onto a "cross" axis                         *
 *                                                                                             *
 * Assumes that the axis given is one generated from a cross product of one of the edge and    *
 * basis vectors.  In this case, the box extent can be optimized and only two triangle verts   *
 * need to be checked.                                                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/8/99     GTH : Created.                                                                 *
 *=============================================================================================*/
static inline bool aabtri_check_cross_axis
(
	float						dp,
	int						dpi,
	float						leb0
)
{
	float		p0;						// distance from box center to vertex 0
	float		axismove;				// size of the move along the axis.
	float		leb1;						// final coordinate of the leading edge of the box
	float		lp;						// leading edge of the polygon.

	p0 = Vector3::Dot_Product(CollisionContext.D,CollisionContext.TestAxis);
	axismove = Vector3::Dot_Product(CollisionContext.Move,CollisionContext.TestAxis);

	// I want the axis centered at the box, pointing towards the triangle
	if (p0 < 0) {
		p0 = -p0;
		axismove = -axismove;
		dp = -dp;
		CollisionContext.TestAxis = -CollisionContext.TestAxis;
		CollisionContext.TestSide = -1.0f;
	} else {
		CollisionContext.TestSide = 1.0f;
	}

	// compute coordinates of the leading edge of the box at t1
	leb1 = leb0 + axismove;

	// compute coordinate of "leading edge of the triangle" relative to the box center.
	lp = 0; CollisionContext.TestPoint = 0;
	if (dp < 0) { lp = dp; CollisionContext.TestPoint = dpi; }
	lp = p0 + lp;

	return aabtri_separation_test(/*CollisionContext,*/lp,leb0,leb1);
}


/***********************************************************************************************
 * aabtri_check_basis_axis -- projects the aab and tri onto a basis axis                       *
 *                                                                                             *
 * Projects the box and triangle onto an axis that is assumed to be a basis                    *
 * vector from the box.  In this case, we can skip a dot-product and use the                   *
 * corresponding extent of the box (which is passed in as leb0).                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/8/99     GTH : Created.                                                                 *
 *=============================================================================================*/
static inline bool aabtri_check_basis_axis
(
	float leb0,
	float dp1,
	float dp2
)
{
	float		dist;						// separation along the axis
	float		axismove;				// size of the move along the axis.
	float		leb1;						// final coordinate of the leading edge of the box
	float		lp;						// leading edge of the polygon.

	dist = Vector3::Dot_Product(CollisionContext.D,CollisionContext.TestAxis);
	axismove = Vector3::Dot_Product(CollisionContext.Move,CollisionContext.TestAxis);

	// we want the axis centered at the box, pointing towards the triangle
	if (dist < 0) {
		dist = -dist;
		axismove = -axismove;
		dp1 = -dp1;
		dp2 = -dp2;
		CollisionContext.TestAxis = -CollisionContext.TestAxis;
		CollisionContext.TestSide = -1.0f;
	} else {
		CollisionContext.TestSide = 1.0f;
	}

	// this is the "optimization", leb0 = one of the extents
	leb1 = leb0 + axismove;

	// compute coordinate of "leading edge of the polygon" relative to the box center.
	lp = 0; CollisionContext.TestPoint = 0;
	if (dp1 < lp) { lp = dp1; CollisionContext.TestPoint = 1; }
	if (dp2 < lp) { lp = dp2; CollisionContext.TestPoint = 2; }
	lp = dist + lp;

	return aabtri_separation_test(/*CollisionContext,*/lp,leb0,leb1);
}


/***********************************************************************************************
 * aabtri_check_normal_axis -- project the box and tri onto the tri-normal                     *
 *                                                                                             *
 * Projects the box and triangle onto an axis that is assumed to be the normal                 *
 * vector from the triangle.  In this case, the triangle extents are zero.                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/8/99     GTH : Created.                                                                 *
 *=============================================================================================*/
static inline bool aabtri_check_normal_axis(void)
{
	float		dist;						// separation along the axis
	float		axismove;				// size of the move along the axis.
	float		leb0;						// initial coordinate of the leading edge of the box
	float		leb1;						// final coordinate of the leading edge of the box
	float		lp;						// leading edge of the polygon.

	dist = Vector3::Dot_Product(CollisionContext.D,CollisionContext.TestAxis);
	axismove = Vector3::Dot_Product(CollisionContext.Move,CollisionContext.TestAxis);

	// we want the axis centered at the box, pointing towards the triangle
	if (dist < 0) {
		dist = -dist;
		axismove = -axismove;
		CollisionContext.TestAxis = -CollisionContext.TestAxis;
		CollisionContext.TestSide = -1.0f;
	} else {
		CollisionContext.TestSide = 1.0f;
	}

	leb0 =	CollisionContext.Box->Extent.X * WWMath::Fabs(CollisionContext.AN[0]) +
				CollisionContext.Box->Extent.Y * WWMath::Fabs(CollisionContext.AN[1]) +
				CollisionContext.Box->Extent.Z * WWMath::Fabs(CollisionContext.AN[2]);
	leb1 = leb0 + axismove;
	CollisionContext.TestPoint = 0;
	lp = dist;	// this is the "optimization", don't have to find lp

	return aabtri_separation_test(/*CollisionContext,*/lp,leb0,leb1);
}

/***********************************************************************************************
 * eval_side -- returns -1,0,+1 depending on the sign of val and side                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/8/99     GTH : Created.                                                                 *
 *=============================================================================================*/
static inline float eval_side(float val,int side)
{
	if (val > 0.0f) {
		return side;
	} else if (val < 0.0f) {
		return -side;
	} else {
		return 0.0f;
	}
}

/***********************************************************************************************
 * aabtri_compute_contact_normal -- computes the normal of the collision                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/8/99     GTH : Created.                                                                 *
 *=============================================================================================*/
static inline void aabtri_compute_contact_normal
(
	Vector3 &						set_norm
)
{
#if 1
	switch(CollisionContext.AxisId)
	{
		case INTERSECTION:
			set_norm = CollisionContext.N;
			set_norm.Normalize();
			break;
		case AXIS_N:
			set_norm = -CollisionContext.Side * CollisionContext.N;
			set_norm.Normalize();
			break;
		case AXIS_A0:
			set_norm = -CollisionContext.Side * Vector3(1.0f,0.0f,0.0f);
			break;
		case AXIS_A1:
			set_norm = -CollisionContext.Side * Vector3(0.0f,1.0f,0.0f);
			break;
		case AXIS_A2:
			set_norm = -CollisionContext.Side * Vector3(0.0f,0.0f,1.0f);
			break;
		case AXIS_A0E0:
			set_norm = -CollisionContext.Side * CollisionContext.AxE[0][0];
			set_norm.Normalize();
			break;
		case AXIS_A1E0:
			set_norm = -CollisionContext.Side * CollisionContext.AxE[1][0];
			set_norm.Normalize();
			break;
		case AXIS_A2E0:
			set_norm = -CollisionContext.Side * CollisionContext.AxE[2][0];
			set_norm.Normalize();
			break;
		case AXIS_A0E1:
			set_norm = -CollisionContext.Side * CollisionContext.AxE[0][1];
			set_norm.Normalize();
			break;
		case AXIS_A1E1:
			set_norm = -CollisionContext.Side * CollisionContext.AxE[1][1];
			set_norm.Normalize();
			break;
		case AXIS_A2E1:
			set_norm = -CollisionContext.Side * CollisionContext.AxE[2][1];
			set_norm.Normalize();
			break;
		case AXIS_A0E2:
			set_norm = -CollisionContext.Side * CollisionContext.AxE[0][2];
			set_norm.Normalize();
			break;
		case AXIS_A1E2:
			set_norm = -CollisionContext.Side * CollisionContext.AxE[1][2];
			set_norm.Normalize();
			break;
		case AXIS_A2E2:
			set_norm = -CollisionContext.Side * CollisionContext.AxE[2][2];
			set_norm.Normalize();
			break;
	}

	WWASSERT(set_norm.Length2() > 0.0f);

#else
	set_norm = *CollisionContext.N;
	set_norm.Normalize();
	if (Vector3::Dot_Product(set_norm,CollisionContext.Move) > 0.0f) {
		set_norm = -(set_norm);
	}
#endif
}

inline void VERIFY_CROSS(const Vector3 & a, const Vector3 & b,const Vector3 & cross)
{
#ifdef WWDEBUG
	Vector3 tmp_cross;
	Vector3::Cross_Product(a,b,&tmp_cross);
	Vector3 diff = cross - tmp_cross;
	WWASSERT(WWMath::Fabs(diff.Length()) < 0.0001f);
#endif
}

/***********************************************************************************************
 * CollisionMath::Collide -- collide an aabox into a triangle                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/8/99     GTH : Created.                                                                 *
 *=============================================================================================*/
bool CollisionMath::Collide
(
	const AABoxClass &		box,
	const Vector3 &			move,
	const TriClass &			tri,
	CastResultStruct *		result
)
{
	TRACK_COLLISION_AABOX_TRI;

	float dp,leb0;

	CollisionContext.Init(box,move,tri,Vector3(0,0,0));

	/*
	** AXIS_N
	*/
	CollisionContext.TestAxis = CollisionContext.N;
	CollisionContext.TestAxisId = AXIS_N;
	CollisionContext.AN[0] = CollisionContext.N.X;
	CollisionContext.AN[1] = CollisionContext.N.Y;
	CollisionContext.AN[2] = CollisionContext.N.Z;
	if (aabtri_check_normal_axis()) goto exit;

	/*
	** AXIS_A0
	*/
	CollisionContext.TestAxis.Set(1,0,0);
	CollisionContext.TestAxisId = AXIS_A0;
	CollisionContext.AE[0][0] = CollisionContext.E[0].X;
	CollisionContext.AE[0][1] = CollisionContext.E[1].X;
	if (aabtri_check_basis_axis(box.Extent.X,CollisionContext.AE[0][0],CollisionContext.AE[0][1])) goto exit;

	/*
	** AXIS_A1
	*/
	CollisionContext.TestAxis.Set(0,1,0);
	CollisionContext.TestAxisId = AXIS_A1;
	CollisionContext.AE[1][0] = CollisionContext.E[0].Y;
	CollisionContext.AE[1][1] = CollisionContext.E[1].Y;
	if (aabtri_check_basis_axis(box.Extent.Y,CollisionContext.AE[1][0],CollisionContext.AE[1][1])) goto exit;

	/*
	** AXIS_A2
	*/
	CollisionContext.TestAxis.Set(0,0,1);
	CollisionContext.TestAxisId = AXIS_A2;
	CollisionContext.AE[2][0] = CollisionContext.E[0].Z;
	CollisionContext.AE[2][1] = CollisionContext.E[1].Z;
	if (aabtri_check_basis_axis(box.Extent.Z,CollisionContext.AE[2][0],CollisionContext.AE[2][1])) goto exit;

	/*
	** AXIS_A0xE0
	*/
	CollisionContext.AxE[0][0].Set(0,-CollisionContext.E[0].Z,CollisionContext.E[0].Y);
	VERIFY_CROSS(Vector3(1,0,0),CollisionContext.E[0],CollisionContext.AxE[0][0]);
	CollisionContext.TestAxis = CollisionContext.AxE[0][0];
	CollisionContext.TestAxisId = AXIS_A0E0;
	if (CollisionContext.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = CollisionContext.AN[0];
		leb0 = box.Extent[1]*WWMath::Fabs(CollisionContext.AE[2][0]) + box.Extent[2]*WWMath::Fabs(CollisionContext.AE[1][0]);
		if (aabtri_check_cross_axis(dp,2,leb0)) goto exit;
	}

	/*
	** AXIS_A0xE1
	*/
	CollisionContext.AxE[0][1].Set(0,-CollisionContext.E[1].Z,CollisionContext.E[1].Y);
	VERIFY_CROSS(Vector3(1,0,0),CollisionContext.E[1],CollisionContext.AxE[0][1]);
	CollisionContext.TestAxis = CollisionContext.AxE[0][1];
	CollisionContext.TestAxisId = AXIS_A0E1;
	if (CollisionContext.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = -CollisionContext.AN[0];
		leb0 = box.Extent[1]*WWMath::Fabs(CollisionContext.AE[2][1]) + box.Extent[2]*WWMath::Fabs(CollisionContext.AE[1][1]);
		if (aabtri_check_cross_axis(dp,1,leb0)) goto exit;
	}

	/*
	** AXIS_A0xE2
	*/
	CollisionContext.AE[0][2] = CollisionContext.E[2].X;
	CollisionContext.AE[1][2] = CollisionContext.E[2].Y;
	CollisionContext.AE[2][2] = CollisionContext.E[2].Z;

	CollisionContext.AxE[0][2].Set(0,-CollisionContext.E[2].Z,CollisionContext.E[2].Y);
	VERIFY_CROSS(Vector3(1,0,0),CollisionContext.E[2],CollisionContext.AxE[0][2]);
	CollisionContext.TestAxis = CollisionContext.AxE[0][2];
	CollisionContext.TestAxisId = AXIS_A0E2;

	if (CollisionContext.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = -CollisionContext.AN[0];
		leb0 = box.Extent[1]*WWMath::Fabs(CollisionContext.AE[2][2]) + box.Extent[2]*WWMath::Fabs(CollisionContext.AE[1][2]);
		if (aabtri_check_cross_axis(dp,1,leb0)) goto exit;
	}

	/*
	** AXIS_A1xE0
	*/
	CollisionContext.AxE[1][0].Set(CollisionContext.E[0].Z,0,-CollisionContext.E[0].X);
	VERIFY_CROSS(Vector3(0,1,0),CollisionContext.E[0],CollisionContext.AxE[1][0]);
	CollisionContext.TestAxis = CollisionContext.AxE[1][0];
	CollisionContext.TestAxisId = AXIS_A1E0;
	if (CollisionContext.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = CollisionContext.AN[1];
		leb0 = box.Extent[0]*WWMath::Fabs(CollisionContext.AE[2][0]) + box.Extent[2]*WWMath::Fabs(CollisionContext.AE[0][0]);
		if (aabtri_check_cross_axis(dp,2,leb0)) goto exit;
	}

	/*
	** AXIS_A1xE1
	*/
	CollisionContext.AxE[1][1].Set(CollisionContext.E[1].Z,0,-CollisionContext.E[1].X);
	VERIFY_CROSS(Vector3(0,1,0),CollisionContext.E[1],CollisionContext.AxE[1][1]);
	CollisionContext.TestAxis = CollisionContext.AxE[1][1];
	CollisionContext.TestAxisId = AXIS_A1E1;
	if (CollisionContext.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = -CollisionContext.AN[1];
		leb0 = box.Extent[0]*WWMath::Fabs(CollisionContext.AE[2][1]) + box.Extent[2]*WWMath::Fabs(CollisionContext.AE[0][1]);
		if (aabtri_check_cross_axis(dp,1,leb0)) goto exit;
	}

	/*
	** AXIS_A1xE2
	*/
	CollisionContext.AxE[1][2].Set(CollisionContext.E[2].Z,0,-CollisionContext.E[2].X);
	VERIFY_CROSS(Vector3(0,1,0),CollisionContext.E[2],CollisionContext.AxE[1][2]);
	CollisionContext.TestAxis = CollisionContext.AxE[1][2];
	CollisionContext.TestAxisId = AXIS_A1E2;
	if (CollisionContext.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = -CollisionContext.AN[1];
		leb0 = box.Extent[0]*WWMath::Fabs(CollisionContext.AE[2][2]) + box.Extent[2]*WWMath::Fabs(CollisionContext.AE[0][2]);
		if (aabtri_check_cross_axis(dp,1,leb0)) goto exit;
	}

	/*
	** AXIS_A2xE0
	*/
	CollisionContext.AxE[2][0].Set(-CollisionContext.E[0].Y,CollisionContext.E[0].X,0);
	VERIFY_CROSS(Vector3(0,0,1),CollisionContext.E[0],CollisionContext.AxE[2][0]);
	CollisionContext.TestAxis = CollisionContext.AxE[2][0];
	CollisionContext.TestAxisId = AXIS_A2E0;
	if (CollisionContext.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = CollisionContext.AN[2];
		leb0 = box.Extent[0]*WWMath::Fabs(CollisionContext.AE[1][0]) + box.Extent[1]*WWMath::Fabs(CollisionContext.AE[0][0]);
		if (aabtri_check_cross_axis(dp,2,leb0)) goto exit;
	}

	/*
	** AXIS_A2xE1
	*/
	CollisionContext.AxE[2][1].Set(-CollisionContext.E[1].Y,CollisionContext.E[1].X,0);
	VERIFY_CROSS(Vector3(0,0,1),CollisionContext.E[1],CollisionContext.AxE[2][1]);
	CollisionContext.TestAxis = CollisionContext.AxE[2][1];
	CollisionContext.TestAxisId = AXIS_A2E1;
	if (CollisionContext.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = -CollisionContext.AN[2];
		leb0 = box.Extent[0]*WWMath::Fabs(CollisionContext.AE[1][1]) + box.Extent[1]*WWMath::Fabs(CollisionContext.AE[0][1]);
		if (aabtri_check_cross_axis(dp,1,leb0)) goto exit;
	}

	/*
	** AXIS_A2xE2
	*/
	CollisionContext.AxE[2][2].Set(-CollisionContext.E[2].Y,CollisionContext.E[2].X,0);
	VERIFY_CROSS(Vector3(0,0,1),CollisionContext.E[2],CollisionContext.AxE[2][2]);
	CollisionContext.TestAxis = CollisionContext.AxE[2][2];
	CollisionContext.TestAxisId = AXIS_A2E2;
	if (CollisionContext.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = -CollisionContext.AN[2];
		leb0 = box.Extent[0]*WWMath::Fabs(CollisionContext.AE[1][2]) + box.Extent[1]*WWMath::Fabs(CollisionContext.AE[0][2]);
		if (aabtri_check_cross_axis(dp,1,leb0)) goto exit;
	}

	/*
	** Last ditch effort, check an axis based on the move vector
	*/
	if (!CollisionContext.StartBad) {
		CollisionContext.TestPoint = CollisionContext.Point;
		CollisionContext.TestAxisId = CollisionContext.AxisId;

		CollisionContext.TestAxis.Set(0,-CollisionContext.Move.Z,CollisionContext.Move.Y);						// A0 X Move
		VERIFY_CROSS(Vector3(1,0,0),CollisionContext.Move,CollisionContext.TestAxis);
		if (CollisionContext.TestAxis.Length2() > AXISLEN_EPSILON2) {
			if (aabtri_check_axis()) goto exit;
		}
		CollisionContext.TestAxis.Set(CollisionContext.Move.Z,0,-CollisionContext.Move.X);						// A1 X Move
		VERIFY_CROSS(Vector3(0,1,0),CollisionContext.Move,CollisionContext.TestAxis);
		if (CollisionContext.TestAxis.Length2() > AXISLEN_EPSILON2) {
			if (aabtri_check_axis()) goto exit;
		}
		CollisionContext.TestAxis.Set(-CollisionContext.Move.Y,CollisionContext.Move.X,0);						// A2 X Move
		VERIFY_CROSS(Vector3(0,0,1),CollisionContext.Move,CollisionContext.TestAxis);
		if (CollisionContext.TestAxis.Length2() > AXISLEN_EPSILON2) {
			if (aabtri_check_axis()) goto exit;
		}
	}

exit:

	/*
	** If MaxFrac is less than zero, clamp it to zero.  Negative fractions can
	** leak through this routine due to the epsilon in the separation test.
	*/
	if (CollisionContext.MaxFrac < 0.0f) {
		CollisionContext.MaxFrac = 0.0f;
	}

	/*
	** If the triangle and box are intersecting before the move, return that
	** result.
	*/
	if (CollisionContext.StartBad) {
		result->StartBad = true;
		result->Fraction = 0.0f;
		result->Normal = *tri.N;
		TRACK_COLLISION_AABOX_TRI_HIT;
		return true;
	}

	/*
	** If the fraction allowed is basically equal to the fraction allowed by
	** another polygon, try to pick the polygon which is least "edge-on" to the
	** move.
	*/
	if ((CollisionContext.MaxFrac <= result->Fraction) && (CollisionContext.MaxFrac < 1.0f)) {

		/*
		** Reflect the normal if it is pointing the same way as our move
		** (probably hitting the back side of a polygon)
		*/
		Vector3 tmp_norm(0.0f,0.0f,0.0f);
		aabtri_compute_contact_normal(tmp_norm);
//		if (Vector3::Dot_Product(tmp_norm,move) > 0.0f) {
//			tmp_norm = -tmp_norm;
//		}

		/*
		** If this polygon cuts off more of the move -OR- this polygon cuts
		** of the same amount but has a "better" normal, then use this normal
		*/
		if (	(WWMath::Fabs(CollisionContext.MaxFrac - result->Fraction) > WWMATH_EPSILON) ||
				(Vector3::Dot_Product(tmp_norm,move) < Vector3::Dot_Product(result->Normal,move)))
		{
			result->Normal = tmp_norm;
			WWASSERT(WWMath::Fabs(result->Normal.Length() - 1.0f) < WWMATH_EPSILON);
		}

		result->Fraction = CollisionContext.MaxFrac;

		TRACK_COLLISION_AABOX_TRI_HIT;
		return true;
	}

	return false;
}



/*
** AABTIntersectStruct
** Scratchpad variables for the AABox-Triangle intersection functions.  One instance
** of this structure will be used for all of the local variables and its pointer will be
** handed of to various inline functions for the axis tests.
** Note that much of the code needs the un-normalized triangle normal.  For this reason,
** I have to compute N rather than copying it from the triangle.  (commenting this to
** avoid re-generating a difficult to find bug that I had)
*/
struct AABTIntersectStruct
{
	AABTIntersectStruct(void) :
		Box(nullptr),
		Tri(nullptr)
	{
	}

	void Init(const AABoxClass &box,const TriClass &tri)
	{
		Box = &box;
		Tri = &tri;
		Vector3::Subtract(*tri.V[0],box.Center,&D);				// vector from center of box to vertex 0
		Vector3::Subtract(*tri.V[1],*tri.V[0],&E[0]);
		Vector3::Subtract(*tri.V[2],*tri.V[0],&E[1]);
		Vector3::Subtract(E[1],E[0],&E[2]);

		Vector3::Cross_Product(E[0],E[1],&N);
	}


	Vector3					D;						// Vector from the center of the box to v0
	float						AE[3][3];			// Dot products of the Basis vectors and edges
	float						AN[3];				// Dot products of the Basis vectors and the normal
	Vector3					AxE[3][3];			// Cross produts of the Basis vectors and edges

	Vector3					E[3];					// edge vectors for the triangle
	Vector3					N;						// normal (NOT normalized!!!)

	const AABoxClass *	Box;
	const TriClass *		Tri;

private:

	// not implemented
	AABTIntersectStruct(const AABTIntersectStruct &);
	AABTIntersectStruct & operator = (const AABTIntersectStruct &);
};

static AABTIntersectStruct IntersectContext;


/***********************************************************************************************
 * aabtri_intersect_cross_axis -- intersection check for a "cross-product" axis                *
 *                                                                                             *
 * axis being checked is a cross product between a triangle edge and a box basis vector        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/4/99     GTH : Created.                                                                 *
 *=============================================================================================*/
static inline bool aabtri_intersect_cross_axis
(
	Vector3 &					axis,
	float							dp,
	float							leb0
)
{
	float		p0;						// distance from box center to vertex 0
	float		lp;						// leading edge of the polygon.

	p0 = Vector3::Dot_Product(IntersectContext.D,axis);

	// I want the axis centered at the box, pointing towards the triangle
	if (p0 < 0) {
		p0 = -p0;
		axis = -axis;
		dp = -dp;
	}

	// compute coordinate of "leading edge of the triangle" relative to the box center.
	lp = 0;
	if (dp < 0) { lp = dp; }
	lp = p0 + lp;

	return (lp - leb0 > -WWMATH_EPSILON);
}


/***********************************************************************************************
 * aabtri_intersect_basis_axis -- intersection check for a basis axis                          *
 *                                                                                             *
 * axis being checked is one of the basis vectors for the box                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/4/99     GTH : Created.                                                                 *
 *=============================================================================================*/
static inline bool aabtri_intersect_basis_axis
(
	Vector3 &					axis,
	float							leb0,
	float							dp1,
	float							dp2
)
{
	float		dist;						// separation along the axis
	float		lp;						// leading edge of the polygon.

	dist = Vector3::Dot_Product(IntersectContext.D,axis);

	// we want the axis centered at the box, pointing towards the triangle
	if (dist < 0) {
		dist = -dist;
		axis = -axis;
		dp1 = -dp1;
		dp2 = -dp2;
	}

	// compute coordinate of "leading edge of the polygon" relative to the box center.
	lp = 0;
	if (dp1 < lp) { lp = dp1; }
	if (dp2 < lp) { lp = dp2; }
	lp = dist + lp;

	return (lp - leb0 > -WWMATH_EPSILON);
}


/***********************************************************************************************
 * aabtri_intersect_normal_axis -- intersection check for the triangle normal                  *
 *                                                                                             *
 * axis being checked is the triangle's normal                                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/4/99     GTH : Created.                                                                 *
 *=============================================================================================*/
static inline bool aabtri_intersect_normal_axis
(
	Vector3 &					axis
)
{
	float		dist;						// separation along the axis
	float		leb0;						// initial coordinate of the leading edge of the box
	float		lp;						// leading edge of the polygon.

	dist = Vector3::Dot_Product(IntersectContext.D,axis);

	// we want the axis centered at the box, pointing towards the triangle
	if (dist < 0) {
		dist = -dist;
		axis = -axis;
	}

	leb0 =	IntersectContext.Box->Extent.X * WWMath::Fabs(IntersectContext.AN[0]) +
				IntersectContext.Box->Extent.Y * WWMath::Fabs(IntersectContext.AN[1]) +
				IntersectContext.Box->Extent.Z * WWMath::Fabs(IntersectContext.AN[2]);
	lp = dist;	// this is the "optimization", don't have to find lp

	return (lp - leb0 > -WWMATH_EPSILON);
}


/***********************************************************************************************
 * CollisionMath::Intersection_Test -- Intersection check for an AABox and a triangle          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   1/20/00    gth : copied from OBBox version and removed some Dot-products                  *
 *=============================================================================================*/
bool CollisionMath::Intersection_Test(const AABoxClass & box,const TriClass & tri)
{
	Vector3 axis;
	float dp,leb0;

	IntersectContext.Init(box,tri);

	/*
	** AXIS_N
	*/
	axis = IntersectContext.N;
	IntersectContext.AN[0] = IntersectContext.N.X;
	IntersectContext.AN[1] = IntersectContext.N.Y;
	IntersectContext.AN[2] = IntersectContext.N.Z;
	if (aabtri_intersect_normal_axis(axis)) return false;

	/*
	** AXIS_A0
	*/
	axis.Set(1,0,0);
	IntersectContext.AE[0][0] = IntersectContext.E[0].X;
	IntersectContext.AE[0][1] = IntersectContext.E[1].Y;
	if (aabtri_intersect_basis_axis(axis,box.Extent.X,IntersectContext.AE[0][0],IntersectContext.AE[0][1])) return false;

	/*
	** AXIS_A1
	*/
	axis.Set(0,1,0);
	IntersectContext.AE[1][0] = IntersectContext.E[0].Y;
	IntersectContext.AE[1][1] = IntersectContext.E[1].Y;
	if (aabtri_intersect_basis_axis(axis,box.Extent.Y,IntersectContext.AE[1][0],IntersectContext.AE[1][1])) return false;

	/*
	** AXIS_A2
	*/
	axis.Set(0,0,1);
	IntersectContext.AE[2][0] = IntersectContext.E[0].Z;
	IntersectContext.AE[2][1] = IntersectContext.E[1].Z;
	if (aabtri_intersect_basis_axis(axis,box.Extent.Z,IntersectContext.AE[2][0],IntersectContext.AE[2][1])) return false;

	/*
	** AXIS_A0xE0
	*/
	Vector3::Cross_Product(Vector3(1,0,0),IntersectContext.E[0],&IntersectContext.AxE[0][0]);
	axis = IntersectContext.AxE[0][0];
	if (axis.Length2() > AXISLEN_EPSILON2) {
		dp = IntersectContext.AN[0];
		leb0 = box.Extent[1]*WWMath::Fabs(IntersectContext.AE[2][0]) + box.Extent[2]*WWMath::Fabs(IntersectContext.AE[1][0]);
		if (aabtri_intersect_cross_axis(axis,dp,leb0)) return false;
	}

	/*
	** AXIS_A0xE1
	*/
	Vector3::Cross_Product(Vector3(1,0,0),IntersectContext.E[1],&IntersectContext.AxE[0][1]);
	axis = IntersectContext.AxE[0][1];
	if (axis.Length2() > AXISLEN_EPSILON2) {
		dp = -IntersectContext.AN[0];
		leb0 = box.Extent[1]*WWMath::Fabs(IntersectContext.AE[2][1]) + box.Extent[2]*WWMath::Fabs(IntersectContext.AE[1][1]);
		if (aabtri_intersect_cross_axis(axis,dp,leb0)) return false;
	}

	/*
	** AXIS_A0xE2
	*/
	Vector3::Cross_Product(Vector3(1,0,0),IntersectContext.E[2],&IntersectContext.AxE[0][2]);
	axis = IntersectContext.AxE[0][2];
	IntersectContext.AE[1][2] = IntersectContext.E[2].Y;
	IntersectContext.AE[2][2] = IntersectContext.E[2].Z;
	if (axis.Length2() > AXISLEN_EPSILON2) {
		dp = -IntersectContext.AN[0];
		leb0 = box.Extent[1]*WWMath::Fabs(IntersectContext.AE[2][2]) + box.Extent[2]*WWMath::Fabs(IntersectContext.AE[1][2]);
		if (aabtri_intersect_cross_axis(axis,dp,leb0)) return false;
	}

	/*
	** AXIS_A1xE0
	*/
	Vector3::Cross_Product(Vector3(0,1,0),IntersectContext.E[0],&IntersectContext.AxE[1][0]);
	axis = IntersectContext.AxE[1][0];
	if (axis.Length2() > AXISLEN_EPSILON2) {
		dp = IntersectContext.AN[1];
		leb0 = box.Extent[0]*WWMath::Fabs(IntersectContext.AE[2][0]) + box.Extent[2]*WWMath::Fabs(IntersectContext.AE[0][0]);
		if (aabtri_intersect_cross_axis(axis,dp,leb0)) return false;
	}

	/*
	** AXIS_A1xE1
	*/
	Vector3::Cross_Product(Vector3(0,1,0),IntersectContext.E[1],&IntersectContext.AxE[1][1]);
	axis = IntersectContext.AxE[1][1];
	if (axis.Length2() > AXISLEN_EPSILON2) {
		dp = -IntersectContext.AN[1];
		leb0 = box.Extent[0]*WWMath::Fabs(IntersectContext.AE[2][1]) + box.Extent[2]*WWMath::Fabs(IntersectContext.AE[0][1]);
		if (aabtri_intersect_cross_axis(axis,dp,leb0)) return false;
	}

	/*
	** AXIS_A1xE2
	*/
	Vector3::Cross_Product(Vector3(0,1,0),IntersectContext.E[2],&IntersectContext.AxE[1][2]);
	axis = IntersectContext.AxE[1][2];
	IntersectContext.AE[0][2] = IntersectContext.E[2].X;
	if (axis.Length2() > AXISLEN_EPSILON2) {
		dp = -IntersectContext.AN[1];
		leb0 = box.Extent[0]*WWMath::Fabs(IntersectContext.AE[2][2]) + box.Extent[2]*WWMath::Fabs(IntersectContext.AE[0][2]);
		if (aabtri_intersect_cross_axis(axis,dp,leb0)) return false;
	}

	/*
	** AXIS_A2xE0
	*/
	Vector3::Cross_Product(Vector3(0,0,1),IntersectContext.E[0],&IntersectContext.AxE[2][0]);
	axis = IntersectContext.AxE[2][0];
	if (axis.Length2() > AXISLEN_EPSILON2) {
		dp = IntersectContext.AN[2];
		leb0 = box.Extent[0]*WWMath::Fabs(IntersectContext.AE[1][0]) + box.Extent[1]*WWMath::Fabs(IntersectContext.AE[0][0]);
		if (aabtri_intersect_cross_axis(axis,dp,leb0)) return false;
	}

	/*
	** AXIS_A2xE1
	*/
	Vector3::Cross_Product(Vector3(0,0,1),IntersectContext.E[1],&IntersectContext.AxE[2][1]);
	axis = IntersectContext.AxE[2][1];
	if (axis.Length2() > AXISLEN_EPSILON2) {
		dp = -IntersectContext.AN[2];
		leb0 = box.Extent[0]*WWMath::Fabs(IntersectContext.AE[1][1]) + box.Extent[1]*WWMath::Fabs(IntersectContext.AE[0][1]);
		if (aabtri_intersect_cross_axis(axis,dp,leb0)) return false;
	}

	/*
	** AXIS_A2xE2
	*/
	Vector3::Cross_Product(Vector3(0,0,1),IntersectContext.E[2],&IntersectContext.AxE[2][2]);
	axis = IntersectContext.AxE[2][2];
	if (axis.Length2() > AXISLEN_EPSILON2) {
		dp = -IntersectContext.AN[2];
		leb0 = box.Extent[0]*WWMath::Fabs(IntersectContext.AE[1][2]) + box.Extent[1]*WWMath::Fabs(IntersectContext.AE[0][2]);
		if (aabtri_intersect_cross_axis(axis,dp,leb0)) return false;
	}

	return true;
}
