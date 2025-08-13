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

// FILE: W3DBibBuffer.h //////////////////////////////////////////////////
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
// File name:  W3DBibBuffer.h
//
// Created:    John Ahlquist, May 2001
//
// Desc:       Draw buffer to handle all the bibs in a scene.
//
//-----------------------------------------------------------------------------

#pragma once

#ifndef __W3DBIB_BUFFER_H_
#define __W3DBIB_BUFFER_H_

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

//-----------------------------------------------------------------------------
//           Type Defines
//-----------------------------------------------------------------------------

/// The individual data for a Bib.
typedef struct {
	Vector3			m_corners[4];				///< Drawing location
	Bool				m_highlight;				///< Use the highlight texture.
	Int					m_color;						///< Tint perhaps.
	ObjectID		m_objectID;					///< The object id this bib corresponds to.
	DrawableID	m_drawableID;				///< The object id this bib corresponds to.
	Bool				m_unused;						///< True if this bib is currently unused.
} TBib;

//
// W3DBibBuffer: Draw buffer for the bibs.
//
//
class W3DBibBuffer
{
friend class BaseHeightMapRenderObjClass;
public:

	W3DBibBuffer(void);
	~W3DBibBuffer(void);
	/// Add a bib at location.  Name is the w3d model name.
	void addBib(Vector3 corners[4], ObjectID id, Bool highlight);
	void addBibDrawable(Vector3 corners[4], DrawableID id, Bool highlight);
	/// Add a bib at location.  Name is the w3d model name.
	void removeBib(ObjectID id);
	void removeBibDrawable(DrawableID id);
	/// Empties the bib buffer.
	void clearAllBibs(void);
	/// Removes highlighting.
	void removeHighlighting(void);
	/// Draws the bibs.
	void renderBibs();
	/// Called when the view changes, and sort key needs to be recalculated.
	/// Normally sortKey gets calculated when a bib becomes visible.
protected:
	enum { INITIAL_BIB_VERTEX=256,
					INITIAL_BIB_INDEX=384,
					MAX_BIBS=1000};
	DX8VertexBufferClass	*m_vertexBib;	///<Bib vertex buffer.
	Int										m_vertexBibSize; ///< Num vertices in bib buffer.
	DX8IndexBufferClass		*m_indexBib;	///<indices defining a triangles for the bib drawing.
	Int							  		m_indexBibSize;	///<indices available in m_indexBib.
	TextureClass *m_bibTexture;	///<Bibs texture
	TextureClass *m_highlightBibTexture;	///<Bibs texture
	Int			m_curNumBibVertices; ///<Number of vertices used in m_vertexBib.
	Int			m_curNumBibIndices;	///<Number of indices used in b_indexBib;
	Int			m_curNumNormalBibIndices; ///< Number of non-highlighted bib index.
	Int			m_curNumNormalBibVertex; ///< Number of non-highlighted bib vertex.

	TBib	m_bibs[MAX_BIBS];			///< The bib buffer.  All bibs are stored here.
	Int			m_numBibs;						///< Number of bibs in m_bibs.
	Bool		m_anythingChanged;	///< Set to true if visibility or sorting changed.
	Bool		m_updateAllKeys;  ///< Set to true when the view changes.
	Bool		m_initialized;		///< True if the subsystem initialized.
	Bool		m_isTerrainPass;  ///< True if the terrain was drawn in this W3D scene render pass.

	void loadBibsInVertexAndIndexBuffers(void); ///< Fills the index and vertex buffers for drawing.
	void allocateBibBuffers(void);							 ///< Allocates the buffers.
	void freeBibBuffers(void);									 ///< Frees the index and vertex buffers.
};

#endif  // end __W3DBIB_BUFFER_H_
