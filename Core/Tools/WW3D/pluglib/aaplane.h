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
 *                     $Archive:: /Commando/Code/wwmath/aaplane.h                             $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 5/19/00 3:12p                                               $*
 *                                                                                             *
 *                    $Revision:: 8                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "always.h"
#include "vector3.h"


/*
** This class is used to describe an "axis-aligned" plane.  I.e, the normal
** of the plane is one of the three coordinate axes.
*/
class AAPlaneClass
{
public:

	enum AxisEnum { XNORMAL = 0, YNORMAL = 1, ZNORMAL = 2 };

	AAPlaneClass(void)																		{ }
	AAPlaneClass(AxisEnum normal,float dist) : Normal(normal),Dist(dist)		{ }

	void Set(AxisEnum normal,float dist);
	void Get_Normal(Vector3 * normal) const;

public:

	AxisEnum			Normal;
	float				Dist;

};

inline void AAPlaneClass::Set(AxisEnum normal,float dist)
{
	Normal = normal;
	Dist = dist;
}

inline void AAPlaneClass::Get_Normal(Vector3 * normal) const
{
	normal->Set(0,0,0);
	(*normal)[Normal] = 1.0f;
}
