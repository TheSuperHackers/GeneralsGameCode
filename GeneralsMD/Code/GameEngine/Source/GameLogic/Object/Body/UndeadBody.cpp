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
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine
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
		{ "SecondLifeMaxHealth",			INI::parseReal,	nullptr,		offsetof( UndeadBodyModuleData, m_secondLifeMaxHealth ) },
		{ nullptr, nullptr, nullptr, 0 }
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
UndeadBody::~UndeadBody()
{

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void UndeadBody::attemptDamage( DamageInfo *damageInfo )
{
	// If we are on our first life, see if this damage will kill us.
	Bool shouldStartSecondLife = FALSE;

	if( damageInfo->in.m_damageType != DAMAGE_UNRESISTABLE
			&& !m_isSecondLife
#if RETAIL_COMPATIBLE_CRC || PRESERVE_PREMATURE_BATTLE_BUS_DEATH
			&& damageInfo->in.m_amount >= getHealth()
#else
			// TheSuperHackers @bugfix Stubbjax 20/09/2025 Battle Buses now correctly apply damage modifiers when calculating lethal damage
			&& estimateDamage(damageInfo->in) >= getHealth()
#endif
			&& IsHealthDamagingDamage(damageInfo->in.m_damageType)
			)
	{
		shouldStartSecondLife = TRUE;
	}

	// After we take it (which allows for damaging special effects), we will do our modifications to the body module
	if( shouldStartSecondLife )
	{
		if( !startSecondLife(damageInfo) )
		{
#if !RETAIL_COMPATIBLE_CRC
			damageInfo->in.m_kill = true;
			damageInfo->in.m_enterSecondLife = false;
			ActiveBody::attemptDamage(damageInfo);
#endif
		}
	}
	else
	{
		ActiveBody::attemptDamage(damageInfo);
	}
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Bool UndeadBody::startSecondLife(DamageInfo *damageInfo)
{
#if RETAIL_COMPATIBLE_CRC
	applySecondLife(damageInfo);
#endif

	damageInfo->in.m_enterSecondLife = TRUE;

	const Int total = SlowDeathBehavior::computeTotalSlowDeathProbability(getObject(), damageInfo);

#if !RETAIL_COMPATIBLE_CRC
	if (total == 0)
		return false;
#endif

	// this returns a value from 1...total, inclusive
	Int roll = GameLogicRandomValue(1, total);

	// Fire one of the Slow Death modules with Second Life at random.
	// The fact that this is not the result of an onDie will cause the special behavior.
	for (BehaviorModule** update = getObject()->getBehaviorModules(); *update; ++update)
	{
		SlowDeathBehaviorInterface* sdu = (*update)->getSlowDeathBehaviorInterface();
		if (sdu != nullptr && sdu->isDieApplicable(damageInfo))
		{
			roll -= sdu->getProbabilityModifier( damageInfo );
			if (roll <= 0)
			{
#if !RETAIL_COMPATIBLE_CRC
				applySecondLife(damageInfo);
#endif
				sdu->beginSlowDeath(damageInfo);
				return true;
			}
		}
	}

	return false;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void UndeadBody::applySecondLife(DamageInfo *damageInfo)
{
	// In second life, bind it to one hit point remaining, then go ahead and take it
	damageInfo->in.m_amount = min(damageInfo->in.m_amount, getHealth() - 1);

	// Damage first to apply hit effects
	ActiveBody::attemptDamage(damageInfo);

	// Flag module as no longer intercepting damage
	m_isSecondLife = TRUE;

	// Modify ActiveBody's max health and initial health
	setMaxHealth(getUndeadBodyModuleData()->m_secondLifeMaxHealth, FULLY_HEAL);

	// Set Armor set flag to use second life armor
	setArmorSetFlag(ARMORSET_SECOND_LIFE);
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
void UndeadBody::loadPostProcess()
{

	// extend base class
	ActiveBody::loadPostProcess();

}
