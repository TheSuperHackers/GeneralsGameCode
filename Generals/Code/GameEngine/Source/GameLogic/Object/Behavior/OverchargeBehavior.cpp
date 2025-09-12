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

// FILE: OverchargeBehavior.cpp ///////////////////////////////////////////////////////////////////
// Author: Colin Day, June 2002
// Desc:   Objects with this behavior module will get the ability to produce more power
//				 for a short amount of time, during this "overcharge" state object health is
//				 slowly reduced
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/Radar.h"
#include "Common/Xfer.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/OverchargeBehavior.h"
#include "GameLogic/Module/PowerPlantUpdate.h"
#include "GameClient/InGameUI.h"



//-------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
OverchargeBehaviorModuleData::OverchargeBehaviorModuleData( void )
{

	m_healthPercentToDrainPerSecond = 0.0f;
	m_notAllowedWhenHealthBelowPercent = 0.0f;

}

//-------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/*static*/ void OverchargeBehaviorModuleData::buildFieldParse( MultiIniFieldParse &p )
{

  UpdateModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] =
	{
		{ "HealthPercentToDrainPerSecond", INI::parsePercentToReal,	NULL, offsetof( OverchargeBehaviorModuleData, m_healthPercentToDrainPerSecond ) },
		{ "NotAllowedWhenHealthBelowPercent", INI::parsePercentToReal, NULL, offsetof( OverchargeBehaviorModuleData, m_notAllowedWhenHealthBelowPercent ) },
		{ 0, 0, 0, 0 }
	};

  p.add( dataFieldParse );

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
OverchargeBehavior::OverchargeBehavior( Thing *thing, const ModuleData* moduleData )
									 : UpdateModule( thing, moduleData )
{

	m_overchargeActive = FALSE;

	// start off sleeping forever until we become active
	setWakeFrame( getObject(), UPDATE_SLEEP_FOREVER );

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
OverchargeBehavior::~OverchargeBehavior( void )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime OverchargeBehavior::update( void )
{

	// if the overcharge is active we need to take away some life
	if( m_overchargeActive )
	{
		Object *us = getObject();

		// get mod data
		const OverchargeBehaviorModuleData *modData = getOverchargeBehaviorModuleData();

		// do some damage
		BodyModuleInterface *body = us->getBodyModule();
		DamageInfo damageInfo;
		damageInfo.in.m_amount = (body->getMaxHealth() * modData->m_healthPercentToDrainPerSecond) / LOGICFRAMES_PER_SECOND;
		damageInfo.in.m_sourceID = us->getID();
		damageInfo.in.m_damageType = DAMAGE_PENALTY;
		damageInfo.in.m_deathType = DEATH_NORMAL;
		us->attemptDamage( &damageInfo );

		// see if our health is below the allowable threshold
		if( body->getHealth() < body->getMaxHealth() * modData->m_notAllowedWhenHealthBelowPercent )
		{

			// turn off the overcharge
			enable( FALSE );

			// do some UI info for the local user if this is theirs
			if( ThePlayerList->getLocalPlayer() == us->getControllingPlayer() )
			{

				// print msg
				TheInGameUI->message( "GUI:OverchargeExhausted" );

				// do radar event
				TheRadar->createEvent( us->getPosition(), RADAR_EVENT_INFORMATION );

			}

			// do nothing else
			return UPDATE_SLEEP_NONE;

		}

	}

	return UPDATE_SLEEP_NONE;

}

// ------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void OverchargeBehavior::onDamage( DamageInfo *damageInfo )
{

}

// ------------------------------------------------------------------------------------------------
/** Flip the state of our 'overcharge-ness' */
// ------------------------------------------------------------------------------------------------
void OverchargeBehavior::toggle( void )
{

	// just toggle using enable()
	enable( !m_overchargeActive );

}

// ------------------------------------------------------------------------------------------------
/** Enable or disable an overcharge */
// ------------------------------------------------------------------------------------------------
void OverchargeBehavior::enable( Bool enable )
{
	Object *us = getObject();

	if( enable == FALSE )
	{

		// if we're turned on, turn off
		if( m_overchargeActive == TRUE )
		{

			// make sure to NOT extend rods for purpose of maintaining proper model condition
			PowerPlantUpdateInterface *ppui;
			for( BehaviorModule **umi = getObject()->getBehaviorModules(); *umi; ++umi)
			{
				ppui = (*umi)->getPowerPlantUpdateInterface();
				if( ppui )
					ppui->extendRods(FALSE);
			}

			Player *player = us->getControllingPlayer();
			if ( player )
				player->removePowerBonus(us);

			// we are no longer active
			m_overchargeActive = FALSE;

			// sleep forever
			setWakeFrame( us, UPDATE_SLEEP_FOREVER );

		}

	}
	else
	{

		// if we're turned off, turn on
		if( m_overchargeActive == FALSE )
		{

			// make sure to extend rods for purpose of maintaining proper model condition
			PowerPlantUpdateInterface *ppui;
			for( BehaviorModule **umi = getObject()->getBehaviorModules(); *umi; ++umi)
			{
				ppui = (*umi)->getPowerPlantUpdateInterface();
				if( ppui )
					ppui->extendRods(TRUE);
			}

			// add the power bonus
			Player *player = us->getControllingPlayer();
			if ( player )
				player->addPowerBonus(us);

			// we are now active
			m_overchargeActive = TRUE;

			// need to update every frame now
			setWakeFrame( us, UPDATE_SLEEP_NONE );

		}

	}

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void OverchargeBehavior::onDelete( void )
{

	// if we haven't been upgraded there is nothing to clean up
	if( m_overchargeActive == FALSE )
		return;

	// remove the power bonus from the player
	Player *player = getObject()->getControllingPlayer();
	if( player )
		player->removePowerBonus( getObject() );

	m_overchargeActive = FALSE;

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void OverchargeBehavior::onCapture( Player *oldOwner, Player *newOwner )
{

	// do nothing if we haven't upgraded yet
	if( m_overchargeActive == FALSE )
		return;

	if (getObject()->isDisabled())
		return;

	// remove power bonus from old owner
	if( oldOwner )
		oldOwner->removePowerBonus( getObject() );

	// add power bonus to the new owner
	if( newOwner )
		newOwner->addPowerBonus( getObject() );

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void OverchargeBehavior::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void OverchargeBehavior::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// overcharge active
	xfer->xferBool( &m_overchargeActive );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void OverchargeBehavior::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

	// Our effect is a fire and forget effect, not an upgrade state that is itself saved, so need to re-fire.
	if( m_overchargeActive && getObject()->getControllingPlayer() )
		getObject()->getControllingPlayer()->addPowerBonus( getObject() );

}
