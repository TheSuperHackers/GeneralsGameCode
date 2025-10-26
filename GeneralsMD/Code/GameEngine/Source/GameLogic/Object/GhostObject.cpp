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

// FILE GhostObject.cpp ///////////////////////////////////////////////////////////////////////////
// Simple base object
// Author: Michael S. Booth, October 2000
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Xfer.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/GhostObject.h"
#include "GameLogic/Object.h"

GhostObjectManager *TheGhostObjectManager = NULL;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
GhostObject::GhostObject(void):
//Added By Sadullah Nader
//Initializations missing and needed
m_parentAngle(0.0f),
// TheSuperHackers @bugfix tomsons26 26/04/2025 Change initialization of m_parentGeometryIsSmall from 0.0f.
// Assigning a float to a bool results in the compiler using a random 1 byte value to assign to the bool.
// @todo Change initialization to 'false' when applicable.
m_parentGeometryIsSmall(true),
m_parentGeometryMajorRadius(0.0f),
m_parentGeometryminorRadius(0.0f),
m_parentObject(NULL),
m_partitionData(NULL)
{
	m_parentPosition.zero();
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
GhostObject::~GhostObject()
{
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void GhostObject::crc( Xfer *xfer )
{
}

// ------------------------------------------------------------------------------------------------
/** Xfer Method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void GhostObject::xfer( Xfer *xfer )
{
	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// parent object
	ObjectID parentObjectID = INVALID_ID;
	if( m_parentObject )
		parentObjectID = m_parentObject->getID();
	xfer->xferObjectID( &parentObjectID );
	if( xfer->getXferMode() == XFER_LOAD )
	{
		// tie up parent object pointer
		m_parentObject = TheGameLogic->findObjectByID( parentObjectID );

		// sanity
		if( parentObjectID != INVALID_ID && m_parentObject == NULL )
		{
			DEBUG_CRASH(( "GhostObject::xfer - Unable to connect m_parentObject" ));
			throw INI_INVALID_DATA;
		}
	}

	// parent geometry type
	xfer->xferUser( &m_parentGeometryType, sizeof( GeometryType ) );

	// parent geometry is small
	xfer->xferBool( &m_parentGeometryIsSmall );

	// parent geometry major radius
	xfer->xferReal( &m_parentGeometryMajorRadius );

	// parent geometry minor radius
	xfer->xferReal( &m_parentGeometryminorRadius );

	// parent angle
	xfer->xferReal( &m_parentAngle );

	// parent position
	xfer->xferCoord3D( &m_parentPosition );

	// partition data
	///@todo write me ---> !!!!!
	// PartitionData	*m_partitionData;	///< our PartitionData
}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void GhostObject::loadPostProcess( void )
{
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
GhostObjectManager::GhostObjectManager(void)
{
	m_lockGhostObjects = FALSE;
	m_saveLockGhostObjects = FALSE;
	m_trackAllPlayers = TheGlobalData->m_enablePlayerObserver;
	m_localPlayer = 0;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
GhostObjectManager::~GhostObjectManager()
{
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void GhostObjectManager::reset(void)
{
	m_trackAllPlayers = TheGlobalData->m_enablePlayerObserver;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
GhostObject *GhostObjectManager::addGhostObject(Object *object, PartitionData *pd)
{
	return 0;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void GhostObjectManager::removeGhostObject(GhostObject *mod)
{
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void GhostObjectManager::updateOrphanedObjects(int *playerIndexList, int playerIndexCount)
{
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void GhostObjectManager::releasePartitionData(void)
{
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void GhostObjectManager::restorePartitionData(void)
{
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void GhostObjectManager::crc( Xfer *xfer )
{
}

// ------------------------------------------------------------------------------------------------
/** Xfer Method:
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void GhostObjectManager::xfer( Xfer *xfer )
{
	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// local player
	xfer->xferInt( &m_localPlayer );
}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void GhostObjectManager::loadPostProcess( void )
{
}
