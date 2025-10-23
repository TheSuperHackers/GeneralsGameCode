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

// SupplyTruckAIUpdate.h ////////////
// Author: Graham Smallwood, February 2002
// Desc:   State machine that controls when and with who a Truck docks

#pragma once

#include "Common/StateMachine.h"
#include "GameLogic/Module/AIUpdate.h"

//-------------------------------------------------------------------------------------------------
class SupplyTruckStateMachine : public StateMachine
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( SupplyTruckStateMachine, "SupplyTruckStateMachine" );
public:
	SupplyTruckStateMachine( Object *owner );

// state transition conditions

	static Bool ownerDocking( State *thisState, void* userData );
	static Bool ownerIdle( State *thisState, void* userData );
	static Bool ownerAvailableForSupplying( State *thisState, void* userData );
	static Bool ownerNotDockingOrIdle( State *thisState, void* userData );
	static Bool isForcedIntoWantingState( State *thisState, void* userData );
	static Bool isForcedIntoBusyState( State *thisState, void* userData );

protected:
	// snapshot interface
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess();
};

//-------------------------------------------------------------------------------------------------
class SupplyTruckWantsToPickUpOrDeliverBoxesState :  public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(SupplyTruckWantsToPickUpOrDeliverBoxesState, "SupplyTruckWantsToPickUpOrDeliverBoxesState")
protected:
	// snapshot interface STUBBED.
	virtual void crc( Xfer *xfer ){};
	virtual void xfer( Xfer *xfer ){XferVersion cv = 1;	XferVersion v = cv; xfer->xferVersion( &v, cv );}
	virtual void loadPostProcess(){};

public:
	SupplyTruckWantsToPickUpOrDeliverBoxesState( StateMachine *machine ) : State( machine, "SupplyTruckWantsToPickUpOrDeliverBoxesState" ) {}
	virtual StateReturnType update();
	virtual StateReturnType onEnter();
	virtual void onExit(StateExitType status);
};
EMPTY_DTOR(SupplyTruckWantsToPickUpOrDeliverBoxesState)

//-------------------------------------------------------------------------------------------------
class RegroupingState :  public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(RegroupingState, "RegroupingState")
protected:
	// snapshot interface STUBBED.
	virtual void crc( Xfer *xfer ){};
	virtual void xfer( Xfer *xfer ){XferVersion cv = 1;	XferVersion v = cv; xfer->xferVersion( &v, cv );}
	virtual void loadPostProcess(){};
public:
	RegroupingState( StateMachine *machine ) : State( machine, "RegroupingState" ) {}
	virtual StateReturnType update(){ return STATE_CONTINUE;}// Nothing to do but wait for a transition
	virtual StateReturnType onEnter();// Will tell me to aiMove back to base.
	virtual void onExit(StateExitType status);
};
EMPTY_DTOR(RegroupingState)

//-------------------------------------------------------------------------------------------------
class DockingState :  public State
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(DockingState, "DockingState")
protected:
	// snapshot interface STUBBED.
	virtual void crc( Xfer *xfer ){};
	virtual void xfer( Xfer *xfer ){XferVersion cv = 1;	XferVersion v = cv; xfer->xferVersion( &v, cv );}
	virtual void loadPostProcess(){};
public:
	DockingState( StateMachine *machine ) :State( machine, "DockingState" ) {}
	virtual StateReturnType update();
	virtual StateReturnType onEnter();
	virtual void onExit(StateExitType status);
};
EMPTY_DTOR(DockingState)

//-------------------------------------------------------------------------------------------------
enum
{
	ST_IDLE,						///< Not doing anything.  Should I autopilot?
	ST_BUSY,						///< Direct player involvement (move) has taken me off autopilot
	ST_WANTING,					///< Search for warehouse or center and dock with it
	ST_REGROUPING,			///< Wanting failed, so hang out at base until something changes.  Autopilot will turn off.
	ST_DOCKING					///< Docking substates are running, wait for them to finish
};

//-------------------------------------------------------------------------------------------------
class SupplyTruckAIUpdateModuleData : public AIUpdateModuleData
{
public:
	Int m_maxBoxesData;
	UnsignedInt m_centerDelay;
	UnsignedInt m_warehouseDelay;
	Real m_warehouseScanDistance;
 	AudioEventRTS m_suppliesDepletedVoice;						///< Sound played when I take the last box.

	SupplyTruckAIUpdateModuleData()
	{
		m_maxBoxesData = 0;
		m_centerDelay = 0;
		m_warehouseDelay = 0;
		m_warehouseScanDistance = 100;
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    AIUpdateModuleData::buildFieldParse(p);

		static const FieldParse dataFieldParse[] =
		{
			{ "MaxBoxes",					INI::parseInt,		NULL, offsetof( SupplyTruckAIUpdateModuleData, m_maxBoxesData ) },
			{ "SupplyCenterActionDelay", INI::parseDurationUnsignedInt, NULL, offsetof( SupplyTruckAIUpdateModuleData, m_centerDelay ) },
			{ "SupplyWarehouseActionDelay", INI::parseDurationUnsignedInt, NULL, offsetof( SupplyTruckAIUpdateModuleData, m_warehouseDelay ) },
			{ "SupplyWarehouseScanDistance", INI::parseReal, NULL, offsetof( SupplyTruckAIUpdateModuleData, m_warehouseScanDistance ) },
 			{ "SuppliesDepletedVoice", INI::parseAudioEventRTS, NULL, offsetof( SupplyTruckAIUpdateModuleData, m_suppliesDepletedVoice) },
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);

	}
};

//-------------------------------------------------------------------------------------------------
class SupplyTruckAIInterface
{
	// This is no longer a leaf behavior.  Someone else needs to combine this
	// with another major AIUpdate.  So provide an interface to satisfy the people
	// who look this up by name.
public:
	virtual Int getNumberBoxes() const = 0;
	virtual Bool loseOneBox() = 0;
	virtual Bool gainOneBox( Int remainingStock ) = 0;

	// returns true if we can fetch/deliver supplies. normally returns true
	// if the AI is idle, but subclasses might add further restrictions.
	virtual Bool isAvailableForSupplying() const = 0;
	virtual Bool isCurrentlyFerryingSupplies() const = 0;
	virtual Real getWarehouseScanDistance() const = 0; ///< How far can I look for a warehouse?

	virtual void setForceWantingState(Bool v) = 0;
	virtual Bool isForcedIntoWantingState() const = 0;
	virtual void setForceBusyState(Bool v) = 0;
	virtual Bool isForcedIntoBusyState() const = 0;
	virtual ObjectID getPreferredDockID() const = 0;
	virtual UnsignedInt getActionDelayForDock( Object *dock ) = 0;
};

//-------------------------------------------------------------------------------------------------
class SupplyTruckAIUpdate : public AIUpdateInterface, public SupplyTruckAIInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( SupplyTruckAIUpdate, "SupplyTruckAIUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( SupplyTruckAIUpdate, SupplyTruckAIUpdateModuleData )

private:

public:

	SupplyTruckAIUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual SupplyTruckAIInterface* getSupplyTruckAIInterface() {return this;}
	virtual const SupplyTruckAIInterface* getSupplyTruckAIInterface() const {return this;}

	virtual Int getNumberBoxes() const { return m_numberBoxes; }
	virtual Bool loseOneBox();
	virtual Bool gainOneBox( Int remainingStock );

	// this is present for subclasses (eg, Chinook) to override, to
	// prevent supply-ferry behavior in some cases (eg, when toting passengers)
	virtual Bool isAvailableForSupplying() const;
	virtual Bool isCurrentlyFerryingSupplies() const;
	virtual Real getWarehouseScanDistance() const; ///< How far can I look for a warehouse?

	virtual void setForceWantingState(Bool v) { m_forcePending = v; } // When a Supply Center creates us (or maybe other sources later), we need to hop into autopilot mode.
	virtual Bool isForcedIntoWantingState() const { return m_forcePending; }

	virtual void setForceBusyState(Bool v) { m_forcedBusyPending = v; }
	virtual Bool isForcedIntoBusyState() const { return m_forcedBusyPending; }

	virtual ObjectID getPreferredDockID() const { return m_preferredDock; }
	virtual UnsignedInt getActionDelayForDock( Object *dock );

	virtual UpdateSleepTime update();

protected:

	virtual AIStateMachine* makeStateMachine();
	virtual void privateDock( Object *obj, CommandSourceType cmdSource );
	virtual void privateIdle(CommandSourceType cmdSource);						///< Enter idle state.

private:
	SupplyTruckStateMachine*	m_supplyTruckStateMachine;
	ObjectID									m_preferredDock;			///< Instead of searching, try this one first
	Int												m_numberBoxes;
	Bool											m_forcePending;				// To prevent a function from doing a setState,
																									// forceWanting will latch into here until serviced.
	Bool											m_forcedBusyPending;	// A supply truck can't tell the difference between Idle since
																									// I'm between docking states, or a Stop command without help.
 	AudioEventRTS m_suppliesDepletedVoice;						///< Sound played when I take the last box.

};
