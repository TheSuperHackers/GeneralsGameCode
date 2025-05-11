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
 *                     $Archive:: /Commando/Code/wwmath/colmath.h                             $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 5/04/01 8:25p                                               $*
 *                                                                                             *
 *                    $Revision:: 19                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef COLMATH_H
#define COLMATH_H

#ifndef ALWAYS_H
#include "always.h"
#endif

#ifndef VECTOR3_H
#include "vector3.h"
#endif

#ifndef CASTRES_H
#include "castres.h"
#endif

class AAPlaneClass;
class PlaneClass;
class LineSegClass;
class TriClass;
class SphereClass;
class AABoxClass;
class OBBoxClass;
class FrustumClass;

const float COLLISION_EPSILON = 0.001f;

/*
** #define COLMATH_STAT_TRACKING to enable stat tracking for the collision math functions
*/
#ifdef WWDEBUG
#define COLMATH_STAT_TRACKING
#endif


/**
** CollisionMath
** This is a collection of the low-level math functions for collision detection.
*/
class CollisionMath
{
public:

	////////////////////////////////////////////////////////////////////////////////////////
	// Intersect Functions.
	// These functions simply return a bool indicating whether the two operands intersect.
	////////////////////////////////////////////////////////////////////////////////////////
	static bool		Intersection_Test(const AABoxClass & box,const TriClass & tri);	
	static bool		Intersection_Test(const AABoxClass & box,const AABoxClass & box2);
	static bool		Intersection_Test(const AABoxClass & box,const OBBoxClass & box2);	
	static bool		Intersection_Test(const OBBoxClass & box,const TriClass & tri);
	static bool		Intersection_Test(const OBBoxClass & box,const AABoxClass & box2);
	static bool		Intersection_Test(const OBBoxClass & box,const OBBoxClass & box2);
	static bool		Intersection_Test(const SphereClass & sphere,const AABoxClass & box);
	static bool		Intersection_Test(const SphereClass & sphere,const OBBoxClass & box);

	////////////////////////////////////////////////////////////////////////////////////////
	// Overlap Functions.
	// Classify the second operand with respect to the first operand.
	// For example Overlap_Test(plane,point) tests whether 'point' is in front of or 
	// behind 'plane'.
	// OverlapType: This enumeration is the result of an overlap test.
	// It indicates whether the the object is in the positive (front/outside) space
	// of the volume, the negative (back/inside) space of the volume, or both (overlapping)
	////////////////////////////////////////////////////////////////////////////////////////
	enum OverlapType
	{
		POS				= 0x01,
		NEG				= 0x02,
		ON					= 0x04,
		BOTH				= 0x08,
		OUTSIDE			= POS,
		INSIDE			= NEG,
		OVERLAPPED		= BOTH,
		FRONT				= POS,
		BACK				= NEG
	};

	// AAPlane functions.  Where is operand B with respect to this AAPlane
	static OverlapType	Overlap_Test(const AAPlaneClass & plane,const Vector3 & point);
	static OverlapType	Overlap_Test(const AAPlaneClass & plane,const LineSegClass & line);
	static OverlapType	Overlap_Test(const AAPlaneClass & plane,const TriClass & tri);
	static OverlapType	Overlap_Test(const AAPlaneClass & plane,const SphereClass & sphere);
	static OverlapType	Overlap_Test(const AAPlaneClass & plane,const AABoxClass & box);
	static OverlapType	Overlap_Test(const AAPlaneClass & plane,const OBBoxClass & box);

	// Plane functions.  Where is operand B with respect to the plane
	static OverlapType	Overlap_Test(const PlaneClass & plane,const Vector3 & point);
	static OverlapType	Overlap_Test(const PlaneClass & plane,const LineSegClass & line);
	static OverlapType	Overlap_Test(const PlaneClass & plane,const TriClass & tri);
	static OverlapType	Overlap_Test(const PlaneClass & plane,const SphereClass & sphere);
	static OverlapType	Overlap_Test(const PlaneClass & plane,const Vector3 & center,const Vector3 & extent);
	inline static OverlapType	Overlap_Test(const PlaneClass & plane,const AABoxClass & box);
	static OverlapType	Overlap_Test(const PlaneClass & plane,const OBBoxClass & box);

	// Sphere functions.  Where is operand B with respect to the sphere
	static OverlapType	Overlap_Test(const SphereClass & sphere,const Vector3 & point);
	static OverlapType	Overlap_Test(const SphereClass & sphere,const LineSegClass & line);
	static OverlapType	Overlap_Test(const SphereClass & sphere,const TriClass & tri);
	static OverlapType	Overlap_Test(const SphereClass & sphere,const SphereClass & sphere2);
	static OverlapType	Overlap_Test(const SphereClass & sphere,const AABoxClass & aabox);
	static OverlapType	Overlap_Test(const SphereClass & sphere,const OBBoxClass & obbox);

	// AABox functions. Where is operand B with respect to the AABox
	static OverlapType	Overlap_Test(const AABoxClass & box,const Vector3 & point);
	static OverlapType	Overlap_Test(const AABoxClass & box,const LineSegClass & line);
	static OverlapType	Overlap_Test(const AABoxClass & box,const TriClass & tri);
	static OverlapType	Overlap_Test(const AABoxClass & box,const AABoxClass & box2);
	static OverlapType	Overlap_Test(const AABoxClass & box,const OBBoxClass & obbox);
	static OverlapType	Overlap_Test(const AABoxClass & box,const SphereClass & sphere);

	// OBBox functions, where is operand B with respect to the OBBox
	static OverlapType	Overlap_Test(const OBBoxClass & box,const Vector3 & point);
	static OverlapType	Overlap_Test(const OBBoxClass & box,const LineSegClass & line);
	static OverlapType	Overlap_Test(const OBBoxClass & box,const TriClass & tri);
	static OverlapType	Overlap_Test(const OBBoxClass & box,const AABoxClass & box2);
	static OverlapType	Overlap_Test(const OBBoxClass & box,const OBBoxClass & box2);

	// Frustum functions
	static OverlapType	Overlap_Test(const FrustumClass & frustum,const Vector3 & point);
	static OverlapType	Overlap_Test(const FrustumClass & frustum,const TriClass & tri);
	static OverlapType	Overlap_Test(const FrustumClass & frustum,const SphereClass & sphere);
	static OverlapType	Overlap_Test(const FrustumClass & frustum,const AABoxClass & box);
	static OverlapType	Overlap_Test(const FrustumClass & frustum,const OBBoxClass & box);

	// Frustum functions for hierachical culling systems.  
	// At your root node, just pass in planes_passed = 0, then the variable will be modified to 
	// indicate which planes that volume was inside.  You can then pass that value in for the 
	// test of all child nodes and optimize away some of the tests.  See AABTreeCullSystemClass 
	// for an example usage.
	static OverlapType	Overlap_Test(const FrustumClass & frustum,const AABoxClass & box,int & planes_passed);
	static OverlapType	Overlap_Test(const FrustumClass & frustum,const OBBoxClass & box,int & planes_passed);

	// Miscellaneous other Overlap tests
	static OverlapType	Overlap_Test(const Vector3 & min,const Vector3 & max,const LineSegClass & line);

	////////////////////////////////////////////////////////////////////////////////////////
	// Collision Functions.
	// Collide the first operand into the last operand.
	// For example Collide(box,move,tri) tests for collision between a box moving into
	// a triangle.
	////////////////////////////////////////////////////////////////////////////////////////

	// Line segment functions.  Intersect this line segment with the given object
	static bool		Collide(const LineSegClass & line,const AAPlaneClass & plane,CastResultStruct * result);
	static bool		Collide(const LineSegClass & line,const PlaneClass & plane,CastResultStruct * result);
	static bool		Collide(const LineSegClass & line,const TriClass & tri,CastResultStruct * result);
	static bool		Collide(const LineSegClass & line,const SphereClass & sphere,CastResultStruct * result);
	static bool		Collide(const LineSegClass & line,const AABoxClass & box,CastResultStruct * result);
	static bool		Collide(const LineSegClass & line,const OBBoxClass & box,CastResultStruct * result);

	// AABox functions
	static bool		Collide(const AABoxClass & box,const Vector3 & move,const PlaneClass & plane,CastResultStruct * result);
	static bool		Collide(const AABoxClass & box,const Vector3 & move,const TriClass & tri,CastResultStruct * result);
	static bool		Collide(const AABoxClass & box,const Vector3 & move,const AABoxClass & box2,CastResultStruct * result);
	static bool		Collide(const AABoxClass & box,const Vector3 & move,const OBBoxClass & box2,const Vector3 & move2,CastResultStruct * result);

	// OBBox functions
	static bool		Collide(const OBBoxClass & box,const Vector3 & move,const PlaneClass & plane,CastResultStruct * result);
	static bool		Collide(const OBBoxClass & box,const Vector3 & move,const TriClass & tri,const Vector3 & move2,CastResultStruct * result);
	static bool		Collide(const OBBoxClass & box,const Vector3 & move,const AABoxClass & box2,const Vector3 & move2,CastResultStruct * result);
	static bool		Collide(const OBBoxClass & box,const Vector3 & move,const OBBoxClass & box2,const Vector3 & move2,CastResultStruct * result);

	////////////////////////////////////////////////////////////////////////////////////////
	// Stats 
	// Note that these functions will only work if you have stat tracking enabled
	////////////////////////////////////////////////////////////////////////////////////////
	struct ColmathStatsStruct
	{	
		ColmathStatsStruct(void);
		void Reset(void);

		int TotalCollisionCount;
		int TotalCollisionHitCount;

		int CollisionRayTriCount;
		int CollisionRayTriHitCount;

		int CollisionAABoxTriCount;
		int CollisionAABoxTriHitCount;
		int CollisionAABoxAABoxCount;
		int CollisionAABoxAABoxHitCount;

		int CollisionOBBoxTriCount;
		int CollisionOBBoxTriHitCount;
		int CollisionOBBoxAABoxCount;
		int CollisionOBBoxAABoxHitCount;
		int CollisionOBBoxOBBoxCount;
		int CollisionOBBoxOBBoxHitCount;
	};
	
	static void									Reset_Stats(void)				{ Stats.Reset(); }
	static const ColmathStatsStruct &	Get_Current_Stats(void)		{ return Stats; }

private:

	static OverlapType eval_overlap_mask(int mask);
	static OverlapType eval_overlap_collision(const CastResultStruct & res);

	static const float COINCIDENCE_EPSILON;

	static ColmathStatsStruct				Stats;
};


inline CollisionMath::OverlapType CollisionMath::eval_overlap_mask(int mask)
{
	// check if all verts are "ON"
	if (mask == ON) {
		return ON;
	}

	// check if all verts are either "ON" or "POS"
	if ((mask & ~(POS | ON)) == 0) {
		return POS;
	}

	// check if all verts are either "ON" or "BACK"
	if ((mask & ~(NEG | ON)) == 0) {
		return NEG;
	}

	// otherwise, poly spans the plane.
	return BOTH;
}


inline CollisionMath::OverlapType CollisionMath::eval_overlap_collision(const CastResultStruct & res)
{
	if (res.Fraction < 1.0f) {
		return BOTH;
	} else {
		if (res.StartBad) {
			return NEG;
		} else {
			return POS;
		}
	}
}


/*
** Stat tracking Macros
*/
#ifdef COLMATH_STAT_TRACKING
#define TRACK_COLLISION_RAY_TRI				Stats.CollisionRayTriCount++; Stats.TotalCollisionCount++;
#define TRACK_COLLISION_RAY_TRI_HIT			Stats.CollisionRayTriHitCount++; Stats.TotalCollisionHitCount;
#define TRACK_COLLISION_AABOX_TRI			Stats.CollisionAABoxTriCount++; Stats.TotalCollisionCount++;
#define TRACK_COLLISION_AABOX_TRI_HIT		Stats.CollisionAABoxTriHitCount++; Stats.TotalCollisionHitCount++;
#define TRACK_COLLISION_AABOX_AABOX			Stats.CollisionAABoxAABoxCount++; Stats.TotalCollisionCount++;
#define TRACK_COLLISION_AABOX_AABOX_HIT	Stats.CollisionAABoxAABoxHitCount++; Stats.TotalCollisionHitCount++;
#define TRACK_COLLISION_OBBOX_TRI			Stats.CollisionOBBoxTriCount++; Stats.TotalCollisionCount++;
#define TRACK_COLLISION_OBBOX_TRI_HIT		Stats.CollisionOBBoxTriHitCount++; Stats.TotalCollisionHitCount++;
#define TRACK_COLLISION_OBBOX_AABOX			Stats.CollisionOBBoxAABoxCount++; Stats.TotalCollisionCount++;
#define TRACK_COLLISION_OBBOX_AABOX_HIT	Stats.CollisionOBBoxAABoxHitCount++; Stats.TotalCollisionHitCount++;
#define TRACK_COLLISION_OBBOX_OBBOX			Stats.CollisionOBBoxOBBoxCount++; Stats.TotalCollisionCount++;
#define TRACK_COLLISION_OBBOX_OBBOX_HIT	Stats.CollisionOBBoxOBBoxHitCount++; Stats.TotalCollisionHitCount++;
#else
#define TRACK_COLLISION_RAY_TRI				
#define TRACK_COLLISION_RAY_TRI_HIT			
#define TRACK_COLLISION_AABOX_TRI			
#define TRACK_COLLISION_AABOX_TRI_HIT		
#define TRACK_COLLISION_AABOX_AABOX			
#define TRACK_COLLISION_AABOX_AABOX_HIT	
#define TRACK_COLLISION_OBBOX_TRI			
#define TRACK_COLLISION_OBBOX_TRI_HIT		
#define TRACK_COLLISION_OBBOX_AABOX			
#define TRACK_COLLISION_OBBOX_AABOX_HIT	
#define TRACK_COLLISION_OBBOX_OBBOX			
#define TRACK_COLLISION_OBBOX_OBBOX_HIT	
#endif

#endif // COLMATH_H
