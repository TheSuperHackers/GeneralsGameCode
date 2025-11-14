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

// FILE: W3DTreeBuffer.h //////////////////////////////////////////////////
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
// File name:  W3DTreeBuffer.h
//
// Created:    John Ahlquist, May 2001
//
// Desc:       Draw buffer to handle all the trees in a scene.
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

//-----------------------------------------------------------------------------
//           Type Defines
//-----------------------------------------------------------------------------

typedef enum {
	ALPINE_TREE = 0,
	DECIDUOUS_TREE = 1,
	SHRUB = 2,
	FENCE = 3
} TTreeType;

/// The individual data for a tree.
typedef struct {
	Vector3 location;					///< Drawing location
	Real		scale;						///< Scale at location.
	Real		sin;							///< Sine of the rotation angle at location.
	Real		cos;							///< Cosine of the rotation angle at location.
	Int			panelStart;				///< Index of the "panel" lod.
	TTreeType			treeType;		///< Type of tree.  Currently only 3 supported.
	Bool		visible;					///< Visible flag, updated each frame.
	Bool		mirrorVisible;		///< Possibly visible in mirror.
	Bool		rotates;					///< Trees rotate to follow the camera in single panel mode, fences don't
	SphereClass bounds;				///< Bounding sphere for culling to set the visible flag.
	Real		sortKey;					///< Sort key, essentially the distance along the look at vector.
} TTree;

//
// W3DTreeBuffer: Draw buffer for the trees.
//
//
class W3DTreeBuffer
{
friend class HeightMapRenderObjClass;
public:

	W3DTreeBuffer(void);
	~W3DTreeBuffer(void);
	/// Add a tree at location.  Name is the w3d model name.
	void addTree(Coord3D location, Real scale, Real angle, AsciiString name, Bool visibleInMirror);
	/// Empties the tree buffer.
	void clearAllTrees(void);
	/// Draws the trees.  Uses camera for culling.
	void drawTrees(CameraClass * camera, RefRenderObjListIterator *pDynamicLightsIterator);
	/// Called when the view changes, and sort key needs to be recalculated.
	/// Normally sortKey gets calculated when a tree becomes visible.
	void doFullUpdate(void) {m_updateAllKeys = true;};
	void setIsTerrain(void) {m_isTerrainPass = true;}; ///< Terrain calls this to tell trees to draw.
	Bool needToDraw(void) {return m_isTerrainPass;};
protected:
	enum { MAX_TREE_VERTEX=4000,
					MAX_TREE_INDEX=2*4000,
					MAX_TREES=2000};
	enum {MAX_TYPES = 4,
				SORT_ITERATIONS_PER_FRAME=10};
	DX8VertexBufferClass	*m_vertexTree;	///<Tree vertex buffer.
	DX8IndexBufferClass			*m_indexTree;	///<indices defining a triangles for the tree drawing.
	TextureClass *m_treeTexture;	///<Trees texture
	Int			m_curNumTreeVertices; ///<Number of vertices used in m_vertexTree.
	Int			m_curNumTreeIndices;	///<Number of indices used in b_indexTree;
	Int			m_curTreeIndexOffset;	///<First index to draw at.  We draw the trees backwards by filling up the index buffer backwards,
																// so any trees that don't fit are far away from the camera.
	TTree	m_trees[MAX_TREES];			///< The tree buffer.  All trees are stored here.
	Int			m_numTrees;						///< Number of trees in m_trees.
	Bool		m_anythingChanged;	///< Set to true if visibility or sorting changed.
	Bool		m_updateAllKeys;  ///< Set to true when the view changes.
	Bool		m_initialized;		///< True if the subsystem initialized.
	Bool		m_isTerrainPass;  ///< True if the terrain was drawn in this W3D scene render pass.

	SphereClass m_typeBounds[MAX_TYPES];	///< Bounding boxes for the base tree models.
	MeshClass *m_typeMesh[MAX_TYPES];			///< W3D mesh models for the trees.
	Vector3 m_cameraLookAtVector;
	void loadTreesInVertexAndIndexBuffers(RefRenderObjListIterator *pDynamicLightsIterator); ///< Fills the index and vertex buffers for drawing.
	void allocateTreeBuffers(void);							 ///< Allocates the buffers.
	void cull(CameraClass * camera);						 ///< Culls the trees.
	void cullMirror(CameraClass * camera);		   ///< Culls the trees in the mirror view.
	Int  doLighting(Vector3 *loc, Real r, Real g, Real b, SphereClass &bounds, RefRenderObjListIterator *pDynamicLightsIterator);
	void freeTreeBuffers(void);									 ///< Frees the index and vertex buffers.
	void sort( Int iterations );								 ///< Performs partial bubble sort.
};
