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
 *                     $Archive:: /Commando/Code/wwmath/aabox.h                               $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 5/08/01 6:35p                                               $*
 *                                                                                             *
 *                    $Revision:: 30                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   AABoxClass::Transform -- transform an aabox                                               *
 *   AABoxClass::Translate -- transform an aabox                                               *
 *   AABoxClass::Init -- create a box which bounds the given points									  *
 *   AABoxClass::Init -- initialize from a min-max form of a box                               *
 *   AABoxClass::Init_Min_Max -- init the box from a min and max vector                        *
 *   AABoxClass::Add_Point -- expand the box to contain the given point                        *
 *   AABoxClass::Project_To_Axis -- compute projection onto the given axis                     *
 *   AABoxClass::Intersects -- test for intersection with another static aabox                 *
 *   AABoxClass::Add_Box -- expand this box to enclose the passed box                          *
 *   AABoxClass::Add_Box -- Expand this box to enclose the passed box                          *
 *   MinMaxAABoxClass::Init -- init the box from an array of points                            *
 *   MinMaxAABoxClass::Init -- initializes this box from a center-extent box                   *
 *   MinMaxAABoxClass::Add_Point -- updates this box so it encloses the given point            *
 *   MinMaxAABoxClass::Add_Box -- update this box to enclose the given box                     *
 *   MinMaxAABoxClass::Add_Box -- Updates this box to enclose the specified box                *
 *   MinMaxAABoxClass::Transform -- Updates this box to enclose its transformed version        *
 *   MinMaxAABoxClass::Translate -- translates the box                                         *
 *   AABoxClass::Init -- Init from a line segment                                              *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#ifndef AABOX_H
#define AABOX_H

#include "always.h"
#include "matrix3d.h"
#include "lineseg.h"
#include "colmath.h"

class AABoxClass;
class MinMaxAABoxClass;
class OBBoxClass;
class TriClass;
class PlaneClass;
struct CastResultStruct;


/*
** AABoxClass
**
** Axis-Aligned Boxes.  I've coded these similar to the OrientedBoxClass only
** without a rotation matrix.  A similar algorithm is used to test the box
** for intersection.
*/
class AABoxClass
{

public:

	WWINLINE AABoxClass(void) { }

	WWINLINE AABoxClass(const Vector3 & center,const Vector3 & extent) :
		Center(center),
		Extent(extent)
	{ }

	AABoxClass(const MinMaxAABoxClass & minmaxbox) { Init(minmaxbox); }

	AABoxClass(Vector3 * points,int num) { Init(points,num); }

	bool operator== (const AABoxClass &src);
	bool operator!= (const AABoxClass &src);

	WWINLINE void		Init(const Vector3& center,const Vector3 & extent) { Center = center; Extent = extent; }
	WWINLINE void		Init(Vector3 * points,int num);
	WWINLINE void		Init(const MinMaxAABoxClass & minmaxbox);
	void		Init(const LineSegClass & line);
	void		Init_Min_Max(const Vector3 & min,const Vector3 & max);
	void		Init_Random(float min_center = -1.0f,float max_center = 1.0f,float min_extent = 0.5f,float max_extent = 1.0f);

	void		Add_Point(const Vector3 & point);
	void		Add_Box(const AABoxClass & b);
	void		Add_Box(const MinMaxAABoxClass & b);
	float		Project_To_Axis(const Vector3 & axis) const;

	void		Transform(const Matrix3D & tm);
	void		Translate(const Vector3 & pos);

	WWINLINE float Volume(void) const { return 2.0*Extent.X * 2.0*Extent.Y * 2.0*Extent.Z; }
	WWINLINE bool		Contains(const Vector3 & point) const;
	WWINLINE bool		Contains(const AABoxClass & other_box) const;
	WWINLINE bool		Contains(const MinMaxAABoxClass & other_box) const;

	static void	Transform(const Matrix3D & tm,const AABoxClass & in,AABoxClass * out);

	Vector3	Center;			// world space center
	Vector3	Extent;			// size of the box in the three directions

};

/*
** MinMaxAABoxClass
** This is another form of an AABox.  It can be faster to build one of these
** and then convert it into a center-extent AABox in some cases.  Its purpose
** is basically that.
*/
class MinMaxAABoxClass
{
public:

	WWINLINE MinMaxAABoxClass(void) { }

	WWINLINE MinMaxAABoxClass(const Vector3 & min_corner,const Vector3 & max_corner) :
		MinCorner(min_corner),
		MaxCorner(max_corner)
	{
	}

	WWINLINE MinMaxAABoxClass(Vector3 * points,int num) { Init(points,num); }

	WWINLINE MinMaxAABoxClass(const AABoxClass & that) { Init(that); }

	WWINLINE void		Init(Vector3 * points,int num);
	WWINLINE void		Init(const AABoxClass & box);
	void		Init_Empty(void);

	void		Add_Point(const Vector3 & point);
	void		Add_Box(const MinMaxAABoxClass & box);
	void		Add_Box(const AABoxClass & box);
	void		Add_Box(const Vector3 & min_corner,const Vector3 & max_corner);

	void		Transform(const Matrix3D & tm);
	void		Translate(const Vector3 & pos);

	WWINLINE float		Volume(void) const { Vector3 size = MaxCorner - MinCorner; return size.X*size.Y*size.Z; }

	Vector3	MinCorner;
	Vector3	MaxCorner;
};




/***********************************************************************************************
 * AABoxClass::Transform -- transform an aabox                                                 *
 *                                                                                             *
 * Note that this function expands the box to enclose its transformed form.                    *
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
WWINLINE void	AABoxClass::Transform(const Matrix3D & tm)
{
	Vector3 oldcenter = Center;
	Vector3 oldextent = Extent;
	tm.Transform_Center_Extent_AABox(oldcenter,oldextent,&Center,&Extent);
}


/***********************************************************************************************
 * AABoxClass::Translate -- translate an aabox                                                 *
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
WWINLINE void	AABoxClass::Translate(const Vector3 & trans)
{
	Center += trans;
}


/***********************************************************************************************
 * AABoxClass::operator== -- Comparison operator                                               *
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
WWINLINE  bool AABoxClass::operator== (const AABoxClass &src)
{
	return (Center == src.Center) && (Extent == src.Extent);
}


/***********************************************************************************************
 * AABoxClass::operator!= -- Comparison operator                                               *
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
WWINLINE bool AABoxClass::operator!= (const AABoxClass &src)
{
	return (Center != src.Center) || (Extent != src.Extent);
}


/***********************************************************************************************
 * AABoxClass::Init -- create a box which bounds the given points		                          *
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
WWINLINE void AABoxClass::Init(Vector3 * points,int num)
{
	Vector3 Min = points[0];
	Vector3 Max = points[0];

	for (int i=1; i<num; i++) {
		if (Min.X > points[i].X) Min.X = points[i].X;
		if (Min.Y > points[i].Y) Min.Y = points[i].Y;
		if (Min.Z > points[i].Z) Min.Z = points[i].Z;

		if (Max.X < points[i].X) Max.X = points[i].X;
		if (Max.Y < points[i].Y) Max.Y = points[i].Y;
		if (Max.Z < points[i].Z) Max.Z = points[i].Z;
	}

	Center = (Max + Min) * 0.5f;
	Extent = (Max - Min) * 0.5f;
}


/***********************************************************************************************
 * AABoxClass::Init -- initialize from a min-max form of a box                                 *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/31/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void AABoxClass::Init(const MinMaxAABoxClass & mmbox)
{
	Center = (mmbox.MaxCorner + mmbox.MinCorner) * 0.5f;
	Extent = (mmbox.MaxCorner - mmbox.MinCorner) * 0.5f;
}


/***********************************************************************************************
 * AABoxClass::Init -- Init from a line segment                                                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/27/2000  gth : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void AABoxClass::Init(const LineSegClass & line)
{
	Vector3 min_corner = line.Get_P0();
	Vector3 max_corner = line.Get_P0();

	if (min_corner.X > line.Get_P1().X) min_corner.X = line.Get_P1().X;
	if (min_corner.Y > line.Get_P1().Y) min_corner.Y = line.Get_P1().Y;
	if (min_corner.Z > line.Get_P1().Z) min_corner.Z = line.Get_P1().Z;

	if (max_corner.X < line.Get_P1().X) max_corner.X = line.Get_P1().X;
	if (max_corner.Y < line.Get_P1().Y) max_corner.Y = line.Get_P1().Y;
	if (max_corner.Z < line.Get_P1().Z) max_corner.Z = line.Get_P1().Z;

	Center = (max_corner + min_corner) * 0.5f;
	Extent = (max_corner - min_corner) * 0.5f;
}

/***********************************************************************************************
 * AABoxClass::Init_Min_Max -- init the box from a min and max vector                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   4/9/99     GTH : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void AABoxClass::Init_Min_Max(const Vector3 & min,const Vector3 & max)
{
	Center = (max + min) * 0.5f;
	Extent = (max - min) * 0.5f;
}


/***********************************************************************************************
 * AABoxClass::Add_Point -- expand the box to contain the given point                          *
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
WWINLINE void AABoxClass::Add_Point(const Vector3 & point)
{
	Vector3 Min = Center - Extent;
	Vector3 Max = Center + Extent;

	if (Min.X > point.X) Min.X = point.X;
	if (Min.Y > point.Y) Min.Y = point.Y;
	if (Min.Z > point.Z) Min.Z = point.Z;

	if (Max.X < point.X) Max.X = point.X;
	if (Max.Y < point.Y) Max.Y = point.Y;
	if (Max.Z < point.Z) Max.Z = point.Z;

	Center = (Max + Min) / 2.0f;
	Extent = (Max - Min) / 2.0f;
}


/***********************************************************************************************
 * AABoxClass::Add_Box -- expand this box to enclose the passed box                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/31/98    GTH : Created.                                                                 *
 *   9/29/2000  gth : Ok to add boxes with zero extent                                         *
 *=============================================================================================*/
WWINLINE void AABoxClass::Add_Box(const AABoxClass & b)
{
	Vector3 newmin = Center - Extent;
	Vector3 newmax = Center + Extent;
	newmin.Update_Min(b.Center - b.Extent);
	newmax.Update_Max(b.Center + b.Extent);

	Center = (newmax + newmin) * 0.5f;
	Extent = (newmax - newmin) * 0.5f;
}


/***********************************************************************************************
 * AABoxClass::Add_Box -- Expand this box to enclose the passed box                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/31/98    GTH : Created.                                                                 *
 *   9/29/2000  gth : Ok to add boxes with zero extent                                         *
 *=============================================================================================*/
WWINLINE void AABoxClass::Add_Box(const MinMaxAABoxClass & b)
{
	Vector3 newmin = Center - Extent;
	Vector3 newmax = Center + Extent;
	newmin.Update_Min(b.MinCorner);
	newmax.Update_Max(b.MaxCorner);

	Center = (newmax + newmin) * 0.5f;
	Extent = (newmax - newmin) * 0.5f;
}

/***********************************************************************************************
 * AABoxClass::Project_To_Axis -- compute projection onto the given axis                       *
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
WWINLINE float AABoxClass::Project_To_Axis(const Vector3 & axis) const
{
	float x = Extent[0] * axis[0];
	float y = Extent[1] * axis[1];
	float z = Extent[2] * axis[2];

	// projection is the sum of the absolute values of the projections of the three extents
	return (WWMath::Fabs(x) + WWMath::Fabs(y) + WWMath::Fabs(z));
}

/***********************************************************************************************
 * AABoxClass::Contains -- Test whether this box contains the given box                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/2/98     GTH : Created.                                                                 *
 *=============================================================================================*/
WWINLINE bool AABoxClass::Contains(const AABoxClass & other_box) const
{
	return CollisionMath::Overlap_Test(*this,other_box) == CollisionMath::INSIDE;
}

/***********************************************************************************************
 * AABoxClass::Contains -- Test whether this box contains the given box                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/2/98     GTH : Created.                                                                 *
 *=============================================================================================*/
WWINLINE bool AABoxClass::Contains(const MinMaxAABoxClass & other_box) const
{
	Vector3 bmin = Center - Extent;
	Vector3 bmax = Center + Extent;

	if (other_box.MinCorner.X < bmin.X) return false;
	if (other_box.MinCorner.Y < bmin.Y) return false;
	if (other_box.MinCorner.Z < bmin.Z) return false;

	if (other_box.MaxCorner.X > bmax.X) return false;
	if (other_box.MaxCorner.Y > bmax.Y) return false;
	if (other_box.MaxCorner.Z > bmax.Z) return false;

	return true;
}

/***********************************************************************************************
 * AABoxClass::Contains -- test whether this box contains the given point                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   9/2/98     GTH : Created.                                                                 *
 *=============================================================================================*/
WWINLINE bool AABoxClass::Contains(const Vector3 & point) const
{
	return CollisionMath::Overlap_Test(*this,point) == CollisionMath::INSIDE;
}

/***********************************************************************************************
 * MinMaxAABoxClass::Init -- init the box from an array of points                              *
 *                                                                                             *
 * Makes a box which encloses the given array of points                                        *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/31/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void MinMaxAABoxClass::Init(Vector3 * points,int num)
{
	assert(num > 0);
	assert(points != NULL);
	MinCorner = points[0];
	MaxCorner = points[0];
	for (int i=0; i<num; i++) {
		MinCorner.Update_Min(points[i]);
		MaxCorner.Update_Max(points[i]);
	}
}


/***********************************************************************************************
 * MinMaxAABoxClass::Init -- initializes this box from a center-extent box                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/31/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void MinMaxAABoxClass::Init(const AABoxClass & box)
{
	MinCorner = box.Center - box.Extent;
	MaxCorner = box.Center + box.Extent;
}


/***********************************************************************************************
 * MinMaxAABoxClass::Add_Point -- updates this box so it encloses the given point              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/31/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void MinMaxAABoxClass::Add_Point(const Vector3 & point)
{
	MinCorner.Update_Min(point);
	MaxCorner.Update_Max(point);
}


/***********************************************************************************************
 * MinMaxAABoxClass::Add_Box -- update this box to enclose the given box                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/31/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void MinMaxAABoxClass::Add_Box(const MinMaxAABoxClass & box)
{
	if (box.MinCorner == box.MaxCorner) return;

	MinCorner.Update_Min(box.MinCorner);
	MaxCorner.Update_Max(box.MaxCorner);
}


/***********************************************************************************************
 * MinMaxAABoxClass::Add_Box -- update this box to enclose the given box                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/31/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void MinMaxAABoxClass::Add_Box(const AABoxClass & box)
{
	if (box.Extent == Vector3(0.0f, 0.0f, 0.0f)) return;

	MinCorner.Update_Min(box.Center - box.Extent);
	MaxCorner.Update_Max(box.Center + box.Extent);
}

/***********************************************************************************************
 * MinMaxAABoxClass::Add_Box -- Updates this box to enclose the specified box                  *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/31/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void MinMaxAABoxClass::Add_Box(const Vector3 & min_corner,const Vector3 & max_corner)
{
	assert(max_corner.X >= min_corner.X);
	assert(max_corner.Y >= min_corner.Y);
	assert(max_corner.Z >= min_corner.Z);

	if (min_corner == max_corner) return;

	MinCorner.Update_Min(min_corner);
	MaxCorner.Update_Max(max_corner);
}


/***********************************************************************************************
 * MinMaxAABoxClass::Transform -- Updates this box to enclose its transformed version          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/31/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void MinMaxAABoxClass::Transform(const Matrix3D & tm)
{
	Vector3 oldmin = MinCorner;
	Vector3 oldmax = MaxCorner;
	tm.Transform_Min_Max_AABox(oldmin,oldmax,&MinCorner,&MaxCorner);
}


/***********************************************************************************************
 * MinMaxAABoxClass::Translate -- translates the box                                           *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   7/31/98    GTH : Created.                                                                 *
 *=============================================================================================*/
WWINLINE void MinMaxAABoxClass::Translate(const Vector3 & pos)
{
	MinCorner+=pos;
	MaxCorner+=pos;
}


#endif
