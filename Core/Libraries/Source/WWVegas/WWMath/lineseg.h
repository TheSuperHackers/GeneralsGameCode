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
 *                     $Archive:: /Commando/Code/wwmath/lineseg.h                             $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 3/16/00 3:16p                                               $*
 *                                                                                             *
 *                    $Revision:: 21                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#if defined(_MSC_VER)
#pragma once
#endif

#ifndef LINESEG_H
#define LINESEG_H

#include "always.h"
#include "vector3.h"
#include "castres.h"

class TriClass;
class AABoxClass;
class OBBoxClass;
class PlaneClass;
class SphereClass;
class Matrix3D;


class LineSegClass
{

public:

	LineSegClass(void) { }
	LineSegClass(const Vector3 & p0,const Vector3 & p1) : P0(p0), P1(p1) { recalculate(); }
	LineSegClass(const LineSegClass & that,const Matrix3D & tm) { Set(that,tm); }

	void					Set(const Vector3 & p0,const Vector3 & p1) { P0 = p0; P1 = p1; recalculate(); }
	void					Set(const LineSegClass & that,const Matrix3D & tm);
	void					Set_Random(const Vector3 & min,const Vector3 & max);

	const Vector3 &	Get_P0() const { return P0; }				// start point
	const Vector3 &	Get_P1() const { return P1; }				// end point
	const Vector3 &	Get_DP() const { return DP; }				// difference of the two points
	const Vector3 &	Get_Dir() const { return Dir; }			// normalized direction.
	float					Get_Length() const { return Length; }	// length of the segment

	void					Compute_Point(float t,Vector3 * set) const	{ Vector3::Add(P0,t*DP,set); }
	
	Vector3				Find_Point_Closest_To(const Vector3 &pos) const;
	bool					Find_Intersection (const LineSegClass &other_line, Vector3 *p1, float *fraction1, Vector3 *p2, float *fraction2) const;

protected:

	void					recalculate(void) { DP = P1 - P0; Dir = DP; Dir.Normalize(); Length = DP.Length(); }

	Vector3				P0;		// start point
	Vector3				P1;		// end point
	Vector3				DP;		// difference of the two points
	Vector3				Dir;		// normalized direction.
	float					Length;	// length of the segment
};


#endif
