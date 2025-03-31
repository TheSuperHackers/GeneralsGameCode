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
 *                     $Archive:: /Commando/Code/wwmath/colmathaabox.h                        $*
 *                                                                                             *
 *              Original Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                      $Author:: Jani_p                                                      $*
 *                                                                                             *
 *                     $Modtime:: 5/08/01 5:01p                                               $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   CollisionMath::Overlap_Test -- test overlap between an AABox and a point                  *
 *   CollisionMath::Overlap_Test -- Tests overlap between two AABoxes                          *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#if defined(_MSC_VER)
#pragma once
#endif

#ifndef COLMATHAABOX_H
#define COLMATHAABOX_H

#include "always.h"
#include "aabox.h"
#include "vector3.h"
#include "lineseg.h"


/***********************************************************************************************
 * CollisionMath::Overlap_Test -- test overlap between an AABox and a point                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/14/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWINLINE CollisionMath::OverlapType CollisionMath::Overlap_Test(const AABoxClass & box,const Vector3 & point)
{
	if (WWMath::Fabs(point.X - box.Center.X) > box.Extent.X) return POS;
	if (WWMath::Fabs(point.Y - box.Center.Y) > box.Extent.Y) return POS;
	if (WWMath::Fabs(point.Z - box.Center.Z) > box.Extent.Z) return POS;

	return NEG;
}

/***********************************************************************************************
 * CollisionMath::Overlap_Test -- Tests overlap between two AABoxes                            *
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
WWINLINE CollisionMath::OverlapType CollisionMath::Overlap_Test(const AABoxClass & box,const AABoxClass & box2)
{
	Vector3 dc;
	Vector3::Subtract(box2.Center,box.Center,&dc);

	if (box.Extent.X + box2.Extent.X < WWMath::Fabs(dc.X)) return POS;
	if (box.Extent.Y + box2.Extent.Y < WWMath::Fabs(dc.Y)) return POS;
	if (box.Extent.Z + box2.Extent.Z < WWMath::Fabs(dc.Z)) return POS;

	if (	(dc.X + box2.Extent.X <= box.Extent.X) &&
			(dc.Y + box2.Extent.Y <= box.Extent.Y) &&
			(dc.Z + box2.Extent.Z <= box.Extent.Z) &&
			(dc.X - box2.Extent.X >= -box.Extent.X) &&
			(dc.Y - box2.Extent.Y >= -box.Extent.Y) &&
			(dc.Z - box2.Extent.Z >= -box.Extent.Z))
	{
		return NEG;	// inside;
	}

	return BOTH;
}

#endif

