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
 *                     $Archive:: /Commando/Code/wwmath/colmathaabox.cpp                      $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 8/30/01 7:40p                                               $*
 *                                                                                             *
 *                    $Revision:: 22                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   CollisionMath::Intersection_Test -- Test intersection between two AABoxes                 *
 *   CollisionMath::Overlap_Test -- Tests overlap between an AABox and a sphere                *
 *   CollisionMath::Overlap_Test -- Tests overlap between an AABox and a triangle              *
 *   CollisionMath::Overlap_Test -- Tests overlap between an AABox and a line segment          *
 *   CollisionMath::Collide -- Collision test for a moving AABox and a plane                   *
 *   aab_separation_test -- tests two AAB's for separation on an axis                          *
 *   CollisionMath::Collide -- Collision test for two moving AABoxes                           *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "colmath.h"
#include "colmathinlines.h"
#include "aaplane.h"
#include "plane.h"
#include "lineseg.h"
#include "tri.h"
#include "sphere.h"
#include "aabox.h"
#include "obbox.h"
#include "wwdebug.h"


/***********************************************************************************************
 * CollisionMath::Intersection_Test -- Test intersection between two AABoxes                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/19/99   gth : Created.                                                                 *
 *=============================================================================================*/
bool CollisionMath::Intersection_Test(const AABoxClass & box,const AABoxClass & box2)
{
	Vector3 dc = box2.Center - box.Center;

	if (box.Extent.X + box2.Extent.X < WWMath::Fabs(dc.X)) return false;
	if (box.Extent.Y + box2.Extent.Y < WWMath::Fabs(dc.Y)) return false;
	if (box.Extent.Z + box2.Extent.Z < WWMath::Fabs(dc.Z)) return false;
	return true;
}

/***********************************************************************************************
 * CollisionMath::Overlap_Test -- Tests overlap between an AABox and a sphere                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/19/99   gth : Created.                                                                 *
 *=============================================================================================*/
CollisionMath::OverlapType CollisionMath::Overlap_Test(const AABoxClass & box,const SphereClass & sphere)
{
	// TODO: this function seems incorrect.  Not currently using it though
	Vector3 dist = sphere.Center - box.Center;

	Vector3 extent;
	extent.X = box.Extent.X + sphere.Radius;
	extent.Y = box.Extent.Y + sphere.Radius;
	extent.Z = box.Extent.Z + sphere.Radius;

	//
	//	Check to see if the sphere is completely outside the box
	//
	if (WWMath::Fabs(dist.X) > extent.X) return OUTSIDE;
	if (WWMath::Fabs(dist.Y) > extent.Y) return OUTSIDE;
	if (WWMath::Fabs(dist.Z) > extent.Z) return OUTSIDE;

	return INSIDE;
}



/***********************************************************************************************
 * CollisionMath::Overlap_Test -- Tests overlap between an AABox and a line segment            *
 *                                                                                             *
 * This function uses separating axes to determine whether an AABox and a line segment overlap *
 * I wrote it when we found that ray-casting was taking a huge amount of time in Renegade.     *
 * Here are some statistics comparing this function to the previous function which used the    *
 * same algorithm that Collide(box,ray) uses.                                                  *
 *                                                                                             *
 *                  collide algorithm (Gems)                separating axis (this one)         *
 * -----------------------------------------------------------------------------------------   *
 * number of tests      10000                                  10000                           *
 * avg cycles             641                                    464                           *
 * outside cycles         652                                    390                           *
 * overlap cycles         610                                    683                           *
 *                                                                                             *
 * The idea for this test is that it will early-exit when it can while the old test can't.     *
 * When we are passing a ray down our culling trees, it is common to encounter boxes that      *
 * are not intersecting the ray.  The faster we can reject these, the better!                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 * box - box to test                                                                           *
 * line - line to test                                                                         *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 * overlap status of the line with respect to the box, either INSIDE, OUTSIDE, or OVERLAPPED   *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/19/99   gth : Created.                                                                 *
 *=============================================================================================*/
CollisionMath::OverlapType CollisionMath::Overlap_Test(const AABoxClass & box,const LineSegClass & line)
{
	// If both endpoints are in, return INSIDE
	// If either was inside, return OVERLAPPED
	int inside_point_count = 0;
	if (CollisionMath::Overlap_Test(box,line.Get_P0()) == INSIDE) inside_point_count++;
	if (CollisionMath::Overlap_Test(box,line.Get_P1()) == INSIDE) inside_point_count++;
	
	if (inside_point_count == 2) {
	
		return INSIDE;
	
	} else if (inside_point_count == 1) {
	
		return OVERLAPPED;
	
	} else {

		// Here I'm using the separating axis theorem to test if the line-segment
		// and the box can be separated by a plane.  Any two convex objects that are
		// not intersecting can be separated by a plane defined by either a 
		// face normal from one of the objects or the cross-product of an edge from 
		// each object.  In the case of an axis-aligned box and a line-segment, we
		// have to check the three coordinate axes and the cross product between
		// each and the direction vector of the line segment.
		//
		// Here is the general test for an arbitrary axis:
		// -----------------------------------------------
		// box_proj = fabs(extent.X*axis.X) + fabs(extent.Y*axis.Y) + fabs(extent.Z*axis.Z)
		// p0_proj = fabs(dot_product(dp0,axis))
		// dp_proj = fabs(dot_product(dp,axis))
		// if (p0_proj > 0) {
		// 	if (p0_proj > box_proj - WWMath::Min(dp_proj,0.0f)) return OUTSIDE;
		// } else {
		// 	if (-p0_proj > box_proj + WWMath::Max(dp_proj,0.0f)) return OUTSIDE;
		// }
		//
		// In practice, there are optimizations you can make on each of the axes that
		// we need to test (see below).

		float box_proj,p0_proj,dp_proj;
		Vector3 dp0;
		Vector3::Subtract(line.Get_P0(),box.Center,&dp0);

		// Project box and line onto the x-axis
		// Since I know the axis is the x-axis, just take the x components of each
		// vector instead of doing the dot-product.  The projection of 'dp' is only
		// counted if it points towards the center of the box (i.e. it decreases the
		// chances of them being separated...)
		box_proj = box.Extent.X;
		p0_proj = dp0.X;
		dp_proj = line.Get_DP().X;
		if (p0_proj > 0) {
			if (p0_proj > box_proj - WWMath::Min(dp_proj,0.0f)) return OUTSIDE;
		} else {
			if (-p0_proj > box_proj + WWMath::Max(dp_proj,0.0f)) return OUTSIDE;
		}

		// Project box and line onto the y-axis
		box_proj = box.Extent.Y;
		p0_proj = dp0.Y;
		dp_proj = line.Get_DP().Y;
		if (p0_proj > 0) {
			if (p0_proj > box_proj - WWMath::Min(dp_proj,0.0f)) return OUTSIDE;
		} else {
			if (-p0_proj > box_proj + WWMath::Max(dp_proj,0.0f)) return OUTSIDE;
		}

		// Project box and line onto the z-axis
		box_proj = box.Extent.Z;
		p0_proj = dp0.Z;
		dp_proj = line.Get_DP().Z;
		if (p0_proj > 0) {
			if (p0_proj > box_proj - WWMath::Min(dp_proj,0.0f)) return OUTSIDE;
		} else {
			if (-p0_proj > box_proj + WWMath::Max(dp_proj,0.0f)) return OUTSIDE;
		}
	
		// Project box and line onto (x cross line)
		// I'm manually optimizing the cross-product and taking advantage of the fact
		// that 'dp' will always project to zero for this axis.
		Vector3 axis;
		axis.Set(0,-line.Get_Dir().Z,line.Get_Dir().Y);	// == (1,0,0) cross (x,y,z)
		box_proj = WWMath::Fabs(axis.Y*box.Extent.Y) + WWMath::Fabs(axis.Z*box.Extent.Z);
		p0_proj = Vector3::Dot_Product(axis,dp0);
		if (WWMath::Fabs(p0_proj) > box_proj) return OUTSIDE;

		// Project box and line onto (y cross line)
		axis.Set(line.Get_Dir().Z,0,-line.Get_Dir().X);	// == (0,1,0) cross (x,y,z)
		box_proj = WWMath::Fabs(axis.X*box.Extent.X) + WWMath::Fabs(axis.Z*box.Extent.Z);
		p0_proj = Vector3::Dot_Product(axis,dp0);
		if (WWMath::Fabs(p0_proj) > box_proj) return OUTSIDE;

		// Project box and line onto (z cross line)
		axis.Set(-line.Get_Dir().Y,line.Get_Dir().X,0);	// == (0,0,1) cross (x,y,z)
		box_proj = WWMath::Fabs(axis.X*box.Extent.X) + WWMath::Fabs(axis.Y*box.Extent.Y);
		p0_proj = Vector3::Dot_Product(axis,dp0);
		if (WWMath::Fabs(p0_proj) > box_proj) return OUTSIDE;

	}
	return OVERLAPPED;
}

#if 0 // Alternate Overlap Test for AABox-Ray
CollisionMath::OverlapType CollisionMath::Overlap_Test(const AABoxClass & box,const LineSegClass & line)
{
	// If both endpoints are in, return INSIDE
	// If either was inside, return OVERLAPPED
	int inside_point_count = 0;
	if (CollisionMath::Overlap_Test(box,line.Get_P0()) == INSIDE) inside_point_count++;
	if (CollisionMath::Overlap_Test(box,line.Get_P1()) == INSIDE) inside_point_count++;
	
	if (inside_point_count == 2) {
	
		return INSIDE;
	
	} else if (inside_point_count == 1) {
	
		return OVERLAPPED;
	
	} else {

		// Now we know that both points are outside of the box so we 
		// have to detect if the ray penetrates the box.
		// I found this algorithm in one of the GEMS books...
		Vector3 boxmin,boxmax;
		Vector3::Subtract(box.Center,box.Extent,&boxmin);
		Vector3::Add(box.Center,box.Extent,&boxmax);

		float candidateplane[3];	// candidate intersection plane distance for each axis 
		float maxt[3];					// t value along the ray for each axis
		Vector3 coord;					// intersection point

		const int BOX_SIDE_NEGATIVE = -1;
		const int BOX_SIDE_MIDDLE = 0;
		const int BOX_SIDE_POSITIVE = 1;

		int quadrant[3];
		for (int i=0; i<3; i++) {
			if (line.Get_P0()[i] < boxmin[i]) {
				quadrant[i] = BOX_SIDE_NEGATIVE;
				candidateplane[i] = boxmin[i];
			} else if (line.Get_P0()[i] > boxmax[i]) {
				quadrant[i] = BOX_SIDE_POSITIVE;
				candidateplane[i] = boxmax[i];
			} else {
				quadrant[i] = BOX_SIDE_MIDDLE;
			}
		}

		// Calculate the 3 distances to the candidate planes
		for (i=0; i<3; i++) {
			if (quadrant[i] != BOX_SIDE_MIDDLE && line.Get_DP()[i] != 0.0f) {
				maxt[i] = (candidateplane[i] - line.Get_P0()[i]) / line.Get_DP()[i];
			} else {
				maxt[i] = -1.0f;
			}
		}

		// Get the largest of the maxt's for the final choice of intersection
		int intersection_plane = 0;
		for (i=1; i<3; i++) {
			if (maxt[i] > maxt[intersection_plane]) {
				intersection_plane = i;
			}
		}

		// If the ray is "in front" of all of the planes, return
		if (maxt[intersection_plane] < 0.0f) {
			return OUTSIDE;
		}
		
		// If the intersection is beyond the end of the ray, return
		if (maxt[intersection_plane] > 1.0f) {
			return OUTSIDE;
		}

		// Check if the ray is inside the box, i.e. on the two planes which
		// are not the intersection planes, the intersection point should
		// be between the min and max of the box.
		for (i=0; i<3; i++) {
			
			if (intersection_plane != i) {
			
				coord[i] = line.Get_P0()[i] + maxt[intersection_plane] * line.Get_DP()[i];
				if ((coord[i] < boxmin[i]) || (coord[i] > boxmax[i])) {
					return OUTSIDE;		// ray is outside the box
				} 

			} else {

				coord[i] = candidateplane[i];

			}
		}
	}
	return OVERLAPPED;
}
#endif // Alternate Overlap Test for AABox-Ray


/***********************************************************************************************
 * CollisionMath::Overlap_Test -- Tests overlap between an AABox and a triangle                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/19/99   gth : Created.                                                                 *
 *=============================================================================================*/
CollisionMath::OverlapType CollisionMath::Overlap_Test(const AABoxClass & box,const TriClass & tri)
{
	CastResultStruct res;
	CollisionMath::Collide(box,Vector3(0,0,0),tri,&res);
	return eval_overlap_collision(res);
}


/***********************************************************************************************
 * CollisionMath::Collide -- Collision test for a moving AABox and a plane                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/19/99   gth : Created.                                                                 *
 *=============================================================================================*/
bool CollisionMath::Collide
(
	const AABoxClass &		box,
	const Vector3 &			move_vector,
	const PlaneClass &		plane,
	CastResultStruct *		result
)
{
	float frac;

	float extent = box.Project_To_Axis(plane.N);
	float dist = Vector3::Dot_Product(plane.N,box.Center) + plane.D;
	float move = Vector3::Dot_Product(plane.N,move_vector);

	if (dist > extent) {
		if (dist + move > extent) {
			// entire move ok!
			frac = 1.0f;
		} else {
			// partial move allowed
			frac = (extent - dist) / move;
		}

	} else if (dist < -extent) {
		if (dist + move < -extent) {
			// entire move ok!
			frac = 1.0f;
		} else {
			// partial move allowed
			frac = (-extent - dist) / move;
		}

	} else {
		result->StartBad = true;
		result->Normal = plane.N;
		return true;
	}

	if (frac < result->Fraction) {
		result->Fraction = frac;
		result->Normal = plane.N;
		if (result->ComputeContactPoint) {
			Vector3 move_dir(move_vector);
			move_dir.Normalize();
			float move_extent = Vector3::Dot_Product(box.Extent,move_dir);
			result->ContactPoint = box.Center + result->Fraction * move_vector + move_extent * move_dir;
		}
		return true;
	}

	return false;
}





/*
** AABCollisionStruct
** Contains all of the intermediate and temporary values used by
** the set of functions used in detecting collisions for aab's
*/
struct AABCollisionStruct
{
	AABCollisionStruct(const AABoxClass &box0,const Vector3 &move0,const AABoxClass & box1,const Vector3 &move1) :
		StartBad(true),													// Startbad is true until one of the axes clears it
		AxisId(-1),															// AxisId will be the axis that allowed the longest move
		MaxFrac(0.0f),														// MaxFrac is the longest allowed move so far
		Box0(box0),															
		Box1(box1)
	{
		Vector3::Subtract(box1.Center,box0.Center,&C);			// vector from center of box0 to center of box1
		Vector3::Subtract(move1,move0,&M);							// move vector relative to stationary box0
	}

	bool						StartBad;			// Inital configuration is intersecting?
	float						MaxFrac;				// Longest move allowed so far
	int						AxisId;				// Last separating axis
	int						Side;					// which side of the interval

	Vector3					C;						// Vector from the center0 to center1
	Vector3					M;						// Move vector relative to stationary box0
	
	const AABoxClass &	Box0;
	const AABoxClass &	Box1;

private:

	//not implemented
	AABCollisionStruct(const AABCollisionStruct&);
	AABCollisionStruct & operator = (const AABCollisionStruct&);
};


/***********************************************************************************************
 * aab_separation_test -- tests two AAB's for separation on an axis                            *
 *                                                                                             *
 * This function is very similar to the obb_separation_test.  If a flaw is found in either,    *
 * we should update the other...                                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/19/99   gth : Created.                                                                 *
 *=============================================================================================*/
static inline bool aab_separation_test
(
	AABCollisionStruct & context,
	int						axis
)
{
	// ra = box0 projection onto the axis
	// rb = box1 projection onto the axis
	// u0 = projected distance between the box centers at t0
	// u1 = projected distance between the box centers at t1
	float ra = context.Box0.Extent[axis];
	float rb = context.Box1.Extent[axis];
	float u0 = context.C[axis];
	float u1 = u0 + context.M[axis];

	float tmp;
	float rsum = ra+rb;

	if ( u0 + WWMATH_EPSILON > rsum ) { 
		context.StartBad = false; 
		if ( u1 > rsum ) { 
			context.MaxFrac = 1.0f; 
			return true;
		} else { 
			tmp = (rsum-u0)/(u1-u0);
			if ( tmp > context.MaxFrac ) {
				context.MaxFrac = tmp; 
				context.AxisId = axis;
				context.Side = +1;
			}
		}
	} else if ( u0 - WWMATH_EPSILON < -rsum ) {
		context.StartBad = false;
		if ( u1 < -rsum ) {
			context.MaxFrac = 1.0f; 
			return true;
		} else {
			tmp = (-rsum-u0)/(u1-u0);
			if ( tmp > context.MaxFrac ) {
				context.MaxFrac = tmp;
				context.AxisId = axis;
				context.Side = -1;
			} 
		} 
	} 
	return false;
}


/***********************************************************************************************
 * CollisionMath::Collide -- Collision test for two moving AABoxes                             *
 *                                                                                             *
 * this function sweeps two AABoxes and finds the first time of collision.                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 * currently there is no parateter for the movement of the second box.  the internal code      *
 * can handle it though.                                                                       *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/19/99   gth : Created.                                                                 *
 *=============================================================================================*/
bool CollisionMath::Collide(const AABoxClass & box,const Vector3 & move,const AABoxClass & box2,CastResultStruct * result)
{
	/*
	**	Test the X-axis
	** ra = box projection onto the axis
	** rb = box2 projection onto the axis
	** u0 = projected distance between the box centers at t0
	** u1 = projected distance between the box centers at t1
	*/
	AABCollisionStruct context(box,move,box2,Vector3(0,0,0));

	if (aab_separation_test(context,0)) {
		goto exit;
	}

	if (aab_separation_test(context,1)) {
		goto exit;
	}
	
	if (aab_separation_test(context,2)) {
		goto exit;
	}

exit:

	if (context.StartBad) {
		result->StartBad = true;
		result->Fraction = 0.0f;
		return true;
	}

	if (context.MaxFrac < result->Fraction) {

		result->Fraction = context.MaxFrac;
		result->Normal.Set(0,0,0);
		result->Normal[context.AxisId] = -context.Side;

		if (result->ComputeContactPoint) {
			//WWASSERT(0); // TODO
			WWDEBUG_SAY(("AABox-AABox collision does not currently support contact point computation"));
		}

		return true;
	}
	return false;
}
