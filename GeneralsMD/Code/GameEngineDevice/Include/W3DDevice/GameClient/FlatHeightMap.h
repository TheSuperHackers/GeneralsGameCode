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

class W3DTerrainBackground;
/// Custom render object that draws the heightmap and handles intersection tests.
/**
Custom W3D render object that's used to process the terrain.  It handles
virtually everything to do with the terrain, including: drawing, lighting,
scorchmarks and intersection tests.
*/
class FlatHeightMapRenderObjClass : public BaseHeightMapRenderObjClass
{

public:

	FlatHeightMapRenderObjClass(void);
	virtual ~FlatHeightMapRenderObjClass(void);

	// DX8_CleanupHook methods
	virtual void ReleaseResources(void);	///< Release all dx8 resources so the device can be reset.
	virtual void ReAcquireResources(void);  ///< Reacquire all resources after device reset.


	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface (W3D methods)
	/////////////////////////////////////////////////////////////////////////////
	virtual void					Render(RenderInfoClass & rinfo);
	virtual void					On_Frame_Update(void);

	///allocate resources needed to render heightmap
	virtual int initHeightData(Int width, Int height, WorldHeightMap *pMap, RefRenderObjListIterator *pLightsIterator,Bool updateExtraPassTiles=TRUE);
	virtual Int freeMapResources(void);	///< free resources used to render heightmap
	virtual void updateCenter(CameraClass *camera, RefRenderObjListIterator *pLightsIterator);
 	virtual void adjustTerrainLOD(Int adj);
	virtual void reset(void);
	virtual void oversizeTerrain(Int tilesToOversize);
	virtual void staticLightingChanged(void);
	virtual void doPartialUpdate(const IRegion2D &partialRange, WorldHeightMap *htMap, RefRenderObjListIterator *pLightsIterator);
  virtual int updateBlock(Int x0, Int y0, Int x1, Int y1, WorldHeightMap *pMap, RefRenderObjListIterator *pLightsIterator){return 0;};

protected:
	W3DTerrainBackground	*m_tiles;
	Int										m_numTiles;
	Int										m_tilesWidth;
	Int										m_tilesHeight;

	enum {	STATE_IDLE,							 // sleeping
					STATE_MOVING,						 // camera moving, updating visibility.
					STATE_MOVING2, 					 // second moving state
					STATE_UPDATE_TEXTURES		 // Camera stopped, updating textures.
	} m_updateState;

protected:
	void releaseTiles(void);

};
