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

// FILE: W3DRoadBuffer.h //////////////////////////////////////////////////
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
// File name:  W3DRoadBuffer.h
//
// Created:    John Ahlquist, May 2001
//
// Desc:       Draw buffer to handle all the roads in a scene.
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
//#include "common/GameFileSystem.h"
#include "Common/FileSystem.h" // for LOAD_TEST_ASSETS
#include "Lib/BaseType.h"
#include "Common/GameType.h"
#include "Common/AsciiString.h"

//-----------------------------------------------------------------------------
//           Forward References
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//           Type Defines
//-----------------------------------------------------------------------------


enum {bottomLeft=0, bottomRight=1, topLeft=2, topRight=3, NUM_CORNERS=4};
#define MAX_LINKS 6
#define DEFAULT_ROAD_SCALE (8.0f)
#define MIN_ROAD_SEGMENT (0.25f)


struct TRoadPt
{
	Vector2 loc;
	Vector2 top;
	Vector2 bottom;
	Int			count;
	Bool		last;
	Bool		multi;
	Bool		isAngled;
	Bool		isJoin;
};

struct TRoadSegInfo
{
	Vector2 loc;
	Vector2 roadNormal;
	Vector2 roadVector;
	Vector2 corners[NUM_CORNERS];
	Real uOffset;
	Real vOffset;
	Real scale;
};

// The individual data for a road segment.
enum {MAX_SEG_VERTEX=500, MAX_SEG_INDEX=2000};
enum TCorner CPP_11(: Int)
{
	SEGMENT,
	CURVE,
	TEE,
	FOUR_WAY,
	THREE_WAY_Y,
	THREE_WAY_H,
	THREE_WAY_H_FLIP,
	ALPHA_JOIN,
	NUM_JOINS
};

class RoadSegment
{
public:
	TRoadPt		m_pt1;					///< Drawing location pt1 to pt2.
	TRoadPt		m_pt2;					///< Drawing location to.
	Real			m_curveRadius;	///< Radius if it is a curve.
	TCorner		m_type;				///< Segment, or curve, or intersection.
	Real			m_scale;				///< Scale.
	Real			m_widthInTexture; ///< Width of the road in the texture.
	Int				m_uniqueID;			///< Road type.
protected:
	Int										m_numVertex;
	VertexFormatXYZDUV1*	m_vb;
	Int										m_numIndex;
	UnsignedShort*				m_ib;
	TRoadSegInfo					m_info;
	SphereClass						m_bounds;
public:
	RoadSegment(void);
	~RoadSegment(void);
public:
	void SetVertexBuffer(VertexFormatXYZDUV1 *vb, Int numVertex);
	void SetIndexBuffer(UnsignedShort *ib, Int numIndex);
	void SetRoadSegInfo(TRoadSegInfo *pInfo) {m_info = *pInfo;};
	void GetRoadSegInfo(TRoadSegInfo *pInfo) {*pInfo = m_info;};
	const SphereClass &getBounds(void) {return m_bounds;};
	Int GetNumVertex(void) {return m_numVertex;};
	Int GetNumIndex(void) {return m_numIndex;};
	Int GetVertices(VertexFormatXYZDUV1 *destination_vb, Int numToCopy);
	Int GetIndices(UnsignedShort *destination_ib, Int numToCopy, Int offset);
	void updateSegLighting(void);
} ;

class RoadType {
public:
	RoadType(void);
	~RoadType(void);
protected:
	TextureClass *m_roadTexture;	///<Roads texture
	DX8VertexBufferClass	*m_vertexRoad;	///<Road vertex buffer.
	DX8IndexBufferClass			*m_indexRoad;	///<indices defining a triangles for the road drawing.
	Int			m_numRoadVertices; ///<Number of vertices used in m_vertexRoad.
	Int			m_numRoadIndices;	///<Number of indices used in b_indexRoad;
	Int					  m_uniqueID;     ///< ID of the road type in INI.
	Bool					m_isAutoLoaded;
	Int						m_stackingOrder; ///< Order in the drawing.  0 drawn first, then 1 and so on.

#ifdef LOAD_TEST_ASSETS
	AsciiString		m_texturePath;
#endif
public:
	void loadTexture(AsciiString path, Int id);
	void applyTexture(void);
	Int getStacking(void) {return m_stackingOrder;}
	void setStacking(Int order) {m_stackingOrder = order;}
	Int getUniqueID(void) {return m_uniqueID;};
	DX8VertexBufferClass	*getVB(void) {return m_vertexRoad;};
	DX8IndexBufferClass		*getIB(void) {return m_indexRoad;}
	Int getNumVertices(void) {return m_numRoadVertices;}
	void setNumIndices(Int num) {m_numRoadIndices=num;}
	void setNumVertices(Int num) {m_numRoadVertices=num;}
	Int getNumIndices(void) {return m_numRoadIndices;}
#ifdef LOAD_TEST_ASSETS
	void setAutoLoaded(void) {m_isAutoLoaded = true;};
	Bool isAutoLoaded(void) {return m_isAutoLoaded;};
	AsciiString getPath(void) {return(m_texturePath);};
	void loadTestTexture(void);
#endif
}	;

class WorldHeightMap;
//
// W3DRoadBuffer: Draw buffer for the roads.
//
//
class W3DRoadBuffer
{
friend class HeightMapRenderObjClass;
public:

	W3DRoadBuffer(void);
	~W3DRoadBuffer(void);
	/// Loads the roads from the map objects list.
	void loadRoads();
	/// Empties the road buffer.
	void clearAllRoads(void);
	/// Draws the roads.  Uses terrain bounds for culling.
	void drawRoads(CameraClass * camera, TextureClass *cloudTexture, TextureClass *noiseTexture, Bool wireframe,
																	Int minX, Int maxX, Int minY, Int maxY, RefRenderObjListIterator *pDynamicLightsIterator);
	/// Sets the map pointer.
	void setMap(WorldHeightMap *pMap);
	/// Updates the diffuse lighting in the buffers.
	void updateLighting(void);

protected:
	RoadType *m_roadTypes;	///<Roads texture
	RoadSegment	*m_roads;			///< The road buffer.  All roads are stored here.
	Int			m_numRoads;						///< Number of roads used in m_roads.
	Bool		m_initialized;		///< True if the subsystem initialized.
	WorldHeightMap *m_map;		///< Pointer to the height map data.
	RefRenderObjListIterator *m_lightsIterator;	///< Lighting iterator.
	Int m_minX, m_maxX, m_minY, m_maxY; ///< Bounds on the terrain to be rendered.
	Int m_curUniqueID;				///< Road type we are rendering at this pass.
	Int m_curRoadType;
#ifdef LOAD_TEST_ASSETS
	Int m_maxUID;				///< Maximum UID.
	Int m_curOpenRoad;  ///< First road type not used.
#endif

	Int m_maxRoadSegments;  ///< Size of m_roads.
	Int m_maxRoadVertex;		///< Size of m_vertexRoad.
	Int m_maxRoadIndex;			///< Size of m_indexRoad.
	Int m_maxRoadTypes;			///< Size of m_roadTypes.
	Int			m_curNumRoadVertices; ///<Number of vertices used in current road type.
	Int			m_curNumRoadIndices;	///<Number of indices used in current road type;

	void addMapObjects(void);
	void addMapObject(RoadSegment *pRoad, Bool updateTheCounts);
	void adjustStacking(Int topUniqueID, Int bottomUniqueID);
	Int findCrossTypeJoinVector(Vector2 loc, Vector2 *joinVector, Int uniqueID);
	void flipTheRoad(RoadSegment *pRoad) {TRoadPt tmp=pRoad->m_pt1; pRoad->m_pt1 = pRoad->m_pt2; pRoad->m_pt2 = tmp;}; ///< Flips the loc1 and loc2 info.
	void insertCurveSegmentAt(Int ndx1, Int ndx2);
	void insertCrossTypeJoins(void);
	void insertJoinAt(Int ndx);
	void miter(Int ndx1, Int ndx2);
	void moveRoadSegTo(Int fromNdx, Int toNdx);
	void checkLinkAfter(Int ndx);
	void checkLinkBefore(Int ndx);
	void updateCounts(RoadSegment *pRoad);
	void updateCountsAndFlags(void);
	void insertCurveSegments(void);
	void insertTeeIntersections(void);
	void insertTee(Vector2 loc, Int index1, Real scale);
	Bool insertY(Vector2 loc, Int index1, Real scale);
	void insert4Way(Vector2 loc, Int index1, Real scale);
	void offset4Way(TRoadPt *pc1, TRoadPt *pc2, TRoadPt *pc3, TRoadPt *pr3, TRoadPt *pc4, Vector2 loc, Vector2 alignVector, Real widthInTexture);
	void offset3Way(TRoadPt *pc1, TRoadPt *pc2, TRoadPt *pc3, Vector2 loc, Vector2 upVector, Vector2 teeVector, Real widthInTexture);
	void offsetY(TRoadPt *pc1, TRoadPt *pc2, TRoadPt *pc3, Vector2 loc, Vector2 upVector, Real widthInTexture);
	void offsetH(TRoadPt *pc1, TRoadPt *pc2, TRoadPt *pc3, Vector2 loc, Vector2 upVector, Vector2 teeVector, Bool flip, Bool mirror, Real widthInTexture);
	void preloadRoadsInVertexAndIndexBuffers(void); ///< Fills the index and vertex buffers for drawing.
	void preloadRoadSegment(RoadSegment *pRoad); ///< Fills the index and vertex buffers for drawing 1 segment.
	void loadCurve(RoadSegment *pRoad, Vector2 loc1, Vector2 loc2, Real scale); ///< Fills the index and vertex buffers for drawing 1 fade.
	void loadTee(RoadSegment *pRoad, Vector2 loc1, Vector2 loc2, Bool is4way, Real scale); ///< Fills the index and vertex buffers for drawing 1 tee intersection.
	void loadY(RoadSegment *pRoad, Vector2 loc1, Vector2 loc2, Real scale); ///< Fills the index and vertex buffers for drawing 1 Y intersection.
	void loadAlphaJoin(RoadSegment *pRoad, Vector2 loc1, Vector2 loc2, Real scale); ///< Fills the index and vertex buffers for drawing 1 alpha blend cap.
	void loadH(RoadSegment *pRoad, Vector2 loc1, Vector2 loc2, Bool flip, Real scale); ///< Fills the index and vertex buffers for drawing 1 h tee intersection.
	void loadFloatSection(RoadSegment *pRoad, Vector2 loc,
														Vector2 roadVector, Real height, Real left, Real right, Real uOffset, Real vOffset, Real scale);
	void loadFloat4PtSection(RoadSegment *pRoad, Vector2 loc,
														Vector2 roadNormal, Vector2 roadVector,
														Vector2 *cornersP,
														Real uOffset, Real vOffset, Real uScale, Real vScale);
	void loadLit4PtSection(RoadSegment *pRoad, UnsignedShort *ib, VertexFormatXYZDUV1 *vb, RefRenderObjListIterator *pDynamicLightsIterator);
	void loadRoadsInVertexAndIndexBuffers(void); ///< Fills the index and vertex buffers for drawing.
	void loadLitRoadsInVertexAndIndexBuffers(RefRenderObjListIterator *pDynamicLightsIterator); ///< Fills the index and vertex buffers for drawing.
	void loadRoadSegment(UnsignedShort *ib, VertexFormatXYZDUV1 *vb, RoadSegment *pRoad); ///< Fills the index and vertex buffers for drawing 1 segment.
	void allocateRoadBuffers(void);							 ///< Allocates the buffers.
	void freeRoadBuffers(void);									 ///< Frees the index and vertex buffers.

	void rotateAbout(Vector2 *ptP, Vector2 center, Real angle);
};
