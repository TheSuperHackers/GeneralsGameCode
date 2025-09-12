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

// FILE: HealContain.cpp //////////////////////////////////////////////////////////////////////////
// Author: Colin Day
// Desc:   Objects that are contained inside a heal contain ... get healed!  oh my!
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/Xfer.h"
#include "GameLogic/Damage.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/HealContain.h"
#include "GameLogic/Module/UpdateModule.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
HealContainModuleData::HealContainModuleData( void )
{

	m_framesForFullHeal = 0;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/*static*/ void HealContainModuleData::buildFieldParse(MultiIniFieldParse& p)
{

  OpenContainModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] =
	{
		{ "TimeForFullHeal", INI::parseDurationUnsignedInt, NULL, offsetof( HealContainModuleData, m_framesForFullHeal ) },
		{ 0, 0, 0, 0 }
	};

  p.add(dataFieldParse);

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
HealContain::HealContain( Thing *thing, const ModuleData *moduleData )
					 : OpenContain( thing, moduleData )
{

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
HealContain::~HealContain( void )
{

}

// ------------------------------------------------------------------------------------------------
/** Per frame update */
// ------------------------------------------------------------------------------------------------
UpdateSleepTime HealContain::update( void )
{

	// extending functionality
	/*UpdateSleepTime result =*/ OpenContain::update();

	// get the module data
	const HealContainModuleData *modData = getHealContainModuleData();

	//
	// for each of our objects, we give them a little health each frame so that when the
	// the TimeTillHealed is up, the object will exit and be fully healed
	//
	Bool doneHealing;
	Object *obj;
	ContainedItemsList::const_iterator it = getContainList().begin();
	while( it != getContainList().end() )
	{

		// get the object
		obj = *it;

		// increment the iterator, which allows failure to not cause an infinite loop
		++it;

		// do the healing on this object
		doneHealing = doHeal( obj, modData->m_framesForFullHeal );

		// if we're done healing, we need to remove us from the healing container
		if( doneHealing == TRUE )
		{
			ExitDoorType exitDoor = reserveDoorForExit(obj->getTemplate(), obj);
			if (exitDoor != DOOR_NONE_AVAILABLE)
				exitObjectViaDoor( obj, exitDoor );
		}

	}

	return UPDATE_SLEEP_NONE;

}

// ------------------------------------------------------------------------------------------------
/** Do the healing for a single object for a single frame. */
// ------------------------------------------------------------------------------------------------
Bool HealContain::doHeal( Object *obj, UnsignedInt framesForFullHeal )
{
	Bool doneHealing = FALSE;

	// setup the healing damageInfo structure with all but the amount
	DamageInfo healInfo;
	healInfo.in.m_damageType = DAMAGE_HEALING;
	healInfo.in.m_deathType = DEATH_NONE;
	healInfo.in.m_sourceID = getObject()->getID();

	// get body module of the thing to heal
	BodyModuleInterface *body = obj->getBodyModule();

	// if we've been in here long enough ... set our health to max
	if( TheGameLogic->getFrame() - obj->getContainedByFrame() >= framesForFullHeal )
	{

		// set the amount to max just to be sure we're at the top
		healInfo.in.m_amount = body->getMaxHealth();

		// set max health
		body->attemptHealing( &healInfo );

		// we're done healing
		doneHealing = TRUE;

	}
	else
	{

		//
		// given the *whole* time it would take to heal this object, lets pretend that the
		// object is at zero health ... and give it a sliver of health as if it were at 0 health
		// and would be fully healed at 'framesForFullHeal'
		//
		healInfo.in.m_amount = body->getMaxHealth() / (Real)framesForFullHeal;

		// do the healing
		body->attemptHealing( &healInfo );

	}

	// return if we're done healing
	return doneHealing;

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void HealContain::crc( Xfer *xfer )
{

	// extend base class
	OpenContain::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void HealContain::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	OpenContain::xfer( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void HealContain::loadPostProcess( void )
{

	// extend base class
	OpenContain::loadPostProcess();

}
