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

// FILE: POWTruckAIUpdate.h ///////////////////////////////////////////////////////////////////////
// Author: Colin Day
// Desc:   AI for the POW Truck
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __POW_TRUCK_AI_UPDATE_H_
#define __POW_TRUCK_AI_UPDATE_H_

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/AIUpdate.h"

#ifdef ALLOW_SURRENDER

// ------------------------------------------------------------------------------------------------
/** NOTE: If you edit module data you must do it in both the Dozer *AND* the Worker */
// ------------------------------------------------------------------------------------------------
class POWTruckAIUpdateModuleData : public AIUpdateModuleData
{

public:

	POWTruckAIUpdateModuleData( void );

	static void buildFieldParse( MultiIniFieldParse &p );

	UnsignedInt m_boredTimeInFrames;		///< after this long we seek out targets in AUTOMATIC mode
	Real m_hangAroundPrisonDistance;		///< this close is considered "at the prison" for purposes of waiting

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
enum POWTruckTask CPP_11(: Int)	// Stored in save file, do not renumber.  jba.
{
	POW_TRUCK_TASK_WAITING						= 0, ///< Waiting for something to do
	POW_TRUCK_TASK_FIND_TARGET				= 1, ///< We need to search out a target to collect
	POW_TRUCK_TASK_COLLECTING_TARGET	= 2, ///< collecting a targeted POW
	POW_TRUCK_TASK_RETURNING_PRISONERS= 3	///< return all POWs to base
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class POWTruckAIUpdateInterface
{

public:

	virtual void setTask( POWTruckTask task, Object *taskObject = NULL ) = 0;
	virtual POWTruckTask getCurrentTask( void ) = 0;
	virtual void loadPrisoner( Object *prisoner ) = 0;
	virtual void unloadPrisonersToPrison( Object *prison ) = 0;

};

//-------------------------------------------------------------------------------------------------
/** The Dozer AI Update interface.  Dozers are workers that are capable of building all the
	* structures available to a player, as well as repairing building, and fortifying
	* civilian structures */
//-------------------------------------------------------------------------------------------------
class POWTruckAIUpdate : public AIUpdateInterface,
												 public POWTruckAIUpdateInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( POWTruckAIUpdate, "POWTruckAIUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( POWTruckAIUpdate, POWTruckAIUpdateModuleData )

public:

	POWTruckAIUpdate( Thing *thing, const ModuleData *moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual void onDelete( void );
	virtual UpdateSleepTime update( void );

	// Pow truck ai interface
	virtual POWTruckAIUpdateInterface *getPOWTruckAIUpdateInterface( void ) { return this; }
	virtual POWTruckTask getCurrentTask( void ) { return m_currentTask; }
	virtual void loadPrisoner( Object *prisoner );
	virtual void unloadPrisonersToPrison( Object *prison );

	//
	// the following methods must be overridden so that if a player issues a command the dozer
	// can exit the internal state machine and do whatever the player says
	//
	virtual void aiDoCommand( const AICommandParms *parms );

protected:

	virtual void setTask( POWTruckTask task, Object *taskObject = NULL );		///< set our current task

	enum POWTruckAIMode	// Stored in save file, do not renumber.  jba.
	{
		AUTOMATIC = 0,
		MANUAL		= 1
	};
	virtual void setAIMode( POWTruckAIMode mode );	///< put truck in automatic or manual mode

	virtual void updateWaiting( void );
	virtual void updateFindTarget( void );
	virtual void updateCollectingTarget( void );
	virtual void updateReturnPrisoners( void );

	virtual Bool validateTarget( const Object *target );	///< is 'target' a valid object to collect
	virtual void doReturnPrisoners( void );								///< initiate a return prisoners to base
	virtual void doReturnToPrison( Object *prison );			///< initiate a return to prison (no prisoner dump off)
	virtual Object *findBestPrison( void );								///< find the best prison for us given our current position
	virtual Object *findBestTarget( void );								///< find the best prisoner for us given our current situation

	// AIUpdateInterface implementations
	virtual void privatePickUpPrisoner( Object *prisoner, CommandSourceType cmdSource );
	virtual void privateReturnPrisoners( Object *prison, CommandSourceType cmdSource );

	POWTruckAIMode m_aiMode;						///< Type of AI, automatic or explicitly controlled be player
	POWTruckTask m_currentTask;					///< our current task
	ObjectID m_targetID;								///< Object ID of the target we're going to collect
	ObjectID m_prisonID;								///< Prison we're going back to
	UnsignedInt m_enteredWaitingFrame;	///< frame we entered the waiting task
	UnsignedInt m_lastFindFrame;				///< so that we don't search for targets too much

};

#endif

#endif  // end __POW_TRUCK_AI_UPDATE_H_
