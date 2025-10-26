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

// FILE: W3DRoadBuffer.cpp ////////////////////////////////////////////////
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
// File name: W3DRoadBuffer.cpp
//
// Created:   John Ahlquist, May 2001
//
// Desc:      Draw buffer to handle all the roads in a scene.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//         Includes
//-----------------------------------------------------------------------------
#include "W3DDevice/GameClient/W3DRoadBuffer.h"

#include <stdio.h>
#include <string.h>
#include <assetmgr.h>
#include <texture.h>
#include "Common/GlobalData.h"
#include "Common/RandomValue.h"
//#include "Common/GameFileSystem.h"
#include "Common/FileSystem.h" // for LOAD_TEST_ASSETS
#include "GameClient/TerrainRoads.h"
#include "W3DDevice/GameClient/TerrainTex.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/W3DDynamicLight.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "W3DDevice/GameClient/W3DShaderManager.h"
#include "WW3D2/camera.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/dx8renderer.h"
#include "WW3D2/mesh.h"
#include "WW3D2/meshmdl.h"

static const Real TEE_WIDTH_ADJUSTMENT = 1.03f;


//-----------------------------------------------------------------------------
//         Private Data
//-----------------------------------------------------------------------------

// A W3D shader that does alpha, texturing, tests zbuffer, doesn't update zbuffer.
#define SC_ALPHA_DETAIL ( SHADE_CNST(ShaderClass::PASS_LEQUAL, ShaderClass::DEPTH_WRITE_DISABLE, ShaderClass::COLOR_WRITE_ENABLE, ShaderClass::SRCBLEND_SRC_ALPHA, \
	ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA, ShaderClass::FOG_DISABLE, ShaderClass::GRADIENT_MODULATE, ShaderClass::SECONDARY_GRADIENT_DISABLE, ShaderClass::TEXTURING_ENABLE, \
	ShaderClass::ALPHATEST_DISABLE, ShaderClass::CULL_MODE_ENABLE, \
	ShaderClass::DETAILCOLOR_SCALE, ShaderClass::DETAILALPHA_DISABLE) )

static ShaderClass detailAlphaShader(SC_ALPHA_DETAIL);


#define SC_ALPHA_MIRROR ( SHADE_CNST(ShaderClass::PASS_LEQUAL, ShaderClass::DEPTH_WRITE_DISABLE, ShaderClass::COLOR_WRITE_ENABLE, ShaderClass::SRCBLEND_ONE, \
	ShaderClass::DSTBLEND_ZERO, ShaderClass::FOG_DISABLE, ShaderClass::GRADIENT_MODULATE, ShaderClass::SECONDARY_GRADIENT_DISABLE, ShaderClass::TEXTURING_ENABLE, \
	ShaderClass::ALPHATEST_DISABLE, ShaderClass::CULL_MODE_DISABLE, \
	ShaderClass::DETAILCOLOR_DISABLE, ShaderClass::DETAILALPHA_DISABLE) )

static ShaderClass detailShader(SC_ALPHA_MIRROR);


// The radius of the center of the road is 1.5 - outer radius 2.0, inner radius 1.0.
static const Real CORNER_RADIUS = 1.5f;


// The radius of the center of the road is 0.5 - outer radius 1.0, inner radius 0.0.
static const Real TIGHT_CORNER_RADIUS = 0.5f;
/*

// ShaderClass::PASS_ALWAYS,

#define SC_ALPHA_2D ( SHADE_CNST(PASS_ALWAYS, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, \
	SRCBLEND_SRC_ALPHA, DSTBLEND_ONE_MINUS_SRC_ALPHA, FOG_DISABLE, GRADIENT_DISABLE, \
	SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE, \
	ALPHATEST_DISABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DE AILALPHA_DISABLE) )
ShaderClass ShaderClass::_PresetAlpha2DShader(SC_ALPHA_2D);
*/

/** Calculate the sign of the cross product.  If the tails of the vectors are both placed
at 0,0, then the cross product can be interpreted as -1 means v2 is to the right of v1,
1 means v2 is to the left of v1, and 0 means v2 is parallel to v1. */

static Int xpSign(const Vector2 &v1, const Vector2 &v2) {
	Real xpdct = v1.X*v2.Y - v1.Y*v2.X;
	if (xpdct<0) return -1;
	if (xpdct>0) return 1;
	return 0;
}

//-----------------------------------------------------------------------------
//         Private Class
//-----------------------------------------------------------------------------

//=============================================================================
// RoadType constructor
//=============================================================================
/** Nulls index & vertex data. */
//=============================================================================
RoadType::RoadType(void):
m_roadTexture(NULL),
m_vertexRoad(NULL),
m_indexRoad(NULL),
m_stackingOrder(0),
m_uniqueID(-1)
{
}

//=============================================================================
// RoadType destructor
//=============================================================================
/** Frees index & vertex data. */
//=============================================================================
RoadType::~RoadType(void)
{
	REF_PTR_RELEASE(m_roadTexture);
	REF_PTR_RELEASE(m_vertexRoad);
	REF_PTR_RELEASE(m_indexRoad);
}

//=============================================================================
// RoadType applyTexture
//=============================================================================
/** Sets the W3D texture. */
//=============================================================================
void RoadType::applyTexture(void)
{
 	W3DShaderManager::setTexture(0,m_roadTexture);
	DX8Wrapper::Set_Index_Buffer(m_indexRoad,0);
	DX8Wrapper::Set_Vertex_Buffer(m_vertexRoad);
}


//=============================================================================
// RoadType loadTexture
//=============================================================================
/** Sets the W3D texture. */
//=============================================================================
void RoadType::loadTexture(AsciiString path, Int ID)
{
	/// @todo - delay loading textures and only load textures referenced by map.
	WW3DAssetManager *pMgr = W3DAssetManager::Get_Instance();

	m_roadTexture = pMgr->Get_Texture(path.str(), MIP_LEVELS_3);

	m_roadTexture->Get_Filter().Set_Mip_Mapping( TextureFilterClass::FILTER_TYPE_BEST );

	m_roadTexture->Get_Filter().Set_U_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_REPEAT);
	m_roadTexture->Get_Filter().Set_V_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_REPEAT);

	m_vertexRoad=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZDUV1,TheGlobalData->m_maxRoadVertex+4,DX8VertexBufferClass::USAGE_DYNAMIC));
	m_indexRoad=NEW_REF(DX8IndexBufferClass,(TheGlobalData->m_maxRoadIndex+4, DX8IndexBufferClass::USAGE_DYNAMIC));
	m_numRoadVertices=0;
	m_numRoadIndices=0;

#ifdef LOAD_TEST_ASSETS
	m_texturePath = path;
#endif
	m_uniqueID = ID;
}

#ifdef LOAD_TEST_ASSETS
//=============================================================================
// RoadType loadTexture
//=============================================================================
/** Sets the W3D texture. */
//=============================================================================
void RoadType::loadTestTexture(void)
{
	if (m_isAutoLoaded && m_uniqueID>0 && !m_texturePath.isEmpty()) {
		/// @todo - delay loading textures and only load textures referenced by map.
		m_roadTexture = NEW_REF(TextureClass, (m_texturePath.str(), m_texturePath.str(), MIP_LEVELS_3));
		m_roadTexture->Get_Filter().Set_Mip_Mapping( TextureFilterClass::FILTER_TYPE_BEST );

		m_roadTexture->Get_Filter().Set_U_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_REPEAT);
		m_roadTexture->Get_Filter().Set_V_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_REPEAT);
	}
}
#endif

//=============================================================================
// RoadSegment constructor
//=============================================================================
/** Nulls index & vertex data. */
//=============================================================================
RoadSegment::RoadSegment(void)
{
	m_numVertex = 0;
	m_vb = NULL;
	m_numIndex = 0;
	m_ib = NULL;
}

//=============================================================================
// RoadSegment destructor
//=============================================================================
/** Frees index & vertex data. */
//=============================================================================
RoadSegment::~RoadSegment(void)
{
	m_numVertex = 0;
	delete[] m_vb;
	m_vb = NULL;

	m_numIndex = 0;
	delete[] m_ib;
	m_ib = NULL;
}



//=============================================================================
// RoadSegment::SetVertexBuffer
//=============================================================================
/** Allocates & sets the vertex entries. */
//=============================================================================
void RoadSegment::SetVertexBuffer(VertexFormatXYZDUV1 *vb, Int numVertex)
{
	delete[] m_vb;
	m_vb = NULL;
	m_numVertex = 0;

	Vector3 verts[MAX_SEG_VERTEX];
	if (numVertex<1 || numVertex > MAX_SEG_VERTEX)
		return;

	m_vb = NEW VertexFormatXYZDUV1[numVertex];	// pool[]ify
	if (!m_vb)
		return;

	m_numVertex = numVertex;
	memcpy(m_vb, vb, numVertex*sizeof(VertexFormatXYZDUV1));
	Int i;
	for (i=0; i<numVertex; i++) {
		verts[i].X = m_vb[i].x;
		verts[i].Y = m_vb[i].y;
		verts[i].Z = m_vb[i].z;
	}
	SphereClass bounds(verts, numVertex);
	m_bounds = bounds;
}

//=============================================================================
// RoadSegment::SetIndexBuffer
//=============================================================================
/** Allocates & sets the index entries. */
//=============================================================================
void RoadSegment::SetIndexBuffer(UnsignedShort *ib, Int numIndex)
{
	delete[] m_ib;
	m_ib = NULL;
	m_numIndex = 0;

	if (numIndex < 1 || numIndex > MAX_SEG_INDEX)
		return;

	m_ib = NEW UnsignedShort[numIndex];
	if (!m_ib)
		return;

	m_numIndex = numIndex;
	memcpy(m_ib, ib, numIndex*sizeof(UnsignedShort));
}

//=============================================================================
// RoadSegment::GetVertices
//=============================================================================
/** Copies vertex entries into destination_vb. */
//=============================================================================
Int RoadSegment::GetVertices(VertexFormatXYZDUV1 *destination_vb, Int numToCopy)
{
	if (m_vb == NULL || numToCopy<1) return	(0);
	if (numToCopy > m_numVertex) return(0);
	memcpy(destination_vb, m_vb, numToCopy*sizeof(VertexFormatXYZDUV1));
	return(numToCopy);
}

//=============================================================================
// RoadSegment::GetIndices
//=============================================================================
/** Copies index entries into destination_ib. */
//=============================================================================
Int RoadSegment::GetIndices(UnsignedShort *destination_ib, Int numToCopy, Int offset)
{
	if (m_ib == NULL || numToCopy<1) return	(0);
	if (numToCopy > m_numIndex) return(0);
	Int i;
	for (i=0; i<numToCopy; i++) {
		destination_ib[i] = m_ib[i]+offset;
	}
	return(numToCopy);
}

//=============================================================================
// RoadSegment::updateSegLighting
//=============================================================================
/** Updates the diffuse lighting in the vertex buffer. */
//=============================================================================
void RoadSegment::updateSegLighting(void)
{
	Int i;
	for (i=0; i<m_numVertex; i++) {
		Int x = m_vb[i].x/MAP_XY_FACTOR+0.5;
		Int y = m_vb[i].y/MAP_XY_FACTOR+0.5;
		x += TheTerrainRenderObject->getMap()->getBorderSize();
		y += TheTerrainRenderObject->getMap()->getBorderSize();
		m_vb[i].diffuse = (255<<24)|TheTerrainRenderObject->getStaticDiffuse(x, y);
	}
}

//-----------------------------------------------------------------------------
//         Private Methods
//-----------------------------------------------------------------------------

//=============================================================================
// W3DRoadBuffer::loadTee
//=============================================================================
/** Loads a tee into the vertex buffer for a road join. */
//=============================================================================
void W3DRoadBuffer::loadTee(RoadSegment *pRoad, Vector2 loc1,
														Vector2 loc2, Bool is4Way, Real scale)
{
	Vector2 roadVector(loc2.X-loc1.X, loc2.Y-loc1.Y);
	Vector2 roadNormal(-roadVector.Y, roadVector.X);
//	Real roadLen = scale;
	if (abs(roadVector.X) < MIN_ROAD_SEGMENT && abs(roadVector.Y) < MIN_ROAD_SEGMENT) {
		roadVector.Set(1.0f, 0.0f);
		roadNormal.Set(0.0f, 1.0f);
	} else {
		roadVector.Normalize();
		roadNormal.Normalize();
	}

	Real uOffset = (425.0f/512.0f);
	Real vOffset = (255.0f/512.0f);
	if (is4Way) {
		uOffset = (425.0f/512.0f);
		vOffset = (425.0f/512.0f);
	}
	Real teeFactor = scale*TEE_WIDTH_ADJUSTMENT/2.0f;
	Real left = pRoad->m_widthInTexture*scale/2.0f;
	loadFloatSection(pRoad, loc1, loc2-loc1, teeFactor, left, teeFactor, uOffset, vOffset, scale);
}

//=============================================================================
// W3DRoadBuffer::loadAlphaJoin
//=============================================================================
/** Loads an alpha join into the vertex buffer for a road join. */
//=============================================================================
void W3DRoadBuffer::loadAlphaJoin(RoadSegment *pRoad, Vector2 loc1,
														Vector2 loc2,Real scale)
{

	Vector2 roadVector(loc2.X-loc1.X, loc2.Y-loc1.Y);
	Vector2 roadNormal(-roadVector.Y, roadVector.X);
//	Real roadLen = scale;
	if (abs(roadVector.X) < MIN_ROAD_SEGMENT && abs(roadVector.Y) < MIN_ROAD_SEGMENT) {
		roadVector.Set(1.0f, 0.0f);
		roadNormal.Set(0.0f, 1.0f);
	} else {
		roadVector.Normalize();
		roadNormal.Normalize();
	}

	Real uOffset = (106.0f/512.0f);
	Real vOffset = (425.0f/512.0f);

	Real roadwidth = scale;
	Real uScale = pRoad->m_widthInTexture;

	roadVector *= roadwidth*48/128; // we just want 48 pixels.
	roadNormal *= uScale*(1+8.0f/128); // we want 8 extra pixels.


	Vector2	corners[NUM_CORNERS];

	corners[topLeft] =  loc1 + roadNormal*0.5f - roadVector*0.65f ;
	corners[bottomLeft] = corners[topLeft] - roadNormal;
	corners[bottomRight] = corners[bottomLeft] + roadVector;
	corners[topRight] = corners[topLeft] + roadVector;
	loadFloat4PtSection(pRoad, loc1, roadNormal, roadVector, corners, uOffset, vOffset, scale, uScale);
}

//=============================================================================
// W3DRoadBuffer::loadY
//=============================================================================
/** Loads a Y into the vertex buffer for a road join. */
//=============================================================================
void W3DRoadBuffer::loadY(RoadSegment *pRoad, Vector2 loc1,
														Vector2 loc2,Real scale)
{

	Vector2 roadVector(loc2.X-loc1.X, loc2.Y-loc1.Y);
	Vector2 roadNormal(-roadVector.Y, roadVector.X);
//	Real roadLen = scale;
	if (abs(roadVector.X) < MIN_ROAD_SEGMENT && abs(roadVector.Y) < MIN_ROAD_SEGMENT) {
		roadVector.Set(1.0f, 0.0f);
		roadNormal.Set(0.0f, 1.0f);
	} else {
		roadVector.Normalize();
		roadNormal.Normalize();
	}

	Real uOffset = (255.0f/512.0f);
	Real vOffset = (226.0f/512.0f);

	Real roadwidth = scale;

	roadVector *= roadwidth;
	roadNormal *= roadwidth;


	Vector2	corners[NUM_CORNERS];

	roadVector *= 1.59f;
	corners[topLeft] =  loc1 + roadNormal*0.29f - roadVector*0.5f ;
	corners[bottomLeft] = corners[topLeft] - roadNormal*1.08f;
	corners[bottomRight] = corners[bottomLeft] + roadVector;
	corners[topRight] = corners[topLeft] + roadVector;
	loadFloat4PtSection(pRoad, loc1, roadNormal, roadVector, corners, uOffset, vOffset, scale, scale);
}

//=============================================================================
// W3DRoadBuffer::loadH
//=============================================================================
/** Loads a h shaped tee into the vertex buffer for a road join. */
//=============================================================================
void W3DRoadBuffer::loadH(RoadSegment *pRoad, Vector2 loc1,
														Vector2 loc2, Bool flip, Real scale)
{

	Vector2 roadVector(loc2.X-loc1.X, loc2.Y-loc1.Y);
	Vector2 roadNormal(-roadVector.Y, roadVector.X);
//	Real roadLen = scale;
	if (abs(roadVector.X) < MIN_ROAD_SEGMENT && abs(roadVector.Y) < MIN_ROAD_SEGMENT) {
		roadVector.Set(1.0f, 0.0f);
		roadNormal.Set(0.0f, 1.0f);
	} else {
		roadVector.Normalize();
		roadNormal.Normalize();
	}

	Real uOffset = (202.0f/512.0f);
	Real vOffset = (364.0f/512.0f);

	Real roadwidth = scale;

	roadVector *= roadwidth;
	roadNormal *= roadwidth;


	Vector2	corners[NUM_CORNERS];

	roadNormal *= 1.35f;
	if (flip) {
		corners[bottomLeft] = loc1 - roadNormal*0.20f - roadVector*pRoad->m_widthInTexture/2;
	}	else {
		corners[bottomLeft] = loc1 - roadNormal*0.8f - roadVector*pRoad->m_widthInTexture/2;
	}
	Vector2 width = roadVector*pRoad->m_widthInTexture/2;
	width = width + roadVector * 1.2f;
	corners[bottomRight] = corners[bottomLeft] + width;
	corners[topRight] = corners[bottomRight] + roadNormal;
	corners[topLeft] = corners[bottomLeft] + roadNormal;
	if (flip) roadNormal = -roadNormal;
	loadFloat4PtSection(pRoad, loc1, roadNormal, roadVector, corners, uOffset, vOffset, scale, scale);
}

//=============================================================================
// W3DRoadBuffer::loadFloatSection
//=============================================================================
/** Loads a section of road using a mesh that floats a little above the
terrain. */
//=============================================================================
void W3DRoadBuffer::loadFloatSection(RoadSegment *pRoad, Vector2 loc,
														Vector2 roadVector, Real halfHeight, Real left, Real right,
														Real uOffset, Real vOffset, Real scale)
{
	if (m_map==NULL) {
		return;
	}

	Vector2 roadNormal(-roadVector.Y, roadVector.X);
	roadVector.Normalize();
	roadVector *= right;

	roadNormal.Normalize();
	if (halfHeight<0) halfHeight = -halfHeight;
	roadNormal *= halfHeight;

	Vector2 roadLeft = roadVector;
	roadLeft.Normalize();
	roadLeft *= left;
	roadVector += roadLeft;
	Vector2 leftCenter = loc;
	leftCenter -= roadLeft;

	Vector2	corners[NUM_CORNERS];

	corners[bottomLeft] = leftCenter - roadNormal;
	corners[bottomRight] = corners[bottomLeft];
	corners[bottomRight] += roadVector;
	corners[topRight] = corners[bottomRight];
 	corners[topRight] += 2*roadNormal;
	corners[topLeft] = corners[bottomLeft];
 	corners[topLeft] += 2*roadNormal;

	loadFloat4PtSection(pRoad, loc, roadNormal, roadVector, corners, uOffset, vOffset, scale, scale);

}

//=============================================================================
// W3DRoadBuffer::loadFloat4PtSection
//=============================================================================
/** Loads a section of road using a mesh that floats a little above the
terrain.  The road is loaded into the quadrilateral defined by the
4 corners points.  loc specifies the point where u==uOffset && v==vOffset, and
the road vector gives the direction of the road, and the road normal is perpendicular
to the road normal.  */
//=============================================================================
void W3DRoadBuffer::loadFloat4PtSection(RoadSegment *pRoad, Vector2 loc,
														Vector2 roadNormal, Vector2 roadVector,
														Vector2 *cornersP,
														Real uOffset, Real vOffset, Real uScale, Real vScale)
{

	const Real FLOAT_AMOUNT = MAP_HEIGHT_SCALE/8;
	const Real MAX_ERROR = MAP_HEIGHT_SCALE*1.1f;
	UnsignedShort ib[MAX_SEG_INDEX];
	VertexFormatXYZDUV1 vb[MAX_SEG_VERTEX];
	Int numRoadVertices = 0;
	Int numRoadIndices = 0;

	TRoadSegInfo info;
	info.loc = loc;
	info.roadNormal = roadNormal;
	info.roadVector = roadVector;
	info.corners[bottomLeft] = cornersP[bottomLeft];
	info.corners[bottomRight] = cornersP[bottomRight];
	info.corners[topLeft] = cornersP[topLeft];
	info.corners[topRight] = cornersP[topRight];
	info.uOffset = uOffset;
	info.vOffset = vOffset;
	info.scale = uScale;
	pRoad->SetRoadSegInfo(&info);



	Real roadLen = roadVector.Length();
	Real halfHeight = roadNormal.Length();
	roadNormal.Normalize();
	roadVector.Normalize();
	Vector2 curVector;
	Int uCount = (roadLen/MAP_XY_FACTOR)+1;
	if (uCount<2) uCount = 2;
	Int vCount = (2*halfHeight/MAP_XY_FACTOR)+1;
	if (vCount<2) vCount = 2;


	const int maxRows = 100;
	typedef struct {
		Bool collapsed;
		Bool deleted;
		Vector3 vtx[maxRows];
		Int diffuseRed;
		Bool lightGradient;
		Int vertexIndex[maxRows];
		Real uIndex;
	} TColumn;
	const Int DIFFUSE_LIMIT = 25; // if more than that, we tesselate :) jba.

	if (vCount>maxRows) vCount = maxRows;
	TColumn prevColumn, curColumn, nextColumn;

	prevColumn.deleted = true;
	curColumn.deleted = true;
	Int i, j, k;
	Vector2 v2 = cornersP[bottomLeft];
	Vector3 origin(v2.X, v2.Y, 0);
	v2 = cornersP[bottomRight] - cornersP[bottomLeft];
	Vector3 uVector1(v2.X, v2.Y, 0);
	v2 = cornersP[topRight] - cornersP[topLeft];
	Vector3 uVector2(v2.X, v2.Y, 0);
	v2 = cornersP[topLeft];
	Vector3 origin2(v2.X, v2.Y, 0);
	v2 = cornersP[topLeft] - cornersP[bottomLeft];
	Vector3 vVector1(v2.X, v2.Y, 0);
	v2 = cornersP[topRight] - cornersP[bottomRight];
	Vector3 vVector2(v2.X, v2.Y, 0);
	uVector2 += (vVector1 - vVector2);
	for (i=0; i<=uCount; i++) {
		Real iFactor = ((Real)i / (uCount-1));
		Real iBarFactor = 1.0f-iFactor;
		if (i<uCount) {
			nextColumn.collapsed = false;
			nextColumn.deleted = false;
			nextColumn.lightGradient = false;
			nextColumn.uIndex = i;

			Real minHeight=m_map->getMaxHeightValue()*MAP_HEIGHT_SCALE;
			Real maxHeight = m_map->getMinHeightValue()*MAP_HEIGHT_SCALE;
			for (j=0; j<vCount; j++) {
				Real jFactor = ((Real)j / (vCount-1));
				Real jBarFactor = 1.0f-jFactor;
				nextColumn.vtx[j] = origin +  (uVector1 * jBarFactor * iFactor) + (uVector2 * jFactor * iFactor) +
													(vVector1 * iBarFactor * jFactor) + (vVector2 * iFactor * jFactor) ;
				Real z = TheTerrainRenderObject->getMaxCellHeight(nextColumn.vtx[j].X, nextColumn.vtx[j].Y);
				if (z<minHeight) minHeight = z;
				if (z>maxHeight) maxHeight = z;
				nextColumn.vertexIndex[j] = -1;
				nextColumn.vtx[j].Z = z;
				Int diffuse = 0;
				if (j==0) {
					nextColumn.diffuseRed = (diffuse & 0x00ff);
				} else {
					Int red = diffuse&0x00ff;
					if (abs(red-nextColumn.diffuseRed) > DIFFUSE_LIMIT) {
						nextColumn.lightGradient = true;
					}
				}
			}

			if (true) { // !nextColumn.lightGradient) {
				nextColumn.collapsed = true;
				nextColumn.vtx[0].Z = maxHeight;
				nextColumn.vtx[1] = nextColumn.vtx[vCount-1];
				nextColumn.vtx[1].Z = maxHeight;
			}	else {
				for (j=0; j<vCount; j++) {
					nextColumn.vtx[j].Z = maxHeight;
				}
			}
			if (i<2) {
				curColumn = nextColumn;
			} else {
				if (prevColumn.collapsed && curColumn.collapsed && nextColumn.collapsed) {
					Bool okToDelete = false;

					Real theZ = prevColumn.vtx[0].Z * (curColumn.uIndex-prevColumn.uIndex) +
										nextColumn.vtx[0].Z * (nextColumn.uIndex-curColumn.uIndex);
					theZ /= nextColumn.uIndex-prevColumn.uIndex;
					if (theZ >= curColumn.vtx[0].Z && theZ < curColumn.vtx[0].Z + MAX_ERROR) {
						theZ = prevColumn.vtx[1].Z * (curColumn.uIndex-prevColumn.uIndex) +
											nextColumn.vtx[1].Z * (nextColumn.uIndex-curColumn.uIndex);
						theZ /= nextColumn.uIndex-prevColumn.uIndex;
						if (theZ >= curColumn.vtx[1].Z && theZ < curColumn.vtx[1].Z + MAX_ERROR) {
							okToDelete = true;
						}
					}
					if (okToDelete) {
						curColumn.deleted = true;
					}
				}
			}
		}
		if (!curColumn.deleted && i!=1) {
			// Write out the vertices.
			for (j=0; j<vCount; j++) {
				Real U, V;
				if (numRoadVertices >= MAX_SEG_INDEX) {
					break;
				}
				curVector.Set(curColumn.vtx[j].X - loc.X, curColumn.vtx[j].Y - loc.Y);
				V = Vector2::Dot_Product(roadNormal, curVector);
				U = Vector2::Dot_Product(roadVector, curVector);
				Int diffuse = 0;
			#ifdef RTS_DEBUG
				//diffuse &= 0xFFFF00FF; // strip out green.
			#endif
				vb[numRoadVertices].u1 = uOffset+U/(uScale*4);
				vb[numRoadVertices].v1 = vOffset-V/(vScale*4);	// Road is 1/16 texture height.
				vb[numRoadVertices].x = curColumn.vtx[j].X;
				vb[numRoadVertices].y = curColumn.vtx[j].Y;
				vb[numRoadVertices].z = curColumn.vtx[j].Z+FLOAT_AMOUNT;
				vb[numRoadVertices].diffuse = diffuse;
				curColumn.vertexIndex[j] = numRoadVertices;
				numRoadVertices++;
				if (j==1 && curColumn.collapsed) {
					break;
				}
			}
			if (numRoadVertices >= MAX_SEG_INDEX) {
				break;
			}
			if (i>1) {
				// Write out the triangles.
				j = 0;
				k = 0;
				while (j<vCount-1 && k<vCount-1) {
					if (numRoadIndices >= MAX_SEG_INDEX) {
						break;
					}
					UnsignedShort *curIb = ib+numRoadIndices;
					if (k==0 || !prevColumn.collapsed) {
						*curIb++ = prevColumn.vertexIndex[j+1];
						*curIb++ = prevColumn.vertexIndex[j];
						*curIb++ = curColumn.vertexIndex[k];
						numRoadIndices+=3;
					}
					if (j==0 || !curColumn.collapsed) {
						Int offset = 1;
						if (curColumn.collapsed && !prevColumn.collapsed) {
							offset = vCount-1;
						}
						*curIb++ = prevColumn.vertexIndex[j+offset];
						*curIb++ = curColumn.vertexIndex[k];
						*curIb++ = curColumn.vertexIndex[k+1];
						numRoadIndices+=3;
					}
					if (prevColumn.collapsed && curColumn.collapsed) {
						break;
					}
					if (!prevColumn.collapsed) {
						j++;
					}
					if (!curColumn.collapsed) {
						k++;
					}
				}
				prevColumn = curColumn;
			}	else if (i==0) {
				prevColumn = curColumn;
			}
			if (numRoadIndices >= MAX_SEG_INDEX) {
				break;
			}
		}
		curColumn = nextColumn;
	}
	pRoad->SetVertexBuffer(vb, numRoadVertices);
	pRoad->SetIndexBuffer(ib, numRoadIndices);
}

//=============================================================================
// W3DRoadBuffer::loadLit4PtSection
//=============================================================================
/** Loads a section of road using a mesh that floats a little above the
terrain.  The road is loaded into the quadrilateral defined by the
4 corners points.  loc specifies the point where u==uOffset && v==vOffset, and
the road vector gives the direction of the road, and the road normal is perpendicular
to the road normal.  */
//=============================================================================
void W3DRoadBuffer::loadLit4PtSection(RoadSegment *pRoad, UnsignedShort *ib, VertexFormatXYZDUV1 *vb, RefRenderObjListIterator *pDynamicLightsIterator)
{

	const Real FLOAT_AMOUNT = MAP_HEIGHT_SCALE/8;
	const Real MAX_ERROR = MAP_HEIGHT_SCALE*1.1f;


	if (pRoad->m_uniqueID != m_curUniqueID) {
		return;
	}
	// Throw out segs out of view.
	if (pRoad->m_pt1.loc.X + pRoad->m_scale/2 < this->m_minX &&
		pRoad->m_pt2.loc.X + pRoad->m_scale/2 < this->m_minX) {
		return;
	}
	if (pRoad->m_pt1.loc.X - pRoad->m_scale/2 > this->m_maxX &&
		pRoad->m_pt2.loc.X - pRoad->m_scale/2 > this->m_maxX) {
		return;
	}
	// Throw out segs out of view.
	if (pRoad->m_pt1.loc.Y + pRoad->m_scale/2 < this->m_minY &&
		pRoad->m_pt2.loc.Y + pRoad->m_scale/2 < this->m_minY) {
		return;
	}
	if (pRoad->m_pt1.loc.Y - pRoad->m_scale/2 > this->m_maxY &&
		pRoad->m_pt2.loc.Y - pRoad->m_scale/2 > this->m_maxY) {
		return;
	}

	Int numLights = 0;
	const Int maxLights = 8;
	LightClass *lights[maxLights];

	for (pDynamicLightsIterator->First(); !pDynamicLightsIterator->Is_Done(); pDynamicLightsIterator->Next()) {
			LightClass *pLight = (LightClass*)pDynamicLightsIterator->Peek_Obj();
			SphereClass bounds = pLight->Get_Bounding_Sphere();
			if (Spheres_Intersect(pRoad->getBounds(), bounds)) {
				lights[numLights] = pLight;
				numLights++;
				if (numLights == maxLights) break;
			}
	}

	if (numLights == 0) return;

	TRoadSegInfo info;
	pRoad->GetRoadSegInfo(&info);
	Real roadLen = info.roadVector.Length();
	Real halfHeight = info.roadNormal.Length();
	info.roadNormal.Normalize();
	info.roadVector.Normalize();
	Vector2 curVector;
	Int uCount = (roadLen/MAP_XY_FACTOR)+1;
	Int vCount = (2*halfHeight/MAP_XY_FACTOR)+1;


	const int maxRows = 100;
	typedef struct {
		Bool collapsed;
		Bool deleted;
		Vector3 vtx[maxRows];
		Int diffuseRed;
		Bool lightGradient;
		Int vertexIndex[maxRows];
		Real uIndex;
	} TColumn;
//	const Int DIFFUSE_LIMIT = 25; // if more than that, we tesselate :) jba.

	if (vCount>maxRows) vCount = maxRows;
	TColumn prevColumn, curColumn, nextColumn;

	prevColumn.deleted = true;
	curColumn.deleted = true;
	Int i, j, k;
	Vector2 v2 = info.corners[bottomLeft];
	Vector3 origin(v2.X, v2.Y, 0);
	v2 = info.corners[bottomRight] - info.corners[bottomLeft];
	Vector3 uVector1(v2.X, v2.Y, 0);
	v2 = info.corners[topRight] - info.corners[topLeft];
	Vector3 uVector2(v2.X, v2.Y, 0);
	v2 = info.corners[topLeft];
	Vector3 origin2(v2.X, v2.Y, 0);
	v2 = info.corners[topLeft] - info.corners[bottomLeft];
	Vector3 vVector1(v2.X, v2.Y, 0);
	v2 = info.corners[topRight] - info.corners[bottomRight];
	Vector3 vVector2(v2.X, v2.Y, 0);
	uVector2 += (vVector1 - vVector2);
	for (i=0; i<=uCount; i++) {
		Real iFactor = ((Real)i / (uCount-1));
		Real iBarFactor = 1.0f-iFactor;
		if (i<uCount) {
			nextColumn.collapsed = false;
			nextColumn.deleted = false;
			nextColumn.lightGradient = false;
			nextColumn.uIndex = i;

			Real minHeight=m_map->getMaxHeightValue()*MAP_HEIGHT_SCALE;
			Real maxHeight = m_map->getMinHeightValue()*MAP_HEIGHT_SCALE;
			for (j=0; j<vCount; j++) {
				Real jFactor = ((Real)j / (vCount-1));
				Real jBarFactor = 1.0f-jFactor;
				nextColumn.vtx[j] = origin +  (uVector1 * jBarFactor * iFactor) + (uVector2 * jFactor * iFactor) +
													(vVector1 * iBarFactor * jFactor) + (vVector2 * iFactor * jFactor) ;
				Real z = TheTerrainRenderObject->getMaxCellHeight(nextColumn.vtx[j].X, nextColumn.vtx[j].Y);
				if (z<minHeight) minHeight = z;
				if (z>maxHeight) maxHeight = z;
				nextColumn.vertexIndex[j] = -1;
				nextColumn.vtx[j].Z = z;
				Int k;
				for (k=0; k<numLights; k++) {
					Vector3 offset = nextColumn.vtx[j] - lights[k]->Get_Position();
					Real range = lights[k]->Get_Attenuation_Range();
					// for culling, expand one cell radius.
					range += MAP_XY_FACTOR;
					if (offset.Length2() < range*range) {
						nextColumn.lightGradient = true;
					}
				}
			}
			if (!nextColumn.lightGradient) {
				nextColumn.collapsed = true;
				nextColumn.vtx[0].Z = maxHeight;
				nextColumn.vtx[1] = nextColumn.vtx[vCount-1];
				nextColumn.vtx[1].Z = maxHeight;
			}	else {
				for (j=0; j<vCount; j++) {
					nextColumn.vtx[j].Z = maxHeight;
				}
			}
			if (i<2) {
				curColumn = nextColumn;
			} else {
				if (prevColumn.collapsed && curColumn.collapsed && nextColumn.collapsed) {
					Bool okToDelete = false;

					Real theZ = prevColumn.vtx[0].Z * (curColumn.uIndex-prevColumn.uIndex) +
										nextColumn.vtx[0].Z * (nextColumn.uIndex-curColumn.uIndex);
					theZ /= nextColumn.uIndex-prevColumn.uIndex;
					if (theZ >= curColumn.vtx[0].Z && theZ < curColumn.vtx[0].Z + MAX_ERROR) {
						theZ = prevColumn.vtx[1].Z * (curColumn.uIndex-prevColumn.uIndex) +
											nextColumn.vtx[1].Z * (nextColumn.uIndex-curColumn.uIndex);
						theZ /= nextColumn.uIndex-prevColumn.uIndex;
						if (theZ >= curColumn.vtx[1].Z && theZ < curColumn.vtx[1].Z + MAX_ERROR) {
							okToDelete = true;
						}
					}
					if (okToDelete) {
						curColumn.deleted = true;
					}
				}
			}
		}
		if (!curColumn.deleted && i!=1) {
			// Write out the vertices.
			for (j=0; j<vCount; j++) {
				Real U, V;
				if (m_curNumRoadVertices >= m_maxRoadVertex) {
					break;
				}
				curVector.Set(curColumn.vtx[j].X - info.loc.X, curColumn.vtx[j].Y - info.loc.Y);
				V = Vector2::Dot_Product(info.roadNormal, curVector);
				U = Vector2::Dot_Product(info.roadVector, curVector);
				Int diffuse = (255<<24)|TheTerrainRenderObject->getStaticDiffuse(curColumn.vtx[j].X/MAP_XY_FACTOR+0.5, curColumn.vtx[j].Y/MAP_XY_FACTOR+0.5);
				Real shadeR, shadeG, shadeB;
				shadeB = (diffuse & 0xFF)/255.0;
				shadeG = ((diffuse>>8) & 0xFF)/255.0;
				shadeR = ((diffuse>>16) & 0xFF)/255.0;
				Int k;
				for (k=0; k<numLights; k++) {
					Real factor;
					if (lights[k]->Get_Type() == LightClass::POINT) {
						Vector3 lightLoc = lights[k]->Get_Position();
						Vector3 vtx = curColumn.vtx[j];
						Vector3 offset = vtx - lightLoc;
						double range, midRange;
						lights[k]->Get_Far_Attenuation_Range(midRange, range);
						if (vtx.X < lightLoc.X-range) continue;
						if (vtx.X > lightLoc.X+range) continue;
						if (vtx.Y < lightLoc.Y-range) continue;
						if (vtx.Y > lightLoc.Y+range) continue;
						Real dist = offset.Length();
						if (dist >= range) continue;
						if (midRange < 0.1) continue;
	#if 1
						factor = 1.0f - (dist - midRange) / (range - midRange);
	#else
						// f = 1.0 / (atten0 + d*atten1 + d*d/atten2);
						if (fabs(range-midRange)<1e-5)	{
							// if the attenuation range is too small assume uniform with cutoff
							factor = 1.0;
						}	else  {
							factor = 1.0f/(0.1+dist/midRange + 5.0f*dist*dist/(range*range));
						}
	#endif
						factor = WWMath::Clamp(factor,0.0f,1.0f);
						Real shade = 0.5f;
						shade *= factor;
						Vector3 diffuse;
						lights[k]->Get_Diffuse(&diffuse);
						Vector3 ambient;
						lights[k]->Get_Ambient(&ambient);
						if (shade > 1.0) shade = 1.0;
						if(shade < 0.0f) shade = 0.0f;
						shadeR += shade*diffuse.X;
						shadeG += shade*diffuse.Y;
						shadeB += shade*diffuse.Z;
						shadeR += factor*ambient.X;
						shadeG += factor*ambient.Y;
						shadeB += factor*ambient.Z;
					}
				}
 				if (shadeR > 1.0) shadeR = 1.0;
				if(shadeR < 0.0f) shadeR = 0.0f;
				if (shadeG > 1.0) shadeG = 1.0;
				if(shadeG < 0.0f) shadeG = 0.0f;
				if (shadeB > 1.0) shadeB = 1.0;
				if(shadeB < 0.0f) shadeB = 0.0f;
				shadeR*=255;
				shadeG*=255;
				shadeB*=255;
				diffuse=REAL_TO_INT(shadeB) | (REAL_TO_INT(shadeG) << 8) | (REAL_TO_INT(shadeR) << 16) | ((int)255 << 24);

			#ifdef RTS_DEBUG
				//diffuse &= 0xFFFF00FF; // strip out green.
			#endif
				vb[m_curNumRoadVertices].u1 = info.uOffset+U/(info.scale*4);
				vb[m_curNumRoadVertices].v1 = info.vOffset-V/(info.scale*4);	// Road is 1/16 texture height.
				vb[m_curNumRoadVertices].x = curColumn.vtx[j].X;
				vb[m_curNumRoadVertices].y = curColumn.vtx[j].Y;
				vb[m_curNumRoadVertices].z = curColumn.vtx[j].Z+FLOAT_AMOUNT;
				vb[m_curNumRoadVertices].diffuse = diffuse;
				curColumn.vertexIndex[j] = m_curNumRoadVertices;
				m_curNumRoadVertices++;
				if (j==1 && curColumn.collapsed) {
					break;
				}
			}
			if (m_curNumRoadVertices >= MAX_SEG_INDEX) {
				break;
			}
			if (i>1 && (!prevColumn.collapsed || !curColumn.collapsed)) {
				// Write out the triangles.
				j = 0;
				k = 0;
				while (j<vCount-1 && k<vCount-1) {
					if (m_curNumRoadIndices >= m_maxRoadIndex) {
						break;
					}
					UnsignedShort *curIb = ib+m_curNumRoadIndices;
					if (k==0 || !prevColumn.collapsed) {
						*curIb++ = prevColumn.vertexIndex[j+1];
						*curIb++ = prevColumn.vertexIndex[j];
						*curIb++ = curColumn.vertexIndex[k];
						m_curNumRoadIndices+=3;
					}
					if (j==0 || !curColumn.collapsed) {
						Int offset = 1;
						if (curColumn.collapsed && !prevColumn.collapsed) {
							offset = vCount-1;
						}
						*curIb++ = prevColumn.vertexIndex[j+offset];
						*curIb++ = curColumn.vertexIndex[k];
						*curIb++ = curColumn.vertexIndex[k+1];
						m_curNumRoadIndices+=3;
					}
					if (prevColumn.collapsed && curColumn.collapsed) {
						break;
					}
					if (!prevColumn.collapsed) {
						j++;
					}
					if (!curColumn.collapsed) {
						k++;
					}
				}
				prevColumn = curColumn;
			}	else if (i==0) {
				prevColumn = curColumn;
			}
			if (m_curNumRoadIndices >= MAX_SEG_INDEX) {
				break;
			}
		}
		curColumn = nextColumn;
	}
}

//=============================================================================
// W3DRoadBuffer::loadCurve
//=============================================================================
/** Loads a curve segment into the vertex buffer for a road end cap or join. */
//=============================================================================
void W3DRoadBuffer::loadCurve(RoadSegment *pRoad, Vector2 loc1, Vector2 loc2, Real scale)
{

	Real uOffset = (4.0f/512.0f);
	Real vOffset = (255.0f/512.0f);	 // CORNER_RADIUS radius 1.5 curve.
	if (pRoad->m_curveRadius == TIGHT_CORNER_RADIUS) {
		vOffset = (425.0f/512.0f);
	}

	Vector2 roadVector(loc2.X-loc1.X, loc2.Y-loc1.Y);
	Vector2 roadNormal(-roadVector.Y, roadVector.X);
	Real roadLen = scale;

	Real curveHeight = pRoad->m_widthInTexture*scale/2.0f;

	roadVector.Normalize();
	roadVector *= roadLen;

	roadNormal.Normalize();
	roadNormal *= abs(curveHeight);

	Vector2	corners[NUM_CORNERS];

	corners[bottomLeft] = loc1 - roadNormal;
	corners[bottomRight] = corners[bottomLeft];
	corners[bottomRight] += roadVector;
	corners[topRight] = corners[bottomRight];
 	corners[topRight] += 2*roadNormal;
	corners[topLeft] = corners[bottomLeft];
 	corners[topLeft] += 2*roadNormal;

	// Tweak a little
	if (pRoad->m_curveRadius == TIGHT_CORNER_RADIUS) {
		corners[bottomLeft] = loc1 - roadNormal;
		corners[bottomRight] = corners[bottomLeft];
		corners[bottomRight] += roadVector*0.5f;
		corners[topRight] = corners[bottomRight];
 		corners[topRight] += 2*roadNormal;
		corners[topLeft] = corners[bottomLeft];
 		corners[topLeft] += 2*roadNormal;

		corners[bottomRight] += roadVector*0.1f;
		corners[bottomRight] += roadNormal*0.2f;
		corners[bottomLeft] -= roadNormal*0.1f;
		corners[bottomLeft] -= roadVector*0.02f;
		corners[topLeft] -= roadVector*0.02f;

		corners[topRight] -= roadVector*0.4f;
		corners[topRight] += roadNormal*0.2f;
	} else {
		corners[bottomLeft] = loc1 - roadNormal;
		corners[bottomRight] = corners[bottomLeft];
		corners[bottomRight] += roadVector;
		corners[topRight] = corners[bottomRight];
 		corners[topRight] += 2*roadNormal;
		corners[topLeft] = corners[bottomLeft];
 		corners[topLeft] += 2*roadNormal;

		corners[bottomRight] += roadVector*0.1f;
		corners[bottomRight] += roadNormal*0.4f;
		corners[bottomLeft] -= roadNormal*0.2f;
		corners[bottomLeft] -= roadVector*0.02f;
		corners[topLeft] -= roadVector*0.02f;

		corners[topRight] -= roadVector*0.4f;
		corners[topRight] += roadNormal*0.4f;
	}

	loadFloat4PtSection(pRoad, loc1, roadNormal, roadVector, corners, uOffset, vOffset, scale, scale);

}

//=============================================================================
// W3DRoadBuffer::preloadRoadSegment
//=============================================================================
/** Loads a road segment into the vertex buffer for drawing. */
//=============================================================================
void W3DRoadBuffer::preloadRoadSegment(RoadSegment *pRoad)
{

	Vector2 roadVector = pRoad->m_pt2.loc-pRoad->m_pt1.loc;
	Vector2 roadNormal(-roadVector.Y, roadVector.X);
//	Real roadLen = roadVector.Length();

	Real uOffset = 0.0f;
	Real vOffset = (85.0f/512.0f);

	Real roadHeight = pRoad->m_widthInTexture*pRoad->m_scale/2.0f;
	roadNormal.Normalize();
	roadNormal *= roadHeight;

	Vector2 corners[NUM_CORNERS];
	corners[bottomLeft] = pRoad->m_pt1.bottom;
	corners[topLeft] = pRoad->m_pt1.top;
	corners[bottomRight] = pRoad->m_pt2.bottom;
	corners[topRight] = pRoad->m_pt2.top;
	loadFloat4PtSection(pRoad, pRoad->m_pt1.loc, roadNormal, roadVector, corners, uOffset, vOffset,
		pRoad->m_scale, pRoad->m_scale);

}

//=============================================================================
// W3DRoadBuffer::preloadRoadsInVertexAndIndexBuffers
//=============================================================================
/** Loads the roads into the vertex buffer for drawing. */
//=============================================================================
void W3DRoadBuffer::preloadRoadsInVertexAndIndexBuffers()
{
	if (!m_initialized) {
		return;
	}

	Int curRoad;

	// Do road segments.
	for (curRoad=0; curRoad<m_numRoads; curRoad++) {
		if (m_roads[curRoad].m_type == SEGMENT) {
			preloadRoadSegment(&m_roads[curRoad]);
		}
	}
	// Do curves.
	for (curRoad=0; curRoad<m_numRoads; curRoad++) {
		if (m_roads[curRoad].m_type == CURVE) {
			loadCurve(&m_roads[curRoad], m_roads[curRoad].m_pt1.loc, m_roads[curRoad].m_pt2.loc, m_roads[curRoad].m_scale);
		}
	}
	// Do tees.
	for (curRoad=0; curRoad<m_numRoads; curRoad++) {
		if (m_roads[curRoad].m_type == THREE_WAY_Y) {
			loadY(&m_roads[curRoad], m_roads[curRoad].m_pt1.loc, m_roads[curRoad].m_pt2.loc,
				m_roads[curRoad].m_scale);
		}
	}
	// Do tees.
	for (curRoad=0; curRoad<m_numRoads; curRoad++) {
		if (m_roads[curRoad].m_type == THREE_WAY_H || m_roads[curRoad].m_type == THREE_WAY_H_FLIP) {
			loadH(&m_roads[curRoad], m_roads[curRoad].m_pt1.loc, m_roads[curRoad].m_pt2.loc,
				m_roads[curRoad].m_type == THREE_WAY_H_FLIP, m_roads[curRoad].m_scale);
		}
	}
	// Do tees.
	for (curRoad=0; curRoad<m_numRoads; curRoad++) {
		if (m_roads[curRoad].m_type == TEE || m_roads[curRoad].m_type == FOUR_WAY) {
			loadTee(&m_roads[curRoad], m_roads[curRoad].m_pt1.loc, m_roads[curRoad].m_pt2.loc,
				(m_roads[curRoad].m_type == FOUR_WAY), m_roads[curRoad].m_scale);
		}
	}
	// Do joins.
	for (curRoad=0; curRoad<m_numRoads; curRoad++) {
		if (m_roads[curRoad].m_type == ALPHA_JOIN) {
			loadAlphaJoin(&m_roads[curRoad], m_roads[curRoad].m_pt1.loc, m_roads[curRoad].m_pt2.loc,
				 m_roads[curRoad].m_scale);
		}
	}
	updateLighting();
}

//=============================================================================
// W3DRoadBuffer::loadRoadsInVertexAndIndexBuffers
//=============================================================================
/** Loads the roads into the vertex buffer for drawing. */
//=============================================================================
void W3DRoadBuffer::loadRoadsInVertexAndIndexBuffers()
{
	if ( !m_initialized) {
		return;
	}
	m_curNumRoadVertices = 0;
	m_curNumRoadIndices = 0;
	VertexFormatXYZDUV1 *vb;
	UnsignedShort *ib;
	// Lock the buffers.
	if (m_roadTypes[m_curRoadType].getIB() == NULL) {
		this->m_roadTypes[m_curRoadType].setNumVertices(0);
		this->m_roadTypes[m_curRoadType].setNumIndices(0);
		return;
	}
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_roadTypes[m_curRoadType].getIB());
	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_roadTypes[m_curRoadType].getVB());
	vb=(VertexFormatXYZDUV1*)lockVtxBuffer.Get_Vertex_Array();
	ib = lockIdxBuffer.Get_Index_Array();
	// Add to the index buffer & vertex buffer.

	Int curRoad;

	// Do road segments.
	TCorner corner;
	for (corner = SEGMENT; corner < NUM_JOINS; corner = (TCorner)(corner+1)) {
		for (curRoad=0; curRoad<m_numRoads; curRoad++) {
			if (m_roads[curRoad].m_type == corner) {
				loadRoadSegment(ib, vb, &m_roads[curRoad]);
			}
		}
	}
	this->m_roadTypes[m_curRoadType].setNumVertices(m_curNumRoadVertices);
	this->m_roadTypes[m_curRoadType].setNumIndices(m_curNumRoadIndices);
}

//=============================================================================
// W3DRoadBuffer::loadLitRoadsInVertexAndIndexBuffers
//=============================================================================
/** Loads the roads into the vertex buffer for drawing. */
//=============================================================================
void W3DRoadBuffer::loadLitRoadsInVertexAndIndexBuffers(RefRenderObjListIterator *pDynamicLightsIterator)
{
	if ( !m_initialized) {
		return;
	}
	m_curNumRoadVertices = 0;
	m_curNumRoadIndices = 0;
	VertexFormatXYZDUV1 *vb;
	UnsignedShort *ib;
	// Lock the buffers.
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_roadTypes[m_curRoadType].getIB());
	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_roadTypes[m_curRoadType].getVB());
	vb=(VertexFormatXYZDUV1*)lockVtxBuffer.Get_Vertex_Array();
	ib = lockIdxBuffer.Get_Index_Array();
	// Add to the index buffer & vertex buffer.

	Int curRoad;
	if (true) {
		// Do road segments.
		TCorner corner;
		for (corner = SEGMENT; corner < NUM_JOINS; corner = (TCorner)(corner+1)) {
			for (curRoad=0; curRoad<m_numRoads; curRoad++) {
				if (m_roads[curRoad].m_type == corner) {
					loadLit4PtSection(&m_roads[curRoad], ib, vb, pDynamicLightsIterator);
				}
			}
		}
	}
	this->m_roadTypes[m_curRoadType].setNumVertices(m_curNumRoadVertices);
	this->m_roadTypes[m_curRoadType].setNumIndices(m_curNumRoadIndices);
}

//=============================================================================
// W3DRoadBuffer::loadRoadSegment
//=============================================================================
/** Loads a road segment into the vertex buffer for drawing. */
//=============================================================================
void W3DRoadBuffer::loadRoadSegment(UnsignedShort *ib, VertexFormatXYZDUV1 *vb, RoadSegment *pRoad)
{
	if (pRoad->m_uniqueID != m_curUniqueID) {
		return;
	}
	// Throw out segs out of view.
	if (pRoad->m_pt1.loc.X + pRoad->m_scale/2 < this->m_minX &&
		pRoad->m_pt2.loc.X + pRoad->m_scale/2 < this->m_minX) {
		return;
	}
	if (pRoad->m_pt1.loc.X - pRoad->m_scale/2 > this->m_maxX &&
		pRoad->m_pt2.loc.X - pRoad->m_scale/2 > this->m_maxX) {
		return;
	}
	// Throw out segs out of view.
	if (pRoad->m_pt1.loc.Y + pRoad->m_scale/2 < this->m_minY &&
		pRoad->m_pt2.loc.Y + pRoad->m_scale/2 < this->m_minY) {
		return;
	}
	if (pRoad->m_pt1.loc.Y - pRoad->m_scale/2 > this->m_maxY &&
		pRoad->m_pt2.loc.Y - pRoad->m_scale/2 > this->m_maxY) {
		return;
	}
	Int curVertex = m_curNumRoadVertices;
	if (curVertex+pRoad->GetNumVertex() >= m_maxRoadVertex) return;
	if (m_curNumRoadIndices+pRoad->GetNumIndex() >= m_maxRoadIndex) return;
	m_curNumRoadVertices += pRoad->GetVertices(vb+curVertex, pRoad->GetNumVertex());
	m_curNumRoadIndices += pRoad->GetIndices(ib+m_curNumRoadIndices, pRoad->GetNumIndex(), curVertex);

}


//=============================================================================
// W3DRoadBuffer::moveRoadSegTo
//=============================================================================
/** Moves a road segment. */
//=============================================================================
void W3DRoadBuffer::moveRoadSegTo(Int fromNdx, Int toNdx)
{
	if (fromNdx<0 || fromNdx>=m_numRoads || toNdx<0 || toNdx>=m_numRoads) {
#ifdef RTS_DEBUG
		DEBUG_LOG(("bad moveRoadSegTo"));
#endif
		return;
	}
	if (fromNdx == toNdx) return;
	RoadSegment cur = m_roads[fromNdx];
	Int i;
	if (fromNdx<toNdx) {
		for (i=fromNdx; i<toNdx; i++) {
			m_roads[i] = m_roads[i+1];
		}
	} else {
		for (i=fromNdx; i>toNdx; i--) {
			m_roads[i] = m_roads[i-1];
		}
	}
	m_roads[toNdx] = cur;

}
//=============================================================================
// W3DRoadBuffer::checkLinkBefore
//=============================================================================
/** Checks to see if any segments need to link before this one. */
//=============================================================================
void W3DRoadBuffer::checkLinkBefore(Int ndx)
{
	// Check for things whose loc == our loc2.
	if (m_roads[ndx].m_pt2.count != 1) {
		return;
	}

	Vector2 loc2 = m_roads[ndx].m_pt2.loc;
#ifdef RTS_DEBUG
	DEBUG_ASSERTLOG(m_roads[ndx].m_pt1.loc == m_roads[ndx+1].m_pt2.loc, ("Bad link"));
	if (ndx>0) {
		DEBUG_ASSERTLOG(m_roads[ndx].m_pt2.loc != m_roads[ndx-1].m_pt1.loc, ("Bad Link"));
	}
#endif

	Int endOfCurSeg = ndx+1;
	while (endOfCurSeg<m_numRoads-1) {
		if (m_roads[endOfCurSeg].m_pt1.loc != m_roads[endOfCurSeg+1].m_pt2.loc) {
			break;
		}
		endOfCurSeg++;
	}
	Int checkNdx = endOfCurSeg+1;
	while (checkNdx < m_numRoads) {
		if (m_roads[checkNdx].m_pt1.loc == loc2) {
#ifdef RTS_DEBUG
			DEBUG_ASSERTLOG(m_roads[checkNdx].m_pt1.count==1, ("Bad count"));
#endif
			moveRoadSegTo(checkNdx, ndx);
			loc2 = m_roads[ndx].m_pt2.loc;
			if (m_roads[ndx].m_pt2.count != 1) return;
			endOfCurSeg++;
		} else if (m_roads[checkNdx].m_pt2.loc == loc2) {
#ifdef RTS_DEBUG
			if (m_roads[checkNdx].m_pt2.count!=1) {
				::OutputDebugString("fooey.\n");
			}
			DEBUG_ASSERTLOG(m_roads[checkNdx].m_pt2.count==1, ("Bad count"));
#endif
			flipTheRoad(&m_roads[checkNdx]);
			moveRoadSegTo(checkNdx, ndx);
			loc2 = m_roads[ndx].m_pt2.loc;
			if (m_roads[ndx].m_pt2.count != 1) return;
			endOfCurSeg++;
		}	else {
			checkNdx++;
		}
		if (checkNdx <= endOfCurSeg) {
			checkNdx = endOfCurSeg+1;
		}
	}

}

//=============================================================================
// W3DRoadBuffer::checkLinkAfter
//=============================================================================
/** Checks to see if any segments need to link after this one. */
//=============================================================================
void W3DRoadBuffer::checkLinkAfter(Int ndx)
{
	// Check for things whose loc == our loc1.
	if (m_roads[ndx].m_pt1.count != 1) {
		return;
	}
	if (ndx >= m_numRoads-1) {
		return;
	}

	Vector2 loc1 = m_roads[ndx].m_pt1.loc;
#ifdef RTS_DEBUG
	DEBUG_ASSERTLOG(m_roads[ndx].m_pt2.loc == m_roads[ndx-1].m_pt1.loc, ("Bad link"));
#endif

	Int checkNdx = ndx+1;
	while (checkNdx < m_numRoads && ndx < m_numRoads-1) {
		if (m_roads[checkNdx].m_pt2.loc == loc1) {
#ifdef RTS_DEBUG
			DEBUG_ASSERTLOG(m_roads[checkNdx].m_pt2.count==1, ("Bad count"));
#endif
			ndx++;
			moveRoadSegTo(checkNdx, ndx);
			loc1 = m_roads[ndx].m_pt1.loc;
			if (m_roads[ndx].m_pt1.count != 1) return;
		} else if (m_roads[checkNdx].m_pt1.loc == loc1) {
#ifdef RTS_DEBUG
			DEBUG_ASSERTLOG(m_roads[checkNdx].m_pt1.count==1, ("Wrong m_pt1.count."));
			if ( m_roads[checkNdx].m_pt1.count!=1) {
				::OutputDebugString("Wrong m_pt1.count.\n");
			}
#endif
			flipTheRoad(&m_roads[checkNdx]);
			ndx++;
			moveRoadSegTo(checkNdx, ndx);
			loc1 = m_roads[ndx].m_pt1.loc;
			if (m_roads[ndx].m_pt1.count != 1) return;
		}	else {
			checkNdx++;
		}
	}

}

static Bool warnSegments = true;
#define CHECK_SEGMENTS {if (m_numRoads >= m_maxRoadSegments) { if (warnSegments) DEBUG_LOG(("****** Too many road segments.  Need to increase ini values.  See john a.")); warnSegments = false; return;}}

//=============================================================================
// W3DRoadBuffer::addMapObject
//=============================================================================
/** Loads the roads from the map objects. */
//=============================================================================
void W3DRoadBuffer::addMapObject(RoadSegment *pRoad, Bool updateTheCounts)
{

	RoadSegment cur = *pRoad;
	Vector2 loc1, loc2;
	loc1 = cur.m_pt1.loc;
	loc2 = cur.m_pt2.loc;
 	Vector2 roadVector(loc2.X-loc1.X, loc2.Y-loc1.Y);
	Vector2 roadNormal(-roadVector.Y, roadVector.X);
	roadNormal.Normalize();
	roadNormal *= (cur.m_scale*cur.m_widthInTexture/2.0f);
	cur.m_pt1.top = loc1+roadNormal;
	cur.m_pt1.bottom = loc1 - roadNormal;
	cur.m_pt2.top = loc2+roadNormal;
	cur.m_pt2.bottom = loc2 - roadNormal;

	if (updateTheCounts) {
		updateCounts(&cur);
	}

	CHECK_SEGMENTS;
	Int i;
	Bool flip = false;
	Bool addBefore = false;
	Bool addAfter = false;
	Bool bothMatch = false;
	for (i=0; i<m_numRoads; i++) {
		bothMatch = false;
		if ( (m_roads[i].m_pt1.loc==loc1 && m_roads[i].m_pt2.loc==loc2) ||
			(m_roads[i].m_pt1.loc==loc2 && m_roads[i].m_pt2.loc==loc1) ) {
			bothMatch = true;	// identical segment, so discard.
			break;
		}
		if (cur.m_pt1.count==1) {
			if ((m_roads[i].m_pt1.loc == loc1)) {
				flip = true;
				addAfter = true;
			}
			if ((m_roads[i].m_pt2.loc ==loc1)) {
				flip = false;
				addBefore = true;
			}
		}
		if (cur.m_pt2.count==1) {
			if ((m_roads[i].m_pt1.loc == loc2)) {
				flip = false;
				addAfter = true;
			}
			if ((m_roads[i].m_pt2.loc == loc2)) {
				flip = true;
				addBefore = true;
			}
		}
		if (addBefore || addAfter) {
			break;
		}
	}
	if (bothMatch) {
		return;
	}
	Int addIndex = i;
	if (addAfter) {
		addIndex++;
	}
	if (addIndex < m_numRoads) {
		for (i=m_numRoads; i>addIndex; i--) {
			m_roads[i] = m_roads[i-1];
		}
	}

	m_roads[addIndex] = cur;
	if (flip) {
		flipTheRoad(&m_roads[addIndex]);
	}
	m_numRoads++;
	if (addBefore) {
		checkLinkBefore(addIndex);
	} else if (addAfter) {
		checkLinkAfter(addIndex);
	}
}

//=============================================================================
// W3DRoadBuffer::addMapObjects
//=============================================================================
/** Loads the roads from the map objects. */
//=============================================================================
void W3DRoadBuffer::addMapObjects()
{
	MapObject *pMapObj;
	MapObject *pMapObj2;
	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
		if (m_numRoads >= m_maxRoadSegments) {
			break;
		}
		if (pMapObj->getFlag(FLAG_ROAD_POINT1)) {
			pMapObj2 = pMapObj->getNext();
#ifdef RTS_DEBUG
			DEBUG_ASSERTLOG(pMapObj2 && pMapObj2->getFlag(FLAG_ROAD_POINT2), ("Bad Flag"));
#endif
			if (pMapObj2==NULL) break;
			if (!pMapObj2->getFlag(FLAG_ROAD_POINT2)) continue;
			Vector2 loc1, loc2;
			loc1.Set(pMapObj->getLocation()->x, pMapObj->getLocation()->y);
			loc2.Set(pMapObj2->getLocation()->x, pMapObj2->getLocation()->y);
			if (loc1.X==loc2.X && loc1.Y==loc2.Y) {
				loc2.X += 0.25;
			}
			RoadSegment	curRoad;
			curRoad.m_scale = DEFAULT_ROAD_SCALE;
			curRoad.m_widthInTexture = 1.0f;
			curRoad.m_uniqueID = 1;
			Bool found = false;
			TerrainRoadType *road = TheTerrainRoads->findRoad( pMapObj->getName() );
			if( road )
			{
				curRoad.m_widthInTexture = road->getRoadWidthInTexture();
				curRoad.m_scale = road->getRoadWidth();
				curRoad.m_uniqueID = road->getID();
				found = TRUE;
			}
#ifdef LOAD_TEST_ASSETS
			const Real DEFAULT_SCALE = 30;
			if (!found) {
				Int i;
				for (i=0; i<m_maxRoadTypes; i++) {
					if (pMapObj->getName() == m_roadTypes[i].getPath()) {
						curRoad.m_scale = DEFAULT_SCALE;
						curRoad.m_uniqueID = m_roadTypes[i].getUniqueID();
						found = true;
					}
				}
			}
			if (!found && m_curOpenRoad<m_maxRoadTypes) {
				m_maxUID++;
				curRoad.m_scale = DEFAULT_SCALE;
				curRoad.m_uniqueID = m_maxUID;
				m_roadTypes[m_curOpenRoad].loadTexture(pMapObj->getName(), m_maxUID);
				m_curOpenRoad++;
			}
#endif
			curRoad.m_pt1.loc = loc1;
			curRoad.m_pt1.isAngled = pMapObj->getFlag(FLAG_ROAD_CORNER_ANGLED);
			curRoad.m_pt1.isJoin = pMapObj->getFlag(FLAG_ROAD_JOIN);
			curRoad.m_pt2.loc =loc2;
			curRoad.m_pt2.isAngled = pMapObj2->getFlag(FLAG_ROAD_CORNER_ANGLED);
			curRoad.m_pt2.isJoin = pMapObj2->getFlag(FLAG_ROAD_JOIN);
			curRoad.m_type = SEGMENT;
			curRoad.m_curveRadius = pMapObj->getFlag(FLAG_ROAD_CORNER_TIGHT)?TIGHT_CORNER_RADIUS:CORNER_RADIUS;

			addMapObject(&curRoad, true);
			pMapObj = pMapObj2;
		}
	}
	Int curCount = m_numRoads;
	Int i;
	m_numRoads = 0;
	for (i=0; i<curCount; i++) {
		RoadSegment curRoad = m_roads[i];
		addMapObject(&curRoad, false);
	}

}

//=============================================================================
// W3DRoadBuffer::updateCounts
//=============================================================================
/** Updates the count and last fields. */
//=============================================================================
void W3DRoadBuffer::updateCounts(RoadSegment *pRoad)
{
	pRoad->m_pt1.last = true;
	pRoad->m_pt2.last = true;
	pRoad->m_pt1.multi = false;
	pRoad->m_pt2.multi = false;
	pRoad->m_pt1.count = 0;
	pRoad->m_pt2.count = 0;
	Vector2 loc1, loc2;
	loc1 = pRoad->m_pt1.loc;
	loc2 = pRoad->m_pt2.loc;
	Int i;
	for (i=0; i<m_numRoads; i++) {
		// Only connect same type of road.
		if (m_roads[i].m_uniqueID != pRoad->m_uniqueID) {
			continue;
		}
		if ((m_roads[i].m_pt1.loc == loc1)) {
			m_roads[i].m_pt1.count++;
			pRoad->m_pt1.count++;
		}
		if ((m_roads[i].m_pt1.loc == loc2)) {
			m_roads[i].m_pt1.count++;
			pRoad->m_pt2.count++;
		}
		if ((m_roads[i].m_pt2.loc ==loc1)) {
			m_roads[i].m_pt2.count++;
			pRoad->m_pt1.count++;
		}
		if ((m_roads[i].m_pt2.loc == loc2)) {
			m_roads[i].m_pt2.count++;
			pRoad->m_pt2.count++;
		}
		m_roads[i].m_pt1.multi = m_roads[i].m_pt1.count > 1;
		m_roads[i].m_pt2.multi = m_roads[i].m_pt2.count > 1;
	}
	pRoad->m_pt1.multi = pRoad->m_pt1.count > 1;
	pRoad->m_pt2.multi = pRoad->m_pt2.count > 1;
}

//=============================================================================
// W3DRoadBuffer::updateCountsAndFlags
//=============================================================================
/** Updates the count and last fields. */
//=============================================================================
void W3DRoadBuffer::updateCountsAndFlags()
{
	Int i, j;
	for (i=0; i<m_numRoads; i++) {
		m_roads[i].m_pt1.last = true;
		m_roads[i].m_pt2.last = true;
		m_roads[i].m_pt1.count = 0;
		m_roads[i].m_pt2.count = 0;
	}
	for (j=m_numRoads-1; j>0; j--) {
		Vector2 loc1, loc2;
		loc1 = m_roads[j].m_pt1.loc;
		loc2 = m_roads[j].m_pt2.loc;
		for (i=0; i<j; i++) {
			if (m_roads[i].m_uniqueID != m_roads[j].m_uniqueID) {
				continue;
			}
			if ((m_roads[i].m_pt1.loc == loc1)) {
				m_roads[i].m_pt1.last = false;
				m_roads[i].m_pt1.count++;
				m_roads[j].m_pt1.count++;
			}
			if ((m_roads[i].m_pt1.loc == loc2)) {
				m_roads[i].m_pt1.last = false;
				m_roads[i].m_pt1.count++;
				m_roads[j].m_pt2.count++;
			}
			if ((m_roads[i].m_pt2.loc ==loc1)) {
				m_roads[i].m_pt2.last = false;
				m_roads[i].m_pt2.count++;
				m_roads[j].m_pt1.count++;
			}
			if ((m_roads[i].m_pt2.loc == loc2)) {
				m_roads[i].m_pt2.last = false;
				m_roads[i].m_pt2.count++;
				m_roads[j].m_pt2.count++;
			}
		}
	}
}


//=============================================================================
// W3DRoadBuffer::insertTee
//=============================================================================
/** Inserts a Tee intersection. */
//=============================================================================
void W3DRoadBuffer::insertTee(Vector2 loc, Int index1, Real scale)
{

	if (insertY(loc,index1, scale)) {
		return;
	}

	// pr1-3 point to the points on the segments that form the tee.
	// They are the points on the segments that are != loc.
	TRoadPt *pr1=NULL;
	TRoadPt *pr2=NULL;
	TRoadPt *pr3=NULL;

	// pc1-3 point to the center points of the segments.  These are the
	// points that are at loc.
	TRoadPt *pc1=NULL;
	TRoadPt *pc2=NULL;
	TRoadPt *pc3=NULL;

	if (m_roads[index1].m_pt1.loc == loc) {
		pr1 = &m_roads[index1].m_pt2;
		pc1 = &m_roads[index1].m_pt1;
	} else {
		pr1 = &m_roads[index1].m_pt1;
		pc1 = &m_roads[index1].m_pt2;
	}
	Int i;
	Int index2=0;
	Int index3=0;
	for (i = index1+1; i<m_numRoads; i++) {
		if (m_roads[i].m_pt1.loc == loc) {
			m_roads[i].m_pt1.count = -2;
			if (pr2==NULL) {
				pr2 = &m_roads[i].m_pt2;
				pc2 = &m_roads[i].m_pt1;
				index2 = i;
			} else {
				pr3 = &m_roads[i].m_pt2;
				pc3 = &m_roads[i].m_pt1;
				index3 = i;
			}
		}
		if (m_roads[i].m_pt2.loc == loc) {
			m_roads[i].m_pt2.count = -2;
			if (pr2==NULL) {
				pr2 = &m_roads[i].m_pt1;
				pc2 = &m_roads[i].m_pt2;
				index2 = i;
			} else {
				pr3 = &m_roads[i].m_pt1;
				pc3 = &m_roads[i].m_pt2;
				index3 = i;
			}
		}
	}
	if (pr2 == NULL || pr3 == NULL) {
		return;
	}

	Vector2 v1 = pr1->loc - loc;
	v1.Normalize();
	Vector2 v2 = pr2->loc - loc;
	v2.Normalize();
	Vector2 v3 = pr3->loc - loc;
	v3.Normalize();
	Real dot12 = v1.Dot_Product(v1, v2);
	Real dot13 = v1.Dot_Product(v1, v3);
	Real dot32 = v1.Dot_Product(v3, v2);
	// The greatest negative dot product is the pair that is heading most opposite each other.
	Bool do12 = false;
	Bool do13 = false;
	Bool do32 = false;

	if (dot12<dot13) {
		if (dot12<dot32) {
			do12 = true;
		} else {
			do32 = true;
		}
	} else {
		if (dot13<dot32) {
			do13 = true;
		} else {
			do32 = true;
		}
	}

	Vector2 upVector;
	Vector2 decider;
	if (do12) {
		upVector = v2-v1;
		decider = v3;
	}
	if (do13) {
		upVector = v3-v1;
		decider = v2;
	}
	if (do32) {
		upVector = v2-v3;
		decider = v1;
	}
	upVector.Normalize();


	// Check to see if the Tee side is slanted.
	const Real cos60 = 0.5f;
	Real dot = fabs(Vector2::Dot_Product(upVector, decider));
	if (dot > cos60) {
		// The arm of the tee is slanted, so do a slant tee.
		Real angle = (PI/2); // 90 degrees.
		Real xpdct = Vector3::Cross_Product_Z(Vector3(upVector.X,upVector.Y,0), Vector3(decider.X, decider.Y,0));
		Bool mirror = false;
		if (xpdct<0) {
			angle = -angle;
			mirror = true;
		}
		upVector.Normalize();
		upVector *= 0.5*scale; // we are offseting one half road width.
		Vector2 teeVector(upVector);
		teeVector.Rotate(angle);

		Bool flip;
		if (do12) {
			flip = xpSign(teeVector, v3) == 1;
			offsetH(pc1, pc2, pc3, loc, upVector, teeVector, flip, mirror, m_roads[index1].m_widthInTexture);
		}
		if (do13) {
			flip = xpSign(teeVector, v2) == 1;
			offsetH(pc1, pc3, pc2, loc, upVector, teeVector, flip, mirror, m_roads[index1].m_widthInTexture);
		}
		if (do32) {
			flip = xpSign(teeVector, v1) == 1;
			offsetH(pc3, pc2, pc1, loc, upVector, teeVector, flip, mirror, m_roads[index1].m_widthInTexture);

		}

		pc1->last = true;
		pc1->count = 0;
		pc2->last = true;
		pc2->count = 0;
		pc3->last = true;
		pc3->count = 0;

		CHECK_SEGMENTS;
		m_roads[m_numRoads].m_pt1.loc.Set(loc);
		m_roads[m_numRoads].m_pt2.loc.Set(loc+teeVector);
		m_roads[m_numRoads].m_pt1.last = true; // if not, that one will clear flag in prior loop.
		m_roads[m_numRoads].m_pt2.last = true; // if not, that one will clear flag in prior loop.
		m_roads[m_numRoads].m_scale = m_roads[index1].m_scale;
		m_roads[m_numRoads].m_widthInTexture = m_roads[index1].m_widthInTexture;
		m_roads[m_numRoads].m_pt1.count = -3;
		m_roads[m_numRoads].m_type = flip?THREE_WAY_H_FLIP:THREE_WAY_H;
		m_roads[m_numRoads].m_uniqueID = m_roads[index1].m_uniqueID;
		m_numRoads++;
	} else {
		// Do a not slanted tee.
		Real angle = (PI/2); // 90 degrees.
		Real xpdct = Vector3::Cross_Product_Z(Vector3(upVector.X,upVector.Y,0), Vector3(decider.X, decider.Y,0));
		if (xpdct<0) angle = -angle;
		upVector.Normalize();
		upVector *= 0.5*scale; // we are offseting one half road width.
		Vector2 teeVector(upVector);
		teeVector.Rotate(angle);

		if (do12) {
			offset3Way(pc1, pc2, pc3, loc, upVector, teeVector, m_roads[index1].m_widthInTexture);
		}
		if (do13) {
			offset3Way(pc1, pc3, pc2, loc, upVector, teeVector, m_roads[index1].m_widthInTexture);
		}
		if (do32) {
			offset3Way(pc3, pc2, pc1, loc, upVector, teeVector, m_roads[index1].m_widthInTexture);
		}
		pc1->last = true;
		pc1->count = 0;
		pc2->last = true;
		pc2->count = 0;
		pc3->last = true;
		pc3->count = 0;

		CHECK_SEGMENTS;
		m_roads[m_numRoads].m_pt1.loc.Set(loc);
		m_roads[m_numRoads].m_pt2.loc.Set(loc+teeVector);
		m_roads[m_numRoads].m_pt1.last = true; // if not, that one will clear flag in prior loop.
		m_roads[m_numRoads].m_pt2.last = true; // if not, that one will clear flag in prior loop.
		m_roads[m_numRoads].m_scale = m_roads[index1].m_scale;
		m_roads[m_numRoads].m_widthInTexture = m_roads[index1].m_widthInTexture;
		m_roads[m_numRoads].m_pt1.count = -3;
		m_roads[m_numRoads].m_type = TEE;
		m_roads[m_numRoads].m_uniqueID = m_roads[index1].m_uniqueID;
		m_numRoads++;
	}
}

//=============================================================================
// W3DRoadBuffer::insertY
//=============================================================================
/** Inserts a Y intersection if the corner meets "Y" criteria. */
//=============================================================================
Bool W3DRoadBuffer::insertY(Vector2 loc, Int index1, Real scale)
{
	// pr1-3 point to the points on the segments that form the tee.
	// They are the points on the segments that are != loc.
	TRoadPt *pr1=NULL;
	TRoadPt *pr2=NULL;
	TRoadPt *pr3=NULL;

	// pc1-3 point to the center points of the segments.  These are the
	// points that are at loc.
	TRoadPt *pc1=NULL;
	TRoadPt *pc2=NULL;
	TRoadPt *pc3=NULL;

	if (m_roads[index1].m_pt1.loc == loc) {
		pr1 = &m_roads[index1].m_pt2;
		pc1 = &m_roads[index1].m_pt1;
	} else {
		pr1 = &m_roads[index1].m_pt1;
		pc1 = &m_roads[index1].m_pt2;
	}
	Int i;
	Int index2=0;
	Int index3=0;
	for (i = index1+1; i<m_numRoads; i++) {
		if (m_roads[i].m_pt1.loc == loc) {
			m_roads[i].m_pt1.count = -2;
			if (pr2==NULL) {
				pr2 = &m_roads[i].m_pt2;
				pc2 = &m_roads[i].m_pt1;
				index2 = i;
			} else {
				pr3 = &m_roads[i].m_pt2;
				pc3 = &m_roads[i].m_pt1;
				index3 = i;
			}
		}
		if (m_roads[i].m_pt2.loc == loc) {
			m_roads[i].m_pt2.count = -2;
			if (pr2==NULL) {
				pr2 = &m_roads[i].m_pt1;
				pc2 = &m_roads[i].m_pt2;
				index2 = i;
			} else {
				pr3 = &m_roads[i].m_pt1;
				pc3 = &m_roads[i].m_pt2;
				index3 = i;
			}
		}
	}
	if (pr2 == NULL || pr3 == NULL) {
		return false;
	}

	Vector2 v1 = pr1->loc - loc;
	v1.Normalize();
	Vector2 v2 = pr2->loc - loc;
	v2.Normalize();
	Vector2 v3 = pr3->loc - loc;
	v3.Normalize();

	Bool do12 = false;
	Bool do13 = false;
	Bool do32 = false;

	Real dot12 = v1.Dot_Product(v1, v2);
	Real dot13 = v1.Dot_Product(v1, v3);
	Real dot32 = v1.Dot_Product(v3, v2);
	Real score12 = 2.0f;
	Real score13 = 2.0f;
	Real score32 = 2.0f;

//	const Real cos60 = 0.5f;
	const Real cos30 = 0.866f;
	const Real cos45 = 0.707f;

	if (dot12 < (-cos30)) return false; // Too close to a straigh line, do a straight side tee.
	if (dot13 < (-cos30)) return false; // Too close to a straigh line, do a straight side tee.
	if (dot32 < (-cos30)) return false; // Too close to a straigh line, to a straight side tee.



	Int s1 = 0;
	Int s2 = xpSign(v1, v2);
	Int s3 = xpSign(v1, v3);

	if (s2!=s3 && (s2+s3==0)) {
		Vector2 v1_90(-v1.Y, v1.X);
		if (xpSign(v1_90, v2) == 1 && xpSign(v1_90, v3) == 1) {
			// s2 & s3 could be Y legs.
			do32 = true;
			score32 = fabs(dot12+cos45) + fabs(dot13+cos45);
		}
	}

	s1 = xpSign(v3, v1);
	s2 = xpSign(v3, v2);
	if (s2!=s1 && (s2+s1==0)) {
		// s2 & s3 could be Y legs.
		Vector2 v3_90(-v3.Y, v3.X);
		if (xpSign(v3_90, v2) == 1 && xpSign(v3_90, v1) == 1) {
			do12 = true;
			score12 = fabs(dot13+cos45) + fabs(dot32+cos45);
		}
	}

	s1 = xpSign(v2, v1);
	s3 = xpSign(v2, v3);
	if (s3!=s1 && (s3+s1==0)) {
		// s2 & s3 could be Y legs.
		Vector2 v2_90(-v2.Y, v2.X);
		if (xpSign(v2_90, v3) == 1 && xpSign(v2_90, v1) == 1) {
			do13 = true;
			score13 = fabs(dot12+cos45) + fabs(dot32+cos45);
		}
	}

	if (score12<score13) {
		do13 = false;
		if (score12<score32) {
			do32 = false;
		} else {
			do12 = false;
		}
	} else {
		do12 = false;
		if (score13<score32) {
			do32 = false;
		} else {
			do13 = false;
		}
	}


	Vector2 upVector;
	if (do12) {
		upVector = v3;
	} else if (do13) {
		upVector = v2;
	} else if (do32) {
		upVector = v1;
	} else {
		return false;
	}

	Real angle = -(PI/2); // -90 degrees.
	upVector.Normalize();
	upVector *= 0.5*scale; // we are offseting one half road width.
	Vector2 teeVector(upVector);
	teeVector.Rotate(angle);

	// Offset the ends of the road to meet the stumps of the y.
	if (do12) {
		if (xpSign(v3, v1) == -1) {
			offsetY(pc1, pc2, pc3, loc, upVector, m_roads[index1].m_widthInTexture);
		} else {
			offsetY(pc2, pc1, pc3, loc, upVector, m_roads[index1].m_widthInTexture);
		}
	}
	if (do13) {
		if (xpSign(v2, v1) == -1) {
			offsetY(pc1, pc3, pc2, loc, upVector, m_roads[index1].m_widthInTexture);
		} else {
			offsetY(pc3, pc1, pc2, loc, upVector, m_roads[index1].m_widthInTexture);
		}
	}
	if (do32) {
		if (xpSign(v1, v3) == -1) {
			offsetY(pc3, pc2, pc1, loc, upVector, m_roads[index1].m_widthInTexture);
		} else {
			offsetY(pc2, pc3, pc1, loc, upVector, m_roads[index1].m_widthInTexture);
		}
	}

	pc1->last = true;
	pc1->count = 0;
	pc2->last = true;
	pc2->count = 0;
	pc3->last = true;
	pc3->count = 0;

	if (m_numRoads >= m_maxRoadSegments) return false;
	m_roads[m_numRoads].m_pt1.loc.Set(loc);
	m_roads[m_numRoads].m_pt2.loc.Set(loc+teeVector);
	m_roads[m_numRoads].m_pt1.last = true; // if not, that one will clear flag in prior loop.
	m_roads[m_numRoads].m_pt2.last = true; // if not, that one will clear flag in prior loop.
	m_roads[m_numRoads].m_scale = m_roads[index1].m_scale;
	m_roads[m_numRoads].m_widthInTexture = m_roads[index1].m_widthInTexture;
	m_roads[m_numRoads].m_pt1.count = -3;
	m_roads[m_numRoads].m_type = THREE_WAY_Y;
	m_roads[m_numRoads].m_uniqueID = m_roads[index1].m_uniqueID;
	m_numRoads++;


	return true;


}

//=============================================================================
// W3DRoadBuffer::offset3Way
//=============================================================================
/** Offsets the points coming into a 3 way intersection so that they move to
the join points of the 3 way intersection. */
//=============================================================================
void W3DRoadBuffer::offset3Way(TRoadPt *pc1, TRoadPt *pc2, TRoadPt *pc3, Vector2 loc, Vector2 upVector,
															 Vector2 teeVector, Real widthInTexture)
{
	pc1->loc = loc - upVector;
	pc2->loc = loc + upVector;
	pc3->loc = loc + teeVector;

	// Adjust the top & bottoms, so they merge smoothly into the tee.

	// Make sure the t vector goes right with respect to the up vector.
	Real xpdct = Vector3::Cross_Product_Z(Vector3(upVector.X,upVector.Y,0), Vector3(teeVector.X, teeVector.Y,0));
	Vector2 rightTee = teeVector;
	if (xpdct<0) {
		rightTee.X = -teeVector.X;
		rightTee.Y = -teeVector.Y;
	}
	rightTee *= (widthInTexture);

	xpdct = Vector3::Cross_Product_Z(Vector3(upVector.X, upVector.Y, 0), Vector3(pc1->top.X-pc1->loc.X, pc1->top.Y-pc1->loc.Y,0));
	if (xpdct>0) {
		pc1->bottom = pc1->loc - rightTee;
		pc1->top = pc1->loc + rightTee;
	} else {
		pc1->bottom = pc1->loc + rightTee;
		pc1->top = pc1->loc - rightTee;
	}
	xpdct = Vector3::Cross_Product_Z(Vector3(upVector.X, upVector.Y, 0), Vector3(pc2->top.X-pc2->loc.X, pc2->top.Y-pc2->loc.Y,0));
	if (xpdct>0) {
		pc2->bottom = pc2->loc - rightTee;
		pc2->top = pc2->loc + rightTee;
	} else {
		pc2->bottom = pc2->loc + rightTee;
		pc2->top = pc2->loc - rightTee;
	}
	upVector *= (widthInTexture);
	xpdct = Vector3::Cross_Product_Z(Vector3(rightTee.X, rightTee.Y, 0), Vector3(pc3->top.X-pc3->loc.X, pc3->top.Y-pc3->loc.Y,0));
	if (xpdct<0) {
		pc3->bottom = pc3->loc - upVector;
		pc3->top = pc3->loc + upVector;
	} else {
		pc3->bottom = pc3->loc + upVector;
		pc3->top = pc3->loc - upVector;
	}
}

//=============================================================================
// W3DRoadBuffer::offsetH
//=============================================================================
/** Offsets the points coming into a 3 way intersection so that they move to
the join points of the 3 way intersection. */
//=============================================================================
void W3DRoadBuffer::offsetH(TRoadPt *pc1, TRoadPt *pc2, TRoadPt *pc3, Vector2 loc,
														Vector2 upVector, Vector2 teeVector,
														Bool flip, Bool mirror, Real widthInTexture)
{

	if (flip != mirror) {
		pc1->loc = loc - upVector*2.05f;
		pc2->loc = loc + upVector*.46f;
	} else {
		pc1->loc = loc - upVector*.46f;
		pc2->loc = loc + upVector*2.05f;
	}
	//pc3->loc = loc + teeVector;

	// Adjust the top & bottoms, so they merge smoothly into the tee.

	// Make sure the t vector goes right with respect to the up vector.
	Real xpdct = Vector3::Cross_Product_Z(Vector3(upVector.X,upVector.Y,0), Vector3(teeVector.X, teeVector.Y,0));
	Vector2 rightTee = teeVector;
	if (xpdct<0) {
		rightTee.X = -teeVector.X;
		rightTee.Y = -teeVector.Y;
	}
	rightTee *= (widthInTexture);

	xpdct = Vector3::Cross_Product_Z(Vector3(upVector.X, upVector.Y, 0), Vector3(pc1->top.X-pc1->loc.X, pc1->top.Y-pc1->loc.Y,0));
	if (xpdct>0) {
		pc1->bottom = pc1->loc - rightTee;
		pc1->top = pc1->loc + rightTee;
	} else {
		pc1->bottom = pc1->loc + rightTee;
		pc1->top = pc1->loc - rightTee;
	}
	xpdct = Vector3::Cross_Product_Z(Vector3(upVector.X, upVector.Y, 0), Vector3(pc2->top.X-pc2->loc.X, pc2->top.Y-pc2->loc.Y,0));
	if (xpdct>0) {
		pc2->bottom = pc2->loc - rightTee;
		pc2->top = pc2->loc + rightTee;
	} else {
		pc2->bottom = pc2->loc + rightTee;
		pc2->top = pc2->loc - rightTee;
	}


	Vector2 arm = teeVector;
	if (flip) {
		arm.Rotate(PI/4);
	} else {
		arm.Rotate(-PI/4);
	}
	Vector2 armNormal(-arm.Y, arm.X);
	armNormal *= widthInTexture;

	pc3->loc += arm*2.10f;
	xpdct = xpSign(arm, pc3->top-loc);
	if (xpdct==1) {
		pc3->top = pc3->loc + armNormal;
		pc3->bottom = pc3->loc - armNormal;
	} else {
		pc3->top = pc3->loc - armNormal;
		pc3->bottom = pc3->loc + armNormal;
	}

}



//=============================================================================
// W3DRoadBuffer::offsetY
//=============================================================================
/** Offsets the points coming into a 3 way intersection so that they move to
the join points of the 3 way intersection. */
//=============================================================================
void W3DRoadBuffer::offsetY(TRoadPt *pc1, TRoadPt *pc2, TRoadPt *pc3, Vector2 loc, Vector2 upVector,
															 Real widthInTexture)
{
	pc3->loc += upVector*0.55f;
	pc3->top += upVector*0.55f;
	pc3->bottom += upVector*0.55f;

	// Adjust the bottom, so they merge smoothly into the tee.

	// Make sure the t vector goes right with respect to the up vector.
	Vector2 arm = upVector;
	arm.Rotate(3*PI/4);
	Vector2 armNormal(-arm.Y, arm.X);
	armNormal *= widthInTexture;
	pc2->loc += arm*1.1f;
	Int xpdct = xpSign(arm, pc2->top-loc);
	if (xpdct==1) {
		pc2->top = pc2->loc + armNormal;
		pc2->bottom = pc2->loc - armNormal;
	} else {
		pc2->top = pc2->loc - armNormal;
		pc2->bottom = pc2->loc + armNormal;
	}

	arm = upVector;
	arm.Rotate(-3*PI/4);
	armNormal.Set(-arm.Y, arm.X);
	armNormal *= widthInTexture;

	pc1->loc += arm*1.1f;
	xpdct = xpSign(arm, pc1->top-loc);
	if (xpdct==1) {
		pc1->top = pc1->loc + armNormal;
		pc1->bottom = pc1->loc - armNormal;
	} else {
		pc1->top = pc1->loc - armNormal;
		pc1->bottom = pc1->loc + armNormal;
	}
}

//=============================================================================
// W3DRoadBuffer::offset4Way
//=============================================================================
/** Offsets the points coming into a 4 way intersection so that they move to
the join points of the 4 way intersection. */
//=============================================================================
void W3DRoadBuffer::offset4Way(TRoadPt *pc1, TRoadPt *pc2, TRoadPt *pc3, TRoadPt *pr3,
															 TRoadPt *pc4, Vector2 loc, Vector2 alignVector, Real widthInTexture)
{
	pc1->loc = loc - alignVector;
	pc2->loc = loc + alignVector;

	Vector2 v3 = pr3->loc - loc;
	Real angle = (PI/2); // 90 degrees.
	Real xpdct = Vector3::Cross_Product_Z(Vector3(alignVector.X,alignVector.Y,0), Vector3(v3.X, v3.Y,0));
	if (xpdct<0) angle = -angle;
	Vector2 teeVector(alignVector);
	teeVector.Rotate(angle);
	pc3->loc = loc + teeVector;
	pc4->loc = loc - teeVector;

	Vector2 realTee(alignVector);
	realTee.Rotate(PI/2);
	realTee *= widthInTexture;

	// Fix up the top & bottom points.
	Vector3 align3(alignVector.X,alignVector.Y,0);

	xpdct = Vector3::Cross_Product_Z(align3, Vector3(pc1->top.X-pc1->loc.X, pc1->top.Y-pc1->loc.Y,0));
	if (xpdct>0) {
		pc1->bottom = pc1->loc - realTee;
		pc1->top = pc1->loc + realTee;
	} else {
		pc1->bottom = pc1->loc + realTee;
		pc1->top = pc1->loc - realTee;
	}
	xpdct = Vector3::Cross_Product_Z(align3, Vector3(pc2->top.X-pc2->loc.X, pc2->top.Y-pc2->loc.Y,0));
	if (xpdct>0) {
		pc2->bottom = pc2->loc - realTee;
		pc2->top = pc2->loc + realTee;
	} else {
		pc2->bottom = pc2->loc + realTee;
		pc2->top = pc2->loc - realTee;
	}
	alignVector *= widthInTexture;

	Vector3 tee3(realTee.X,realTee.Y,0);
	xpdct = Vector3::Cross_Product_Z(tee3, Vector3(pc3->top.X-pc3->loc.X, pc3->top.Y-pc3->loc.Y,0));
	if (xpdct<0) {
		pc3->bottom = pc3->loc - alignVector;
		pc3->top = pc3->loc + alignVector;
	} else {
		pc3->bottom = pc3->loc + alignVector;
		pc3->top = pc3->loc - alignVector;
	}
	xpdct = Vector3::Cross_Product_Z(tee3, Vector3(pc4->top.X-pc4->loc.X, pc4->top.Y-pc4->loc.Y,0));
	if (xpdct<0) {
		pc4->bottom = pc4->loc - alignVector;
		pc4->top = pc4->loc + alignVector;
	} else {
		pc4->bottom = pc4->loc + alignVector;
		pc4->top = pc4->loc - alignVector;
	}

	pc1->last = true;
	pc1->count = 0;
	pc2->last = true;
	pc2->count = 0;
	pc3->last = true;
	pc3->count = 0;
	pc4->last = true;
	pc4->count = 0;

}

//=============================================================================
// W3DRoadBuffer::insert4Way
//=============================================================================
/** Inserts a 4 way intersection. */
//=============================================================================
void W3DRoadBuffer::insert4Way(Vector2 loc, Int index1, Real scale)
{
	// pr1-4 point to the points on the segments that form the tee.
	// They are the points on the segments that are != loc.
	TRoadPt *pr1=NULL;
	TRoadPt *pr2=NULL;
	TRoadPt *pr3=NULL;
	TRoadPt *pr4=NULL;

	// pc1-4 point to the center points of the segments.  These are the
	// points that are at loc.
	TRoadPt *pc1=NULL;
	TRoadPt *pc2=NULL;
	TRoadPt *pc3=NULL;
	TRoadPt *pc4=NULL;

	if (m_roads[index1].m_pt1.loc == loc) {
		pr1 = &m_roads[index1].m_pt2;
		pc1 = &m_roads[index1].m_pt1;
	} else {
		pr1 = &m_roads[index1].m_pt1;
		pc1 = &m_roads[index1].m_pt2;
	}
	Int i;
	for (i = index1+1; i<m_numRoads; i++) {
		if (m_roads[i].m_pt1.loc == loc) {
			m_roads[i].m_pt1.count = -2;
			if (pr2==NULL) {
				pr2 = &m_roads[i].m_pt2;
				pc2 = &m_roads[i].m_pt1;
			} else if (pr3==NULL) {
				pr3 = &m_roads[i].m_pt2;
				pc3 = &m_roads[i].m_pt1;
			}	else {
				pr4 = &m_roads[i].m_pt2;
				pc4 = &m_roads[i].m_pt1;
			}
		}
		if (m_roads[i].m_pt2.loc == loc) {
			m_roads[i].m_pt2.count = -2;
			if (pr2==NULL) {
				pr2 = &m_roads[i].m_pt1;
				pc2 = &m_roads[i].m_pt2;
			} else if (pr3==NULL) {
				pr3 = &m_roads[i].m_pt1;
				pc3 = &m_roads[i].m_pt2;
			}	else {
				pr4 = &m_roads[i].m_pt1;
				pc4 = &m_roads[i].m_pt2;
			}
		}
	}
	if (pr2 == NULL || pr3 == NULL || pr4==NULL) {
		return;
	}

	Vector2 v1 = pr1->loc - loc;
	v1.Normalize();
	Vector2 v2 = pr2->loc - loc;
	v2.Normalize();
	Vector2 v3 = pr3->loc - loc;
	v3.Normalize();
	Vector2 v4 = pr4->loc - loc;
	v4.Normalize();

	Real dot12 = v1.Dot_Product(v1, v2);
	Real dot13 = v1.Dot_Product(v1, v3);
	Real dot14 = v1.Dot_Product(v1, v4);
	Real dot23 = v1.Dot_Product(v2, v3);
	Real dot24 = v1.Dot_Product(v2, v4);
	Real dot34 = v1.Dot_Product(v3, v4);
	// The most negative dot product is the pair that is heading most opposite each other.

	Int curPair = 12;
	Real curDot = dot12;
	if (dot13<curDot) {
		curPair = 13;
		curDot = dot13;
	}
	if (dot14<curDot) {
		curPair = 14;
		curDot = dot14;
	}
	if (dot23<curDot) {
		curPair = 23;
		curDot = dot23;
	}
	if (dot24<curDot) {
		curPair = 24;
		curDot = dot24;
	}
	if (dot34<curDot) {
		curPair = 34;
		curDot = dot34;
	}
	Bool do12 = (curPair==12);	 // Do segment 1 to 2
	Bool do13 = (curPair==13);	 // Do segment 1 to 3 etc.
	Bool do14 = (curPair==14);
	Bool do23 = (curPair==23);
	Bool do24 = (curPair==24);
	Bool do34 = (curPair==34);

	Vector2 alignVector;
	if (do12) {
		alignVector = v2-v1;
	}
	if (do13) {
		alignVector = v3-v1;
	}
	if (do14) {
		alignVector = v4-v1;
	}
	if (do23) {
		alignVector = v3-v2;
	}
	if (do24) {
		alignVector = v4-v2;
	}
	if (do34) {
		alignVector = v4-v3;
	}
	alignVector.Normalize();
	alignVector *= 0.5*scale; // we are offseting one half road width.


	if (do12) {
		offset4Way(pc1, pc2, pc3, pr3, pc4, loc, alignVector, m_roads[index1].m_widthInTexture);
	}
	if (do13) {
		offset4Way(pc1, pc3, pc2, pr2, pc4, loc, alignVector, m_roads[index1].m_widthInTexture);
	}
	if (do14) {
		offset4Way(pc1, pc4, pc3, pr3, pc2, loc, alignVector, m_roads[index1].m_widthInTexture);
	}
	if (do23) {
		offset4Way(pc2, pc3, pc1, pr1, pc4, loc, alignVector, m_roads[index1].m_widthInTexture);
	}
	if (do24) {
		offset4Way(pc2, pc4, pc1, pr1, pc3, loc, alignVector, m_roads[index1].m_widthInTexture);
	}
	if (do34) {
		offset4Way(pc3, pc4, pc1, pr1, pc2, loc, alignVector, m_roads[index1].m_widthInTexture);
	}
	if (alignVector.X<0) {
		// Since the 4 way intersection is symmetrical, make it go right (pos x)
		alignVector.X = -alignVector.X;
		alignVector.Y = -alignVector.Y;
	}
	CHECK_SEGMENTS;
	m_roads[m_numRoads].m_pt1.loc.Set(loc);
	m_roads[m_numRoads].m_pt2.loc.Set(loc+alignVector);
	m_roads[m_numRoads].m_pt1.last = true; // if not, that one will clear flag in prior loop.
	m_roads[m_numRoads].m_pt2.last = true; // if not, that one will clear flag in prior loop.
	m_roads[m_numRoads].m_scale = m_roads[index1].m_scale;
	m_roads[m_numRoads].m_widthInTexture = TEE_WIDTH_ADJUSTMENT; // It is symmetrical.
	m_roads[m_numRoads].m_pt1.count = -4;
	m_roads[m_numRoads].m_type = FOUR_WAY;
	m_roads[m_numRoads].m_uniqueID = m_roads[index1].m_uniqueID;
	m_numRoads++;



}


//=============================================================================
// W3DRoadBuffer::insertTeeIntersections
//=============================================================================
/** Inserts Tee intersections at 3 way intersections. */
//=============================================================================
void W3DRoadBuffer::insertTeeIntersections(void)
{
	// Insert the tees.
	Int numRoadSegments = m_numRoads;
	Int i;
//	Int segmentStartIndex = -1;
	for (i=0; i<numRoadSegments; i++) {
		if (m_roads[i].m_type != SEGMENT) {
			continue;
		}
		if (m_roads[i].m_pt1.count==2) {
			insertTee(m_roads[i].m_pt1.loc, i, m_roads[i].m_scale);
		}
		if (m_roads[i].m_pt2.count==2) {
			insertTee(m_roads[i].m_pt2.loc, i, m_roads[i].m_scale);
		}
		if (m_roads[i].m_pt1.count==3) {
			insert4Way(m_roads[i].m_pt1.loc, i, m_roads[i].m_scale);
		}
		if (m_roads[i].m_pt2.count==3) {
			insert4Way(m_roads[i].m_pt2.loc, i, m_roads[i].m_scale);
		}
	}
}

//=============================================================================
// W3DRoadBuffer::insertCurveSegments
//=============================================================================
/** Inserts curved segments along connected segments. */
//=============================================================================
void W3DRoadBuffer::insertCurveSegments(void)
{
	// Insert the curve segments.
	Int numRoadSegments = m_numRoads;
	Int i;
	Int segmentStartIndex = -1;
	for (i=0; i<numRoadSegments; i++) {
		if (i<numRoadSegments-1 && m_roads[i].m_pt1.loc == m_roads[i+1].m_pt2.loc) {
			if (m_roads[i+1].m_pt2.count==1 && m_roads[i].m_pt1.count==1) {
				insertCurveSegmentAt(i, i+1);
				if (segmentStartIndex < 0) {
					segmentStartIndex = i;
				}
			}
		}	else if (segmentStartIndex>=0) {
			if (m_roads[i].m_pt1.loc == m_roads[segmentStartIndex].m_pt2.loc) {
				if (m_roads[segmentStartIndex].m_pt2.count==1 && m_roads[i].m_pt1.count==1) {
					insertCurveSegmentAt(i, segmentStartIndex);
				}
			}
			segmentStartIndex = -1;
		}
	}
}
//=============================================================================
// W3DRoadBuffer::findCrossTypeJoinVector
//=============================================================================
/** Finds a road segment of different type && sets the join vector. */
//=============================================================================
Int W3DRoadBuffer::findCrossTypeJoinVector(Vector2 loc, Vector2 *joinVector, Int uniqueID)
{
	Vector2 newVector = *joinVector;
	// Insert the curve segments.
	Int numRoadSegments = m_numRoads;
	Int i;
	for (i=0; i<numRoadSegments; i++) {
		// we only want different road types.
		if (m_roads[i].m_uniqueID == uniqueID) continue;
		// Only join to straight line segments.
		if (m_roads[i].m_type != SEGMENT) continue;
		Vector2 loc1, loc2;
		loc1 = m_roads[i].m_pt1.loc;
		loc2 = m_roads[i].m_pt2.loc;
		Region2D bounds;
		bounds.lo.x = loc1.X;
		bounds.lo.y = loc1.Y;
		bounds.hi = bounds.lo;
		if (bounds.lo.x > loc2.X) bounds.lo.x = loc2.X;
		if (bounds.lo.y > loc2.Y) bounds.lo.y = loc2.Y;
		if (bounds.hi.x < loc2.X) bounds.hi.x = loc2.X;
		if (bounds.hi.y < loc2.Y) bounds.hi.y = loc2.Y;
		bounds.lo.x -= m_roads[i].m_scale/2;
		bounds.lo.y -= m_roads[i].m_scale/2;
		bounds.hi.x += m_roads[i].m_scale/2;
		bounds.hi.y += m_roads[i].m_scale/2;
		if (loc.X >= bounds.lo.x && loc.Y >= bounds.lo.y && loc.X <= bounds.hi.x && loc.Y <= bounds.hi.y) {
			Vector3 v1 = Vector3(loc1.X, loc1.Y, 0);
			Vector3 v2 = Vector3(loc2.X, loc2.Y, 0);
			LineSegClass roadLine(v1,v2);
			Vector3 vLoc(loc.X, loc.Y, 0);
			Vector3 ptOnLine = roadLine.Find_Point_Closest_To(vLoc);
			Real dist = Vector3::Distance(ptOnLine, vLoc);
			if (dist < m_roads[i].m_scale*0.55f) {
				Vector2 roadVec = loc2-loc1;
				if (xpSign(roadVec, *joinVector) == 1) {
					roadVec.Rotate(PI/2);
				} else {
					roadVec.Rotate(-PI/2);
				}
				newVector = roadVec;
				*joinVector = newVector;
				return m_roads[i].m_uniqueID;
			}
		}

	}
	return 0;
}


//=============================================================================
// W3DRoadBuffer::adjustStacking
//=============================================================================
/** Adjusts the stacking order. */
//=============================================================================
void W3DRoadBuffer::adjustStacking(Int topUniqueID, Int bottomUniqueID)
{
	Int i, j;
	for (i=0; i<m_maxRoadTypes; i++) {
		if (m_roadTypes[i].getUniqueID() == topUniqueID) break;
	}
	DEBUG_ASSERTLOG(i<m_maxRoadTypes, ("***** Wrong unique id- john a should fix."));
	if (i>=m_maxRoadTypes) return;

	for (j=0; j<m_maxRoadTypes; j++) {
		if (m_roadTypes[j].getUniqueID() == bottomUniqueID) break;
	}
	DEBUG_ASSERTLOG(j<m_maxRoadTypes, ("***** Wrong unique id- john a should fix."));
	if (j>=m_maxRoadTypes) return;

	if (m_roadTypes[i].getStacking() > m_roadTypes[j].getStacking()) {
		return; // It's already on top.
	}
	Int newStacking = m_roadTypes[j].getStacking();
	for (j=0; j<m_maxRoadTypes; j++) {
		if (m_roadTypes[j].getStacking()>newStacking) {
			m_roadTypes[j].setStacking(m_roadTypes[j].getStacking()+1);
		}
	}
	m_roadTypes[i].setStacking(newStacking+1);

}


//=============================================================================
// W3DRoadBuffer::insertCrossTypeJoins
//=============================================================================
/** Inserts alpha blend type joins at open ends. */
//=============================================================================
void W3DRoadBuffer::insertCrossTypeJoins(void)
{
	// Insert the curve segments.
	Int numRoadSegments = m_numRoads;
	Int i;
	for (i=0; i<numRoadSegments; i++) {
		Vector2 loc1, loc2;
		Bool isPt1 = false;
		if ((m_roads[i].m_pt2.count==0 && m_roads[i].m_pt2.isJoin)) {
			loc1 = m_roads[i].m_pt2.loc;
			loc2 = m_roads[i].m_pt1.loc;

		} else if ((m_roads[i].m_pt1.count==0 && m_roads[i].m_pt1.isJoin)) {
			loc1 = m_roads[i].m_pt1.loc;
			loc2 = m_roads[i].m_pt2.loc;
			isPt1 = true;
		}	else {
			continue;
		}
		Vector2 joinVector(1, 0);


		Vector2 roadVector(loc2.X-loc1.X, loc2.Y-loc1.Y);
		roadVector.Normalize();
		joinVector = roadVector;
		Int otherID = findCrossTypeJoinVector(loc1, &joinVector, m_roads[i].m_uniqueID);
		if (!otherID ) {
			joinVector*=100;
		}
		Vector2 roadNormal(-roadVector.Y, roadVector.X);
		Vector2 joinNormal(-joinVector.Y, joinVector.X);


		Vector2 p1 = loc1 + roadNormal * m_roads[i].m_scale * m_roads[i].m_widthInTexture / 2;
		Vector2 p2 = loc2 + roadNormal * m_roads[i].m_scale * m_roads[i].m_widthInTexture / 2;

		Vector3 v1 = Vector3(p1.X, p1.Y, 0);
		Vector3 v2 = Vector3(p2.X, p2.Y, 0);
		LineSegClass roadLine(v1,v2);
		Vector3 vLoc1(loc1.X, loc1.Y, 0);
		v1 = Vector3(joinNormal.X, joinNormal.Y, 0)+vLoc1;
		LineSegClass joinLine(vLoc1,v1);
		Vector3 pInt1, pInt2;
		//Vector3 pInt3, pInt4;



		Real nu; // not used.
		Vector2 top = m_roads[i].m_pt1.top;
		if (joinLine.Find_Intersection(roadLine, &pInt1, &nu, &pInt2, &nu) ) {
			if (isPt1) {
				m_roads[i].m_pt1.top.Set(pInt1.X, pInt1.Y);
				top = m_roads[i].m_pt1.top;
			}	else {
				m_roads[i].m_pt2.bottom.Set(pInt1.X, pInt1.Y);
				top = m_roads[i].m_pt2.bottom;
			}
		}
		p1 = loc1 - roadNormal * m_roads[i].m_scale * m_roads[i].m_widthInTexture / 2;
		p2 = loc2 - roadNormal * m_roads[i].m_scale * m_roads[i].m_widthInTexture / 2;
		v1.Set(p1.X, p1.Y, 0);
		v2.Set(p2.X, p2.Y, 0);
		roadLine.Set(v1,v2);
		Vector2 bottom = m_roads[i].m_pt1.bottom;
		if (joinLine.Find_Intersection(roadLine, &pInt1, &nu, &pInt2, &nu) ) {
			if (isPt1) {
				m_roads[i].m_pt1.bottom.Set(pInt1.X, pInt1.Y);
				bottom = m_roads[i].m_pt1.bottom;
			}	else {
				m_roads[i].m_pt2.top.Set(pInt1.X, pInt1.Y);
				bottom = m_roads[i].m_pt2.top;
			}
		}

		bottom = bottom-top;
		Real scaleAdjustment = bottom.Length() / (m_roads[i].m_scale * m_roads[i].m_widthInTexture);
		if (otherID) {
			adjustStacking(m_roads[i].m_uniqueID, otherID);
		}
		CHECK_SEGMENTS;
		m_roads[m_numRoads].m_pt1.loc.Set(loc1);
		m_roads[m_numRoads].m_pt2.loc.Set(loc1+joinVector);
		m_roads[m_numRoads].m_pt1.last = true; // if not, that one will clear flag in prior loop.
		m_roads[m_numRoads].m_pt2.last = true; // if not, that one will clear flag in prior loop.
		m_roads[m_numRoads].m_scale = m_roads[i].m_scale;
		m_roads[m_numRoads].m_widthInTexture = m_roads[i].m_scale * scaleAdjustment;
		m_roads[m_numRoads].m_pt1.count = 0;
		m_roads[m_numRoads].m_type = ALPHA_JOIN;
		m_roads[m_numRoads].m_uniqueID = m_roads[i].m_uniqueID;
		m_numRoads++;
	}
}


//=============================================================================
// W3DRoadBuffer::miter
//=============================================================================
/** Adjusts the end points to create a smooth miter join between road segments. */
//=============================================================================
void W3DRoadBuffer::miter(Int ndx1, Int ndx2)
{
	// adjust a mitered join.  jba.
	Vector3 p1 = Vector3(m_roads[ndx1].m_pt1.top.X, m_roads[ndx1].m_pt1.top.Y, 0);
	Vector3 p2 = Vector3(m_roads[ndx1].m_pt2.top.X, m_roads[ndx1].m_pt2.top.Y, 0);
	LineSegClass offsetLine1(p1, p2);
	p1 = Vector3(m_roads[ndx2].m_pt1.top.X, m_roads[ndx2].m_pt1.top.Y, 0);
	p2 = Vector3(m_roads[ndx2].m_pt2.top.X, m_roads[ndx2].m_pt2.top.Y, 0);
	LineSegClass offsetLine2(p1,p2);
	Vector3 pInt1, pInt2;
	Real nu; // not used.
	if (offsetLine1.Find_Intersection(offsetLine2, &pInt1, &nu, &pInt2, &nu) ) {
		m_roads[ndx2].m_pt2.top.X = pInt1.X;
		m_roads[ndx2].m_pt2.top.Y = pInt1.Y;
		m_roads[ndx1].m_pt1.top.X = pInt1.X;
		m_roads[ndx1].m_pt1.top.Y = pInt1.Y;
	}
	p1 = Vector3(m_roads[ndx1].m_pt1.bottom.X, m_roads[ndx1].m_pt1.bottom.Y, 0);
	p2 = Vector3(m_roads[ndx1].m_pt2.bottom.X, m_roads[ndx1].m_pt2.bottom.Y, 0);
	offsetLine1=LineSegClass (p1,p2);
	p1 = Vector3(m_roads[ndx2].m_pt1.bottom.X, m_roads[ndx2].m_pt1.bottom.Y, 0);
	p2 = Vector3(m_roads[ndx2].m_pt2.bottom.X, m_roads[ndx2].m_pt2.bottom.Y, 0);
	offsetLine2 = LineSegClass(p1,p2);
	if (offsetLine1.Find_Intersection(offsetLine2, &pInt1, &nu, &pInt2, &nu) ) {
		m_roads[ndx2].m_pt2.bottom.X = pInt1.X;
		m_roads[ndx2].m_pt2.bottom.Y = pInt1.Y;
		m_roads[ndx1].m_pt1.bottom.X = pInt1.X;
		m_roads[ndx1].m_pt1.bottom.Y = pInt1.Y;
	}
}

//=============================================================================
// W3DRoadBuffer::insertCurveSegmentAt
//=============================================================================
/** Insertes curves at the corner of 2 segments. */
//=============================================================================
void W3DRoadBuffer::insertCurveSegmentAt(Int ndx1, Int ndx2)
{
	const Real DOT_LIMIT = 0.5f;	// If the dot product of the new line is less than this, abort.
	Real radius = m_roads[ndx1].m_curveRadius*m_roads[ndx1].m_scale;
	Vector2 originalPt = m_roads[ndx1].m_pt1.loc;
	// we got a segment.
	LineSegClass line1(Vector3(m_roads[ndx1].m_pt1.loc.X, m_roads[ndx1].m_pt1.loc.Y, 0), Vector3(m_roads[ndx1].m_pt2.loc.X, m_roads[ndx1].m_pt2.loc.Y, 0));
	LineSegClass line2(Vector3(m_roads[ndx2].m_pt1.loc.X, m_roads[ndx2].m_pt1.loc.Y, 0), Vector3(m_roads[ndx2].m_pt2.loc.X, m_roads[ndx2].m_pt2.loc.Y, 0));
	Vector2 *pr1, *pr2, *pr3, *pr4;

	if (m_roads[ndx1].m_uniqueID != m_roads[ndx2].m_uniqueID) {
		return;
	}
	Real curSin = Vector3::Dot_Product(line1.Get_Dir(), line2.Get_Dir());
	Real xpdct = Vector3::Cross_Product_Z(line1.Get_Dir(), line2.Get_Dir());
	Bool turnRight;
	if (xpdct > 0) {
		pr1 = &m_roads[ndx1].m_pt1.loc;
		pr2 = &m_roads[ndx1].m_pt2.loc;
		pr3 = &m_roads[ndx2].m_pt1.loc;
		pr4 = &m_roads[ndx2].m_pt2.loc;
		turnRight = true;
	} else {
		pr4 = &m_roads[ndx1].m_pt1.loc;
		pr3 = &m_roads[ndx1].m_pt2.loc;
		pr2 = &m_roads[ndx2].m_pt1.loc;
		pr1 = &m_roads[ndx2].m_pt2.loc;
		turnRight = false;
 		line1.Set(Vector3(pr1->X, pr1->Y, 0), Vector3(pr2->X, pr2->Y, 0));
 		line2.Set(Vector3(pr3->X, pr3->Y, 0), Vector3(pr4->X, pr4->Y, 0));
	}
	Real angle = WWMath::Acos(curSin);
	Real count = angle / (PI/6.0f); // number of 30 degree steps.
	if (count<0.9 || m_roads[ndx1].m_pt1.isAngled) {
		miter(ndx1, ndx2);
		return;

	}

	Vector3 offset1(radius*line1.Get_Dir());
	Vector3 offset2(radius*line2.Get_Dir());
	offset1.Rotate_Z(-PI/2);
	offset2.Rotate_Z(-PI/2);

	Vector3 p1 = Vector3(pr1->X, pr1->Y, 0)+offset1;
	Vector3 p2 = Vector3(pr2->X, pr2->Y, 0)+offset1;
	LineSegClass offsetLine1(p1,p2);
	p1 = Vector3(pr3->X, pr3->Y, 0)+offset2;
	p2 = Vector3(pr4->X, pr4->Y, 0)+offset2;
	LineSegClass offsetLine2(p1,p2);
	Vector3 pInt1, pInt2;
	Vector3 pInt3, pInt4;
	Real nu; // not used.
	if (offsetLine1.Find_Intersection(offsetLine2, &pInt1, &nu, &pInt2, &nu) ) {
		m_roads[ndx2].m_pt2.last = true;
		LineSegClass cross1(pInt1, pInt1-offset2);
		LineSegClass cross2(pInt1, pInt1-offset1);
		cross1.Find_Intersection(line2, &pInt1, &nu, &pInt2, &nu);
		cross2.Find_Intersection(line1, &pInt3, &nu, &pInt4, &nu);
		// Make sure the lines didn't clip out of existence.
		Real theDot = Vector3::Dot_Product(line2.Get_Dir(), pInt1-Vector3(pr3->X, pr3->Y, 0));
		if (theDot < DOT_LIMIT) {
			*pr1 = originalPt;
			*pr4 = originalPt;
			miter(ndx1, ndx2);
			return;
		}
		theDot = Vector3::Dot_Product(line1.Get_Dir(), Vector3(pr2->X, pr2->Y, 0)-pInt3);
		if (theDot < DOT_LIMIT) {
			*pr1 = originalPt;
			*pr4 = originalPt;
			miter(ndx1, ndx2);
			return;
		}
		*pr4 = Vector2(pInt1.X, pInt1.Y);
		Real angle = -PI/6.0f; // -30 degrees.
		Vector2 pt2 = *pr4;
		Vector2 pt1 = *pr3;
		Vector2 direction(pt1.X-pt2.X, pt1.Y-pt2.Y);
		// offset = normal of the vector from pt1 to pt2.
		Vector2 centerOfCurve(-direction.Y,  direction.X);
		centerOfCurve.Normalize();
		centerOfCurve *= m_roads[ndx1].m_curveRadius*m_roads[ndx1].m_scale;
		centerOfCurve += pt2;

		rotateAbout(&pt2, centerOfCurve, angle);
		direction.Rotate(angle);
		pt1 = pt2+direction;

		m_roads[m_numRoads].m_pt1.loc=pt2;
		m_roads[m_numRoads].m_pt2.loc=pt1;

		CHECK_SEGMENTS;
		m_roads[m_numRoads].m_pt1.last = true; // if not, that one will clear flag in prior loop.
		m_roads[m_numRoads].m_pt2.last = true; // if not, that one will clear flag in prior loop.
		m_roads[m_numRoads].m_scale = m_roads[ndx1].m_scale;
		m_roads[m_numRoads].m_widthInTexture = m_roads[ndx1].m_widthInTexture;
		m_roads[m_numRoads].m_type = CURVE;
		m_roads[m_numRoads].m_curveRadius = m_roads[ndx1].m_curveRadius;
		m_roads[m_numRoads].m_uniqueID = m_roads[ndx1].m_uniqueID;
		m_numRoads++;
		if (count > 2.0) {
			Int i;
			for (i=2; i<count; i++) {
				// offset = normal of the vector from pt1 to pt2.
				direction.Rotate(angle);
				rotateAbout(&pt2, centerOfCurve, angle);
				pt1 = pt2+direction;
				CHECK_SEGMENTS;
				m_roads[m_numRoads].m_pt1.loc.Set(pt2);
				m_roads[m_numRoads].m_pt2.loc.Set(pt1);
				m_roads[m_numRoads].m_pt1.last = true; // if not, that one will clear flag in prior loop.
				m_roads[m_numRoads].m_pt2.last = true; // if not, that one will clear flag in prior loop.
				m_roads[m_numRoads].m_scale = m_roads[ndx1].m_scale;
				m_roads[m_numRoads].m_widthInTexture = m_roads[ndx1].m_widthInTexture;
				m_roads[m_numRoads].m_type = CURVE;
				m_roads[m_numRoads].m_curveRadius = m_roads[ndx1].m_curveRadius;
				m_roads[m_numRoads].m_uniqueID = m_roads[ndx1].m_uniqueID;
				m_numRoads++;
			}
		}

		*pr1 = Vector2(pInt3.X, pInt3.Y);

		m_roads[ndx1].m_pt1.last = true;
		if (count > 1.0) {
			pt2 = *pr1;
			pt1 = *pr1+*pr1-*pr2;
			direction.Set(pt1.X-pt2.X, pt1.Y-pt2.Y);
			pt1 = pt2+direction;
			CHECK_SEGMENTS;
			m_roads[m_numRoads].m_pt1.loc.Set(pt2);
			m_roads[m_numRoads].m_pt2.loc.Set(pt1);
			m_roads[m_numRoads].m_pt1.last = true; // if not, that one will clear flag in prior loop.
			m_roads[m_numRoads].m_pt2.last = true; // if not, that one will clear flag in prior loop.
			m_roads[m_numRoads].m_scale = m_roads[ndx1].m_scale;
			m_roads[m_numRoads].m_widthInTexture = m_roads[ndx1].m_widthInTexture;
			m_roads[m_numRoads].m_type = CURVE;
			m_roads[m_numRoads].m_curveRadius = m_roads[ndx1].m_curveRadius;
			m_roads[m_numRoads].m_uniqueID = m_roads[ndx1].m_uniqueID;
			m_numRoads++;
		}

		// Recalculate top & bottom.
 		Vector2 roadVector = m_roads[ndx1].m_pt2.loc - m_roads[ndx1].m_pt1.loc;
		Vector2 roadNormal(-roadVector.Y, roadVector.X);
		roadNormal.Normalize();
		roadNormal *= (m_roads[ndx1].m_scale*m_roads[ndx1].m_widthInTexture/2.0f);
		m_roads[ndx1].m_pt1.top = m_roads[ndx1].m_pt1.loc+roadNormal;
		m_roads[ndx1].m_pt1.bottom = m_roads[ndx1].m_pt1.loc - roadNormal;

 		roadVector = m_roads[ndx2].m_pt2.loc - m_roads[ndx2].m_pt1.loc;
		roadNormal = Vector2(-roadVector.Y, roadVector.X);
		roadNormal.Normalize();
		roadNormal *= (m_roads[ndx2].m_scale*m_roads[ndx2].m_widthInTexture/2.0f);
		m_roads[ndx2].m_pt2.top = m_roads[ndx2].m_pt2.loc+roadNormal;
		m_roads[ndx2].m_pt2.bottom = m_roads[ndx2].m_pt2.loc - roadNormal;

	}


}

//=============================================================================
// W3DRoadBuffer::rotateAbout
//=============================================================================
/** Rotates ptP about center. */
//=============================================================================
void W3DRoadBuffer::rotateAbout(Vector2 *ptP, Vector2 center, Real angle)
{
	Vector2 offset;
	offset.X = ptP->X - center.X;
	offset.Y = ptP->Y - center.Y;
	Vector2 orgOffset = offset;
	offset.Rotate(angle);
	offset.Y -= orgOffset.Y;
	offset.X -= orgOffset.X;
	*ptP += offset;
}

//-----------------------------------------------------------------------------
//         Public Functions
//-----------------------------------------------------------------------------

//=============================================================================
// W3DRoadBuffer::~W3DRoadBuffer
//=============================================================================
/** Destructor. Releases w3d assets. */
//=============================================================================
W3DRoadBuffer::~W3DRoadBuffer(void)
{
	freeRoadBuffers();
	REF_PTR_RELEASE(m_map);
}

//=============================================================================
// W3DRoadBuffer::W3DRoadBuffer
//=============================================================================
/** Constructor.  */
//=============================================================================
W3DRoadBuffer::W3DRoadBuffer(void)	:
	m_roads(NULL),
	m_numRoads(0),
	m_initialized(false),
	m_map(NULL),
#ifdef LOAD_TEST_ASSETS
	m_maxUID(0),
#endif // LOAD_TEST_ASSETS
	m_lightsIterator(NULL),
	m_maxRoadSegments(500),
	m_maxRoadTypes(8),
	m_maxRoadVertex(1000),
	m_maxRoadIndex(2000),
	m_curRoadType(0)

{
	allocateRoadBuffers();
}


//=============================================================================
// W3DRoadBuffer::freeRoadBuffers
//=============================================================================
/** Frees the index and vertex buffers. */
//=============================================================================
void W3DRoadBuffer::freeRoadBuffers(void)
{
	delete[] m_roads;
	m_roads = NULL;

	delete[] m_roadTypes;
	m_roadTypes = NULL;
}

//=============================================================================
// W3DRoadBuffer::allocateRoadBuffers
//=============================================================================
/** Allocates the index and vertex buffers. */
//=============================================================================
void W3DRoadBuffer::allocateRoadBuffers(void)
{
	Int i = 0;

	// save data for max limits
	m_maxRoadSegments = TheGlobalData->m_maxRoadSegments;
	m_maxRoadVertex = TheGlobalData->m_maxRoadVertex;
	m_maxRoadIndex = TheGlobalData->m_maxRoadIndex;
	m_maxRoadTypes = TheGlobalData->m_maxRoadTypes;

#ifdef LOAD_TEST_ASSETS
	m_maxRoadTypes+=4;
#endif

	m_curNumRoadVertices=0;
	m_curNumRoadIndices=0;
	m_roads = MSGNEW("RoadBuffer") RoadSegment[m_maxRoadSegments];
	m_roadTypes = MSGNEW("RoadBuffer") RoadType[m_maxRoadTypes];

	// load roads from INI
	TerrainRoadType *road;
	i = 0;
	for( road = TheTerrainRoads->firstRoad(); road; road = TheTerrainRoads->nextRoad( road ) )
	{

		// get a path to the texture file
		if( i < m_maxRoadTypes )
		{
			Int id = road->getID();
			m_roadTypes[ i++ ].loadTexture( road->getTexture(), id );
#ifdef LOAD_TEST_ASSETS
			if( m_maxUID < id )
				m_maxUID = id;
#endif // LOAD_TEST_ASSETS

		}

	}

#ifdef LOAD_TEST_ASSETS
	while (i<m_maxRoadTypes && m_roadTypes[i].isAutoLoaded()) {
		m_roadTypes[i++].loadTestTexture();
	}
	m_curOpenRoad = i;
#endif

	m_initialized = true;

}

//=============================================================================
// W3DRoadBuffer::clearAllRoads
//=============================================================================
/** Removes all roads. */
//=============================================================================
void W3DRoadBuffer::clearAllRoads(void)
{
	Int i;
	if (m_roads)
	for (i=0; i<m_numRoads; i++) {
		m_roads[i].SetIndexBuffer(NULL,0);
		m_roads[i].SetVertexBuffer(NULL,0);
	}
	m_numRoads = 0;
	if (m_roadTypes)
	for (i=0; i<m_maxRoadTypes; i++) {
		m_roadTypes[i].setStacking(0); // Reset stacking orders
	}
}
//=============================================================================
// W3DRoadBuffer::setMap
//=============================================================================
/** Sets the height map. */
//=============================================================================
void W3DRoadBuffer::setMap(WorldHeightMap *pMap)
{
	REF_PTR_SET(m_map, pMap);
}

//=============================================================================
// W3DRoadBuffer::loadRoads
//=============================================================================
/** Loads the roads from the map objects. */
//=============================================================================
void W3DRoadBuffer::loadRoads()
{
	if (!m_initialized) {
		return;
	}
	// Free any existing road segments.
	clearAllRoads();
	//Int ticks = ::GetTickCount();
	addMapObjects();
	updateCountsAndFlags();
	insertTeeIntersections();
	insertCurveSegments();
	insertCrossTypeJoins();
	preloadRoadsInVertexAndIndexBuffers();
	//ticks = ::GetTickCount() - ticks;
	//char buf[256];
	//sprintf(buf, "%d road segs, %d milisec.\n", m_numRoads, ticks);
	//::OutputDebugString(buf);
}

//=============================================================================
// W3DRoadBuffer::updateLighting
//=============================================================================
/** Draws the roads.  Uses terrain bounds to cull. */
//=============================================================================
void W3DRoadBuffer::updateLighting(void)
{
	Int curRoad;
	// Do road segments.
	for (curRoad=0; curRoad<m_numRoads; curRoad++) {
		m_roads[curRoad].updateSegLighting();
	}
}

//=============================================================================
// W3DRoadBuffer::drawRoads
//=============================================================================
/** Draws the roads.   */
//=============================================================================
void W3DRoadBuffer::drawRoads(CameraClass * camera, TextureClass *cloudTexture, TextureClass *noiseTexture, Bool wireframe,
															Int minX, Int maxX, Int minY, Int maxY, RefRenderObjListIterator *pDynamicLightsIterator)
{
	m_minX = minX*MAP_XY_FACTOR;
	m_maxX = maxX*MAP_XY_FACTOR;
	m_minY = minY*MAP_XY_FACTOR;
	m_maxY = maxY*MAP_XY_FACTOR;

	Int i;

	Int maxStacking = 0;
	for (i=0; i<m_maxRoadTypes; i++) {
		if (m_roadTypes[i].getStacking() > maxStacking) {
			maxStacking = m_roadTypes[i].getStacking();
		}
	}
	Int stacking;
	W3DShaderManager::ShaderTypes st=W3DShaderManager::ST_ROAD_BASE; //set default shader
	if (cloudTexture)
	{	st=W3DShaderManager::ST_ROAD_BASE_NOISE1;
		if (noiseTexture)
			st=W3DShaderManager::ST_ROAD_BASE_NOISE12;
	}
	else
	if (noiseTexture)
		st=W3DShaderManager::ST_ROAD_BASE_NOISE2;

	Int devicePasses = 1;	//assume regular rendering
 	//Find number of passes required to render current shader
	devicePasses=W3DShaderManager::getShaderPasses(st);

	W3DShaderManager::setTexture(1,cloudTexture);	//cloud
	W3DShaderManager::setTexture(2,noiseTexture);	//noise/lightmap

	for (stacking=0; stacking <= maxStacking; stacking++) {
		for (i=0; i<m_maxRoadTypes; i++) {
			if (stacking != m_roadTypes[i].getStacking()) {
				continue;
			}
			m_curUniqueID = m_roadTypes[i].getUniqueID();
			m_curRoadType = i;
			loadRoadsInVertexAndIndexBuffers();
			if (m_roadTypes[i].getNumIndices() == 0) continue;
			if (wireframe) {
				m_roadTypes[i].applyTexture();
				DX8Wrapper::Set_Texture(0,NULL);
				DX8Wrapper::Set_Shader(detailShader); // shows clipping.
			} else {
				m_roadTypes[i].applyTexture();
			}
	#ifdef RTS_DEBUG
			//DX8Wrapper::Set_Shader(detailShader); // shows clipping.
	#endif
			for (Int pass=0; pass < devicePasses; pass++)
			{
				if (!wireframe)
		 			W3DShaderManager::setShader(st, pass);
				//Draw all this road type.
				DX8Wrapper::Draw_Triangles(	0, m_roadTypes[i].getNumIndices()/3, 0,	m_roadTypes[i].getNumVertices());
			}

			if (!wireframe)	//shader was applied at least once?
 				W3DShaderManager::resetShader(st);
		}
	}

#if 0
	// Need to use a separate set of index & vertex buffers for this.  jba.
	DX8Wrapper::Set_Index_Buffer(NULL,0);
	DX8Wrapper::Set_Vertex_Buffer(NULL);
	if (pDynamicLightsIterator) {
		for (i=0; i<m_maxRoadTypes; i++) {
			m_curRoadType = i;
			m_curUniqueID = m_roadTypes[i].getUniqueID();
			if (m_curUniqueID < 0 || m_curUniqueID >= m_maxRoadTypes) continue;
			loadLitRoadsInVertexAndIndexBuffers(pDynamicLightsIterator);
			if (this->m_curNumRoadIndices == 0) continue;
			if (wireframe) {
					DX8Wrapper::Set_Texture(0,NULL);
			} else {
				m_roadTypes[i].applyTexture();
				if (cloudTexture) {
					DX8Wrapper::Set_Texture(1,cloudTexture);
				}
			}
			DX8Wrapper::Set_Shader(detailAlphaShader);
			//Draw all the roads.
			DX8Wrapper::Draw_Triangles(	0, m_curNumRoadIndices/3, 0,	m_curNumRoadVertices);
		}
	}
#endif
	m_curRoadType = 0;
}


