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

// FILE: RailedTransportDockUpdate.cpp ////////////////////////////////////////////////////////////
// Author: Colin Day, August 2002
// Desc:   Railed Transport Dock Update
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameClient/InGameUI.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/Module/OpenContain.h"
#include "GameLogic/Module/RailedTransportDockUpdate.h"

// TYPES //////////////////////////////////////////////////////////////////////////////////////////
enum { UNLOAD_ALL = -1 };

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
RailedTransportDockUpdateModuleData::RailedTransportDockUpdateModuleData( void )
{

	m_pullInsideDurationInFrames = 0;
	m_pushOutsideDurationInFrames = 0;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
/*static*/ void RailedTransportDockUpdateModuleData::buildFieldParse( MultiIniFieldParse &p )
{
  DockUpdateModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] =
	{

		{ "PullInsideDuration", INI::parseDurationUnsignedInt, NULL, offsetof( RailedTransportDockUpdateModuleData, m_pullInsideDurationInFrames ) },
		{ "PushOutsideDuration",INI::parseDurationUnsignedInt, NULL, offsetof( RailedTransportDockUpdateModuleData, m_pushOutsideDurationInFrames ) },
		{ 0, 0, 0, 0 }

	};

  p.add( dataFieldParse );

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
RailedTransportDockUpdate::RailedTransportDockUpdate( Thing *thing, const ModuleData *moduleData )
												 : DockUpdate( thing, moduleData )
{

	m_dockingObjectID = INVALID_ID;
	m_pullInsideDistancePerFrame = 0.0f;
	m_unloadingObjectID = INVALID_ID;
	m_pushOutsideDistancePerFrame = 0.0f;
	m_unloadCount = UNLOAD_ALL;

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
RailedTransportDockUpdate::~RailedTransportDockUpdate( void )
{

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UpdateSleepTime RailedTransportDockUpdate::update( void )
{

	// extend functionality
	UpdateSleepTime result;

	result = DockUpdate::update();

	// do any pull in docking we need to
	doPullInDocking();

	// do any push out docking
	doPushOutDocking();

	return UPDATE_SLEEP_NONE;
}

// ------------------------------------------------------------------------------------------------
/** The dock action callback, return FALSE when done docking */
// ------------------------------------------------------------------------------------------------
Bool RailedTransportDockUpdate::action( Object *docker, Object *drone )
{
	Object *us = getObject();

	// sanity
	if( docker == NULL )
		return FALSE;

	// set this object as docking with us if not already done so
	if( m_dockingObjectID != docker->getID() )
	{

		m_dockingObjectID = docker->getID();

		// don't let the user interact with this object anymore
		TheGameLogic->deselectObject(docker, PLAYERMASK_ALL, TRUE);
		docker->setStatus( MAKE_OBJECT_STATUS_MASK( OBJECT_STATUS_UNSELECTABLE ) );

		// hold the object so physics doesn't mess with it anymore
		docker->setDisabled( DISABLED_HELD );

		//
		// given the amount of time we want it to take to "pull" the object inside, figure out
		// how much distance it should traverse towards our center every frame
		//
		const Coord3D *dockerPos = docker->getPosition();
		const Coord3D *dockPos = us->getPosition();
		Coord3D v;
		v.x = dockPos->x - dockerPos->x;
		v.y = dockPos->y - dockerPos->y;
		v.z = dockPos->z - dockerPos->z;

		// how far do we have to go
		Real mag = v.length();

		// now that we know how far we must go, now much distance should we travel every frame
		const RailedTransportDockUpdateModuleData *modData = getRailedTransportDockUpdateModuleData();
		m_pullInsideDistancePerFrame = mag / modData->m_pullInsideDurationInFrames;

		// orient docker so its facing toward the transport
		Coord2D angleVector;
		angleVector.x = dockPos->x - dockerPos->x;
		angleVector.y = dockPos->y - dockerPos->y;
		docker->setOrientation( angleVector.toAngle() );

	}

	return TRUE;

}

// ------------------------------------------------------------------------------------------------
/** Is clear to enter the railed transport */
// ------------------------------------------------------------------------------------------------
Bool RailedTransportDockUpdate::isClearToEnter( Object const *docker ) const
{
	const Object *us = getObject();

	// first do base class restrictions
	Bool clear = DockUpdate::isClearToEnter( docker );
	if( clear == FALSE )
		return FALSE;

	// we have additional requirements, we are a transporting dock so we can't be full
	ContainModuleInterface *contain = us->getContain();
	if( contain && contain->isValidContainerFor( docker, TRUE ) == FALSE )
		return FALSE;

	return TRUE;

}

// ------------------------------------------------------------------------------------------------
/** Is anything currently loading or unloading */
// ------------------------------------------------------------------------------------------------
Bool RailedTransportDockUpdate::isLoadingOrUnloading( void )
{

	if( m_unloadingObjectID != INVALID_ID || m_dockingObjectID != INVALID_ID )
		return TRUE;

	return FALSE;

}

// ------------------------------------------------------------------------------------------------
/** Start the unload process */
// ------------------------------------------------------------------------------------------------
void RailedTransportDockUpdate::unloadAll( void )
{

	// sanity, if we're already unloading, ignore this command and just allow us to finish
	if( m_unloadingObjectID != INVALID_ID )
		return;

	// start our first object unloading and continue through them all
	m_unloadCount = UNLOAD_ALL;
	unloadNext();

}

// ------------------------------------------------------------------------------------------------
/** Unload a single individual only */
// ------------------------------------------------------------------------------------------------
void RailedTransportDockUpdate::unloadSingleObject( Object *obj )
{

	// start the unload process of a single object
	m_unloadCount = 1;
	unloadNext();

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
/** If we have an object recorded as currently docking with us, pull that object inside
	* and when it is inside, contain it */
// ------------------------------------------------------------------------------------------------
void RailedTransportDockUpdate::doPullInDocking( void )
{

	//
	// if we're pulling an object inside of us, do that pull now.  we need this so that the
	// railed transport can "pull" objects inside it because typically those objects can only drive
	// on land and have a hard time driving "inside" the railed transport ... so we fake it!
	//
	if( m_dockingObjectID != INVALID_ID )
	{
		Object *us = getObject();
		Object *docker = TheGameLogic->findObjectByID( m_dockingObjectID );

		// check for docker gone
		if( docker == NULL )
			m_dockingObjectID = INVALID_ID;

		// pull it
		if( docker )
		{
			const Coord3D *dockerPos = docker->getPosition();
			const Coord3D *dockPos = us->getPosition();

			// get the vector from the docker to the dock pos
			Coord3D v;
			v.x = dockPos->x - dockerPos->x;
			v.y = dockPos->y - dockerPos->y;
			v.z = dockPos->z - dockerPos->z;
			v.normalize();

			// apply "movement" to the vector
			v.x *= m_pullInsideDistancePerFrame;
			v.y *= m_pullInsideDistancePerFrame;

			// apply current position of the docker to the vector
			v.x += dockerPos->x;
			v.y += dockerPos->y;
			v.z  = dockerPos->z;  // keep Z height the same and just scoot along the ground

			// set the new position
			docker->setPosition( &v );

			//
			// set the model condition for the object as "moving" even though it really
			// isn't in the traditional sense, but we don't want them to scoot slide into
			// the transport and look wierd
			//
			docker->setModelConditionState( MODELCONDITION_MOVING );

			// if we're at the destination then stop and put is inside the dock object
			Real distSq = ThePartitionManager->getDistanceSquared( docker, us, FROM_CENTER_2D );
			Real closeEnoughDistance = 6.0f;
			if( distSq <= (closeEnoughDistance * closeEnoughDistance) )
			{

				// the object is now no longer "moving"
				docker->clearModelConditionState( MODELCONDITION_MOVING );

				// stop the dock action
				cancelDock( docker );

				// stop the docker from doing anything by going idle
				AIUpdateInterface *dockerAI = docker->getAIUpdateInterface();
				if( dockerAI )
					dockerAI->aiIdle( CMD_FROM_AI );

				// put object inside us
				ContainModuleInterface *contain = us->getContain();
				if( contain )
				{
					contain->addToContain( docker );
				}

				// no object is docking now
				m_dockingObjectID = INVALID_ID;

			}

		}

	}

}

// ------------------------------------------------------------------------------------------------
/** If we have an object recorded as being pushed out of us then do that here */
// ------------------------------------------------------------------------------------------------
void RailedTransportDockUpdate::doPushOutDocking( void )
{

	if( m_unloadingObjectID )
	{
		Object *unloader = TheGameLogic->findObjectByID( m_unloadingObjectID );

		// if unloader is not found (like they got destroyed) unload the next object inside
		if( unloader == NULL )
		{

			unloadNext();
			return;

		}

		// pull it
		if( unloader )
		{
			const Coord3D *unloaderPos = unloader->getPosition();

			// get the destination point as the DOCKEND
			Coord3D destPos;
			getExitPosition( unloader, &destPos );
			destPos.z = TheTerrainLogic->getGroundHeight( destPos.x, destPos.y );

			// get the vector from the unloader to the destination point
			Coord3D v;
			v.x = destPos.x - unloaderPos->x;
			v.y = destPos.y - unloaderPos->y;
			v.z = destPos.z - unloaderPos->z;
			v.normalize();

			// apply "movement" to that vector
			v.x *= m_pushOutsideDistancePerFrame;
			v.y *= m_pushOutsideDistancePerFrame;

			// apply current position of the unloader to the vector
			v.x += unloaderPos->x;
			v.y += unloaderPos->y;
			v.z  = destPos.z;  // keep Z height the same and just scoot along the ground

			// set the new position
			unloader->setPosition( &v );

			//
			// set the model condition for the object as "moving" even though it really
			// isn't in the traditional sense, but we don't want them to scoot slide into
			// the transport and look wierd
			//
			unloader->setModelConditionState( MODELCONDITION_MOVING );

			// if we're at the destination then stop and unload the next object if present
			Real distSq = sqr( destPos.x - v.x ) + sqr( destPos.y - v.y ) + sqr( destPos.z - v.z );
			Real closeEnoughDistance = 3.0f;
			if( distSq <= sqr( closeEnoughDistance ) )
			{
				Object *us = getObject();

				// the object is now no longer "moving"
				unloader->clearModelConditionState( MODELCONDITION_MOVING );

				// set the unloaded object as idle
				AIUpdateInterface *unloaderAI = unloader->getAIUpdateInterface();
				if( unloaderAI )
					unloaderAI->aiIdle( CMD_FROM_AI );

				// clear the held status from this unloading object
				unloader->clearDisabled( DISABLED_HELD );

				// we can now be selected by the player again
				unloader->clearStatus( MAKE_OBJECT_STATUS_MASK( OBJECT_STATUS_UNSELECTABLE ) );

				// tell the unloader to move to one of the dock positions and out of the way
				Drawable *draw = us->getDrawable();
				if( unloaderAI && draw )
				{
					Coord3D finalPos;

					draw->getPristineBonePositions( "DOCKWAITING07", 0, &finalPos, NULL, 1 );
					us->convertBonePosToWorldPos( &finalPos, NULL, &finalPos, NULL );
					unloaderAI->aiMoveToPosition( &finalPos, CMD_FROM_AI );

				}

				// unload the next object
				unloadNext();

			}

		}

	}

}

// ------------------------------------------------------------------------------------------------
/** Iterate callback for the finding the first contained object */
// ------------------------------------------------------------------------------------------------
static void getFirstContain( Object *obj, void *userData )
{
	Object **firstContain = (Object **)userData;

	// if object has been found get out of here
	if( *firstContain != NULL )
		return;

	// assign this as the first object found
	*firstContain = obj;

}

// ------------------------------------------------------------------------------------------------
/** Start the next object contained by us as "unloading and coming out" */
// ------------------------------------------------------------------------------------------------
void RailedTransportDockUpdate::unloadNext( void )
{
	Object *us = getObject();

	// by default, setup our unloading process to be done with no objects being considered
	m_unloadingObjectID = INVALID_ID;

	//
	// if our unload count is zero we can't unload any more until we receive a command to
	// unload another one or everything we've got
	//
	if( m_unloadCount == 0 )
		return;

	// better be an open container
	ContainModuleInterface *contain = us->getContain();
	OpenContain *openContain = contain ? contain->asOpenContain() : NULL;
	DEBUG_ASSERTCRASH( openContain, ("Unloading next from railed transport, but '%s' has no open container",
										 us->getTemplate()->getName().str()) );

	// get the first contained object
	Object *unloader = NULL;
	openContain->iterateContained( getFirstContain, &unloader, FALSE );
	if( unloader )
	{

		// remove us from the container
		openContain->removeFromContain( unloader );

		// set position of the loader to our position
		unloader->setPosition( us->getPosition() );

		// orient unloader to the same angle as us so we can drive out the front
		unloader->setOrientation( us->getOrientation() );

		// mark us as HELD so physics or anything else can't mess with our position
		unloader->setDisabled( DISABLED_HELD );

		//
		// get the dock point that we're going to go to ... that is where we came in
		// at the DOCKEND point
		//
		Coord3D dockPosition;
		getExitPosition( unloader, &dockPosition );

		// get unloader position
		const Coord3D *unloaderPos = unloader->getPosition();

		// how far is it from our current position to the dock position
		Coord3D v;
		v.x = dockPosition.x - unloaderPos->x;
		v.y = dockPosition.y - unloaderPos->y;
		v.z = dockPosition.z - unloaderPos->z;
		Real mag = v.length();

		// now that we know how far we must go, now much distance should we travel every frame
		const RailedTransportDockUpdateModuleData *modData = getRailedTransportDockUpdateModuleData();
		m_pushOutsideDistancePerFrame = mag / modData->m_pushOutsideDurationInFrames;

		// set this as our current unloader
		m_unloadingObjectID = unloader->getID();

		// we've now used an unload (if we're keeping count for single exits)
		if( m_unloadCount != UNLOAD_ALL )
			--m_unloadCount;

	}

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void RailedTransportDockUpdate::crc( Xfer *xfer )
{

	// extend base class
	DockUpdate::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void RailedTransportDockUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	DockUpdate::xfer( xfer );

	// docking object id
	xfer->xferObjectID( &m_dockingObjectID );

	// pull inside distance per frame
	xfer->xferReal( &m_pullInsideDistancePerFrame );

	// unloading object id
	xfer->xferObjectID( &m_unloadingObjectID );

	// push outside distance per frame
	xfer->xferReal( &m_pushOutsideDistancePerFrame );

	// unload count
	xfer->xferInt( &m_unloadCount );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void RailedTransportDockUpdate::loadPostProcess( void )
{

	// extend base class
	DockUpdate::loadPostProcess();

}
