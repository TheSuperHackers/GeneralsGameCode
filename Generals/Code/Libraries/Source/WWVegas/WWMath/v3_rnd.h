/*
**	Command & Conquer Generals(tm)
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
 *                 Project Name : G                                                            *
 *                                                                                             *
 *                     $Archive:: /Commando/Code/wwmath/v3_rnd.h                              $*
 *                                                                                             *
 *                      $Author:: Greg_h                                                      $*
 *                                                                                             *
 *                     $Modtime:: 7/09/99 9:49a                                               $*
 *                                                                                             *
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#if defined(_MSC_VER)
#pragma once
#endif

#ifndef V3_RND_H
#define V3_RND_H

#include "always.h"
#include "vector3.h"
#include "random.h"
#include <limits.h>

/*
** Vector3Randomizer is an abstract class for generating random Vector3s.
** Examples: generating vectors in a sphere, cylinder, etc.
** This file contains several concrete derived classes; others may be defined
** in future.
** A possible future extension to this class would be to add a method to
** efficiently get an array of random points.
*/

class Vector3Randomizer {

	public:

		enum 
		{
			CLASSID_UNKNOWN		= 0xFFFFFFFF,
			CLASSID_SOLIDBOX		= 0,
			CLASSID_SOLIDSPHERE,
			CLASSID_HOLLOWSPHERE,
			CLASSID_SOLIDCYLINDER,
			CLASSID_MAXKNOWN,
			CLASSID_LAST			= 0x0000FFFF
		};

		virtual ~Vector3Randomizer(void)																		{ }

		// RTTI identifiction
		virtual unsigned int				Class_ID (void) const											= 0;

		// Return a random vector
		virtual void						Get_Vector(Vector3 &vector) 									= 0;

		// Get the maximum component possible for generated vectors
		virtual float						Get_Maximum_Extent(void)										= 0;

		// Scale all vectors produced in future
		virtual void						Scale(float scale)												= 0;

		// Clone the randomizer
		virtual Vector3Randomizer *	Clone(void) const													= 0;

	protected:

		// Derived classes should have protected copy CTors so users use the Clone() function

		// Utility functions
		float Get_Random_Float_Minus1_To_1()	{ return Randomizer * OOIntMax; }
		float Get_Random_Float_0_To_1()			{ return ((unsigned int)Randomizer) * OOUIntMax; }

		static const float OOIntMax;
		static const float OOUIntMax;
		static Random3Class	Randomizer;

	private:

		// Derived classes should have a private dummy assignment operator to block usage
};


/*
** Vector3SolidBoxRandomizer is a randomizer for generating points uniformly distributed inside a
** box which is centered on the origin.
*/
class Vector3SolidBoxRandomizer : public Vector3Randomizer {

	public:

		Vector3SolidBoxRandomizer(const Vector3 & extents);

		virtual unsigned int				Class_ID (void) const { return CLASSID_SOLIDBOX; }
		virtual const Vector3 &			Get_Extents (void) const { return Extents; }
		virtual void						Get_Vector(Vector3 &vector);
		virtual float						Get_Maximum_Extent(void);
		virtual void						Scale(float scale);
		virtual Vector3Randomizer *	Clone(void) const	{ return W3DNEW Vector3SolidBoxRandomizer(*this); }

	protected:

		// Derived classes should have protected copy CTors so users use the Clone() function
		Vector3SolidBoxRandomizer(const Vector3SolidBoxRandomizer &src) : Extents(src.Extents) { }

	private:

		// Derived classes should have a private dummy assignment operator to block usage
		Vector3SolidBoxRandomizer & operator = (const Vector3SolidBoxRandomizer &that) { that; return *this; }

		Vector3	Extents;
};


/*
** Vector3SolidSphereRandomizer is a randomizer for generating points uniformly distributed inside
** a sphere which is centered on the origin.
*/
class Vector3SolidSphereRandomizer : public Vector3Randomizer {

	public:

		Vector3SolidSphereRandomizer(float radius);

		virtual unsigned int				Class_ID (void) const { return CLASSID_SOLIDSPHERE; }
		virtual float						Get_Radius (void) const { return Radius; }
		virtual void						Get_Vector(Vector3 &vector);
		virtual float						Get_Maximum_Extent(void);
		virtual void						Scale(float scale);
		virtual Vector3Randomizer *	Clone(void) const	{ return W3DNEW Vector3SolidSphereRandomizer(*this); }

	protected:

		// Derived classes should have protected copy CTors so users use the Clone() function
		Vector3SolidSphereRandomizer(const Vector3SolidSphereRandomizer &src) : Radius(src.Radius) { }

	private:

		// Derived classes should have a private dummy assignment operator to block usage
		Vector3SolidSphereRandomizer & operator = (const Vector3SolidSphereRandomizer &that) { that; return *this; }

		float	Radius;
};


/*
** Vector3HollowSphereRandomizer is a randomizer for generating points uniformly distributed on the
** surface of a sphere which is centered on the origin.
*/
class Vector3HollowSphereRandomizer : public Vector3Randomizer {

	public:

		Vector3HollowSphereRandomizer(float radius);

		virtual unsigned int				Class_ID (void) const { return CLASSID_HOLLOWSPHERE; }
		virtual float						Get_Radius (void) const { return Radius; }
		virtual void						Get_Vector(Vector3 &vector);
		virtual float						Get_Maximum_Extent(void);
		virtual void						Scale(float scale);
		virtual Vector3Randomizer *	Clone(void) const	{ return W3DNEW Vector3HollowSphereRandomizer(*this); }

	protected:

		// Derived classes should have protected copy CTors so users use the Clone() function
		Vector3HollowSphereRandomizer(const Vector3HollowSphereRandomizer &src) : Radius(src.Radius) { }

	private:

		// Derived classes should have a private dummy assignment operator to block usage
		Vector3HollowSphereRandomizer & operator = (const Vector3HollowSphereRandomizer &that) { that; return *this; }

		float	Radius;
};


/*
** Vector3SolidCylinderRandomizer is a randomizer for generating points uniformly distributed
** inside a cylinder which is centered on the origin (set extent to 0 for a disk).
*/
class Vector3SolidCylinderRandomizer : public Vector3Randomizer {

	public:

		Vector3SolidCylinderRandomizer(float extent, float radius);

		virtual unsigned int				Class_ID (void) const { return CLASSID_SOLIDCYLINDER; }
		virtual float						Get_Radius (void) const { return Radius; }
		virtual float						Get_Height (void) const { return Extent; }
		virtual void						Get_Vector(Vector3 &vector);
		virtual float						Get_Maximum_Extent(void);
		virtual void						Scale(float scale);
		virtual Vector3Randomizer *	Clone(void) const	{ return W3DNEW Vector3SolidCylinderRandomizer(*this); }

	protected:

		// Derived classes should have protected copy CTors so users use the Clone() function
		Vector3SolidCylinderRandomizer(const Vector3SolidCylinderRandomizer &src) : Extent(src.Extent), Radius(src.Radius) { }

	private:

		// Derived classes should have a private dummy assignment operator to block usage
		Vector3SolidCylinderRandomizer & operator = (const Vector3SolidCylinderRandomizer &that) { that; return *this; }

		float	Extent;
		float	Radius;
};


#endif


