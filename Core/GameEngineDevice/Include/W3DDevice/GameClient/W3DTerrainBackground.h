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

// FILE: W3DTerrainBackground.h //////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:    RTS3
//
// File name:  W3DTerrainBackground.h
//
// Created:    John Ahlquist, May 2001
//
// Desc:       Draw buffer to handle all the bibs in a scene.
//
//-----------------------------------------------------------------------------

#pragma once

//-----------------------------------------------------------------------------
//           Includes
//-----------------------------------------------------------------------------
#include "always.h"
#include "rendobj.h"
#include "w3d_file.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "shader.h"
#include "vertmaterial.h"
#include "Lib/BaseType.h"
#include "Common/GameType.h"
#include "Common/AsciiString.h"

//-----------------------------------------------------------------------------
//           Forward References
//-----------------------------------------------------------------------------
class MeshClass;
class WorldHeightMap;
class TerrainTextureClass;

//-----------------------------------------------------------------------------
//           Type Defines
//-----------------------------------------------------------------------------

//
// W3DTerrainBackground: Draw buffer for the bibs.
//
//
class W3DTerrainBackground
{
friend class HeightMapRenderObjClass;
public:

	W3DTerrainBackground(void);
	~W3DTerrainBackground(void);
	/// Draws the terrain.
	void drawVisiblePolys(RenderInfoClass & rinfo, Bool disableTextures);
	void setFlip(WorldHeightMap *htMap); ///< Sets the flip bit for required vertices.
	void doPartialUpdate(const IRegion2D &partialRange, WorldHeightMap *htMap, Bool doTextures );
	void doTesselatedUpdate(const IRegion2D &partialRange, WorldHeightMap *htMap, Bool doTextures );
	void allocateTerrainBuffers(WorldHeightMap *htMap, Int xOrigin, Int yOrigin, Int width);							 ///< Allocates the buffers.
	void updateCenter(CameraClass *camera); // notify camera moved [3/24/2003]
	void updateTexture(void); // notify camera moved [3/24/2003]
	Bool isCulled(void) {return m_cullStatus==CULL_STATUS_INVISIBLE;}
	Int getTexMultiplier(void) {return m_texMultiplier;}
protected:
	enum {CULL_STATUS_UNKNOWN, CULL_STATUS_VISIBLE, CULL_STATUS_INVISIBLE} m_cullStatus;
	AABoxClass						m_bounds;

	DX8VertexBufferClass	*m_vertexTerrain;	///<Terrain vertex buffer.
	Int										m_vertexTerrainSize; ///< Num vertices in bib buffer.
	DX8IndexBufferClass		*m_indexTerrain;	///<indices defining a triangles for the bib drawing.
	Int							  		m_indexTerrainSize;	///<indices available in m_indexTerrain.
	TerrainTextureClass *m_terrainTexture;	///<Terrain texture
	TerrainTextureClass *m_terrainTexture2X;	///<Terrain texture
	TerrainTextureClass *m_terrainTexture4X;	///<Terrain texture
	enum {TEX4X=4, TEX2X=2, TEX1X=1} m_texMultiplier;
	Int			m_curNumTerrainVertices; ///<Number of vertices used in m_vertexTerrain.
	Int			m_curNumTerrainIndices;	///<Number of indices used in b_indexTerrain;

	Int			m_xOrigin;
	Int			m_yOrigin;
	Int			m_width;
	WorldHeightMap *m_map;

	Bool		m_anythingChanged;	///< Set to true if visibility or sorting changed.
	Bool		m_initialized;		///< True if the subsystem initialized.

protected:
	typedef enum {HORIZONTAL, VERTICAL} TDirection;
	void freeTerrainBuffers(void);									 ///< Frees the index and vertex buffers.
	void fillVBRecursive(UnsignedShort *ib, Int xOffset, Int yOffset, Int width, UnsignedShort *ndx, Int &curIndex);
	void setFlipRecursive(Int xOffset, Int yOffset, Int width);
	Bool advanceLeft(ICoord2D &left, Int xOffset, Int yOffset, Int width);
	Bool advanceRight(ICoord2D &left, Int xOffset, Int yOffset, Int width);
};
