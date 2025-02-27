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

// FILE: AutoFindHealingUpdate.cpp //////////////////////////////////////////////////////////////////////////
// Author: Kris Morness, August 2002
// Desc:   Update module to handle independent targeting of hazards to cleanup.
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#define DEFINE_WEAPONSLOTTYPE_NAMES

#include "Common/RandomValue.h"
#include "Common/ThingTemplate.h"
#include "Common/Player.h"
#include "Common/Xfer.h"

#include "GameClient/Drawable.h"

#include "GameLogic/GameLogic.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Object.h"
#include "GameLogic/ObjectIter.h"
#include "GameLogic/Module/AutoFindHealingUpdate.h"
#include "GameLogic/Module/PhysicsUpdate.h"
#include "GameLogic/Weapon.h"
#include "GameLogic/WeaponSet.h"
#include "GameLogic/Module/AIUpdate.h"


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
AutoFindHealingUpdateModuleData::AutoFindHealingUpdateModuleData()
{
	m_scanFrames				= 0;
	m_scanRange					= 0.0f;
	m_neverHeal					= 0.95f;
	m_alwaysHeal				= 0.25f;
}

//-------------------------------------------------------------------------------------------------
/*static*/ void AutoFindHealingUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	ModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "ScanRate",							INI::parseDurationUnsignedInt,	NULL, offsetof( AutoFindHealingUpdateModuleData, m_scanFrames ) },
		{ "ScanRange",						INI::parseReal,									NULL, offsetof( AutoFindHealingUpdateModuleData, m_scanRange ) },
		{ "NeverHeal",						INI::parseReal,									NULL, offsetof( AutoFindHealingUpdateModuleData, m_neverHeal ) },
		{ "AlwaysHeal",						INI::parseReal,									NULL, offsetof( AutoFindHealingUpdateModuleData, m_alwaysHeal ) },
		{ 0, 0, 0, 0 }
	};
	p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
AutoFindHealingUpdate::AutoFindHealingUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	m_nextScanFrames						= 0;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
AutoFindHealingUpdate::~AutoFindHealingUpdate( void )
{

}


//-------------------------------------------------------------------------------------------------
void AutoFindHealingUpdate::onObjectCreated()
{

}

//-------------------------------------------------------------------------------------------------
/** The update callback. */
//-------------------------------------------------------------------------------------------------
UpdateSleepTime AutoFindHealingUpdate::update()
{
/// @todo srj use SLEEPY_UPDATE here
	Object *obj = getObject();
	if (obj->getControllingPlayer()->getPlayerType() == PLAYER_HUMAN) {
		return UPDATE_SLEEP_NONE;
	}
	const AutoFindHealingUpdateModuleData *data = getAutoFindHealingUpdateModuleData();

	//Optimized firing at acquired target
	if( m_nextScanFrames > 0 )
	{
		m_nextScanFrames--;
		return UPDATE_SLEEP_NONE;
	}
	m_nextScanFrames = data->m_scanFrames;

	AIUpdateInterface *ai = obj->getAI();
	if (ai==NULL) return UPDATE_SLEEP_NONE;

	// Check health.
	BodyModuleInterface *body = obj->getBodyModule();
	if (!body) return UPDATE_SLEEP_NONE;
	//	If we're real healthy, don't bother looking for healing.
	if (body->getHealth() > body->getMaxHealth()*data->m_neverHeal) {
		return UPDATE_SLEEP_NONE;
	}

	if(  !ai->isIdle() )
	{
		// For now, only heal if idle.  jba.
		return UPDATE_SLEEP_NONE;
		//	If we're > min health, and busy, keep at it.
		if (body->getHealth() > body->getMaxHealth()*data->m_alwaysHeal) {
			return UPDATE_SLEEP_NONE;
		}
	}

	//Periodic scanning (expensive)
	Object *healUnit = scanClosestTarget();
	if(healUnit)
	{
		ai->aiGetHealed(healUnit, CMD_FROM_AI);
	}
	return UPDATE_SLEEP_NONE;
}


//-------------------------------------------------------------------------------------------------
Object* AutoFindHealingUpdate::scanClosestTarget()
{
	const AutoFindHealingUpdateModuleData *data = getAutoFindHealingUpdateModuleData();
	Object *me = getObject();
	Object *bestTarget = NULL;
	Real closestDistSqr=0;

	ObjectIterator *iter = ThePartitionManager->iterateObjectsInRange( me->getPosition(), data->m_scanRange, FROM_CENTER_2D );
	MemoryPoolObjectHolder hold(iter);

	for( Object *other = iter->first(); other; other = iter->next() )
	{
		if( !other->isKindOf( KINDOF_HEAL_PAD ) )
		{
			//Not a valid target.
			continue;
		}

		Real fDistSqr =  ThePartitionManager->getDistanceSquared( me, other, FROM_CENTER_2D ) ;
		if (bestTarget==NULL) {
			bestTarget = other;
			closestDistSqr = fDistSqr;
			continue;
		}

		if( fDistSqr < closestDistSqr )
		{
			bestTarget = other;
			closestDistSqr = fDistSqr;
			continue;
		}
	}

	return bestTarget;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void AutoFindHealingUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void AutoFindHealingUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// next scan frames
	xfer->xferInt( &m_nextScanFrames );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void AutoFindHealingUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}
