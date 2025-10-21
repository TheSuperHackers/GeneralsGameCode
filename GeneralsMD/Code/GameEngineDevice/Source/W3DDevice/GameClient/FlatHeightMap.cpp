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

// FILE: Heightmap.cpp ////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: Heightmap.cpp
//
// Created:   Mark W., John Ahlquist, April/May 2001
//
// Desc:      Draw the terrain and scorchmarks in a scene.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//         Includes
//-----------------------------------------------------------------------------
#include "W3DDevice/GameClient/FlatHeightMap.h"

#include <stdlib.h>
#include <assetmgr.h>
#include <texture.h>
#include <tri.h>
#include <colmath.h>
#include <coltest.h>
#include <rinfo.h>
#include <camera.h>
#include <d3dx8core.h>
#include "Common/GlobalData.h"
#include "Common/PerfTimer.h"

#include "GameClient/TerrainVisual.h"
#include "GameClient/View.h"
#include "GameClient/Water.h"

#include "GameLogic/AIPathfind.h"
#include "GameLogic/TerrainLogic.h"
#include "W3DDevice/GameClient/TerrainTex.h"
#include "W3DDevice/GameClient/W3DDynamicLight.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DTerrainTracks.h"
#include "W3DDevice/GameClient/W3DTerrainBackground.h"
#include "W3DDevice/GameClient/W3DBibBuffer.h"
#include "W3DDevice/GameClient/W3DTreeBuffer.h"
#include "W3DDevice/GameClient/W3DRoadBuffer.h"
#include "W3DDevice/GameClient/W3DBridgeBuffer.h"
#include "W3DDevice/GameClient/W3DWaypointBuffer.h"
#include "W3DDevice/GameClient/W3DCustomEdging.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "W3DDevice/GameClient/W3DShaderManager.h"
#include "W3DDevice/GameClient/W3DShadow.h"
#include "W3DDevice/GameClient/W3DWater.h"
#include "W3DDevice/GameClient/W3DShroud.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/light.h"
#include "WW3D2/scene.h"
#include "W3DDevice/GameClient/W3DPoly.h"
#include "W3DDevice/GameClient/W3DCustomScene.h"

#include "Common/PerfTimer.h"
#include "Common/UnitTimings.h" //Contains the DO_UNIT_TIMINGS define jba.


FlatHeightMapRenderObjClass *TheFlatHeightMap = NULL;

//-----------------------------------------------------------------------------
//         Private Data
//-----------------------------------------------------------------------------
#define SC_DETAIL_BLEND ( SHADE_CNST(ShaderClass::PASS_LEQUAL, ShaderClass::DEPTH_WRITE_ENABLE, ShaderClass::COLOR_WRITE_ENABLE, ShaderClass::SRCBLEND_ONE, \
	ShaderClass::DSTBLEND_ZERO, ShaderClass::FOG_DISABLE, ShaderClass::GRADIENT_MODULATE, ShaderClass::SECONDARY_GRADIENT_DISABLE, ShaderClass::TEXTURING_ENABLE, \
	ShaderClass::ALPHATEST_DISABLE, ShaderClass::CULL_MODE_ENABLE, ShaderClass::DETAILCOLOR_SCALE, ShaderClass::DETAILALPHA_DISABLE) )

static ShaderClass detailOpaqueShader(SC_DETAIL_BLEND);


#define DEFAULT_MAX_BATCH_SHORELINE_TILES		512	//maximum number of terrain tiles rendered per call (must fit in one VB)
#define DEFAULT_MAX_MAP_SHORELINE_TILES		4096	//default size of array allocated to hold all map shoreline tiles.

#define ADJUST_FROM_INDEX_TO_REAL(k) ((k-m_map->getBorderSizeInline())*MAP_XY_FACTOR)
inline Int IABS(Int x) {	if (x>=0) return x; return -x;};

const Int CELLS_PER_TILE = 16; // In order to be efficient in texture, needs to be a power of 2. [3/24/2003]


//-----------------------------------------------------------------------------
//         Private Functions
//-----------------------------------------------------------------------------

//=============================================================================
// FlatHeightMapRenderObjClass::freeMapResources
//=============================================================================
/** Frees the w3d resources used to draw the terrain. */
//=============================================================================
Int FlatHeightMapRenderObjClass::freeMapResources(void)
{
	BaseHeightMapRenderObjClass::freeMapResources();

	return 0;
}




//-----------------------------------------------------------------------------
//         Public Functions
//-----------------------------------------------------------------------------

//=============================================================================
// FlatHeightMapRenderObjClass::~FlatHeightMapRenderObjClass
//=============================================================================
/** Destructor. Releases w3d assets. */
//=============================================================================
FlatHeightMapRenderObjClass::~FlatHeightMapRenderObjClass(void)
{
	releaseTiles();
	TheFlatHeightMap = NULL;
}

//=============================================================================
// FlatHeightMapRenderObjClass::FlatHeightMapRenderObjClass
//=============================================================================
/** Constructor. Mostly nulls out the member variables. */
//=============================================================================
FlatHeightMapRenderObjClass::FlatHeightMapRenderObjClass(void):
m_tiles(NULL),
m_tilesWidth(0),
m_tilesHeight(0),
m_numTiles(0),
m_updateState(STATE_IDLE)
{
	TheFlatHeightMap = this;
}


//=============================================================================
// FlatHeightMapRenderObjClass::adjustTerrainLOD
//=============================================================================
/** Adjust the terrain Level Of Detail.  If adj > 0 , increases LOD 1 step, if
adj < 0 decreases it one step, if adj==0, then just sets up for the current LOD */
//=============================================================================
void FlatHeightMapRenderObjClass::adjustTerrainLOD(Int adj)
{
	BaseHeightMapRenderObjClass::adjustTerrainLOD(adj);
}

//=============================================================================
// FlatHeightMapRenderObjClass::ReleaseResources
//=============================================================================
/** Releases all w3d assets, to prepare for Reset device call. */
//=============================================================================
void FlatHeightMapRenderObjClass::ReleaseResources(void)
{
	BaseHeightMapRenderObjClass::ReleaseResources();
}

//=============================================================================
// FlatHeightMapRenderObjClass::ReAcquireResources
//=============================================================================
/** Reallocates all W3D assets after a reset.. */
//=============================================================================
void FlatHeightMapRenderObjClass::ReAcquireResources(void)
{
	if (m_map) {
		Int width = (m_map->getXExtent()+CELLS_PER_TILE-2)/CELLS_PER_TILE;
		Int height = (m_map->getYExtent()+CELLS_PER_TILE-2)/CELLS_PER_TILE;
		Int i, j;
		Int numTiles = width*height;
		m_tiles = new W3DTerrainBackground[numTiles];
		m_numTiles = numTiles;
		m_tilesWidth = width;
		m_tilesHeight = height;
		for	(i=0; i<m_tilesWidth; i++) {
			for (j=0; j<m_tilesHeight; j++) {
				W3DTerrainBackground *tile = m_tiles+j*m_tilesWidth+i;
				tile->allocateTerrainBuffers(m_map, i*CELLS_PER_TILE, j*CELLS_PER_TILE, CELLS_PER_TILE);
				tile->setFlip(m_map);
			}
		}
		IRegion2D range;
		range.lo.x = 0;
		range.lo.y = 0;
		range.hi.x = m_map->getXExtent();
		range.hi.y = m_map->getYExtent();
		for	(i=0; i<m_tilesWidth; i++) {
			for (j=0; j<m_tilesHeight; j++) {
				W3DTerrainBackground *tile = m_tiles+j*m_tilesWidth+i;
				tile->doPartialUpdate(range, m_map, true);
			}
		}
	}
	BaseHeightMapRenderObjClass::ReAcquireResources();

}


//=============================================================================
// FlatHeightMapRenderObjClass::reset
//=============================================================================
/** Updates the macro noise/lightmap texture (pass 3) */
//=============================================================================
void FlatHeightMapRenderObjClass::reset(void)
{
	BaseHeightMapRenderObjClass::reset();
}

//=============================================================================
// FlatHeightMapRenderObjClass::oversizeTerrain
//=============================================================================
/** Sets the terrain oversize amount. */
//=============================================================================
void FlatHeightMapRenderObjClass::oversizeTerrain(Int tilesToOversize)
{
	// Not needed with flat version. [3/20/2003]
}



//=============================================================================
// HeightMapRenderObjClass::doPartialUpdate
//=============================================================================
/** Updates a partial block of vertices from [x0,y0 to x1,y1]
The coordinates in partialRange are map cell coordinates, relative to the entire map.
The vertex coordinates and texture coordinates, as well as static lighting are updated.
*/
void FlatHeightMapRenderObjClass::doPartialUpdate(const IRegion2D &partialRange, WorldHeightMap *htMap, RefRenderObjListIterator *pLightsIterator)
{
	if (htMap) {
		REF_PTR_SET(m_map, htMap);
	}
	Int i, j;
	for	(i=0; i<m_tilesWidth; i++) {
		for (j=0; j<m_tilesHeight; j++) {
			W3DTerrainBackground *tile = m_tiles+j*m_tilesWidth+i;
			tile->doPartialUpdate(partialRange, htMap, true);
		}
	}
}

//=============================================================================
// FlatHeightMapRenderObjClass::releaseTiles
//=============================================================================
/** Releases tiles.*/
//=============================================================================
void FlatHeightMapRenderObjClass::releaseTiles(void)
{
	delete [] m_tiles;
	m_tiles = NULL;

	m_tilesWidth = 0;
	m_tilesHeight = 0;
	m_numTiles = 0;
}


//=============================================================================
// FlatHeightMapRenderObjClass::initHeightData
//=============================================================================
/** Allocate a heightmap of x by y vertices and fill with initial height values.
Also allocates all rendering resources such as vertex buffers, index buffers,
shaders, and materials.*/
//=============================================================================
Int FlatHeightMapRenderObjClass::initHeightData(Int x, Int y, WorldHeightMap *pMap, RefRenderObjListIterator *pLightsIterator, Bool updateExtraPassTiles)
{

	BaseHeightMapRenderObjClass::initHeightData(x, y, pMap, pLightsIterator);

	Int width = (pMap->getXExtent()+CELLS_PER_TILE-2)/CELLS_PER_TILE;
	Int height = (pMap->getYExtent()+CELLS_PER_TILE-2)/CELLS_PER_TILE;

	Int numTiles = width*height;

	Int i, j;
	pMap->clearFlipStates();
	if (m_tiles && m_tilesWidth==width && m_tilesHeight==height) {
		// current allocation matches. Just redo vertex & index buffers. [3/21/2003]
		for	(i=0; i<m_tilesWidth; i++) {
			for (j=0; j<m_tilesHeight; j++) {
				W3DTerrainBackground *tile = m_tiles+j*m_tilesWidth+i;
				tile->setFlip(pMap);
			}
		}
	}	else {
		releaseTiles();
		m_tiles = new W3DTerrainBackground[numTiles];
		m_numTiles = numTiles;
		m_tilesWidth = width;
		m_tilesHeight = height;
		for	(i=0; i<m_tilesWidth; i++) {
			for (j=0; j<m_tilesHeight; j++) {
				W3DTerrainBackground *tile = m_tiles+j*m_tilesWidth+i;
				tile->allocateTerrainBuffers(pMap, i*CELLS_PER_TILE, j*CELLS_PER_TILE, CELLS_PER_TILE);
				tile->setFlip(pMap);
			}
		}
	}
	IRegion2D range;
	range.lo.x = 0;
	range.lo.y = 0;
	range.hi.x = pMap->getXExtent();
	range.hi.y = pMap->getYExtent();
	for	(i=0; i<m_tilesWidth; i++) {
		for (j=0; j<m_tilesHeight; j++) {
			W3DTerrainBackground *tile = m_tiles+j*m_tilesWidth+i;
			tile->doPartialUpdate(range, pMap, true);
		}
	}
	return 0;
}



//=============================================================================
// FlatHeightMapRenderObjClass::On_Frame_Update
//=============================================================================
/** Updates the diffuse color values in the vertices as affected by the dynamic lights.*/
//=============================================================================
void FlatHeightMapRenderObjClass::On_Frame_Update(void)
{
#ifdef DO_UNIT_TIMINGS
#pragma MESSAGE("*** WARNING *** DOING DO_UNIT_TIMINGS!!!!")
	return;
#endif

	BaseHeightMapRenderObjClass::On_Frame_Update();

	switch(m_updateState) {
		case STATE_IDLE: return;
		case STATE_MOVING : m_updateState = STATE_MOVING2; return;
		case STATE_MOVING2: {
			Int i, j;
			for	(i=0; i<m_tilesWidth; i++) {
				for (j=0; j<m_tilesHeight; j++) {
					W3DTerrainBackground *tile = m_tiles+j*m_tilesWidth+i;
					tile->updateTexture();
				}
			}
			m_updateState = STATE_IDLE;
		}
	}

}

//=============================================================================
// FlatHeightMapRenderObjClass::staticLightingChanged
//=============================================================================
/** Notification that all lighting needs to be recalculated. */
//=============================================================================
void FlatHeightMapRenderObjClass::staticLightingChanged( void )
{
	BaseHeightMapRenderObjClass::staticLightingChanged();
	if (m_map==NULL) {
		return;
	}
	Int i, j;
	IRegion2D bounds;
	bounds.lo.x = 0;
	bounds.lo.y = 0;
	bounds.hi.x = m_tilesWidth*CELLS_PER_TILE;
	bounds.hi.y = m_tilesHeight*CELLS_PER_TILE;
	for	(i=0; i<m_tilesWidth; i++) {
		for (j=0; j<m_tilesHeight; j++) {
			W3DTerrainBackground *tile = m_tiles+j*m_tilesWidth+i;
			tile->doPartialUpdate(bounds, m_map, true);
		}
	}
}


//=============================================================================
// FlatHeightMapRenderObjClass::updateCenter
//=============================================================================
/** Updates the positioning of the drawn portion of the height map in the
heightmap.  As the view slides around, this determines what is the actually
rendered portion of the terrain.  Only a 96x96 section is rendered at any time,
even though maps can be up to 1024x1024.  This function determines which subset
is rendered. */
//=============================================================================
void FlatHeightMapRenderObjClass::updateCenter(CameraClass *camera , RefRenderObjListIterator *pLightsIterator)
{
#ifdef DO_UNIT_TIMINGS
#pragma MESSAGE("*** WARNING *** DOING DO_UNIT_TIMINGS!!!!")
	return;
#endif
	BaseHeightMapRenderObjClass::updateCenter(camera, pLightsIterator);
	m_needFullUpdate = false;
	Int i, j;
	Int culled = 0;
	static Int prevCulled = 0;
	Int t2X = 0;
	static Int prevT2X = 0;
	Int t4X = 0;
	static Int prevT4X = 0;
	for	(i=0; i<m_tilesWidth; i++) {
		for (j=0; j<m_tilesHeight; j++) {
			W3DTerrainBackground *tile = m_tiles+j*m_tilesWidth+i;
			tile->updateCenter(camera);
			if (tile->isCulled()) {
				culled++;
			}
			Int tx = tile->getTexMultiplier();
			if (tx==4) {
				t4X++;
			} else if (tx==2) {
				t2X++;
			}
		}
	}
	if (culled!=prevCulled || t4X!=prevT4X || t2X!=prevT2X) {
		DEBUG_LOG(("%d of %d culled, %d 4X, %d 2X.", culled, m_numTiles, t4X, t2X));
		prevCulled = culled;
		prevT2X = t2X;
		prevT4X = t4X;
	}
	m_updateState = STATE_MOVING;
}

//=============================================================================
// FlatHeightMapRenderObjClass::Render
//=============================================================================
/** Renders (draws) the terrain. */
//=============================================================================
//DECLARE_PERF_TIMER(Terrain_Render)

void FlatHeightMapRenderObjClass::Render(RenderInfoClass & rinfo)
{
	//USE_PERF_TIMER(Terrain_Render)

	Int devicePasses;
	W3DShaderManager::ShaderTypes st;
	const Bool doCloud = useCloud();

	if (doCloud)
	{
		// TheSuperHackers @tweak Updates the cloud movement before applying it to the world.
		// Is now decoupled from logic step.
		W3DShaderManager::updateCloud();
	}

	Matrix3D tm(Transform);
	// If there are trees, tell them to draw at the transparent time to draw.
	if (m_treeBuffer) {
		m_treeBuffer->setIsTerrain();
	}


#ifdef DO_UNIT_TIMINGS
#pragma MESSAGE("*** WARNING *** DOING DO_UNIT_TIMINGS!!!!")
	return;
#endif

#ifdef EXTENDED_STATS
	if (DX8Wrapper::stats.m_disableTerrain) {
		return;
	}
#endif

	DX8Wrapper::Set_Light_Environment(rinfo.light_environment);

	// Force shaders to update.
	m_stageTwoTexture->restore();
	DX8Wrapper::Set_Texture(0,NULL);
	DX8Wrapper::Set_Texture(1,NULL);
	ShaderClass::Invalidate();

	//	tm.Scale(ObjSpaceExtent);
	DX8Wrapper::Set_Transform(D3DTS_WORLD,tm);


	DX8Wrapper::Set_Material(m_vertexMaterialClass);
	DX8Wrapper::Set_Shader(m_shaderClass);

 	st=W3DShaderManager::ST_FLAT_TERRAIN_BASE; //set default shader

 	//set correct shader based on current settings
 	if (TheGlobalData->m_useLightMap && doCloud)
 	{	st=W3DShaderManager::ST_FLAT_TERRAIN_BASE_NOISE12;
 	}
 	else
 	if (TheGlobalData->m_useLightMap)
 	{	//lightmap only
 		st=W3DShaderManager::ST_FLAT_TERRAIN_BASE_NOISE2;
 	}
 	else
 	if (doCloud)
 	{	//cloudmap only
 		st=W3DShaderManager::ST_FLAT_TERRAIN_BASE_NOISE1;
 	}



	//Find number of passes required to render current shader
 	devicePasses=W3DShaderManager::getShaderPasses(st);

 	if (m_disableTextures)
 		devicePasses=1;	//force to 1 lighting-only pass

 	//Specify all textures that this shader may need.
 	W3DShaderManager::setTexture(0,m_stageZeroTexture);
	if (m_shroud && rinfo.Additional_Pass_Count() && !m_disableTextures)
	{
		W3DShaderManager::setTexture(0,TheTerrainRenderObject->getShroud()->getShroudTexture());
	}

 	W3DShaderManager::setTexture(1,NULL);	// Set by the tile later. [3/31/2003]
 	W3DShaderManager::setTexture(2,m_stageTwoTexture);	//cloud
 	W3DShaderManager::setTexture(3,m_stageThreeTexture);//noise
	//Disable writes to destination alpha channel (if there is one)
	if (DX8Wrapper::getBackBufferFormat() == WW3D_FORMAT_A8R8G8B8) {
		DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE,D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_RED);
	}

	Int pass;
	Int yCoordMax = 0;
	Int yCoordMin = m_map->getXExtent();
	Int xCoordMax = 0;
	Int xCoordMin = m_map->getYExtent();
 	for (pass=0; pass<devicePasses; pass++) {
		Bool disableTex = m_disableTextures;
		if (m_disableTextures ) {
			DX8Wrapper::Set_Shader(ShaderClass::_PresetOpaque2DShader);
			DX8Wrapper::Set_Texture(0,NULL);
		} else {
			W3DShaderManager::setShader(st, pass);
		}

		Int i, j;
		for	(i=0; i<m_tilesWidth; i++) {
			for (j=0; j<m_tilesHeight; j++) {
				W3DTerrainBackground *tile = m_tiles+j*m_tilesWidth+i;
				if (pass>0) {
					disableTex = TRUE; // doing cloud/noise
				}
				if (!tile->isCulled()) {
					tile->drawVisiblePolys(rinfo, disableTex);
					if (i*CELLS_PER_TILE<xCoordMin) {
						xCoordMin = i*CELLS_PER_TILE;
					}
					if (j*CELLS_PER_TILE<yCoordMin) {
						yCoordMin = j*CELLS_PER_TILE;
					}
					if ((i+1)*CELLS_PER_TILE>xCoordMax) {
						xCoordMax = (i+1)*CELLS_PER_TILE;
					}
					if ((j+1)*CELLS_PER_TILE>yCoordMax) {
						yCoordMax = (j+1)*CELLS_PER_TILE;
					}
				}
			}
		}
	}

	if (pass)	//shader was applied at least once?
 		W3DShaderManager::resetShader(st);
#if 1

	//Draw feathered shorelines
	renderShoreLines(&rinfo.Camera);

#ifdef DO_ROADS
	DX8Wrapper::Set_Texture(0,NULL);
	DX8Wrapper::Set_Texture(1,NULL);
	m_stageTwoTexture->restore();

	ShaderClass::Invalidate();
	if (!ShaderClass::Is_Backface_Culling_Inverted()) {
		DX8Wrapper::Set_Material(m_vertexMaterialClass);
		if (Scene) {
			RTS3DScene *pMyScene = (RTS3DScene *)Scene;
			RefRenderObjListIterator pDynamicLightsIterator(pMyScene->getDynamicLights());
			m_roadBuffer->drawRoads(&rinfo.Camera, doCloud?m_stageTwoTexture:NULL, TheGlobalData->m_useLightMap?m_stageThreeTexture:NULL,
				m_disableTextures,xCoordMin-m_map->getBorderSizeInline(), xCoordMax-m_map->getBorderSizeInline(), yCoordMin-m_map->getBorderSizeInline(), yCoordMax-m_map->getBorderSizeInline(), &pDynamicLightsIterator);
		}
	}
#endif

#ifdef DO_SCORCH
	DX8Wrapper::Set_Texture(0,NULL);
	DX8Wrapper::Set_Texture(1,NULL);
	m_stageTwoTexture->restore();

	ShaderClass::Invalidate();
	if (!ShaderClass::Is_Backface_Culling_Inverted()) {
		drawScorches();
	}
#endif
	DX8Wrapper::Set_Texture(0,NULL);
	DX8Wrapper::Set_Texture(1,NULL);
	m_stageTwoTexture->restore();
	ShaderClass::Invalidate();
	DX8Wrapper::Apply_Render_State_Changes();

	m_bridgeBuffer->drawBridges(&rinfo.Camera, m_disableTextures, m_stageTwoTexture);

	if (TheTerrainTracksRenderObjClassSystem)
		TheTerrainTracksRenderObjClassSystem->flush();

	ShaderClass::Invalidate();
	DX8Wrapper::Apply_Render_State_Changes();

	m_waypointBuffer->drawWaypoints(rinfo);

	m_bibBuffer->renderBibs();
#endif
	// We do some custom blending, so tell the shader class to reset everything.
	DX8Wrapper::Set_Texture(0,NULL);
	DX8Wrapper::Set_Texture(1,NULL);
	m_stageTwoTexture->restore();
	ShaderClass::Invalidate();
	DX8Wrapper::Set_Material(NULL);

}

