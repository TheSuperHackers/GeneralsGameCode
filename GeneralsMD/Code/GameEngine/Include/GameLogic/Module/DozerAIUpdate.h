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

// FILE: DozerAIUpdate.h //////////////////////////////////////////////////////////////////////////
// Author: Colin Day, February 2002
// Desc:   Dozer AI behavior
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __DOZERAIUPDATE_H_
#define __DOZERAIUPDATE_H_

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/AIUpdate.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class AudioEventRTS;

//-------------------------------------------------------------------------------------------------
/** The Dozer primary state machine */
//-------------------------------------------------------------------------------------------------
class DozerPrimaryStateMachine : public StateMachine
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( DozerPrimaryStateMachine, "DozerPrimaryStateMachine" );

public:

  DozerPrimaryStateMachine( Object *owner );
	// virtual destructor prototypes provided by memory pool object

	//-----------------------------------------------------------------------------------------------
	// state transition conditions
	static Bool isBuildMostImportant( State *thisState, void* userData );
	static Bool isRepairMostImportant( State *thisState, void* userData );
	static Bool isFortifyMostImportant( State *thisState, void* userData );

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
};


//-------------------------------------------------------------------------------------------------
/** Dozer behaviors that use action sub state machines */
//-------------------------------------------------------------------------------------------------
enum DozerTask CPP_11(: Int) // These enums are saved in the game save file, so DO NOT renumber them. jba.
{
	DOZER_TASK_INVALID = -1,
	DOZER_TASK_FIRST = 0,
	DOZER_TASK_BUILD = DOZER_TASK_FIRST,		///< go build something
	DOZER_TASK_REPAIR = 1,											///< go repair something
	DOZER_TASK_FORTIFY = 2,											///< go fortify something

	DOZER_NUM_TASKS													// keep this last
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
enum DozerDockPoint CPP_11(: Int)	 // These enums are saved in the game save file, so DO NOT renumber them. jba.
{
	DOZER_DOCK_POINT_START	= 0,
	DOZER_DOCK_POINT_ACTION	= 1,
	DOZER_DOCK_POINT_END		= 2,
	DOZER_NUM_DOCK_POINTS  // keep this one last
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
enum DozerBuildSubTask CPP_11(: Int)  // These enums are saved in the game save file, so DO NOT renumber them. jba.
{
	DOZER_SELECT_BUILD_DOCK_LOCATION		= 0,
	DOZER_MOVING_TO_BUILD_DOCK_LOCATION	=	1,
	DOZER_DO_BUILD_AT_DOCK							= 2
};

// ------------------------------------------------------------------------------------------------
/** This is no longer a leaf behavior.  Someone else needs to combine this
	* with another major AIUpdate.  So provide an interface to satisfy the people
	* who look this up by name. */
// ------------------------------------------------------------------------------------------------
class DozerAIInterface
{

	// This is no longer a leaf behavior.  Someone else needs to combine this
	// with another major AIUpdate.  So provide an interface to satisfy the people
	// who look this up by name.

public:

	virtual void onDelete( void ) = 0;

	virtual Real getRepairHealthPerSecond( void ) const = 0;	///< get health to repair per second
	virtual Real getBoredTime( void ) const = 0;							///< how long till we're bored
	virtual Real getBoredRange( void ) const = 0;							///< when we're bored, we look this far away to do things

	// methods to override for the dozer behaviors
	virtual Object *construct( const ThingTemplate *what,
														 const Coord3D *pos, Real angle,
														 Player *owningPlayer,
														 Bool isRebuild ) = 0;


	// get task information
	virtual DozerTask getMostRecentCommand( void ) = 0;				///< return task that was most recently issued
	virtual Bool isTaskPending( DozerTask task ) = 0;					///< is there a desire to do the requested task
	virtual ObjectID getTaskTarget( DozerTask task ) = 0;			///< get target of task
	virtual Bool isAnyTaskPending( void ) = 0;								///< is there any dozer task pending

	virtual DozerTask getCurrentTask( void ) const = 0;							///< return the current task we're doing
	// the following should only be used from inside the Dozer state machine!
	// !!! *DO NOT CALL THIS AND SET THE TASK DIRECTLY TO AFFECT BEHAVIOR* !!! ///
	virtual void setCurrentTask( DozerTask task ) = 0;				///< set the current task of the dozer

	virtual Bool getIsRebuild( void ) = 0;										///< get whether or not this is a rebuild.

	// task actions
	virtual void newTask( DozerTask task, Object *target ) = 0;	///< set a desire to do the requrested task
	virtual void cancelTask( DozerTask task ) = 0;							///< cancel this task from the queue, if it's the current task the dozer will stop working on it

	// internal methods to manage behavior from within the dozer state machine
	virtual void internalTaskComplete( DozerTask task ) = 0;					///< set a dozer task as successfully completed
	virtual void internalCancelTask( DozerTask task ) = 0;						///< cancel this task from the dozer
	virtual void internalTaskCompleteOrCancelled( DozerTask task ) = 0;	///< this is called when tasks are cancelled or completed

	/** return a dock point for the action and task (if valid) ... note it can return NULL
	if no point has been set for the combination of task and point */
	virtual const Coord3D* getDockPoint( DozerTask task, DozerDockPoint point ) = 0;

	virtual void setBuildSubTask( DozerBuildSubTask subTask ) = 0;
	virtual DozerBuildSubTask getBuildSubTask( void ) = 0;

	// repairing
	virtual Bool canAcceptNewRepair( Object *obj ) = 0;
	virtual void createBridgeScaffolding( Object *bridgeTower ) = 0;
	virtual void removeBridgeScaffolding( Object *bridgeTower ) = 0;

	virtual void startBuildingSound( const AudioEventRTS *sound, ObjectID constructionSiteID ) = 0;
	virtual void finishBuildingSound() = 0;

};

// ------------------------------------------------------------------------------------------------
/** NOTE: If you edit module data you must do it in both the Dozer *AND* the Worker */
// ------------------------------------------------------------------------------------------------
class DozerAIUpdateModuleData : public AIUpdateModuleData
{

public:

	DozerAIUpdateModuleData( void );

	// !!!
	// !!! NOTE: If you edit module data you must do it in both the Dozer *AND* the Worker !!!
	// !!!

	Real m_repairHealthPercentPerSecond;	///< how many health points per second the dozer repairs at
	Real m_boredTime;											///< after this many frames, a dozer will try to find something to do on its own
	Real m_boredRange;										///< range the dozers try to auto repair when they're bored

	static void buildFieldParse( MultiIniFieldParse &p );

};

//-------------------------------------------------------------------------------------------------
/** The Dozer AI Update interface.  Dozers are workers that are capable of building all the
	* structures available to a player, as well as repairing building, and fortifying
	* civilian structures */
//-------------------------------------------------------------------------------------------------
class DozerAIUpdate : public AIUpdateInterface, public DozerAIInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( DozerAIUpdate, "DozerAIUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( DozerAIUpdate, DozerAIUpdateModuleData )

public:
	static Bool findGoodBuildOrRepairPosition(const Object* me, const Object* target, Coord3D& positionOut);
	static Object* findGoodBuildOrRepairPositionAndTarget(Object* me, Object* target, Coord3D& positionOut);

public:

	DozerAIUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual DozerAIInterface* getDozerAIInterface() {return this;}
	virtual const DozerAIInterface* getDozerAIInterface() const {return this;}

	virtual void onDelete( void );

	//
	// module data methods ... this is LAME, multiple inheritance off an interface with replicated
	// data and code, ick!
	// NOTE: If you edit module data you must do it in both the Dozer *AND* the Worker
	//
	virtual Real getRepairHealthPerSecond( void ) const;	///< get health to repair per second
	virtual Real getBoredTime( void ) const;							///< how long till we're bored
	virtual Real getBoredRange( void ) const;							///< when we're bored, we look this far away to do things

	// methods to override for the dozer behaviors
	virtual Object* construct( const ThingTemplate *what,
														 const Coord3D *pos, Real angle,
														 Player *owningPlayer,
														 Bool isRebuild );								///< construct an object


	// get task information
	virtual DozerTask getMostRecentCommand( void );				///< return task that was most recently issued
	virtual Bool isTaskPending( DozerTask task );					///< is there a desire to do the requested task
	virtual ObjectID getTaskTarget( DozerTask task );			///< get target of task
	virtual Bool isAnyTaskPending( void );								///< is there any dozer task pending
	virtual DozerTask getCurrentTask( void ) const { return m_currentTask; }	///< return the current task we're doing
	virtual void setCurrentTask( DozerTask task ) { m_currentTask = task; }	///< set the current task of the dozer

	virtual Bool getIsRebuild( void ) { return m_isRebuild; }	///< get whether or not this building is a rebuild.

	// task actions
	virtual void newTask( DozerTask task, Object *target );	///< set a desire to do the requrested task
	virtual void cancelTask( DozerTask task );							///< cancel this task from the queue, if it's the current task the dozer will stop working on it

	// internal methods to manage behavior from within the dozer state machine
	virtual void internalTaskComplete( DozerTask task );					///< set a dozer task as successfully completed
	virtual void internalCancelTask( DozerTask task );						///< cancel this task from the dozer
	virtual void internalTaskCompleteOrCancelled( DozerTask task );	///< this is called when tasks are cancelled or completed

	/** return a dock point for the action and task (if valid) ... note it can return NULL
	if no point has been set for the combination of task and point */
	virtual const Coord3D* getDockPoint( DozerTask task, DozerDockPoint point );

	virtual void setBuildSubTask( DozerBuildSubTask subTask ) { m_buildSubTask = subTask; };
	virtual DozerBuildSubTask getBuildSubTask( void ) { return m_buildSubTask; }

	virtual UpdateSleepTime update( void );											///< the update entry point

	// repairing
	virtual Bool canAcceptNewRepair( Object *obj );
	virtual void createBridgeScaffolding( Object *bridgeTower );
	virtual void removeBridgeScaffolding( Object *bridgeTower );

	virtual void startBuildingSound( const AudioEventRTS *sound, ObjectID constructionSiteID );
	virtual void finishBuildingSound();

	//
	// the following methods must be overridden so that if a player issues a command the dozer
	// can exit the internal state machine and do whatever the player says
	//
	virtual void aiDoCommand(const AICommandParms* parms);


protected:

	virtual void privateRepair( Object *obj, CommandSourceType cmdSource );	///< repair the target
	virtual void privateResumeConstruction( Object *obj, CommandSourceType cmdSource );  ///< resume construction on obj

	struct DozerTaskInfo
	{
		ObjectID m_targetObjectID;				///< target object ID of task
		UnsignedInt m_taskOrderFrame;			///< logic frame we decided we wanted to do this task
	} m_task[ DOZER_NUM_TASKS ];				///< tasks we want to do indexed by DozerTask

	DozerPrimaryStateMachine *m_dozerMachine;  ///< the custom state machine for Dozer behavior
	DozerTask m_currentTask;						///< current task the dozer is attending to (if any)
	AudioEventRTS	m_buildingSound;			///< sound is pulled from the object we are building!
	Bool m_isRebuild;										///< is this a rebuild of a previous building?

	//
	// the following info array can be used if we want to have more complicated approaches
	// to our target depending on our task
	//
	struct DozerDockPointInfo
	{
		Bool valid;						///< this point has been set and is valid
		Coord3D location;			///< WORLD location
	} m_dockPoint[ DOZER_NUM_TASKS ][ DOZER_NUM_DOCK_POINTS ];

	DozerBuildSubTask m_buildSubTask;		///< for building and actually docking for the build

private:

	void createMachines( void );		///< create our behavior machines we need

};


#endif // __DOZERAIUPDATE_H_

