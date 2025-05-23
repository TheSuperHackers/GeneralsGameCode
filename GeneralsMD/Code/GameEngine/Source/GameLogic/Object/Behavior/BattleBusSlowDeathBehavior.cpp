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

// FILE: BattleBusSlowDeathBehavior.cpp /////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, June 2003
// Desc:   Death for the Battle Bus.  Can do a fake slow death before the real one if triggered intentionally
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/Xfer.h"

#include "GameLogic/Module/BattleBusSlowDeathBehavior.h"

#include "Common/ModelState.h"

#include "GameClient/FXList.h"

#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/ObjectCreationList.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/Module/PhysicsUpdate.h"

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

enum
{
	GROUND_CHECK_DELAY = 10,		///< Check for colliding with the ground only after this long, to prevent hitting on the way up
	EMPTY_HULK_CHECK_DELAY = 15 ///< Check for empty hulk every this often instead of every frame
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
BattleBusSlowDeathBehaviorModuleData::BattleBusSlowDeathBehaviorModuleData( void )
{

	m_fxStartUndeath = NULL;
	m_oclStartUndeath = NULL;

	m_fxHitGround = NULL;
	m_oclHitGround = NULL;

	m_throwForce = 1.0f;
	m_percentDamageToPassengers = 0.0f;
	m_emptyHulkDestructionDelay = 0;

}  // end BattleBusSlowDeathBehaviorModuleData

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/*static*/ void BattleBusSlowDeathBehaviorModuleData::buildFieldParse( MultiIniFieldParse &p )
{
  SlowDeathBehaviorModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] = 
	{

		{ "FXStartUndeath",	INI::parseFXList,	NULL, offsetof( BattleBusSlowDeathBehaviorModuleData, m_fxStartUndeath ) },
		{ "OCLStartUndeath", INI::parseObjectCreationList, NULL, offsetof( BattleBusSlowDeathBehaviorModuleData, m_oclStartUndeath ) },

		{ "FXHitGround",	INI::parseFXList,	NULL, offsetof( BattleBusSlowDeathBehaviorModuleData, m_fxHitGround ) },
		{ "OCLHitGround", INI::parseObjectCreationList, NULL, offsetof( BattleBusSlowDeathBehaviorModuleData, m_oclHitGround ) },

		{ "ThrowForce", INI::parseReal, NULL, offsetof( BattleBusSlowDeathBehaviorModuleData, m_throwForce ) },
		{ "PercentDamageToPassengers", INI::parsePercentToReal, NULL, offsetof( BattleBusSlowDeathBehaviorModuleData, m_percentDamageToPassengers ) },
		{ "EmptyHulkDestructionDelay", INI::parseDurationUnsignedInt, NULL, offsetof( BattleBusSlowDeathBehaviorModuleData, m_emptyHulkDestructionDelay ) },

		{ 0, 0, 0, 0 }

	};

  p.add( dataFieldParse );

}  // end buildFieldParse

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
BattleBusSlowDeathBehavior::BattleBusSlowDeathBehavior( Thing *thing, const ModuleData *moduleData )
										: SlowDeathBehavior( thing, moduleData )
{

	m_isRealDeath = FALSE;
	m_isInFirstDeath = FALSE;
	m_groundCheckFrame = 0;
	m_penaltyDeathFrame = 0;
}  // end BattleBusSlowDeathBehavior

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
BattleBusSlowDeathBehavior::~BattleBusSlowDeathBehavior( void )
{

}  // end ~BattleBusSlowDeathBehavior
 
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void BattleBusSlowDeathBehavior::onDie( const DamageInfo *damageInfo )
{
	m_isRealDeath = TRUE;// Set beforehand because onDie could result in picking us to beginSlowDeath
	m_isInFirstDeath = FALSE; // and clear this incase we died while in the alternate death

	SlowDeathBehavior::onDie(damageInfo);
}  // end onDie

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void BattleBusSlowDeathBehavior::beginSlowDeath( const DamageInfo *damageInfo )
{
	Object *me = getObject();
	const BattleBusSlowDeathBehaviorModuleData * data = getBattleBusSlowDeathBehaviorModuleData();
	if( !m_isRealDeath )
	{
		// We can intercept and do our extra slow death if this is not from a real onDie
		m_isInFirstDeath = TRUE;
		m_groundCheckFrame = TheGameLogic->getFrame() + GROUND_CHECK_DELAY;

		// First do the special effects
		FXList::doFXObj(data->m_fxStartUndeath, me );
		ObjectCreationList::create(data->m_oclStartUndeath, me, NULL );

		if( me->getAI() )
		{
			// Then stop what we were doing
			me->getAI()->aiIdle(CMD_FROM_AI);
		}

		if( me->getPhysics() )
		{
			// Then stop physically
			me->getPhysics()->clearAcceleration();
			me->getPhysics()->scrubVelocity2D(0);
			
			// Then get chucked in the air
			Coord3D throwForce;
			throwForce.x = 0;
			throwForce.y = 0;
			throwForce.z = data->m_throwForce;
			me->getPhysics()->applyShock(&throwForce);
			me->getPhysics()->applyRandomRotation();
		}

		// And finally hit those inside for some damage
		if( me->getContain() )
			me->getContain()->processDamageToContained(data->m_percentDamageToPassengers);
		
		setWakeFrame(getObject(), UPDATE_SLEEP_NONE);
	}
	else
	{
		// If a real death, we aren't allowed to do anything
		SlowDeathBehavior::beginSlowDeath( damageInfo );
	}

}  // end beginSlowDeath

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UpdateSleepTime BattleBusSlowDeathBehavior::update( void )
{
	Object *me = getObject();
	const BattleBusSlowDeathBehaviorModuleData * data = getBattleBusSlowDeathBehaviorModuleData();
	if( m_isInFirstDeath )
	{
		// First death means we are doing our throw up in the air conversion

		if( (m_groundCheckFrame< TheGameLogic->getFrame() )  &&  !me->isAboveTerrain() )
		{
			// We're done since we hit the ground
			m_isInFirstDeath = FALSE;

			// Do the special FX
			FXList::doFXObj(data->m_fxHitGround, me );
			ObjectCreationList::create(data->m_oclHitGround, me, NULL );
			me->setModelConditionState(MODELCONDITION_SECOND_LIFE);

			// And stop us forever
			if( me->getAI() )
			{
				// Stop doing anything again (could try to move while in air)
				me->getAI()->aiIdle(CMD_FROM_AI);
			}
			if( me->getPhysics() )
			{
				// And stop physically again
				me->getPhysics()->clearAcceleration();
				me->getPhysics()->scrubVelocity2D(0);
			}
			me->setDisabled(DISABLED_HELD);

			// We can only sleep if we don't have to watch out for being empty.
			if( data->m_emptyHulkDestructionDelay == 0 )
				return UPDATE_SLEEP_FOREVER;
			else
				return UPDATE_SLEEP_NONE; // None, so we check for empty as soon as possible
		}
		return UPDATE_SLEEP_NONE;// None, since we are waiting to be able to check for ground collision
	}
	else if( m_isRealDeath )
	{
		// If a real death, we aren't allowed to do anything

		return SlowDeathBehavior::update();
	}
	else
	{
		// If neither death is active, we must be awake to check for emptiness

		const ContainModuleInterface *contain = me->getContain();
		// Safety, no need to be awake if no special case to wait for
		if( contain == NULL )
			return UPDATE_SLEEP_FOREVER;

		if( m_penaltyDeathFrame != 0 )
		{
			// We have been timed for death, see if we have re-filled first
			if( contain->getContainCount() > 0 )
			{
				m_penaltyDeathFrame = 0;
				return UPDATE_SLEEP(EMPTY_HULK_CHECK_DELAY);
			}
			else if( TheGameLogic->getFrame() > m_penaltyDeathFrame )
			{
				// No salvation, see if the timer is up to be killed.  Penalty prevents effects, Extra 4 prevents this die module from recalling.
				// Can't use Suicided death because a Terrorist actually inflicts Suicide deaths on others.
				me->kill(DAMAGE_PENALTY, DEATH_EXTRA_4);
				return UPDATE_SLEEP_FOREVER;// Forever, since we are dead and some other death module is in charge
			}
			else
			{
				return UPDATE_SLEEP(EMPTY_HULK_CHECK_DELAY);
			}
		}
		else
		{
			// We are not marked for death, so see if we should be
			if( contain->getContainCount() == 0 )
			{
				m_penaltyDeathFrame = TheGameLogic->getFrame() + data->m_emptyHulkDestructionDelay;
			}
			return UPDATE_SLEEP(EMPTY_HULK_CHECK_DELAY);// Stay awake regardless
		}
	}
}  // end update

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void BattleBusSlowDeathBehavior::crc( Xfer *xfer )
{

	// extend base class 
	SlowDeathBehavior::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void BattleBusSlowDeathBehavior::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	SlowDeathBehavior::xfer( xfer );

	xfer->xferBool( &m_isRealDeath );

	xfer->xferBool( &m_isInFirstDeath );

	xfer->xferUnsignedInt( &m_groundCheckFrame );

	xfer->xferUnsignedInt( &m_penaltyDeathFrame );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void BattleBusSlowDeathBehavior::loadPostProcess( void )
{

	// extend base class
	SlowDeathBehavior::loadPostProcess();

}  // end loadPostProcess
