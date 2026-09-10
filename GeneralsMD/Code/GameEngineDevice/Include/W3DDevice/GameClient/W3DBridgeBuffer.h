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

// FILE: W3DBridgeBuffer.h //////////////////////////////////////////////////
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
// File name:  W3DBridgeBuffer.h
//
// Created:    John Ahlquist, May 2001
//
// Desc:       Draw buffer to handle all the bridges in a scene.
//
//-----------------------------------------------------------------------------

#pragma once
#include <array>
#include <span>
#include <vector>
import Graphics.Scene.Surfaces.Renderer;

//-----------------------------------------------------------------------------
//           Includes
//-----------------------------------------------------------------------------
#include "WWLib/always.h"
#include "W3DDevice/GameClient/W3DRenderObject.h"
import Graphics.Scene.Surfaces.Geometry;
import Graphics.Materials.State;
#include "Lib/BaseType.h"
#include "Common/GameType.h"
#include "Common/Dict.h"
#include "Common/AsciiString.h"

//-----------------------------------------------------------------------------
//           Forward References
//-----------------------------------------------------------------------------
class W3DMeshRenderObject;
class W3DTerrainLogic;
class W3DAssetManager;
class W3DSimpleScene;
enum BodyDamageType CPP_11(: Int);

//-----------------------------------------------------------------------------
//           Type Defines
//-----------------------------------------------------------------------------

typedef enum {
	FIXED_BRIDGE = 0,
	SECTIONAL_BRIDGE = 1
} TBridgeType;

class BridgeInfo;
/// The individual data for a bridge.
class W3DBridge
{
protected:
	Vector3 m_start;						///< Drawing location
	Vector3 m_end;							///< Drawing location
	Real		m_scale;						///< Width scale.
	Real		m_length;
	TBridgeType	m_bridgeType;		///< Type of bridge.  Currently only 2 supported.
	SphereClass m_bounds;				///< Bounding sphere for culling to set the visible flag.
	W3DTextureHandle *m_bridgeTexture;
	W3DMeshRenderObject *m_leftMesh;			///< W3D mesh models for the bridges.
	Matrix3D	m_leftMtx;				///< Transform for the left mesh.
	Real			m_minY;						///< min y vertex.
	Real			m_maxY;						///< max y vertex.
	Real			m_leftMinX;				///< m_leftMesh min x vertex.
	Real			m_leftMaxX;				///< m_leftMesh max x vertex.
	W3DMeshRenderObject *m_sectionMesh;		///< W3D mesh models for the bridges.
	Matrix3D	m_sectionMtx;			///< Transform for the section mesh.
	Real			m_sectionMinX;		///< m_sectionMesh min x vertex.
	Real			m_sectionMaxX;		///< m_sectionMesh max x vertex.
	W3DMeshRenderObject *m_rightMesh;			///< W3D mesh models for the bridges.
	Matrix3D	m_rightMtx;				///< Transform for the right mesh.
	Real			m_rightMinX;			///< m_rightMesh min x vertex.
	Real			m_rightMaxX;			///< m_rightMesh max x vertex.
	Int				m_firstIndex;			///< Starting index buffer.
	Int				m_numVertex;			///< Number of vertex used.
	Int				m_firstVertex;		///< First vertex.
	Int				m_numPolygons;		///< Number of polygons to draw.
	Bool			m_visible;
	AsciiString m_templateName;					///< Name of the bridge type.
	BodyDamageType m_curDamageState;
	Bool			m_enabled;
    Graphics::SurfaceMeshHandle m_graphicsMesh;

protected:
	Int getModelVerticesFixed(Graphics::SurfaceVertex *destination_vb, Int curVertex, const Matrix3D &mtx, W3DMeshRenderObject *pMesh, Graphics::SceneObjectList<W3DRenderObject>::Cursor *pLightsIterator);
	Int getModelIndices(UnsignedShort *destination_ib, Int curIndex, Int vertexOffset, W3DMeshRenderObject *pMesh);
	Int getModelVertices(Graphics::SurfaceVertex *destination_vb, Int curVertex,  Real xOffset,
																Vector3 &vec, Vector3 &vecNormal, Vector3 &vecZ, Vector3 &offset,
																const Matrix3D &mtx,
																W3DMeshRenderObject *pMesh, Graphics::SceneObjectList<W3DRenderObject>::Cursor *pLightsIterator);

public:
	W3DBridge();
	~W3DBridge();

	void init(Vector3 fromLoc, Vector3 toLoc, AsciiString name);
	AsciiString getTemplateName() {return m_templateName;}
	const Vector3* getStart() const {return &m_start;}
	const Vector3* getEnd() const { return &m_end;}
	Bool load(BodyDamageType curDamageState);
	BodyDamageType getDamageState() {return m_curDamageState;};
	void setDamageState(BodyDamageType state) { m_curDamageState = state;};
	void getIndicesNVertices(UnsignedShort *destination_ib, Graphics::SurfaceVertex *destination_vb, Int *curIndexP, Int *curVertexP, Graphics::SceneObjectList<W3DRenderObject>::Cursor *pLightsIterator);
	Bool cullBridge(W3DCamera * camera);						 ///< Culls the bridges.  Returns true if visibility changed.
	void clearBridge();		///< Frees all objects associated with a bridge.
	Bool isVisible() {return m_visible;};
	Bool isEnabled() {return m_enabled;};
	void setEnabled(Bool enable) {m_enabled = enable;};
    bool uploadGeometry(std::span<const Graphics::SurfaceVertex> vertices, std::span<const UnsignedShort> indices);
    void releaseGeometry();
    Graphics::SurfaceMeshHandle getGraphicsMesh() const { return m_graphicsMesh; }
    W3DTextureHandle *getTexture() const { return m_bridgeTexture; }
	void getBridgeInfo(BridgeInfo *pInfo);
};

//
// W3DBridgeBuffer: Draw buffer for the bridges.
//
//
class W3DBridgeBuffer
{
friend class BaseHeightMapRenderObjClass;
public:

	W3DBridgeBuffer();
	~W3DBridgeBuffer();
	/// Empties the bridge buffer.
	void clearAllBridges();
	/// Draws the bridges.  Uses camera for culling.
	void drawBridges(W3DCamera * camera, Bool wireframe, W3DTextureHandle *cloudTexture);
	/// Called when the view changes, and sort key needs to be recalculated.
	/// Normally sortKey gets calculated when a bridge becomes visible.
	void doFullUpdate() {m_updateVis = true;};
	void loadBridges(W3DTerrainLogic *pTerrainLogic, Bool saveGame); ///< Loads the bridges from the map objects list.
	void worldBuilderUpdateBridgeTowers( W3DAssetManager *assetManager, W3DSimpleScene *scene );			///< for the editor and showing visual bridge towers
	void updateCenter(W3DCamera *camera, Graphics::SceneObjectList<W3DRenderObject>::Cursor *pLightsIterator);
	enum { MAX_BRIDGE_VERTEX=12000, //make sure it stays under 65535
					MAX_BRIDGE_INDEX=2*MAX_BRIDGE_VERTEX,	//make sure it stays under 65535
					MAX_BRIDGES=200};
protected:
    std::vector<Graphics::SurfaceVertex> m_vertices;
    std::vector<UnsignedShort> m_indices;
	W3DTextureHandle *m_bridgeTexture;	///<Bridges texture
	Int			m_curNumBridgeVertices; ///<Number of vertices used in m_vertexBridge.
	Int			m_curNumBridgeIndices;	///<Number of indices used in b_indexBridge;
	W3DBridge	m_bridges[MAX_BRIDGES];			///< The bridge buffer.  All bridges are stored here.
	Int			m_numBridges;						///< Number of bridges in m_bridges.
	Bool		m_initialized;		///< True if the subsystem initialized.
	Bool		m_updateVis;			///< True if the camera moved, and we need to recalculate visibility.
	Bool		m_anythingChanged;	///< Set to true if visibility changed.
	/// Add a bridge at location.  Name is the gdf item name.
	void addBridge(Vector3 fromLoc, Vector3 toLoc, AsciiString name, W3DTerrainLogic *pTerrainLogic, Dict *props);
	void loadBridgesInVertexAndIndexBuffers(Graphics::SceneObjectList<W3DRenderObject>::Cursor *pLightsIterator); ///< Fills the index and vertex buffers for drawing.
	void allocateBridgeBuffers();							 ///< Allocates the buffers.
	void cull(W3DCamera * camera);						 ///< Culls the bridges.
	void freeBridgeBuffers();									 ///< Frees the index and vertex buffers.
};
