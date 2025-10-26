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
 *                     $Archive:: /Commando/Code/wwmath/colmathsphere.cpp                     $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 4/25/01 2:05p                                               $*
 *                                                                                             *
 *                    $Revision:: 6                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   CollisionMath::Intersection_Test -- Sphere - AAbox intersection                           *
 *   CollisionMath::Intersection_Test -- Sphere - OBBox intersection                           *
 *   CollisionMath::Overlap_Test -- Sphere - Point overlap test                                *
 *   CollisionMath::Overlap_Test -- sphere line overlap test                                   *
 *   CollisionMath::Overlap_Test -- sphere triangle overlap test                               *
 *   CollisionMath::Overlap_Test -- Sphere - Sphere overlap test                               *
 *   CollisionMath::Overlap_Test -- Sphere - AABox overlap test                                *
 *   CollisionMath::Overlap_Test -- Sphere - OBBox overlap test                                *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "colmath.h"
#include "aaplane.h"
#include "plane.h"
#include "lineseg.h"
#include "tri.h"
#include "sphere.h"
#include "aabox.h"
#include "obbox.h"
#include "wwdebug.h"


// Sphere Intersection fucntions.  Does the sphere intersect the passed in object
/***********************************************************************************************
 * CollisionMath::Intersection_Test -- Sphere - AAbox intersection                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/25/2001  gth : Created.                                                                 *
 *=============================================================================================*/
bool CollisionMath::Intersection_Test(const SphereClass & sphere,const AABoxClass & box)
{
	/*
	** Simple but slightly inaccurate test, expand the box by the sphere's radius, then
	** test whether the sphere is contained in that new box.  This is actually testing
	** against a cube which encloses the sphere...
	*/
	Vector3 dc = box.Center - sphere.Center;
	if (WWMath::Fabs(dc.X) < box.Extent.X + sphere.Radius) return false;
	if (WWMath::Fabs(dc.Y) < box.Extent.Y + sphere.Radius) return false;
	if (WWMath::Fabs(dc.Z) < box.Extent.Z + sphere.Radius) return false;
	return true;
}


/***********************************************************************************************
 * CollisionMath::Intersection_Test -- Sphere - OBBox intersection                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/25/2001  gth : Created.                                                                 *
 *=============================================================================================*/
bool CollisionMath::Intersection_Test(const SphereClass & sphere,const OBBoxClass & box)
{
	/*
	** Compute the sphere's position in the box's coordinate system
	*/
	Matrix3D tm(box.Basis,box.Center);
	Vector3 box_rel_center;
	Matrix3D::Inverse_Transform_Vector(tm,sphere.Center,&box_rel_center);

	if (box.Extent.X < WWMath::Fabs(box_rel_center.X)) return false;
	if (box.Extent.Y < WWMath::Fabs(box_rel_center.Y)) return false;
	if (box.Extent.Z < WWMath::Fabs(box_rel_center.Z)) return false;

	return true;
}

// Sphere Overlap functions.  Where is operand B with respect to the sphere
/***********************************************************************************************
 * CollisionMath::Overlap_Test -- Sphere - Point overlap test                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/25/2001  gth : Created.                                                                 *
 *=============================================================================================*/
CollisionMath::OverlapType
CollisionMath::Overlap_Test(const SphereClass & sphere,const Vector3 & point)
{
	float r2 = (point - sphere.Center).Length2();
	if (r2 < sphere.Radius * sphere.Radius - COINCIDENCE_EPSILON) {
		return NEG;
	}
	if (r2 > sphere.Radius * sphere.Radius + COINCIDENCE_EPSILON) {
		return POS;
	}
	return ON;
}


/***********************************************************************************************
 * CollisionMath::Overlap_Test -- sphere line overlap test                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/25/2001  gth : Created.                                                                 *
 *=============================================================================================*/
CollisionMath::OverlapType
CollisionMath::Overlap_Test(const SphereClass & /*sphere*/,const LineSegClass & /*line*/)
{
	WWASSERT(0); //TODO
	return POS;
}


/***********************************************************************************************
 * CollisionMath::Overlap_Test -- sphere triangle overlap test                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/25/2001  gth : Created.                                                                 *
 *=============================================================================================*/
CollisionMath::OverlapType
CollisionMath::Overlap_Test(const SphereClass & /*sphere*/,const TriClass & /*tri*/)
{
	WWASSERT(0); //TODO
	return POS;
}


/***********************************************************************************************
 * CollisionMath::Overlap_Test -- Sphere - Sphere overlap test                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/25/2001  gth : Created.                                                                 *
 *=============================================================================================*/
CollisionMath::OverlapType
CollisionMath::Overlap_Test(const SphereClass & sphere,const SphereClass & sphere2)
{
	CollisionMath::OverlapType retval = OUTSIDE;

	float radius	= sphere.Radius + sphere2.Radius;
	float dist2		= (sphere2.Center - sphere.Center).Length2();

	if (dist2 == 0 && sphere.Radius == sphere2.Radius) {
		retval = OVERLAPPED;
	} else if (dist2 <= radius * radius - COINCIDENCE_EPSILON) {
		retval = INSIDE;
	}

	return retval;
}


/***********************************************************************************************
 * CollisionMath::Overlap_Test -- Sphere - AABox overlap test                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/25/2001  gth : Created.                                                                 *
 *=============================================================================================*/
CollisionMath::OverlapType
CollisionMath::Overlap_Test(const SphereClass & sphere,const AABoxClass & aabox)
{
	// TODO: overlap function that detects containment?
	return ( Intersection_Test(sphere,aabox) ? BOTH : POS );
}


/***********************************************************************************************
 * CollisionMath::Overlap_Test -- Sphere - OBBox overlap test                                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/25/2001  gth : Created.                                                                 *
 *=============================================================================================*/
CollisionMath::OverlapType
CollisionMath::Overlap_Test(const SphereClass & sphere,const OBBoxClass & obbox)
{
	// TODO: overlap function that detects containment?
	return ( Intersection_Test(sphere,obbox) ? BOTH : POS );
}


