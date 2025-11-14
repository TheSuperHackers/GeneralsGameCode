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

// FILE: AIGuard.h
/*---------------------------------------------------------------------------*/
/* EA Pacific                                                                */
/* Confidential Information	                                                 */
/* Copyright (C) 2001 - All Rights Reserved                                  */
/* DO NOT DISTRIBUTE                                                         */
/*---------------------------------------------------------------------------*/
/* Project:    RTS3                                                          */
/* File name:  AIGuard.h                                                     */
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
	AI_GUARD_INNER = 5000,					///< Attack anything within this area till death
	AI_GUARD_IDLE,									///< Wait till something shows up to attack.
	AI_GUARD_OUTER,									///< Attack anything within this area that has been aggressive, until the timer expires
	AI_GUARD_RETURN,								///< Restore to a position within the inner circle
	AI_GUARD_GET_CRATE,							///< Pick up a crate from an enemy we killed.
	AI_GUARD_ATTACK_AGGRESSOR,			///< Attack something that attacked me (that I can attack)
};

//--------------------------------------------------------------------------------------
class ExitConditions : public AttackExitConditionsInterface
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

	ExitConditions() : m_attackGiveUpFrame(0), m_conditionsToConsider(0), m_radiusSqr(0.0f)
	{
		//Added By Sadullah Nader
		// Initializations missing and needed
		m_center.zero();
	}

	virtual Bool shouldExit(const StateMachine* machine) const;
};


// FORWARD DECLARATIONS ///////////////////////////////////////////////////////

//--------------------------------------------------------------------------------------
class AIGuardMachine : public StateMachine
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( AIGuardMachine, "AIGuardMachinePool" );

private:
	ObjectID								m_targetToGuard;
	const PolygonTrigger *	m_areaToGuard;
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
	AIGuardMachine( Object *owner );
	Object* findTargetToGuardByID( void ) { return TheGameLogic->findObjectByID(m_targetToGuard); }
	void setTargetToGuard( const Object *object ) { m_targetToGuard = object ? object->getID() : INVALID_ID; }

	const Coord3D *getPositionToGuard( void ) const { return &m_positionToGuard; }
	void setTargetPositionToGuard( const Coord3D *pos) { m_positionToGuard = *pos; }

	const PolygonTrigger *getAreaToGuard( void ) const { return m_areaToGuard; }
	void setAreaToGuard( const PolygonTrigger *area) { m_areaToGuard = area; }

	void setNemesisID(ObjectID id) { m_nemesisToAttack = id; }
	ObjectID getNemesisID() const { return m_nemesisToAttack; }

	GuardMode getGuardMode() const { return m_guardMode; }
	void setGuardMode(GuardMode guardMode) { m_guardMode = guardMode; }

	Bool lookForInnerTarget(void);

	static Real getStdGuardRange(const Object* obj);
};

//--------------------------------------------------------------------------------------
class AIGuardInnerState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIGuardInnerState, "AIGuardInnerState")
public:
	AIGuardInnerState( StateMachine *machine ) : State( machine, "AIGuardInner" )
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
	AIGuardMachine* getGuardMachine() { return (AIGuardMachine*)getMachine(); }

	ExitConditions m_exitConditions;
	AIAttackState *m_attackState;
};

//--------------------------------------------------------------------------------------
class AIGuardIdleState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIGuardIdleState, "AIGuardIdleState")
public:
	AIGuardIdleState( StateMachine *machine ) : State( machine, "AIGuardIdleState" ) { }
	virtual StateReturnType onEnter( void );
	virtual StateReturnType update( void );
	virtual void onExit( StateExitType status );
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
private:
	AIGuardMachine* getGuardMachine() { return (AIGuardMachine*)getMachine(); }

	UnsignedInt m_nextEnemyScanTime;
	Coord3D			m_guardeePos;						///< Where the object we are guarding was last.
};
EMPTY_DTOR(AIGuardIdleState)

//--------------------------------------------------------------------------------------
class AIGuardOuterState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIGuardOuterState, "AIGuardOuterState")
public:
	AIGuardOuterState( StateMachine *machine ) : State( machine, "AIGuardOuter" )
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
	AIGuardMachine* getGuardMachine() { return (AIGuardMachine*)getMachine(); }

	ExitConditions m_exitConditions;
	AIAttackState *m_attackState;
};

//--------------------------------------------------------------------------------------
class AIGuardReturnState : public AIInternalMoveToState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIGuardReturnState, "AIGuardReturnState")
private:
	AIGuardMachine* getGuardMachine() { return (AIGuardMachine*)getMachine(); }
public:
	AIGuardReturnState( StateMachine *machine ) : AIInternalMoveToState( machine, "AIGuardReturn" )
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
EMPTY_DTOR(AIGuardReturnState)


//--------------------------------------------------------------------------------------
class AIGuardPickUpCrateState : public AIPickUpCrateState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIGuardPickUpCrateState, "AIGuardPickUpCrateState")
public:
	AIGuardPickUpCrateState( StateMachine *machine );
	virtual StateReturnType onEnter( void );
	virtual StateReturnType update( void );
	virtual void onExit( StateExitType status );
};
EMPTY_DTOR(AIGuardPickUpCrateState)

//--------------------------------------------------------------------------------------
class AIGuardAttackAggressorState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIGuardAttackAggressorState, "AIGuardAttackAggressorState")
public:
	AIGuardAttackAggressorState( StateMachine *machine );
	virtual StateReturnType onEnter( void );
	virtual StateReturnType update( void );
	virtual void onExit( StateExitType status );
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
private:
	AIGuardMachine* getGuardMachine() { return (AIGuardMachine*)getMachine(); }
	ExitConditions m_exitConditions;
	AIAttackState *m_attackState;
};

//--------------------------------------------------------------------------------------
