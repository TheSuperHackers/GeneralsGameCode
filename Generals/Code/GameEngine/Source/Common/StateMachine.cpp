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

// StateMachine.cpp
// Implementation of basic state machine
// Author: Michael S. Booth, January 2002

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Errors.h"
#include "Common/StateMachine.h"
#include "Common/ThingTemplate.h"
#include "Common/GameState.h"
#include "Common/GlobalData.h"
#include "Common/Xfer.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"

#include "ref_ptr.h"


//------------------------------------------------------------------------------ Performance Timers 
//#include "Common/PerfMetrics.h"
//#include "Common/PerfTimer.h"

//static PerfTimer s_stateMachineTimer("StateMachine::update", false, PERFMETRICS_LOGIC_STARTFRAME, PERFMETRICS_LOGIC_STOPFRAME);
//-------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 * Constructor
 */
State::State( StateMachine *machine, AsciiString name )	
#ifdef STATE_MACHINE_DEBUG
: m_name(name)
#endif
{
	m_ID = INVALID_STATE_ID;
	m_successStateID = INVALID_STATE_ID;
	m_failureStateID = INVALID_STATE_ID;
	m_machine = machine;
}

//-----------------------------------------------------------------------------
/**
 * Add another state transition condition for this state
 */
void State::friend_onCondition( StateTransFuncPtr test, StateID toStateID, void* userData, const char* description )
{
	m_transitions.push_back(TransitionInfo(test, toStateID, userData, description));
}


//-----------------------------------------------------------------------------
class StIncrementer
{
private:
	Int& num;
public:
	StIncrementer(Int& n) : num(n)
	{
		++num;
	}
	~StIncrementer()
	{
		--num;
	}
};
#ifdef STATE_MACHINE_DEBUG
//-----------------------------------------------------------------------------
std::vector<StateID> * State::getTransitions( void ) 
{ 
	std::vector<StateID> *ids = new std::vector<StateID>;
	ids->push_back(m_successStateID);
	ids->push_back(m_failureStateID);
	// check transition condition list
	if (!m_transitions.empty())
	{
		for(std::vector<TransitionInfo>::const_iterator it = m_transitions.begin(); it != m_transitions.end(); ++it)
		{
			ids->push_back(it->toStateID);
		}
	}
	return ids;
}
#endif

//-----------------------------------------------------------------------------
/**
 * Given a return code, handle state transitions
 */
StateReturnType State::friend_checkForTransitions( StateReturnType status )
{
	static Int checkfortransitionsnum = 0;
	
	StIncrementer inc(checkfortransitionsnum);
	if (checkfortransitionsnum >= 20) 
	{
		DEBUG_CRASH(("checkfortransitionsnum is > 20"));
		return STATE_FAILURE;
	}

	DEBUG_ASSERTCRASH(!IS_STATE_SLEEP(status), ("Please handle sleep states prior to this"));

	// handle transitions
	switch( status )
	{
		case STATE_SUCCESS:
			// check if machine should exit
			if (m_successStateID == EXIT_MACHINE_WITH_SUCCESS)
			{
				getMachine()->internalSetState( MACHINE_DONE_STATE_ID );
				return STATE_SUCCESS;
			}
			else if (m_successStateID == EXIT_MACHINE_WITH_FAILURE)
			{
				getMachine()->internalSetState( MACHINE_DONE_STATE_ID );
				return STATE_FAILURE;
			}

			// move to new state
			return getMachine()->internalSetState( m_successStateID );

		case STATE_FAILURE:
			// check if machine should exit
			if (m_failureStateID == EXIT_MACHINE_WITH_SUCCESS)
			{
				getMachine()->internalSetState( MACHINE_DONE_STATE_ID );
				return STATE_SUCCESS;
			}
			else if (m_failureStateID == EXIT_MACHINE_WITH_FAILURE)
			{
				getMachine()->internalSetState( MACHINE_DONE_STATE_ID );
				return STATE_FAILURE;
			}

			// move to new state
			return getMachine()->internalSetState( m_failureStateID );

		case STATE_CONTINUE:

			// check transition condition list
			if (!m_transitions.empty())
			{
				for(std::vector<TransitionInfo>::const_iterator it = m_transitions.begin(); it != m_transitions.end(); ++it)
				{
					if (it->test( this, it->userData ))
					{
						// test returned true, change to associated state

	#ifdef STATE_MACHINE_DEBUG
						if (getMachine()->getWantsDebugOutput()) 
						{
							DEBUG_LOG(("%d '%s' -- '%s' condition '%s' returned true!", TheGameLogic->getFrame(), getMachineOwner()->getTemplate()->getName().str(),
											getMachine()->getName().str(), it->description ? it->description : "[no description]"));
						}
	#endif

						// check if machine should exit
						if (it->toStateID == EXIT_MACHINE_WITH_SUCCESS)
						{
							return STATE_SUCCESS;
						}
						else if (it->toStateID == EXIT_MACHINE_WITH_FAILURE)
						{
							return STATE_FAILURE;
						}

						// move to new state
						return getMachine()->internalSetState( it->toStateID );
					}
				}
			}
			break;
	}

	// the machine keeps running
	return STATE_CONTINUE;
}

//-----------------------------------------------------------------------------
/**
 * Given a return code, handle state transitions
 */
StateReturnType State::friend_checkForSleepTransitions( StateReturnType status )
{
	static Int checkfortransitionsnum = 0;
	
	StIncrementer inc(checkfortransitionsnum);
	if (checkfortransitionsnum >= 20) 
	{
		DEBUG_CRASH(("checkforsleeptransitionsnum is > 20"));
		return STATE_FAILURE;
	}

	DEBUG_ASSERTCRASH(IS_STATE_SLEEP(status), ("Please only pass sleep states here"));

	// check transition condition list
	if (m_transitions.empty())
		return status;

	for(std::vector<TransitionInfo>::const_iterator it = m_transitions.begin(); it != m_transitions.end(); ++it)
	{
		if (!it->test( this, it->userData ))
			continue;

		// test returned true, change to associated state

#ifdef STATE_MACHINE_DEBUG
		if (getMachine()->getWantsDebugOutput()) 
		{
			DEBUG_LOG(("%d '%s' -- '%s' condition '%s' returned true!", TheGameLogic->getFrame(), getMachineOwner()->getTemplate()->getName().str(),
							getMachine()->getName().str(), it->description ? it->description : "[no description]"));
		}
#endif

		// check if machine should exit
		if (it->toStateID == EXIT_MACHINE_WITH_SUCCESS)
		{
			return STATE_SUCCESS;
		}
		else if (it->toStateID == EXIT_MACHINE_WITH_FAILURE)
		{
			return STATE_FAILURE;
		}
		else
		{
			// move to new state
			return getMachine()->internalSetState( it->toStateID );
		}
	}

	return status;
}
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 * Constructor
 */
StateMachine::StateMachine( Object *owner, AsciiString name )
{
	m_owner = owner;
	m_sleepTill = 0;
	m_defaultStateID = INVALID_STATE_ID;
	m_defaultStateInited = false;
	m_currentState = NULL;
	m_locked = false;
#ifdef STATE_MACHINE_DEBUG
	m_name = name;
	m_debugOutput = false;
	m_lockedby = NULL;
#endif
	internalClear();
}

//-----------------------------------------------------------------------------
/**
 * Destructor.  Destroy any states attached to this machine.
 */
StateMachine::~StateMachine()
{

	// do not allow current state to exit
	if (m_currentState)
		m_currentState->onExit( EXIT_RESET );

	std::map<StateID, State *>::iterator i;

	// delete all states in the mapping
	for( i = m_stateMap.begin(); i != m_stateMap.end(); ++i )
	{
		if ((*i).second)
			deleteInstance((*i).second);
	}
}

//-----------------------------------------------------------------------------
#ifdef STATE_MACHINE_DEBUG
Bool StateMachine::getWantsDebugOutput() const 
{ 
	if (m_debugOutput)
	{
		return true;
	}

	if (TheGlobalData->m_stateMachineDebug)
	{
		return true;
	}

#ifdef DEBUG_OBJECT_ID_EXISTS
	if (TheObjectIDToDebug != 0 && getOwner() != NULL && getOwner()->getID() == TheObjectIDToDebug)
	{
		return true;
	}
#endif

	return false;
}
#endif

//-----------------------------------------------------------------------------
/**
 * Clear the internal variables of state machine to known values.
 */
void StateMachine::internalClear()
{
	m_goalObjectID = INVALID_ID;
	m_goalPosition.x = 0.0f;
	m_goalPosition.y = 0.0f;
	m_goalPosition.z = 0.0f;
#ifdef STATE_MACHINE_DEBUG
	if (getWantsDebugOutput())
	{
		DEBUG_LOG(("%d '%s' -- '%s' %x internalClear()", TheGameLogic->getFrame(), m_owner->getTemplate()->getName().str(), m_name.str(), this));
	}
#endif
}

//-----------------------------------------------------------------------------
/**
 * Clear the machine
 */
void StateMachine::clear()
{
	// if the machine is locked, it cannot be cleared
	if (m_locked)
	{
#ifdef STATE_MACHINE_DEBUG
		if (m_currentState) DEBUG_LOG((" cur state '%s'", m_currentState->getName().str()));
		DEBUG_LOG(("machine is locked (by %s), cannot be cleared (Please don't ignore; this generally indicates a potential logic flaw)",m_lockedby));
#endif
		return;
	}

	// invoke the old state's onExit()
	if (m_currentState)
		m_currentState->onExit( EXIT_RESET );

	m_currentState = NULL;

	internalClear();
}

//-----------------------------------------------------------------------------
/**
 * Reset the machine to its default state
 */
StateReturnType StateMachine::resetToDefaultState()
{
	// if the machine is locked, it cannot be reset
	if (m_locked)
	{
#ifdef STATE_MACHINE_DEBUG
		if (m_currentState) DEBUG_LOG((" cur state '%s'", m_currentState->getName().str()));
		DEBUG_LOG(("machine is locked (by %s), cannot be cleared (Please don't ignore; this generally indicates a potential logic flaw)",m_lockedby));
#endif
		return STATE_FAILURE;
	}

	if (!m_defaultStateInited)
	{
		DEBUG_CRASH(("you may not call resetToDefaultState before initDefaultState"));
		return STATE_FAILURE;
	}

	// allow current state to exit with EXIT_RESET if present
	if (m_currentState)
		m_currentState->onExit( EXIT_RESET );
	m_currentState = NULL;

	//
	// the current state has done an onExit, clear the internal guts before we set
	// the new state, to clear it after the new state is set might be overwriting
	// things the new state transition causes to happen
	//
	internalClear();

	// change to the default state
	StateReturnType status = internalSetState( m_defaultStateID );

	DEBUG_ASSERTCRASH( status != STATE_FAILURE, ( "StateMachine::resetToDefaultState() Error setting default state" ) );

	return status;

}

//-----------------------------------------------------------------------------
/**
 * Run one step of the machine
 */
StateReturnType StateMachine::updateStateMachine()
{
	UnsignedInt now = TheGameLogic->getFrame();
	if (m_sleepTill != 0 && now < m_sleepTill)
	{
		if( m_currentState == NULL )
		{
			return STATE_FAILURE;
		}
		return m_currentState->friend_checkForSleepTransitions( STATE_SLEEP(m_sleepTill - now) );
	}

	// not sleeping anymore
	m_sleepTill = 0;

	if (m_currentState)
	{
		// TheSuperHackers @bugfix xezon 20/05/2025 Defer the deletion of this state machine.
		// Calling m_currentState->update() can release this state machine in certain circumstances,
		// for example if something kills the entity of this state machine as a result of this state update.
		// See https://github.com/TheSuperHackers/GeneralsGameCode/issues/212
		RefCountPtr<StateMachine> refThis = RefCountPtr<StateMachine>::Create_AddRef(this);

		// update() can change m_currentState, so save it for a moment...
		State* stateBeforeUpdate = m_currentState;

		// execute this state
		StateReturnType status = m_currentState->update();

		// it is possible that the state's update() method clears the state machine.
		if (m_currentState == NULL)
		{
			return STATE_FAILURE;
		}
		
		// here's the scenario: 
		// -- State A calls foo() and then says "sleep for 2000 frames".
		// -- however, foo() called setState() to State B. thus our current state is not the same.
		// -- thus, if the state changed, we must ignore any sleep result and pretend we got STATE_CONTINUE,
		// so that the new state will be called immediately.
		if (stateBeforeUpdate != m_currentState)
		{
			status = STATE_CONTINUE;
		}

		if (IS_STATE_SLEEP(status))
		{
			// hey, we're sleepy!
			m_sleepTill = now + GET_STATE_SLEEP_FRAMES(status);
			return m_currentState->friend_checkForSleepTransitions( STATE_SLEEP(m_sleepTill - now) );
		}
		else
		{
			// check for state transitions, possibly exiting this machine
			return m_currentState->friend_checkForTransitions( status );
		}
	}
	else
	{
		DEBUG_CRASH(("State machine has no current state -- did you remember to call initDefaultState?"));
		return STATE_FAILURE;
	}
}

//-----------------------------------------------------------------------------
/**
 * Given a unique (for this machine) id number, and an instance of the
 * State class, the machine records this as a possible state, and
 * retains the id mapping.
 * These state id's are used to change the machine's state via setState().
 */
void StateMachine::defineState( StateID id, State *state, StateID successID, StateID failureID, const StateConditionInfo* conditions )
{
#ifdef STATE_MACHINE_DEBUG
	DEBUG_ASSERTCRASH(m_stateMap.find( id ) == m_stateMap.end(), ("duplicate state ID in statemachine %s\n",m_name.str()));
#endif

	// map the ID to the state
	m_stateMap.insert( std::map<StateID, State *>::value_type( id, state ) );

	// store the ID in the state itself, as well
	state->friend_setID( id );

	state->friend_onSuccess(successID);
	state->friend_onFailure(failureID);
	
	while (conditions && conditions->test != NULL)
	{
		state->friend_onCondition(conditions->test, conditions->toStateID, conditions->userData);
		++conditions;
	}

	if (m_defaultStateID == INVALID_STATE_ID)
		m_defaultStateID = id;
}

//-----------------------------------------------------------------------------
/**
 * Given a state ID, return the state instance
 */
State *StateMachine::internalGetState( StateID id )
{
	// locate the actual state associated with the given ID
	std::map<StateID, State *>::iterator i;
	i = m_stateMap.find( id );

	if (i == m_stateMap.end())
	{
		DEBUG_CRASH(( "StateMachine::internalGetState(): Invalid state" ));
		throw ERROR_BAD_ARG;
	}

	return (*i).second;
}

//-----------------------------------------------------------------------------
/**
 * Change the current state of the machine.
 * This causes the old state's onExit() method to be invoked,
 * and the new state's onEnter() method to be invoked.
 */
StateReturnType StateMachine::setState( StateID newStateID )
{
	// if the machine is locked, it cannot change state via external events
	if (m_locked)
	{
#ifdef STATE_MACHINE_DEBUG
		if (m_currentState) DEBUG_LOG((" cur state '%s'", m_currentState->getName().str()));
		DEBUG_LOG(("machine is locked (by %s), cannot be cleared (Please don't ignore; this generally indicates a potential logic flaw)",m_lockedby));
#endif
		return STATE_CONTINUE;
	}

	return internalSetState( newStateID );
}

//-----------------------------------------------------------------------------
/**
 * Change the current state of the machine.
 * This causes the old state's onExit() method to be invoked,
 * and the new state's onEnter() method to be invoked.
 */
StateReturnType StateMachine::internalSetState( StateID newStateID )
{
	State *newState = NULL;

	// anytime the state changes, stop sleeping
	m_sleepTill = 0;

	// if we're not setting the "done" state ID we will continue with the actual transition
	if( newStateID != MACHINE_DONE_STATE_ID )
	{

		// if incoming state is invalid, go to the machine's default state
		if (newStateID == INVALID_STATE_ID)
		{
			newStateID = m_defaultStateID;
			if (newStateID == INVALID_STATE_ID)
			{
				DEBUG_CRASH(("you may NEVER set the current state to an invalid state id."));
				return STATE_FAILURE;
			}
		}

		// extract the state associated with the given ID
		newState = internalGetState( newStateID );
#ifdef STATE_MACHINE_DEBUG
		if (getWantsDebugOutput()) 
		{
			StateID curState = INVALID_STATE_ID;
			if (m_currentState) {
				curState = m_currentState->getID();
			}
			DEBUG_LOG(("%d '%s' -- '%s' %x exit ", TheGameLogic->getFrame(), m_owner->getTemplate()->getName().str(), m_name.str(), this));
			if (m_currentState) {
				DEBUG_LOG((" '%s' ", m_currentState->getName().str()));
			} else {
				DEBUG_LOG((" INVALID_STATE_ID "));
			}
			if (newState) {
				DEBUG_LOG(("enter '%s' ", newState->getName().str()));
			} else {
				DEBUG_LOG(("to INVALID_STATE"));
			}
		}
#endif
	}

	// invoke the old state's onExit()
	if (m_currentState)
		m_currentState->onExit( EXIT_NORMAL );

	// set the new state
	m_currentState = newState;

	// invoke the new state's onEnter()
	/// @todo It might be useful to pass the old state in... (MSB)
	if( m_currentState )
	{
		// onEnter() could conceivably change m_currentState, so save it for a moment...
		State* stateBeforeEnter = m_currentState;

		StateReturnType status = m_currentState->onEnter();

		// it is possible that the state's onEnter() method may cause the state to be destroyed
		if (m_currentState == NULL)
		{
			return STATE_FAILURE;
		}

		// here's the scenario: 
		// -- State A calls foo() and then says "sleep for 2000 frames".
		// -- however, foo() called setState() to State B. thus our current state is not the same.
		// -- thus, if the state changed, we must ignore any sleep result and pretend we got STATE_CONTINUE,
		// so that the new state will be called immediately.
		if (stateBeforeEnter != m_currentState)
		{
			status = STATE_CONTINUE;
		}

		if (IS_STATE_SLEEP(status))
		{
			// hey, we're sleepy!
			UnsignedInt now = TheGameLogic->getFrame();
			m_sleepTill = now + GET_STATE_SLEEP_FRAMES(status);
			return m_currentState->friend_checkForSleepTransitions( STATE_SLEEP(m_sleepTill - now) );
		}
		else
		{
			// check for state transitions, possibly exiting this machine
			return m_currentState->friend_checkForTransitions( status );
		}
	}
	else
	{
		return STATE_CONTINUE;  // irrelevant return code, but we must return something
	}
}

//-----------------------------------------------------------------------------
/**
 * Define the default state of the machine, and
 * set the machine's state to it.
 */
StateReturnType StateMachine::initDefaultState()
{
#ifdef DEBUG_LOGGING
#ifdef STATE_MACHINE_DEBUG
#define REALLY_VERBOSE_LOG(x) /* */
	// Run through all the transitions and make sure there aren't any transitions to undefined states. jba. [8/18/2003]
	std::map<StateID, State *>::iterator i;
	REALLY_VERBOSE_LOG(("SM_BEGIN\n"));
	for( i = m_stateMap.begin(); i != m_stateMap.end(); ++i ) {
		State *state = (*i).second;
		StateID id = state->getID();
		// Check transitions. [8/18/2003]
		std::vector<StateID> *ids = state->getTransitions();
		// check transitions
		REALLY_VERBOSE_LOG(("State %s(%d) : ", state->getName().str(), id));
		if (!ids->empty())
		{
			for(std::vector<StateID>::const_iterator it = ids->begin(); it != ids->end(); ++it)
			{
				StateID curID = *it;
				REALLY_VERBOSE_LOG(("%d('", curID));
				if (curID == INVALID_STATE_ID) {
					REALLY_VERBOSE_LOG(("INVALID_STATE_ID', "));
					continue;
				}
				if (curID == EXIT_MACHINE_WITH_SUCCESS) {
					REALLY_VERBOSE_LOG(("EXIT_MACHINE_WITH_SUCCESS', "));
					continue;
				}
				if (curID == EXIT_MACHINE_WITH_FAILURE) {
					REALLY_VERBOSE_LOG(("EXIT_MACHINE_WITH_FAILURE', "));
					continue;
				}
				// locate the actual state associated with the given ID
				std::map<StateID, State *>::iterator i;
				i = m_stateMap.find( curID );

				if (i == m_stateMap.end()) {
					DEBUG_LOG(("\nState %s(%d) : ", state->getName().str(), id));
					DEBUG_LOG(("Transition %d not found", curID));
					DEBUG_LOG(("This MUST BE FIXED!!!jba"));
					DEBUG_CRASH(("Invalid transition."));
				} else {
					State *st = (*i).second;
					if (st->getName().isNotEmpty()) {
						REALLY_VERBOSE_LOG(("%s') ", st->getName().str()));
					}
				}
			}
		}
		REALLY_VERBOSE_LOG(("\n"));
		delete ids;
		ids = NULL;
	}
	REALLY_VERBOSE_LOG(("SM_END\n\n"));
#endif	
#endif
	DEBUG_ASSERTCRASH(!m_locked, ("Machine is locked here, but probably should not be"));
	if (m_defaultStateInited)
	{
		DEBUG_CRASH(("you may not call initDefaultState twice for the same StateMachine"));
		return STATE_FAILURE;
	}
	else
	{
		m_defaultStateInited = true;
		return internalSetState( m_defaultStateID );
	}
}

//-----------------------------------------------------------------------------
void StateMachine::setGoalObject( const Object *obj ) 
{ 
	if (m_locked)
		return;

	internalSetGoalObject( obj );
}

//-----------------------------------------------------------------------------
Bool StateMachine::isGoalObjectDestroyed() const
{ 
	if (m_goalObjectID == 0) 
	{
		return false; // never had a goal object
	}
	return getGoalObject() == NULL;
}

//-----------------------------------------------------------------------------
void StateMachine::halt() 
{ 
	m_locked = true;
	m_currentState = NULL; // don't exit current state, just clear it.
#ifdef STATE_MACHINE_DEBUG
	if (getWantsDebugOutput())
	{
		DEBUG_LOG(("%d '%s' -- '%s' %x halt()", TheGameLogic->getFrame(), m_owner->getTemplate()->getName().str(), m_name.str(), this));
	}	
#endif
}

//-----------------------------------------------------------------------------
void StateMachine::internalSetGoalObject( const Object *obj ) 
{ 
	if (obj) {
		m_goalObjectID = obj->getID(); 
		internalSetGoalPosition(obj->getPosition());
	}
	else {
		m_goalObjectID = INVALID_ID;
	}
}

//-----------------------------------------------------------------------------
Object *StateMachine::getGoalObject() 
{ 
	return TheGameLogic->findObjectByID( m_goalObjectID ); 
}

//-----------------------------------------------------------------------------
const Object *StateMachine::getGoalObject() const
{ 
	return TheGameLogic->findObjectByID( m_goalObjectID ); 
}

//-----------------------------------------------------------------------------
void StateMachine::setGoalPosition( const Coord3D *pos ) 
{ 
	if (m_locked)
		return;

	internalSetGoalPosition( pos );
}

//-----------------------------------------------------------------------------
void StateMachine::internalSetGoalPosition( const Coord3D *pos ) 
{ 
	if (pos) {
		m_goalPosition = *pos; 
		// Don't clear the goal object, or everything breaks.  Like construction of buildings.
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void StateMachine::crc( Xfer *xfer )
{

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer Method
	* Version Info
	* 1: Initial version
	*/
// ------------------------------------------------------------------------------------------------
void StateMachine::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	xfer->xferUnsignedInt(&m_sleepTill);
	xfer->xferUnsignedInt(&m_defaultStateID);
	StateID curStateID = getCurrentStateID();
	xfer->xferUnsignedInt(&curStateID);
	if (xfer->getXferMode() == XFER_LOAD)	{
		// We are going to jump into the current state.	We don't call onEnter or onExit, because the 
		// state was already active when we saved.
		m_currentState = internalGetState( curStateID );
	}

	Bool snapshotAllStates = false;
#ifdef RTS_DEBUG
	//snapshotAllStates = true;
#endif
	xfer->xferBool(&snapshotAllStates);
	if (snapshotAllStates) {
		std::map<StateID, State *>::iterator i;
		// count all states in the mapping
		Int count = 0;
		for( i = m_stateMap.begin(); i != m_stateMap.end(); ++i )
			count++;
		Int saveCount = count;
		xfer->xferInt(&saveCount);
		if (saveCount!=count) {
			DEBUG_CRASH(("State count mismatch - %d expected, %d read", count, saveCount));
			throw SC_INVALID_DATA;
		}
		for( i = m_stateMap.begin(); i != m_stateMap.end(); ++i ) {
			State *state = (*i).second;
			StateID id = state->getID();
			xfer->xferUnsignedInt(&id);
			if (id!=state->getID()) {
				DEBUG_CRASH(("State ID mismatch - %d expected, %d read", state->getID(), id));
				throw SC_INVALID_DATA;
			}
			xfer->xferSnapshot(state);
		}

	}	else {
		xfer->xferSnapshot(m_currentState);
	}


	xfer->xferObjectID(&m_goalObjectID);
	xfer->xferCoord3D(&m_goalPosition);
	xfer->xferBool(&m_locked);
	xfer->xferBool(&m_defaultStateInited);
}  // end xfer


// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void StateMachine::loadPostProcess( void )
{

}  // end loadPostProcess

