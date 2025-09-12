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

// FILE: UndeadBody.cpp ////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, June 2003
// Desc:	 First death is intercepted and sets flags and setMaxHealth.  Second death is handled normally.
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/Xfer.h"
#include "GameLogic/Module/UndeadBody.h"

#include "GameLogic/Object.h"
#include "GameLogic/Module/SlowDeathBehavior.h"

// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
void UndeadBodyModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  ActiveBodyModuleData::buildFieldParse(p);
	static const FieldParse dataFieldParse[] =
	{
		{ "SecondLifeMaxHealth",			INI::parseReal,	NULL,		offsetof( UndeadBodyModuleData, m_secondLifeMaxHealth ) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UndeadBodyModuleData::UndeadBodyModuleData()
{
	m_secondLifeMaxHealth = 1;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UndeadBody::UndeadBody( Thing *thing, const ModuleData* moduleData )
						 : ActiveBody( thing, moduleData )
{
	m_isSecondLife = FALSE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UndeadBody::~UndeadBody( void )
{

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void UndeadBody::attemptDamage( DamageInfo *damageInfo )
{
	// If we are on our first life, see if this damage will kill us.  If it will, bind it to one hitpoint
	// remaining, then go ahead and take it.
	Bool shouldStartSecondLife = FALSE;

	if( damageInfo->in.m_damageType != DAMAGE_UNRESISTABLE
			&& !m_isSecondLife
			&& damageInfo->in.m_amount >= getHealth()
			&& IsHealthDamagingDamage(damageInfo->in.m_damageType)
			)
	{
		damageInfo->in.m_amount = min( damageInfo->in.m_amount, getHealth() - 1 );
		shouldStartSecondLife = TRUE;
	}

	ActiveBody::attemptDamage(damageInfo);

	// After we take it (which allows for damaging special effects), we will do our modifications to the body module
	if( shouldStartSecondLife )
		startSecondLife(damageInfo);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void UndeadBody::startSecondLife(DamageInfo *damageInfo)
{
	const UndeadBodyModuleData *data = getUndeadBodyModuleData();

	// Flag module as no longer intercepting damage
	m_isSecondLife = TRUE;

	// Modify ActiveBody's max health and initial health
	setMaxHealth(data->m_secondLifeMaxHealth, FULLY_HEAL);

	// Set Armor set flag to use second life armor
	setArmorSetFlag(ARMORSET_SECOND_LIFE);

	// Fire the Slow Death module.  The fact that this is not the result of an onDie will cause the special behavior
	Int total = 0;
	BehaviorModule** update = getObject()->getBehaviorModules();
	for( ; *update; ++update )
	{
		SlowDeathBehaviorInterface* sdu = (*update)->getSlowDeathBehaviorInterface();
		if (sdu != NULL  && sdu->isDieApplicable(damageInfo) )
		{
			total += sdu->getProbabilityModifier( damageInfo );
		}
	}
	DEBUG_ASSERTCRASH(total > 0, ("Hmm, this is wrong"));


	// this returns a value from 1...total, inclusive
	Int roll = GameLogicRandomValue(1, total);

	for( update = getObject()->getBehaviorModules(); *update; ++update)
	{
		SlowDeathBehaviorInterface* sdu = (*update)->getSlowDeathBehaviorInterface();
		if (sdu != NULL && sdu->isDieApplicable(damageInfo))
		{
			roll -= sdu->getProbabilityModifier( damageInfo );
			if (roll <= 0)
			{
				sdu->beginSlowDeath(damageInfo);
				return;
			}
		}
	}

}


// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void UndeadBody::crc( Xfer *xfer )
{

	// extend base class
	ActiveBody::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void UndeadBody::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	ActiveBody::xfer( xfer );

	xfer->xferBool(&m_isSecondLife);

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void UndeadBody::loadPostProcess( void )
{

	// extend base class
	ActiveBody::loadPostProcess();

}
