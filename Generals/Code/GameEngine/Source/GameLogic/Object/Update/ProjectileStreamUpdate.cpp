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

// FILE: ProjectileStreamUpdate.cpp //////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, May 2002
// Desc:   Tracks all projectiles fired so they can be drawn as a stream
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Xfer.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/ProjectileStreamUpdate.h"
#include "WWMath/vector3.h"

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ProjectileStreamUpdate::ProjectileStreamUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	ObjectID m_projectileIDs[MAX_PROJECTILE_STREAM];
	for( Int index = 0; index < MAX_PROJECTILE_STREAM; index++ )
	{
		m_projectileIDs[index] = INVALID_ID;
	}

	m_owningObject = INVALID_ID;
	m_nextFreeIndex = 0;
	m_firstValidIndex = 0;
} 

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ProjectileStreamUpdate::~ProjectileStreamUpdate( void )
{

}

//-------------------------------------------------------------------------------------------------
/** The update callback. */
//-------------------------------------------------------------------------------------------------
UpdateSleepTime ProjectileStreamUpdate::update( void )
{
	cullFrontOfList();

	// Update the draw module about our points (arrange array so they can read?)

	if( considerDying() )
		TheGameLogic->destroyObject( getObject() );

	return UPDATE_SLEEP_NONE;
}

void ProjectileStreamUpdate::addProjectile( ObjectID sourceID, ObjectID newID )
{
	DEBUG_ASSERTCRASH( m_owningObject == INVALID_ID  ||  m_owningObject == sourceID, ("Two objects are trying to use the same Projectile stream.") );//Don't cross the streams!
	if( m_owningObject == INVALID_ID )
		m_owningObject = sourceID;

	// Keep track of the id in a circular array
	m_projectileIDs[ m_nextFreeIndex ] = newID;
	m_nextFreeIndex = (m_nextFreeIndex + 1) % MAX_PROJECTILE_STREAM;
	DEBUG_ASSERTCRASH( m_nextFreeIndex != m_firstValidIndex, ("Need to increase the allowed number of simultaneous particles in ProjectileStreamUpdate.") );
}

void ProjectileStreamUpdate::cullFrontOfList()
{
	while( (m_firstValidIndex != m_nextFreeIndex)  &&  (TheGameLogic->findObjectByID( m_projectileIDs[m_firstValidIndex] ) == NULL) )
	{
		// Chew off the front if they are gone.  Don't chew on the middle, as bad ones there are just a break in the chain
		m_firstValidIndex = (m_firstValidIndex + 1) % MAX_PROJECTILE_STREAM;
	}
}

Bool ProjectileStreamUpdate::considerDying()
{
	if( m_firstValidIndex == m_nextFreeIndex  &&  m_owningObject != INVALID_ID )
	{
		//If I have no projectiles to watch, and my master is dead, then yes, I want to die
		if( TheGameLogic->findObjectByID(m_owningObject) == NULL )
			return TRUE;
	}

	return FALSE;
}

void ProjectileStreamUpdate::getAllPoints( Vector3 *points, Int *count )
{
	Int pointCount = 0;
	Int pointIndex = m_firstValidIndex;


	Object *obj = TheGameLogic->findObjectByID(m_owningObject);


	while( pointIndex != m_nextFreeIndex )
	{
		// Go through the array I think of as good.  Holes in the middle get 0,0,0.  I write
		// to pointCount because I am unrolling a circular array into the same sized flat one
		// since I am writing anyway.
		Object *projectile = TheGameLogic->findObjectByID( m_projectileIDs[pointIndex] );

		if( projectile )
		{
			Coord3D thisPoint = *projectile->getPosition();
			points[pointCount].X = thisPoint.x;
			points[pointCount].Y = thisPoint.y;
			points[pointCount].Z = thisPoint.z;
			

			if ( obj && obj->isKindOf( KINDOF_VEHICLE ) )				// this makes the stream skim along my roof, if I have a roof
			{
				const Coord3D *pos = obj->getPosition();
				Real myTop = obj->getGeometryInfo().getMaxHeightAbovePosition() + pos->z + 0.5f;
				Coord3D delta;
				delta.x = pos->x - points[pointCount].X;
				delta.y = pos->y - points[pointCount].Y;
				delta.z = 0.0f;
				if( delta.length() <= obj->getGeometryInfo().getMajorRadius() * 1.5f )
					points[pointCount].Z = MAX( points[pointCount].Z, myTop );
			}




		}
		else
		{
			points[pointCount].X = 0;
			points[pointCount].Y = 0;
			points[pointCount].Z = 0;
		}

		pointIndex = (pointIndex + 1) % MAX_PROJECTILE_STREAM;
		pointCount++;
	}

	*count = pointCount;
}

void ProjectileStreamUpdate::setPosition( const Coord3D *newPosition )
{
	Object *me = getObject();
	me->setPosition( newPosition );
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void ProjectileStreamUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void ProjectileStreamUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// projectile ids
	xfer->xferUser( m_projectileIDs, sizeof( ObjectID ) * MAX_PROJECTILE_STREAM );

	// next free index
	xfer->xferInt( &m_nextFreeIndex );

	// first valid index
	xfer->xferInt( &m_firstValidIndex );

	// owning object
	xfer->xferObjectID( &m_owningObject );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void ProjectileStreamUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
