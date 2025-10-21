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
 *                     $Archive:: /Commando/Code/ww3d2/inttest.h                              $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/14/01 9:19a                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifndef INTTEST_H
#define INTTEST_H

#include "always.h"
#include "aabox.h"
#include "obbox.h"
#include "tri.h"
#include "colmath.h"
#include "coltype.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IntersectionTestClass
//
// This is the base class for all of the 'Intersection' functions.  The intersection tests are
// purely boolean tests.  The base class only contains the CollisionType of the test.
//
// Every IntersectionTestClass should have the following functions:
//
//		bool Cull(const Vector3 & min,const Vector3 & max);
//		bool Cull(const AABoxClass & box);
//		bool Intersect_Triangle(const TriClass & tri);
//
// These are not virtual because I don't want to pay the price of virtual function
// calls at the point in the code where these are used.  It may be possible to
// write template functions if we use these exact function prototpyes for all
// collision test classes though.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
class IntersectionTestClass
{
public:
	IntersectionTestClass(int collision_type) : CollisionType(collision_type)								{ }
	IntersectionTestClass(const IntersectionTestClass & that) :	CollisionType(that.CollisionType)	{ }

public:
	int								CollisionType;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// AABoxIntersectionTestClass
//
// This is an intersection test which uses an Axis-Aligned Box
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
class AABoxIntersectionTestClass : public IntersectionTestClass
{
public:
	AABoxIntersectionTestClass(const AABoxClass & box,int collision_type) :
		IntersectionTestClass(collision_type),
		Box(box)
	{
	}

	AABoxIntersectionTestClass(const AABoxIntersectionTestClass & that) :
		IntersectionTestClass(that),
		Box(that.Box)
	{
	}

	bool								Cull(const Vector3 & cull_min,const Vector3 & cull_max);
	bool								Cull(const AABoxClass & cull_box);
	bool								Intersect_Triangle(const TriClass & tri);

public:
	AABoxClass						Box;					// world space aabox that we want to test with

};

inline bool AABoxIntersectionTestClass::Cull(const Vector3 & cull_min,const Vector3 & cull_max)
{
	Vector3 box_min;
	Vector3::Subtract(Box.Center,Box.Extent,&box_min);

	Vector3 box_max;
	Vector3::Add(Box.Center,Box.Extent,&box_max);

	if ((box_min.X > cull_max.X) || (box_max.X < cull_min.X)) return true;
	if ((box_min.Y > cull_max.Y) || (box_max.Y < cull_min.Y)) return true;
	if ((box_min.Z > cull_max.Z) || (box_max.Z < cull_min.Z)) return true;

	return false;
}

inline bool AABoxIntersectionTestClass::Cull(const AABoxClass & cull_box)
{
	Vector3 dc;
	Vector3 r;
	Vector3::Subtract(cull_box.Center,Box.Center,&dc);
	Vector3::Add(cull_box.Extent,Box.Extent,&r);

	if (WWMath::Fabs(dc.X) > r.X) return true;
	if (WWMath::Fabs(dc.Y) > r.Y) return true;
	if (WWMath::Fabs(dc.Z) > r.Z) return true;

	return false;
}

inline bool AABoxIntersectionTestClass::Intersect_Triangle(const TriClass & tri)
{
	return CollisionMath::Intersection_Test(Box,tri);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// OBBoxIntersectionTestClass
//
// This is an intersection test which uses an Axis-Aligned Box
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
class OBBoxIntersectionTestClass : public IntersectionTestClass
{
public:

	OBBoxIntersectionTestClass(const OBBoxClass & box,int collision_type);
	OBBoxIntersectionTestClass(const OBBoxIntersectionTestClass & that);
	OBBoxIntersectionTestClass(const OBBoxIntersectionTestClass & that,const Matrix3D & tm);
	OBBoxIntersectionTestClass(const AABoxIntersectionTestClass & that,const Matrix3D & tm);

	bool								Cull(const Vector3 & min,const Vector3 & max);
	bool								Cull(const AABoxClass & box);
	bool								Intersect_Triangle(const TriClass & tri);

protected:
	void								update_bounding_box(void);

public:
	OBBoxClass						Box;					// world space obbox that we want to test with
	AABoxClass						BoundingBox;		// axis aligned w-s bounding box
};


inline OBBoxIntersectionTestClass::OBBoxIntersectionTestClass(const OBBoxClass & box,int collision_type) :
	IntersectionTestClass(collision_type),
	Box(box)
{
	update_bounding_box();
}

inline OBBoxIntersectionTestClass::OBBoxIntersectionTestClass(const OBBoxIntersectionTestClass & that) :
	IntersectionTestClass(that),
	Box(that.Box)
{
	update_bounding_box();
}

inline OBBoxIntersectionTestClass::OBBoxIntersectionTestClass
(
	const OBBoxIntersectionTestClass & that,
	const Matrix3D & tm
) :
	IntersectionTestClass(that)
{
	OBBoxClass::Transform(tm,that.Box,&Box);
	update_bounding_box();
}

inline OBBoxIntersectionTestClass::OBBoxIntersectionTestClass
(
	const AABoxIntersectionTestClass & that,
	const Matrix3D & tm
) :
	IntersectionTestClass(that)
{
	Matrix3D::Transform_Vector(tm,that.Box.Center,&(Box.Center));
	Box.Extent = that.Box.Extent;
	Box.Basis = tm; // copies the 3x3 rotation portion of the transform
	update_bounding_box();
}

inline bool OBBoxIntersectionTestClass::Cull(const Vector3 & cull_min,const Vector3 & cull_max)
{
	Vector3 box_min;
	Vector3::Subtract(BoundingBox.Center,BoundingBox.Extent,&box_min);

	Vector3 box_max;
	Vector3::Add(BoundingBox.Center,BoundingBox.Extent,&box_max);

	if ((box_min.X > cull_max.X) || (box_max.X < cull_min.X)) return true;
	if ((box_min.Y > cull_max.Y) || (box_max.Y < cull_min.Y)) return true;
	if ((box_min.Z > cull_max.Z) || (box_max.Z < cull_min.Z)) return true;

	return false;
}

inline bool OBBoxIntersectionTestClass::Cull(const AABoxClass & cull_box)
{
	Vector3 dc;
	Vector3 r;
	Vector3::Subtract(cull_box.Center,BoundingBox.Center,&dc);
	Vector3::Add(cull_box.Extent,BoundingBox.Extent,&r);

	if (WWMath::Fabs(dc.X) > r.X) return true;
	if (WWMath::Fabs(dc.Y) > r.Y) return true;
	if (WWMath::Fabs(dc.Z) > r.Z) return true;

	return false;
}

inline bool OBBoxIntersectionTestClass::Intersect_Triangle(const TriClass & tri)
{
	return CollisionMath::Intersection_Test(Box,tri);
}

inline void OBBoxIntersectionTestClass::update_bounding_box(void)
{
	BoundingBox.Center = Box.Center;
	Box.Basis.Rotate_AABox_Extent(Box.Extent,&BoundingBox.Extent);
}



#endif
