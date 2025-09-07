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

// TurretAI.cpp
// Turret behavior implementation
// Author: Steven Johnson, April 2002

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#define DEFINE_WEAPONSLOTTYPE_NAMES

#include "Common/GameAudio.h"
#include "Common/PerfTimer.h"
#include "Common/RandomValue.h"
#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"

#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/TerrainLogic.h"
#include "GameLogic/TurretAI.h"
#include "GameLogic/Weapon.h"
#include "GameLogic/WeaponSet.h"

const UnsignedInt WAIT_INDEFINITELY = 0xffffffff;


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
static StateReturnType frameToSleepTime(
	UnsignedInt frame1,
	UnsignedInt frame2 = FOREVER,
	UnsignedInt frame3 = FOREVER,
	UnsignedInt frame4 = FOREVER
)
{
	if (frame1 > frame2) frame1 = frame2;
	if (frame1 > frame3) frame1 = frame3;
	if (frame1 > frame4) frame1 = frame4;
	UnsignedInt now = TheGameLogic->getFrame();
	if (frame1 > now)
	{
		return STATE_SLEEP(frame1 - now);
	}
	else
	{
		// ignore times that are in the past, since this can frequently happen
		return STATE_CONTINUE;
	}
}

//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------
/**
 * Create a TurretAI state machine. Define all of the states the machine
 * can possibly be in, and set the initial (default) state.
 */
TurretStateMachine::TurretStateMachine( TurretAI* tai, Object *obj, AsciiString name ) : m_turretAI(tai), StateMachine( obj, name )
{
	static const StateConditionInfo fireConditions[] =
	{
		StateConditionInfo(outOfWeaponRangeObject, TURRETAI_AIM, NULL),
		StateConditionInfo(NULL, NULL, NULL)	// keep last
	};

	// order matters: first state is the default state.
	defineState( TURRETAI_IDLE,					newInstance(TurretAIIdleState)( this ), TURRETAI_IDLE, TURRETAI_IDLESCAN );
	defineState( TURRETAI_IDLESCAN,			newInstance(TurretAIIdleScanState)( this ), TURRETAI_HOLD, TURRETAI_HOLD );
	defineState( TURRETAI_AIM,					newInstance(TurretAIAimTurretState)( this ), TURRETAI_FIRE, TURRETAI_HOLD );
	defineState( TURRETAI_FIRE,					newInstance(AIAttackFireWeaponState)( this, tai ), TURRETAI_AIM, TURRETAI_AIM, fireConditions );
	defineState( TURRETAI_RECENTER,			newInstance(TurretAIRecenterTurretState)( this ), TURRETAI_IDLE, TURRETAI_IDLE );
	defineState( TURRETAI_HOLD,					newInstance(TurretAIHoldTurretState)( this ), TURRETAI_RECENTER, TURRETAI_RECENTER );
}

//----------------------------------------------------------------------------------------------------------
TurretStateMachine::~TurretStateMachine()
{
}

//----------------------------------------------------------------------------------------------------------
void TurretStateMachine::clear()
{
	StateMachine::clear();

	TurretAI* turret = getTurretAI();
	if (turret)
		turret->friend_notifyStateMachineChanged();
}

//----------------------------------------------------------------------------------------------------------
StateReturnType TurretStateMachine::resetToDefaultState()
{
	StateReturnType tmp = StateMachine::resetToDefaultState();

	TurretAI* turret = getTurretAI();
	if (turret)
		turret->friend_notifyStateMachineChanged();

	return tmp;
}

//----------------------------------------------------------------------------------------------------------
StateReturnType TurretStateMachine::setState(StateID newStateID)
{
	StateID oldID = getCurrentStateID();
	StateReturnType tmp = StateMachine::setState(newStateID);

	TurretAI* turret = getTurretAI();
	if (turret && oldID != newStateID)
		turret->friend_notifyStateMachineChanged();

	return tmp;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void TurretStateMachine::crc( Xfer *xfer )
{
}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer Method */
// ------------------------------------------------------------------------------------------------
void TurretStateMachine::xfer( Xfer *xfer )
{
	XferVersion cv = 1;
	XferVersion v = cv;
	xfer->xferVersion( &v, cv );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void TurretStateMachine::loadPostProcess( void )
{
}  // end loadPostProcess

//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------
TurretAIData::TurretAIData()
{
	m_turnRate = DEFAULT_TURN_RATE;
	m_pitchRate = DEFAULT_PITCH_RATE;
	m_naturalTurretAngle = 0.0f;
	m_naturalTurretPitch = 0.0f;
	for( Int slotIndex = 0; slotIndex < WEAPONSLOT_COUNT; ++slotIndex )
	{
		m_turretFireAngleSweep[slotIndex] = 0.0f;
		m_turretSweepSpeedModifier[slotIndex] = 1.0f;
	}
	m_firePitch = 0.0f;
	m_minPitch = 0.0f;
	m_groundUnitPitch = 0;
	m_turretWeaponSlots = 0;
#ifdef INTER_TURRET_DELAY
	m_interTurretDelay = 0;
#endif
	m_minIdleScanAngle = 0.0f;
	m_maxIdleScanAngle = 0.0f;
	m_groundUnitPitch = 0.0f;

	m_minIdleScanInterval = 9999999;
	m_maxIdleScanInterval = 9999999;
	m_recenterTime = 2*LOGICFRAMES_PER_SECOND;
	m_initiallyDisabled = false;
	m_firesWhileTurning = FALSE;
	m_isAllowsPitch = false;
}

//-------------------------------------------------------------------------------------------------
static void parseTWS(INI* ini, void * /*instance*/, void * store, const void* /*userData*/)
{
	UnsignedInt* tws = (UnsignedInt*)store;
	const char* token = ini->getNextToken();
	while (token != NULL)
	{
		WeaponSlotType wslot = (WeaponSlotType)INI::scanIndexList(token, TheWeaponSlotTypeNames);
		*tws |= (1 << wslot);
		token = ini->getNextTokenOrNull();
	}
}

//-------------------------------------------------------------------------------------------------
void TurretAIData::parseTurretSweep(INI* ini, void *instance, void * /*store*/, const void* userData)
{
	TurretAIData* self = (TurretAIData*)instance;
	WeaponSlotType wslot = (WeaponSlotType)INI::scanIndexList(ini->getNextToken(), TheWeaponSlotTypeNames);
	INI::parseAngleReal( ini, instance, &self->m_turretFireAngleSweep[wslot], NULL );
}

//-------------------------------------------------------------------------------------------------
void TurretAIData::parseTurretSweepSpeed(INI* ini, void *instance, void * /*store*/, const void* userData)
{
	TurretAIData* self = (TurretAIData*)instance;
	WeaponSlotType wslot = (WeaponSlotType)INI::scanIndexList(ini->getNextToken(), TheWeaponSlotTypeNames);
	INI::parseReal( ini, instance, &self->m_turretSweepSpeedModifier[wslot], NULL );
}

//----------------------------------------------------------------------------------------------------------
void TurretAIData::buildFieldParse(MultiIniFieldParse& p)
{
	static const FieldParse dataFieldParse[] =
	{
		{ "TurretTurnRate",					INI::parseAngularVelocityReal,				NULL, offsetof( TurretAIData, m_turnRate ) },
		{ "TurretPitchRate",				INI::parseAngularVelocityReal,				NULL, offsetof( TurretAIData, m_pitchRate ) },
		{ "NaturalTurretAngle",			INI::parseAngleReal,									NULL, offsetof( TurretAIData, m_naturalTurretAngle ) },
		{ "NaturalTurretPitch",			INI::parseAngleReal,									NULL, offsetof( TurretAIData, m_naturalTurretPitch ) },
		{ "FirePitch",							INI::parseAngleReal,									NULL, offsetof( TurretAIData, m_firePitch ) },
		{ "MinPhysicalPitch",				INI::parseAngleReal,									NULL, offsetof( TurretAIData, m_minPitch ) },
		{ "GroundUnitPitch",				INI::parseAngleReal,									NULL, offsetof( TurretAIData, m_groundUnitPitch ) },
		{ "TurretFireAngleSweep",		TurretAIData::parseTurretSweep,				NULL, NULL },
		{ "TurretSweepSpeedModifier",TurretAIData::parseTurretSweepSpeed,	NULL, NULL },
		{ "ControlledWeaponSlots",	parseTWS,															NULL, offsetof( TurretAIData, m_turretWeaponSlots ) },
		{ "AllowsPitch",						INI::parseBool,												NULL, offsetof( TurretAIData, m_isAllowsPitch ) },
#ifdef INTER_TURRET_DELAY
		{ "InterTurretDelay",				INI::parseDurationUnsignedInt,				NULL, offsetof( TurretAIData, m_interTurretDelay ) },
#endif
		{ "MinIdleScanAngle",				INI::parseAngleReal,									NULL, offsetof( TurretAIData, m_minIdleScanAngle ) },
		{ "MaxIdleScanAngle",				INI::parseAngleReal,									NULL, offsetof( TurretAIData, m_maxIdleScanAngle ) },
		{ "MinIdleScanInterval",		INI::parseDurationUnsignedInt,				NULL, offsetof( TurretAIData, m_minIdleScanInterval ) },
		{ "MaxIdleScanInterval",		INI::parseDurationUnsignedInt,				NULL, offsetof( TurretAIData, m_maxIdleScanInterval ) },
		{ "RecenterTime",						INI::parseDurationUnsignedInt,				NULL, offsetof( TurretAIData, m_recenterTime ) },
		{ "InitiallyDisabled",			INI::parseBool,												NULL, offsetof( TurretAIData, m_initiallyDisabled ) },
		{ "FiresWhileTurning",			INI::parseBool,												NULL, offsetof( TurretAIData, m_firesWhileTurning ) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);

}

//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------
TurretAI::TurretAI(Object* owner, const TurretAIData* data, WhichTurretType tur) :
	m_owner(owner),
	m_whichTurret(tur),
	m_data(data),
	m_turretStateMachine(NULL),
	m_playRotSound(false),
	m_playPitchSound(false),
	m_positiveSweep(true),
	m_enableSweepUntil(0),
	m_sleepUntil(0),
	m_didFire(false),
	m_target(TARGET_NONE),
	m_targetWasSetByIdleMood(false),
	m_enabled(!data->m_initiallyDisabled),
	m_firesWhileTurning(data->m_firesWhileTurning),
	m_isForceAttacking(false),
	//Added By Sadullah Nader
	//Initialization(s) inserted
	m_victimInitialTeam(NULL)
	//
{
	//Added By Sadullah Nader
	//Initialization(s) inserted
	m_continuousFireExpirationFrame = -1;
	//
	if (!m_data)
	{
		DEBUG_CRASH(("TurretAI MUST have ModuleData"));
		throw INI_INVALID_DATA;
	}

	if (m_data->m_turretWeaponSlots == 0)
	{
		DEBUG_CRASH(("TurretAI MUST specify controlled weapon slots!"));
		throw INI_INVALID_DATA;
	}
	m_angle = getNaturalTurretAngle();
	m_pitch = getNaturalTurretPitch();

#ifdef RTS_DEBUG
	char smbuf[256];
	sprintf(smbuf, "TurretStateMachine for tur %08lx slot %d",this,tur);
	const char* smname = smbuf;
#else
	const char* smname = "TurretStateMachine";
#endif
	m_turretStateMachine = newInstance(TurretStateMachine)(this, m_owner, smname);
	m_turretStateMachine->initDefaultState();

	m_turretRotOrPitchSound = *(m_owner->getTemplate()->getPerUnitSound("TurretMoveLoop"));
}

//----------------------------------------------------------------------------------------------------------
TurretAI::~TurretAI()
{
	stopRotOrPitchSound();

	deleteInstance(m_turretStateMachine);
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void TurretAI::crc( Xfer *xfer )
{
}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer Method */
// ------------------------------------------------------------------------------------------------
void TurretAI::xfer( Xfer *xfer )
{
  // version
  const XferVersion currentVersion = 2;
  XferVersion version = currentVersion;
  xfer->xferVersion( &version, currentVersion );

/* These 4 are loaded on creation, and don't change. jba.
	const TurretAIData*				m_data;
	WhichTurretType						m_whichTurret;
	Object*										m_owner;
	AudioEventRTS							m_turretRotOrPitchSound;		///< Sound of turret rotation
	*/
	xfer->xferSnapshot(m_turretStateMachine);

	xfer->xferReal(&m_angle);
	xfer->xferReal(&m_pitch);
	xfer->xferUnsignedInt(&m_enableSweepUntil);

	xfer->xferUser(&m_target, sizeof(m_target));
	xfer->xferUnsignedInt(&m_continuousFireExpirationFrame);
	Bool tmpBool;
#define UNPACK_AND_XFER(val) {tmpBool = val; xfer->xferBool(&tmpBool); val = tmpBool;}
	UNPACK_AND_XFER(m_playRotSound);
	UNPACK_AND_XFER(m_playPitchSound);
	UNPACK_AND_XFER(m_positiveSweep);
	UNPACK_AND_XFER(m_didFire);
	UNPACK_AND_XFER(m_enabled);
	UNPACK_AND_XFER(m_firesWhileTurning);
	UNPACK_AND_XFER(m_targetWasSetByIdleMood);
#undef UNPACK_AND_XFER

	if (version >= 2)
		xfer->xferUnsignedInt(&m_sleepUntil);

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void TurretAI::loadPostProcess( void )
{
	Object *victim = m_turretStateMachine->getGoalObject();
	if (victim) {
		m_victimInitialTeam = victim->getTeam();
	}
}  // end loadPostProcess

//----------------------------------------------------------------------------------------------------------
Bool TurretAI::friend_turnTowardsAngle(Real desiredAngle, Real rateModifier, Real relThresh)
{
	desiredAngle = normalizeAngle(desiredAngle);

	// rotate turret back to zero angle
	Real origAngle = getTurretAngle();
	Real actualAngle = origAngle;
	Real turnRate = getTurnRate() * rateModifier;
	Real angleDiff = normalizeAngle(desiredAngle - actualAngle);

	// Are we close enough to the desired angle to just snap there?
	if (fabs(angleDiff) < turnRate)
	{
		// we are centered
		actualAngle = desiredAngle;

		getOwner()->clearModelConditionState(MODELCONDITION_TURRET_ROTATE);
	}
	else
	{
		if (angleDiff > 0)
			actualAngle += turnRate;
		else
			actualAngle -= turnRate;

		getOwner()->setModelConditionState(MODELCONDITION_TURRET_ROTATE);
		m_playRotSound = true;
	}

	m_angle = normalizeAngle(actualAngle);

	if( m_angle != origAngle )
		getOwner()->reactToTurretChange( m_whichTurret, origAngle, m_pitch );

	Bool aligned = fabs(m_angle - desiredAngle) <= relThresh;

	return aligned;
}

//----------------------------------------------------------------------------------------------------------
Bool TurretAI::friend_turnTowardsPitch(Real desiredPitch, Real rateModifier)
{
	if (!isAllowsPitch())
		return true;

	desiredPitch = normalizeAngle(desiredPitch);

	// rotate turret back to zero angle
	Real actualPitch = getTurretPitch();
	Real pitchRate = getPitchRate() * rateModifier;
	Real pitchDiff = normalizeAngle(desiredPitch - actualPitch);

	if (fabs(pitchDiff) < pitchRate)
	{
		// we are centered
		actualPitch = desiredPitch;
	}
	else
	{
		if (pitchDiff > 0)
			actualPitch += pitchRate;
		else
			actualPitch -= pitchRate;
		m_playPitchSound = true;
	}

	m_pitch = normalizeAngle(actualPitch);

	return (m_pitch == desiredPitch);
}

//----------------------------------------------------------------------------------------------------------
Bool TurretAI::isWeaponSlotOkToFire(WeaponSlotType wslot) const
{
  // If we turrets are linked, ai wants us to fire together, regardless of slot
  if( getOwner()->getAI()->areTurretsLinked() )
    return TRUE;

	return isWeaponSlotOnTurret(wslot);
}

//----------------------------------------------------------------------------------------------------------
Real TurretAI::getTurretFireAngleSweepForWeaponSlot( WeaponSlotType slot ) const
{
	return m_data->m_turretFireAngleSweep[slot];
}

//----------------------------------------------------------------------------------------------------------
Real TurretAI::getTurretSweepSpeedModifierForWeaponSlot( WeaponSlotType slot ) const
{
	return m_data->m_turretSweepSpeedModifier[slot];
}

//----------------------------------------------------------------------------------------------------------
void TurretAI::notifyFired()
{
	m_didFire = true;
}

//----------------------------------------------------------------------------------------------------------
void TurretAI::notifyNewVictimChosen(Object* victim)
{
	setTurretTargetObject(victim, FALSE);
}

//----------------------------------------------------------------------------------------------------------
Bool TurretAI::isTryingToAimAtTarget(const Object* victim) const
{
	StateID sid = m_turretStateMachine->getCurrentStateID();

	Object* obj;
	Coord3D pos;
	return (sid == TURRETAI_AIM && friend_getTurretTarget(obj, pos) == TARGET_OBJECT && obj == victim);
}

//----------------------------------------------------------------------------------------------------------
Bool TurretAI::isOwnersCurWeaponOnTurret() const
{
	WeaponSlotType wslot;
	Weapon* w = m_owner->getCurrentWeapon(&wslot);
	return w != NULL && isWeaponSlotOnTurret(wslot);
}

//----------------------------------------------------------------------------------------------------------
Bool TurretAI::isWeaponSlotOnTurret(WeaponSlotType wslot) const
{
	return (m_data->m_turretWeaponSlots & (1 << wslot)) != 0;
}

//----------------------------------------------------------------------------------------------------------
TurretTargetType TurretAI::friend_getTurretTarget( Object*& obj, Coord3D& pos, Bool clearDeadTargets ) const
{
	obj = NULL;
	pos.zero();

	if (m_target == TARGET_OBJECT)
	{
		obj = m_turretStateMachine->getGoalObject();
		// clear it explicitly if null -- could be deceased but holding the
		// old (bogus) objectid internally
		if( clearDeadTargets )
		{
			if (obj == NULL || obj->isEffectivelyDead())
			{
				m_turretStateMachine->setGoalObject(NULL);
				m_target = TARGET_NONE;
				m_targetWasSetByIdleMood = false;
			}
		}
	}
	else if (m_target == TARGET_POSITION)
	{
		obj = NULL;
		pos = *m_turretStateMachine->getGoalPosition();
	}

	return m_target;
}

//----------------------------------------------------------------------------------------------------------
void TurretAI::removeSelfAsTargeter()
{
	// be paranoid, in case we are called from dtors, etc.
	if (m_target == TARGET_OBJECT && m_turretStateMachine != NULL)
	{
		Object* self = m_owner;
		Object* target = m_turretStateMachine->getGoalObject();
		if (self != NULL && target != NULL)
		{
			AIUpdateInterface* targetAI = target->getAI();
			if (targetAI)
			{
				targetAI->addTargeter(self->getID(), false);
			}
		}
	}
}

//----------------------------------------------------------------------------------------------------------
void TurretAI::setTurretTargetObject( Object *victim, Bool forceAttacking )
{
	if( !victim || victim->isEffectivelyDead() ||	!isOwnersCurWeaponOnTurret() )
	{
		if( !getOwner()->getAI()->areTurretsLinked() )
		{
			victim = NULL;
		}
	}

	if (victim == NULL)
	{
		// if nuking the victim, remove self as targeter before doing anything else.
		// (note that we never ADD self as targeter here; that is done in the aim state)
		removeSelfAsTargeter();
	}

	m_turretStateMachine->setGoalObject( victim );	// could be null! this is OK!
	m_target = victim ? TARGET_OBJECT : TARGET_NONE;
	m_targetWasSetByIdleMood = false;
	m_isForceAttacking = forceAttacking;

	StateID sid = m_turretStateMachine->getCurrentStateID();
	if (victim != NULL)
	{
		// if we're already in the aim state, don't call setState, since
		// it would go thru the exit/enter stuff, which we don't really want
		// to do...
		if (sid != TURRETAI_AIM && sid != TURRETAI_FIRE)
			m_turretStateMachine->setState( TURRETAI_AIM );
		m_victimInitialTeam = victim->getTeam();
	}
	else
	{
		// only change states if we are aiming.
		if (sid == TURRETAI_AIM || sid == TURRETAI_FIRE)
			m_turretStateMachine->setState(TURRETAI_HOLD);
		m_victimInitialTeam = NULL;
	}
}

//----------------------------------------------------------------------------------------------------------
void TurretAI::setTurretTargetPosition( const Coord3D* pos )
{
	if (!pos ||	!isOwnersCurWeaponOnTurret())
	{
		if( !getOwner()->getAI()->areTurretsLinked() )
		{
			pos = NULL;
		}
	}

	// remove self as targeter before doing anything else.
	// (note that we never ADD self as targeter here; that is done in the aim state)
	removeSelfAsTargeter();

	m_turretStateMachine->setGoalObject( NULL );
	if (pos)
		m_turretStateMachine->setGoalPosition( pos );
	m_target = pos ? TARGET_POSITION : TARGET_NONE;
	m_targetWasSetByIdleMood = false;

	StateID sid = m_turretStateMachine->getCurrentStateID();
	if (pos != NULL)
	{
		// if we're already in the aim state, don't call setState, since
		// it would go thru the exit/enter stuff, which we don't really want
		// to do...
		if (sid != TURRETAI_AIM && sid != TURRETAI_FIRE)
			m_turretStateMachine->setState( TURRETAI_AIM );
		m_victimInitialTeam = NULL;
	}
	else
	{
		// only change states if we are aiming.
		if (sid == TURRETAI_AIM || sid == TURRETAI_FIRE)
			m_turretStateMachine->setState(TURRETAI_HOLD);
		m_victimInitialTeam = NULL;
	}
}

//----------------------------------------------------------------------------------------------------------
void TurretAI::recenterTurret()
{
	m_turretStateMachine->setState( TURRETAI_RECENTER );
}

//----------------------------------------------------------------------------------------------------------
Bool TurretAI::isTurretInNaturalPosition() const
{

  if( this->getOwner()->testStatus( OBJECT_STATUS_UNDER_CONSTRUCTION))
    return true;//ML so that under-construction base-defenses do not re-center while under construction



	if( getNaturalTurretAngle() == getTurretAngle() &&
			getNaturalTurretPitch() == getTurretPitch() )
	{
		return true;
	}
	return false;
}

//----------------------------------------------------------------------------------------------------------
void TurretAI::friend_notifyStateMachineChanged()
{
	m_sleepUntil = TheGameLogic->getFrame();
}

//----------------------------------------------------------------------------------------------------------
/*
	Note that TurretAI isn't a behavior at all, so doesn't really "sleep"; however, it does return
	this information so that its AIUpdate owner can decide how long it (the AIUpdate) should sleep.
	So it behooves you to maximize "sleep" potential here! (srj)
*/
DECLARE_PERF_TIMER(TurretAI)
UpdateSleepTime TurretAI::updateTurretAI()
{
	USE_PERF_TIMER(TurretAI)

#if defined(RTS_DEBUG)
	DEBUG_ASSERTCRASH(!m_enabled ||
							m_turretStateMachine->peekSleepTill() == 0 ||
							m_turretStateMachine->peekSleepTill() >= m_sleepUntil, ("Turret Machine is less sleepy than turret"));
#endif

	UnsignedInt now = TheGameLogic->getFrame();
	if (m_sleepUntil != 0 && now < m_sleepUntil)
	{
		return UPDATE_SLEEP(m_sleepUntil - now);
	}

	//DEBUG_LOG(("updateTurretAI frame %d: %08lx",TheGameLogic->getFrame(),getOwner()));
	UpdateSleepTime subMachineSleep = UPDATE_SLEEP_FOREVER;	// assume the best!

	// either we don't care about continuous fire stuff, or we care, but time has elapsed
	if ((!m_firesWhileTurning) || (m_continuousFireExpirationFrame <= now))
	{
		m_playRotSound = false;
		m_playPitchSound = false;
	}

	if (m_enabled || m_turretStateMachine->getCurrentStateID() == TURRETAI_RECENTER)
	{
		m_didFire = false;

		// run the behavior state machine BEFORE doing sound check
		StateReturnType stRet = m_turretStateMachine->updateStateMachine();

		if (m_didFire)
		{
			// if we fired, enable sweeping for a few frames.
			const Int ENABLE_SWEEP_FRAME_COUNT = 3;
			m_enableSweepUntil = now + ENABLE_SWEEP_FRAME_COUNT;
			m_continuousFireExpirationFrame = now + ENABLE_SWEEP_FRAME_COUNT;// so the recent firing will not interrupt the moving sound
		}

		if (m_playRotSound || m_playPitchSound)
			startRotOrPitchSound();
		else
			stopRotOrPitchSound();

		if (IS_STATE_SLEEP(stRet))
		{
			Int frames = GET_STATE_SLEEP_FRAMES(stRet);
			if (frames < subMachineSleep)
				subMachineSleep = UPDATE_SLEEP(frames);
		}
		else
		{
			// it's STATE_CONTINUE, STATE_SUCCESS, or STATE_FAILURE,
			// any of which will probably require next frame
			subMachineSleep = UPDATE_SLEEP_NONE;
		}

	}	// if enabled or recentering

	m_sleepUntil = now + subMachineSleep;

#if defined(RTS_DEBUG)
	DEBUG_ASSERTCRASH(!m_enabled ||
							m_turretStateMachine->peekSleepTill() == 0 ||
							m_turretStateMachine->peekSleepTill() >= m_sleepUntil, ("Turret Machine is less sleepy than turret"));
#endif

	return subMachineSleep;
}

//-------------------------------------------------------------------------------------------------
void TurretAI::setTurretEnabled( Bool enabled )
{
	if (enabled && !m_enabled)
	{
		// be sure we wake up!
		m_sleepUntil = TheGameLogic->getFrame();
	}
	m_enabled = enabled;
}

//-------------------------------------------------------------------------------------------------
/**
 * Start turret rotation sound
 */
void TurretAI::startRotOrPitchSound()
{
	if (!m_turretRotOrPitchSound.isCurrentlyPlaying())
	{
		m_turretRotOrPitchSound.setObjectID(m_owner->getID());
		m_turretRotOrPitchSound.setPlayingHandle(TheAudio->addAudioEvent(&m_turretRotOrPitchSound));
	}
}

//-------------------------------------------------------------------------------------------------
/**
 * Stop turret rotation sound
 */
void TurretAI::stopRotOrPitchSound()
{
	if (m_turretRotOrPitchSound.isCurrentlyPlaying())
	{
		TheAudio->removeAudioEvent(m_turretRotOrPitchSound.getPlayingHandle());
	}
}

#ifdef INTER_TURRET_DELAY
//----------------------------------------------------------------------------------------------------------
void TurretAI::getOtherTurretWeaponInfo(Int& numSelf, Int& numSelfReloading, Int& numSelfReady, Int& numOther, Int& numOtherReloading, Int& numOtherReady) const
{
	numSelf = 0;
	numSelfReady = 0;
	numSelfReloading = 0;
	numOther = 0;
	numOtherReady = 0;
	numOtherReloading = 0;

	for( Int i = 0; i < WEAPONSLOT_COUNT;	i++ )
	{
		// ignore empty slots.
		const Weapon* w = getOwner()->getWeaponInWeaponSlot((WeaponSlotType)i);
		if (w == NULL)
			continue;

		// ignore the weapons on this turret.
		if (isWeaponSlotOnTurret((WeaponSlotType)i))
		{
			++numSelf;
			if (w->getStatus() == RELOADING_CLIP)
				++numSelfReloading;
			else if (w->getStatus() == READY_TO_FIRE)
				++numSelfReady;
		}
		else
		{
			++numOther;
			if (w->getStatus() == RELOADING_CLIP)
				++numOtherReloading;
			else if (w->getStatus() == READY_TO_FIRE)
				++numOtherReady;
		}
	}
}
#endif

//----------------------------------------------------------------------------------------------------------
Bool TurretAI::friend_isAnyWeaponInRangeOf(const Object* o) const
{
	// see if we've moved out of range, and if so, nuke the target.
	for (Int i = 0; i < WEAPONSLOT_COUNT;	i++ )
	{
		// ignore empty slots.
		const Weapon* w = getOwner()->getWeaponInWeaponSlot((WeaponSlotType)i);
		if (w == NULL || !isWeaponSlotOnTurret((WeaponSlotType)i))
			continue;

		if (w->isWithinAttackRange(getOwner(), o)
				// srj sez: not sure if we want to do this or not.
				// jba sez: no, don't do this.  isWithinAttackRange checks terrain los now, and
				// some weapons (like tomahawk) can fire beyond los, so this check is problematical here.
				//  && w->isClearFiringLineOfSightTerrain(getOwner(), o)
			)
		{
			return true;
		}
	}

	return false;
}

//----------------------------------------------------------------------------------------------------------
Bool TurretAI::friend_isSweepEnabled() const
{
	if (m_enableSweepUntil != 0 && m_enableSweepUntil > TheGameLogic->getFrame())
		return true;

	return false;
}

//----------------------------------------------------------------------------------------------------------
UnsignedInt TurretAI::friend_getNextIdleMoodTargetFrame() const
{
	const Object* obj = getOwner();
	const AIUpdateInterface *ai = obj->getAIUpdateInterface();
	// ai can be null during object construction.
	return ai ? ai->getNextMoodCheckTime() : TheGameLogic->getFrame();
}

//----------------------------------------------------------------------------------------------------------
void TurretAI::friend_checkForIdleMoodTarget()
{
	Object* obj = getOwner();
	AIUpdateInterface *ai = obj->getAIUpdateInterface();

	// This state is used internally some places, so we don't necessarily want to be looking for targets
	// Places that use AI_IDLE internally should set this to false in the constructor. jkmcd
	UnsignedInt moodAdjust = ai->getMoodMatrixActionAdjustment(MM_Action_Idle);
	if (moodAdjust & MAA_Affect_Range_IgnoreAll)
		return;

	// If we're supposed to attack based on mood, etc, then we will do so.
	Object* enemy = ai->getNextMoodTarget( true, true );
	if (enemy)
	{
		setTurretTargetObject(enemy, FALSE);
		obj->chooseBestWeaponForTarget(enemy, PREFER_MOST_DAMAGE, CMD_FROM_AI);
		// set this magic flag to indicate that this was an independent attack-of-opportunity,
		// and we should check to see if out-of-range, and if so, nuke it
		m_targetWasSetByIdleMood = true;
	}
}

#ifdef INTER_TURRET_DELAY
//----------------------------------------------------------------------------------------------------------
UnsignedInt TurretAI::friend_getInterTurretDelay()
{
	if (m_data->m_interTurretDelay == 0)
		return 0;

	Int numSelf;
	Int numSelfReloading;
	Int numSelfReady;
	Int numOther;
	Int numOtherReloading;
	Int numOtherReady;
	getOtherTurretWeaponInfo(numSelf, numSelfReloading, numSelfReady, numOther, numOtherReloading, numOtherReady);

	AIUpdateInterface* ai = getOwner()->getAIUpdateInterface();
	WhichTurretType t = ai->friend_getTurretSync();
	if (t == TURRET_INVALID || t == m_whichTurret)
	{
		if (numSelf == numSelfReady)
		{
			ai->friend_setTurretSync(m_whichTurret);
			// if we just got the flag, use our delay, otherwise go immediately
			if (t == TURRET_INVALID)
				return m_data->m_interTurretDelay;
			else
				return 0;
		}
		else if (numSelf == numSelfReloading)
		{
			// we're done... relinquish the flag
			ai->friend_setTurretSync(TURRET_INVALID);
			return WAIT_INDEFINITELY;
		}
		else
		{
			// we're not ready, but not reloading either, so retain the flag and wait
			return WAIT_INDEFINITELY;
		}
	}
	else
	{
		// someone else has it, so just wait.
		return WAIT_INDEFINITELY;
	}
}
#endif

//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------
StateReturnType TurretAIAimTurretState::onEnter()
{
	// don't set the "turret-rotate" bit here; wait and see if we really turn or not.
#ifdef INTER_TURRET_DELAY
	m_extraDelay = 0;
#endif
	return STATE_CONTINUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * Rotate our turret to point at the machine's goal
 */
StateReturnType TurretAIAimTurretState::update()
{
	//DEBUG_LOG(("TurretAIAimTurretState frame %d: %08lx",TheGameLogic->getFrame(),getTurretAI()->getOwner()));

	TurretAI* turret = getTurretAI();
	Object* obj = turret->getOwner();
	AIUpdateInterface* ai = obj->getAIUpdateInterface();
	if( !ai )
	{
		return STATE_FAILURE;
	}

	Object* enemy;
	AIUpdateInterface* enemyAI=NULL;
	Coord3D enemyPosition;
	Bool preventing = false;
	TurretTargetType targetType =  turret->friend_getTurretTarget(enemy, enemyPosition);
	Object *enemyForDistanceCheckOnly = enemy;	// Note: Do not use this anywhere except for the range check.

	Bool nothingInRange = false;
	switch (targetType)
	{
		case TARGET_NONE:
		{
			return STATE_FAILURE;
		}

		case TARGET_OBJECT:
		{
			Bool isPrimaryEnemy = (enemy && enemy == ai->getGoalObject());
			// if the enemy is gone, or we're out of range, or it changed teams, the attack is over
			Bool ableToAttackTarget = obj->isAbleToAttack();
			if (ableToAttackTarget)
			{
				// srj sez: since we have already acquired this target, we should use
				// the CONTINUED attack tests, not the new ones.
				CanAttackResult result = obj->getAbleToAttackSpecificObject(
							turret->isForceAttacking() ? ATTACK_CONTINUED_TARGET_FORCED : ATTACK_CONTINUED_TARGET,
							enemy,
							ai->getLastCommandSource()
						);
				ableToAttackTarget = result == ATTACKRESULT_POSSIBLE || result == ATTACKRESULT_POSSIBLE_AFTER_MOVING;
			}

			nothingInRange = !turret->friend_isAnyWeaponInRangeOf(enemy);
			if (enemy == NULL || !ableToAttackTarget ||
					(!isPrimaryEnemy && nothingInRange) ||
					enemy->getTeam() != turret->friend_getVictimInitialTeam()
			)
			{
				if (turret->friend_getTargetWasSetByIdleMood())
				{
					turret->setTurretTargetObject(NULL, FALSE);
				}
				return STATE_FAILURE;
			}

			// aim turret towards enemy (turret angle is relative to its parent object)
			if (enemy->isKindOf(KINDOF_BRIDGE))
			{
				// Special case - bridges have two attackable points at either end.
				TBridgeAttackInfo info;
				TheTerrainLogic->getBridgeAttackPoints(enemy, &info);
				Real distSqr = ThePartitionManager->getDistanceSquared( obj, &info.attackPoint1, FROM_BOUNDINGSPHERE_3D );
				if (distSqr > ThePartitionManager->getDistanceSquared( obj, &info.attackPoint2, FROM_BOUNDINGSPHERE_3D ) )
				{
					enemyPosition = info.attackPoint2;
				}
				else
				{
					enemyPosition = info.attackPoint1;
				}
			}
			else
			{
				enemyPosition = *enemy->getPosition();
			}

			enemyAI = enemy ? enemy->getAI() : NULL;

			// add ourself as a targeter BEFORE calling isTemporarilyPreventingAimSuccess().
			// we do this every time thru, just in case we get into a squabble with our ai
			// over whether or not we are a targeter... (srj)
			if (enemyAI)
				enemyAI->addTargeter(obj->getID(), true);

			preventing = enemyAI && enemyAI->isTemporarilyPreventingAimSuccess();

			// don't use 'enemy' after this point, just the position. to help
			// enforce this, we'll null it out.
			enemy = NULL;
			break;
		}

		case TARGET_POSITION:
		{
			// nothing, just break.
			break;
		}
	}

	WeaponSlotType slot;
	Weapon *curWeapon = obj->getCurrentWeapon( &slot );
	if (!curWeapon)
	{
		DEBUG_CRASH(("TurretAIAimTurretState::update - curWeapon is NULL."));
		return STATE_FAILURE;
	}

	Real turnSpeedModifier = 1.0f;// Just like how recentering turns you half speed, sweeping can change your turn speed

	Real relAngle = ThePartitionManager->getRelativeAngle2D( obj, &enemyPosition );

	Real aimAngle = relAngle;
	Real sweep = turret->getTurretFireAngleSweepForWeaponSlot( slot );
	if (sweep > 0.0f && turret->friend_isSweepEnabled())
	{
		if (turret->friend_getPositiveSweep())
			aimAngle += sweep;
		else
			aimAngle -= sweep;

		turnSpeedModifier = turret->getTurretSweepSpeedModifierForWeaponSlot( slot );
	}

	const Real REL_THRESH = 0.035f;	// about 2 degrees. (getRelativeAngle2D is current only accurate to about 1.25 degrees)
	Bool turnAlignedToNemesis = turret->friend_turnTowardsAngle(aimAngle, turnSpeedModifier, REL_THRESH);

	// this section we do even if sweep is "disabled", so that we can start firing
	// once we get into sweep "range"
	if (sweep > 0.0f)
	{
		if (turnAlignedToNemesis)
			turret->friend_setPositiveSweep(!turret->friend_getPositiveSweep());

		Real angleDiff = normalizeAngle(relAngle - turret->getTurretAngle());
		turnAlignedToNemesis = (fabs(angleDiff) < sweep);
	}

	Bool pitchAlignedToNemesis = true;

	// Now do pitch
	if( turret->isAllowsPitch() )
	{
		Real desiredPitch = 0;

		// Some turrets want to fire at a set pitch, and some want to fire straight to the target
		if( turret->getFirePitch() > 0 )
		{
			desiredPitch = turret->getFirePitch();
		}
		else
		{
			// Find the pitch to the target, but unlike Turn, we can't go 360 so bind at 0;
			Coord3D v;
			ThePartitionManager->getVectorTo(obj, &enemyPosition, FROM_CENTER_3D, v);

			//GetVectorTo only takes Object as the first, but we want the angle from our Weapon to the
			// target, not us to the target.  Raise our side to get the line to make sense.
			v.z -= obj->getGeometryInfo().getMaxHeightAbovePosition() / 2; // I kinda hate our logic/client split.
			//The point to fire from should be intrinsic to the turret, but in reality it is very slow to look it up.

 			Real actualPitch;
 			if( v.length() > 0 )
 				actualPitch = ASin( v.z / v.length() );
 			else
 				actualPitch = 0;// Don't point at NAN, just point at 0 if they are right on us

			desiredPitch = actualPitch;
			if( desiredPitch < turret->getMinPitch() )
			{
				desiredPitch = turret->getMinPitch();
			}
			if (turret->getGroundUnitPitch() > 0) {
				Bool adjust = false;
				if (!enemy) {
					adjust = true; // adjust for ground targets.
				}
				if (enemy && enemy->isKindOf(KINDOF_IMMOBILE)) {
					adjust = true;
				}
				if (enemyAI && enemyAI->isDoingGroundMovement()) {
					adjust = true;
				}
				if (adjust) {
					Real range = curWeapon->getAttackRange(obj);
					Real dist = v.length();
					if (range<1) range = 1; // paranoia. jba.
					// As the unit gets closer, reduce the pitch so we don't shoot over him.
					Real groundPitch = turret->getGroundUnitPitch() * (dist/range);
					desiredPitch = actualPitch+groundPitch;
					if (desiredPitch < turret->getMinPitch()) {
						desiredPitch = turret->getMinPitch();
					}
				}
			}

		}

		pitchAlignedToNemesis = turret->friend_turnTowardsPitch(desiredPitch, 1.0f);
	}

	// For now, we require that we're within range before we can successfully exit the AIM state,
	// and move into the FIRE state.
	if (turnAlignedToNemesis && pitchAlignedToNemesis &&
		((enemyForDistanceCheckOnly && curWeapon->isWithinAttackRange(obj, enemyForDistanceCheckOnly)) ||
		 (!enemyForDistanceCheckOnly && curWeapon->isWithinAttackRange(obj, &enemyPosition))))
	{
#ifdef INTER_TURRET_DELAY
		if (m_extraDelay > 0)
		{
			--m_extraDelay;
		}
		else
		{
			m_extraDelay = turret->friend_getInterTurretDelay();
			if (m_extraDelay == WAIT_INDEFINITELY)
			{
				m_extraDelay = 0;
				return STATE_CONTINUE;
			}
		}
		if (m_extraDelay != 0)
		{
			return STATE_CONTINUE;
		}
#endif
		if (preventing || nothingInRange)
		{
			return STATE_CONTINUE;
		}
		return STATE_SUCCESS;
	}

	// stay in aiming state until directed to do something else
	return STATE_CONTINUE;
}

//-------------------------------------------------------------------------------------------------
void TurretAIAimTurretState::onExit( StateExitType status )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------
StateReturnType TurretAIRecenterTurretState::onEnter()
{
	return STATE_CONTINUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * Rotate the owner's turret to its home orientation.
 */

StateReturnType TurretAIRecenterTurretState::update()
{
	//DEBUG_LOG(("TurretAIRecenterTurretState frame %d: %08lx",TheGameLogic->getFrame(),getTurretAI()->getOwner()));


  if( getMachineOwner()->testStatus( OBJECT_STATUS_UNDER_CONSTRUCTION))
    return STATE_CONTINUE;//ML so that under-construction base-defenses do not re-center while under construction


	TurretAI* turret = getTurretAI();
	Bool angleAligned = turret->friend_turnTowardsAngle(turret->getNaturalTurretAngle(), 0.5f, 0.0f);
	Bool pitchAligned = turret->friend_turnTowardsPitch(turret->getNaturalTurretPitch(), 0.5f);

	if( angleAligned && pitchAligned )
		return STATE_SUCCESS;

	return STATE_CONTINUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * Stop rotation sound
 */
void TurretAIRecenterTurretState::onExit( StateExitType status )
{
}

//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void TurretAIIdleState::crc( Xfer *xfer )
{
}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer Method */
// ------------------------------------------------------------------------------------------------
void TurretAIIdleState::xfer( Xfer *xfer )
{
  // version
  XferVersion currentVersion = 1;
  XferVersion version = currentVersion;
  xfer->xferVersion( &version, currentVersion );

	xfer->xferUnsignedInt(&m_nextIdleScan);
}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void TurretAIIdleState::loadPostProcess( void )
{
}  // end loadPostProcess

//----------------------------------------------------------------------------------------------------------
void TurretAIIdleState::resetIdleScan()
{
	UnsignedInt now = TheGameLogic->getFrame();
	UnsignedInt delay = GameLogicRandomValue(getTurretAI()->getMinIdleScanInterval(), getTurretAI()->getMaxIdleScanInterval());
	m_nextIdleScan = now + delay;
}

//----------------------------------------------------------------------------------------------------------
StateReturnType TurretAIIdleState::onEnter()
{
	AIUpdateInterface *ai = getMachineOwner()->getAIUpdateInterface();
	// ai doesn't exist if the object was just created this frame.
	if (ai)
	{
		ai->resetNextMoodCheckTime();
		if (ai->friend_getTurretSync() == getTurretAI()->friend_getWhichTurret())
			ai->friend_setTurretSync(TURRET_INVALID);
	}

	resetIdleScan();

	TurretAI* turret = getTurretAI();
	return frameToSleepTime(turret->friend_getNextIdleMoodTargetFrame(), m_nextIdleScan);
}

//----------------------------------------------------------------------------------------------------------
StateReturnType TurretAIIdleState::update()
{
	//DEBUG_LOG(("TurretAIIdleState frame %d: %08lx",TheGameLogic->getFrame(),getTurretAI()->getOwner()));

	UnsignedInt now = TheGameLogic->getFrame();
	if (now >= m_nextIdleScan)
	{
		// this is redundant, since we're exiting the state, and will reset
		// it again in onEnter next time (srj)
		// resetIdleScan();

		return STATE_FAILURE;
	}

	TurretAI* turret = getTurretAI();
	turret->friend_checkForIdleMoodTarget();

	return frameToSleepTime(turret->friend_getNextIdleMoodTargetFrame(), m_nextIdleScan);
}

//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void TurretAIIdleScanState::crc( Xfer *xfer )
{
}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer Method */
// ------------------------------------------------------------------------------------------------
void TurretAIIdleScanState::xfer( Xfer *xfer )
{
  // version
  XferVersion currentVersion = 1;
  XferVersion version = currentVersion;
  xfer->xferVersion( &version, currentVersion );

	xfer->xferReal(&m_desiredAngle);
}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void TurretAIIdleScanState::loadPostProcess( void )
{
}  // end loadPostProcess

//----------------------------------------------------------------------------------------------------------
StateReturnType TurretAIIdleScanState::onEnter()
{
	Real minA = getTurretAI()->getMinIdleScanAngle();
	Real maxA = getTurretAI()->getMaxIdleScanAngle();
	if (minA == 0.0f && maxA == 0.0f)
		return STATE_SUCCESS;

	m_desiredAngle = minA + GameLogicRandomValueReal(0, maxA - minA);
	if (GameLogicRandomValue( 0, 1 ) == 0)
		m_desiredAngle = -m_desiredAngle;

	return STATE_CONTINUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * Rotate the owner's turret to its scan orientation.
 */

StateReturnType TurretAIIdleScanState::update()
{
	//DEBUG_LOG(("TurretAIIdleScanState frame %d: %08lx",TheGameLogic->getFrame(),getTurretAI()->getOwner()));

  if( getMachineOwner()->testStatus( OBJECT_STATUS_UNDER_CONSTRUCTION))
    return STATE_CONTINUE;//ML so that under-construction base-defenses do not idle-scan while under construction

	Bool angleAligned = getTurretAI()->friend_turnTowardsAngle(getTurretAI()->getNaturalTurretAngle() + m_desiredAngle, 0.5f, 0.0f);
	Bool pitchAligned = getTurretAI()->friend_turnTowardsPitch(getTurretAI()->getNaturalTurretPitch(), 0.5f);

	if( angleAligned && pitchAligned )
		return STATE_SUCCESS;

	return STATE_CONTINUE;
}

//-------------------------------------------------------------------------------------------------
/**
 * Stop rotation sound
 */
void TurretAIIdleScanState::onExit( StateExitType status )
{
}

//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void TurretAIHoldTurretState::crc( Xfer *xfer )
{
}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer Method */
// ------------------------------------------------------------------------------------------------
void TurretAIHoldTurretState::xfer( Xfer *xfer )
{
  // version
  XferVersion currentVersion = 1;
  XferVersion version = currentVersion;
  xfer->xferVersion( &version, currentVersion );

	xfer->xferUnsignedInt(&m_timestamp);
}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void TurretAIHoldTurretState::loadPostProcess( void )
{
}  // end loadPostProcess

//----------------------------------------------------------------------------------------------------------
StateReturnType TurretAIHoldTurretState::onEnter()
{
	m_timestamp = TheGameLogic->getFrame() + getTurretAI()->getRecenterTime();

	TurretAI* turret = getTurretAI();
	return frameToSleepTime(turret->friend_getNextIdleMoodTargetFrame(), m_timestamp);
}

//-------------------------------------------------------------------------------------------------
void TurretAIHoldTurretState::onExit( StateExitType status )
{
}

//----------------------------------------------------------------------------------------------------------
/**
 * Hold the turret's position for a short time before returning to center.
 */

StateReturnType TurretAIHoldTurretState::update()
{
	//DEBUG_LOG(("TurretAIHoldTurretState frame %d: %08lx",TheGameLogic->getFrame(),getTurretAI()->getOwner()));

	if (TheGameLogic->getFrame() >= m_timestamp)
		return STATE_SUCCESS;

	TurretAI* turret = getTurretAI();
	turret->friend_checkForIdleMoodTarget();

	return frameToSleepTime(turret->friend_getNextIdleMoodTargetFrame(), m_timestamp);
}
