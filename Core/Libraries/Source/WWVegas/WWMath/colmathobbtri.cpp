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
 *                     $Archive:: /Commando/Code/wwmath/colmathobbtri.cpp                     $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 11/28/01 5:56p                                              $*
 *                                                                                             *
 *                    $Revision:: 16                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   obbtri_collision_separation_test -- test the projected extents for separation             *
 *   obbtri_check_collision_axis -- project the obb and tri onto an arbitrary axis             *
 *   obbtri_check_collision_cross_axis -- intersection check for a "cross-product" axis        *
 *   obbtri_check_collision_basis_axis -- intersection check for a basis axis                  *
 *   obbtri_check_collision_normal_axis -- intersection check for the triangle normal          *
 *   eval_side -- returns -1,0,+1 depending on the signe of val and side                       *
 *   obbtri_compute_contact_normal -- computes the normal of the collision                     *
 *   eval_A0_point -- contact point parameters for A0xEx                                       *
 *   eval_A1_point -- contact point parameters for A1xEx                                       *
 *   eval_A2_point -- contact point parameters for A2xEx                                       *
 *   obbtri_compute_contact_point -- compute the contact point                                 *
 *   CollisionMath::Collide -- collide an obbox into a triangle                                *
 *   obbtri_intersection_separation_test -- test the projected extents for intersection        *
 *   obbtri_check_intersection_cross_axis -- intersection check for a "cross-product" axis     *
 *   obbtri_check_intersection_basis_axis -- intersection check for a basis axis               *
 *   obbtri_check_intersection_normal_axis -- intersection check for the triangle normal       *
 *   CollisionMath::Intersection_Test -- Intersection check for an OBBox and a triangle        *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "colmath.h"
#include "obbox.h"
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
	
	OBBox->Triangle collision

   As with most of the collision detection functions, this code is based on the theorem
	that given any two non-intersecting convex polyhedra, a separating plane/axis 
	can be found that will be defined by one of the face normals of one of the polyhedra
	or the cross product of an edge from each polyhedra.  The key optimization for
	boxes is that many of these axes are the same.  The normal to a face is the same
	as the axis for four of the edges of the box, etc.

	This test must be done using the tri normal, the three basis vectors
	for the box, and three vectors defined by the cross products of the basis
	and edge vectors.  When testing the basis vectors, there will be two extents for
	the tri (the other two vectors projected onto the axis being tested). 
	The appropriate extent must be used in the test (largest value which is pointing
	towards the direction of the box). In addition, I've found that I must test axes 
	defined by the cross products of	the move vector and the box axes.  These tests 
	catch "false collisions" where the box passes very close to the triangle but does
	not actually hit it.

	Each axis test will use the following logic:
	Project D onto the axis being used, it is the separation distance.  If the
	projection of the extent of the box + the projection of the extent of the
	tri is greater than D*axis then the two intersect

******************************************************************************************/

/**
** BoxTriColStruct
** Scratchpad variables for the OBBox-Triangle collision detection functions.  One instance
** of this structure will be used for all of the local variables and its pointer will be 
** handed of to various inline functions for the axis tests.
** Note that much of the code needs the un-normalized triangle normal.  For this reason,
** I have to compute N rather than copying it from the triangle.  (commenting this to
** avoid re-generating a difficult to find bug that I had)
**
** Note that the MaxFrac variable starts negative so that we can accept slightly negative
** fractions (interpenetrations).  But we clamp the returned result to 0.0 so that we never
** allow an object to get more embedded, just to possibly break itself free if it is within
** the epsilon.  This is important because sometimes objects seem to intersect simply due to 
** floating point roundoff error...
*/
struct BTCollisionStruct
{
	BTCollisionStruct(const OBBoxClass &box,const Vector3 &move,const TriClass &tri,const Vector3 &trimove) :
		Box(box),															
		Tri(tri),
		BoxMove(move),
		TriMove(trimove)
	{
		Reset();
	}

	void Reset(void) 
	{
		StartBad = true;				// true until an axis clears it
		MaxFrac = -1.0f;				// maximum move allowed so far (accept slightly negative but clamp to zero at end)
		AxisId = INTERSECTION;		// axis that allowed the longest move
		Point = 0;						// index of triangle point that was closest to the box
		Side = 0;						// side of the interval

		Vector3::Subtract(*Tri.V[0],Box.Center,&D);				// vector from center of box to vertex 0
		Vector3::Subtract(BoxMove,TriMove,&Move);						// move vector relative to stationary triangle

		Vector3::Subtract(*Tri.V[1],*Tri.V[0],&E[0]);			
		Vector3::Subtract(*Tri.V[2],*Tri.V[0],&E[1]);			
		Vector3::Subtract(E[1],E[0],&E[2]);

		A[0].Set(Box.Basis[0][0],Box.Basis[1][0],Box.Basis[2][0]);
		A[1].Set(Box.Basis[0][1],Box.Basis[1][1],Box.Basis[2][1]);
		A[2].Set(Box.Basis[0][2],Box.Basis[1][2],Box.Basis[2][2]);
	
		Vector3::Cross_Product(E[0],E[1],&N);	
	}

	bool						StartBad;			// Inital configuration is intersecting?
	float						MaxFrac;				// Longest move allowed so far
	int						AxisId;				// Last separating axis
	int						Point;				// Index of the "closest" triangle point (or one of them)
	int						Side;					// which side of the interval
	
	int						TestSide;			// Was the axis we're working on flipped
	int						TestAxisId;			// Axis 'id' we're working on
	int						TestPoint;			// Index of the closest vertex
	Vector3					TestAxis;			// Axis we're working on

	Vector3					D;						// Vector from the center of the box to v0
	Vector3					Move;					// Move vector relative to stationary triangle
	float						AE[3][3];			// Dot products of the Basis vectors and edges
	float						AN[3];				// Dot products of the Basis vectors and the normal
	Vector3					AxE[3][3];			// Cross produts of the Basis vectors and edges

	Vector3					A[3];					// basis vectors for the box
	Vector3					E[3];					// edge vectors for the triangle 
	Vector3					N;						// normal (NOT normalized!!!)
	Vector3					FinalD;				// Vector from center of box to v0 at end of move

	const OBBoxClass &	Box;
	const TriClass &		Tri;
	const Vector3 &		BoxMove;
	const Vector3 &		TriMove;

private:
	
	// not implemented
	BTCollisionStruct(const BTCollisionStruct &);
	BTCollisionStruct & operator = (const BTCollisionStruct &);
};





/***********************************************************************************************
 * obbtri_collision_separation_test -- test the projected extents for separation               *
 *                                                                                             *
 * Once the extents are projected onto the axis, this function contains                        *
 * the logic that determines the allowed fraction.                                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 * true = objects are SEPARATED                                                                *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/8/99     GTH : Created.                                                                 *
 *=============================================================================================*/
static inline bool obbtri_collision_separation_test
(
	BTCollisionStruct & context,
	float lp,float leb0,float leb1
)	
{
	/*
	** - compute 'EPSILON' normalized to the length of the axis
	** - If (I'm no more than 'EPSILON' embedded in the polygon)
	**   - not startbad
   **   - If (I'm moving towards)
	**     - fraction = How far I can move before embedding (can be negative if started embedded)
	**     - If (I can take entire move)
	**       - accept entire move
	**     - Else If (I can move more than I could have before; *negative always fails!)
	**       - update fraction
	**   - Else 
	**     - Accept entire move since I'm not moving towards the polygon
	*/
	float eps = 0.0f;
	if (lp - leb0 <= 0.0f) {
		eps = COLLISION_EPSILON * context.TestAxis.Length();	// trying to only compute epsilon if I have to
	}

	if (lp - leb0 > -eps) {
		context.StartBad = false;
		if (leb1 - leb0 > 0.0f) {
			float frac = (lp-leb0)/(leb1-leb0);	
			if (frac >= 1.0f) {
				/* moving toward but not hitting triangle */ 
				context.AxisId = context.TestAxisId;
				context.MaxFrac = 1.0f; 
				return true;  
			} else {
				/* moving toward, hitting triangle */
				if (frac > context.MaxFrac) { 
					context.MaxFrac = frac; 
					context.AxisId = context.TestAxisId; 
					context.Side = context.TestSide;
					context.Point = context.TestPoint;
				}
			}
		} else {
			/* moving away or not moving */ 
			context.AxisId = context.TestAxisId;
			context.MaxFrac = 1.0f; 
			return true;  
		}
	}
	return false;	
}


/***********************************************************************************************
 * obbtri_check_collision_axis -- project the obb and tri onto an arbitrary axis               *
 *                                                                                             *
 * projects the box and the triangle onto the given axis and calls                             *
 * obbtri_separation_test.  return true if separation was detected                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 * true = objects are SEPARATED                                                                *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/8/99     GTH : Created.                                                                 *
 *=============================================================================================*/
static inline bool obbtri_check_collision_axis(BTCollisionStruct & context)
{
	float		dist;						// separation along the axis
	float		axismove;				// size of the move along the axis.
	float		leb0;						// initial coordinate of the leading edge of the box
	float		leb1;						// final coordinate of the leading edge of the box
	float		lp;						// leading edge of the polygon.
	float		tmp;						// temporary

	dist = Vector3::Dot_Product(context.D,context.TestAxis);
	axismove = Vector3::Dot_Product(context.Move,context.TestAxis);

	// I want the axis centered at the box, pointing towards the triangle
	if (dist < 0) {		
		dist = -dist;
		axismove = -axismove;
		context.TestAxis = -context.TestAxis;
		context.TestSide = -1.0f;
	} else {
		context.TestSide = 1.0f;
	}

	// compute coordinates of the leading edge of the box at t0 and t1
	leb0 =	context.Box.Extent.X * WWMath::Fabs(Vector3::Dot_Product(context.TestAxis,context.A[0])) +
				context.Box.Extent.Y * WWMath::Fabs(Vector3::Dot_Product(context.TestAxis,context.A[1])) + 
				context.Box.Extent.Z * WWMath::Fabs(Vector3::Dot_Product(context.TestAxis,context.A[2]));
	leb1 = leb0 + axismove;
	
	// compute coordinate of "leading edge of the triangle" relative to the box center.
	lp = 0;
	tmp = Vector3::Dot_Product(context.E[0],context.TestAxis); if (tmp < lp) lp = tmp;
	tmp = Vector3::Dot_Product(context.E[1],context.TestAxis); if (tmp < lp) lp = tmp;
	lp = dist + lp;	

	return obbtri_collision_separation_test(context,lp,leb0,leb1);
}


/***********************************************************************************************
 * obbtri_check_collision_cross_axis -- projects obb and tri onto a "cross" axis               *
 *                                                                                             *
 * Assumes that the axis given is one generated from a cross product of one of the edge and    *
 * basis vectors.  In this case, the box extent can be optimized and only two triangle verts   *
 * need to be checked.                                                                         *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 * true = objects are SEPARATED                                                                *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/8/99     GTH : Created.                                                                 *
 *=============================================================================================*/
static inline bool obbtri_check_collision_cross_axis
(
	BTCollisionStruct &	context,
	float						dp,
	int						dpi,
	float						leb0
)
{
	float		p0;						// distance from box center to vertex 0
	float		axismove;				// size of the move along the axis.
	float		leb1;						// final coordinate of the leading edge of the box
	float		lp;						// leading edge of the polygon.

	p0 = Vector3::Dot_Product(context.D,context.TestAxis);
	axismove = Vector3::Dot_Product(context.Move,context.TestAxis);

	// I want the axis centered at the box, pointing towards the triangle
	if (p0 < 0) {		
		p0 = -p0;
		axismove = -axismove;
		dp = -dp;
		context.TestAxis = -context.TestAxis;
		context.TestSide = -1.0f;
	} else {
		context.TestSide = 1.0f;
	}

	// compute coordinates of the leading edge of the box at t1
	leb1 = leb0 + axismove;
	
	// compute coordinate of "leading edge of the triangle" relative to the box center.
	lp = 0; context.TestPoint = 0;
	if (dp < 0) { lp = dp; context.TestPoint = dpi; }
	lp = p0 + lp;	

	return obbtri_collision_separation_test(context,lp,leb0,leb1);
}


/***********************************************************************************************
 * obbtri_check_collision_basis_axis -- projects the obb and tri onto a basis axis             *
 *                                                                                             *
 * Projects the box and triangle onto an axis that is assumed to be a basis                    *
 * vector from the box.  In this case, we can skip a dot-product and use the                   *
 * corresponding extent of the box (which is passed in as leb0).                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 * true = objects are SEPARATED                                                                *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/8/99     GTH : Created.                                                                 *
 *=============================================================================================*/
static inline bool obbtri_check_collision_basis_axis
(
	BTCollisionStruct & context,
	float leb0,
	float dp1,
	float dp2
)
{
	float		dist;						// separation along the axis
	float		axismove;				// size of the move along the axis.
	float		leb1;						// final coordinate of the leading edge of the box
	float		lp;						// leading edge of the polygon.

	dist = Vector3::Dot_Product(context.D,context.TestAxis);
	axismove = Vector3::Dot_Product(context.Move,context.TestAxis);

	// we want the axis centered at the box, pointing towards the triangle
	if (dist < 0) {		
		dist = -dist;
		axismove = -axismove;
		dp1 = -dp1;
		dp2 = -dp2;
		context.TestAxis = -context.TestAxis;
		context.TestSide = -1.0f;
	} else {
		context.TestSide = 1.0f;
	}

	// this is the "optimization", leb0 = one of the extents
	leb1 = leb0 + axismove;	
	
	// compute coordinate of "leading edge of the polygon" relative to the box center.
	lp = 0; context.TestPoint = 0;
	if (dp1 < lp) { lp = dp1; context.TestPoint = 1; }
	if (dp2 < lp) { lp = dp2; context.TestPoint = 2; }
	lp = dist + lp;	

	return obbtri_collision_separation_test(context,lp,leb0,leb1);
}


/***********************************************************************************************
 * obbtri_check_collision_normal_axis -- project the box and tri onto the tri-normal           *
 *                                                                                             *
 * Projects the box and triangle onto an axis that is assumed to be the normal                 *
 * vector from the triangle.  In this case, the triangle extents are zero.                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 * true = objects are SEPARATED                                                                *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/8/99     GTH : Created.                                                                 *
 *=============================================================================================*/
static inline bool obbtri_check_collision_normal_axis(BTCollisionStruct & context)
{
	float		dist;						// separation along the axis
	float		axismove;				// size of the move along the axis.
	float		leb0;						// initial coordinate of the leading edge of the box
	float		leb1;						// final coordinate of the leading edge of the box
	float		lp;						// leading edge of the polygon.

	dist = Vector3::Dot_Product(context.D,context.TestAxis);
	axismove = Vector3::Dot_Product(context.Move,context.TestAxis);

	// we want the axis centered at the box, pointing towards the triangle
	if (dist < 0) {
		dist = -dist;
		axismove = -axismove;
		context.TestAxis = -context.TestAxis;
		context.TestSide = -1.0f;
	} else {
		context.TestSide = 1.0f;
	}

	leb0 =	context.Box.Extent.X * WWMath::Fabs(context.AN[0]) +
				context.Box.Extent.Y * WWMath::Fabs(context.AN[1]) + 
				context.Box.Extent.Z * WWMath::Fabs(context.AN[2]);
	leb1 = leb0 + axismove;
	context.TestPoint = 0;
	lp = dist;	// this is the "optimization", don't have to find lp

	return obbtri_collision_separation_test(context,lp,leb0,leb1);
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
 * obbtri_compute_contact_normal -- computes the normal of the collision                       *
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
static inline void obbtri_compute_contact_normal
(
	const BTCollisionStruct &	context,
	Vector3 *						set_normal
)
{
	switch(context.AxisId) 
	{
		case INTERSECTION:
//			WWASSERT(0);
			break;
		case AXIS_N:
			*set_normal = -context.Side * *context.Tri.N;
			break;
		case AXIS_A0:
			*set_normal = -context.Side * context.A[0];
			break;
		case AXIS_A1:
			*set_normal = -context.Side * context.A[1];
			break;
		case AXIS_A2:
			*set_normal = -context.Side * context.A[2];
			break;
		case AXIS_A0E0:
			*set_normal = -context.Side * context.AxE[0][0];
			set_normal->Normalize();
			break;
		case AXIS_A1E0:
			*set_normal = -context.Side * context.AxE[1][0];
			set_normal->Normalize();
			break;
		case AXIS_A2E0:
			*set_normal = -context.Side * context.AxE[2][0];
			set_normal->Normalize();
			break;
		case AXIS_A0E1:
			*set_normal = -context.Side * context.AxE[0][1];
			set_normal->Normalize();
			break;
		case AXIS_A1E1:
			*set_normal = -context.Side * context.AxE[1][1];
			set_normal->Normalize();
			break;
		case AXIS_A2E1:
			*set_normal = -context.Side * context.AxE[2][1];
			set_normal->Normalize();
			break;
		case AXIS_A0E2:
			*set_normal = -context.Side * context.AxE[0][2];
			set_normal->Normalize();
			break;
		case AXIS_A1E2:
			*set_normal = -context.Side * context.AxE[1][2];
			set_normal->Normalize();
			break;
		case AXIS_A2E2:
			*set_normal = -context.Side * context.AxE[2][2];
			set_normal->Normalize();
			break;
	}
	WWASSERT(set_normal->Length2() > 0.0f);
}


/***********************************************************************************************
 * eval_A0_point -- contact point parameters for A0xEx                                         *
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
static inline void eval_A0_point(const BTCollisionStruct & context,float * x,int edge)
{
	float yval,den;
	Vector3 DxE;
	
	x[1] = -eval_side(context.AE[2][edge],context.Side) * context.Box.Extent[1];
	x[2] = eval_side(context.AE[1][edge],context.Side) * context.Box.Extent[2];
	if (context.Point == 0) { yval = 0.0f; } else { yval = 1.0f; }

	den = Vector3::Dot_Product(context.N,context.AxE[0][edge]);
	if (WWMath::Fabs(den) > 0.0f) {

		Vector3::Cross_Product(context.FinalD,context.E[edge],&DxE);
		x[0] = Vector3::Dot_Product(context.N,DxE);
		if (edge == 0) {
			x[0] -= context.N.Length2() * yval;
		} else {
			x[0] += context.N.Length2() * yval;
		}
		x[0] -= Vector3::Dot_Product(context.N,context.AxE[1][edge]) * x[1];
		x[0] -= Vector3::Dot_Product(context.N,context.AxE[2][edge]) * x[2];
		x[0] /= den;

	} else {

		x[0] = 0.0f;

	}
}


/***********************************************************************************************
 * eval_A1_point -- contact point parameters for A1xEx                                         *
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
static inline void eval_A1_point(const BTCollisionStruct & context,float * x,int edge)
{
	float yval,den;
	Vector3 DxE;

	x[0] = eval_side(context.AE[2][edge],context.Side) * context.Box.Extent[0];
	x[2] = -eval_side(context.AE[0][edge],context.Side) * context.Box.Extent[2];
	if (context.Point == 0) { yval = 0.0f; } else { yval = 1.0f; }
	
	den = Vector3::Dot_Product(context.N,context.AxE[1][edge]);
	if (WWMath::Fabs(den) > 0.0f) {

		Vector3::Cross_Product(context.FinalD,context.E[edge],&DxE);
		x[1] = Vector3::Dot_Product(context.N,DxE);
		if (edge == 0) {
			x[1] -= context.N.Length2() * yval;
		} else {
			x[1] += context.N.Length2() * yval;
		}
		x[1] -= Vector3::Dot_Product(context.N,context.AxE[0][edge]) * x[0];
		x[1] -= Vector3::Dot_Product(context.N,context.AxE[2][edge]) * x[2];
		x[1] /= den;
	
	} else {

		x[1] = 0.0f;
	
	}
}

/***********************************************************************************************
 * eval_A2_point -- contact point parameters for A2xEx                                         *
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
static inline void eval_A2_point(const BTCollisionStruct & context,float * x,int edge)
{
	float yval,den;
	Vector3 DxE;

	x[0] = -eval_side(context.AE[1][edge],context.Side) * context.Box.Extent[0];
	x[1] = eval_side(context.AE[0][edge],context.Side) * context.Box.Extent[1];
	if (context.Point == 0) { yval = 0.0f; } else { yval = 1.0f; }
	
	den = Vector3::Dot_Product(context.N,context.AxE[2][edge]);
	if (WWMath::Fabs(den) > 0.0f) {

		Vector3::Cross_Product(context.FinalD,context.E[edge],&DxE);
		x[2] = Vector3::Dot_Product(context.N,DxE);
		if (edge == 0) {
			x[2] -= context.N.Length2() * yval;
		} else {
			x[2] += context.N.Length2() * yval;
		}
		x[2] -= Vector3::Dot_Product(context.N,context.AxE[0][edge]) * x[0];
		x[2] -= Vector3::Dot_Product(context.N,context.AxE[1][edge]) * x[1];
		x[2] /= den;
	
	} else {

		x[2] = 0.0f;
	
	}
}


/***********************************************************************************************
 * obbtri_compute_contact_point -- compute the contact point                                   *
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
static inline void obbtri_compute_contact_point
(
	BTCollisionStruct &	context,
	CastResultStruct *	result
)
{
	int i;
	float x[3];

	if (context.AxisId >= AXIS_A0E0) {
		Vector3 newc = context.Box.Center + result->Fraction * context.BoxMove;
		Vector3 newv0 = *context.Tri.V[0] + result->Fraction * context.TriMove;
		context.FinalD = newv0 - newc;
	}

	switch (context.AxisId) 
	{
	
	case INTERSECTION:
		WWASSERT(0);
		return;

	case AXIS_N:		// part of the box is touching the face of the triangle

		for (i = 0; i < 3; i++) {
			x[i] = eval_side(context.AN[i],context.Side) * context.Box.Extent[i];
      }
		break;
	
	case AXIS_A0:		// part of the triangle is touching a face of the box
	case AXIS_A1:
	case AXIS_A2:
		
		if (context.Point == 0) {
			result->ContactPoint = *context.Tri.V[0];
		} else if (context.Point == 1) {
			result->ContactPoint = *context.Tri.V[1];
		} else {
			result->ContactPoint = *context.Tri.V[2];
		}
		Vector3::Add(result->ContactPoint,result->Fraction * context.TriMove,&(result->ContactPoint));
		return;

	case AXIS_A0E0:	// one of the x-aligned edges of the box is contacting edge0 
		eval_A0_point(context,x,0);
		break;
	
	case AXIS_A0E1:	// one of the x-aligned edges of the box is contacting edge1 
		eval_A0_point(context,x,1);
		break;

	case AXIS_A0E2:	// one of the x-aligned edges of the box is contacting edge2
		eval_A0_point(context,x,2);
		break;

	case AXIS_A1E0:	// one of the y-aligned edges of the box is contacting edge0
		eval_A1_point(context,x,0);
		break;

	case AXIS_A1E1:	// one of the y-aligned edges of the box is contacting edge1
		eval_A1_point(context,x,1);
		break;

	case AXIS_A1E2:	// one of the y-aligned edges of the box is contacting edge2
		eval_A1_point(context,x,2);
		break;

	case AXIS_A2E0:	// one of the z-aligned edges of the box is contacting edge0
		eval_A2_point(context,x,0);
		break;

	case AXIS_A2E1:	// one of the z-aligned edges of the box is contacting edge1
		eval_A2_point(context,x,1);
		break;
	
	case AXIS_A2E2:	// one of the z-aligned edges of the box is contacting edge3
		eval_A2_point(context,x,2);
		break;

	}

	/*
	** In certain circumstances, the x's computed above are outside the size of 
	** the box.  I have tracked at least one of these cases to a situation where the
	** separating axis is the triangle normal but one of the AxE axes also lines up
	** with the normal.  In this case, the Normal was 0,0,25 and the A0xE0 axis
	** was 0,0,5.  When the fraction was calculated for both of these, they were
	** the same up to the six decimal places that MSVC will show me in the debugger.
	** However, the fraction computed for the 0,0,5 axis was larger by some very small
	** amount.  This causes it to be used as the "separating axis".  Looks like floating 
	** point roundoff error to me.  The problem is that since the axis was perpendicular 
	** to the triangle, the "nearest-point" logic chose V0 which resulted in the 
	** calculation for x[0] being way off.  The equations for finding the contact point 
	** in the AxE axis case assume that the axis is not normal to the triangle since 
	** that should be handled in the simpler routine for the N separating axis...
	** Since I am at a loss for how to deal with this problem, I'm going to clamp all 
	** of the x's to be within the box here...
	*/
	if (x[0] > context.Box.Extent.X) {
		x[0] = context.Box.Extent.X;
	} else if (x[0] < -context.Box.Extent.X) {
		x[0] = -context.Box.Extent.X;
	}

	if (x[1] > context.Box.Extent.Y) {
		x[1] = context.Box.Extent.Y;
	} else if (x[1] < -context.Box.Extent.Y) {
		x[1] = -context.Box.Extent.Y;
	}
	
	if (x[2] > context.Box.Extent.Z) {
		x[2] = context.Box.Extent.Z;
	} else if (x[2] < -context.Box.Extent.Z) {
		x[2] = -context.Box.Extent.Z;
	}

	/*
	** Now, compute the point
	*/
	result->ContactPoint.X =	context.Box.Center.X + 
										x[0]*context.A[0].X + 
										x[1]*context.A[1].X + 
										x[2]*context.A[2].X;

	result->ContactPoint.Y =	context.Box.Center.Y + 
										x[0]*context.A[0].Y + 
										x[1]*context.A[1].Y + 
										x[2]*context.A[2].Y;

	result->ContactPoint.Z =	context.Box.Center.Z + 
										x[0]*context.A[0].Z + 
										x[1]*context.A[1].Z + 
										x[2]*context.A[2].Z;
	
	Vector3::Add(result->ContactPoint,result->Fraction * context.BoxMove,&(result->ContactPoint));
}



/***********************************************************************************************
 * CollisionMath::Collide -- collide an obbox into a triangle                                  *
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
	const OBBoxClass &		box,
	const Vector3 &			move,
	const TriClass &			tri,
	const Vector3 &			trimove,
	CastResultStruct *		result
)
{
	TRACK_COLLISION_OBBOX_TRI;
	float dp,leb0;
	BTCollisionStruct	context(box,move,tri,trimove);

	/*
	** AXIS_N
	*/
	context.TestAxis = context.N;
	context.TestAxisId = AXIS_N;
	context.AN[0] = Vector3::Dot_Product(context.A[0],context.N);
	context.AN[1] = Vector3::Dot_Product(context.A[1],context.N);
	context.AN[2] = Vector3::Dot_Product(context.A[2],context.N);
	if (obbtri_check_collision_normal_axis(context)) goto exit;

	/*
	** AXIS_A0
	*/
	context.TestAxis = context.A[0];
	context.TestAxisId = AXIS_A0;
	context.AE[0][0] = Vector3::Dot_Product(context.A[0],context.E[0]);
	context.AE[0][1] = Vector3::Dot_Product(context.A[0],context.E[1]);
	if (obbtri_check_collision_basis_axis(context,box.Extent.X,context.AE[0][0],context.AE[0][1])) goto exit;

	/*
	** AXIS_A1
	*/
	context.TestAxis = context.A[1];
	context.TestAxisId = AXIS_A1;
	context.AE[1][0] = Vector3::Dot_Product(context.A[1],context.E[0]);
	context.AE[1][1] = Vector3::Dot_Product(context.A[1],context.E[1]);
	if (obbtri_check_collision_basis_axis(context,box.Extent.Y,context.AE[1][0],context.AE[1][1])) goto exit;

	/*
	** AXIS_A2
	*/
	context.TestAxis = context.A[2];
	context.TestAxisId = AXIS_A2;
	context.AE[2][0] = Vector3::Dot_Product(context.A[2],context.E[0]);
	context.AE[2][1] = Vector3::Dot_Product(context.A[2],context.E[1]);
	if (obbtri_check_collision_basis_axis(context,box.Extent.Z,context.AE[2][0],context.AE[2][1])) goto exit;

	/*
	** AXIS_A0xE0
	*/
	Vector3::Cross_Product(context.A[0],context.E[0],&context.AxE[0][0]);
	context.TestAxis = context.AxE[0][0];
	context.TestAxisId = AXIS_A0E0;
	if (context.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = context.AN[0];
		leb0 = box.Extent[1]*WWMath::Fabs(context.AE[2][0]) + box.Extent[2]*WWMath::Fabs(context.AE[1][0]);
		if (obbtri_check_collision_cross_axis(context,dp,2,leb0)) goto exit;
	}

	/*
	** AXIS_A0xE1
	*/
	Vector3::Cross_Product(context.A[0],context.E[1],&context.AxE[0][1]);
	context.TestAxis = context.AxE[0][1];
	context.TestAxisId = AXIS_A0E1;
	if (context.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = -context.AN[0];
		leb0 = box.Extent[1]*WWMath::Fabs(context.AE[2][1]) + box.Extent[2]*WWMath::Fabs(context.AE[1][1]);
		if (obbtri_check_collision_cross_axis(context,dp,1,leb0)) goto exit;
	}

	/*
	** AXIS_A0xE2
	*/
	Vector3::Cross_Product(context.A[0],context.E[2],&context.AxE[0][2]);
	context.TestAxis = context.AxE[0][2];
	context.TestAxisId = AXIS_A0E2;
	context.AE[1][2] = Vector3::Dot_Product(context.A[1],context.E[2]);
	context.AE[2][2] = Vector3::Dot_Product(context.A[2],context.E[2]);
	if (context.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = -context.AN[0];
		leb0 = box.Extent[1]*WWMath::Fabs(context.AE[2][2]) + box.Extent[2]*WWMath::Fabs(context.AE[1][2]);
		if (obbtri_check_collision_cross_axis(context,dp,1,leb0)) goto exit;
	}

	/*
	** AXIS_A1xE0
	*/
	Vector3::Cross_Product(context.A[1],context.E[0],&context.AxE[1][0]);
	context.TestAxis = context.AxE[1][0];
	context.TestAxisId = AXIS_A1E0;
	if (context.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = context.AN[1];
		leb0 = box.Extent[0]*WWMath::Fabs(context.AE[2][0]) + box.Extent[2]*WWMath::Fabs(context.AE[0][0]);
		if (obbtri_check_collision_cross_axis(context,dp,2,leb0)) goto exit;
	}

	/*
	** AXIS_A1xE1
	*/
	Vector3::Cross_Product(context.A[1],context.E[1],&context.AxE[1][1]);
	context.TestAxis = context.AxE[1][1];
	context.TestAxisId = AXIS_A1E1;
	if (context.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = -context.AN[1];
		leb0 = box.Extent[0]*WWMath::Fabs(context.AE[2][1]) + box.Extent[2]*WWMath::Fabs(context.AE[0][1]);
		if (obbtri_check_collision_cross_axis(context,dp,1,leb0)) goto exit;
	}

	/*
	** AXIS_A1xE2
	*/
	Vector3::Cross_Product(context.A[1],context.E[2],&context.AxE[1][2]);
	context.TestAxis = context.AxE[1][2];
	context.TestAxisId = AXIS_A1E2;
	context.AE[0][2] = Vector3::Dot_Product(context.A[0],context.E[2]);
	if (context.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = -context.AN[1];
		leb0 = box.Extent[0]*WWMath::Fabs(context.AE[2][2]) + box.Extent[2]*WWMath::Fabs(context.AE[0][2]);
		if (obbtri_check_collision_cross_axis(context,dp,1,leb0)) goto exit;
	}

	/*
	** AXIS_A2xE0
	*/
	Vector3::Cross_Product(context.A[2],context.E[0],&context.AxE[2][0]);
	context.TestAxis = context.AxE[2][0];
	context.TestAxisId = AXIS_A2E0;
	if (context.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = context.AN[2];
		leb0 = box.Extent[0]*WWMath::Fabs(context.AE[1][0]) + box.Extent[1]*WWMath::Fabs(context.AE[0][0]);
		if (obbtri_check_collision_cross_axis(context,dp,2,leb0)) goto exit;
	}

	/*
	** AXIS_A2xE1
	*/
	Vector3::Cross_Product(context.A[2],context.E[1],&context.AxE[2][1]);
	context.TestAxis = context.AxE[2][1];
	context.TestAxisId = AXIS_A2E1;
	if (context.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = -context.AN[2];
		leb0 = box.Extent[0]*WWMath::Fabs(context.AE[1][1]) + box.Extent[1]*WWMath::Fabs(context.AE[0][1]);
		if (obbtri_check_collision_cross_axis(context,dp,1,leb0)) goto exit;
	}
	
	/*
	** AXIS_A2xE2
	*/
	Vector3::Cross_Product(context.A[2],context.E[2],&context.AxE[2][2]);
	context.TestAxis = context.AxE[2][2];
	context.TestAxisId = AXIS_A2E2;
	if (context.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = -context.AN[2];
		leb0 = box.Extent[0]*WWMath::Fabs(context.AE[1][2]) + box.Extent[1]*WWMath::Fabs(context.AE[0][2]);
		if (obbtri_check_collision_cross_axis(context,dp,1,leb0)) goto exit;
	}

	/*
	** Last ditch effort, check an axis based on the move vector
	*/
	if (!context.StartBad) {
		context.TestPoint = context.Point;
		context.TestAxisId = context.AxisId;

		Vector3::Cross_Product(context.Move,context.A[0],&context.TestAxis);
		if (context.TestAxis.Length2() > AXISLEN_EPSILON2) {
			if (obbtri_check_collision_axis(context)) goto exit;
		}
		Vector3::Cross_Product(context.Move,context.A[1],&context.TestAxis);
		if (context.TestAxis.Length2() > AXISLEN_EPSILON2) {
			if (obbtri_check_collision_axis(context)) goto exit;
		}
		Vector3::Cross_Product(context.Move,context.A[2],&context.TestAxis);
		if (context.TestAxis.Length2() > AXISLEN_EPSILON2) {
			if (obbtri_check_collision_axis(context)) goto exit;
		}
	}

exit:

#pragma message ("(gth) disabling an assert in obb->tri collision, investigate later\n")
#if 0
	WWASSERT((context.AxisId != INTERSECTION) || (context.StartBad));
#else
	if (context.AxisId == INTERSECTION) {
		context.StartBad = true;
	}
#endif

	/*
	** If the triangle and box are intersecting before the move, return that 
	** result.
	*/
	if (context.StartBad) {
		result->StartBad = true;
		result->Fraction = 0.0f;
		result->Normal = *tri.N;
		TRACK_COLLISION_OBBOX_TRI_HIT;
		return true;
	}

	/*
	** If the fraction allowed is basically equal to the fraction allowed by 
	** another polygon, try to pick the polygon which is least "edge-on" to the
	** move.
	*/
	if (context.MaxFrac < 0.0f) {
		context.MaxFrac = 0.0f;
	}

	if ((context.MaxFrac < 1.0f) && (context.MaxFrac <= result->Fraction)) {

		Vector3 normal;
		obbtri_compute_contact_normal(context,&normal);

		if (	(WWMath::Fabs(context.MaxFrac - result->Fraction) > WWMATH_EPSILON) ||
				(Vector3::Dot_Product(normal,move) < Vector3::Dot_Product(result->Normal,move)) )
		{
			result->Normal = normal; //obbtri_compute_contact_normal(context,result);
		}
				
		result->Fraction = context.MaxFrac;

		if (result->ComputeContactPoint) {
			obbtri_compute_contact_point(context,result);	
		}
		TRACK_COLLISION_OBBOX_TRI_HIT;
		return true;
	}
	
	return false;
}


/***********************************************************************************************
	
	OBBox-Triangle Intersection

	The following code implements a simple boolean intersection check.  It uses the same
	algorithm as the collision function but can avoid some of the calculations.  For a very
	simple implementation of this algorithm, see Oriented_Box_Intersects_Tri in obbox.h

***********************************************************************************************/

/**
** BTIntersectStruct
** Scratchpad variables for the OBBox-Triangle intersection functions.  One instance
** of this structure will be used for all of the local variables and its pointer will be 
** handed of to various inline functions for the axis tests.
** Note that much of the code needs the un-normalized triangle normal.  For this reason,
** I have to compute N rather than copying it from the triangle.  (commenting this to
** avoid re-generating a difficult to find bug that I had)
*/
struct BTIntersectStruct
{
	BTIntersectStruct(const OBBoxClass &box,const TriClass &tri) :
		Box(box),															
		Tri(tri)
	{
		Vector3::Subtract(*tri.V[0],box.Center,&D);				// vector from center of box to vertex 0
		Vector3::Subtract(*tri.V[1],*tri.V[0],&E[0]);			
		Vector3::Subtract(*tri.V[2],*tri.V[0],&E[1]);			
		Vector3::Subtract(E[1],E[0],&E[2]);

		A[0].Set(box.Basis[0][0],box.Basis[1][0],box.Basis[2][0]);
		A[1].Set(box.Basis[0][1],box.Basis[1][1],box.Basis[2][1]);
		A[2].Set(box.Basis[0][2],box.Basis[1][2],box.Basis[2][2]);
	
		Vector3::Cross_Product(E[0],E[1],&N);	
	}

	Vector3					D;						// Vector from the center of the box to v0
	float						AE[3][3];			// Dot products of the Basis vectors and edges
	float						AN[3];				// Dot products of the Basis vectors and the normal
	Vector3					AxE[3][3];			// Cross produts of the Basis vectors and edges

	Vector3					A[3];					// basis vectors for the box
	Vector3					E[3];					// edge vectors for the triangle 
	Vector3					N;						// normal (NOT normalized!!!)

	Vector3					TestAxis;			// separating axis currently being tested

	const OBBoxClass &	Box;
	const TriClass &		Tri;

private:
	
	// not implemented
	BTIntersectStruct(const BTIntersectStruct &);
	BTIntersectStruct & operator = (const BTIntersectStruct &);
};


/***********************************************************************************************
 * obbtri_intersection_separation_test -- test the projected extents for intersection          *
 *                                                                                             *
 * Once the extents are projected onto the axis, this function contains                        *
 * the logic that determines whether the box and triangle intersect.                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 * context - the BTIntersectStruct containing the data for this intersection test              *
 * lp - the leading edge of the polygon projected onto the axis                                *
 * leb0 - the leading edge of the box projected onto the axis                                  *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 * true = objects are separated                                                                *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/3/2000   gth : Created.                                                                 *
 *=============================================================================================*/
static inline bool obbtri_intersection_separation_test
(
	BTIntersectStruct &	context,
	float						lp,
	float						leb0
)
{
	/*
	** Only compute the normalized epsilon if we need to.
	** - compute 'EPSILON' normalized to the length of the axis
	** - If (I'm no more than 'EPSILON' embedded in the polygon) then the box and tri are separated
	*/
	float eps = 0.0f;
	if (lp - leb0 <= 0.0f) {
		eps = COLLISION_EPSILON * context.TestAxis.Length();	// trying to only compute epsilon if I have to
	}

	return (lp - leb0 > -eps);
}


/***********************************************************************************************
 * obbtri_check_intersection_cross_axis -- intersection check for a "cross-product" axis       *
 *                                                                                             *
 * axis being checked is a cross product between a triangle edge and a box basis vector        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * true = the objects are SEPARATED                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/4/99     GTH : Created.                                                                 *
 *=============================================================================================*/
static inline bool obbtri_check_intersection_cross_axis
(
	BTIntersectStruct &	context,
	float						dp,
	float						leb0
)
{
	float		p0;						// distance from box center to vertex 0
	float		lp;						// leading edge of the polygon.

	p0 = Vector3::Dot_Product(context.D,context.TestAxis);

	// I want the axis centered at the box, pointing towards the triangle
	if (p0 < 0) {		
		context.TestAxis = -context.TestAxis;
		p0 = -p0;
		dp = -dp;
	}

	// compute coordinate of "leading edge of the triangle" relative to the box center.
	lp = 0;
	if (dp < 0) { lp = dp; }
	lp = p0 + lp;	

	return obbtri_intersection_separation_test(context,lp,leb0);
}


/***********************************************************************************************
 * obbtri_check_intersection_basis_axis -- intersection check for a basis axis                 *
 *                                                                                             *
 * axis being checked is one of the basis vectors for the box                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 * true = the objects are SEPARATED                                                            *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/4/99     GTH : Created.                                                                 *
 *=============================================================================================*/
static inline bool obbtri_check_intersection_basis_axis
(
	BTIntersectStruct &	context,
	float						leb0,
	float						dp1,
	float						dp2
)
{
	float		dist;						// separation along the axis
	float		lp;						// leading edge of the polygon.

	dist = Vector3::Dot_Product(context.D,context.TestAxis);

	// we want the axis centered at the box, pointing towards the triangle
	if (dist < 0) {		
		context.TestAxis = -context.TestAxis;
		dist = -dist;
		dp1 = -dp1;
		dp2 = -dp2;
	} 

	// compute coordinate of "leading edge of the polygon" relative to the box center.
	lp = 0;
	if (dp1 < lp) { lp = dp1; }
	if (dp2 < lp) { lp = dp2; }
	lp = dist + lp;	

	return obbtri_intersection_separation_test(context,lp,leb0);
}


/***********************************************************************************************
 * obbtri_check_intersection_normal_axis -- intersection check for the triangle normal         *
 *                                                                                             *
 * axis being checked is the triangle's normal                                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 * true = the objects are SEPARATED                                                            *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/4/99     GTH : Created.                                                                 *
 *=============================================================================================*/
static inline bool obbtri_check_intersection_normal_axis
(
	BTIntersectStruct &	context
)
{
	float		dist;						// separation along the axis
	float		leb0;						// initial coordinate of the leading edge of the box
	float		lp;						// leading edge of the polygon.

	dist = Vector3::Dot_Product(context.D,context.TestAxis);

	// we want the axis centered at the box, pointing towards the triangle
	if (dist < 0) {
		context.TestAxis = -context.TestAxis;
		dist = -dist;
	}

	leb0 =	context.Box.Extent.X * WWMath::Fabs(context.AN[0]) +
				context.Box.Extent.Y * WWMath::Fabs(context.AN[1]) + 
				context.Box.Extent.Z * WWMath::Fabs(context.AN[2]);
	lp = dist;	// this is the "optimization", don't have to find lp

	return obbtri_intersection_separation_test(context,lp,leb0);
}


/***********************************************************************************************
 * CollisionMath::Intersection_Test -- Intersection check for an OBBox and a triangle          *
 *                                                                                             *
 * INPUT:                                                                                      *
 * box - obbox to be tested                                                                    *
 * tri - triangle to be tested                                                                 *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 * true = objects INTERSECT                                                                    *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * note that the other inline functions are "quick-reject" functions which return true when    *
 * the objects are separated.                                                                  *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   5/4/99     GTH : Created.                                                                 *
 *=============================================================================================*/
bool CollisionMath::Intersection_Test(const OBBoxClass & box,const TriClass & tri)
{
	float dp,leb0;
	BTIntersectStruct	context(box,tri);

	/*
	** AXIS_N
	*/
	context.TestAxis = context.N;
	context.AN[0] = Vector3::Dot_Product(context.A[0],context.N);
	context.AN[1] = Vector3::Dot_Product(context.A[1],context.N);
	context.AN[2] = Vector3::Dot_Product(context.A[2],context.N);
	if (obbtri_check_intersection_normal_axis(context)) return false;

	/*
	** AXIS_A0
	*/
	context.TestAxis = context.A[0];
	context.AE[0][0] = Vector3::Dot_Product(context.A[0],context.E[0]);
	context.AE[0][1] = Vector3::Dot_Product(context.A[0],context.E[1]);
	if (obbtri_check_intersection_basis_axis(context,box.Extent.X,context.AE[0][0],context.AE[0][1])) return false;

	/*
	** AXIS_A1
	*/
	context.TestAxis = context.A[1];
	context.AE[1][0] = Vector3::Dot_Product(context.A[1],context.E[0]);
	context.AE[1][1] = Vector3::Dot_Product(context.A[1],context.E[1]);
	if (obbtri_check_intersection_basis_axis(context,box.Extent.Y,context.AE[1][0],context.AE[1][1])) return false;

	/*
	** AXIS_A2
	*/
	context.TestAxis = context.A[2];
	context.AE[2][0] = Vector3::Dot_Product(context.A[2],context.E[0]);
	context.AE[2][1] = Vector3::Dot_Product(context.A[2],context.E[1]);
	if (obbtri_check_intersection_basis_axis(context,box.Extent.Z,context.AE[2][0],context.AE[2][1])) return false;

	/*
	** AXIS_A0xE0
	*/
	Vector3::Cross_Product(context.A[0],context.E[0],&context.AxE[0][0]);
	context.TestAxis = context.AxE[0][0];
	if (context.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = context.AN[0];
		leb0 = box.Extent[1]*WWMath::Fabs(context.AE[2][0]) + box.Extent[2]*WWMath::Fabs(context.AE[1][0]);
		if (obbtri_check_intersection_cross_axis(context,dp,leb0)) return false;
	}

	/*
	** AXIS_A0xE1
	*/
	Vector3::Cross_Product(context.A[0],context.E[1],&context.AxE[0][1]);
	context.TestAxis = context.AxE[0][1];
	if (context.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = -context.AN[0];
		leb0 = box.Extent[1]*WWMath::Fabs(context.AE[2][1]) + box.Extent[2]*WWMath::Fabs(context.AE[1][1]);
		if (obbtri_check_intersection_cross_axis(context,dp,leb0)) return false;
	}

	/*
	** AXIS_A0xE2
	*/
	Vector3::Cross_Product(context.A[0],context.E[2],&context.AxE[0][2]);
	context.TestAxis = context.AxE[0][2];
	context.AE[1][2] = Vector3::Dot_Product(context.A[1],context.E[2]);
	context.AE[2][2] = Vector3::Dot_Product(context.A[2],context.E[2]);
	if (context.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = -context.AN[0];
		leb0 = box.Extent[1]*WWMath::Fabs(context.AE[2][2]) + box.Extent[2]*WWMath::Fabs(context.AE[1][2]);
		if (obbtri_check_intersection_cross_axis(context,dp,leb0)) return false;
	}

	/*
	** AXIS_A1xE0
	*/
	Vector3::Cross_Product(context.A[1],context.E[0],&context.AxE[1][0]);
	context.TestAxis = context.AxE[1][0];
	if (context.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = context.AN[1];
		leb0 = box.Extent[0]*WWMath::Fabs(context.AE[2][0]) + box.Extent[2]*WWMath::Fabs(context.AE[0][0]);
		if (obbtri_check_intersection_cross_axis(context,dp,leb0)) return false;
	}

	/*
	** AXIS_A1xE1
	*/
	Vector3::Cross_Product(context.A[1],context.E[1],&context.AxE[1][1]);
	context.TestAxis = context.AxE[1][1];
	if (context.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = -context.AN[1];
		leb0 = box.Extent[0]*WWMath::Fabs(context.AE[2][1]) + box.Extent[2]*WWMath::Fabs(context.AE[0][1]);
		if (obbtri_check_intersection_cross_axis(context,dp,leb0)) return false;
	}

	/*
	** AXIS_A1xE2
	*/
	Vector3::Cross_Product(context.A[1],context.E[2],&context.AxE[1][2]);
	context.TestAxis = context.AxE[1][2];
	context.AE[0][2] = Vector3::Dot_Product(context.A[0],context.E[2]);
	if (context.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = -context.AN[1];
		leb0 = box.Extent[0]*WWMath::Fabs(context.AE[2][2]) + box.Extent[2]*WWMath::Fabs(context.AE[0][2]);
		if (obbtri_check_intersection_cross_axis(context,dp,leb0)) return false;
	}

	/*
	** AXIS_A2xE0
	*/
	Vector3::Cross_Product(context.A[2],context.E[0],&context.AxE[2][0]);
	context.TestAxis = context.AxE[2][0];
	if (context.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = context.AN[2];
		leb0 = box.Extent[0]*WWMath::Fabs(context.AE[1][0]) + box.Extent[1]*WWMath::Fabs(context.AE[0][0]);
		if (obbtri_check_intersection_cross_axis(context,dp,leb0)) return false;
	}

	/*
	** AXIS_A2xE1
	*/
	Vector3::Cross_Product(context.A[2],context.E[1],&context.AxE[2][1]);
	context.TestAxis = context.AxE[2][1];
	if (context.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = -context.AN[2];
		leb0 = box.Extent[0]*WWMath::Fabs(context.AE[1][1]) + box.Extent[1]*WWMath::Fabs(context.AE[0][1]);
		if (obbtri_check_intersection_cross_axis(context,dp,leb0)) return false;
	}
	
	/*
	** AXIS_A2xE2
	*/
	Vector3::Cross_Product(context.A[2],context.E[2],&context.AxE[2][2]);
	context.TestAxis = context.AxE[2][2];
	if (context.TestAxis.Length2() > AXISLEN_EPSILON2) {
		dp = -context.AN[2];
		leb0 = box.Extent[0]*WWMath::Fabs(context.AE[1][2]) + box.Extent[1]*WWMath::Fabs(context.AE[0][2]);
		if (obbtri_check_intersection_cross_axis(context,dp,leb0)) return false;
	}

	return true;
}



