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

// StateMachine.h
// Finite state machine encapsulation
// Author: Michael S. Booth, January 2002

#pragma once

#ifndef _STATE_MACHINE_H_
#define _STATE_MACHINE_H_

#include "Common/GameMemory.h"
#include "Common/GameType.h"
#include "Common/ModelState.h"
#include "Common/Snapshot.h"
#include "Common/Xfer.h"

#include "refcount.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class State;
class StateMachine;
class Object;

//#undef STATE_MACHINE_DEBUG
#if defined(RTS_DEBUG)
	#define STATE_MACHINE_DEBUG
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

enum { MACHINE_DONE_STATE_ID = 999998, INVALID_STATE_ID = 999999 };
typedef UnsignedInt StateID;									///< used to denote individual states
typedef Bool (*StateTransFuncPtr)( State *state, void* userData );

/**
 * State return codes
 */
enum StateReturnType CPP_11(: Int) 
{ 
	// note that all positive values are reserved for STATE_SLEEP!

	STATE_CONTINUE	= 0,						///< stay in this state (only for update method)
	STATE_SUCCESS		= -1,						///< state finished successfully, go to next state
	STATE_FAILURE		= -2,						///< state finished abnormally, go to next state
};

#define STATE_SLEEP(numFrames)				((StateReturnType)(numFrames))
#define IS_STATE_SLEEP(ret)						((Int)(ret) > 0)
#define GET_STATE_SLEEP_FRAMES(ret)		((UnsignedInt)(ret))

// (we use 0x3fffffff so that we can add offsets and not overflow...
//		at 30fps that's around ~414 days!)
#define STATE_SLEEP_FOREVER		STATE_SLEEP(0x3fffffff)

// this is mainly useful for states that enclose other state machines...
// where, even if the enclosed machine is sleeping, the encloser still needs
// to run every frame.
inline StateReturnType CONVERT_SLEEP_TO_CONTINUE(StateReturnType s)
{
	return IS_STATE_SLEEP(s) ? STATE_CONTINUE : s;
}

// this is mainly useful for states that enclose other state machines...
// where the encloser and enclosee both might sleep. we need to choose the min
// sleep time that satisfies both.
inline StateReturnType MIN_SLEEP(UnsignedInt encloserSleep, StateReturnType encloseeResult)
{
	if (IS_STATE_SLEEP(encloseeResult))
	{
		UnsignedInt encloseeSleep = GET_STATE_SLEEP_FRAMES(encloseeResult);
		return STATE_SLEEP(min(encloserSleep, encloseeSleep));
	}
	else
	{
		// if enclosee needs to stay awake, we better do so, regardless
		return encloseeResult;
	}
}


/** 
 * Special argument for onCondition. It means when the given condition
 * becomes true, the state machine will exit and return the given status.
 */
enum 
{ 
	EXIT_MACHINE_WITH_SUCCESS = 9998,
	EXIT_MACHINE_WITH_FAILURE = 9999
};

/** 
 * Parameters for onExit().
 */
enum StateExitType CPP_11(: Int)
{
	EXIT_NORMAL,							///< state exited due to normal state transitioning
	EXIT_RESET								///< state exited due to state machine reset
};

struct StateConditionInfo
{
	StateTransFuncPtr	test;
	StateID						toStateID;
	void*							userData;

	StateConditionInfo(StateTransFuncPtr t, StateID id, void* ud) : test(t), toStateID(id), userData(ud) { }
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 * An abstraction of a machine's "state".
 */
class State : public MemoryPoolObject, public Snapshot
{
	MEMORY_POOL_GLUE_ABC( State )						///< this abstract class needs memory pool hooks
public:
	State( StateMachine *machine, AsciiString name);
	// already defined by MPO.
	//virtual ~State() { }

	virtual StateReturnType onEnter() { return STATE_CONTINUE; }	///< executed once when entering state
	virtual void onExit( StateExitType status ) { }											///< executed once when leaving state
	virtual StateReturnType update() = 0;	///< implements this state's behavior, decides when to change state

	virtual Bool isIdle() const { return false; }
	virtual Bool isAttack() const { return false; }
	virtual Bool isGuardIdle() const { return FALSE; }
	//Definition of busy -- when explicitly in the busy state. Moving or attacking is not considered busy!
	virtual Bool isBusy() const { return false; }

	inline StateMachine* getMachine() { return m_machine; }		///< return the machine this state is part of
	inline StateID getID() const { return m_ID; }			///< get this state's id 

	Object* getMachineOwner();
	const Object* getMachineOwner() const;
	Object* getMachineGoalObject();
	const Object* getMachineGoalObject() const;
	const Coord3D* getMachineGoalPosition() const;

#ifdef STATE_MACHINE_DEBUG
	virtual AsciiString getName() const {return m_name;}
	std::vector<StateID> *getTransitions(void);
#endif

	// for internal use by the StateMachine class ---------------------------------------------------------
	inline void friend_setID( StateID id ) { m_ID = id; }			///< define this state's id (for use only by StateMachine class)
	void friend_onSuccess( StateID toStateID ) { m_successStateID = toStateID; }	///< define which state to move to after successful completion
	void friend_onFailure( StateID toStateID ) { m_failureStateID = toStateID; }	///< define which state to move to after failure 
	void friend_onCondition( StateTransFuncPtr test, StateID toStateID, void* userData, const char* description = NULL );	///< define when to change state
	StateReturnType friend_checkForTransitions( StateReturnType status );	///< given a return code, handle state transitions
	StateReturnType friend_checkForSleepTransitions( StateReturnType status );	///< given a return code, handle state transitions

protected:

#ifdef STATE_MACHINE_DEBUG
	inline void setName(AsciiString n) { m_name = n; }
#endif

protected:
	// snapshot interface	 - pure virtual here.
	// Essentially all the member data gets set up on creation and shouldn't change.
	// So none of it needs to be saved, and it nicely forces all user states to 
	// remember to implement crc, xfer & loadPostProcess.  jba
	virtual void crc( Xfer *xfer )=0;
	virtual void xfer( Xfer *xfer )=0;
	virtual void loadPostProcess()=0;

private:

	struct TransitionInfo
	{
		StateTransFuncPtr		test;											///< the condition evaluation function
		StateID							toStateID;								///< the state to transition to
		void*								userData;									///< data passed to transFuncPtr.
#ifdef STATE_MACHINE_DEBUG
		const char*					description;							///< description (for debugging purposes)
#endif

		TransitionInfo(StateTransFuncPtr t, StateID id, void* ud, const char* desc) : 
			test(t), 
			toStateID(id),
			userData(ud)
#ifdef STATE_MACHINE_DEBUG
			, description(desc) 
#endif
		{ }
	};

	StateID m_ID;																///< this state's ID
	StateID m_successStateID;										///< state to move to upon success
	StateID m_failureStateID;										///< state to move to upon failure
	std::vector<TransitionInfo> m_transitions;	///< possible transitions from this state

	StateMachine *m_machine;										///< the state machine this state is part of
protected:
#ifdef STATE_MACHINE_DEBUG
	AsciiString m_name;													///< Human readable name of this state - for debugging.  jba.
#endif
};
inline State::~State() { }

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

/**
 * A finite state machine.
 */
class StateMachine : public MemoryPoolObject, public Snapshot
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( StateMachine, "StateMachinePool" );

public:
	/**
	 * All of the states used by this machine should be
	 * instantiated and defined via defineState() in the
	 * machine's constructor.
	 */
	StateMachine( Object *owner, AsciiString name );
	// virtual destructor defined by MemoryPool

	virtual StateReturnType updateStateMachine();				///< run one step of the machine

	virtual void clear();																///< clear the machine's internals to a known, initialized state

	virtual StateReturnType resetToDefaultState();			///< clear the machine's internals and set to the default state

	/** must be called to jump the StateMachine into the default state... this may fail
		(return a failure code), so it can't be done via the ctor. you MUST call this before
		using the state machine. */
	virtual StateReturnType initDefaultState();	

	virtual StateReturnType setState( StateID newStateID );			///< change the current state of the machine (which may cause further state changes, due to onEnter)

	void Add_Ref() const { m_refCount.Add_Ref(); }
	void Release_Ref() const { m_refCount.Release_Ref(MemoryPoolObject::deleteInstanceInternal, this); }
	UnsignedByte Num_Refs() const { return m_refCount.Num_Refs(); }

	StateID getCurrentStateID() const { return m_currentState ? m_currentState->getID() : INVALID_STATE_ID; }	///< return the id of the current state of the machine
	Bool isInIdleState() const { return m_currentState ? m_currentState->isIdle() : true; }	// stateless things are considered 'idle'
	Bool isInAttackState() const { return m_currentState ? m_currentState->isAttack() : true; }	// stateless things are considered 'idle'
	Bool isInForceAttackState() const { return m_currentState ? m_currentState->isIdle() : true; }	// stateless things are considered 'idle'
	Bool isInGuardIdleState() const { return m_currentState ? m_currentState->isGuardIdle() : FALSE; } // stateless things aren't guard idle.

	//Definition of busy -- when explicitly in the busy state. Moving or attacking is not considered busy!
	Bool isInBusyState() const { return m_currentState ? m_currentState->isBusy() : false; }	// stateless things are not considered 'busy'

	// no, this is now deprecated. you should strive to avoid having to get the current state.
	// try to make do with getCurrentStateID() or isInIdleState() instead. (srj)
	// State *getCurrentState() { return m_currentState; }	///< return the current state of the machine

	/**
	 * Lock/unlock this state machine.
	 * If a machine is locked, it cannot be reset, or given external setStates(), etc.
	 */
	void lock(const char* msg) 
	{
		m_locked = true; 
#ifdef STATE_MACHINE_DEBUG
		m_lockedby = msg;
#endif
	}

	void unlock() 
	{ 
		m_locked = false; 
#ifdef STATE_MACHINE_DEBUG
		m_lockedby = NULL;
#endif
	}

	Bool isLocked() const { return m_locked; }

	/**
	 * Get the object that "owns" this machine.
	 * There need not be an object with a machine, but it is so common
	 * that it is useful to have this method in the generic state machine.
	 */
	Object *getOwner() { return m_owner; }
	const Object *getOwner() const { return m_owner; }

	// common parameters for state machines
	void setGoalObject( const Object *obj );
	Object *getGoalObject();
	const Object *getGoalObject() const;
	void setGoalPosition( const Coord3D *pos );
	const Coord3D *getGoalPosition() const { return &m_goalPosition; }
	Bool isGoalObjectDestroyed() const;  ///< Returns true if we had a goal object, but it has been destroyed. 
	
	virtual void halt(void); ///< Stops the state machine & disables it in preparation for deleting it.

	//
	// The following methods are for internal use by the State class
	//
	StateReturnType internalSetState( StateID newStateID );	///< for internal use only - change the current state of the machine

#if defined(RTS_DEBUG)
	UnsignedInt peekSleepTill() const { return m_sleepTill; }
#endif

#ifdef STATE_MACHINE_DEBUG
	Bool getWantsDebugOutput() const;
	void setDebugOutput( Bool output ) { m_debugOutput = output; }
	void setName( AsciiString name) {m_name = name;}
	inline AsciiString getName() const {return m_name;}
	virtual AsciiString getCurrentStateName() const { return m_currentState ? m_currentState->getName() : AsciiString::TheEmptyString;}
#else
	inline Bool getWantsDebugOutput() const { return false; }
	inline AsciiString getCurrentStateName() const { return AsciiString::TheEmptyString;}
#endif

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();	

protected:

	/**
	 * Given a unique (to this machine) integer ID representing a state, and an instance 
	 * of that state, the machine records this as a possible state, and
	 * internally maps the given integer ID to the state instance.
	 * These state id's are used to change the machine's state via setState().
	 */
	void defineState( StateID id, State *state, 
										StateID successID, 
										StateID failureID,
										const StateConditionInfo* conditions = NULL);	

	State* internalGetState( StateID id );

private:

	void internalClear();
	void internalSetGoalObject( const Object *obj );
	void internalSetGoalPosition( const Coord3D *pos);


	std::map<StateID, State *>	m_stateMap;			///< the mapping of ids to states
	Object*											m_owner;				///< object that "owns" this machine 

	UnsignedInt		m_sleepTill;									///< if nonzero, we are sleeping 'till this frame

	StateID				m_defaultStateID;									///< the default state of the machine
	State*				m_currentState;

	ObjectID			m_goalObjectID;										///< the object of interest for this state
	Coord3D				m_goalPosition;										///< the position of interest for this state

	Bool					m_locked;													///< whether this machine is locked or not
	Bool					m_defaultStateInited;							///< if initDefaultState has been called

	RefCountValue<UnsignedByte> m_refCount;

#ifdef STATE_MACHINE_DEBUG
	Bool					m_debugOutput;
	AsciiString		m_name;													///< Human readable name of this state - for debugging.  jba.
	const char*		m_lockedby;
#endif
};

//-----------------------------------------------------------------------------

inline Object* State::getMachineOwner() { return m_machine->getOwner(); }
inline const Object* State::getMachineOwner() const { return m_machine->getOwner(); }

inline Object* State::getMachineGoalObject() { return m_machine->getGoalObject(); }		///< return the machine this state is part of
inline const Object* State::getMachineGoalObject() const { return m_machine->getGoalObject(); }		///< return the machine this state is part of

inline const Coord3D* State::getMachineGoalPosition() const { return m_machine->getGoalPosition(); }		///< return the machine this state is part of

//-----------------------------------------------------------------------------------------------------------
/**
	A utility state that immediately succeeds.
*/
class SuccessState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(SuccessState, "SuccessState")		
public:
	SuccessState( StateMachine *machine ) : State( machine, "SuccessState") { }
	virtual StateReturnType onEnter() { return STATE_SUCCESS; }
	virtual StateReturnType update() { return STATE_SUCCESS; }
protected:
	// snapshot interface STUBBED.
	virtual void crc( Xfer *xfer ){};
	virtual void xfer( Xfer *xfer ){XferVersion cv = 1;	XferVersion v = cv; xfer->xferVersion( &v, cv );}
	virtual void loadPostProcess(){};

};
EMPTY_DTOR(SuccessState)

//-----------------------------------------------------------------------------------------------------------
/**
	A utility state that immediately fails.
*/
class FailureState : public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(FailureState, "FailureState")		
public:
	FailureState( StateMachine *machine ) : State( machine, "FailureState") { }
	virtual StateReturnType onEnter() { return STATE_FAILURE; }
	virtual StateReturnType update() { return STATE_FAILURE; }
protected:
	// snapshot interface STUBBED.
	virtual void crc( Xfer *xfer ){};
	virtual void xfer( Xfer *xfer ){XferVersion cv = 1;	XferVersion v = cv; xfer->xferVersion( &v, cv );}
	virtual void loadPostProcess(){};
};
EMPTY_DTOR(FailureState)
 

//-----------------------------------------------------------------------------------------------------------
/**
	A utility state that never exits (except due to conditions).
*/
class ContinueState :  public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(ContinueState, "ContinueState")		
public:
	ContinueState( StateMachine *machine ) : State( machine, "ContinueState" ) { }
	virtual StateReturnType onEnter() { return STATE_CONTINUE; }
	virtual StateReturnType update() { return STATE_CONTINUE; }
protected:
	// snapshot interface STUBBED.
	virtual void crc( Xfer *xfer ){};
	virtual void xfer( Xfer *xfer ){XferVersion cv = 1;	XferVersion v = cv; xfer->xferVersion( &v, cv );}
	virtual void loadPostProcess(){};
};
EMPTY_DTOR(ContinueState)


//-----------------------------------------------------------------------------------------------------------
/**
	A utility state that sleeps forever.
*/
class SleepState :  public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(SleepState, "SleepState")		
public:
	SleepState( StateMachine *machine ) : State( machine, "SleepState" ) { }
	virtual StateReturnType onEnter() { return STATE_SLEEP_FOREVER; }
	virtual StateReturnType update() { return STATE_SLEEP_FOREVER; }
protected:
	// snapshot interface STUBBED.
	virtual void crc( Xfer *xfer ){};
	virtual void xfer( Xfer *xfer ){XferVersion cv = 1;	XferVersion v = cv; xfer->xferVersion( &v, cv );}
	virtual void loadPostProcess(){};
};
EMPTY_DTOR(SleepState)


//-----------------------------------------------------------------------------------------------------------
// TheSuperHackers @info Misappropriates deleteInstance to call Release_Ref to keep the StateMachine fix small.
// @todo Replace calls to deleteInstance with RefCountPtr<StateMachine> when so appropriate.
inline void deleteInstance(StateMachine* machine)
{
	machine->Release_Ref();
}


#endif // _STATE_MACHINE_H_
