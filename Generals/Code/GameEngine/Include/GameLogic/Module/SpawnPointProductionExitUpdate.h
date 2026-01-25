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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: SpawnPointProductionExitUpdate.h /////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, April, 2002
// Desc:		Hand off produced Units to me so I can Exit them into the world with my specific style
//					This instance puts guys at named bones.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameLogic/Module/UpdateModule.h"
#include "Common/INI.h"
#include "Lib/BaseType.h"

class Object;

enum
{
	MAX_SPAWN_POINTS = 10	///< Size the array that holds the bone positions
};

//-------------------------------------------------------------------------------------------------
class SpawnPointProductionExitUpdateModuleData : public UpdateModuleData
{
public:
	AsciiString m_spawnPointBoneNameData;

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    UpdateModuleData::buildFieldParse(p);
		static const FieldParse dataFieldParse[] =
		{
			{ "SpawnPointBoneName",		INI::parseAsciiString,		nullptr, offsetof( SpawnPointProductionExitUpdateModuleData, m_spawnPointBoneNameData ) },
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);
	}
};

//-------------------------------------------------------------------------------------------------
class SpawnPointProductionExitUpdate : public UpdateModule, public ExitInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( SpawnPointProductionExitUpdate, "SpawnPointProductionExitUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( SpawnPointProductionExitUpdate, SpawnPointProductionExitUpdateModuleData )

public:

	virtual ExitInterface* getUpdateExitInterface() { return this; }

	SpawnPointProductionExitUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	// Required funcs to fufill interface requirements
	virtual Bool isExitBusy() const {return FALSE;}	///< Contain style exiters are getting the ability to space out exits, so ask this before reserveDoor as a kind of no-commitment check.
	virtual ExitDoorType reserveDoorForExit( const ThingTemplate* objType, Object *specificObject );
	virtual void exitObjectViaDoor( Object *newObj, ExitDoorType exitDoor );
	virtual void unreserveDoorForExit( ExitDoorType exitDoor );
	virtual void setRallyPoint( const Coord3D * ){}
	virtual const Coord3D *getRallyPoint() const { return nullptr; }
	virtual void exitObjectByBudding( Object *newObj, Object *budHost ) { return; }

	virtual UpdateSleepTime update()										{ return UPDATE_SLEEP_FOREVER; }

protected:
	Bool m_bonesInitialized;													///< To prevent creation bugs, only init the World coords when first asked for one
	Int m_spawnPointCount;														///< How many in the array are actually live and valid
	Coord3D m_worldCoordSpawnPoints[MAX_SPAWN_POINTS];///< Where my little friends will be created
	Real m_worldAngleSpawnPoints[MAX_SPAWN_POINTS];		///< And what direction they should face
	ObjectID m_spawnPointOccupier[MAX_SPAWN_POINTS];	///< Who I think is in each spot.  I can validate their existence to see if I am free to exit something.

	// Required func to fufill Module requirement

private:
	void initializeBonePositions();	///< Look up the bone positions and store them in world space coords
	void revalidateOccupiers();			///< Do a lookup on all our ID's and clear the dead ones.
};
