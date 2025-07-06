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

// FILE: BridgeScaffoldBehavior.h /////////////////////////////////////////////////////////////////
// Author: Colin Day, September 2002
// Desc:   Bridge scaffold
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __BRIDGE_SCAFFOLD_BEHAVIOR_H_
#define __BRIDGE_SCAFFOLD_BEHAVIOR_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/UpdateModule.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
enum ScaffoldTargetMotion CPP_11(: Int)
{
	STM_STILL,
	STM_RISE,
	STM_BUILD_ACROSS,
	STM_TEAR_DOWN_ACROSS,
	STM_SINK,
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class BridgeScaffoldBehaviorInterface
{

public:

	virtual void setPositions( const Coord3D *createPos,
														 const Coord3D *riseToPos,
														 const Coord3D *buildPos ) = 0;
	virtual void setMotion( ScaffoldTargetMotion targetMotion ) = 0;
	virtual ScaffoldTargetMotion getCurrentMotion( void ) = 0;
	virtual void reverseMotion( void ) = 0;
	virtual void setLateralSpeed( Real lateralSpeed ) = 0;
	virtual void setVerticalSpeed( Real verticalSpeed ) = 0;

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class BridgeScaffoldBehavior : public UpdateModule,
															 public BridgeScaffoldBehaviorInterface
{

	MAKE_STANDARD_MODULE_MACRO( BridgeScaffoldBehavior );
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( BridgeScaffoldBehavior, "BridgeScaffoldBehavior" )

public:

	BridgeScaffoldBehavior( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	// behavior module methods
	virtual BridgeScaffoldBehaviorInterface* getBridgeScaffoldBehaviorInterface() { return this; }

	// update methods
	virtual UpdateSleepTime update( void );

	// bridge scaffold interface methods
	virtual void setPositions( const Coord3D *createPos,
														 const Coord3D *riseToPos,
														 const Coord3D *buildPos );
	virtual void setMotion( ScaffoldTargetMotion targetMotion );
	virtual ScaffoldTargetMotion getCurrentMotion( void ) { return m_targetMotion; }
	virtual void reverseMotion( void );
	virtual void setLateralSpeed( Real lateralSpeed ) { m_lateralSpeed = lateralSpeed; }
	virtual void setVerticalSpeed( Real verticalSpeed ) { m_verticalSpeed = verticalSpeed; }

	// public interface acquisition
	static BridgeScaffoldBehaviorInterface *getBridgeScaffoldBehaviorInterfaceFromObject( Object *obj );

protected:

	void doVerticalMotion( void );				///< do rise/sink vertical motion
	void doLateralmotion( void );					///< do lateral motion

	ScaffoldTargetMotion m_targetMotion;	///< which way our motion should be going (build up, still, tear down etc)
	Coord3D m_createPos;									///< initial position of object creation (in ground)
	Coord3D m_riseToPos;									///< position we "rise to" out of the ground
	Coord3D m_buildPos;										///< position we move to and stop at on the bridge surface
	Real m_lateralSpeed;									///< speed for lateral motions
	Real m_verticalSpeed;									///< speed for vertical motions
	Coord3D m_targetPos;									///< current target position for our motion type

};


#endif  // end __BRIDGE_SCAFFOLD_BEHAVIOR_H_
