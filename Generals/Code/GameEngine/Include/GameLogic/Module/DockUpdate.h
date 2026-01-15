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

// FILE: DockUpdate.h /////////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood Feb 2002
// Desc:   Behavior common to all DockUpdates is here.  Everything but action()
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/INI.h"
#include "Common/GameMemory.h"
#include "Common/STLTypedefs.h"
#include "GameLogic/Module/UpdateModule.h"

enum
{
	DEFAULT_APPROACH_VECTOR_SIZE = 10
};

enum
{
	DYNAMIC_APPROACH_VECTOR_FLAG = -1
};

//-------------------------------------------------------------------------------------------------
class DockUpdateModuleData : public UpdateModuleData
{
public:
	Int m_numberApproachPositionsData; // A positive number is an absolute, DYNAMIC_APPROACH_VECTOR_FLAG means dynamic vector
	Bool m_isAllowPassthrough;

	DockUpdateModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

//-------------------------------------------------------------------------------------------------
class DockUpdate : public UpdateModule , public DockUpdateInterface
{

	MEMORY_POOL_GLUE_ABC( DockUpdate )
	MAKE_STANDARD_MODULE_MACRO_ABC( DockUpdate )
	MAKE_STANDARD_MODULE_DATA_MACRO_ABC( DockUpdate, DockUpdateModuleData )

public:

	DockUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor provided by memory pool object

	/** Returns true if it is okay for the docker to approach and prepare to dock.
			False could mean the queue is full, for example.
	*/
	virtual Bool isClearToApproach( Object const* docker ) const;

	/** Give me a Queue point to drive to, and record that that point is taken.
			Returning null means there are none free
	*/
	virtual Bool reserveApproachPosition( Object* docker, Coord3D *position, Int *index );

	/** Give me the next Queue point to drive to, and record that that point is taken.
	*/
	virtual Bool advanceApproachPosition( Object* docker, Coord3D *position, Int *index );

	/** Return true when it is OK for docker to begin entering the dock
			The Dock will lift the restriction on one particular docker on its own,
			so you must continually ask.
	*/
	virtual Bool isClearToEnter( Object const* docker ) const;

	/** Return true when it is OK for docker to request a new Approach position.  The dock is in
			charge of keeping track of holes in the line, but the docker will remind us of their spot.
	*/
	virtual Bool isClearToAdvance( Object const* docker, Int dockerIndex ) const;

	/** Give me the point that is the start of your docking path
			Returning null means there is none free
			All functions take docker as arg so we could have multiple docks on a building.
			Docker is not assumed, it is recorded and checked.
	*/
	virtual void getEnterPosition( Object* docker, Coord3D *position );

	/** Give me the middle point of the dock process where the action() happens */
	virtual void getDockPosition( Object* docker, Coord3D *position );

	/** Give me the point to drive to when I am done */
	virtual void getExitPosition( Object* docker, Coord3D *position );

	virtual void onApproachReached( Object* docker );		///< I have reached the Approach Point.
	virtual void onEnterReached( Object* docker );			///< I have reached the Enter Point.
	virtual void onDockReached( Object* docker );				///< I have reached the Dock point
	virtual void onExitReached( Object* docker );				///< I have reached the exit.  You are no longer busy

	//The fact that action() is not here is intentional.  This object cannot exist.  You must
	//derive off it and implement action().

	virtual void cancelDock( Object* docker );	///< Clear me from any reserved points, and if I was the reason you were Busy, you aren't anymore.

	virtual Bool isDockOpen( void ) { return m_dockOpen; }				///< Is the dock open to accepting dockers
	virtual void setDockOpen( Bool open ) { m_dockOpen = open; }	///< Open/Close the dock

	virtual Bool isAllowPassthroughType();	///< Not all docks allow you to path through them in your AIDock machine

	virtual Bool isRallyPointAfterDockType(){return FALSE;} ///< A minority of docks want to give you a final command to their rally point

	virtual void setDockCrippled( Bool setting ); ///< Game Logic can set me as inoperative.  I get to decide what that means.

	virtual UpdateSleepTime update();	///< In charge of lifting dock restriction for one registered as Approached if all is ready

protected:

	// These are const data read from the drawable when first asked for them
	Coord3D m_enterPosition;
	Coord3D m_dockPosition;
	Coord3D m_exitPosition;
	Int m_numberApproachPositions;
	Int m_numberApproachPositionBones;

	// These are real variables local to my specific needs here in DockUpdate
	Bool m_positionsLoaded;	///< FALSE until we have loaded all the docking positions

	VecCoord3D m_approachPositions;
	ObjectIDVector m_approachPositionOwners;	///< Who is in or at least reserved each spot
	BoolVector m_approachPositionReached;			///< Which positions have actually been reached

	ObjectID m_activeDocker;	///< we could expand this to multiple dock paths since we always get docker in our methods
	Bool m_dockerInside; ///< This is true while our active docker is between Enter and Exit. This is shorter than activeDocker's lifetime as it doesn't include approach to enter
	Bool m_dockCrippled; ///< Has game logic set me as crippled?

	Bool m_dockOpen;  ///< Is the dock open for dockers

	void loadDockPositions();  ///< load all the dock positions
	Coord3D computeApproachPosition( Int positionIndex, Object *forWhom ); ///< Do a smart lookup of this bone position
};
