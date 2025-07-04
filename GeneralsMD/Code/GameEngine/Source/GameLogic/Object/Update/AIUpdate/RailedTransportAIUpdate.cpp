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

// FILE: RailedTransportAIUpdate.cpp //////////////////////////////////////////////////////////////
// Author: Colin Day, August 2002
// Desc: Railed Transport AI
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/ThingTemplate.h"
#include "Common/GameState.h"
#include "GameLogic/Locomotor.h"
#include "GameLogic/Module/RailedTransportAIUpdate.h"
#include "GameLogic/Module/RailedTransportDockUpdate.h"
#include "GameLogic/Object.h"

// TYPES //////////////////////////////////////////////////////////////////////////////////////////
static const Int INVALID_PATH = -1;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
RailedTransportAIUpdateModuleData::RailedTransportAIUpdateModuleData( void )
{

}  // end RailedTransportAIUpdateModuleData

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void RailedTransportAIUpdateModuleData::buildFieldParse( MultiIniFieldParse &p )
{
  AIUpdateModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] = 
	{
		{ "PathPrefixName",		INI::parseAsciiString, NULL,	offsetof( RailedTransportAIUpdateModuleData, m_pathPrefixName ) },
		{ 0, 0, 0, 0 }
	};

  p.add( dataFieldParse );

}  // end buildFieldParse

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
RailedTransportAIUpdate::RailedTransportAIUpdate( Thing *thing, const ModuleData *moduleData )
											 : AIUpdateInterface( thing, moduleData )              
{

	m_inTransit = FALSE;
	for( Int i = 0; i < MAX_WAYPOINT_PATHS; ++i )
	{

		m_path[ i ].startWaypointID = 0;
		m_path[ i ].endWaypointID = 0;

	}  // end for i
	m_numPaths = 0;
	m_currentPath = INVALID_PATH;
	m_waypointDataLoaded = FALSE;

}  // end RailedTransportAIUpdate

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
RailedTransportAIUpdate::~RailedTransportAIUpdate( void )
{

}  // end ~RailedTransportAIUpdate

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void RailedTransportAIUpdate::loadWaypointData( void )
{
	const RailedTransportAIUpdateModuleData *modData = getRailedTransportAIUpdateModuleData();

	// find all the possible waypoint paths we can use
	Waypoint *start, *end;
	AsciiString name;
	for( Int i = 0; i < MAX_WAYPOINT_PATHS; ++i )
	{

		// find start waypoint
		name.format( "%sStart%02d", modData->m_pathPrefixName.str(), i + 1 );
		start = TheTerrainLogic->getWaypointByName( name );

		// find end waypoint
		name.format( "%sEnd%02d", modData->m_pathPrefixName.str(), i + 1 );
		end = TheTerrainLogic->getWaypointByName( name );

		// if we have a start and an end, we have a valid path
		if( start && end )
		{

			m_path[ i ].startWaypointID = start->getID();
			m_path[ i ].endWaypointID = end->getID();
			m_numPaths++;

		}  // end if

	}  // end for i

	// waypoint data is loaded
	m_waypointDataLoaded = TRUE;

}  // end loadWaypointData

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void RailedTransportAIUpdate::pickAndMoveToInitialLocation( void )
{
	Object *us = getObject();
	const Coord3D *ourPos = us->getPosition();

	// select the path with the closest ending waypoint to our location
	Waypoint *waypoint, *closestEndWaypoint = NULL;
	Int closestPath = INVALID_PATH;
	Real closestDist = 99999999.9f;
	for( Int i = 0; i < m_numPaths; ++i )
	{

		// get this waypoint
		waypoint = TheTerrainLogic->getWaypointByID( m_path[ i ].endWaypointID );
		if( waypoint )
		{
			Coord3D v;

			// vector from us to waypoint
			v.x = waypoint->getLocation()->x - ourPos->x;
			v.y = waypoint->getLocation()->y - ourPos->y;
			v.z = waypoint->getLocation()->z - ourPos->z;

			// what is the distance
			Real dist = v.length();

			// if this distance is smaller, use this one
			if( dist < closestDist )
			{

				closestDist = dist;
				closestPath = i;
				closestEndWaypoint = waypoint;

			}  // end if

		}  // end if

	}  // end for i

	// a path must have been found
	DEBUG_ASSERTCRASH( closestPath != INVALID_PATH,
										 ("No suitable starting waypoint path could be found for '%s'\n",
										 us->getTemplate()->getName().str()) );

	// follow the waypoint path to its destination end point
	aiFollowWaypointPath( closestEndWaypoint, CMD_FROM_AI );

	// this is now our current path
	m_currentPath = closestPath;

	// we are now "in transit"
	setInTransit( TRUE );

}  // end pickAndMoveToInitialLocation

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UpdateSleepTime RailedTransportAIUpdate::update( void )
{
	Object *us = getObject();

	// load the waypoint data if not loaded
	if( m_waypointDataLoaded == FALSE )
		loadWaypointData();

	// extend base class
	UpdateSleepTime result;
	result = AIUpdateInterface::update();

	// railed transports move ultra accurate like
	Locomotor *currentLocomotor = getCurLocomotor();
	if( currentLocomotor )
		currentLocomotor->setUltraAccurate( TRUE );
	
	//
	// if we have no current path selected pick one and move to the end waypoint of that
	// path.  this will set us up in an initial position at the end of the closest path
	// so that stuff can be loaded into us
	//
	if( m_currentPath == INVALID_PATH && m_numPaths > 0 )
		pickAndMoveToInitialLocation();

	//
	// if we're in transit, see if we're close enough to the destination waypoint to be
	// considered as "there" and open up the dock
	//
	if( m_inTransit )
	{
	
		// sanity
		DEBUG_ASSERTCRASH( m_currentPath != INVALID_PATH,
											 ("RailedTransportAIUpdate: Invalid current path '%s'\n", m_currentPath) );

		// get our target waypoint
		Waypoint *waypoint = TheTerrainLogic->getWaypointByID( m_path[ m_currentPath ].endWaypointID );
		
		// sanity
		DEBUG_ASSERTCRASH( waypoint, ("RailedTransportAIUpdate: Invalid target waypoint") );

		if (waypoint)
		{
			// how far away are we from the target waypoint
			const Coord3D *start = us->getPosition();
			const Coord3D *end = waypoint->getLocation();
			Coord3D v;
			v.x = end->x - start->x;
			v.y = end->y - start->y;
			v.z = end->z - start->z;
			Real dist = v.length();
			if( dist <= 5.0f || isIdle() )
			{

				// we are no longer in transit
				setInTransit( FALSE );

			}  // end if
		}
		else
		{
			setInTransit( FALSE );
		}

	}  // end if

	return UPDATE_SLEEP_NONE;

}  // end update

// ------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void RailedTransportAIUpdate::aiDoCommand( const AICommandParms *parms )
{

	// if not allowed to respond to any command get out of here
	if( isAllowedToRespondToAiCommands(parms) == FALSE )
		return;

	// we ignore all commands from the player except the one to start a transit and to unload
	if( parms->m_cmdSource == CMD_FROM_PLAYER && 
			parms->m_cmd != AICMD_EXECUTE_RAILED_TRANSPORT &&
			parms->m_cmd != AICMD_EVACUATE )
		return;

	// call the default do command
	AIUpdateInterface::aiDoCommand( parms );
	
}  // end aiDoCommand

///////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE ////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void RailedTransportAIUpdate::setInTransit( Bool inTransit )
{
	Object *us = getObject();
	DockUpdateInterface *dui = us->getDockUpdateInterface();

	// open up the dock
	if( dui )
		dui->setDockOpen( !inTransit );

	// no longer in transit
	m_inTransit = inTransit;

}  // end setInTransit

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void RailedTransportAIUpdate::privateExecuteRailedTransport( CommandSourceType cmdSource )
{
	Object *us = getObject();

	//
	// find us our railed dock interface, note that we MUST go through the modules here because
	// if we just call the method getReailedTransportDockUpdateInterface, it will execute
	// the method for *THIS AI UPDATE MODULE* which of course is not our dock update
	//
	RailedTransportDockUpdateInterface *rtdui = NULL;
	for( BehaviorModule **u = us->getBehaviorModules(); *u; ++u )
		if( (rtdui = (*u)->getRailedTransportDockUpdateInterface()) != NULL )
			break;

	// if we've in the process of loading or unloading anything we can't do a transport sequence
	if( rtdui == NULL || rtdui->isLoadingOrUnloading() )
		return;

	// pick the next path
	if( ++m_currentPath >= m_numPaths )
		m_currentPath = 0;

	// find the start waypoint for our current path
	Waypoint *startWaypoint = TheTerrainLogic->getWaypointByID( m_path[ m_currentPath ].startWaypointID );
	DEBUG_ASSERTCRASH( startWaypoint, ("RailedTransportAIUpdate: Start waypoint not found") );

	// follow this waypoint path
	aiFollowWaypointPath( startWaypoint, CMD_FROM_AI );

	// we are now in transit
	setInTransit( TRUE );

}  // end privateExecuteRailedTransport

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void RailedTransportAIUpdate::privateEvacuate( Int exposeStealthUnits, CommandSourceType cmdSource )
{
	Object *us = getObject();

	//
	// find us our railed dock interface, note that we MUST go through the modules here because
	// if we just call the method getReailedTransportDockUpdateInterface, it will execute
	// the method for *THIS AI UPDATE MODULE* which of course is not our dock update
	//
	RailedTransportDockUpdateInterface *rtdui = NULL;
	for( BehaviorModule **u = us->getBehaviorModules(); *u; ++u )
		if( (rtdui = (*u)->getRailedTransportDockUpdateInterface()) != NULL )
			break;

	// sanity
	if( rtdui == NULL )
		return;

	// can't unload when in transit
	if( m_inTransit == TRUE )
		return;

	// cannot evacuate when we're loading or unloading anything
	if( rtdui->isLoadingOrUnloading() )
		return;

	// start the manual undocking process
	rtdui->unloadAll();

}  // end privateEvacuate

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void RailedTransportAIUpdate::crc( Xfer *xfer )
{
	// extend base class
	AIUpdateInterface::crc(xfer);
}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void RailedTransportAIUpdate::xfer( Xfer *xfer )
{
  XferVersion currentVersion = 1;
  XferVersion version = currentVersion;
  xfer->xferVersion( &version, currentVersion );
 
 // extend base class
	AIUpdateInterface::xfer(xfer);

	xfer->xferBool(&m_inTransit);
	xfer->xferInt(&m_numPaths);
	if (m_numPaths > MAX_WAYPOINT_PATHS) {
		DEBUG_CRASH(("m_numPaths %d exceeds limit %d.", m_numPaths, MAX_WAYPOINT_PATHS));
		throw SC_INVALID_DATA;
	}
	Int i;
	for (i=0; i<m_numPaths; i++) {
		xfer->xferUnsignedInt(&m_path[i].startWaypointID);
		xfer->xferUnsignedInt(&m_path[i].endWaypointID);
	}
	xfer->xferInt(&m_currentPath);
	xfer->xferBool(&m_waypointDataLoaded);

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void RailedTransportAIUpdate::loadPostProcess( void )
{
 // extend base class
	AIUpdateInterface::loadPostProcess();
}  // end loadPostProcess
