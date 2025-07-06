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
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /VSS_Sync/ww3d2/coltest.h                                   $*
 *                                                                                             *
 *                   Org Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               * 
 *                                                                                             * 
 *                     $Modtime:: 07/01/02 12:45p                                              $*
 *                                                                                             *
 *                    $Revision:: 5                                                           $*
 *                                                                                             *
 * 07/01/02 KM Coltype enum change to avoid MAX conflicts									   *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef COLTEST_H
#define COLTEST_H

#include "always.h"
#include "castres.h"
#include "lineseg.h"
#include "aabox.h"
#include "obbox.h"
#include "tri.h"
#include "colmath.h"
#include "coltype.h"

class RenderObjClass;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CollisionTestClass
// 
// Each type of collision test will have an associated class which
// ties together all of the information necessary for the test.  
// These classes also provide a perfect place to add any information
// which can be pre-calculated.
//
// The base class: CollisionTestClass simply contains a pointer to
// the user's CastResultStruct which will contain the results of
// the collision test.  I store a pointer to a result structure
// because in some cases, new CollisionTestClasses are created in
// the process of checking (e.g. if the test needs to be transformed
// into another coordinate system) and I did not want to be 
// constantly copying the result structure around.  So, the user
// must allocate a result structure (usually on the stack) and keep it 
// until the CollisionTestClass is discarded.
//
// Every CollisionTestClass should have the following functions:
//
//		bool Cull(const Vector3 & min,const Vector3 & max);
//		bool Cull(const AABoxClass & box);
//		bool Cast_To_Triangle(const TriClass & tri);
//
// These are not virtual because I don't want to pay the price of virtual function
// calls at the point in the code where these are used.  It may be possible to 
// write template functions if we use these exact function prototpyes for all 
// collision test classes though.
//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CollisionTestClass
{
public:
	CollisionTestClass(CastResultStruct * res,int collision_type);
	CollisionTestClass(const CollisionTestClass & that);

public:
	CastResultStruct *			Result;
	int								CollisionType; 
	RenderObjClass *				CollidedRenderObj;		
};


inline CollisionTestClass::CollisionTestClass(CastResultStruct * res,int collision_type) :
	Result(res),
	CollisionType(collision_type),
	CollidedRenderObj(NULL)
{
}

inline CollisionTestClass::CollisionTestClass(const CollisionTestClass & that) :
	Result(that.Result),
	CollisionType(that.CollisionType),
	CollidedRenderObj(that.CollidedRenderObj)
{
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// RayCollisionTestClass
// 
// Contains a linesegment to be tested and of course the inherited
// pointer to a CollisionStruct for the result
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
class RayCollisionTestClass : public CollisionTestClass
{
public:
	
	RayCollisionTestClass(const LineSegClass & ray,CastResultStruct * res,int collision_type = COLL_TYPE_0,bool check_translucent=false, bool check_hidden=false);
	RayCollisionTestClass(const RayCollisionTestClass & raytest,const Matrix3D & tm);
		
	bool Cull(const Vector3 & min,const Vector3 & max);
	bool Cull(const AABoxClass & box);
	bool Cast_To_Triangle(const TriClass & tri);

public:

	LineSegClass 		Ray;
	bool CheckTranslucent;
	bool CheckHidden;

private:
	
	// not implemented
	RayCollisionTestClass(const RayCollisionTestClass &);
	RayCollisionTestClass & operator = (const RayCollisionTestClass &);

};


inline RayCollisionTestClass::RayCollisionTestClass(const LineSegClass & ray,CastResultStruct * res,int collision_type,bool check_translucent, bool check_hidden) :
	CollisionTestClass(res,collision_type),
	Ray(ray),
	CheckTranslucent(check_translucent),
	CheckHidden(check_hidden)
{
}

inline RayCollisionTestClass::RayCollisionTestClass(const RayCollisionTestClass & raytest,const Matrix3D & tm) :
	CollisionTestClass(raytest),
	Ray(raytest.Ray,tm),
	CheckTranslucent(raytest.CheckTranslucent),
	CheckHidden(raytest.CheckHidden)
{
}
		
inline bool RayCollisionTestClass::Cull(const Vector3 & min,const Vector3 & max) 
{ 
	return (CollisionMath::Overlap_Test(min,max,Ray) == CollisionMath::POS);
}

inline bool RayCollisionTestClass::Cull(const AABoxClass & box) 
{ 
	return (CollisionMath::Overlap_Test(box,Ray) == CollisionMath::POS);
}

inline bool RayCollisionTestClass::Cast_To_Triangle(const TriClass & tri) 
{
	return CollisionMath::Collide(Ray,tri,Result);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AABoxCollisionTestClass
// 
// Contains an Axis Aligned Box and a movement vector to be tested
// for collision.  Also adds Min and Max vectors both for the initial
// box and for the volume swept out by the box.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AABoxCollisionTestClass : public CollisionTestClass
{
public:
	
	AABoxCollisionTestClass(const AABoxClass & aabox,const Vector3 & move,CastResultStruct * res,int collision_type = COLL_TYPE_0);
	AABoxCollisionTestClass(const AABoxCollisionTestClass & that);

	enum ROTATION_TYPE 
	{
		ROTATE_NONE = 0,
		ROTATE_Z90,
		ROTATE_Z180,
		ROTATE_Z270
	};

	bool							Cull(const Vector3 & min,const Vector3 & max);
	bool							Cull(const AABoxClass & box);
	bool							Cast_To_Triangle(const TriClass & tri);

	void							Translate(const Vector3 & translation);
	void							Rotate(ROTATION_TYPE rotation);
	void							Transform(const Matrix3D & tm);

public:

	AABoxClass					Box;
	Vector3						Move;

	Vector3						SweepMin;
	Vector3						SweepMax;

private:
	
	// not implemented
	AABoxCollisionTestClass & operator = (const AABoxCollisionTestClass &);

};


inline void AABoxCollisionTestClass::Translate(const Vector3 & translation)
{
	// translate the test by the desired translation vector
	Box.Center += translation;
	SweepMin += translation;
	SweepMax += translation;
}
	
inline bool AABoxCollisionTestClass::Cull(const Vector3 & min,const Vector3 & max) 
{ 
	if ((SweepMin.X > max.X) || (SweepMax.X < min.X)) {
		return true;
	}

	if ((SweepMin.Y > max.Y) || (SweepMax.Y < min.Y)) {
		return true;
	}

	if ((SweepMin.Z > max.Z) || (SweepMax.Z < min.Z)) {
		return true;
	}
	return false;
}

inline bool AABoxCollisionTestClass::Cast_To_Triangle(const TriClass & tri) 
{
	return CollisionMath::Collide(Box,Move,tri,Result);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OBBoxCollisionTestClass
// 
// Contains an Oriented Bounding Box and a movement vector to be tested
// for collision.  Also adds Min and Max vectors (axis aligned box) 
// both for the initial box and for the volume swept out by the box.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
class OBBoxCollisionTestClass : public CollisionTestClass
{
public:

	OBBoxCollisionTestClass(const OBBoxClass & obbox,const Vector3 & move,CastResultStruct * res,int type = COLL_TYPE_0);
	OBBoxCollisionTestClass(const OBBoxCollisionTestClass & that);
	OBBoxCollisionTestClass(const OBBoxCollisionTestClass & that,const Matrix3D & tm);
	OBBoxCollisionTestClass(const AABoxCollisionTestClass & that,const Matrix3D & tm);
	
	bool Cull(const Vector3 & min,const Vector3 & max);
	bool Cull(const AABoxClass & box);
	bool Cast_To_Triangle(const TriClass & tri);

public:

	OBBoxClass	 				Box;
	Vector3						Move;
	Vector3						SweepMin;
	Vector3						SweepMax;

private:
	// not implemented
	OBBoxCollisionTestClass & operator = (const OBBoxCollisionTestClass &);
};


inline bool OBBoxCollisionTestClass::Cull(const Vector3 & min,const Vector3 & max) 
{ 
	if ((SweepMin.X > max.X) || (SweepMax.X < min.X)) {
		return true;
	}

	if ((SweepMin.Y > max.Y) || (SweepMax.Y < min.Y)) {
		return true;
	}

	if ((SweepMin.Z > max.Z) || (SweepMax.Z < min.Z)) {
		return true;
	}
	return false;
}

inline bool OBBoxCollisionTestClass::Cast_To_Triangle(const TriClass & tri) 
{
	return CollisionMath::Collide(Box,Move,tri,Vector3(0,0,0),Result);
}


#endif
