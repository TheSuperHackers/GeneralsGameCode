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

/* $Header: /Commando/Code/Tools/pluglib/plane.h 8     4/22/98 6:36p Greg_h $ */
/***********************************************************************************************
 ***                            Confidential - Westwood Studios                              ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Voxel Technology                                             *
 *                                                                                             *
 *                    File Name : PLANE.h                                                      *
 *                                                                                             *
 *                   Programmer : Greg Hjelstrom                                               *
 *                                                                                             *
 *                   Start Date : 03/17/97                                                     *
 *                                                                                             *
 *                  Last Update : March 17, 1997 [GH]                                          *
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifndef VECTOR3_H
#include "vector3.h"
#endif

/*
** PlaneClass
**
** 3D-planes.  This class needs to be fleshed out but it does what I need for now.
*/

class PlaneClass
{
public:

	Vector3	N;
	float		D;

	PlaneClass(void) : N(0.0f,0.0f,1.0f), D(0.0f) { }
	PlaneClass(float a,float b,float c,float d) : N(a,b,c),D(d) { }
	PlaneClass(const Vector3 & normal,float dist) : N(normal), D(dist) { }

	// Create a plane given the normal and a point on the plane
	PlaneClass(const Vector3 & normal,const Vector3 & point);

	// Create a plane out of three points, ordered according to a right-hand convention.
	PlaneClass(const Vector3 & point1, const Vector3 & point2, const Vector3 & point3);

	void Set(float a,float b,float c,float d) { N[0] = a; N[1] = b; N[2] = c; D = d; }
};

inline PlaneClass::PlaneClass(const Vector3 & normal,const Vector3 & point)
{
	N = normal;
	D = Vector3::Dot_Product(normal , point);
}

inline PlaneClass::PlaneClass(const Vector3 & point1, const Vector3 & point2, const Vector3 & point3)
{
	N = Vector3::Cross_Product((point2 - point1), (point3 - point1));
	if (N != Vector3(0.0f, 0.0f, 0.0f)) {
		// Points are not colinear. Normalize N and calculate D.
		N.Normalize();
		D = N * point1;
	} else {
		// They are colinear - return default plane (constructors can't fail).
		N = Vector3(0.0f, 0.0f, 1.0f);
		D = 0.0f;
	}
}

inline bool In_Front(const Vector3 & point,const PlaneClass & plane)
{
	double dist = point * plane.N;
	return (dist > plane.D);
}
