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

// FILE: SupplyWarehouseCripplingBehavior.cpp /////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, Septemmber 2002
// Desc:   Behavior that Disables the building on ReallyDamaged edge state, and manages an Update timer to heal
///////////////////////////////////////////////////////////////////////////////////////////////////


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/Xfer.h"
#include "GameLogic/Module/SupplyWarehouseCripplingBehavior.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/BodyModule.h"

//-------------------------------------------------------------------------------------------------
SupplyWarehouseCripplingBehaviorModuleData::SupplyWarehouseCripplingBehaviorModuleData()
{
	m_selfHealSupression = 0; ///< Time since last damage until I can start to heal
	m_selfHealDelay = 0;			///< Once I am okay to heal, how often to do so
	m_selfHealAmount = 0;							///< And how much
}

//-------------------------------------------------------------------------------------------------
/*static*/ void SupplyWarehouseCripplingBehaviorModuleData::buildFieldParse(MultiIniFieldParse& p)
{

	static const FieldParse dataFieldParse[] =
	{
		{ "SelfHealSupression",	INI::parseDurationUnsignedInt,	NULL, offsetof(SupplyWarehouseCripplingBehaviorModuleData, m_selfHealSupression) },
		{ "SelfHealDelay",			INI::parseDurationUnsignedInt,	NULL, offsetof(SupplyWarehouseCripplingBehaviorModuleData, m_selfHealDelay) },
		{ "SelfHealAmount",			INI::parseReal,									NULL, offsetof(SupplyWarehouseCripplingBehaviorModuleData, m_selfHealAmount) },
		{ 0, 0, 0, 0 }
	};

  UpdateModuleData::buildFieldParse(p);
  p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SupplyWarehouseCripplingBehavior::SupplyWarehouseCripplingBehavior( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	m_healingSupressedUntilFrame = 0;
	m_nextHealingFrame = 0;
	setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
SupplyWarehouseCripplingBehavior::~SupplyWarehouseCripplingBehavior( void )
{
}

//-------------------------------------------------------------------------------------------------
/** Damage has been dealt, this is an opportunity to react to that damage */
//-------------------------------------------------------------------------------------------------
void SupplyWarehouseCripplingBehavior::onDamage( DamageInfo *damageInfo )
{
	UnsignedInt now = TheGameLogic->getFrame();
	resetSelfHealSupression();
	setWakeFrame(getObject(), UPDATE_SLEEP(m_healingSupressedUntilFrame - now));// we got hit, time to get up for work after a quick snooze
}

//-------------------------------------------------------------------------------------------------
void SupplyWarehouseCripplingBehavior::onBodyDamageStateChange(const DamageInfo* damageInfo, BodyDamageType oldState, BodyDamageType newState)
{
	if( newState == BODY_REALLYDAMAGED )
		startCrippledEffects();
	else if( oldState == BODY_REALLYDAMAGED )
		stopCrippledEffects();
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UpdateSleepTime SupplyWarehouseCripplingBehavior::update()
{
	// Supression is handled by sleeping the module, so if I am here, I know it is time to heal.
	const SupplyWarehouseCripplingBehaviorModuleData* md = getSupplyWarehouseCripplingBehaviorModuleData();
	UnsignedInt now = TheGameLogic->getFrame();
	m_nextHealingFrame = now + md->m_selfHealDelay;

	getObject()->attemptHealing(md->m_selfHealAmount, NULL);

	if( getObject()->getBodyModule()->getHealth() == getObject()->getBodyModule()->getMaxHealth() )
		return UPDATE_SLEEP_FOREVER;// this can't be in onHealing, as the healing comes from here
	// in the update, and sleep settings in onHealing would be overridden by my return value.

	// Delay between heals is also handled by sleeping the module.  How cool is that?
	return UPDATE_SLEEP(m_nextHealingFrame - now);
}

// ------------------------------------------------------------------------------------------------
void SupplyWarehouseCripplingBehavior::resetSelfHealSupression()
{
	const SupplyWarehouseCripplingBehaviorModuleData* md = getSupplyWarehouseCripplingBehaviorModuleData();
	UnsignedInt now = TheGameLogic->getFrame();

	m_healingSupressedUntilFrame = now + md->m_selfHealSupression;
	m_nextHealingFrame = m_healingSupressedUntilFrame;
}

// ------------------------------------------------------------------------------------------------
void SupplyWarehouseCripplingBehavior::startCrippledEffects()
{
	DockUpdateInterface *myDock = getObject()->getDockUpdateInterface();
	myDock->setDockCrippled( TRUE );
}

// ------------------------------------------------------------------------------------------------
void SupplyWarehouseCripplingBehavior::stopCrippledEffects()
{
	DockUpdateInterface *myDock = getObject()->getDockUpdateInterface();
	myDock->setDockCrippled( FALSE );
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void SupplyWarehouseCripplingBehavior::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void SupplyWarehouseCripplingBehavior::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// healing supressed until frame
	xfer->xferUnsignedInt( &m_healingSupressedUntilFrame );

	// next healing frame
	xfer->xferUnsignedInt( &m_nextHealingFrame );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void SupplyWarehouseCripplingBehavior::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
