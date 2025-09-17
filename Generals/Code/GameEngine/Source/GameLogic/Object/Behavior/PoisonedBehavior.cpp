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

// FILE: PoisonedBehavior.cpp /////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, July 2002
// Desc:   Behavior that reacts to poison Damage by continuously damaging us further in an Update
///////////////////////////////////////////////////////////////////////////////////////////////////


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameLogic/Module/PoisonedBehavior.h"
#include "GameLogic/Damage.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"


// tinting is all handled in drawable, now, Graham look near the bottom of Drawable::UpdateDrawable()
//static const RGBColor poisonedTint = {0.0f, 1.0f, 0.0f};

//-------------------------------------------------------------------------------------------------
PoisonedBehaviorModuleData::PoisonedBehaviorModuleData()
{
	m_poisonDamageIntervalData = 0; // How often I retake poison damage dealt me
	m_poisonDurationData = 0;				// And how long after the last poison dose I am poisoned
}

//-------------------------------------------------------------------------------------------------
/*static*/ void PoisonedBehaviorModuleData::buildFieldParse(MultiIniFieldParse& p)
{

	static const FieldParse dataFieldParse[] =
	{
		{ "PoisonDamageInterval", INI::parseDurationUnsignedInt, NULL, offsetof(PoisonedBehaviorModuleData, m_poisonDamageIntervalData) },
		{ "PoisonDuration", INI::parseDurationUnsignedInt, NULL, offsetof(PoisonedBehaviorModuleData, m_poisonDurationData) },
		{ 0, 0, 0, 0 }
	};

  UpdateModuleData::buildFieldParse(p);
  p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
PoisonedBehavior::PoisonedBehavior( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	m_poisonDamageFrame = 0;
	m_poisonOverallStopFrame = 0;
	m_poisonDamageAmount = 0.0f;
	m_poisonSource = INVALID_ID;
	m_deathType = DEATH_POISONED;
	setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
PoisonedBehavior::~PoisonedBehavior( void )
{
}

//-------------------------------------------------------------------------------------------------
/** Damage has been dealt, this is an opportunity to react to that damage */
//-------------------------------------------------------------------------------------------------
void PoisonedBehavior::onDamage( DamageInfo *damageInfo )
{
	if( damageInfo->in.m_damageType == DAMAGE_POISON )
		startPoisonedEffects( damageInfo );
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void PoisonedBehavior::onHealing( DamageInfo *damageInfo )
{
	stopPoisonedEffects();

	setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UpdateSleepTime PoisonedBehavior::update()
{
	const PoisonedBehaviorModuleData* d = getPoisonedBehaviorModuleData();
	UnsignedInt now = TheGameLogic->getFrame();

	if( m_poisonOverallStopFrame == 0 )
	{
		DEBUG_CRASH(("hmm, this should not happen"));
		return UPDATE_SLEEP_FOREVER;
		//we aren't poisoned, so nevermind
	}

	if (m_poisonDamageFrame != 0 && now >= m_poisonDamageFrame)
	{
		// If it is time to do damage, then do it and reset the damage timer
		DamageInfo damage;
		damage.in.m_amount = m_poisonDamageAmount;
		damage.in.m_sourceID = m_poisonSource;
		damage.in.m_damageType = DAMAGE_UNRESISTABLE; // Not poison, as that will infect us again
		damage.in.m_deathType = m_deathType;
		getObject()->attemptDamage( &damage );

		m_poisonDamageFrame = now + d->m_poisonDamageIntervalData;
	}

	// If we are now at zero we need to turn off our special effects...
	// unless the poison killed us, then we continue to be a pulsating toxic pus ball
	if( m_poisonOverallStopFrame != 0 &&
			now >= m_poisonOverallStopFrame &&
			!getObject()->isEffectivelyDead())
	{
		stopPoisonedEffects();
	}

	return calcSleepTime();
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UpdateSleepTime PoisonedBehavior::calcSleepTime()
{
	// UPDATE_SLEEP requires a count-of-frames, not an absolute-frame, so subtract 'now'
	UnsignedInt now = TheGameLogic->getFrame();
	if (m_poisonOverallStopFrame == 0 || m_poisonOverallStopFrame == now)
		return UPDATE_SLEEP_FOREVER;
	return frameToSleepTime(m_poisonDamageFrame, m_poisonOverallStopFrame);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void PoisonedBehavior::startPoisonedEffects( const DamageInfo *damageInfo )
{
	const PoisonedBehaviorModuleData* d = getPoisonedBehaviorModuleData();
	UnsignedInt now = TheGameLogic->getFrame();

	// We are going to take the damage dealt by the original poisoner every so often for a while.
	m_poisonDamageAmount = damageInfo->out.m_actualDamageDealt;
#if !RETAIL_COMPATIBLE_CRC
	// TheSuperHackers @bugfix Stubbjax 03/09/2025 Allow poison damage to award xp to the poison source.
	m_poisonSource = damageInfo->in.m_sourceID;
#endif

	m_poisonOverallStopFrame = now + d->m_poisonDurationData;

	// If we are getting re-poisoned, don't reset the damage counter if running, but do set it if unset
	if( m_poisonDamageFrame != 0 )
		m_poisonDamageFrame = min( m_poisonDamageFrame, now + d->m_poisonDamageIntervalData );
	else
		m_poisonDamageFrame = now + d->m_poisonDamageIntervalData;

	m_deathType = damageInfo->in.m_deathType;

	Drawable *myDrawable = getObject()->getDrawable();
	if( myDrawable )
		myDrawable->setTintStatus( TINT_STATUS_POISONED );// Graham, It has changed, see UpdateDrawable()

	setWakeFrame(getObject(), calcSleepTime());
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void PoisonedBehavior::stopPoisonedEffects()
{
	m_poisonDamageFrame = 0;
	m_poisonOverallStopFrame = 0;
	m_poisonDamageAmount = 0.0f;
	m_poisonSource = INVALID_ID;

	Drawable *myDrawable = getObject()->getDrawable();
	if( myDrawable )
		myDrawable->clearTintStatus( TINT_STATUS_POISONED );// Graham, It has changed, see UpdateDrawable()
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void PoisonedBehavior::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void PoisonedBehavior::xfer( Xfer *xfer )
{

	// version
#if RETAIL_COMPATIBLE_XFER_SAVE
	const XferVersion currentVersion = 2;
#else
	const XferVersion currentVersion = 3;
#endif
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// poisoned damage frame
	xfer->xferUnsignedInt( &m_poisonDamageFrame );

	// poison overall stop frame
	xfer->xferUnsignedInt( &m_poisonOverallStopFrame );

	// poison damage amount
	xfer->xferReal( &m_poisonDamageAmount );

	if (version >= 2)
	{
		xfer->xferUser(&m_deathType, sizeof(m_deathType));
	}

#if !RETAIL_COMPATIBLE_XFER_SAVE
	if (version >= 3)
	{
		xfer->xferObjectID(&m_poisonSource);
	}
#endif
}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void PoisonedBehavior::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}
