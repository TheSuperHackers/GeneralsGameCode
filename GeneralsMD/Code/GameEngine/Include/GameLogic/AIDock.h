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

// AIDock.h
// Docking behavior
// Author: Michael S. Booth, February 2002

#pragma once

#include "Common/GameMemory.h"
#include "GameLogic/AIStateMachine.h"

/**
 * The states of the Docking state machine.
 */
enum
{
	AI_DOCK_APPROACH,									///< given a queue pos, move to it
	AI_DOCK_WAIT_FOR_CLEARANCE,				///< wait for dock to give us enter clearance
	AI_DOCK_ADVANCE_POSITION,					///< Advance in approach position as line moves forward
	AI_DOCK_MOVE_TO_ENTRY,						///< move to the dock entrance
	AI_DOCK_MOVE_TO_DOCK,							///< move to the actual dock position
	AI_DOCK_PROCESS_DOCK,							///< invoke the dock's action until it is done
	AI_DOCK_MOVE_TO_EXIT,							///< move to the dock exit, can exit the dock machine
	AI_DOCK_MOVE_TO_RALLY						  ///< Move to rally if desired, exit the dock machine no matter what
};

//-----------------------------------------------------------------------------------------------------------
/**
 * The docking state machine.
 */
class AIDockMachine : public StateMachine
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( AIDockMachine, "AIDockMachinePool" );

public:
	/**
	 * The implementation of this constructor defines the states
	 * used by this machine.
	 */
	AIDockMachine( Object *owner );

	static Bool ableToAdvance( State *thisState, void* userData ); // Condition for scooting forward in line while waiting
	virtual void halt(void); ///< Stops the state machine & disables it in preparation for deleting it.

	Int m_approachPosition;	///< The Approach Position I am holding, to make scoot forward checks quicker.

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

};

// Please do not use these states in some other machine.  I know that wouldn't even make sense, but they
// cast their getMachine to an AIDock machine to store stuff across states so you'd crash.
//-----------------------------------------------------------------------------------------------------------
class AIDockApproachState : public AIInternalMoveToState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIDockApproachState, "AIDockApproachState")
public:
	AIDockApproachState( StateMachine *machine ) : AIInternalMoveToState( machine, "AIDockApproachState" ) { }
	virtual StateReturnType onEnter( void );
	virtual void onExit( StateExitType status );
	virtual StateReturnType update( void );
protected:
	// snapshot interface STUBBED.
	virtual void crc( Xfer *xfer ){};
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess(){};
};
EMPTY_DTOR(AIDockApproachState)

//-----------------------------------------------------------------------------------------------------------
class AIDockWaitForClearanceState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIDockWaitForClearanceState, "AIDockWaitForClearanceState")
public:
	AIDockWaitForClearanceState( StateMachine *machine ) : State( machine, "AIDockWaitForClearanceState" ), m_enterFrame(0) { }
	virtual StateReturnType onEnter( void );
	virtual StateReturnType update( void );
	virtual void onExit( StateExitType status );
protected:
	UnsignedInt m_enterFrame;
protected:
	// snapshot interface STUBBED.
	virtual void crc( Xfer *xfer ){};
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess(){};
};
EMPTY_DTOR(AIDockWaitForClearanceState)

//-----------------------------------------------------------------------------------------------------------
class AIDockAdvancePositionState : public AIInternalMoveToState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIDockAdvancePositionState, "AIDockAdvancePositionState")
public:
	AIDockAdvancePositionState( StateMachine *machine ) : AIInternalMoveToState( machine, "AIDockApproachState" ) { }
	virtual StateReturnType onEnter( void );
	virtual void onExit( StateExitType status );
	virtual StateReturnType update( void );
};
EMPTY_DTOR(AIDockAdvancePositionState)

//-----------------------------------------------------------------------------------------------------------
class AIDockMoveToEntryState : public AIInternalMoveToState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIDockMoveToEntryState, "AIDockMoveToEntryState")
public:
	AIDockMoveToEntryState( StateMachine *machine ) : AIInternalMoveToState( machine, "AIDockMoveToEntryState" ) { }
	virtual StateReturnType onEnter( void );
	virtual void onExit( StateExitType status );
	virtual StateReturnType update( void );
};
EMPTY_DTOR(AIDockMoveToEntryState)

//-----------------------------------------------------------------------------------------------------------
class AIDockMoveToDockState : public AIInternalMoveToState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIDockMoveToDockState, "AIDockMoveToDockState")
public:
	AIDockMoveToDockState( StateMachine *machine ) : AIInternalMoveToState( machine, "AIDockMoveToDockState" ) { }
	virtual StateReturnType onEnter( void );
	virtual void onExit( StateExitType status );
	virtual StateReturnType update( void );
};
EMPTY_DTOR(AIDockMoveToDockState)

//-----------------------------------------------------------------------------------------------------------
class AIDockMoveToRallyState : public AIInternalMoveToState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIDockMoveToRallyState, "AIDockMoveToRallyState")
public:
	AIDockMoveToRallyState( StateMachine *machine ) : AIInternalMoveToState( machine, "AIDockMoveToRallyState" ) { }
	virtual StateReturnType onEnter( void );
	virtual void onExit( StateExitType status );
	virtual StateReturnType update( void );
};
EMPTY_DTOR(AIDockMoveToRallyState)

//-----------------------------------------------------------------------------------------------------------
class AIDockProcessDockState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIDockProcessDockState, "AIDockProcessDockState")
public:
	AIDockProcessDockState( StateMachine *machine );
	virtual StateReturnType onEnter( void );
	virtual void onExit( StateExitType status );
	virtual StateReturnType update( void );

	void setNextDockActionFrame();//This puts a delay between callings of Action to tweak the speed of docking.
	UnsignedInt m_nextDockActionFrame;// In the unlikely event of saving a game in the middle of docking, you may
	// complete a Action a few frames sooner than you would have: It does not need to be saved.
	Object* findMyDrone();

protected:
	// snapshot interface STUBBED.
	virtual void crc( Xfer *xfer ){};
	virtual void xfer( Xfer *xfer ){XferVersion cv = 1;	XferVersion v = cv; xfer->xferVersion( &v, cv );}
	virtual void loadPostProcess(){};

private:
	ObjectID m_droneID;			///< If I have a drone, the drone will get repaired too.

};
EMPTY_DTOR(AIDockProcessDockState)

//-----------------------------------------------------------------------------------------------------------
class AIDockMoveToExitState : public AIInternalMoveToState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIDockMoveToExitState, "AIDockMoveToExitState")
public:
	AIDockMoveToExitState( StateMachine *machine ) : AIInternalMoveToState( machine, "AIDockMoveToExitState" ) { }
	virtual StateReturnType onEnter( void );
	virtual void onExit( StateExitType status );
	virtual StateReturnType update( void );
};
EMPTY_DTOR(AIDockMoveToExitState)

//-----------------------------------------------------------------------------------------------------------
