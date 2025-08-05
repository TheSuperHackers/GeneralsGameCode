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

// FILE: BaseRegenerateUpdate.cpp /////////////////////////////////////////////////////////////////
// Author: Colin Day, July 2002
// Desc:   Update module for base objects automatically regenerating health
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/GlobalData.h"
#include "Common/Xfer.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/BaseRegenerateUpdate.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Object.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
BaseRegenerateUpdateModuleData::BaseRegenerateUpdateModuleData()
{

}

//-------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void BaseRegenerateUpdateModuleData::buildFieldParse( MultiIniFieldParse &p )
{
  UpdateModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] =
	{
		{ 0, 0, 0, 0 }
	};

  p.add( dataFieldParse );

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
BaseRegenerateUpdate::BaseRegenerateUpdate( Thing *thing, const ModuleData* moduleData )
										: UpdateModule( thing, moduleData )
{

	if( TheGlobalData->m_baseRegenHealthPercentPerSecond == 0.0f )
	{
		setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);
	}
	else
	{
		setWakeFrame(getObject(), UPDATE_SLEEP_NONE);
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
BaseRegenerateUpdate::~BaseRegenerateUpdate( void )
{
}

//-------------------------------------------------------------------------------------------------
/** Damage has been dealt, this is an opportunity to reach to that damage */
//-------------------------------------------------------------------------------------------------
void BaseRegenerateUpdate::onDamage( DamageInfo *damageInfo )
{
	if (TheGlobalData->m_baseRegenHealthPercentPerSecond > 0.0 &&
			damageInfo->in.m_damageType != DAMAGE_HEALING)
	{
		setWakeFrame(getObject(), UPDATE_SLEEP(TheGlobalData->m_baseRegenDelay));
	}
	else
	{
		setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);
	}
}

//-------------------------------------------------------------------------------------------------
/** The update callback, this is pretty similar to AutoHealUpdate, but that module is supposed
	* to be used in concert with an upgrade and doesn't have any of the "only regenerate
	* if we haven't been damaged recently" restrictions */
//-------------------------------------------------------------------------------------------------
UpdateSleepTime BaseRegenerateUpdate::update( void )
{
	// this is us!
	Object *me = getObject();

	if( me->testStatus(OBJECT_STATUS_UNDER_CONSTRUCTION) )
	{
		return UPDATE_SLEEP_NONE;
	}

	if( me->testStatus(OBJECT_STATUS_SOLD) )
	{
		return UPDATE_SLEEP_FOREVER;
	}

	// do not heal if we are at max health already
	BodyModuleInterface *body = me->getBodyModule();
	if( body->getMaxHealth() == body->getHealth() )
	{
		return UPDATE_SLEEP_FOREVER;	// sleep till we get damaged again
	}
	else
	{
		// we don't really need to heal every frame. do it every 3 frames or so.
		const Int HEAL_RATE = 3;

		// do some healing
		Real amount = HEAL_RATE * (body->getMaxHealth() * TheGlobalData->m_baseRegenHealthPercentPerSecond) /
														 LOGICFRAMES_PER_SECOND;
		me->attemptHealing(amount, me);

		return UPDATE_SLEEP(HEAL_RATE);
	}
}  // end update

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void BaseRegenerateUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void BaseRegenerateUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void BaseRegenerateUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
