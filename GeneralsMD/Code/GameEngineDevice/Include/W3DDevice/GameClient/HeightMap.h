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


#pragma once


#ifndef __HEIGHTMAP_H_
#define __HEIGHTMAP_H_

#include "always.h"
#include "rendobj.h"
#include "w3d_file.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "dx8wrapper.h"
#include "shader.h"
#include "vertmaterial.h"
#include "Lib/BaseType.h"
#include "Common/GameType.h"
#include "WorldHeightMap.h"
#include "BaseHeightMap.h"

#define VERTEX_BUFFER_TILE_LENGTH	32		//tiles of side length 32 (grid of 33x33 vertices).

// Adjust the triangles to make cliff sides most attractive.  jba.
#define FLIP_TRIANGLES 1


/// Custom render object that draws the heightmap and handles intersection tests.
/**
Custom W3D render object that's used to process the terrain.  It handles
virtually everything to do with the terrain, including: drawing, lighting,
scorchmarks and intersection tests.
*/



class HeightMapRenderObjClass : public BaseHeightMapRenderObjClass
{	

public:

	HeightMapRenderObjClass(void);
	virtual ~HeightMapRenderObjClass(void);

	// DX8_CleanupHook methods
	virtual void ReleaseResources(void);	///< Release all dx8 resources so the device can be reset.
	virtual void ReAcquireResources(void);  ///< Reacquire all resources after device reset.


	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface (W3D methods)
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Render(RenderInfoClass & rinfo);
	virtual void					On_Frame_Update(void); 

	///allocate resources needed to render heightmap
	virtual int initHeightData(Int width, Int height, WorldHeightMap *pMap, RefRenderObjListIterator *pLightsIterator, Bool updateExtraPassTiles=TRUE);
	virtual Int freeMapResources(void);	///< free resources used to render heightmap
	virtual void updateCenter(CameraClass *camera, RefRenderObjListIterator *pLightsIterator);

	void renderExtraBlendTiles(void);			///< render 3-way blend tiles that have blend of 3 textures.


	virtual void staticLightingChanged(void);
	virtual	void adjustTerrainLOD(Int adj);
	virtual void reset(void);
	virtual void doPartialUpdate(const IRegion2D &partialRange, WorldHeightMap *htMap, RefRenderObjListIterator *pLightsIterator);

	virtual void oversizeTerrain(Int tilesToOversize);
  
	virtual int updateBlock(Int x0, Int y0, Int x1, Int y1, WorldHeightMap *pMap, RefRenderObjListIterator *pLightsIterator);

protected:
	Int *m_extraBlendTilePositions;	///<array holding x,y tile positions of all extra blend tiles. (used for 3 textures per tile).
	Int m_numExtraBlendTiles;		///<number of blend tiles in m_extraBlendTilePositions.
	Int	m_numVisibleExtraBlendTiles; ///<number rendered last frame.	
	Int m_extraBlendTilePositionsSize;	//<total size of array including unused memory.
	DX8VertexBufferClass	**m_vertexBufferTiles;	///<collection of smaller vertex buffers that make up 1 heightmap
	char	**m_vertexBufferBackup;	///< In memory copy of the vertex buffer data for quick update of dynamic lighting.
	Int m_originX; ///<  Origin point in the grid.  Slides around.
	Int m_originY; ///< Origin point in the grid.  Slides around.
	DX8IndexBufferClass			*m_indexBuffer;	///<indices defining triangles in a VB tile.
	Int	m_numVBTilesX;	///<dimensions of array containing all the vertex buffers 
	Int	m_numVBTilesY;	///<dimensions of array containing all the vertex buffers
	Int m_numVertexBufferTiles;	///<number of vertex buffers needed to store this heightmap
	Int	m_numBlockColumnsInLastVB;///<a VB tile may be partially filled, this indicates how many 2x2 vertex blocks are filled.
	Int	m_numBlockRowsInLastVB;///<a VB tile may be partially filled, this indicates how many 2x2 vertex blocks are filled.


	UnsignedInt doTheDynamicLight(VERTEX_FORMAT *vb, VERTEX_FORMAT *vbMirror, Vector3*light, Vector3*normal, W3DDynamicLight *pLights[], Int numLights);
	Int getXWithOrigin(Int x);
	Int getYWithOrigin(Int x);
	///update vertex diffuse color for dynamic lights inside given rectangle
	Int updateVBForLight(DX8VertexBufferClass *pVB, char *data, Int x0, Int y0, Int x1, Int y1, Int originX, Int originY, W3DDynamicLight *pLights[], Int numLights);
	Int updateVBForLightOptimized(DX8VertexBufferClass	*pVB, char *data, Int x0, Int y0, Int x1, Int y1, Int originX, Int originY, W3DDynamicLight *pLights[], Int numLights);
	///update vertex buffer vertices inside given rectangle
	Int updateVB(DX8VertexBufferClass	*pVB, char *data, Int x0, Int y0, Int x1, Int y1, Int originX, Int originY, WorldHeightMap *pMap, RefRenderObjListIterator *pLightsIterator);
	///upate vertex buffers associated with the given rectangle
	void initDestAlphaLUT(void);	///<initialize water depth LUT stored in m_destAlphaTexture
	void renderTerrainPass(CameraClass *pCamera);	///< renders additional terrain pass.
	Int	getNumExtraBlendTiles(Bool visible) { return visible?m_numVisibleExtraBlendTiles:m_numExtraBlendTiles;}
	void freeIndexVertexBuffers(void);


  
};

#endif  // end __HEIGHTMAP_H_
