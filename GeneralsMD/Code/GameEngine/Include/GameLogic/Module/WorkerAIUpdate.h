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

// FILE: WorkerAIUpdate.h //////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, June 2002
// Desc:   A Worker is a unit that is both a Dozer and a Supply Truck.  Holy Fuck.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "Common/StateMachine.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/DozerAIUpdate.h"
#include "GameLogic/Module/SupplyTruckAIUpdate.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
class WorkerStateMachine : public StateMachine
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( WorkerStateMachine, "WorkerStateMachine" );
public:
	WorkerStateMachine( Object *owner );

// state transition conditions
	static Bool supplyTruckSubMachineWantsToEnter( State *thisState, void* userData );
	static Bool supplyTruckSubMachineReadyToLeave( State *thisState, void* userData );

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
};

// ------------------------------------------------------------------------------------------------
/** NOTE: If you edit module data you must do it in both the Dozer *AND* the Worker */
// ------------------------------------------------------------------------------------------------
class WorkerAIUpdateModuleData : public AIUpdateModuleData
{

public:

	// !!!
	// !!! NOTE: If you edit module data you must do it in both the Dozer *AND* the Worker !!!
	// !!!
	Int m_maxBoxesData;
	Real m_repairHealthPercentPerSecond;	///< how many health points per second the dozer repairs at
	Real m_boredTime;											///< after this many frames, a dozer will try to find something to do on its own
	Real m_boredRange;										///< range the dozers try to auto repair when they're bored
	UnsignedInt m_centerDelay;
	UnsignedInt m_warehouseDelay;
	Real m_warehouseScanDistance;
 	AudioEventRTS m_suppliesDepletedVoice;						///< Sound played when I take the last box.
	Int m_upgradedSupplyBoost;

	WorkerAIUpdateModuleData()
	{
		m_maxBoxesData = 0;
		m_repairHealthPercentPerSecond = 0.0f;
		m_boredTime = 0.0f;
		m_boredRange = 0.0f;
		m_centerDelay = 0;
		m_warehouseDelay = 0;
		m_warehouseScanDistance = 100;
		m_upgradedSupplyBoost = 0;
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    AIUpdateModuleData::buildFieldParse(p);

		static const FieldParse dataFieldParse[] =
		{
			{ "MaxBoxes",					INI::parseInt,		nullptr, offsetof( WorkerAIUpdateModuleData, m_maxBoxesData ) },
			{ "RepairHealthPercentPerSecond",	INI::parsePercentToReal,	nullptr, offsetof( WorkerAIUpdateModuleData, m_repairHealthPercentPerSecond ) },
			{ "BoredTime",										INI::parseDurationReal,		nullptr, offsetof( WorkerAIUpdateModuleData, m_boredTime ) },
			{ "BoredRange",										INI::parseReal,						nullptr, offsetof( WorkerAIUpdateModuleData, m_boredRange ) },
			{ "SupplyCenterActionDelay", INI::parseDurationUnsignedInt, nullptr, offsetof( WorkerAIUpdateModuleData, m_centerDelay ) },
			{ "SupplyWarehouseActionDelay", INI::parseDurationUnsignedInt, nullptr, offsetof( WorkerAIUpdateModuleData, m_warehouseDelay ) },
			{ "SupplyWarehouseScanDistance", INI::parseReal, nullptr, offsetof( WorkerAIUpdateModuleData, m_warehouseScanDistance ) },
 			{ "SuppliesDepletedVoice", INI::parseAudioEventRTS, nullptr, offsetof( WorkerAIUpdateModuleData, m_suppliesDepletedVoice) },
 			{ "UpgradedSupplyBoost", INI::parseInt, nullptr, offsetof( WorkerAIUpdateModuleData, m_upgradedSupplyBoost) },
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);
	}
};

class WorkerAIInterface
{
	public:
		virtual void exitingSupplyTruckState() = 0; ///< This worker is leaving a supply truck task and should go back to Dozer mode.
};

class WorkerAIUpdate : public AIUpdateInterface, public DozerAIInterface, public SupplyTruckAIInterface, public WorkerAIInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( WorkerAIUpdate, "WorkerAIUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( WorkerAIUpdate, WorkerAIUpdateModuleData )

public:

	WorkerAIUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual DozerAIInterface* getDozerAIInterface() {return this;}
	virtual const DozerAIInterface* getDozerAIInterface() const {return this;}
	virtual SupplyTruckAIInterface* getSupplyTruckAIInterface() {return this;}
	virtual const SupplyTruckAIInterface* getSupplyTruckAIInterface() const {return this;}
	virtual WorkerAIInterface* getWorkerAIInterface() { return this; }
	virtual const WorkerAIInterface* getWorkerAIInterface() const { return this; }

	// Dozer side
	virtual void onDelete( void );

	virtual Real getRepairHealthPerSecond( void ) const;	///< get health to repair per second
	virtual Real getBoredTime( void ) const;							///< how long till we're bored
	virtual Real getBoredRange( void ) const;							///< when we're bored, we look this far away to do things

	virtual Object *construct( const ThingTemplate *what,
														 const Coord3D *pos, Real angle,
														 Player *owningPlayer,
														 Bool isRebuild );			///< construct a building

	// get task information
	virtual DozerTask getMostRecentCommand( void );				///< return task that was most recently issued
	virtual Bool isTaskPending( DozerTask task );					///< is there a desire to do the requested task
	virtual ObjectID getTaskTarget( DozerTask task );			///< get target of task
	virtual Bool isAnyTaskPending( void );								///< is there any dozer task pending
	virtual DozerTask getCurrentTask( void ) const { return m_currentTask; }	///< return the current task we're doing
	virtual void setCurrentTask( DozerTask task ) { m_currentTask = task; }		///< set the current task of the dozer

	virtual Bool getIsRebuild( void ) { return m_isRebuild; } ///< get whether or not our task is a rebuild.

	// task actions
	virtual void newTask( DozerTask task, Object* target );	///< set a desire to do the requrested task
	virtual void cancelTask( DozerTask task );						///< cancel this task from the queue, if it's the current task the dozer will stop working on it

	// internal methods to manage behavior from within the dozer state machine
	virtual void internalTaskComplete( DozerTask task );					///< set a dozer task as successfully completed
	virtual void internalCancelTask( DozerTask task );						///< cancel this task from the dozer
	virtual void internalTaskCompleteOrCancelled( DozerTask task );	///< this is called when tasks are cancelled or completed

	/** return a dock point for the action and task (if valid) ... note it can return nullptr
	if no point has been set for the combination of task and point */
	virtual const Coord3D* getDockPoint( DozerTask task, DozerDockPoint point );

	virtual void setBuildSubTask( DozerBuildSubTask subTask ) { m_buildSubTask = subTask; };
	virtual DozerBuildSubTask getBuildSubTask( void ) { return m_buildSubTask; }
	//
	// the following methods must be overridden so that if a player issues a command the dozer
	// can exit the internal state machine and do whatever the player says
	//
	virtual void aiDoCommand(const AICommandParms* parms);

// Supply truck stuff
	virtual Int getNumberBoxes() const { return m_numberBoxes; }
	virtual Bool loseOneBox();
	virtual Bool gainOneBox( Int remainingStock );

	virtual Bool isAvailableForSupplying() const;
	virtual Bool isCurrentlyFerryingSupplies() const;
	virtual Real getWarehouseScanDistance() const; ///< How far can I look for a warehouse?

	virtual void setForceBusyState(Bool v) { m_forcedBusyPending = v; }
	virtual Bool isForcedIntoBusyState() const { return m_forcedBusyPending; }

	virtual void setForceWantingState(Bool v){ m_forcePending = v; }
	virtual Bool isForcedIntoWantingState() const { return m_forcePending; }
	virtual ObjectID getPreferredDockID() const { return m_preferredDock; }
	virtual UnsignedInt getActionDelayForDock( Object *dock );

// worker specific
	Bool isSupplyTruckBrainActiveAndBusy();
	void resetSupplyTruckBrain();
	void resetDozerBrain();

	virtual void exitingSupplyTruckState(); ///< This worker is leaving a supply truck task and should go back to Dozer mode.

	virtual UpdateSleepTime update();				///< the update entry point

	// repairing
	virtual Bool canAcceptNewRepair( Object *obj );
	virtual void createBridgeScaffolding( Object *bridgeTower );
	virtual void removeBridgeScaffolding( Object *bridgeTower );

	virtual void startBuildingSound( const AudioEventRTS *sound, ObjectID constructionSiteID );
	virtual void finishBuildingSound();

	virtual Int getUpgradedSupplyBoost() const;

protected:

// Dozer data
	struct DozerTaskInfo
	{
		ObjectID m_targetObjectID;				///< target object ID of task
		UnsignedInt m_taskOrderFrame;			///< logic frame we decided we wanted to do this task
	} m_task[ DOZER_NUM_TASKS ];				///< tasks we want to do indexed by DozerTask


	DozerTask m_currentTask;						///< current task the dozer is attending to (if any)

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

// Supply Truck data
	Int m_numberBoxes;
	ObjectID									m_preferredDock;			///< Instead of searching, try this one first
	Bool m_forcePending; // To prevent a function from doing a setState, forceWanting will latch into here until serviced.
	Bool m_isRebuild;	// is our current construction task a rebuild?
	Bool m_forcedBusyPending;	// A supply truck can't tell the difference between Idle since
														// I'm between docking states, or a Stop command without help.


	WorkerStateMachine *m_workerMachine;
	// The two state machines are not in Worker's machine because I need to be able to accept
	// Dozer like commands while acting as a Supply Truck - that's how I switch.
	DozerPrimaryStateMachine *m_dozerMachine;  ///< the custom state machine for Dozer behavior
	SupplyTruckStateMachine *m_supplyTruckStateMachine;

	AudioEventRTS	m_buildingSound;			///< sound is pulled from the object we are building!

protected:
	virtual void privateRepair( Object *obj, CommandSourceType cmdSource );	///< repair the target
	virtual void privateResumeConstruction( Object *obj, CommandSourceType cmdSource );  ///< resume construction on obj
	virtual void privateDock( Object *obj, CommandSourceType cmdSource );
	virtual void privateIdle(CommandSourceType cmdSource);						///< Enter idle state.

private:

	void createMachines( void );		///< create our behavior machines we need
 	AudioEventRTS m_suppliesDepletedVoice;						///< Sound played when I take the last box.

};
