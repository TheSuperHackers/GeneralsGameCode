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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: Thing.h //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information					         
//                Copyright (C) 2001 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
// Project:    RTS3
//
// File name:  Thing.h
//
// Created:    Colin Day, May 2001
//
// Desc:       Things are the base class for objects and drawables, objects
//						 are logic side representations while drawables are client
//						 side.  Common data will be held in the Thing defined here
//						 and systems that need to work with both of them will work with
//						 "Things"
//
//-----------------------------------------------------------------------------

#pragma once

#ifndef __THING_H_
#define __THING_H_

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//           Includes                                                      
//-----------------------------------------------------------------------------
#include "Common/GameMemory.h"
#include "Common/KindOf.h"
#include "Common/Override.h"
#include "WWMath/matrix3d.h"							///< @todo Decide if we're keeping the WWMath libs (MSB)

//-----------------------------------------------------------------------------
//           Forward References
//-----------------------------------------------------------------------------

class Object;
class AIObject;
class Drawable;
class Team;
class ThingTemplate;

//-----------------------------------------------------------------------------
//           Type Defines
//-----------------------------------------------------------------------------

//=====================================
// class Thing
//=====================================
/** A thing is the common base class for objects and drawables.  It will
	* hold common information to both and systems that need to work with both
	* objects and drawables should work with things instead.  You can not
	* instantiate things, they are purely virtual */
//=====================================
class Thing : public MemoryPoolObject
{
	// note, it is explicitly OK to pass null for 'thing' here;
	// they will check for null and return null in these cases.
	friend inline Object *AsObject(Thing *thing) { return thing ? thing->asObjectMeth() : NULL; }
	friend inline Drawable *AsDrawable(Thing *thing) { return thing ? thing->asDrawableMeth() : NULL; }
	friend inline const Object *AsObject(const Thing *thing) { return thing ? thing->asObjectMeth() : NULL; }
	friend inline const Drawable *AsDrawable(const Thing *thing) { return thing ? thing->asDrawableMeth() : NULL; }

	MEMORY_POOL_GLUE_ABC(Thing)

public:

	Thing( const ThingTemplate *thingTemplate );

	/** 
		return the thing template for this thing.
	*/
	const ThingTemplate *getTemplate() const;

	// convenience method for patching isKindOf thru to template.
	Bool isKindOf(KindOfType t) const;
	Bool isKindOfMulti(const KindOfMaskType& mustBeSet, const KindOfMaskType& mustBeClear) const;
	Bool isAnyKindOf(const KindOfMaskType& anyKindOf) const;

	// physical properties
	void setPosition( const Coord3D *pos );

	/// the nice thing about this is that we don't have to recalc out cached terrain stuff.
	void setPositionZ( Real z );

	// note that calling this always orients the object straight up on the Z-axis,
	// or aligned with the terrain if we have the magic flag set.
	// don't want this behavior? then call setTransformMatrix instead.
	void setOrientation( Real angle );

	inline const Coord3D *getPosition() const { return &m_cachedPos; }
	inline Real getOrientation() const { return m_cachedAngle; }
	const Coord3D *getUnitDirectionVector2D() const;
	void getUnitDirectionVector2D(Coord3D& dir) const;
	void getUnitDirectionVector3D(Coord3D& dir) const;

	Real getHeightAboveTerrain() const;
	Real getHeightAboveTerrainOrWater() const;

	Bool isAboveTerrain() const { return getHeightAboveTerrain() > 0.0f; }
	Bool isAboveTerrainOrWater() const { return getHeightAboveTerrainOrWater() > 0.0f; }

 	/** Ground vehicles moving down a slope will get slightly above the terrain.
	If we treat this as airborne, then they slide down slopes.  This checks whether
	they are high enough that we should let them act like they're flying. jba. */
	Bool isSignificantlyAboveTerrain() const ;

	void convertBonePosToWorldPos(const Coord3D* bonePos, const Matrix3D* boneTransform, Coord3D* worldPos, Matrix3D* worldTransform) const;

	void setTransformMatrix( const Matrix3D *mx );												///< set the world transformation matrix
	const Matrix3D* getTransformMatrix() const { return &m_transform; }		///< return the world transformation matrix

	void transformPoint( const Coord3D *in, Coord3D *out );								///< transform this point using the m_transform matrix of this thing

protected:

	// Virtual method since objects can be on bridges and need to calculate heigh above terrain differently.
	virtual Real calculateHeightAboveTerrain(void) const;		// Calculates the actual height above terrain.  Doesn't use cache.

	virtual Object *asObjectMeth() { return NULL; }
	virtual Drawable *asDrawableMeth() { return NULL; }
	virtual const Object *asObjectMeth() const { return NULL; }
	virtual const Drawable *asDrawableMeth() const { return NULL; }

	virtual void reactToTransformChange(const Matrix3D* oldMtx, const Coord3D* oldPos, Real oldAngle) = 0;

private:

	// note that it is declared 'const' -- the assumption being that
	// since ThingTemplates are shared between many, many Things, the Thing
	// should never be able to change it.
	OVERRIDE<ThingTemplate> m_template;	///< reference back to template database
#if defined(_DEBUG) || defined(_INTERNAL)
	AsciiString m_templateName;
#endif
	/*
		yes, private; it's important that some of these only be modified
		by going thru the right methods, thus we make 'em private to enforce this.

		note that "m_transform" is the true description of location; the other fields
		(pos, angle, etc) are all simply cached values used for efficiency and convenience.
		you should NEVER modify them directly, because that won't change anything!
	*/
	Matrix3D m_transform;									///< the 3D orientation and position of this Thing

	enum
	{
		VALID_DIRVECTOR = 0x01,
		VALID_ALTITUDE_TERRAIN = 0x02,
		VALID_ALTITUDE_SEALEVEL = 0x04
	};

	mutable Coord3D		m_cachedPos;												///< position of thing
	mutable Real			m_cachedAngle;											///< orientation of thing
	mutable Coord3D		m_cachedDirVector;									///< unit direction vector
	mutable Real			m_cachedAltitudeAboveTerrain;
	mutable Real			m_cachedAltitudeAboveTerrainOrWater;
	mutable Int				m_cacheFlags;

}; 


//-----------------------------------------------------------------------------
//           Externals                                                     
//-----------------------------------------------------------------------------

#endif // $label

