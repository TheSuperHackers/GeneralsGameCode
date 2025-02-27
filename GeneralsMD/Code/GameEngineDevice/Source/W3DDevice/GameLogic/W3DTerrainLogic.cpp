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

// FILE: W3DTerrainLogic.cpp //////////////////////////////////////////////////////////////////////
// W3D implementation details for logical terrain
// Author: Colin Day, April 2001
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "Common/GameMemory.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameLogic/W3DTerrainLogic.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "Common/PerfTimer.h"
#include "Common/MapReaderWriterInfo.h"
#include "Common/GlobalData.h"
#include "Common/Xfer.h"
#include "GameClient/GameClient.h"

#include "GameClient/MapUtil.h"
#include "GameLogic/AI.h"
#include "GameLogic/AIPathfind.h"


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DTerrainLogic::W3DTerrainLogic():
m_mapMinZ(0),
m_mapMaxZ(1)
{
	m_mapData = NULL;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
W3DTerrainLogic::~W3DTerrainLogic()
{

	// free terrain data

}

//-------------------------------------------------------------------------------------------------
/** Device DEPENDENT implementation init details for logical terrain */
//-------------------------------------------------------------------------------------------------
void W3DTerrainLogic::init( void )
{

	// enhancing functionality
	TerrainLogic::init();
	m_mapDX = 0;
	m_mapDY = 0;
	m_mapMinZ = 0;
	m_mapMaxZ = 1;

}

//-------------------------------------------------------------------------------------------------
/** Reset */
//-------------------------------------------------------------------------------------------------
void W3DTerrainLogic::reset( void )
{
	TerrainLogic::reset();
	m_mapDX = 0;
	m_mapDY = 0;
	m_mapMinZ = 0;
	m_mapMaxZ = 1;
	WorldHeightMap::freeListOfMapObjects();
}

//-------------------------------------------------------------------------------------------------
/** newMap */
//-------------------------------------------------------------------------------------------------
void W3DTerrainLogic::newMap( Bool saveGame )
{

	TheTerrainRenderObject->loadRoadsAndBridges( this, saveGame );
	TerrainLogic::newMap( saveGame );
}

//-------------------------------------------------------------------------------------------------
/** Update */
//-------------------------------------------------------------------------------------------------
void W3DTerrainLogic::update( void )
{
	TerrainLogic::update();
}

//-------------------------------------------------------------------------------------------------
/** Device DEPENDENT implementation for load details of logical terrain.
Note - if query is true, we are  */
//-------------------------------------------------------------------------------------------------
Bool W3DTerrainLogic::loadMap( AsciiString filename , Bool query )
{
	if(!TheMapCache)
		return FALSE;

	WorldHeightMap *terrainHeightMap;				///< holds raw heightmap data samples

	CachedFileInputStream fileStrm;
	if ( !fileStrm.open(filename) )
	{
		return FALSE;
	}

	ChunkInputStream *pStrm = &fileStrm;

	// allocate new height map data to read from file
	// this will be a dummy object only containing logical map data
	terrainHeightMap = NEW WorldHeightMap(pStrm, true);

	if (terrainHeightMap)
	{	//copy loaded data
		// Get the whole map, because we don't know which boundary is active yet
		m_mapDX=terrainHeightMap->getXExtent();
		m_mapDY=terrainHeightMap->getYExtent();

		// now, get all the boudnaries, and set the current active boundary to boundary 0.
		m_boundaries = terrainHeightMap->getAllBoundaries();
		m_activeBoundary = 0;

		Int i, j, minHt, maxHt;

		minHt = terrainHeightMap->getMaxHeightValue();
		maxHt = 0;

		for (j=0; j<m_mapDY; j++) {
			for (i=0; i<m_mapDX; i++) {
				Short cur = terrainHeightMap->getHeight(i,j);
				if (cur<minHt) minHt = cur;
				if (maxHt<cur) maxHt = cur;
			}
		}
		m_mapMinZ = minHt * MAP_HEIGHT_SCALE;
		m_mapMaxZ = maxHt * MAP_HEIGHT_SCALE;
		//release temporary object used for loading height values
		REF_PTR_RELEASE(terrainHeightMap);
	}
	else
		return FALSE;	//could not create heightmap object.  File not found?

	// Note - It is very important that this get called AFTER the map is read in.  jba.
	// enhancing functionality
	if( TerrainLogic::loadMap( filename, query ) == false )
		return FALSE;

	// Map file now contains lighting & time of day info.
	if( TheWritableGlobalData->setTimeOfDay( TheGlobalData->m_timeOfDay ) )
		TheGameClient->setTimeOfDay( TheGlobalData->m_timeOfDay );

	return TRUE;  // success

}

//-------------------------------------------------------------------------------------------------
/** Get the 3D extent of the terrain in world coordinates */
//-------------------------------------------------------------------------------------------------
void W3DTerrainLogic::getExtent( Region3D *extent ) const
{
	extent->lo.x = 0.0f;

	extent->lo.y = 0.0f;

	// Note - m_boundaries are stored in height map grids wide, so we have to
	// multiply by the grid width.
	if (m_boundaries.size() > 0) {
		extent->hi.x = m_boundaries[m_activeBoundary].x*MAP_XY_FACTOR;
		extent->hi.y = m_boundaries[m_activeBoundary].y*MAP_XY_FACTOR;
	} else {
		extent->hi.x = 0.0f;
		extent->hi.y = 0.0f;
	}

	// Note - m_mapMin and Max Z are already in real coord space.
	extent->lo.z = m_mapMinZ;
	extent->hi.z = m_mapMaxZ;
}

//-------------------------------------------------------------------------------------------------
/** Get the 3D largest bounds defined in the map. */
//-------------------------------------------------------------------------------------------------
void W3DTerrainLogic::getMaximumPathfindExtent( Region3D *extent ) const
{
	extent->lo.x = 0.0f;

	extent->lo.y = 0.0f;
	extent->hi.x = 0.0f;
	extent->hi.y = 0.0f;

	// Note - m_boundaries are stored in height map grids wide, so we have to
	// multiply by the grid width.
	size_t i;
	for (i=0; i<m_boundaries.size(); i++) {
		if (extent->hi.x < m_boundaries[i].x*MAP_XY_FACTOR) {
			extent->hi.x = m_boundaries[i].x*MAP_XY_FACTOR;
		}
		if (extent->hi.y < m_boundaries[i].y*MAP_XY_FACTOR) {
			extent->hi.y = m_boundaries[i].y*MAP_XY_FACTOR;
		}
	}

	// Note - m_mapMin and Max Z are already in real coord space.
	extent->lo.z = m_mapMinZ;
	extent->hi.z = m_mapMaxZ;
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void W3DTerrainLogic::getExtentIncludingBorder( Region3D *extent ) const
{
	extent->lo.x = 0.0f;
	extent->lo.y = 0.0f;

	Real border = TheTerrainRenderObject->getMap()->getBorderSizeInline() * MAP_XY_FACTOR;
	extent->lo.x -= border;
	extent->lo.y -= border;
	extent->hi.x = (m_mapDX * MAP_XY_FACTOR)-border;
	extent->hi.y = (m_mapDY * MAP_XY_FACTOR)-border;
}

//-------------------------------------------------------------------------------------------------
Bool W3DTerrainLogic::isClearLineOfSight(const Coord3D& pos, const Coord3D& posOther) const
{
	if (TheTerrainRenderObject)
	{
		return TheTerrainRenderObject->isClearLineOfSight(pos, posOther);
	}
	else
	{
		return false;
	}
}

//-------------------------------------------------------------------------------------------------
/** W3D specific get height function for logical terrain */
//-------------------------------------------------------------------------------------------------
Real W3DTerrainLogic::getGroundHeight( Real x, Real y, Coord3D* normal ) const
{
#define USE_THE_TERRAIN_OBJECT
#ifdef USE_THE_TERRAIN_OBJECT
	// TheSuperHackers @logic-client-separation helmutbuhler 11/04/2025
	// W3DTerrainLogic shouldn't depend on TheTerrainRenderObject!
	if (TheTerrainRenderObject)
	{
		return TheTerrainRenderObject->getHeightMapHeight(x,y,normal);
	}
	else
	{
		if (normal)
		{
			//return a default normal pointing up
			normal->x=0.0f;
			normal->y=0.0f;
			normal->z=1.0f;
		}
		return 0;
	}
#endif
}

//-------------------------------------------------------------------------------------------------
/** Get the height considering the layer. */
//-------------------------------------------------------------------------------------------------
Real W3DTerrainLogic::getLayerHeight( Real x, Real y, PathfindLayerEnum layer, Coord3D* normal, Bool clip ) const
{
#ifdef USE_THE_TERRAIN_OBJECT

	if (!TheTerrainRenderObject)
	{
		if (normal)
		{
			//return a default normal pointing up
			normal->x=0.0f;
			normal->y=0.0f;
			normal->z=1.0f;
		}
		return 0;
	}

	Real height = TheTerrainRenderObject->getHeightMapHeight(x,y,normal);

	if (layer != LAYER_GROUND)
	{
		Coord3D loc;
		loc.x = x;
		loc.y = y;
		loc.z = height;
		if (layer == LAYER_WALL)
		{
			if (!clip || TheAI->pathfinder()->isPointOnWall(&loc))
			{
				return TheAI->pathfinder()->getWallHeight();
			}
			else
			{
				return height;
			}
		}
		Bridge* pBridge;
		if ((pBridge = findBridgeLayerAt(&loc, layer, clip)) != 0)
		{
			Real bridgeHeight = pBridge->getBridgeHeight(&loc, normal);
			if (bridgeHeight > height)
			{
				return bridgeHeight;	// Don't return bridge height if it's in the ground.
			}
		}
	}

	return height;

#endif
}

//-------------------------------------------------------------------------------------------------
/** W3D isCliffCell for terrain logic */
//-------------------------------------------------------------------------------------------------
Bool W3DTerrainLogic::isCliffCell( Real x, Real y) const
{
	// TheSuperHackers @logic-client-separation helmutbuhler 11/04/2025
	// W3DTerrainLogic shouldn't depend on TheTerrainRenderObject!
	return TheTerrainRenderObject->isCliffCell(x,y);

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void W3DTerrainLogic::crc( Xfer *xfer )
{

	// extend base class
	TerrainLogic::crc( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Xfer
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void W3DTerrainLogic::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	TerrainLogic::xfer( xfer );

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void W3DTerrainLogic::loadPostProcess( void )
{

	// extend base class
	TerrainLogic::loadPostProcess();

}
