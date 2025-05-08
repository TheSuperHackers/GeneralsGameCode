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
 *                     $Archive:: /Commando/Code/wwmath/colmathplane.cpp                      $*
 *                                                                                             *
 *                    Org Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               * 
 *                                                                                             * 
 *                     $Modtime:: 06/26/02 4:04p                                             $*
 *                                                                                             *
 *                    $Revision:: 10                                                           $*
 *                                                                                             *
 * 06/26/02 KM Matrix name change to avoid MAX conflicts                                       *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "colmath.h"
#include "colmathplane.h"
#include "aaplane.h"
#include "plane.h"
#include "lineseg.h"
#include "tri.h"
#include "sphere.h"
#include "aabox.h"
#include "obbox.h"
#include "wwdebug.h"



CollisionMath::OverlapType
CollisionMath::Overlap_Test(const AAPlaneClass & plane,const Vector3 & point)
{
	float delta = point[plane.Normal] - plane.Dist;
	if (delta > COINCIDENCE_EPSILON) {
		return POS;
	} 
	if (delta < -COINCIDENCE_EPSILON) {
		return NEG;
	}
	return ON;
}

CollisionMath::OverlapType
CollisionMath::Overlap_Test(const AAPlaneClass & plane,const LineSegClass & line)
{
	int mask = 0;
	mask |= CollisionMath::Overlap_Test(plane,line.Get_P0());
	mask |= CollisionMath::Overlap_Test(plane,line.Get_P1());
	return eval_overlap_mask(mask);
}

CollisionMath::OverlapType
CollisionMath::Overlap_Test(const AAPlaneClass & plane,const TriClass & tri)
{
	int mask = 0;
	mask |= CollisionMath::Overlap_Test(plane,*tri.V[0]);
	mask |= CollisionMath::Overlap_Test(plane,*tri.V[1]);
	mask |= CollisionMath::Overlap_Test(plane,*tri.V[2]);
	return eval_overlap_mask(mask);
}

CollisionMath::OverlapType
CollisionMath::Overlap_Test(const AAPlaneClass & plane,const SphereClass & sphere)
{
	float delta = sphere.Center[plane.Normal] - plane.Dist;
	if (delta > sphere.Radius) {
		return POS;
	} 
	if (delta < sphere.Radius) {
		return NEG;
	}
	return BOTH;
}

CollisionMath::OverlapType
CollisionMath::Overlap_Test(const AAPlaneClass & plane,const AABoxClass & box)
{
	float delta;
	int mask = 0;

	// check the 'min' side of the box
	delta = (box.Center[plane.Normal] - box.Extent[plane.Normal]) - plane.Dist;
	if (delta > WWMATH_EPSILON) {
		mask |= POS;
	} else if (delta < -WWMATH_EPSILON) {
		mask |= NEG;
	} else {
		mask |= ON;
	}
	
	// check the 'max' side of the box
	delta = (box.Center[plane.Normal] + box.Extent[plane.Normal]) - plane.Dist;
	if (delta > WWMATH_EPSILON) {
		mask |= POS;
	} else if (delta < -WWMATH_EPSILON) {
		mask |= NEG;
	} else {
		mask |= ON;
	}
	
	return eval_overlap_mask(mask);
}


CollisionMath::OverlapType
CollisionMath::Overlap_Test(const AAPlaneClass & /*plane*/,const OBBoxClass & /*box*/)
{
// TODO
	WWASSERT(0);
	return POS;
}


// Plane functions.  Where is operand B with respect to the plane

CollisionMath::OverlapType
CollisionMath::Overlap_Test(const PlaneClass & plane,const LineSegClass & line)
{
	int mask = 0;
	mask |= CollisionMath::Overlap_Test(plane,line.Get_P0());
	mask |= CollisionMath::Overlap_Test(plane,line.Get_P1());
	return eval_overlap_mask(mask);
}

CollisionMath::OverlapType
CollisionMath::Overlap_Test(const PlaneClass & plane,const TriClass & tri)
{
	int mask = 0;
	mask |= CollisionMath::Overlap_Test(plane,*tri.V[0]);
	mask |= CollisionMath::Overlap_Test(plane,*tri.V[1]);
	mask |= CollisionMath::Overlap_Test(plane,*tri.V[2]);
	return eval_overlap_mask(mask);
}

CollisionMath::OverlapType
CollisionMath::Overlap_Test(const PlaneClass & plane,const SphereClass & sphere)
{
	float dist = Vector3::Dot_Product(sphere.Center,plane.N) - plane.D;
	if (dist > sphere.Radius) {
		return POS;
	} 
	if (dist < -sphere.Radius) {
		return NEG;
	}
	return BOTH;
}

CollisionMath::OverlapType
CollisionMath::Overlap_Test(const PlaneClass & plane,const OBBoxClass & box)
{
	// rotate the plane normal into box coordinates
	Vector3 local_normal;
	Vector3 posfarpt;
	Vector3 negfarpt;
	Matrix3x3::Transpose_Rotate_Vector(box.Basis,plane.N,&local_normal);

	get_far_extent(local_normal,box.Extent,&posfarpt);

	// transform the two extreme box coordinates into world space
	Matrix3x3::Rotate_Vector(box.Basis,posfarpt,&posfarpt);
	negfarpt = -posfarpt;
	posfarpt += box.Center;
	negfarpt += box.Center;
	
	// overlap test
	if (Overlap_Test(plane,negfarpt) == POS) {
		return POS;
	}
	if (Overlap_Test(plane,posfarpt) == NEG) {
		return NEG;
	}
	return BOTH;
}
