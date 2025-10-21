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
 *                 Project Name : wwmath                                                       *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwmath/obbox.h                               $*
 *                                                                                             *
 *                    Org Author:: Greg_h                                                       *
 *                                                                                             *
 *                       Author : Kenny Mitchell                                               *
 *                                                                                             *
 *                     $Modtime:: 06/26/02 4:04p                                             $*
 *                                                                                             *
 *                    $Revision:: 24                                                          $*
 *                                                                                             *
 * 06/26/02 KM Matrix name change to avoid MAX conflicts                                       *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   OBBoxClass::Transform -- transform an oriented box                                        *
 *   OBBoxClass::Project_To_Axis -- compute projection onto the given axis                     *
 *   OBBoxClass::Compute_Point -- computes position of a parametricly defined point            *
 *   OBBoxClass::Compute_Axis_Aligned_Extent -- computes extent of an AABox enclosing this box *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifndef OBBOX_H
#define OBBOX_H

#include "always.h"
#include "vector3.h"
#include "matrix3.h"
#include "matrix3d.h"
#include "wwmath.h"
#include "castres.h"

class TriClass;
class AABoxClass;
class PlaneClass;


/*
** OBBoxClass
**
** Oriented-Bounding-Box Class.
** This is a collision box in world space.
** Center - position of the center of the box
** Extents - size of the box
** Basis - rotation matrix defining the orientation of the box
**
** To find the world space coordinates of the "+x,+y,+z" corner of
** the bounding box you could use this equation:
** Vector3 corner = Center + Basis * Extent;
*/
class OBBoxClass
{
public:

	OBBoxClass(void) { }

	OBBoxClass(const OBBoxClass & that) :
		Basis(that.Basis),
		Center(that.Center),
		Extent(that.Extent)
	{ }

	OBBoxClass(const Vector3 & center,const Vector3 & extent) :
		Basis(1),
		Center(center),
		Extent(extent)
	{ }

	OBBoxClass(const Vector3 & center,const Vector3 & extent,const Matrix3x3 & basis) :
		Basis(basis),
		Center(center),
		Extent(extent)
	{ }

	OBBoxClass(const Vector3 * points, int num_points);

	bool operator== (const OBBoxClass &src);
	bool operator!= (const OBBoxClass &src);

	void		Init_From_Box_Points(Vector3 * points,int num_points);
	void		Init_Random(float min_extent = 0.5f,float max_extent = 1.0f);
	float		Project_To_Axis(const Vector3 & axis) const;
	float		Volume(void) const { return 2.0*Extent.X * 2.0*Extent.Y * 2.0*Extent.Z; }
	void		Compute_Point(float params[3],Vector3 * set_point) const;
	void		Compute_Axis_Aligned_Extent(Vector3 * set_extent) const;

	Matrix3x3	Basis;
	Vector3		Center;
	Vector3		Extent;

	static void	Transform(const Matrix3D & tm,const OBBoxClass & in,OBBoxClass * out);
};

// Test functions: slow, easy to understand version of box intersection code :)
bool Oriented_Boxes_Intersect(const OBBoxClass & box0,const OBBoxClass & box1);
bool Oriented_Boxes_Collide(const OBBoxClass & box0,const Vector3 & v0,const OBBoxClass & box1,const Vector3 & v1,float dt);
bool Oriented_Box_Intersects_Tri(const OBBoxClass & box,const TriClass & tri);


/***********************************************************************************************
 * OBBoxClass::Project_To_Axis -- compute projection onto the given axis                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
inline float OBBoxClass::Project_To_Axis(const Vector3 & axis) const
{
	float x = Extent[0] * Vector3::Dot_Product(axis,Vector3(Basis[0][0],Basis[1][0],Basis[2][0]));
	float y = Extent[1] * Vector3::Dot_Product(axis,Vector3(Basis[0][1],Basis[1][1],Basis[2][1]));
	float z = Extent[2] * Vector3::Dot_Product(axis,Vector3(Basis[0][2],Basis[1][2],Basis[2][2]));

	// projection is the sum of the absolute values of the projections of the three extents
	return (WWMath::Fabs(x) + WWMath::Fabs(y) + WWMath::Fabs(z));
}


/***********************************************************************************************
 * OBBoxClass::Transform -- transform an oriented box                                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   2/24/98    GTH : Created.                                                                 *
 *=============================================================================================*/
inline void OBBoxClass::Transform
(
	const Matrix3D &		tm,
	const OBBoxClass &	in,
	OBBoxClass *			out
)
{
	WWASSERT(out);
	WWASSERT(out!=&in);

	out->Extent = in.Extent;
	Matrix3D::Transform_Vector(tm,in.Center,&(out->Center));
	Matrix3x3::Multiply(tm,in.Basis,&(out->Basis));
}


/***********************************************************************************************
 * OBBoxClass::Compute_Point -- computes position of a parametricly defined point              *
 *                                                                                             *
 * set_point = Center + params[0]*A0 + params[1]*A1 + params[2]*A2                             *
 *                                                                                             *
 * INPUT:                                                                                      *
 * params - parametric description of a point in the box.  -1 < params[i] < 1                  *
 * set_point - pointer to a Vector3 to set.                                                    *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/2/99     GTH : Created.                                                                 *
 *=============================================================================================*/
inline void OBBoxClass::Compute_Point(float params[3],Vector3 * set_point) const
{
	Vector3 point = Extent;
	point.X *= params[0];
	point.Y *= params[1];
	point.Z *= params[2];

	Matrix3x3::Rotate_Vector(Basis,point,set_point);
	Vector3::Add(Center,*set_point,set_point);
}


/***********************************************************************************************
 * OBBoxClass::Compute_Axis_Aligned_Extent -- computes extent of an AABox enclosing this box   *
 *                                                                                             *
 * INPUT:                                                                                      *
 * set_extent - pointer to a Vector3 to put the result into                                    *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   11/15/99   gth : Created.                                                                 *
 *=============================================================================================*/
inline void OBBoxClass::Compute_Axis_Aligned_Extent(Vector3 * set_extent) const
{
	WWASSERT(set_extent != NULL);

	// x extent is the box projected onto the x axis
	set_extent->X =	WWMath::Fabs(Extent[0] * Basis[0][0]) +
							WWMath::Fabs(Extent[1] * Basis[0][1]) +
							WWMath::Fabs(Extent[2] * Basis[0][2]);

	set_extent->Y =	WWMath::Fabs(Extent[0] * Basis[1][0]) +
							WWMath::Fabs(Extent[1] * Basis[1][1]) +
							WWMath::Fabs(Extent[2] * Basis[1][2]);

	set_extent->Z =	WWMath::Fabs(Extent[0] * Basis[2][0]) +
							WWMath::Fabs(Extent[1] * Basis[2][1]) +
							WWMath::Fabs(Extent[2] * Basis[2][2]);
}


/***********************************************************************************************
 * OBBoxClass::operator== -- Comparison operator                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/21/00    PDS : Created.                                                                 *
 *=============================================================================================*/
inline bool OBBoxClass::operator== (const OBBoxClass &src)
{
	return (Center == src.Center) && (Extent == src.Extent) && (Basis == src.Basis);
}


/***********************************************************************************************
 * OBBoxClass::operator!= -- Comparison operator                                               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   6/21/00    PDS : Created.                                                                 *
 *=============================================================================================*/
inline bool OBBoxClass::operator!= (const OBBoxClass &src)
{
	return (Center != src.Center) || (Extent != src.Extent) || (Basis != src.Basis);
}

#endif
