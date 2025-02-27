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

// FILE: ProjectileStreamUpdate.h //////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, May 2002
// Desc:   Tracks all projectiles fired so they can be drawn as a stream
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpdateModule.h"
// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;
class Vector3;

enum
{
	MAX_PROJECTILE_STREAM = 20
};

//-------------------------------------------------------------------------------------------------
/** The default	update module */
//-------------------------------------------------------------------------------------------------
class ProjectileStreamUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( ProjectileStreamUpdate, "ProjectileStreamUpdate" )
	MAKE_STANDARD_MODULE_MACRO( ProjectileStreamUpdate );

public:

	ProjectileStreamUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	void addProjectile( ObjectID sourceID, ObjectID newID, ObjectID victimID, const Coord3D *victimPos );	///< This projectile was just shot, so keep track of it.
	void getAllPoints( Vector3 *points, Int *count );					///< unroll circlular array and write down all projectile positions
	void setPosition( const Coord3D *newPosition );						///< I need to exist at the place I want to draw since only (near) on screen Drawables get updated

	virtual UpdateSleepTime update();

protected:


	void cullFrontOfList();
	Bool considerDying();

	ObjectID m_projectileIDs[MAX_PROJECTILE_STREAM];
	Int m_nextFreeIndex;
	Int m_firstValidIndex;
	ObjectID m_owningObject;

	ObjectID m_targetObject;///< Need to insert a hole if target changes, so track target ID and target position
	Coord3D m_targetPosition;
};
