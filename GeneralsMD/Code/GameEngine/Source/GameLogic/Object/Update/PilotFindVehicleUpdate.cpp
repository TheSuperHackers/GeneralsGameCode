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

// FILE: PilotFindVehicleUpdate.cpp //////////////////////////////////////////////////////////////////////////
// Author: Kris Morness, August 2002
// Desc:   Update module to handle independent targeting of hazards to cleanup.
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#define DEFINE_WEAPONSLOTTYPE_NAMES

#include "GameClient/Drawable.h"

#include "Common/ActionManager.h"
#include "Common/Player.h"
#include "Common/RandomValue.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"

#include "GameLogic/GameLogic.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Object.h"
#include "GameLogic/ObjectIter.h"
#include "GameLogic/Module/PilotFindVehicleUpdate.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/CollideModule.h"



//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
PilotFindVehicleUpdateModuleData::PilotFindVehicleUpdateModuleData()
{
	m_scanFrames				= 0;
	m_scanRange					= 0.0f;
	m_minHealth					= 0.5f;
}

//-------------------------------------------------------------------------------------------------
/*static*/ void PilotFindVehicleUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	ModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "ScanRate",							INI::parseDurationUnsignedInt,	NULL, offsetof( PilotFindVehicleUpdateModuleData, m_scanFrames ) },
		{ "ScanRange",						INI::parseReal,									NULL, offsetof( PilotFindVehicleUpdateModuleData, m_scanRange ) },
		{ "MinHealth",						INI::parseReal,									NULL, offsetof( PilotFindVehicleUpdateModuleData, m_minHealth ) },
		{ 0, 0, 0, 0 }
	};
	p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
PilotFindVehicleUpdate::PilotFindVehicleUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	m_didMoveToBase = false;
	setWakeFrame(getObject(), UPDATE_SLEEP_NONE);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
PilotFindVehicleUpdate::~PilotFindVehicleUpdate( void )
{

}


//-------------------------------------------------------------------------------------------------
void PilotFindVehicleUpdate::onObjectCreated()
{

}

//-------------------------------------------------------------------------------------------------
/** The update callback. */
//-------------------------------------------------------------------------------------------------
UpdateSleepTime PilotFindVehicleUpdate::update()
{

	Object *obj = getObject();
	if (obj->getControllingPlayer()->getPlayerType() == PLAYER_HUMAN) {
		// This is an ai only behavior.
		return UPDATE_SLEEP_FOREVER;
	}
	const PilotFindVehicleUpdateModuleData *data = getPilotFindVehicleUpdateModuleData();

	AIUpdateInterface *ai = obj->getAI();
	if (ai==NULL) return UPDATE_SLEEP_FOREVER;

	if(  !ai->isIdle() )
	{
		return UPDATE_SLEEP(data->m_scanFrames);
	}

	//Periodic scanning (expensive)
	Object *upgradeUnit = scanClosestTarget();
	if(upgradeUnit)
	{
		ai->aiEnter(upgradeUnit, CMD_FROM_AI);
		m_didMoveToBase = false;
	} else {
		Coord3D pos;
		// Try moving to base 1 time.
		if (!m_didMoveToBase && obj->getControllingPlayer()->getAiBaseCenter(&pos)) {
			ai->aiMoveToPosition(&pos, CMD_FROM_AI);
			m_didMoveToBase = true;
		}
	}
	return UPDATE_SLEEP(data->m_scanFrames);
}


//-------------------------------------------------------------------------------------------------
Object* PilotFindVehicleUpdate::scanClosestTarget()
{
	const PilotFindVehicleUpdateModuleData *data = getPilotFindVehicleUpdateModuleData();
	Object *me = getObject();

	PartitionFilterAcceptByKindOf kindFilter(MAKE_KINDOF_MASK(KINDOF_VEHICLE), KINDOFMASK_NONE);
	PartitionFilterAlive aliveFilter;
	PartitionFilterPlayer playerFilter(me->getControllingPlayer(), true);
	PartitionFilterSameMapStatus filterMapStatus(getObject());
	PartitionFilter *filters[5];
	filters[0] = &kindFilter;
	filters[1] = &aliveFilter;
	filters[2] = &playerFilter;
	filters[3] = &filterMapStatus;
	filters[4] = NULL;

	ObjectIterator *iter = ThePartitionManager->iterateObjectsInRange( me->getPosition(), data->m_scanRange,
		FROM_CENTER_2D, filters, ITER_SORTED_NEAR_TO_FAR );
	MemoryPoolObjectHolder hold(iter);

	for( Object *other = iter->first(); other; other = iter->next() )
	{
		// Check health.
		BodyModuleInterface *body = other->getBodyModule();
		if (!body) continue;
		//	If we're real healthy, don't bother looking for healing.
		if (body->getHealth() < body->getMaxHealth()*data->m_minHealth)
		{
			continue;
		}
		// first, see if we'd like to collide with 'other'
		for (BehaviorModule** m = me->getBehaviorModules(); *m; ++m)
		{
			CollideModuleInterface* collide = (*m)->getCollide();
			if (!collide)
				continue;

			if( collide->wouldLikeToCollideWith( other ) )
			{
				return other;
			}
		}
	}
	return NULL;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void PilotFindVehicleUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void PilotFindVehicleUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// did move to base
	xfer->xferBool( &m_didMoveToBase );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void PilotFindVehicleUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}
