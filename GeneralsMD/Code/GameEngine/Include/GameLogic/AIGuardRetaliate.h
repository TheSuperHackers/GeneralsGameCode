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

// FILE: AIGuardRetaliate.h
/*---------------------------------------------------------------------------*/
/* Electronic Arts Los Angeles                                               */
/* Confidential Information	                                                 */
/* Copyright (C) 2003 - All Rights Reserved                                  */
/* DO NOT DISTRIBUTE                                                         */
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  AIGuardRetaliate.h                                            */
/* Created:    Kris Morness July 2003                                        */
/* Desc:       // Define Guard Retaliation states for AI                     */
/*---------------------------------------------------------------------------*/

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////
#include "Common/GameMemory.h"
#include "GameLogic/AIStateMachine.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/AI.h"

// DEFINES ////////////////////////////////////////////////////////////////////
// TYPE DEFINES ///////////////////////////////////////////////////////////////
enum
{
	// prevent collisions with other states that we might use, (namely AI_IDLE)
	AI_GUARD_RETALIATE_INNER = 5000,					///< Attack anything within this area till death
	AI_GUARD_RETALIATE_IDLE,									///< Wait till something shows up to attack.
	AI_GUARD_RETALIATE_OUTER,									///< Attack anything within this area that has been aggressive, until the timer expires
	AI_GUARD_RETALIATE_RETURN,								///< Restore to a position within the inner circle
	AI_GUARD_RETALIATE_GET_CRATE,							///< Pick up a crate from an enemy we killed.
	AI_GUARD_RETALIATE_ATTACK_AGGRESSOR,			///< Attack something that attacked me (that I can attack)
};

//--------------------------------------------------------------------------------------
class GuardRetaliateExitConditions : public AttackExitConditionsInterface
{
public:

	enum ExitConditionsEnum
	{
		ATTACK_ExitIfOutsideRadius		= 0x01,
		ATTACK_ExitIfExpiredDuration	= 0x02,
		ATTACK_ExitIfNoUnitFound			= 0x04
	};

	Int							m_conditionsToConsider;
	Coord3D					m_center;								// can be updated at any time by owner
	Real						m_radiusSqr;						// can be updated at any time by owner
	UnsignedInt			m_attackGiveUpFrame;		// frame at which we give up (if using)

	GuardRetaliateExitConditions() : m_attackGiveUpFrame(0), m_conditionsToConsider(0), m_radiusSqr(0.0f)
	{
		m_center.zero();
	}

	virtual Bool shouldExit(const StateMachine* machine) const;
};


// FORWARD DECLARATIONS ///////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------
class AIGuardRetaliateMachine : public StateMachine
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( AIGuardRetaliateMachine, "AIGuardRetaliateMachinePool" );

private:
	Coord3D									m_positionToGuard;
	ObjectID								m_nemesisToAttack;

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

public:
	/**
	 * The implementation of this constructor defines the states
	 * used by this machine.
	 */
	AIGuardRetaliateMachine( Object *owner );

	virtual Bool isIdle() const;

	const Coord3D *getPositionToGuard( void ) const { return &m_positionToGuard; }
	void setTargetPositionToGuard( const Coord3D *pos) { m_positionToGuard = *pos; }

	void setNemesisID(ObjectID id) { m_nemesisToAttack = id; }
	ObjectID getNemesisID() const { return m_nemesisToAttack; }

	Bool lookForInnerTarget(void);

	static Real getStdGuardRange(const Object* obj);
};

//--------------------------------------------------------------------------------------
class AIGuardRetaliateInnerState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIGuardRetaliateInnerState, "AIGuardRetaliateInnerState")
public:
	AIGuardRetaliateInnerState( StateMachine *machine ) : State( machine, "AIGuardRetaliateInner" )
	{
		m_attackState = 0;
		m_enterState = 0;
	}
	virtual StateReturnType onEnter( void );
	virtual StateReturnType update( void );
	virtual void onExit( StateExitType status );
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
private:
	AIGuardRetaliateMachine* getGuardMachine() { return (AIGuardRetaliateMachine*)getMachine(); }

	GuardRetaliateExitConditions m_exitConditions;
	AIAttackState *m_attackState;
	AIEnterState *m_enterState;
};

//--------------------------------------------------------------------------------------
class AIGuardRetaliateIdleState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIGuardRetaliateIdleState, "AIGuardRetaliateIdleState")
public:
	AIGuardRetaliateIdleState( StateMachine *machine ) : State( machine, "AIGuardRetaliateIdleState" ) { }
	virtual StateReturnType onEnter( void );
	virtual StateReturnType update( void );
	virtual void onExit( StateExitType status );
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
private:
	AIGuardRetaliateMachine* getGuardMachine() { return (AIGuardRetaliateMachine*)getMachine(); }

	UnsignedInt m_nextEnemyScanTime;
	Coord3D			m_guardeePos;						///< Where the object we are guarding was last.
};
EMPTY_DTOR(AIGuardRetaliateIdleState)

//--------------------------------------------------------------------------------------
class AIGuardRetaliateOuterState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIGuardRetaliateOuterState, "AIGuardRetaliateOuterState")
public:
	AIGuardRetaliateOuterState( StateMachine *machine ) : State( machine, "AIGuardRetaliateOuter" )
	{
		m_attackState = nullptr;
	}
	virtual StateReturnType onEnter( void );
	virtual StateReturnType update( void );
	virtual void onExit( StateExitType status );
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
private:
	AIGuardRetaliateMachine* getGuardMachine() { return (AIGuardRetaliateMachine*)getMachine(); }

	GuardRetaliateExitConditions m_exitConditions;
	AIAttackState *m_attackState;
};

//--------------------------------------------------------------------------------------
class AIGuardRetaliateReturnState : public AIInternalMoveToState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIGuardRetaliateReturnState, "AIGuardRetaliateReturnState")
private:
	AIGuardRetaliateMachine* getGuardMachine() { return (AIGuardRetaliateMachine*)getMachine(); }
public:
	AIGuardRetaliateReturnState( StateMachine *machine ) : AIInternalMoveToState( machine, "AIGuardRetaliateReturn" )
	{
		m_nextReturnScanTime = 0;
	}
	virtual StateReturnType onEnter( void );
	virtual StateReturnType update( void );
	virtual void onExit( StateExitType status );
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
private:
	UnsignedInt m_nextReturnScanTime;
};
EMPTY_DTOR(AIGuardRetaliateReturnState)


//--------------------------------------------------------------------------------------
class AIGuardRetaliatePickUpCrateState : public AIPickUpCrateState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIGuardRetaliatePickUpCrateState, "AIGuardRetaliatePickUpCrateState")
public:
	AIGuardRetaliatePickUpCrateState( StateMachine *machine );
	virtual StateReturnType onEnter( void );
	virtual StateReturnType update( void );
	virtual void onExit( StateExitType status );
};
EMPTY_DTOR(AIGuardRetaliatePickUpCrateState)

//--------------------------------------------------------------------------------------
class AIGuardRetaliateAttackAggressorState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIGuardRetaliateAttackAggressorState, "AIGuardRetaliateAttackAggressorState")
public:
	AIGuardRetaliateAttackAggressorState( StateMachine *machine );
	virtual StateReturnType onEnter( void );
	virtual StateReturnType update( void );
	virtual void onExit( StateExitType status );
#ifdef STATE_MACHINE_DEBUG
	virtual AsciiString getName() const ;
#endif
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
private:
	AIGuardRetaliateMachine* getGuardMachine() { return (AIGuardRetaliateMachine*)getMachine(); }
	GuardRetaliateExitConditions m_exitConditions;
	AIAttackState *m_attackState;
};

//--------------------------------------------------------------------------------------
