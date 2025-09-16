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

// AIStateMachine.h
// Finite state machine encapsulation
// Author: Michael S. Booth, January 2002

#pragma once

#ifndef _AI_STATE_MACHINE_H_
#define _AI_STATE_MACHINE_H_

#include "Lib/BaseType.h"

#include "Common/AudioEventRTS.h"
#include "Common/GameMemory.h"
#include "Common/StateMachine.h"

#include "GameLogic/TerrainLogic.h"

// FORWARD DECLARATIONS ///////////////////////////////////////////////////////////////////////////
class AIGuardMachine;
class AIGuardRetaliateMachine;
class AITNGuardMachine;
class Weapon;
class Team;
class AIAttackState;
class AttackStateMachine;
class AIGroup;
class Squad;

//-----------------------------------------------------------------------------------------------------------


/**
 * The AI state IDs.
 * Each of these constants will be associated with an instance of a State class
 * in a given StateMachine.
 */
enum AIStateType CPP_11(: Int)
{
	AI_IDLE,
	AI_MOVE_TO,																///< move to the GoalObject or GoalPosition
	AI_FOLLOW_WAYPOINT_PATH_AS_TEAM,					///< follow a waypoint path as a team
	AI_FOLLOW_WAYPOINT_PATH_AS_INDIVIDUALS,		///< follow a waypoint path
	AI_FOLLOW_WAYPOINT_PATH_AS_TEAM_EXACT,		///< follow a waypoint path as a team
	AI_FOLLOW_WAYPOINT_PATH_AS_INDIVIDUALS_EXACT,///< follow a waypoint path
	AI_FOLLOW_PATH,														///< follow a simple list of points
	AI_FOLLOW_EXITPRODUCTION_PATH,						///< the same, but only when exiting production facility
	AI_WAIT,
	AI_ATTACK_POSITION,												///< attempt to kill GoalPosition
	AI_ATTACK_OBJECT,													///< attempt to kill GoalObject
	AI_FORCE_ATTACK_OBJECT,										///< attempt to kill GoalObject, force fire on it.
	AI_ATTACK_AND_FOLLOW_OBJECT,							///< attempt to kill GoalObject, following it if necessary (and possible)
	AI_DEAD,
	AI_DOCK,																	///< dock with GoalObject, if GoalObject has a DockUpdate module
	AI_ENTER,																	///< move to GoalObject and "enter" it when close
	AI_GUARD,																	///< guard your current location
	AI_HUNT,																	///< seek and destroy behavior
	AI_WANDER,																///< Wander around following a waypoint path.
	AI_PANIC,																	///< Run around screaming following a waypoint path.
	AI_ATTACK_SQUAD,													///< Set the unit to attempt to kill all objects in goalSquad.
	AI_GUARD_TUNNEL_NETWORK,									///< Guard from inside a tunnel network.
	AI_GET_REPAIRED,													///< Get repaired at a repair depot
	AI_MOVE_OUT_OF_THE_WAY,										///< Move out of the way of another unit.
	AI_MOVE_AND_TIGHTEN,											///< Move in order to tighten up a formation.
	AI_MOVE_AND_EVACUATE,											///< Move to, then empty transport.
	AI_MOVE_AND_EVACUATE_AND_EXIT,						///< Move to, then empty transport.
	AI_MOVE_AND_DELETE,												///< Move to, then delete self.
	AI_ATTACK_AREA,														///< Attack units in an area.
	AI_HACK_INTERNET,													///< Hack internet for free money (no target required).
	AI_ATTACK_MOVE_TO,												///< Attack-move to a location
	AI_ATTACKFOLLOW_WAYPOINT_PATH_AS_INDIVIDUALS,	///< Attack-Follow down a waypoint path as individuals
	AI_ATTACKFOLLOW_WAYPOINT_PATH_AS_TEAM,				///< Attack-Follow down a waypoint path as a team
	AI_FACE_OBJECT,
	AI_FACE_POSITION,
	AI_RAPPEL_INTO,														/**< rappel from current pos down to target object.
																								if target is building, will enter and kill lots of folks.
																								if target is null, will rappel to ground. */
	AI_COMBATDROP,														/**< attempt to send AI_RAPPEL_INTO to contents. */
	AI_EXIT,																	///< exit the obj, waiting if necessary
	AI_PICK_UP_CRATE,													///< Pick up a crate created by a kill.  jba.
	AI_MOVE_AWAY_FROM_REPULSORS,							///< Civilians are running away from repulsors. (enemies or dead civs, usually) jba
	AI_WANDER_IN_PLACE,												///< Civilians just wander around a spot, rather than along a path.
	AI_BUSY,																	///< This is a state that things will be in when they are busy doing random stuff that doesn't require AI interaction.
	AI_EXIT_INSTANTLY,												///< exit this obj, without waiting -- do it in the onEnter! This frame!
	AI_GUARD_RETALIATE,												///< attacks attacker but with restrictions (hybrid of attack and guard).
};

//-----------------------------------------------------------------------------------------------------------
// generically state transition conditions
extern Bool outOfWeaponRangeObject( State *thisState, void* userData );
extern Bool outOfWeaponRangePosition( State *thisState, void* userData );
extern Bool wantToSquishTarget( State *thisState, void* userData );

//-----------------------------------------------------------------------------------------------------------
/**
  The AI state machine.  This is used by AIUpdate to implement all of the
  commands in the AICommandInterface.

	NOTE NOTE NOTE NOTE NOTE

	Do NOT subclass this unless you want ALL of the states this machine possesses.
	If you only want SOME of the states, please make a new StateMachine, descended
	from StateMachine, NOT AIStateMachine. Thank you. (srj)

	NOTE NOTE NOTE NOTE NOTE

 */
class AIStateMachine : public StateMachine
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( AIStateMachine, "AIStateMachine" );

public:
	/**
	 * The implementation of this constructor defines the states
	 * used by this machine.
	 */
	AIStateMachine( Object *owner, AsciiString name );

	virtual void clear();
	virtual StateReturnType resetToDefaultState();
	virtual StateReturnType setState( StateID newStateID );

	/// @todo Rethink state parameter passing. Continuing in this fashion will have a pile of params in the machine (MSB)
	void setGoalPath( const std::vector<Coord3D>* path );
	void addToGoalPath( const Coord3D *pathPoint );
	const Coord3D *getGoalPathPosition( Int i ) const;		///< return path position at index "i"
	Int getGoalPathSize() const { return m_goalPath.size(); }


	void setGoalWaypoint( const Waypoint *way );		///< move toward this waypoint, continue if connected
	const Waypoint *getGoalWaypoint();

	// All of these wind up storing a Squad, as it is the only one that can always be safely created.
	void setGoalTeam( const Team *team );
	void setGoalSquad( const Squad *squad );
	void setGoalAIGroup( const AIGroup *group );

	Squad *getGoalSquad(void);

	StateReturnType setTemporaryState( StateID newStateID, Int frameLimitCoount );			///< change the temporary state of the machine, and number of frames limit.
	StateID getTemporaryState(void) const {return m_temporaryState?m_temporaryState->getID():INVALID_STATE_ID;}

public:	// overrides.
	virtual StateReturnType updateStateMachine();				///< run one step of the machine
#ifdef STATE_MACHINE_DEBUG
	virtual AsciiString getCurrentStateName() const ;
#endif

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

private:
	std::vector<Coord3D>	m_goalPath;					///< defines a simple path to follow
	const Waypoint *			m_goalWaypoint;
	Squad *								m_goalSquad;

	/** A temporary state to run for a while (usually AI_MOVE_OUT_OF_THE_WAY).
	Doesn't clear or reset the state machine, so it goes back to doing what it was doing.  jba. */
	State									*m_temporaryState;
	UnsignedInt						m_temporaryStateFramEnd; ///< Last frame to run m_temporaryState.
};


//-----------------------------------------------------------------------------------------------------------
class AttackStateMachine : public StateMachine
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( AttackStateMachine, "AttackStateMachine" );

public:
// Attack states.
enum StateType CPP_11(: Int)
	{
		CHASE_TARGET,													///< Chase a moving target (optionally following it)
		APPROACH_TARGET,											///< Approach a non-moving target.
		AIM_AT_TARGET,												///< rotate to face GoalObject or GoalPosition
		FIRE_WEAPON,													///< fire the machine owner's current weapon
	};
	AttackStateMachine( Object *owner, AIAttackState* att, AsciiString name, Bool follow, Bool attackingObject, Bool forceAttacking );

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
};

//-----------------------------------------------------------------------------------------------------------
class AIIdleState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIIdleState, "AIIdleState")
private:

	UnsignedShort m_initialSleepOffset;
	Bool m_shouldLookForTargets;
	Bool m_inited;

	void doInitIdleState();

public:
	enum AIIdleTargetingType
	{
		LOOK_FOR_TARGETS,
		DO_NOT_LOOK_FOR_TARGETS
	};
	virtual Bool isIdle(void) const { return true; }
	AIIdleState( StateMachine *machine, AIIdleTargetingType shouldLookForTargets);
	virtual StateReturnType onEnter();
	virtual StateReturnType update();

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

};
EMPTY_DTOR(AIIdleState)

//-----------------------------------------------------------------------------------------------------------
/**
 * Basic pathfinding and moving to a goal position.
 * Not for direct use (hence no associated state ID), but for deriving from.
 */
class AIInternalMoveToState : public State
{
	MEMORY_POOL_GLUE_ABC(AIInternalMoveToState)
public:
	AIInternalMoveToState( StateMachine *machine, AsciiString name ) : State( machine, name )
	{
		m_goalPosition.zero();
		m_pathGoalPosition.zero();
		m_pathTimestamp = 0;
		m_blockedRepathTimestamp = 0;
		m_adjustDestinations = true;
		m_goalLayer = LAYER_INVALID;
	}

	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();

protected:

	void setAdjustsDestination(Bool b) { m_adjustDestinations = b; }
	Bool getAdjustsDestination() const;

	void setGoalPos(const Coord3D* pos) { m_goalPosition = *pos; }
	const Coord3D* getGoalPos() const { return &m_goalPosition; }

	virtual Bool computePath();												///< compute the path

	void forceRepath()
	{
		m_pathGoalPosition.x = m_pathGoalPosition.y = m_pathGoalPosition.z = -100.0f;
		m_pathTimestamp = -MIN_REPATH_TIME;
	}

private:
	void startMoveSound();

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

private:
	enum { MIN_REPATH_TIME = 10 };										///< minimum # of frames must elapse before re-pathing
protected:
	Coord3D							m_goalPosition;											///< the goal position to move to
	PathfindLayerEnum		m_goalLayer;										///< The layer we are moving towards.
	Coord3D				m_pathGoalPosition;									///< the position our current path leads to
private:
	AudioHandle		m_ambientPlayingHandle;							///< Audio handle for the looping sound that we may play.
	UnsignedInt		m_pathTimestamp;										///< time of last pathfind
	UnsignedInt		m_blockedRepathTimestamp;						///< time of last blocked pathfind
	Bool					m_adjustDestinations;								///< Adjust destinations to avoid stacking units on top of each other.  Normally true, but
																										//   occasionally false for things like car bombs.
protected:
	Bool					m_waitingForPath;										///< If we are waiting for a path.
	Bool					m_tryOneMoreRepath;									///< If true, after we complete movement do another compute path.
};
EMPTY_DTOR(AIInternalMoveToState)

//-----------------------------------------------------------------------------------------------------------
/**
 * Move to the GoalPosition, or GoalObject.
 */
class AIRappelState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIRappelState, "AIRappelState")
protected:
	Real m_rappelRate;
	Real m_destZ;
	Bool m_targetIsBldg;
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

public:
	AIRappelState( StateMachine *machine );
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();
};
EMPTY_DTOR(AIRappelState)

//-----------------------------------------------------------------------------------------------------------
/**
 * Spin. Busy-like
 */
class AIBusyState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIBusyState, "AIBusyState")
public:
	AIBusyState( StateMachine *machine ) : State( machine, "AIBusyState" ) { }
	virtual StateReturnType onEnter() { return STATE_CONTINUE; }
	virtual void onExit( StateExitType status ) { }
	virtual StateReturnType update() { return STATE_CONTINUE; }
	virtual Bool isBusy(void) const { return true; }
protected:
	// snapshot interface STUBBED.
	virtual void crc( Xfer *xfer ){};
	virtual void xfer( Xfer *xfer ){XferVersion cv = 1;	XferVersion v = cv; xfer->xferVersion( &v, cv );}
	virtual void loadPostProcess(){};

};
EMPTY_DTOR(AIBusyState)

//-----------------------------------------------------------------------------------------------------------
/**
 * Move to the GoalPosition, or GoalObject.
 */
class AIMoveToState : public AIInternalMoveToState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIMoveToState, "AIMoveToState")
protected:
	Bool m_isMoveTo;
public:
	AIMoveToState( StateMachine *machine );
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();
};
EMPTY_DTOR(AIMoveToState)

//-----------------------------------------------------------------------------------------------------------
/**
 * Moves out of the way of another object.
 */
class AIMoveOutOfTheWayState : public AIInternalMoveToState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIMoveOutOfTheWayState, "AIMoveOutOfTheWayState")
public:
	AIMoveOutOfTheWayState( StateMachine *machine ) : AIInternalMoveToState( machine, "AIMoveOutOfTheWayState" ) { }
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();
protected:
	virtual Bool computePath();												///< compute the path

};
EMPTY_DTOR(AIMoveOutOfTheWayState)

//-----------------------------------------------------------------------------------------------------------
/**
 * Moves toward goal pos to tighen up a formation.
 */
class AIMoveAndTightenState : public AIInternalMoveToState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIMoveAndTightenState, "AIMoveAndTightenState")
public:
	AIMoveAndTightenState( StateMachine *machine ) : AIInternalMoveToState( machine, "AIMoveAndTightenState" )
	{
		m_checkForPath = false;
		m_okToRepathTimes = 0;
	}
	AIMoveAndTightenState( StateMachine *machine, const char *name ) : AIInternalMoveToState( machine, name )
	{
		m_checkForPath = false;
		m_okToRepathTimes = 0;
	}
	virtual StateReturnType onEnter();
	virtual StateReturnType update();
protected:
	virtual Bool computePath();												///< compute the path
	Int m_okToRepathTimes;
	Bool m_checkForPath;
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
};
EMPTY_DTOR(AIMoveAndTightenState)

//-----------------------------------------------------------------------------------------------------------
/**
 * Moves toward goal pos to get away from an enemy.
 */
class AIMoveAwayFromRepulsorsState : public AIMoveAndTightenState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIMoveAwayFromRepulsorsState, "AIMoveAwayFromRepulsorsState")
public:
	AIMoveAwayFromRepulsorsState( StateMachine *machine ) : AIMoveAndTightenState( machine, "AIMoveAwayFromRepulsors" ) { }
	virtual StateReturnType onEnter();
	virtual StateReturnType update();
	virtual void onExit( StateExitType status );
protected:
	virtual Bool computePath();												///< compute the path

};
EMPTY_DTOR(AIMoveAwayFromRepulsorsState)

//-----------------------------------------------------------------------------------------------------------
/**
 * Compute a valid spot to fire on GoalObject, and move there.
 */
class AIAttackApproachTargetState : public AIInternalMoveToState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIAttackApproachTargetState, "AIAttackApproachTargetState")
public:
	AIAttackApproachTargetState( StateMachine *machine, Bool follow, Bool attackingObject, Bool forceAttacking ) :
		AIInternalMoveToState( machine, "AIAttackApproachTargetState" ),
		m_follow(follow),
		m_isAttackingObject(attackingObject),
		m_isInitialApproach(true),
		m_isForceAttacking(forceAttacking)
	{
		// we're setting m_isInitialApproach to true in the constructor because we want the first pass
		// through this state to allow a unit to attack incidental targets (if it is turreted)
	}
	virtual Bool isAttack() const { return TRUE; }
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();
protected:
	virtual Bool computePath();												///< compute the path

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

private:

	enum { MIN_RECOMPUTE_TIME = 10 };

	StateReturnType updateInternal( void );

	Coord3D				m_prevVictimPos;									///< Where we think our victim is
	UnsignedInt		m_approachTimestamp;							///< When we last computed an approach goal
	Bool					m_follow;													///< If true, follow object until it dies
	Bool					m_isAttackingObject;							///< If false, attacking position
	Bool					m_stopIfInRange;									///< If true, we check and stop as soon we can fire.  Used when we have to path to the object instead of to a firing position.
	Bool					m_isInitialApproach;							///< If true, we can attack other units along the way. We will check for them every N frames (N specified in AI.ini)
	Bool					m_isForceAttacking;
};
EMPTY_DTOR(AIAttackApproachTargetState)

//-----------------------------------------------------------------------------------------------------------
/**
 * Chases a moving GoalObject.  If goal is not object, or not moving, or attacker lacks turret, exits with success.
 */
class AIAttackPursueTargetState : public AIInternalMoveToState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIAttackPursueTargetState, "AIAttackPursueTargetState")
public:
	AIAttackPursueTargetState( StateMachine *machine, Bool follow, Bool attackingObject, Bool forceAttacking ) :
		AIInternalMoveToState( machine, "AIAttackPursueTargetState" ),
		m_follow(follow),
		m_isAttackingObject(attackingObject),
		m_isInitialApproach(true),
		m_isForceAttacking(forceAttacking),
		m_approachTimestamp(0),
		m_stopIfInRange(false)
	{
		// we're setting m_isInitialApproach to true in the constructor because we want the first pass
		// through this state to allow a unit to attack incidental targets (if it is turreted)
	}
	virtual Bool isAttack() const { return TRUE; }
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();
protected:
	virtual Bool computePath();												///< compute the path

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

private:

	enum { MIN_RECOMPUTE_TIME = 10 };

	StateReturnType updateInternal( void );

	Coord3D				m_prevVictimPos;									///< Where we think our victim is
	UnsignedInt		m_approachTimestamp;							///< When we last computed an approach goal
	Bool					m_follow;													///< If true, follow object until it dies
	Bool					m_isAttackingObject;							///< If false, attacking position
	Bool					m_stopIfInRange;									///< If true, we check and stop as soon we can fire.  Used when we have to path to the object instead of to a firing position.
	Bool					m_isInitialApproach;							///< If true, we can attack other units along the way. We will check for them every N frames (N specified in AI.ini)
	Bool					m_isForceAttacking;
};
EMPTY_DTOR(AIAttackPursueTargetState)

//-----------------------------------------------------------------------------------------------------------
/**
 * Pick up an upgrade crate after a kill.
 */
class AIPickUpCrateState : public AIInternalMoveToState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIPickUpCrateState, "AIPickUpCrateState")
public:
	AIPickUpCrateState( StateMachine *machine ) :
		AIInternalMoveToState( machine, "AIAttackPickUpCrateState" )
	{
		// we're setting m_isInitialApproach to true in the constructor because we want the first pass
		// through this state to allow a unit to attack incidental targets (if it is turreted)
	}
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();
protected:
	virtual Bool computePath();												///< compute the path

private:
	Int			m_delayCounter;
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
};
EMPTY_DTOR(AIPickUpCrateState)

//-------------------------------------------------------------------------------------------------
/**
 * Execute an Attack-Move
 */
 class AIAttackMoveToState : public AIMoveToState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIAttackMoveToState, "AIAttackMoveToState")
public:
	AIAttackMoveToState( StateMachine *machine );
	//virtual ~AIAttackMoveToState();

	virtual Bool isAttack() const { return m_attackMoveMachine ? m_attackMoveMachine->isInAttackState() : FALSE; }
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();
#ifdef STATE_MACHINE_DEBUG
	virtual AsciiString getName() const ;
#endif

protected:
	enum {ATTACK_RETRY_COUNT=5};
	enum {ATTACK_CLOSE_ENOUGH_CELLS=8};
	CommandSourceType	m_commandSrc;		// Original command source.  We switch to CMD_FROM_AI when auto-acquiring.
	StateMachine *m_attackMoveMachine;
	UnsignedInt		m_frameToSleepUntil;
	Int						m_retryCount;
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
};

//-----------------------------------------------------------------------------------------------------------
/**
 * Follow a waypoint path
 */
class AIFollowWaypointPathState : public AIInternalMoveToState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIFollowWaypointPathState, "AIFollowWaypointPathState")
public:
	AIFollowWaypointPathState( StateMachine *machine, Bool asGroup ) : m_isFollowWaypointPathState(true),
		m_moveAsGroup(asGroup), AIInternalMoveToState( machine, "AIFollowWaypointPathState" )
	{
		m_angle = 0.0f;
	}
	AIFollowWaypointPathState( StateMachine *machine, Bool asGroup, Bool isFollow) :
		m_isFollowWaypointPathState(isFollow), m_moveAsGroup(asGroup), AIInternalMoveToState( machine,
																"AIFollowWaypointPathState" )
	{
		m_angle = 0.0f;
	}
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

protected:
	Coord2D m_groupOffset;
	Real		m_angle;
	Int  m_framesSleeping;
	const Waypoint *m_currentWaypoint;
	const Waypoint *m_priorWaypoint;
	Bool m_appendGoalPosition;

	const Bool m_moveAsGroup;
	// this is necessary because we derive from FollowWaypointPathState, but we do not want to incur the
	// expense of checking RTTI to determine whether we are actually a FollowWaypointPathState or not
	const Bool m_isFollowWaypointPathState;	// derived classes should set this false.

protected:
	void computeGoal(Bool useGroupOffsets);
	Real calcExtraPathDistance(void);
	const Waypoint *getNextWaypoint(void);
	Bool hasNextWaypoint(void);
};
EMPTY_DTOR(AIFollowWaypointPathState)

//-----------------------------------------------------------------------------------------------------------
/**
 * Follow a waypoint path exactly
 */
class AIFollowWaypointPathExactState : public AIInternalMoveToState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIFollowWaypointPathExactState, "AIFollowWaypointPathExactState")
public:
	AIFollowWaypointPathExactState( StateMachine *machine, Bool asGroup ) : m_moveAsGroup(asGroup),
		m_lastWaypoint(NULL),
		AIInternalMoveToState( machine, "AIFollowWaypointPathExactState" ) { }
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

protected:
	const Waypoint *m_lastWaypoint;
	const Bool m_moveAsGroup;
};
EMPTY_DTOR(AIFollowWaypointPathExactState)

//-----------------------------------------------------------------------------------------------------------
/**
 * Follow a waypoint path, attacking targets encountered along the way
 */
class AIAttackFollowWaypointPathState : public AIFollowWaypointPathState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIAttackFollowWaypointPathState, "AIAttackFollowWaypointPathState")
public:
	AIAttackFollowWaypointPathState( StateMachine *machine, Bool asGroup );
	//virtual ~AIAttackFollowWaypointPathState();

	virtual Bool isAttack() const { return m_attackFollowMachine ? m_attackFollowMachine->isInAttackState() : FALSE; }

	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();
#ifdef STATE_MACHINE_DEBUG
	virtual AsciiString getName() const ;
#endif

protected:
	// We can (and want to) use the attack-move-to logic for when to attack and when to do a normal attack.
	StateMachine *m_attackFollowMachine;
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
};

//-----------------------------------------------------------------------------------------------------------
/**
 * Wanders around following a waypoint path.
 */
class AIWanderState : public AIFollowWaypointPathState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIWanderState, "AIWanderState")
public:
	AIWanderState( StateMachine *machine ) : AIFollowWaypointPathState( machine, false )
	{
#ifdef STATE_MACHINE_DEBUG
		setName("AIWanderState");
#endif
		m_timer = 0;
		m_waitFrames = 0;
	}
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

protected:
	Int		m_waitFrames;
	Int		m_timer;
};
EMPTY_DTOR(AIWanderState)

//-----------------------------------------------------------------------------------------------------------
/**
 * Wanders around a point.
 */
class AIWanderInPlaceState : public AIInternalMoveToState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIWanderInPlaceState, "AIWanderInPlaceState")
public:
	AIWanderInPlaceState( StateMachine *machine ) : AIInternalMoveToState( machine, "AIWanderInPlaceState" )
	{
		m_origin.zero();
		m_waitFrames = 0;
		m_timer = 0;
	}
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

protected:
	void computeWanderGoal();
	Coord3D m_origin;									///< The point we're wandering around.
	Int		m_waitFrames;
	Int		m_timer;
};
EMPTY_DTOR(AIWanderInPlaceState)

//-----------------------------------------------------------------------------------------------------------
/**
 * Runs around screaming following a waypoint path.
 */
class AIPanicState : public AIFollowWaypointPathState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIPanicState, "AIPanicState")
public:
	AIPanicState( StateMachine *machine ) : AIFollowWaypointPathState( machine, false)
	{
#ifdef STATE_MACHINE_DEBUG
		setName("AIPanicState");
#endif
	}
	virtual StateReturnType onEnter();
	virtual StateReturnType update();
	virtual void onExit( StateExitType status );
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

protected:
	Int		m_waitFrames;
	Int		m_timer;
};
EMPTY_DTOR(AIPanicState)


//-----------------------------------------------------------------------------------------------------------
/**
 * Follow simple list of points.
 */
class AIFollowPathState : public AIInternalMoveToState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIFollowPathState, "AIFollowPathState")
public:
	AIFollowPathState( StateMachine *machine, AsciiString name = "AIFollowPathState" ) :
		m_adjustFinal(true),
		m_adjustFinalOverride(false),
		m_index(0),
		m_retryCount(10),
		AIInternalMoveToState( machine, name ) { }
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();

protected:

	Int getCurPathIndex() const { return m_index; }
	void setAdjustFinalDestination(Bool b) { m_adjustFinal = b; }
	void setAdjustFinalDestinationEvenIfNotDoingGroundMovement(Bool b) { m_adjustFinalOverride = b; }

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

private:
	Int m_index;																		///< current path index
	Bool m_adjustFinal;
	Bool m_adjustFinalOverride;
	Int m_retryCount;
};
EMPTY_DTOR(AIFollowPathState)

//-----------------------------------------------------------------------------------------------------------
/**
 * Move to and unload transport.
 */
class AIMoveAndEvacuateState : public AIInternalMoveToState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIMoveAndEvacuateState, "AIMoveAndEvacuateState")
public:
	AIMoveAndEvacuateState( StateMachine *machine ) : AIInternalMoveToState( machine, "AIMoveAndEvacuateState" )
	{
		m_origin.zero();
	}
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

private:
	Coord3D m_origin;													///< current position - set as goal on exit in case we follow with MoveToAndDelete.
};
EMPTY_DTOR(AIMoveAndEvacuateState)

//-----------------------------------------------------------------------------------------------------------
/**
 * Move to and delete self.  Used for removing reinforcement transports.
 */
class AIMoveAndDeleteState : public AIInternalMoveToState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIMoveAndDeleteState, "AIMoveAndDeleteState")
public:
	AIMoveAndDeleteState( StateMachine *machine ) : AIInternalMoveToState( machine, "AIMoveAndDeleteState" )
	{
		m_appendGoalPosition = FALSE;
	}
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

protected:
	Bool m_appendGoalPosition;
};
EMPTY_DTOR(AIMoveAndDeleteState)

//-----------------------------------------------------------------------------------------------------------
class AIAttackAimAtTargetState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIAttackAimAtTargetState, "AIAttackAimAtTargetState")
public:
	AIAttackAimAtTargetState( StateMachine *machine, Bool attackingObject, Bool forceAttacking ) :
		State( machine, "AIAttackAimAtTargetState" ),
		m_isAttackingObject(attackingObject),
		m_canTurnInPlace(false),
		m_isForceAttacking(forceAttacking),
		m_setLocomotor(false)
	{
	}
	virtual Bool isAttack() const { return TRUE; }
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
private:
	const Bool m_isAttackingObject;
	Bool m_canTurnInPlace;
	Bool m_setLocomotor;
	Bool m_isForceAttacking;
};
EMPTY_DTOR(AIAttackAimAtTargetState)

//-----------------------------------------------------------------------------------------------------------
class AIWaitState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIWaitState, "AIWaitState")
public:
	AIWaitState( StateMachine *machine ) : State( machine,"AIWaitState" ) { }
	virtual StateReturnType update();
protected:
	// snapshot interface STUBBED.
	virtual void crc( Xfer *xfer ){};
	virtual void xfer( Xfer *xfer ){XferVersion cv = 1;	XferVersion v = cv; xfer->xferVersion( &v, cv );}
	virtual void loadPostProcess(){};
};
EMPTY_DTOR(AIWaitState)

//-----------------------------------------------------------------------------------------------------------
class NotifyWeaponFiredInterface	// an ABC
{
public:
	virtual void notifyFired() = 0;
	virtual void notifyNewVictimChosen(Object* victim) = 0;
	virtual Bool isWeaponSlotOkToFire(WeaponSlotType wslot) const = 0;
	virtual Bool isAttackingObject() const = 0;
	virtual const Coord3D* getOriginalVictimPos() const = 0;
};

//-----------------------------------------------------------------------------------------------------------
class AIAttackFireWeaponState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIAttackFireWeaponState, "AIAttackFireWeaponState")
public:
	AIAttackFireWeaponState( StateMachine *machine, NotifyWeaponFiredInterface* att ) :
		State( machine, "AIAttackFireWeaponState" ),
		m_att(att)
	{
	}
	virtual Bool isAttack() const { return TRUE; }
	virtual StateReturnType update();
	virtual void onExit( StateExitType status );
	virtual StateReturnType onEnter();
protected:
	// snapshot interface STUBBED.
	virtual void crc( Xfer *xfer ){};
	virtual void xfer( Xfer *xfer ){XferVersion cv = 1;	XferVersion v = cv; xfer->xferVersion( &v, cv );}
	virtual void loadPostProcess(){};
private:
	NotifyWeaponFiredInterface *const m_att;		// this is NOT owned by us and should not be freed
};
EMPTY_DTOR(AIAttackFireWeaponState)

//-----------------------------------------------------------------------------------------------------------
class AttackExitConditionsInterface
{
public:
	virtual Bool shouldExit(const StateMachine* machine) const = 0;
};

//-----------------------------------------------------------------------------------------------------------
class AIAttackState : public State, public NotifyWeaponFiredInterface
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIAttackState, "AIAttackState")
public:

	AIAttackState( StateMachine *machine, Bool follow, Bool attackingObject, Bool forceAttacking, AttackExitConditionsInterface* attackParameters);
	//~AIAttackState();

	virtual Bool isAttack() const { return TRUE; }
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();

	virtual void notifyFired() { /* nothing */ }
	virtual void notifyNewVictimChosen(Object* victim);
	virtual const Coord3D* getOriginalVictimPos() const { return &m_originalVictimPos; }
	virtual Bool isWeaponSlotOkToFire(WeaponSlotType wslot) const { return true; }
	virtual Bool isAttackingObject() const { return m_isAttackingObject; }
	virtual Bool isForceAttacking() const { return m_isForceAttacking; }
#ifdef STATE_MACHINE_DEBUG
	virtual AsciiString getName() const ;
#endif

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

private:

	Bool chooseWeapon();

	AttackStateMachine*							m_attackMachine;						///< state sub-machine for attack behavior
	AttackExitConditionsInterface*	m_attackParameters;					///< these are not owned by this, and will not be deleted on destruction
	Team*														m_victimTeam;								///< recorded onEnter because if it changes during attack , it may no longer be a valid target.
	Coord3D													m_originalVictimPos;				///< position of first obj/pos attacked... used for ContinueAttackRange.
	const Weapon*						m_lockedWeaponOnEnter;
	const Bool							m_follow;
	const Bool							m_isAttackingObject;								// if false, attacking position
	const Bool							m_isForceAttacking;
};

//-----------------------------------------------------------------------------------------------------------
class AIAttackSquadState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIAttackSquadState, "AIAttackSquadState")
public:
	AIAttackSquadState( StateMachine *machine, AttackExitConditionsInterface *attackParameters = NULL) :
			State( machine , "AIAttackSquadState") {	}
	//~AIAttackSquadState();

	virtual Bool isAttack() const { return m_attackSquadMachine ? m_attackSquadMachine->isInAttackState() : FALSE; }
	virtual StateReturnType onEnter( void );
	virtual void onExit( StateExitType status );
	virtual StateReturnType update( void );
	Object *chooseVictim(void);
#ifdef STATE_MACHINE_DEBUG
	virtual AsciiString getName() const ;
#endif


protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

private:
	StateMachine *m_attackSquadMachine;						///< state sub-machine for attack behavior
};

//-----------------------------------------------------------------------------------------------------------
class AIDeadState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIDeadState, "AIDeadState")
public:
	AIDeadState( StateMachine *machine ) : State( machine, "AIDeadState" ) { }
	virtual StateReturnType onEnter();
	virtual StateReturnType update();
	virtual void onExit( StateExitType status );
protected:
	// snapshot interface STUBBED.
	virtual void crc( Xfer *xfer ){};
	virtual void xfer( Xfer *xfer ){XferVersion cv = 1;	XferVersion v = cv; xfer->xferVersion( &v, cv );}
	virtual void loadPostProcess(){};
};
EMPTY_DTOR(AIDeadState)

//-----------------------------------------------------------------------------------------------------------
class AIDockState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIDockState, "AIDockState")
public:
	AIDockState( StateMachine *machine ) : State( machine, "AIDockState" ), m_dockMachine(NULL), m_usingPrecisionMovement(FALSE) { }
	//~AIDockState();
	virtual Bool isAttack() const { return m_dockMachine ? m_dockMachine->isInAttackState() : FALSE; }
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();
#ifdef STATE_MACHINE_DEBUG
	virtual AsciiString getName() const ;
#endif

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

private:
	StateMachine *m_dockMachine;				///< state sub-machine for docking behavior
	Bool m_usingPrecisionMovement;			///< keep a record of precision movement so we can restore the right state on exit
};
//-----------------------------------------------------------------------------------------------------------
/**
 * Move close to GoalObject and enter it.
 */
class AIEnterState : public AIInternalMoveToState
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIEnterState, "AIEnterState")
protected:
	ObjectID m_entryToClear;
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
public:
	AIEnterState( StateMachine *machine ) : AIInternalMoveToState( machine, "AIEnterState" ) { }
	virtual StateReturnType onEnter();
	virtual StateReturnType update();
	virtual void onExit( StateExitType status );
};
EMPTY_DTOR(AIEnterState)

//-----------------------------------------------------------------------------------------------------------
class AIExitState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIExitState, "AIExitState")
protected:
	ObjectID m_entryToClear;
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
public:
	AIExitState( StateMachine *machine ) : State( machine, "AIExitState" ) { }
	virtual StateReturnType onEnter();
	virtual StateReturnType update();
	virtual void onExit( StateExitType status );
};
EMPTY_DTOR(AIExitState)

//-----------------------------------------------------------------------------------------------------------
class AIExitInstantlyState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIExitInstantlyState, "AIExitInstantlyState")
protected:
	ObjectID m_entryToClear;
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
public:
	AIExitInstantlyState( StateMachine *machine ) : State( machine, "AIExitInstantlyState" ) { }
	virtual StateReturnType onEnter();
	virtual StateReturnType update();
	virtual void onExit( StateExitType status );
};
EMPTY_DTOR(AIExitInstantlyState)


//-----------------------------------------------------------------------------------------------------------
/**
 * Guard location
 */
class AIGuardState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIGuardState, "AIGuardState")
public:
	AIGuardState( StateMachine *machine ) : State( machine, "AIGuardState" ), m_guardMachine(NULL)
	{
		m_guardMachine = NULL;
	}
	//~AIGuardState();
	virtual Bool isAttack() const;
	virtual Bool isGuardIdle() const;
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();
#ifdef STATE_MACHINE_DEBUG
	virtual AsciiString getName() const ;
#endif
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

private:
	AIGuardMachine *m_guardMachine;					///< state sub-machine for guard behavior
};

//-----------------------------------------------------------------------------------------------------------
/**
 * Guard retaliate against object (hybrid attack / guard)
 */
class AIGuardRetaliateState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIGuardRetaliateState, "AIGuardRetaliateState")
public:
	AIGuardRetaliateState( StateMachine *machine ) : State( machine, "AIGuardRetaliateState" ), m_guardRetaliateMachine(NULL) {}
	//~AIGuardRetaliateState();
	virtual Bool isAttack() const;
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();
#ifdef STATE_MACHINE_DEBUG
	virtual AsciiString getName() const ;
#endif
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

private:
	AIGuardRetaliateMachine *m_guardRetaliateMachine;					///< state sub-machine for retaliate behavior
};

//-----------------------------------------------------------------------------------------------------------
/**
 * Guard from inside a tunnel network.
 */
class AITunnelNetworkGuardState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AITunnelNetworkGuardState, "AITunnelNetworkGuardState")
public:
	AITunnelNetworkGuardState( StateMachine *machine ) : State( machine, "AITunnelNetworkGuardState" ), m_guardMachine(NULL)
	{
		m_guardMachine = NULL;
	}
	//~AIGuardState();
	virtual Bool isAttack() const;
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();
#ifdef STATE_MACHINE_DEBUG
	virtual AsciiString getName() const ;
#endif
protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

private:
	AITNGuardMachine *m_guardMachine;					///< state sub-machine for guard behavior
};

//-----------------------------------------------------------------------------------------------------------
/**
 * Seek and destroy behavior
 */
class AIHuntState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIHuntState, "AIHuntState")
public:
	AIHuntState( StateMachine *machine ) : State( machine, "AIHuntState" ), m_huntMachine(NULL)
	{
		m_nextEnemyScanTime = 0;
	}
	//~AIHuntState();
	virtual Bool isAttack() const;
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();
#ifdef STATE_MACHINE_DEBUG
	virtual AsciiString getName() const ;
#endif

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

private:
	StateMachine* m_huntMachine;					///< state sub-machine for hunt behavior
	UnsignedInt m_nextEnemyScanTime;			///< time of last enemy scan
	enum { ENEMY_SCAN_RATE = LOGICFRAMES_PER_SECOND };
};

//-----------------------------------------------------------------------------------------------------------
/**
 * Seek and destroy in area
 */
class AIAttackAreaState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIAttackAreaState, "AIAttackAreaState")
public:
	AIAttackAreaState( StateMachine *machine ) : State( machine, "AIAttackAreaState" ), m_attackMachine(NULL),
		m_nextEnemyScanTime(0) { }
	//~AIAttackAreaState();
	virtual Bool isAttack() const { return m_attackMachine ? m_attackMachine->isInAttackState() : FALSE; }
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();
#ifdef STATE_MACHINE_DEBUG
	virtual AsciiString getName() const ;
#endif

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

private:
	StateMachine *m_attackMachine;					///< state sub-machine for attack behavior
	UnsignedInt m_nextEnemyScanTime;			///< time of last enemy scan
	enum { ENEMY_SCAN_RATE = LOGICFRAMES_PER_SECOND };
};

//-----------------------------------------------------------------------------------------------------------
// Faces a position or an object (depending on goal)
//-----------------------------------------------------------------------------------------------------------
class AIFaceState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(AIFaceState, "AIFaceState")
public:
	AIFaceState( StateMachine *machine, Bool obj ) : State( machine, "AIFaceState" ), m_canTurnInPlace(false), m_obj(obj) { }
	virtual StateReturnType onEnter();
	virtual void onExit( StateExitType status );
	virtual StateReturnType update();
protected:
	// snapshot interface .
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();

protected:
	const Bool m_obj;
	Bool m_canTurnInPlace;
};
EMPTY_DTOR(AIFaceState)

#endif
