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
 *                     $Archive:: /Commando/Code/wwmath/castres.h                             $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 11/14/00 2:59p                                              $*
 *                                                                                             *
 *                    $Revision:: 10                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "vector3.h"
#include "bittype.h"


/**
** CastResultStruct
** Result of a volume or ray cast operation will be stored in the following structure
** NOTE: If you can avoid it, do not enable ComputeContactPoint.  When casting rays, it is more
** efficient to use the resulting Fraction to compute the contact point outside of the
** collision detection code.  In the case of AABox sweeping for character collision detection,
** you don't usually need the actual point of contact, etc etc.
**
** The default state of ComputeContactPoint is *false*
*/
struct CastResultStruct
{
	CastResultStruct(void)	{ Reset(); }
	void		Reset(void)		{ StartBad = false; Fraction = 1.0f; Normal.Set(0,0,0); SurfaceType = 0; ComputeContactPoint = false; ContactPoint.Set(0,0,0); }

	bool		StartBad;		// was the initial configuration interpenetrating something?
	float		Fraction;		// fraction of the move up until collision
	Vector3	Normal;			// surface normal at the collision point
	uint32	SurfaceType;	// surface type of polygon at collision point (see W3D_SURFACE_TYPES in w3d_file.h)

	bool		ComputeContactPoint;		// This signals the collision code to compute the point of collision
	Vector3	ContactPoint;				// This will be set to the point of collision if ComputeContactPoint is true

};
