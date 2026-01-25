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

// TurretAI.h
// Author: Steven Johnson, April 2002
// Simple turret behavior

#pragma once

#include "Common/StateMachine.h"
#include "Common/GameMemory.h"

const Real DEFAULT_TURN_RATE = 0.01f;
const Real DEFAULT_PITCH_RATE = 0.01f;

/**
 * The TurretAI state IDs.
 * Each of these constants will be associated with an instance of a State class
 * in a given StateMachine.
 */
enum TurretStateType CPP_11(: Int)
{
	TURRETAI_IDLE,
	TURRETAI_IDLESCAN,
	TURRETAI_AIM,															///< aim turret at GoalObject
	TURRETAI_FIRE,														///< fire turret at GoalObject
	TURRETAI_RECENTER,												///< rotate turret back to default position
	TURRETAI_HOLD,														///< hold turret position for a bit before recenter

	NUM_TURRETAI_STATES
};

// FORWARD DECLARATIONS ///////////////////////////////////////////////////////////////////////////
class Weapon;
class TurretAI;

//-----------------------------------------------------------------------------------------------------------
/**
 * The AI state machine.  This is used by AIUpdate to implement all of the
 * commands in the AICommandInterface.
 */
class TurretStateMachine : public StateMachine
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( TurretStateMachine, "TurretStateMachine" );

public:
	/**
	 * The implementation of this constructor defines the states
	 * used by this machine.
	 */
	TurretStateMachine( TurretAI* tai, Object* owner, AsciiString name );

	TurretAI* getTurretAI() const { return m_turretAI; }

	virtual void clear();
	virtual StateReturnType resetToDefaultState();
	virtual StateReturnType setState( StateID newStateID );

private:
	TurretAI* m_turretAI;

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
};

//-----------------------------------------------------------------------------------------------------------
class TurretState : public State
{
	MEMORY_POOL_GLUE_ABC(TurretState)
protected:
	TurretState( TurretStateMachine* machine, AsciiString name ) : State( machine, name) { }
	TurretAI* getTurretAI() { return ((TurretStateMachine*)getMachine())->getTurretAI(); }
};
EMPTY_DTOR(TurretState)

//-----------------------------------------------------------------------------------------------------------
class TurretAIIdleState : public TurretState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(TurretAIIdleState, "TurretAIIdleState")
public:
	TurretAIIdleState( TurretStateMachine* machine ) : TurretState( machine, "TurretAIIdleState"), m_nextIdleScan(0) { }
	virtual StateReturnType onEnter();
	virtual StateReturnType update();

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
private:
	void resetIdleScan();
	UnsignedInt m_nextIdleScan;
};
EMPTY_DTOR(TurretAIIdleState)

//-----------------------------------------------------------------------------------------------------------
class TurretAIIdleScanState : public TurretState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(TurretAIIdleScanState, "TurretAIIdleScanState")
public:
	TurretAIIdleScanState( TurretStateMachine* machine ) : TurretState( machine, "TurretAIIdleScanState"), m_desiredAngle(0) { }
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
private:
	Real m_desiredAngle;
};
EMPTY_DTOR(TurretAIIdleScanState)

//-----------------------------------------------------------------------------------------------------------
/**
 * Aim a turret at GoalObject
 */
class TurretAIAimTurretState : public TurretState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(TurretAIAimTurretState, "TurretAIAimTurretState")
private:
#ifdef INTER_TURRET_DELAY
	UnsignedInt m_extraDelay;
#endif
public:
	TurretAIAimTurretState( TurretStateMachine* machine ) : TurretState( machine, "TurretAIAimTurretState" )
	{

	}
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();
protected:
	// snapshot interface	STUBBED - no member vars to save. jba.
	virtual void crc( Xfer *xfer ){};
	virtual void xfer( Xfer *xfer ){XferVersion cv = 1;	XferVersion v = cv; xfer->xferVersion( &v, cv );}
	virtual void loadPostProcess(){};
};
EMPTY_DTOR(TurretAIAimTurretState)

//-----------------------------------------------------------------------------------------------------------
/**
 * Rotate a turret back to its default position
 */
class TurretAIRecenterTurretState : public TurretState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(TurretAIRecenterTurretState, "TurretAIRecenterTurretState")
public:
	TurretAIRecenterTurretState( TurretStateMachine* machine ) : TurretState( machine, "TurretAIRecenterTurretState" ) { }
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();
protected:
	// snapshot interface	STUBBED - no member vars to save. jba.
	virtual void crc( Xfer *xfer ){};
	virtual void xfer( Xfer *xfer ){XferVersion cv = 1;	XferVersion v = cv; xfer->xferVersion( &v, cv );}
	virtual void loadPostProcess(){};
};
EMPTY_DTOR(TurretAIRecenterTurretState)

//-----------------------------------------------------------------------------------------------------------
/**
 * Hold turret position for a bit before recenter
 */
class TurretAIHoldTurretState : public TurretState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(TurretAIHoldTurretState, "TurretAIHoldTurretState")
private:
	UnsignedInt m_timestamp;										///< frame this state was last entered
public:
	TurretAIHoldTurretState( TurretStateMachine* machine ) : TurretState( machine , "AIHoldTurretState")
	{
		m_timestamp = 0;
	}
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
};
EMPTY_DTOR(TurretAIHoldTurretState)

//-------------------------------------------------------------------------------------------------
class TurretAIData : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(TurretAIData, "TurretAIData")
public:
	Real						m_turnRate;
	Real						m_pitchRate;
	Real						m_naturalTurretAngle;
	Real						m_naturalTurretPitch;
	Real						m_turretFireAngleSweep[WEAPONSLOT_COUNT];	///< if nonzero, sweep within +/- this angle range while firing
	Real						m_turretSweepSpeedModifier[WEAPONSLOT_COUNT];	///< While sweeping, change your speed by this
	Real						m_firePitch;						///< if nonzero, I am on target at this fixed pitch, not when pointing at target
	Real						m_minPitch;							///< dependent on allowspitch. defaults to 0 (horizontal). The lowest pitch allowed (negative to allow pointing down of a high turret)
	Real						m_groundUnitPitch;			///< dependent on allowspitch. defaults to 0 (horizontal). The lowest pitch allowed when firing at ground units to give the weapon an arc.  jba
	UnsignedInt			m_turretWeaponSlots;		///< which WeaponSlots are controlled by this turret
#ifdef INTER_TURRET_DELAY
	UnsignedInt			m_interTurretDelay;			///< special-case for multiturret battleships
#endif
	Real						m_minIdleScanAngle;			///< max angle the turret can turn while idling
	Real						m_maxIdleScanAngle;			///< max angle the turret can turn while idling
	UnsignedInt			m_minIdleScanInterval;	///< min interval between idle scans
	UnsignedInt			m_maxIdleScanInterval;	///< max interval between idle scans
	UnsignedInt			m_recenterTime;					///< time to wait before recentering turret
	Bool						m_initiallyDisabled;		///< manually controlled and disabled.
	Bool						m_firesWhileTurning;    ///< so the firing state does not instantly expire the turning state
	Bool						m_isAllowsPitch;				///< This type of turret can pitch up and down as well as spin

	TurretAIData();
	static void buildFieldParse(MultiIniFieldParse& p);

	static void parseTurretSweep(INI* ini, void *instance, void *store, const void* userData);
	static void parseTurretSweepSpeed(INI* ini, void *instance, void *store, const void* userData);
};
EMPTY_DTOR(TurretAIData)

//-----------------------------------------------------------------------------------------------------
enum TurretTargetType CPP_11(: Int)
{
	TARGET_NONE,
	TARGET_OBJECT,
	TARGET_POSITION
};

//-----------------------------------------------------------------------------------------------------
/**
 * Turret behavior implementation.
 */
class TurretAI : public MemoryPoolObject, public Snapshot, public NotifyWeaponFiredInterface
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( TurretAI, "TurretAI" )

public:
	TurretAI(Object* owner, const TurretAIData* data, WhichTurretType tur);
	// virtual destructor prototype provided by memory pool declaration

	Real getTurretAngle() const { return m_angle; }
	Real getTurretPitch() const { return m_pitch; }
	Real getMinPitch() const { return m_data->m_minPitch; }
	Bool isAllowsPitch() const { return m_data->m_isAllowsPitch; }
	Real getTurnRate() const { return m_data->m_turnRate; }
	Real getNaturalTurretAngle() const { return m_data->m_naturalTurretAngle; }
	Real getPitchRate() const { return m_data->m_pitchRate; }
	Real getFirePitch() const { return m_data->m_firePitch; }
	Real getGroundUnitPitch() const { return m_data->m_groundUnitPitch; }
	Real getNaturalTurretPitch() const { return m_data->m_naturalTurretPitch;	}
	Real getTurretFireAngleSweepForWeaponSlot( WeaponSlotType slot ) const;
	Real getTurretSweepSpeedModifierForWeaponSlot( WeaponSlotType slot ) const;
	Real getMinIdleScanAngle() const { return m_data->m_minIdleScanAngle;	}
	Real getMaxIdleScanAngle() const { return m_data->m_maxIdleScanAngle;	}
	UnsignedInt getMinIdleScanInterval() const { return m_data->m_minIdleScanInterval;	}
	UnsignedInt getMaxIdleScanInterval() const { return m_data->m_maxIdleScanInterval;	}
	UnsignedInt getRecenterTime() const { return m_data->m_recenterTime;	}
	Object* getOwner() { return m_owner; }
	const Object* getOwner() const { return m_owner; }

	Bool isOwnersCurWeaponOnTurret() const;
	Bool isWeaponSlotOnTurret(WeaponSlotType wslot) const;
	Bool isAttackingObject() const { return m_target == TARGET_OBJECT; }
	Bool isForceAttacking() const { return m_isForceAttacking; }

	// this will cause the turret to continuously track the given victim.
	// passing null will allow the turret to revert back to "normal" position.
	void setTurretTargetObject(Object* o, Bool forceAttacking);
	void setTurretTargetPosition(const Coord3D* pos);
	void recenterTurret();
	Bool isTurretInNaturalPosition() const;

	void setTurretEnabled( Bool enabled );
	Bool isTurretEnabled() const { return m_enabled; }

	/**
		return true iff the turret is trying to aim at the victim, BUT not yet pointing in
		the right dir.
	*/
	Bool isTryingToAimAtTarget(const Object* victim) const;

	UpdateSleepTime updateTurretAI();			///< implement this module's behavior

	virtual void notifyFired();
	virtual void notifyNewVictimChosen(Object* victim);
	virtual const Coord3D* getOriginalVictimPos() const { return nullptr; }	// yes, we return nullptr here
	virtual Bool isWeaponSlotOkToFire(WeaponSlotType wslot) const;

	// these are only for use by the state machines... don't call them otherwise, please
	Bool friend_turnTowardsAngle(Real desiredAngle, Real rateModifier, Real relThresh);
	Bool friend_turnTowardsPitch(Real pitch, Real rateModifier);
	Bool friend_getPositiveSweep() const { return m_positiveSweep; }
	void friend_setPositiveSweep(Bool b) { m_positiveSweep = b; }
	Bool friend_isSweepEnabled() const;
	WhichTurretType friend_getWhichTurret() const { return m_whichTurret; }
#ifdef INTER_TURRET_DELAY
	UnsignedInt friend_getInterTurretDelay();
#endif
	Bool friend_isAnyWeaponInRangeOf(const Object* o) const;
	TurretTargetType friend_getTurretTarget(Object*& obj, Coord3D& pos) const;
	Bool friend_getTargetWasSetByIdleMood() const { return m_targetWasSetByIdleMood; }
	const Team* friend_getVictimInitialTeam() const { return m_victimInitialTeam; }
	void friend_checkForIdleMoodTarget();
	UnsignedInt friend_getNextIdleMoodTargetFrame() const;
	void friend_notifyStateMachineChanged();

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

private:


	void startRotOrPitchSound();					///< start turret rotation sound
	void stopRotOrPitchSound();						///< stop turret rotation sound
	void removeSelfAsTargeter();

#ifdef INTER_TURRET_DELAY
	void getOtherTurretWeaponInfo(Int& numSelf, Int& numSelfReloading, Int& numSelfReady, Int& numOther, Int& numOtherReloading, Int& numOtherReady) const;
#endif

	const TurretAIData*				m_data;
	const WhichTurretType			m_whichTurret;
	Object*										m_owner;
	TurretStateMachine*				m_turretStateMachine;							///< the state machine
	Real											m_angle;										///< angle of the turret
	Real											m_pitch;										///< pitch angle, if this supports it
	AudioEventRTS							m_turretRotOrPitchSound;		///< Sound of turret rotation
	UnsignedInt								m_enableSweepUntil;
	Team*											m_victimInitialTeam; // The team of the victim at the BEGINNING of the attack. If it changes, we may need to stop the attack.
	mutable TurretTargetType	m_target;
	UnsignedInt								m_continuousFireExpirationFrame;
	UnsignedInt								m_sleepUntil;

	Bool										m_playRotSound : 1;
	Bool										m_playPitchSound : 1;
	Bool										m_positiveSweep : 1;
	Bool										m_didFire : 1;
	Bool										m_enabled : 1;
	Bool										m_firesWhileTurning : 1;
	Bool										m_isForceAttacking : 1;
	mutable Bool						m_targetWasSetByIdleMood : 1;

};
