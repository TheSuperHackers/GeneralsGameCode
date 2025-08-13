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

// FILE: QueueProductionExitUpdate.h /////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, January, 2002
// Desc:		Hand off produced Units to me so I can Exit them into the world with my specific style
//					This instance refuses to spit a second out until the first is clear
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef _QUEUE_PRODUCTION_EXIT_UPDATE_H
#define _QUEUE_PRODUCTION_EXIT_UPDATE_H

#include "GameLogic/Module/UpdateModule.h"
#include "Common/INI.h"
#include "Lib/BaseType.h"

class Object;

//-------------------------------------------------------------------------------------------------
class QueueProductionExitUpdateModuleData : public UpdateModuleData
{
public:
	Coord3D m_unitCreatePoint;
	Coord3D m_naturalRallyPoint;
	UnsignedInt m_exitDelayData;
	Bool m_allowAirborneCreationData;
	UnsignedInt m_initialBurst;

	QueueProductionExitUpdateModuleData()
	{
		m_unitCreatePoint.zero();
		m_naturalRallyPoint.zero();
		m_exitDelayData = 0;
		m_allowAirborneCreationData = FALSE;
		m_initialBurst = 0;
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    UpdateModuleData::buildFieldParse(p);
		static const FieldParse dataFieldParse[] =
		{
			{ "UnitCreatePoint",				INI::parseCoord3D,							NULL, offsetof( QueueProductionExitUpdateModuleData, m_unitCreatePoint ) },
			{ "NaturalRallyPoint",			INI::parseCoord3D,							NULL, offsetof( QueueProductionExitUpdateModuleData, m_naturalRallyPoint ) },
			{ "ExitDelay",							INI::parseDurationUnsignedInt,	NULL, offsetof( QueueProductionExitUpdateModuleData, m_exitDelayData ) },
			{ "AllowAirborneCreation",	INI::parseBool,									NULL, offsetof( QueueProductionExitUpdateModuleData, m_allowAirborneCreationData ) },
			{ "InitialBurst",						INI::parseUnsignedInt,					NULL, offsetof( QueueProductionExitUpdateModuleData, m_initialBurst ) },
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);
	}
};

//-------------------------------------------------------------------------------------------------
class QueueProductionExitUpdate : public UpdateModule, public ExitInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( QueueProductionExitUpdate, "QueueProductionExitUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( QueueProductionExitUpdate, QueueProductionExitUpdateModuleData )

public:

	virtual ExitInterface* getUpdateExitInterface() { return this; }

	QueueProductionExitUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	// Required funcs to fufill interface requirements
	virtual Bool isExitBusy() const {return FALSE;}	///< Contain style exiters are getting the ability to space out exits, so ask this before reserveDoor as a kind of no-commitment check.
	virtual ExitDoorType reserveDoorForExit( const ThingTemplate* objType, Object *specificObject );
	virtual void exitObjectViaDoor( Object *newObj, ExitDoorType exitDoor );
	virtual void exitObjectByBudding( Object *newObj, Object *budHost );
	virtual void unreserveDoorForExit( ExitDoorType exitDoor );

	virtual void setRallyPoint( const Coord3D *pos );			///< define a "rally point" for units to move towards
	virtual const Coord3D *getRallyPoint( void ) const;			///< define a "rally point" for units to move towards
	virtual Bool getExitPosition( Coord3D& exitPosition ) const;					///< access to the "Door" position of the production object
	virtual Bool getNaturalRallyPoint( Coord3D& rallyPoint, Bool offset = TRUE ) const;			///< get the natural "rally point" for units to move towards

	virtual UpdateSleepTime update();

protected:

	UnsignedInt m_currentDelay;							///< When this hits zero, I can exit again
	Coord3D			m_rallyPoint;								///< Where units should move to after they have reached the "natural" rally point
	Bool				m_rallyPointExists;					///< Only move to the rally point if this is true
	Real				m_creationClearDistance;		///< I can think of myself as ready when the previous guy is this far away.
	UnsignedInt m_currentBurstCount;				///< how many times must I still override the delay timer

	Bool isFreeToExit() const;
};

inline void QueueProductionExitUpdate::setRallyPoint( const Coord3D *pos )
{
	m_rallyPoint = *pos;
	m_rallyPointExists = true;
}

inline const Coord3D *QueueProductionExitUpdate::getRallyPoint( void )  const
{
	if (m_rallyPointExists)
		return &m_rallyPoint;

	return NULL;
}

#endif
