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
 *                     $Archive:: /Commando/Code/wwmath/colmathfrustum.cpp                    $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 3/29/00 5:40p                                               $*
 *                                                                                             *
 *                    $Revision:: 7                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
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
#include "frustum.h"
#include "wwdebug.h"


// TODO: Most of these overlap functions actually do not catch all cases of when
// the primitive is outside of the frustum...


// Frustum functions
CollisionMath::OverlapType
CollisionMath::Overlap_Test(const FrustumClass & frustum,const Vector3 & point)
{
	int mask = 0;

	for (int i = 0; i < 6; i++) {
		int result = CollisionMath::Overlap_Test(frustum.Planes[i],point);
		if (result == OUTSIDE) {
			return OUTSIDE;
		}
		mask |= result;
	}

	if (mask == INSIDE) {
		return INSIDE;
	}
	return OVERLAPPED;
}

CollisionMath::OverlapType
CollisionMath::Overlap_Test(const FrustumClass & frustum,const TriClass & tri)
{
	int mask = 0;

	// TODO: doesn't catch all cases...
	for (int i = 0; i < 6; i++) {
		int result = CollisionMath::Overlap_Test(frustum.Planes[i],tri);
		if (result == OUTSIDE) {
			return OUTSIDE;
		}
		mask |= result;
	}

	if (mask == INSIDE) {
		return INSIDE;
	}
	return OVERLAPPED;
}

CollisionMath::OverlapType
CollisionMath::Overlap_Test(const FrustumClass & frustum,const SphereClass & sphere)
{
	int mask = 0;

	// TODO: doesn't catch all cases...
	for (int i = 0; i < 6; i++) {
		int result = CollisionMath::Overlap_Test(frustum.Planes[i],sphere);
		if (result == OUTSIDE) {
			return OUTSIDE;
		}
		mask |= result;
	}

	if (mask == INSIDE) {
		return INSIDE;
	}
	return OVERLAPPED;
}

CollisionMath::OverlapType
CollisionMath::Overlap_Test(const FrustumClass & frustum,const AABoxClass & box)
{
	int mask = 0;

	// TODO: doesn't catch all cases...
	for (int i = 0; i < 6; i++) {
		int result = CollisionMath::Overlap_Test(frustum.Planes[i],box);
		if (result == OUTSIDE) {
			return OUTSIDE;
		}
		mask |= result;
	}

	if (mask == INSIDE) {
		return INSIDE;
	}
	return OVERLAPPED;
}


CollisionMath::OverlapType
CollisionMath::Overlap_Test(const FrustumClass & frustum,const OBBoxClass & box)
{
	int mask = 0;

	// TODO: doesn't catch all cases...
	for (int i = 0; i < 6; i++) {
		int result = CollisionMath::Overlap_Test(frustum.Planes[i],box);
		if (result == OUTSIDE) {
			return OUTSIDE;
		}
		mask |= result;
	}

	if (mask == INSIDE) {
		return INSIDE;
	}
	return OVERLAPPED;
}


CollisionMath::OverlapType
CollisionMath::Overlap_Test(const FrustumClass & frustum,const OBBoxClass & box,int & planes_passed)
{
	int mask = 0;

	// TODO: doesn't catch all cases...
	for (int i = 0; i < 6; i++) {

		int plane_bit = (1<<i);

		// only check this plane if we have to
		if ((planes_passed & plane_bit) == 0) {

			int result = CollisionMath::Overlap_Test(frustum.Planes[i],box);
			if (result == OUTSIDE) {
				return OUTSIDE;
			} else if (result == INSIDE) {
				planes_passed |= plane_bit;
			}
			mask |= result;

		} else {

			mask |= INSIDE;

		}
	}

	if (mask == INSIDE) {
		return INSIDE;
	}
	return OVERLAPPED;
}

