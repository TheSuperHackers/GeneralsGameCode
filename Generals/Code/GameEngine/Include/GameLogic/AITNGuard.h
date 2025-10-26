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

// FILE: AITNGuard.h
/*---------------------------------------------------------------------------*/
/* EA Pacific                                                                */
/* Confidential Information	                                                 */
/* Copyright (C) 2001 - All Rights Reserved                                  */
/* DO NOT DISTRIBUTE                                                         */
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  AITNGuard.h                                                     */
/* Created:    John K. McDonald, Jr., 3/29/2002                              */
/* Desc:       // Define Guard states for AI                                 */
/* Revision History:                                                         */
/*		3/29/2002 : Initial creation                                           */
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
	AI_TN_GUARD_INNER = 5000,					///< Attack anything within this area till death
	AI_TN_GUARD_IDLE,									///< Wait till something shows up to attack.
	AI_TN_GUARD_OUTER,									///< Attack anything within this area that has been aggressive, until the timer expires
	AI_TN_GUARD_RETURN,								///< Restore to a position within the inner circle
	AI_TN_GUARD_GET_CRATE,							///< Pick up a crate from an enemy we killed.
	AI_TN_GUARD_ATTACK_AGGRESSOR,			///< Attack something that attacked me (that I can attack)
};

//--------------------------------------------------------------------------------------
class TunnelNetworkExitConditions : public AttackExitConditionsInterface
{
public:

	UnsignedInt			m_attackGiveUpFrame;		// frame at which we give up (if using)

	TunnelNetworkExitConditions() : m_attackGiveUpFrame(0)
	{
	}

	virtual Bool shouldExit(const StateMachine* machine) const;
};


// FORWARD DECLARATIONS ///////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------
class AITNGuardMachine : public StateMachine
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( AITNGuardMachine, "AITNGuardMachinePool" );

private:
	Coord3D									m_positionToGuard;
	ObjectID								m_nemesisToAttack;
	GuardMode								m_guardMode;

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
	AITNGuardMachine( Object *owner );

	const Coord3D *getPositionToGuard( void ) const { return &m_positionToGuard; }
	void setTargetPositionToGuard( const Coord3D *pos) { m_positionToGuard = *pos; }

	void setNemesisID(ObjectID id) { m_nemesisToAttack = id; }
	ObjectID getNemesisID() const { return m_nemesisToAttack; }

	GuardMode getGuardMode() const { return m_guardMode; }
	void setGuardMode(GuardMode guardMode) { m_guardMode = guardMode; }

	Bool lookForInnerTarget(void);

	static Real getStdGuardRange(const Object* obj);
};

//--------------------------------------------------------------------------------------
class AITNGuardInnerState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AITNGuardInnerState, "AITNGuardInnerState")
public:
	AITNGuardInnerState( StateMachine *machine ) : State( machine, "AITNGuardInner" )
	{
		m_attackState = NULL;
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
	AITNGuardMachine* getGuardMachine() { return (AITNGuardMachine*)getMachine(); }

	TunnelNetworkExitConditions m_exitConditions;
	Bool			m_scanForEnemy;
	AIAttackState *m_attackState;
};

//--------------------------------------------------------------------------------------
class AITNGuardIdleState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AITNGuardIdleState, "AITNGuardIdleState")
public:
	AITNGuardIdleState( StateMachine *machine ) : State( machine, "AITNGuardIdleState" ) { }
	virtual StateReturnType onEnter( void );
	virtual StateReturnType update( void );
	virtual void onExit( StateExitType status );
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
private:
	AITNGuardMachine* getGuardMachine() { return (AITNGuardMachine*)getMachine(); }

	UnsignedInt m_nextEnemyScanTime;
	Coord3D			m_guardeePos;						///< Where the object we are guarding was last.
};
EMPTY_DTOR(AITNGuardIdleState)

//--------------------------------------------------------------------------------------
class AITNGuardOuterState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AITNGuardOuterState, "AITNGuardOuterState")
public:
	AITNGuardOuterState( StateMachine *machine ) : State( machine, "AITNGuardOuter" )
	{
		m_attackState = NULL;
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
	AITNGuardMachine* getGuardMachine() { return (AITNGuardMachine*)getMachine(); }

	TunnelNetworkExitConditions m_exitConditions;
	AIAttackState *m_attackState;
};

//--------------------------------------------------------------------------------------
class AITNGuardReturnState : public AIEnterState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AITNGuardReturnState, "AITNGuardReturnState")
private:
	AITNGuardMachine* getGuardMachine() { return (AITNGuardMachine*)getMachine(); }
public:
	AITNGuardReturnState( StateMachine *machine ) : AIEnterState(machine )
	{
		m_nextReturnScanTime = 0;
	}
	virtual StateReturnType onEnter( void );
	virtual StateReturnType update( void );
	virtual void onExit( StateExitType status );

protected:
	UnsignedInt m_nextReturnScanTime;

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
};
EMPTY_DTOR(AITNGuardReturnState)


//--------------------------------------------------------------------------------------
class AITNGuardPickUpCrateState : public AIPickUpCrateState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AITNGuardPickUpCrateState, "AITNGuardPickUpCrateState")
public:
	AITNGuardPickUpCrateState( StateMachine *machine );
	virtual StateReturnType onEnter( void );
	virtual StateReturnType update( void );
	virtual void onExit( StateExitType status );
};
EMPTY_DTOR(AITNGuardPickUpCrateState)

//--------------------------------------------------------------------------------------
class AITNGuardAttackAggressorState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AITNGuardAttackAggressorState, "AITNGuardAttackAggressorState")
public:
	AITNGuardAttackAggressorState( StateMachine *machine );
	virtual StateReturnType onEnter( void );
	virtual StateReturnType update( void );
	virtual void onExit( StateExitType status );
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
private:
	AITNGuardMachine* getGuardMachine() { return (AITNGuardMachine*)getMachine(); }
	TunnelNetworkExitConditions m_exitConditions;
	AIAttackState *m_attackState;
};

//--------------------------------------------------------------------------------------
