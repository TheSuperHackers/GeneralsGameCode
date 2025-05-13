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
 *                     $Archive:: /Commando/Code/wwmath/colmathplane.h                        $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 3/29/00 4:42p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   get_far_extent -- gets extents of a box projected onto an axis                            *
 *   CollisionMath::Overlap_Test -- Tests overlap between a plane and a point                  *
 *   CollisionMath::Overlap_Test -- Tests overlap between a plane and an AABox                 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#ifndef COLMATHPLANE_H
#define COLMATHPLANE_H

#include "always.h"
#include "plane.h"
#include "aabox.h"

/*
** Inline collision functions dealing with planes
** This module is meant to be included only in .CPP files after you include colmath.h
** It is not automatically included in order to reduce file dependencies...
*/

/***********************************************************************************************
 * get_far_extent -- gets extents of a box projected onto an axis                              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/29/2000  gth : Created.                                                                 *
 *=============================================================================================*/
inline void get_far_extent(const Vector3 & normal,const Vector3 & extent,Vector3 * posfarpt)
{
	if (WWMath::Fast_Is_Float_Positive(normal.X)) {
		posfarpt->X = extent.X;
	} else {
		posfarpt->X = -extent.X;
	}

	if (WWMath::Fast_Is_Float_Positive(normal.Y)) {
		posfarpt->Y = extent.Y;
	} else {
		posfarpt->Y = -extent.Y;
	}

	if (WWMath::Fast_Is_Float_Positive(normal.Z)) {
		posfarpt->Z = extent.Z;
	} else {
		posfarpt->Z = -extent.Z;
	}
}


/***********************************************************************************************
 * CollisionMath::Overlap_Test -- Tests overlap between a plane and a point                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/29/2000  gth : Created.                                                                 *
 *=============================================================================================*/
inline 
CollisionMath::OverlapType
CollisionMath::Overlap_Test(const PlaneClass & plane,const Vector3 & point)
{
	float delta = Vector3::Dot_Product(point,plane.N) - plane.D;
	if (delta > COINCIDENCE_EPSILON) {
		return POS;
	} 
	if (delta < -COINCIDENCE_EPSILON) {
		return NEG;
	}
	return ON;
}


/***********************************************************************************************
 * CollisionMath::Overlap_Test -- Tests overlap between a plane and an AABox                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/29/2000  gth : Created.                                                                 *
 *=============================================================================================*/
inline
CollisionMath::OverlapType
CollisionMath::Overlap_Test(const PlaneClass & plane,const AABoxClass & box)
{
	// First, we determine the the near and far points of the box in the
	// direction of the plane normal
	Vector3 posfarpt;
	Vector3 negfarpt;

	get_far_extent(plane.N,box.Extent,&posfarpt);
	
	negfarpt = -posfarpt;
	posfarpt += box.Center;
	negfarpt += box.Center;
	if (Overlap_Test(plane,negfarpt) == POS) {
		return POS;
	}
	if (Overlap_Test(plane,posfarpt) == NEG) {
		return NEG;
	}
	return BOTH;
}


#endif

