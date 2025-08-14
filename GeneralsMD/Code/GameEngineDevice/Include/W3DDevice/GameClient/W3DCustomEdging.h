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

// FILE: W3DCustomEdging.h //////////////////////////////////////////////////
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
// File name:  W3DCustomEdging.h
//
// Created:    John Ahlquist, May 2001
//
// Desc:       Draw buffer to handle all the trees in a scene.
//
//-----------------------------------------------------------------------------

#pragma once

#ifndef __W3DCUSTOM_EDGING_H_
#define __W3DCUSTOM_EDGING_H_

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

class WorldHeightMap;
//-----------------------------------------------------------------------------
//           Type Defines
//-----------------------------------------------------------------------------

//
// W3DCustomEdging: Draw buffer for the trees.
//
//
class W3DCustomEdging
{
friend class HeightMapRenderObjClass;
public:

	W3DCustomEdging(void);
	~W3DCustomEdging(void);
	void addEdging(Coord3D location, Real scale, Real angle, AsciiString name, Bool visibleInMirror);
	/// Empties the tree buffer.
	void clearAllEdging(void);
	/// Draws the trees.  Uses camera for culling.
	void drawEdging( WorldHeightMap *pMap, Int minX, Int maxX, Int minY, Int maxY,
		TextureClass * terrainTexture, TextureClass * cloudTexture, TextureClass * noiseTexture );
	/// Called when the view changes, and sort key needs to be recalculated.
	/// Normally sortKey gets calculated when a tree becomes visible.
	void doFullUpdate(void) {clearAllEdging();};
protected:
#define MAX_BLENDS 2000
	enum { MAX_EDGE_VERTEX=4*MAX_BLENDS,
					MAX_EDGE_INDEX=6*MAX_BLENDS};
	DX8VertexBufferClass	*m_vertexEdging;	///<Edging vertex buffer.
	DX8IndexBufferClass			*m_indexEdging;	///<indices defining a triangles for the tree drawing.
	Int			m_curNumEdgingVertices; ///<Number of vertices used in m_vertexEdging.
	Int			m_curNumEdgingIndices;	///<Number of indices used in b_indexEdging;
	Int			m_curEdgingIndexOffset;	///<First index to draw at.  We draw the trees backwards by filling up the index buffer backwards,
																// so any trees that don't fit are far away from the camera.
	Bool		m_anythingChanged;	///< Set to true if visibility or sorting changed.
	Bool		m_initialized;		///< True if the subsystem initialized.

	void allocateEdgingBuffers(void);							 ///< Allocates the buffers.
	void freeEdgingBuffers(void);									 ///< Frees the index and vertex buffers.
	void loadEdgingsInVertexAndIndexBuffers(WorldHeightMap *pMap, Int minX, Int maxX, Int minY, Int maxY);
};

#endif  // end __W3DCUSTOM_EDGING_H_
