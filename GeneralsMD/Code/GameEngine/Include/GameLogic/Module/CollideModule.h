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

// FILE: CollideModule.h /////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, September 2001
// Desc:
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common/Module.h"
#include "GameLogic/Module/BehaviorModule.h"

//-------------------------------------------------------------------------------------------------
/** OBJECT COLLIDE MODULE
	- Called when two objects collide (or when object collides with ground)
	- Note in the 'collide' method that 'other' can be null, this indicates a
		collision with the ground
	- Also note the 'collide' method is the response for the object that THIS module
		belongs to, we do not need to worry about the collision module of 'other',
		it will have its own collide action called separately */
//-------------------------------------------------------------------------------------------------
class CollideModuleInterface
{
public:
	virtual void onCollide( Object *other, const Coord3D *loc, const Coord3D *normal ) = 0;
	virtual Bool wouldLikeToCollideWith(const Object* other) const = 0;
	virtual Bool isHijackedVehicleCrateCollide() const = 0;
	virtual Bool isSabotageBuildingCrateCollide() const = 0;
	virtual Bool isCarBombCrateCollide() const = 0;
	virtual Bool isRailroad() const = 0;
	virtual Bool isSalvageCrateCollide() const = 0;

};

//-------------------------------------------------------------------------------------------------
class CollideModuleData : public BehaviorModuleData
{
public:

	static void buildFieldParse(MultiIniFieldParse& p)
	{
		BehaviorModuleData::buildFieldParse(p);
	}
};


//-------------------------------------------------------------------------------------------------
class CollideModule : public BehaviorModule,
											public CollideModuleInterface
{

	MEMORY_POOL_GLUE_ABC( CollideModule )
	MAKE_STANDARD_MODULE_MACRO_ABC( CollideModule )
	MAKE_STANDARD_MODULE_DATA_MACRO_ABC(CollideModule, CollideModuleData)

public:

	CollideModule( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype defined by MemoryPoolObject

	static Int getInterfaceMask() { return MODULEINTERFACE_COLLIDE; }

	// BehaviorModule
	virtual CollideModuleInterface* getCollide() { return this; }

	virtual void onCollide( Object *other, const Coord3D *loc, const Coord3D *normal ) = 0;

	/// this is used for things like pilots, to determine if they can "enter" something
	virtual Bool wouldLikeToCollideWith(const Object* other) const { return false; }
	virtual Bool isHijackedVehicleCrateCollide() const { return false; }
	virtual Bool isSabotageBuildingCrateCollide() const { return false; }
	virtual Bool isCarBombCrateCollide() const { return false; }
	virtual Bool isRailroad() const { return false;}
	virtual Bool isSalvageCrateCollide() const { return false; }

};
inline CollideModule::CollideModule( Thing *thing, const ModuleData* moduleData ) : BehaviorModule( thing, moduleData ) { }
inline CollideModule::~CollideModule() { }
//-------------------------------------------------------------------------------------------------
