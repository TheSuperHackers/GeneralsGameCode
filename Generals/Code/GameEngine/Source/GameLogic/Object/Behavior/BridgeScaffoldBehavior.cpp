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

// FILE: BridgeScaffoldBehavior.cpp ///////////////////////////////////////////////////////////////
// Author: Colin Day, September 2002
// Desc:   Bridge scaffold
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"
#include "Common/Xfer.h"
#include "GameClient/InGameUI.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/BridgeScaffoldBehavior.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
BridgeScaffoldBehavior::BridgeScaffoldBehavior( Thing *thing, const ModuleData *moduleData )
											: UpdateModule( thing, moduleData )
{

	m_targetMotion = STM_STILL;
	m_createPos.zero();
	m_riseToPos.zero();
	m_buildPos.zero();
	m_targetPos.zero();
	m_lateralSpeed = 1.0f;
	m_verticalSpeed = 1.0f;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
BridgeScaffoldBehavior::~BridgeScaffoldBehavior( void )
{

}

// ------------------------------------------------------------------------------------------------
/** Set all of the target positions that we're care about as a moving scaffold object */
// ------------------------------------------------------------------------------------------------
void BridgeScaffoldBehavior::setPositions( const Coord3D *createPos,
																					 const Coord3D *riseToPos,
																					 const Coord3D *buildPos )
{

	m_createPos = *createPos;
	m_riseToPos = *riseToPos;
	m_buildPos = *buildPos;

}

// ------------------------------------------------------------------------------------------------
/** Set us moving to the right target position for the requested motion type */
// ------------------------------------------------------------------------------------------------
void BridgeScaffoldBehavior::setMotion( ScaffoldTargetMotion targetMotion )
{

	// save the target motion type
	m_targetMotion = targetMotion;

	// given the target motion, pick a destination target
	switch( m_targetMotion )
	{

		// --------------------------------------------------------------------------------------------
		case STM_RISE:
		case STM_TEAR_DOWN_ACROSS:
			m_targetPos = m_riseToPos;
			break;

		// --------------------------------------------------------------------------------------------
		case STM_BUILD_ACROSS:
			m_targetPos = m_buildPos;
			break;

		// --------------------------------------------------------------------------------------------
		case STM_SINK:
			m_targetPos = m_createPos;
			break;

	}

}

// ------------------------------------------------------------------------------------------------
/** Whatever our current state of motion is, reverse it */
// ------------------------------------------------------------------------------------------------
void BridgeScaffoldBehavior::reverseMotion( void )
{

	switch( m_targetMotion )
	{

		case STM_STILL:
			setMotion( STM_TEAR_DOWN_ACROSS );
			break;

		case STM_RISE:
			setMotion( STM_SINK );
			break;

		case STM_BUILD_ACROSS:
			setMotion( STM_TEAR_DOWN_ACROSS );
			break;

		case STM_TEAR_DOWN_ACROSS:
			setMotion( STM_BUILD_ACROSS );
			break;

		case STM_SINK:
			setMotion( STM_RISE );
			break;

	}

}

// ------------------------------------------------------------------------------------------------
/** The update method */
// ------------------------------------------------------------------------------------------------
UpdateSleepTime BridgeScaffoldBehavior::update( void )
{

	// do nothing if we're not in motion
	if( m_targetMotion == STM_STILL )
		return UPDATE_SLEEP_NONE;

	// get our info
	Object *us = getObject();
	const Coord3D *ourPos = us->getPosition();

	// compute direction vector from our position to the target position
	Coord3D dirV;
	dirV.x = m_targetPos.x - ourPos->x;
	dirV.y = m_targetPos.y - ourPos->y;
	dirV.z = m_targetPos.z - ourPos->z;

	// use normalized direction vector "v" to do the pulling movement
	Coord3D v = dirV;
	v.normalize();

	// depending on our motion type, we move at different speeds
	Real topSpeed = 1.0f;
	Coord3D *start, *end;
	switch( m_targetMotion )
	{

		case STM_RISE:
			topSpeed = m_verticalSpeed;
			start = &m_createPos;
			end = &m_riseToPos;
			break;

		case STM_SINK:
			topSpeed = m_verticalSpeed;
			start = &m_riseToPos;
			end = &m_createPos;
			break;

		case STM_BUILD_ACROSS:
			topSpeed = m_lateralSpeed;
			start = &m_riseToPos;
			end = &m_buildPos;
			break;

		case STM_TEAR_DOWN_ACROSS:
			topSpeed = m_lateralSpeed;
			start = &m_buildPos;
			end = &m_riseToPos;
			break;

		default:
			DEBUG_CRASH(("Unhandled case in BridgeScaffoldBehavior::update()"));
			return UPDATE_SLEEP_NONE;

	}

	// adjust speed so it's slower at the end of motion
	Coord3D speedVector;
	speedVector.x = end->x - start->x;
	speedVector.y = end->y - start->y;
	speedVector.z = end->z - start->z;
	Real totalDistance = speedVector.length() * 0.25f;
	speedVector.x = end->x - ourPos->x;
	speedVector.y = end->y - ourPos->y;
	speedVector.z = end->z - ourPos->z;
	Real ourDistance = speedVector.length();
	Real speed = (ourDistance / totalDistance) * topSpeed;
	Real minSpeed = topSpeed * 0.08f;
	if( speed < minSpeed )
		speed = minSpeed;
	if( speed > topSpeed )
		speed = topSpeed;

	//
	// make sure that speed can't get so incredibly small that we never finish our
	// movement no matter what the speed and distance are
	//
	if( speed < 0.001f )
		speed = 0.001f;

	// compute the new position given the speed
	Coord3D newPos;
	newPos.x = v.x * speed + ourPos->x;
	newPos.y = v.y * speed + ourPos->y;
	newPos.z = v.z * speed + ourPos->z;

	//
	// will this new position push us beyond our target destination, we will take the vector
	// from the new position to the destination and the vector from our current present position
	// tot he destination and dot them togehter ... if the result is < 0 then we have will
	// overshoot the distance if we use the new position
	//
	Coord3D tooFarVector;
	tooFarVector.x = m_targetPos.x - newPos.x;
	tooFarVector.y = m_targetPos.y - newPos.y;
	tooFarVector.z = m_targetPos.z - newPos.z;
	if( tooFarVector.x * dirV.x + tooFarVector.y * dirV.y + tooFarVector.z * dirV.z <= 0.0f )
	{

		// use the destination position
		newPos = m_targetPos;

		//
		// we have reached our target position, switch motion to the next position in
		// the chain (which may be stay still and don't move anymore)
		//
		switch( m_targetMotion )
		{

			case STM_RISE: setMotion( STM_BUILD_ACROSS ); break;
			case STM_BUILD_ACROSS: setMotion( STM_STILL ); break;
			case STM_TEAR_DOWN_ACROSS: setMotion( STM_SINK ); break;

			case STM_SINK:
			{

				// we are done with a sinking motion, destroy the scaffold object as our job is done
				TheGameLogic->destroyObject( us );
				break;

			}

		}

	}

	// set the new position
	us->setPosition( &newPos );

	// do not sleep
	return UPDATE_SLEEP_NONE;

}

// ------------------------------------------------------------------------------------------------
/** STATIC MEMBER:
	* Helper function to retrieve a bridge scaffold interface from an object if one is present */
// ------------------------------------------------------------------------------------------------
BridgeScaffoldBehaviorInterface *BridgeScaffoldBehavior::getBridgeScaffoldBehaviorInterfaceFromObject( Object *obj )
{

	// sanity
	if( obj == nullptr )
		return nullptr;

	// get the bridge tower behavior interface
	BridgeScaffoldBehaviorInterface *bridgeScaffoldInterface = nullptr;
	BehaviorModule **bmi;
	for( bmi = obj->getBehaviorModules(); *bmi; ++bmi )
	{

		bridgeScaffoldInterface = (*bmi)->getBridgeScaffoldBehaviorInterface();
		if( bridgeScaffoldInterface )
			return bridgeScaffoldInterface;

	}

	// interface not found
	return nullptr;

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void BridgeScaffoldBehavior::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void BridgeScaffoldBehavior::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// target motion
	xfer->xferUser( &m_targetMotion, sizeof( ScaffoldTargetMotion ) );

	// create pos
	xfer->xferCoord3D( &m_createPos );

	// rise to pos
	xfer->xferCoord3D( &m_riseToPos );

	// build pos
	xfer->xferCoord3D( &m_buildPos );

	// lateral speed
	xfer->xferReal( &m_lateralSpeed );

	// vertical speed
	xfer->xferReal( &m_verticalSpeed );

	// current target pos
	xfer->xferCoord3D( &m_targetPos );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void BridgeScaffoldBehavior::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}
