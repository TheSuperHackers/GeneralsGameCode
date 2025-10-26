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
 *                 Project Name : WW3D                                                         *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/ww3d2/coltest.cpp                            $*
 *                                                                                             *
 *                       Author:: Greg Hjelstrom                                               *
 *                                                                                             *
 *                     $Modtime:: 5/07/01 10:26a                                              $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#include "coltest.h"


AABoxCollisionTestClass::AABoxCollisionTestClass(const AABoxCollisionTestClass & that) :
	CollisionTestClass(that),
	Box(that.Box),
	Move(that.Move),
	SweepMin(that.SweepMin),
	SweepMax(that.SweepMax)
{
}

AABoxCollisionTestClass::AABoxCollisionTestClass(const AABoxClass & aabox,const Vector3 & move,CastResultStruct * res,int collision_type) :
	CollisionTestClass(res,collision_type),
	Box(aabox),
	Move(move)
{
	SweepMin = Box.Center - Box.Extent;
	SweepMax = Box.Center + Box.Extent;

	Vector3 endmin = Box.Center + move - Box.Extent;
	Vector3 endmax = Box.Center + move + Box.Extent;

	if (endmax.X > SweepMax.X) SweepMax.X = endmax.X;
	if (endmax.Y > SweepMax.Y) SweepMax.Y = endmax.Y;
	if (endmax.Z > SweepMax.Z) SweepMax.Z = endmax.Z;

	if (endmin.X < SweepMin.X) SweepMin.X = endmin.X;
	if (endmin.Y < SweepMin.Y) SweepMin.Y = endmin.Y;
	if (endmin.Z < SweepMin.Z) SweepMin.Z = endmin.Z;
}


bool AABoxCollisionTestClass::Cull(const AABoxClass & box)
{
//	const float MOVE_THRESHOLD = 2.0f;
//	if (WWMath::Fabs(Move.X) + WWMath::Fabs(Move.Y) + WWMath::Fabs(Move.Z) > MOVE_THRESHOLD) {
//		CastResultStruct res;
//		return !Box.Cast_To_Box(Move,box,&res);
//	} else {
		Vector3 min_corner;
		Vector3 max_corner;
		Vector3::Subtract(box.Center,box.Extent,&min_corner);
		Vector3::Add(box.Center,box.Extent,&max_corner);

		if ((SweepMin.X > max_corner.X) || (SweepMax.X < min_corner.X)) {
			return true;
		}

		if ((SweepMin.Y > max_corner.Y) || (SweepMax.Y < min_corner.Y)) {
			return true;
		}

		if ((SweepMin.Z > max_corner.Z) || (SweepMax.Z < min_corner.Z)) {
			return true;
		}
		return false;
//	}
}


void AABoxCollisionTestClass::Rotate(ROTATION_TYPE rotation)
{

#ifndef NDEBUG

	int i;
	Matrix3D tm(1);
	switch(rotation) {
		case ROTATE_NONE:
			break;
		case ROTATE_Z90:
			tm = Matrix3D::RotateZ90;
			break;
		case ROTATE_Z180:
			tm = Matrix3D::RotateZ180;
			break;
		case ROTATE_Z270:
			tm = Matrix3D::RotateZ270;
			break;
	}

#ifdef ALLOW_TEMPORARIES
	Vector3 realcenter = tm * Box.Center;
#else
	Vector3 realcenter;
	tm.mulVector3(Box.Center, realcenter);
#endif

	Vector3 pts[8];
	Vector3 & min = SweepMin;
	Vector3 & max = SweepMax;

	pts[0].Set(min.X,min.Y,min.Z);
	pts[1].Set(min.X,max.Y,min.Z);
	pts[2].Set(max.X,max.Y,min.Z);
	pts[3].Set(max.X,min.Y,min.Z);

	pts[4].Set(min.X,min.Y,max.Z);
	pts[5].Set(min.X,max.Y,max.Z);
	pts[6].Set(max.X,max.Y,max.Z);
	pts[7].Set(max.X,min.Y,max.Z);

//	for (i=0; i<8; i++) {
//		pts[i] = tm * pts[i];
//	}
	tm.mulVector3Array(pts, 8);

	Vector3 realmin = pts[0];
	Vector3 realmax = pts[0];

	for (i=1; i<8; i++) {
		if (realmin.X >= pts[i].X) realmin.X = pts[i].X;
		if (realmin.Y >= pts[i].Y) realmin.Y = pts[i].Y;
		if (realmin.Z >= pts[i].Z) realmin.Z = pts[i].Z;

		if (realmax.X <= pts[i].X) realmax.X = pts[i].X;
		if (realmax.Y <= pts[i].Y) realmax.Y = pts[i].Y;
		if (realmax.Z <= pts[i].Z) realmax.Z = pts[i].Z;
	}


#endif


	// rotate the test by the desired rotation about the Z axis, special cased for
	// 90 degree rotations about Z.  arbitrary rotations cause the axis aligned
	// box to not be aligned any more :-)
	float tmp,minx,miny,maxx,maxy;
	switch(rotation) {
		case ROTATE_NONE:
			break;

		case ROTATE_Z90:
			// rotate the center point and the move vector
			tmp = Box.Center.X;		Box.Center.X = -Box.Center.Y;		Box.Center.Y = tmp;
			tmp = Move.X;				Move.X = -Move.Y;						Move.Y = tmp;

			// swap x and y for the extent
			tmp = Box.Extent.X;		Box.Extent.X = Box.Extent.Y;		Box.Extent.Y = tmp;

			// update sweep bounding box
			minx = SweepMin.X; miny = SweepMin.Y; maxx = SweepMax.X; maxy = SweepMax.Y;
			SweepMin.X = -maxy;
			SweepMin.Y = minx;
			SweepMax.X = -miny;
			SweepMax.Y = maxx;
			break;

		case ROTATE_Z180:
			// rotate center and move vector
			Box.Center.X = -Box.Center.X;
			Box.Center.Y = -Box.Center.Y;
			Move.X = -Move.X;
			Move.Y = -Move.Y;

			// update min/max boxes
			minx = SweepMin.X; miny = SweepMin.Y; maxx = SweepMax.X; maxy = SweepMax.Y;
			SweepMin.X = -maxx;
			SweepMin.Y = -maxy;
			SweepMax.X = -minx;
			SweepMax.Y = -miny;
			break;

		case ROTATE_Z270:
			// rotate center and move.
			tmp = Box.Center.X;		Box.Center.X = Box.Center.Y;		Box.Center.Y = -tmp;
			tmp = Move.X;				Move.X = Move.Y;						Move.Y = -tmp;

			// update extent (x and y axis swap)
			tmp = Box.Extent.X;		Box.Extent.X = Box.Extent.Y;		Box.Extent.Y = tmp;

			// update min/max boxes
			minx = SweepMin.X; miny = SweepMin.Y; maxx = SweepMax.X; maxy = SweepMax.Y;
			SweepMin.X = miny;
			SweepMin.Y = -maxx;
			SweepMax.X = maxy;
			SweepMax.Y = -minx;
			break;
	}

#ifndef NDEBUG

	assert((Box.Center - realcenter).Length() < 0.001f);
	assert((SweepMin - realmin).Length() < 0.001f);
	assert((SweepMax - realmax).Length() < 0.001f);

#endif
}


void AABoxCollisionTestClass::Transform(const Matrix3D & tm)
{
	// NOTE: this function will expand the box to enclose the rotated
	// form of the original box.  In practice, this function was only
	// implemented to double-check the results of the Translate and Rotate
	// functions.
	int i;
	Vector3 tmpcenter = Box.Center;
	Vector3 tmpextent = Box.Extent;
	tm.Transform_Center_Extent_AABox(tmpcenter,tmpextent,&Box.Center,&Box.Extent);

	Move = tm.Rotate_Vector(Move);

	Vector3 pts[8];
	Vector3 & min = SweepMin;
	Vector3 & max = SweepMax;

	pts[0].Set(min.X,min.Y,min.Z);
	pts[1].Set(min.X,max.Y,min.Z);
	pts[2].Set(max.X,max.Y,min.Z);
	pts[3].Set(max.X,min.Y,min.Z);

	pts[4].Set(min.X,min.Y,max.Z);
	pts[5].Set(min.X,max.Y,max.Z);
	pts[6].Set(max.X,max.Y,max.Z);
	pts[7].Set(max.X,min.Y,max.Z);

//	for (i=0; i<8; i++) {
//		pts[i] = tm * pts[i];
//	}
	tm.mulVector3Array(pts, 8);

	Vector3 realmin = pts[0];
	Vector3 realmax = pts[0];

	for (i=1; i<8; i++) {
		if (realmin.X >= pts[i].X) realmin.X = pts[i].X;
		if (realmin.Y >= pts[i].Y) realmin.Y = pts[i].Y;
		if (realmin.Z >= pts[i].Z) realmin.Z = pts[i].Z;

		if (realmax.X <= pts[i].X) realmax.X = pts[i].X;
		if (realmax.Y <= pts[i].Y) realmax.Y = pts[i].Y;
		if (realmax.Z <= pts[i].Z) realmax.Z = pts[i].Z;
	}

	SweepMin = realmin;
	SweepMax = realmax;
}



OBBoxCollisionTestClass::OBBoxCollisionTestClass
(
	const OBBoxClass & obbox,
	const Vector3 & move,
	CastResultStruct * res,
	int type
) :
	CollisionTestClass(res,type),
	Box(obbox),
	Move(move)
{
	Vector3 max_extent;
	max_extent.X =	WWMath::Fabs(Box.Basis[0][0] * Box.Extent.X) +
						WWMath::Fabs(Box.Basis[0][1] * Box.Extent.Y) +
						WWMath::Fabs(Box.Basis[0][2] * Box.Extent.Z) + 0.01f;

	max_extent.Y =	WWMath::Fabs(Box.Basis[1][0] * Box.Extent.X) +
						WWMath::Fabs(Box.Basis[1][1] * Box.Extent.Y) +
						WWMath::Fabs(Box.Basis[1][2] * Box.Extent.Z) + 0.01f;

	max_extent.Z =	WWMath::Fabs(Box.Basis[2][0] * Box.Extent.X) +
						WWMath::Fabs(Box.Basis[2][1] * Box.Extent.Y) +
						WWMath::Fabs(Box.Basis[2][2] * Box.Extent.Z) + 0.01f;

	SweepMin = Box.Center - max_extent;
	SweepMax = Box.Center + max_extent;

	Vector3 endmin = Box.Center + move - max_extent;
	Vector3 endmax = Box.Center + move + max_extent;

	if (endmax.X > SweepMax.X) SweepMax.X = endmax.X;
	if (endmax.Y > SweepMax.Y) SweepMax.Y = endmax.Y;
	if (endmax.Z > SweepMax.Z) SweepMax.Z = endmax.Z;

	if (endmin.X < SweepMin.X) SweepMin.X = endmin.X;
	if (endmin.Y < SweepMin.Y) SweepMin.Y = endmin.Y;
	if (endmin.Z < SweepMin.Z) SweepMin.Z = endmin.Z;
}

OBBoxCollisionTestClass::OBBoxCollisionTestClass(const OBBoxCollisionTestClass & that) :
	CollisionTestClass(that),
	Box(that.Box),
	Move(that.Move),
	SweepMin(that.SweepMin),
	SweepMax(that.SweepMax)
{
}

OBBoxCollisionTestClass::OBBoxCollisionTestClass
(
	const OBBoxCollisionTestClass &	that,
	const Matrix3D	&						tm
) :
	CollisionTestClass(that)
{
	tm.Transform_Min_Max_AABox(that.SweepMin,that.SweepMax,&SweepMin,&SweepMax);
	Matrix3D::Rotate_Vector(tm,that.Move,&Move);
	OBBoxClass::Transform(tm,that.Box,&Box);
}

OBBoxCollisionTestClass::OBBoxCollisionTestClass
(
	const AABoxCollisionTestClass &	that,
	const Matrix3D &						tm
) :
	CollisionTestClass(that)
{
	tm.Transform_Min_Max_AABox(that.SweepMin,that.SweepMax,&SweepMin,&SweepMax);
	Matrix3D::Rotate_Vector(tm,that.Move,&Move);
	Matrix3D::Transform_Vector(tm,that.Box.Center,&(Box.Center));
	Box.Extent = that.Box.Extent;
	Box.Basis = tm; // copies the 3x3 rotation portion of the transform
}


bool OBBoxCollisionTestClass::Cull(const AABoxClass & box)
{
	Vector3 min_corner;
	Vector3 max_corner;
	Vector3::Subtract(box.Center,box.Extent,&min_corner);
	Vector3::Add(box.Center,box.Extent,&max_corner);

	if ((SweepMin.X > max_corner.X) || (SweepMax.X < min_corner.X)) {
		return true;
	}

	if ((SweepMin.Y > max_corner.Y) || (SweepMax.Y < min_corner.Y)) {
		return true;
	}

	if ((SweepMin.Z > max_corner.Z) || (SweepMax.Z < min_corner.Z)) {
		return true;
	}
	return false;
}

