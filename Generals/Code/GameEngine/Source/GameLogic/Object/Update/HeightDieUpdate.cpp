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

// FILE: HeightDieUpdate.cpp //////////////////////////////////////////////////////////////////////
// Author: Objects that will die when the are a certain height above the terrain or objects
// Desc:   Colin Day, April 2002
///////////////////////////////////////////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/ThingTemplate.h"
#include "Common/Xfer.h"
#include "GameClient/InGameUI.h"
#include "GameClient/ParticleSys.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/TerrainLogic.h"
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/Module/HeightDieUpdate.h"
#include "GameLogic/Module/PhysicsUpdate.h"


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
HeightDieUpdateModuleData::HeightDieUpdateModuleData( void )
{

	m_targetHeightAboveTerrain = 0.0f;
	m_targetHeightIncludesStructures = FALSE;
	m_onlyWhenMovingDown = FALSE;
	m_destroyAttachedParticlesAtHeight = -1.0f;
	m_snapToGroundOnDeath = FALSE;
	m_initialDelay = 0;

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void HeightDieUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{

  UpdateModuleData::buildFieldParse( p );

	static const FieldParse dataFieldParse[] =
	{
		{ "TargetHeight", INI::parseReal, NULL, offsetof( HeightDieUpdateModuleData, m_targetHeightAboveTerrain ) },
		{ "TargetHeightIncludesStructures", INI::parseBool, NULL, offsetof( HeightDieUpdateModuleData, m_targetHeightIncludesStructures ) },
		{ "OnlyWhenMovingDown", INI::parseBool, NULL, offsetof( HeightDieUpdateModuleData, m_onlyWhenMovingDown ) },
		{ "DestroyAttachedParticlesAtHeight", INI::parseReal, NULL, offsetof( HeightDieUpdateModuleData, m_destroyAttachedParticlesAtHeight ) },
		{ "SnapToGroundOnDeath", INI::parseBool, NULL, offsetof( HeightDieUpdateModuleData, m_snapToGroundOnDeath ) },
		{ "InitialDelay", INI::parseDurationUnsignedInt, NULL, offsetof( HeightDieUpdateModuleData, m_initialDelay ) },
		{ 0, 0, 0, 0 }

	};

  p.add(dataFieldParse);

}

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
HeightDieUpdate::HeightDieUpdate( Thing *thing, const ModuleData* moduleData )
																: UpdateModule( thing, moduleData )
{
	m_hasDied = FALSE;
	m_particlesDestroyed = FALSE;
	m_lastPosition.x = -1.0f;
	m_lastPosition.y = -1.0f;
	m_lastPosition.z = -1.0f;
	m_earliestDeathFrame = UINT_MAX;
	// m_lastPosition = *thing->getPosition();

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
HeightDieUpdate::~HeightDieUpdate( void )
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime HeightDieUpdate::update( void )
{
	UnsignedInt now = TheGameLogic->getFrame();
	if( m_earliestDeathFrame == UINT_MAX )
		m_earliestDeathFrame = now + getHeightDieUpdateModuleData()->m_initialDelay;

	// If at least a one frame delay has been set, then stop for a while
	if( m_earliestDeathFrame > now )
		return UPDATE_SLEEP_NONE;

	// do nothing if we're contained within other objects ... like a transport
	if( getObject()->getContainedBy() != NULL )
	{

		// keep track of our last position even though we're not doing anything yet
		m_lastPosition = *getObject()->getPosition();

		// get outta here
		return UPDATE_SLEEP_NONE;

	}

	// get the module data
	const HeightDieUpdateModuleData *modData = getHeightDieUpdateModuleData();

	// get our current position
	const Coord3D *pos = getObject()->getPosition();

	Bool directionOK = TRUE;
	if( m_hasDied == FALSE )
	{

		if( modData->m_onlyWhenMovingDown )
		{

			if( pos->z >= m_lastPosition.z )
				directionOK = FALSE;

		}

		// get the terrain height
		Real terrainHeightAtPos = TheTerrainLogic->getGroundHeight( pos->x, pos->y );

		// if including structures, check for bridges
		if (modData->m_targetHeightIncludesStructures)
		{
			PathfindLayerEnum layer = TheTerrainLogic->getHighestLayerForDestination(pos);
			if (layer != LAYER_GROUND)
			{
				Real layerHeight = TheTerrainLogic->getLayerHeight(pos->x, pos->y, layer);
				if (layerHeight > terrainHeightAtPos)
					terrainHeightAtPos = layerHeight;
			}
		}

		//
		// our target height to die at is by default the height specified in the INI entry above
		// the terrain ... we may change our target height if we care about dying above
		// objects under us (see below)
		//
		Real targetHeight = terrainHeightAtPos + modData->m_targetHeightAboveTerrain;

		//
		// if we consider objects under us ... we will die when we are the specified distance above
		// those objects
		//
		if( modData->m_targetHeightIncludesStructures == TRUE )
		{

			// scan all objects in the radius of our extent and find the tallest height among them
			PartitionFilterAcceptByKindOf filter1( MAKE_KINDOF_MASK( KINDOF_STRUCTURE ),KINDOFMASK_NONE );
			PartitionFilter *filters[] = { &filter1, NULL };
			Real range = getObject()->getGeometryInfo().getBoundingCircleRadius();
			ObjectIterator *iter = ThePartitionManager->iterateObjectsInRange( getObject(),
																																				 range,
																																				 FROM_BOUNDINGSPHERE_3D,
																																				 filters );
			MemoryPoolObjectHolder hold( iter );
			Object *obj;

			Real tallestHeight = 0.0f;
			Real thisHeight;
			for( obj = iter->first(); obj; obj = iter->next() )
			{

				// ignore ourselves
				if( obj == getObject() )
					continue;

				// store the height of the tallest object under us
				thisHeight = obj->getGeometryInfo().getMaxHeightAbovePosition();

				if( thisHeight > tallestHeight )
					tallestHeight = thisHeight;

			}

			//
			// our target height is either the height above the terrain as specified by the INI
			// entry for the object that has this update ... or it is the building height of the
			// tallest thing under us
			//
			if( tallestHeight > modData->m_targetHeightAboveTerrain )
				targetHeight = tallestHeight + terrainHeightAtPos;

		}

		// if we are below the target height ... DIE!
		if( pos->z < targetHeight && directionOK )
		{

			// if we're supposed to snap us to the ground on death do so
			// AND: even if we're not snapping to ground, be sure we don't go BELOW ground
			if( modData->m_snapToGroundOnDeath || pos->z < terrainHeightAtPos )
			{
				Coord3D ground;

				ground.x = pos->x;
				ground.y = pos->y;
				ground.z = terrainHeightAtPos;
				getObject()->setPosition( &ground );

			}

			// kill the object
			getObject()->kill();

			// we have died ... don't do this again
			m_hasDied = TRUE;

		}

	}

	//
	// if our height is below the destroy attached particles height above the terrain, clean
	// them up from the particle system
	//
	if( m_particlesDestroyed == FALSE && pos->z < modData->m_destroyAttachedParticlesAtHeight && (m_hasDied || directionOK) )
	{

		// destroy them
		TheParticleSystemManager->destroyAttachedSystems( getObject() );

		// don't do this again
		m_particlesDestroyed = TRUE;

	}

	// save our current position as the last position we monitored
	m_lastPosition = *pos;

	return UPDATE_SLEEP_NONE;

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void HeightDieUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version
	* 2: m_earliestDeathFrame
*/
// ------------------------------------------------------------------------------------------------
void HeightDieUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 2;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// has died
	xfer->xferBool( &m_hasDied );

	// particles destroyed
	xfer->xferBool( &m_particlesDestroyed );

	// last position
	xfer->xferCoord3D( &m_lastPosition );

	if( version >= 2 )
		xfer->xferUnsignedInt( &m_earliestDeathFrame );
	else
		m_earliestDeathFrame = 0;

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void HeightDieUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}
