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

#ifndef __W3DBRIDGE_BUFFER_H_
#define __W3DBRIDGE_BUFFER_H_

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
#include "Common/Dict.h"
#include "Common/AsciiString.h"

//-----------------------------------------------------------------------------
//           Forward References
//-----------------------------------------------------------------------------
class MeshClass; 
class W3DTerrainLogic;
class W3DAssetManager;
class SimpleSceneClass;

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
	TextureClass *m_bridgeTexture;
	MeshClass *m_leftMesh;			///< W3D mesh models for the bridges.
	Matrix3D	m_leftMtx;				///< Transform for the left mesh.
	Real			m_minY;						///< min y vertex.
	Real			m_maxY;						///< max y vertex.
	Real			m_leftMinX;				///< m_leftMesh min x vertex.
	Real			m_leftMaxX;				///< m_leftMesh max x vertex.
	MeshClass *m_sectionMesh;		///< W3D mesh models for the bridges.
	Matrix3D	m_sectionMtx;			///< Transform for the section mesh.
	Real			m_sectionMinX;		///< m_sectionMesh min x vertex.
	Real			m_sectionMaxX;		///< m_sectionMesh max x vertex.
	MeshClass *m_rightMesh;			///< W3D mesh models for the bridges.
	Matrix3D	m_rightMtx;				///< Transform for the right mesh.
	Real			m_rightMinX;			///< m_rightMesh min x vertex.
	Real			m_rightMaxX;			///< m_rightMesh max x vertex.
	Int				m_firstIndex;			///< Starting index buffer.
	Int				m_numVertex;			///< Number of vertex used.
	Int				m_firstVertex;		///< First vertex.
	Int				m_numPolygons;		///< Number of polygons to draw.
	Bool			m_visible;
	AsciiString m_templateName;					///< Name of the bridge type.
	enum BodyDamageType m_curDamageState;
	Bool			m_enabled;

protected:
	Int getModelVerticesFixed(VertexFormatXYZNDUV1 *destination_vb, Int curVertex, const Matrix3D &mtx, MeshClass *pMesh, RefRenderObjListIterator *pLightsIterator);
	Int getModelIndices(UnsignedShort *destination_ib, Int curIndex, Int vertexOffset, MeshClass *pMesh);
	Int getModelVertices(VertexFormatXYZNDUV1 *destination_vb, Int curVertex,  Real xOffset,
																Vector3 &vec, Vector3 &vecNormal, Vector3 &vecZ, Vector3 &offset, 
																const Matrix3D &mtx, 
																MeshClass *pMesh, RefRenderObjListIterator *pLightsIterator);

public:
	W3DBridge(void);
	~W3DBridge(void);

	void init(Vector3 fromLoc, Vector3 toLoc, AsciiString name);
	AsciiString getTemplateName(void) {return m_templateName;}
	const Vector3* getStart(void) const {return &m_start;}
	const Vector3* getEnd(void) const { return &m_end;}
	Bool load(enum BodyDamageType curDamageState);
	enum BodyDamageType getDamageState(void) {return m_curDamageState;};
	void setDamageState(enum BodyDamageType state) { m_curDamageState = state;};
	void getIndicesNVertices(UnsignedShort *destination_ib, VertexFormatXYZNDUV1 *destination_vb, Int *curIndexP, Int *curVertexP, RefRenderObjListIterator *pLightsIterator);
	Bool cullBridge(CameraClass * camera);						 ///< Culls the bridges.  Returns true if visibility changed.
	void clearBridge(void);		///< Frees all objects associated with a bridge.
	Bool isVisible(void) {return m_visible;};
	Bool isEnabled(void) {return m_enabled;};
	void setEnabled(Bool enable) {m_enabled = enable;};
	void renderBridge(Bool wireframe);
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

	W3DBridgeBuffer(void);
	~W3DBridgeBuffer(void);
	/// Empties the bridge buffer.
	void clearAllBridges(void);
	/// Draws the bridges.  Uses camera for culling.
	void drawBridges(CameraClass * camera, Bool wireframe, TextureClass *cloudTexture);
	/// Called when the view changes, and sort key needs to be recalculated.
	/// Normally sortKey gets calculated when a bridge becomes visible.
	void doFullUpdate(void) {m_updateVis = true;};
	void loadBridges(W3DTerrainLogic *pTerrainLogic, Bool saveGame); ///< Loads the bridges from the map objects list.
	void worldBuilderUpdateBridgeTowers( W3DAssetManager *assetManager, SimpleSceneClass *scene );			///< for the editor and showing visual bridge towers
	void updateCenter(CameraClass *camera, RefRenderObjListIterator *pLightsIterator);
	enum { MAX_BRIDGE_VERTEX=12000, //make sure it stays under 65535
					MAX_BRIDGE_INDEX=2*MAX_BRIDGE_VERTEX,	//make sure it stays under 65535 
					MAX_BRIDGES=200};
protected:
	DX8VertexBufferClass	*m_vertexBridge;	///<Bridge vertex buffer.
	DX8IndexBufferClass			*m_indexBridge;	///<indices defining a triangles for the bridge drawing.
	VertexMaterialClass *m_vertexMaterial;
	TextureClass *m_bridgeTexture;	///<Bridges texture
	Int			m_curNumBridgeVertices; ///<Number of vertices used in m_vertexBridge.
	Int			m_curNumBridgeIndices;	///<Number of indices used in b_indexBridge;
	W3DBridge	m_bridges[MAX_BRIDGES];			///< The bridge buffer.  All bridges are stored here.
	Int			m_numBridges;						///< Number of bridges in m_bridges.
	Bool		m_initialized;		///< True if the subsystem initialized.
	Bool		m_updateVis;			///< True if the camera moved, and we need to recalculate visibility.
	Bool		m_anythingChanged;	///< Set to true if visibility changed.
	/// Add a bridge at location.  Name is the gdf item name.
	void addBridge(Vector3 fromLoc, Vector3 toLoc, AsciiString name, W3DTerrainLogic *pTerrainLogic, Dict *props);
	void loadBridgesInVertexAndIndexBuffers(RefRenderObjListIterator *pLightsIterator); ///< Fills the index and vertex buffers for drawing.
	void allocateBridgeBuffers(void);							 ///< Allocates the buffers.
	void cull(CameraClass * camera);						 ///< Culls the bridges.
	void freeBridgeBuffers(void);									 ///< Frees the index and vertex buffers.
};

#endif  // end __W3DBRIDGE_BUFFER_H_
