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
 *                     $Archive:: /Commando/Code/wwmath/frustum.h                             $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 3/29/00 10:20a                                              $*
 *                                                                                             *
 *                    $Revision:: 3                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "vector3.h"
#include "plane.h"


class FrustumClass
{
public:
	void Init(			const Matrix3D & camera,
							const Vector2 & viewport_min,
							const Vector2 & viewport_max,
							float znear,
							float zfar );

	const Vector3 &	Get_Bound_Min(void) const		{ return BoundMin; }
	const Vector3 &	Get_Bound_Max(void) const		{ return BoundMax; }

public:

	Matrix3D				CameraTransform;
	// Plane 0: NEAR
	// Plane 1: bottom
	// Plane 2: right
	// Plane 3: top
	// Plane 4: left
	// Plane 5: FAR
	PlaneClass			Planes[6];
	// Corner 0: NEAR upper left
	// Corner 1: NEAR upper right
	// Corner 2: NEAR lower left
	// Corner 3: NEAR lower right
	// Corner 4: FAR upper left
	// Corner 5: FAR upper right
	// Corner 6: FAR lower left
	// Corner 7: FAR lower right
	Vector3				Corners[8];
	Vector3				BoundMin;
	Vector3				BoundMax;
};
