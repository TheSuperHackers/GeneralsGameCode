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

// FILE: FiringTracker.cpp //////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, February 2002
// Desc:   Keeps track of shots fired and people targeted for weapons that want a history of such a thing
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/AudioHandleSpecialValues.h"
#include "Common/GameType.h"
#include "Common/GameAudio.h"
#include "Common/PerfTimer.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"

#include "GameLogic/FiringTracker.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/ObjectHelper.h"
#include "GameLogic/Object.h"
#include "GameLogic/Weapon.h"

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

//-------------------------------------------------------------------------------------------------
FiringTracker::FiringTracker(Thing* thing, const ModuleData *modData) : UpdateModule( thing, modData )
{
	m_consecutiveShots = 0;
	m_victimID = INVALID_ID;
	m_frameToStartCooldown = 0;
 	m_frameToForceReload = 0;
	m_frameToStopLoopingSound = 0;
	m_audioHandle = AHSV_NoSound;
	setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);
}

//-------------------------------------------------------------------------------------------------
FiringTracker::~FiringTracker()
{
	// no need to protect this.
	TheAudio->removeAudioEvent( m_audioHandle );
	m_audioHandle = AHSV_NoSound;
}

//-------------------------------------------------------------------------------------------------
Int FiringTracker::getNumConsecutiveShotsAtVictim( const Object *victim ) const
{
	if( victim == NULL )
		return 0;// safety, this function is for asking about shots at a victim

	if( victim->getID() != m_victimID )
		return 0;// nope, not shooting at him

	return m_consecutiveShots;// this is how any times I have shot at this hoser right now
}

//-------------------------------------------------------------------------------------------------
void FiringTracker::shotFired(const Weapon* weaponFired, ObjectID victimID)
{
	UnsignedInt now = TheGameLogic->getFrame();
	Object *me = getObject();
	const Object *victim = TheGameLogic->findObjectByID(victimID); // May be null for ground shot

	if( victim && victim->testStatus(OBJECT_STATUS_FAERIE_FIRE) )
	{
		if( !me->testWeaponBonusCondition(WEAPONBONUSCONDITION_TARGET_FAERIE_FIRE) )
		{
			// We shoot faster at guys marked thusly
			me->setWeaponBonusCondition(WEAPONBONUSCONDITION_TARGET_FAERIE_FIRE);
		}
	}
	else
	{
		// A ground shot or the lack of the status on the target will clear this
		if( me->testWeaponBonusCondition(WEAPONBONUSCONDITION_TARGET_FAERIE_FIRE) )
		{
			me->clearWeaponBonusCondition(WEAPONBONUSCONDITION_TARGET_FAERIE_FIRE);
		}
	}

	if( victimID == m_victimID )
	{
		// Shooting at the same guy
		++m_consecutiveShots;
	}
	else if( now < m_frameToStartCooldown )
	{
		// Switching targets within the coast time is valid, and we will not spin down
		++m_consecutiveShots;
		m_victimID = victimID;
	}
	else
	{
		// Start the count over for the new guy
		m_consecutiveShots = 1;
		m_victimID = victimID;
	}

 	// Push back the time that we should force a reload with each shot
 	UnsignedInt autoReloadDelay = weaponFired->getAutoReloadWhenIdleFrames();
 	if( autoReloadDelay > 0 )
 		m_frameToForceReload = now + autoReloadDelay;

	UnsignedInt coast = weaponFired->getContinuousFireCoastFrames();
	if (coast)
		m_frameToStartCooldown = weaponFired->getPossibleNextShotFrame() + coast;
	else
		m_frameToStartCooldown = 0;

	Int shotsNeededOne = weaponFired->getContinuousFireOneShotsNeeded();
	Int shotsNeededTwo = weaponFired->getContinuousFireTwoShotsNeeded();

	if( me->testWeaponBonusCondition( WEAPONBONUSCONDITION_CONTINUOUS_FIRE_MEAN ) )
	{
		// Can either go up or down from here.
		if( m_consecutiveShots < shotsNeededOne )
			coolDown();
		else if( m_consecutiveShots > shotsNeededTwo )
			speedUp();
	}
	else if( me->testWeaponBonusCondition( WEAPONBONUSCONDITION_CONTINUOUS_FIRE_FAST ) )
	{
		// Only place I can go here from here is all the way down
		if( m_consecutiveShots < shotsNeededTwo )
		{
			coolDown();
		}
	}
	else
	{
		// Check to go up
		if( m_consecutiveShots > shotsNeededOne )
			speedUp();
	}

	UnsignedInt fireSoundLoopTime = weaponFired->getFireSoundLoopTime();
	if (fireSoundLoopTime != 0)
	{
		// If the sound has stopped playing, then we need to re-add it.
		if (m_frameToStopLoopingSound == 0 || !TheAudio->isCurrentlyPlaying(m_audioHandle))
		{
			AudioEventRTS audio = weaponFired->getFireSound();
			audio.setObjectID(getObject()->getID());
			m_audioHandle = TheAudio->addAudioEvent( &audio );
		}
		m_frameToStopLoopingSound = now + fireSoundLoopTime;
	}
	else
	{
		AudioEventRTS fireAndForgetSound = weaponFired->getFireSound();
		fireAndForgetSound.setObjectID(getObject()->getID());
		TheAudio->addAudioEvent(&fireAndForgetSound);
		m_frameToStopLoopingSound = 0;
	}


	setWakeFrame(me, calcTimeToSleep());
}

//-------------------------------------------------------------------------------------------------
UpdateSleepTime FiringTracker::update()
{
	//DEBUG_ASSERTCRASH(m_frameToStartCooldown != 0 || m_frameToStopLoopingSound != 0, ("hmm, should be asleep"));

	UnsignedInt now = TheGameLogic->getFrame();

 	// I have been idle long enough that I should reload, so I do not hang around with a near empty clip forever.
 	if( m_frameToForceReload != 0  &&  now >= m_frameToForceReload )
 	{
 		getObject()->reloadAllAmmo(TRUE);
 		m_frameToForceReload = 0;
 	}
 
	// If it has been too long since I fired.  I have to start over.
	// (don't call if we don't need to cool down... it's expensive!)

	if (m_frameToStopLoopingSound != 0)
	{
		if (now >= m_frameToStopLoopingSound)
		{
			TheAudio->removeAudioEvent( m_audioHandle );
			m_audioHandle = AHSV_NoSound;
			m_frameToStopLoopingSound = 0;
		}
	}

	if( m_frameToStartCooldown != 0 && now > m_frameToStartCooldown )
	{
		m_frameToStartCooldown = now + LOGICFRAMES_PER_SECOND;
		coolDown();// if this is the coolest call to cooldown, it will set m_frameToStartCooldown to zero
		return UPDATE_SLEEP(LOGICFRAMES_PER_SECOND);
	}

	UpdateSleepTime sleepTime = calcTimeToSleep();

	return sleepTime;
}

//-------------------------------------------------------------------------------------------------
UpdateSleepTime FiringTracker::calcTimeToSleep()
{
 	// Figure out the longest amount of time we can sleep as unneeded
 
 	// If all the timers are off, then we aren't needed at all
 	if (m_frameToStopLoopingSound == 0 && m_frameToStartCooldown == 0 && m_frameToForceReload == 0)
   		return UPDATE_SLEEP_FOREVER;
   
 	// Otherwise, we need to wake up to service the shortest timer
   	UnsignedInt now = TheGameLogic->getFrame();
 	UnsignedInt sleepTime = UPDATE_SLEEP_FOREVER;
 	if( m_frameToStopLoopingSound != 0 )
 	{
 		if( m_frameToStopLoopingSound <= now )
 			sleepTime = UPDATE_SLEEP_NONE;
 		else if( (m_frameToStopLoopingSound - now) < sleepTime )
 			sleepTime = m_frameToStopLoopingSound - now;
 	}
 	if( m_frameToStartCooldown != 0 )
 	{
 		if( m_frameToStartCooldown <= now )
 			sleepTime = UPDATE_SLEEP_NONE;
 		else if( (m_frameToStartCooldown - now) < sleepTime )
 			sleepTime = m_frameToStartCooldown - now;
 	}
 	if( m_frameToForceReload != 0 )
 	{
 		if( m_frameToForceReload <= now )
 			sleepTime = UPDATE_SLEEP_NONE;
 		else if( (m_frameToForceReload - now) < sleepTime )
 			sleepTime = m_frameToForceReload - now;
 	}
 
 	return UPDATE_SLEEP(sleepTime);
}

//-------------------------------------------------------------------------------------------------
void FiringTracker::speedUp()
{
	ModelConditionFlags clr, set;
	Object *self = getObject();
	
	if( self->testWeaponBonusCondition( WEAPONBONUSCONDITION_CONTINUOUS_FIRE_FAST ) )
	{
		//self->clearWeaponBonusCondition( WEAPONBONUSCONDITION_CONTINUOUS_FIRE_MEAN );
		//self->clearModelConditionState( MODELCONDITION_CONTINUOUS_FIRE_MEAN );
		//self->clearModelConditionState( MODELCONDITION_CONTINUOUS_FIRE_SLOW );
	}
	else if(self->testWeaponBonusCondition( WEAPONBONUSCONDITION_CONTINUOUS_FIRE_MEAN ) )
	{
		const AudioEventRTS *soundToPlayPtr = self->getTemplate()->getPerUnitSound( "VoiceRapidFire" );
		AudioEventRTS soundToPlay = *soundToPlayPtr;
		soundToPlay.setObjectID( self->getID() );
		TheAudio->addAudioEvent( &soundToPlay );

		// These flags are exclusive, not cumulative
		self->setWeaponBonusCondition( WEAPONBONUSCONDITION_CONTINUOUS_FIRE_FAST );
		set.set(MODELCONDITION_CONTINUOUS_FIRE_FAST);

		// These flags are exclusive, not cumulative
		self->clearWeaponBonusCondition( WEAPONBONUSCONDITION_CONTINUOUS_FIRE_MEAN );
		clr.set(MODELCONDITION_CONTINUOUS_FIRE_MEAN);
		clr.set(MODELCONDITION_CONTINUOUS_FIRE_SLOW);


	}
	else 
	{

		self->setWeaponBonusCondition( WEAPONBONUSCONDITION_CONTINUOUS_FIRE_MEAN );
		set.set(MODELCONDITION_CONTINUOUS_FIRE_MEAN);

		// These flags are exclusive, not cumulative
		self->clearWeaponBonusCondition( WEAPONBONUSCONDITION_CONTINUOUS_FIRE_FAST );
		clr.set(MODELCONDITION_CONTINUOUS_FIRE_FAST);
		clr.set(MODELCONDITION_CONTINUOUS_FIRE_SLOW);

	}

	self->clearAndSetModelConditionFlags(clr, set);

}

//-------------------------------------------------------------------------------------------------
void FiringTracker::coolDown()
{
	ModelConditionFlags clr, set;

	if( getObject()->testWeaponBonusCondition( WEAPONBONUSCONDITION_CONTINUOUS_FIRE_FAST ) 
	 || getObject()->testWeaponBonusCondition( WEAPONBONUSCONDITION_CONTINUOUS_FIRE_MEAN ))
	{

		// Straight to zero from wherever it is
		set.set(MODELCONDITION_CONTINUOUS_FIRE_SLOW);
		
		getObject()->clearWeaponBonusCondition( WEAPONBONUSCONDITION_CONTINUOUS_FIRE_FAST );
		getObject()->clearWeaponBonusCondition( WEAPONBONUSCONDITION_CONTINUOUS_FIRE_MEAN );
		clr.set(MODELCONDITION_CONTINUOUS_FIRE_FAST);
		clr.set(MODELCONDITION_CONTINUOUS_FIRE_MEAN);


	}
	else // we stop, now
	{

		getObject()->clearWeaponBonusCondition( WEAPONBONUSCONDITION_CONTINUOUS_FIRE_FAST );
		getObject()->clearWeaponBonusCondition( WEAPONBONUSCONDITION_CONTINUOUS_FIRE_MEAN );
		// Just chillin, nothing to change
		clr.set(MODELCONDITION_CONTINUOUS_FIRE_FAST);
		clr.set(MODELCONDITION_CONTINUOUS_FIRE_MEAN);
		clr.set(MODELCONDITION_CONTINUOUS_FIRE_SLOW);

		m_frameToStartCooldown = 0;
	}

	getObject()->clearAndSetModelConditionFlags(clr, set);

	// Start everything over
	m_consecutiveShots = 0;
	m_victimID = INVALID_ID;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void FiringTracker::crc( Xfer *xfer )
{

	// object helper base class
	UpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void FiringTracker::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// object helper base class
	UpdateModule::xfer( xfer );

	// consecutive shots
	xfer->xferInt( &m_consecutiveShots );

	// victim id
	xfer->xferObjectID( &m_victimID );

	// frame to start cooldown
	xfer->xferUnsignedInt( &m_frameToStartCooldown );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void FiringTracker::loadPostProcess( void )
{

	// object helper back class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
