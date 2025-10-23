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

// FILE: SupplyCenterProductionExitUpdate.h /////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, June, 2002
// Desc:		Hand off produced Units to me so I can Exit them into the world with my specific style
//					This instance kicks things it outputs into SupplyTruck autopilot after exiting.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GameLogic/Module/UpdateModule.h"

class Object;

//-------------------------------------------------------------------------------------------------
class SupplyCenterProductionExitUpdateModuleData : public UpdateModuleData
{
public:
	Coord3D m_unitCreatePoint;
	Coord3D m_naturalRallyPoint;

	SupplyCenterProductionExitUpdateModuleData()
	{
		m_unitCreatePoint.zero();
		m_naturalRallyPoint.zero();
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    UpdateModuleData::buildFieldParse(p);
		static const FieldParse dataFieldParse[] =
		{
			{ "UnitCreatePoint",		INI::parseCoord3D,		NULL, offsetof( SupplyCenterProductionExitUpdateModuleData, m_unitCreatePoint ) },
			{ "NaturalRallyPoint",  INI::parseCoord3D,		NULL, offsetof( SupplyCenterProductionExitUpdateModuleData, m_naturalRallyPoint ) },
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);
	}
};

//-------------------------------------------------------------------------------------------------
class SupplyCenterProductionExitUpdate : public UpdateModule, public ExitInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( SupplyCenterProductionExitUpdate, "SupplyCenterProductionExitUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( SupplyCenterProductionExitUpdate, SupplyCenterProductionExitUpdateModuleData )

public:

	virtual ExitInterface* getUpdateExitInterface() { return this; }

	SupplyCenterProductionExitUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	// Required funcs to fufill interface requirements
	virtual Bool isExitBusy() const {return FALSE;}	///< Contain style exiters are getting the ability to space out exits, so ask this before reserveDoor as a kind of no-commitment check.
	virtual ExitDoorType reserveDoorForExit( const ThingTemplate* objType, Object *specificObject ) { return DOOR_1; }
	virtual void exitObjectViaDoor( Object *newObj, ExitDoorType exitDoor );
	virtual void unreserveDoorForExit( ExitDoorType exitDoor ) { /* nothing */ }
	virtual void exitObjectByBudding( Object *newObj, Object *budHost ) { return; }

	virtual void setRallyPoint( const Coord3D *pos );			///< define a "rally point" for units to move towards
	virtual const Coord3D *getRallyPoint( void ) const;			///< define a "rally point" for units to move towards
	virtual Bool getExitPosition( Coord3D& exitPosition ) const;					///< access to the "Door" position of the production object
	virtual Bool getNaturalRallyPoint( Coord3D& rallyPoint, Bool offset = TRUE ) const;			///< get the natural "rally point" for units to move towards

	virtual UpdateSleepTime update()										{ return UPDATE_SLEEP_FOREVER; }

protected:

	Coord3D m_rallyPoint;						///< Where units should move to after they have reached the "natural" rally point
	Bool m_rallyPointExists;				///< Only move to the rally point if this is true

	// Required func to fufill Module requirement
};

inline void SupplyCenterProductionExitUpdate::setRallyPoint( const Coord3D *pos )
{
	m_rallyPoint = *pos;
	m_rallyPointExists = true;
}

inline const Coord3D *SupplyCenterProductionExitUpdate::getRallyPoint( void ) const
{
	if (m_rallyPointExists)
		return &m_rallyPoint;

	return NULL;
}
