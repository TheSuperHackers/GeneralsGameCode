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
 *                     $Archive:: /Commando/Code/WWMath/sphere.h                              $*
 *                                                                                             *
 *                       Author:: Greg_h                                                       *
 *                                                                                             *
 *                     $Modtime:: 3/29/01 10:37a                                              $*
 *                                                                                             *
 *                    $Revision:: 18                                                          $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   SphereClass::SphereClass -- constructor                                                   *
 *   SphereClass::Init -- assign a new center and radius to this sphere                        *
 *   SphereClass::Re_Center -- move the center, update radius to enclose old sphere            *
 *   SphereClass::Add_Sphere -- expands 'this' sphere to enclose the given sphere              *
 *   SphereClass::Transform -- transforms this sphere                                          *
 *   SphereClass::Volume -- returns the volume of this sphere                                  *
 *   SphereClass::operator+= -- 'Add' a sphere to this one                                     *
 *   SphereClass::operator *= -- transform this sphere by the given radius                     *
 *   Spheres_Intersect -- test whether two spheres intersect                                   *
 *   Add_Spheres -- Add two spheres together, creating sphere which encloses both              *
 *   operator + -- Add two spheres together, creating a sphere which encloses both             *
 *   Transform Sphere -- transform a sphere                                                    *
 *   Transform_Sphere -- transform a sphere                                                    *
 *   operator * -- Transform a sphere                                                          *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */


#pragma once

#ifndef SPHERE_H
#define SPHERE_H

#include "always.h"
#include "vector3.h"
#include "matrix3d.h"


/////////////////////////////////////////////////////////////////////
// SphereClass
//
// Center - center of the sphere.
// Radius - radius of the sphere
//
/////////////////////////////////////////////////////////////////////
class SphereClass
{
public:
	inline SphereClass(void) { };
	inline SphereClass(const Vector3 & center,float radius) { Init(center,radius); }
	inline SphereClass(const Matrix3D& mtx,const Vector3 & center,float radius) { Init(mtx,center,radius); }
	inline SphereClass(const Vector3 & center,const SphereClass & s0);
	inline SphereClass(const Vector3 *Position, const int VertCount);

	inline void Init(const Vector3 & pos,float radius);
	inline void Init(const Matrix3D& mtx,const Vector3 & pos,float radius);
	inline void Re_Center(const Vector3 & center);
	inline void Add_Sphere(const SphereClass & s);
	inline void Transform(const Matrix3D & tm);
	inline float Volume(void) const;

	inline SphereClass & operator += (const SphereClass & s);
	inline SphereClass & operator *= (const Matrix3D & m);

	Vector3	Center;
	float		Radius;
};


/***********************************************************************************************
 * SphereClass::SphereClass -- constructor                                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/12/98    GTH : Created.                                                                 *
 *=============================================================================================*/
inline SphereClass::SphereClass(const Vector3 & center,const SphereClass & s0)
{
	float dist = (s0.Center - center).Length();
	Center = center;
	Radius = s0.Radius + dist;
}

inline SphereClass::SphereClass(const Vector3 *Position,const int VertCount)
{
	int i;
	double dx,dy,dz;

	// bounding sphere
	// Using the algorithm described in Graphics Gems I page 301.
	// This algorithm supposedly generates a bounding sphere within
	// 5% of the optimal one but is much faster and simpler to implement.
	Vector3 xmin(Position[0].X,Position[0].Y,Position[0].Z);
	Vector3 xmax(Position[0].X,Position[0].Y,Position[0].Z);
	Vector3 ymin(Position[0].X,Position[0].Y,Position[0].Z);
	Vector3 ymax(Position[0].X,Position[0].Y,Position[0].Z);
	Vector3 zmin(Position[0].X,Position[0].Y,Position[0].Z);
	Vector3 zmax(Position[0].X,Position[0].Y,Position[0].Z);


	// FIRST PASS:
	// finding the 6 minima and maxima points
	for (i=1; i<VertCount; i++) {
		if (Position[i].X < xmin.X) {
			xmin.X = Position[i].X; xmin.Y = Position[i].Y; xmin.Z = Position[i].Z;
		}
		if (Position[i].X > xmax.X) {
			xmax.X = Position[i].X; xmax.Y = Position[i].Y; xmax.Z = Position[i].Z;
		}
		if (Position[i].Y < ymin.Y) {
			ymin.X = Position[i].X; ymin.Y = Position[i].Y; ymin.Z = Position[i].Z;
		}
		if (Position[i].Y > ymax.Y) {
			ymax.X = Position[i].X; ymax.Y = Position[i].Y; ymax.Z = Position[i].Z;
		}
		if (Position[i].Z < zmin.Z) {
			zmin.X = Position[i].X; zmin.Y = Position[i].Y; zmin.Z = Position[i].Z;
		}
		if (Position[i].Z > zmax.Z) {
			zmax.X = Position[i].X; zmax.Y = Position[i].Y; zmax.Z = Position[i].Z;
		}
	}

	// xspan = distance between the 2 points xmin and xmax squared.
	// same goes for yspan and zspan.
	dx = xmax.X - xmin.X;
	dy = xmax.Y - xmin.Y;
	dz = xmax.Z - xmin.Z;
	double xspan = dx*dx + dy*dy + dz*dz;

	dx = ymax.X - ymin.X;
	dy = ymax.Y - ymin.Y;
	dz = ymax.Z - ymin.Z;
	double yspan = dx*dx + dy*dy + dz*dz;

	dx = zmax.X - zmin.X;
	dy = zmax.Y - zmin.Y;
	dz = zmax.Z - zmin.Z;
	double zspan = dx*dx + dy*dy + dz*dz;


	// Set points dia1 and dia2 to the maximally separated pair
	// This will be the diameter of the initial sphere
	Vector3 dia1 = xmin;
	Vector3 dia2 = xmax;
	double maxspan = xspan;

	if (yspan > maxspan) {
		maxspan = yspan;
		dia1 = ymin;
		dia2 = ymax;
	}
	if (zspan > maxspan) {
		maxspan = zspan;
		dia1 = zmin;
		dia2 = zmax;
	}


	// Compute initial center and radius and radius squared
	Vector3 center;
	center.X = (dia1.X + dia2.X) / 2.0f;
	center.Y = (dia1.Y + dia2.Y) / 2.0f;
	center.Z = (dia1.Z + dia2.Z) / 2.0f;

	dx = dia2.X - center.X;
	dy = dia2.Y - center.Y;
	dz = dia2.Z - center.Z;

	double radsqr = dx*dx + dy*dy + dz*dz;
	double radius = sqrt(radsqr);


	// SECOND PASS:
	// Increment current sphere if any points fall outside of it.
	for (i=0; i<VertCount; i++) {

		dx = Position[i].X - center.X;
		dy = Position[i].Y - center.Y;
		dz = Position[i].Z - center.Z;

		double testrad2 = dx*dx + dy*dy + dz*dz;

		if (testrad2 > radsqr) {

			// this point was outside the old sphere, compute a new
			// center point and radius which contains this point
			double testrad = sqrt(testrad2);

			// adjust center and radius
			radius = (radius + testrad) / 2.0;
			radsqr = radius * radius;

			double oldtonew = testrad - radius;
			center.X = (radius * center.X + oldtonew * Position[i].X) / testrad;
			center.Y = (radius * center.Y + oldtonew * Position[i].Y) / testrad;
			center.Z = (radius * center.Z + oldtonew * Position[i].Z) / testrad;
		}
	}

	Center = center;
	Radius = radius;
}


/***********************************************************************************************
 * SphereClass::Init -- assign a new center and radius to this sphere                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/12/98    GTH : Created.                                                                 *
 *=============================================================================================*/
inline void SphereClass::Init(const Vector3 & pos,float radius)
{
	Center = pos;
	Radius = radius;
}

/***********************************************************************************************
 * SphereClass::Init -- assign a new center and radius to this sphere                          *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/12/98    GTH : Created.                                                                 *
 *=============================================================================================*/
inline void SphereClass::Init(const Matrix3D& mtx, const Vector3 & pos,float radius)
{
#ifdef ALLOW_TEMPORARIES
	Center = mtx * pos;
#else
	mtx.mulVector3(pos, Center);
#endif
	Radius = radius;
}


/***********************************************************************************************
 * SphereClass::Re_Center -- move the center, update radius to enclose old sphere              *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/12/98    GTH : Created.                                                                 *
 *=============================================================================================*/
inline void SphereClass::Re_Center(const Vector3 & center)
{
	float dist = (Center - center).Length();
	Center = center;
	Radius += dist;
}


/***********************************************************************************************
 * SphereClass::Add_Sphere -- expands 'this' sphere to enclose the given sphere                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/12/98    GTH : Created.                                                                 *
 *=============================================================================================*/
inline void SphereClass::Add_Sphere(const SphereClass & s)
{
	if (s.Radius == 0.0f) return;

	float dist = (s.Center - Center).Length();
	if (dist == 0.0f) {
		Radius = (Radius > s.Radius) ? Radius : s.Radius;
		return;
	}

	float rnew = (dist + Radius + s.Radius) / 2.0f;

   // If rnew is smaller than either of the two sphere radii (it can't be
   // smaller than both of them), this means that the smaller sphere is
   // completely inside the larger, and the result of adding the two is
   // simply the larger sphere. If rnew isn't less than either of them, it is
   // the new radius - calculate the new center.
   if (rnew < Radius) {
      // The existing sphere is the result - do nothing.
   } else {
      if (rnew < s.Radius) {
         // The new sphere is the result:
         Init(s.Center, s.Radius);
      } else {
         // Neither sphere is completely inside the other, so rnew is the new
         // radius - calculate the new center
	      float lerp = (rnew - Radius) / dist;
	      Vector3 center = (s.Center - Center) * lerp + Center;
	      Init(center, rnew);
      }
   }
}

/***********************************************************************************************
 * SphereClass::Transform -- transforms this sphere                                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/12/99    GTH : Created.                                                                 *
 *=============================================================================================*/
inline void SphereClass::Transform(const Matrix3D & tm)
{
	// warning, assumes Orthogonal matrix
#ifdef ALLOW_TEMPORARIES
	Center = tm * Center;
#else
	tm.mulVector3(Center, Center);
#endif
}


/***********************************************************************************************
 * SphereClass::Volume -- returns the volume of this sphere                                    *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   3/22/99    GTH : Created.                                                                 *
 *=============================================================================================*/
inline float SphereClass::Volume(void) const
{
	return (4.0 / 3.0) * WWMATH_PI * (Radius * Radius * Radius);
}

/***********************************************************************************************
 * SphereClass::operator+= -- 'Add' a sphere to this one                                       *
 *                                                                                             *
 *    Expands 'this' sphere to also enclose the given sphere                                   *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/12/98    GTH : Created.                                                                 *
 *=============================================================================================*/
inline SphereClass & SphereClass::operator += (const SphereClass & s)
{
	Add_Sphere(s);
	return *this;
}


/***********************************************************************************************
 * SphereClass::operator *= -- transform this sphere by the given radius                       *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/12/98    GTH : Created.                                                                 *
 *=============================================================================================*/
inline SphereClass & SphereClass::operator *= (const Matrix3D & m)
{
	Init(m, Center, Radius);
	return *this;
}


/***********************************************************************************************
 * Spheres_Intersect -- test whether two spheres intersect                                     *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/12/98    GTH : Created.                                                                 *
 *=============================================================================================*/
inline bool Spheres_Intersect(const SphereClass & s0,const SphereClass & s1)
{
	Vector3 delta = s0.Center - s1.Center;
	float dist2 = Vector3::Dot_Product(delta, delta);

	if (dist2 < (s0.Radius + s1.Radius) * (s0.Radius + s1.Radius)) {
		return true;
	} else {
		return false;
	}
}


/***********************************************************************************************
 * Add_Spheres -- Add two spheres together, creating sphere which encloses both                *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/12/98    GTH : Created.                                                                 *
 *=============================================================================================*/
inline SphereClass Add_Spheres(const SphereClass & s0, const SphereClass & s1)
{
	if (s0.Radius == 0.0f) {
		return s1;
	} else {
		SphereClass result(s0);
		result.Add_Sphere(s1);
		return result;
	}
}


/***********************************************************************************************
 * operator + -- Add two spheres together, creating a sphere which encloses both               *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/12/98    GTH : Created.                                                                 *
 *=============================================================================================*/
inline SphereClass operator + (const SphereClass & s0,const SphereClass & s1)
{
	return Add_Spheres(s0,s1);
}


/***********************************************************************************************
 * Transform Sphere -- transform a sphere                                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/12/98    GTH : Created.                                                                 *
 *=============================================================================================*/
inline SphereClass Transform_Sphere(const Matrix3D & m, const SphereClass & s)
{
	// Warning, assumes Orthogonal matrix
	return SphereClass(m,s.Center,s.Radius);
}


/***********************************************************************************************
 * Transform_Sphere -- transform a sphere                                                      *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/12/98    GTH : Created.                                                                 *
 *=============================================================================================*/
inline void Transform_Sphere(const Matrix3D & m, const SphereClass & s,SphereClass & res)
{
	// warning, assumes Orthogonal matrix
#ifdef ALLOW_TEMPORARIES
	res.Center = m*s.Center;
#else
	m.mulVector3(s.Center, res.Center);
#endif
	res.Radius = s.Radius;
}


/***********************************************************************************************
 * operator * -- Transform a sphere                                                            *
 *                                                                                             *
 * INPUT:                                                                                      *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   8/12/98    GTH : Created.                                                                 *
 *=============================================================================================*/
inline SphereClass operator * (const Matrix3D & m, const SphereClass & s)
{
	return Transform_Sphere(m,s);
}



#endif

