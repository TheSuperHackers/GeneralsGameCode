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

// FILE: W3DTerrainBackground.cpp ////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       EA Pacific.
//
//                       Confidential Information
//                Copyright (C) 2003 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: W3DTerrainBackground.cpp
//
// Created:   John Ahlquist, March 2003
//
// Desc:      Draw buffer to handle backup terrain at lower res.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//         Includes
//-----------------------------------------------------------------------------
#include "W3DDevice/GameClient/W3DTerrainBackground.h"

#include <stdio.h>
#include <string.h>
#include <assetmgr.h>
#include <texture.h>
#include "Common/GlobalData.h"
#include "GameClient/View.h"
#include "W3DDevice/GameClient/TerrainTex.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/dx8renderer.h"
#include "WW3D2/camera.h"


//-----------------------------------------------------------------------------
//         Private Data
//-----------------------------------------------------------------------------
// A W3D shader that does alpha, texturing, tests zbuffer, doesn't update zbuffer.
#define SC_DETAIL ( SHADE_CNST(ShaderClass::PASS_LEQUAL, ShaderClass::DEPTH_WRITE_ENABLE, ShaderClass::COLOR_WRITE_ENABLE, ShaderClass::SRCBLEND_ONE, \
	ShaderClass::DSTBLEND_ZERO, ShaderClass::FOG_DISABLE, ShaderClass::GRADIENT_MODULATE, ShaderClass::SECONDARY_GRADIENT_DISABLE, ShaderClass::TEXTURING_ENABLE, \
	ShaderClass::ALPHATEST_DISABLE, ShaderClass::CULL_MODE_DISABLE, \
	ShaderClass::DETAILCOLOR_DISABLE, ShaderClass::DETAILALPHA_DISABLE) )

static ShaderClass detailShader(SC_DETAIL);

const Int PIXELS_PER_GRID = 8; // default tex resolution allocated for each tile. jba. [3/24/2003]

//-----------------------------------------------------------------------------
//         Private Functions
//-----------------------------------------------------------------------------


//=============================================================================
// W3DTerrainBackground::loadTerrainInVertexAndIndexBuffers
//=============================================================================
/** Loads the terrain into the vertex buffer for drawing. */
//=============================================================================
void W3DTerrainBackground::setFlip(WorldHeightMap *htMap)
{
	if (m_map==NULL) return;
	if (htMap) {
		REF_PTR_SET(m_map, htMap);
	}
	if (!m_initialized) {
		return;
	}

	setFlipRecursive(0, 0, m_width);


}


const Int STEP=4;
//=============================================================================
// W3DTerrainBackground::doPartialUpdate
//=============================================================================
/** Updates a partial block of vertices from [x0,y0 to x1,y1]
The coordinates in partialRange are map cell coordinates, relative to the entire map.
The vertex coordinates and texture coordinates, as well as static lighting are updated.
*/
void W3DTerrainBackground::doPartialUpdate(const IRegion2D &partialRange, WorldHeightMap *htMap, Bool doTextures )
{
	if (m_map==NULL) return;
	if (htMap) {
		REF_PTR_SET(m_map, htMap);
	}

	if (!m_initialized) {
		return;
	}
	doTesselatedUpdate(partialRange, htMap, doTextures);

	return;

	Int requiredVertexSize = (m_width+1) * (m_width+1) + 6;
	if (m_vertexTerrainSize<requiredVertexSize || m_vertexTerrain==NULL) {
		m_vertexTerrainSize = requiredVertexSize;
		REF_PTR_RELEASE(m_vertexTerrain);
		REF_PTR_RELEASE(m_indexTerrain);
		m_vertexTerrain=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZDUV1,m_vertexTerrainSize+4,DX8VertexBufferClass::USAGE_DEFAULT));
	}

	Int requiredIndexSize = (m_width+1) * (m_width+1) + 6;
	if (m_indexTerrainSize<requiredIndexSize || m_indexTerrain==NULL) {
		m_indexTerrainSize = requiredIndexSize;
		REF_PTR_RELEASE(m_indexTerrain);
		m_indexTerrain=NEW_REF(DX8IndexBufferClass,(m_indexTerrainSize+4,DX8IndexBufferClass::USAGE_DEFAULT));
	}
	Int minX = m_xOrigin;
	Int minY = m_yOrigin;
	Int maxX = m_xOrigin + m_width;
	Int maxY = m_yOrigin + m_width;
	Int limitX = m_map->getXExtent()-1;
	Int limitY = m_map->getYExtent()-1;
	if (maxX>limitX) maxX = limitX;
	if (maxY>limitY) maxY = limitY;

	if (partialRange.lo.x > maxX) return;
	if (partialRange.lo.y > maxY) return;
	if (partialRange.hi.x < minX) return;
	if (partialRange.hi.y < minY) return;

	m_curNumTerrainVertices = 0;
	//m_curNumTerrainIndices = 0;
	VertexFormatXYZDUV2 *vb;
	UnsignedShort *ib;
	// Lock the buffer.
	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexTerrain);
	vb=(VertexFormatXYZDUV2*)lockVtxBuffer.Get_Vertex_Array();
	// Add to the vertex buffer.

	VertexFormatXYZDUV2 *curVb = vb;
	MinMaxAABoxClass bounds;
	bounds.Init_Empty();

	Int i, j;
	for (j=minY; j<=maxY; j+=STEP) {
		for (i=minX; i<=maxX; i+=STEP) {
			if (m_curNumTerrainVertices >= m_vertexTerrainSize) return;
			curVb->diffuse = (0<<24)|TheTerrainRenderObject->getStaticDiffuse(i,j);
			Vector3 pos;
			pos.Z = ((float)m_map->getHeight(i,j)*MAP_HEIGHT_SCALE);
			pos.X = (i)*MAP_XY_FACTOR - m_map->getBorderSizeInline()*MAP_XY_FACTOR;
			pos.Y = (j)*MAP_XY_FACTOR - m_map->getBorderSizeInline()*MAP_XY_FACTOR;
			curVb->u1 = (float)(i-minX)/(float)(m_width);
			curVb->v1 = 1.0f - (float)(j-minY)/(float)(m_width);
			curVb->x = pos.X;
			curVb->y = pos.Y;
			curVb->z = pos.Z;
			curVb++;
			m_curNumTerrainVertices++;
			bounds.Add_Point(pos);
		}
	}
	m_bounds.Init(bounds);

	if (m_terrainTexture == NULL || doTextures) {
		REF_PTR_RELEASE(m_terrainTexture);
		REF_PTR_RELEASE(m_terrainTexture2X);
		REF_PTR_RELEASE(m_terrainTexture4X);
		m_terrainTexture = m_map->getFlatTexture(m_xOrigin, m_yOrigin, m_width, PIXELS_PER_GRID);
		//	DEBUG ONLY. jba. m_terrainTexture =  (TerrainTextureClass *)NEW_REF(TextureClass, ("TBBib.tga"));
		m_terrainTexture->Get_Filter().Set_U_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
		m_terrainTexture->Get_Filter().Set_V_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
	}

	if (m_curNumTerrainIndices == 0) {
		// Only do the index buffer if it has never been done.  Index values don't change. jba.
		DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexTerrain);
		ib = lockIdxBuffer.Get_Index_Array();
		UnsignedShort *curIb = ib;
		Int yOffset = ((maxX - minX)/STEP+1);
		Int width = yOffset;
		Int height = (maxY - minY)/STEP;
		*curIb++ = width-1;
		m_curNumTerrainIndices++;
		for (j=0; j<height; j++) {
			*curIb++ = j*yOffset + yOffset + width-1;
			m_curNumTerrainIndices++;
			for (i=width-2; i>=0; i--) {
				if (m_curNumTerrainIndices+2 > m_indexTerrainSize) return;
				*curIb++ = j*yOffset + i;
				*curIb++ = j*yOffset + i+yOffset;
				m_curNumTerrainIndices+=2;
			}
			j++;
			if (j<height) {
				*curIb++ = j*yOffset + yOffset;
				m_curNumTerrainIndices++;
				for (i=1; i<width; i++) {
					if (m_curNumTerrainIndices+2 > m_indexTerrainSize) return;
					*curIb++ = j*yOffset + i;
					*curIb++ = j*yOffset + i+yOffset;
					m_curNumTerrainIndices+=2;
				}
			}
		}
	}
}

//=============================================================================
// W3DTerrainBackground::fillVBRecursive
//=============================================================================
/** Fills in vertex & index buffers.
*/
Bool W3DTerrainBackground::advanceLeft(ICoord2D &left, Int xOffset, Int yOffset, Int width)
{
	while (left.y < yOffset+width) {
		left.y++;
		if (m_map->getFlipState(left.x+m_xOrigin, left.y+m_yOrigin)) {
			return true;
		}
	}
	while (left.x < xOffset+width-1) {
		left.x++;
		if (m_map->getFlipState(left.x+m_xOrigin, left.y+m_yOrigin)) {
			return true;
		}
	}
	return false;
}

//=============================================================================
// W3DTerrainBackground::fillVBRecursive
//=============================================================================
/** Fills in vertex & index buffers.
*/
Bool W3DTerrainBackground::advanceRight(ICoord2D &right, Int xOffset, Int yOffset, Int width)
{
	while (right.x < xOffset+width) {
		right.x++;
		if (m_map->getFlipState(right.x+m_xOrigin, right.y+m_yOrigin)) {
			return true;
		}
	}
	while (right.y < yOffset+width-1) {
		right.y++;
		if (m_map->getFlipState(right.x+m_xOrigin, right.y+m_yOrigin)) {
			return true;
		}
	}
	return false;
}

//=============================================================================
// W3DTerrainBackground::fillVBRecursive
//=============================================================================
/** Fills in vertex & index buffers.
*/
void W3DTerrainBackground::fillVBRecursive(UnsignedShort *ib, Int xOffset, Int yOffset,
																					 Int width, UnsignedShort *ndx, Int &curIndex)
{

	Int bottomLeftNdx	= ndx[xOffset+yOffset*(m_width+1)];
	Int topRightNdx	= ndx[xOffset+width + (yOffset+width)*(m_width+1)];

	Int limitX = m_map->getXExtent()-1;
	Int limitY = m_map->getYExtent()-1;
	Int i, j;
	Bool match = true;
	Int minX = m_xOrigin+xOffset;
	Int minY = m_yOrigin+yOffset;
	Int cornerHeight = m_map->getHeight(minX, minY);

	for (i=0; i<=width; i++) {
		for (j=0; j<=width; j++) {
			Int k = minX+i;
			k = k<limitX?k:limitX;
			Int l = minY+j;
			l = l<limitY?l:limitY;
			if (cornerHeight!=m_map->getHeight(k, l)) {
				match = false;
				break;
			}
		}
	}
	if (width==1) {
		match = true;
	}

	if (match) {


		UnsignedShort prevNdxLeft;
		UnsignedShort prevNdxRight;
		ICoord2D left;
		left.x = xOffset;
		left.y = yOffset;
		ICoord2D right;
		right.x = xOffset;
		right.y = yOffset;
		advanceLeft(left, xOffset, yOffset, width);
		advanceRight(right, xOffset, yOffset, width);

		if (ib) {
			ib[curIndex] = bottomLeftNdx;
		}
		curIndex++;

		prevNdxRight = ndx[right.x+right.y*(m_width+1)];
		if (ib) {
			ib[curIndex] = prevNdxRight;
		}
		curIndex++;

		prevNdxLeft = ndx[left.x+left.y*(m_width+1)];
		if (ib) {
			ib[curIndex] = prevNdxLeft;
		}
		curIndex++;
		Bool didLeft = true;
		Bool didRight = true;
		while (didLeft || didRight) {
			didLeft = advanceLeft(left, xOffset, yOffset, width);
			if (didLeft) {

				if (ib) {
					ib[curIndex] = prevNdxLeft;
				}
				curIndex++;

				if (ib) {
					ib[curIndex] = prevNdxRight;
				}
				curIndex++;

				prevNdxLeft = ndx[left.x+left.y*(m_width+1)];
				if (ib) {
					ib[curIndex] = prevNdxLeft;
				}
				curIndex++;
			}
			didRight = advanceRight(right, xOffset, yOffset, width);
			if (didRight) {

				if (ib) {
					ib[curIndex] = prevNdxLeft;
				}
				curIndex++;

				if (ib) {
					ib[curIndex] = prevNdxRight;
				}
				curIndex++;

				prevNdxRight = ndx[right.x+right.y*(m_width+1)];
				if (ib) {
					ib[curIndex] = prevNdxRight;
				}
				curIndex++;
			}

		}

		if (ib) {
			ib[curIndex] = prevNdxLeft;
		}

		curIndex++;

		if (ib) {
			ib[curIndex] = prevNdxRight;
		}
		curIndex++;

		if (ib) {
			ib[curIndex] = topRightNdx;
		}
		curIndex++;

		return;
	}
	Int halfWidth = width/2;

	fillVBRecursive(ib, xOffset, yOffset, halfWidth, ndx, curIndex);
	fillVBRecursive(ib, xOffset, yOffset+halfWidth, halfWidth, ndx, curIndex);
	fillVBRecursive(ib, xOffset+halfWidth, yOffset, halfWidth, ndx, curIndex);
	fillVBRecursive(ib, xOffset+halfWidth, yOffset+halfWidth, halfWidth, ndx, curIndex);

}

//=============================================================================
// W3DTerrainBackground::fillVBRecursive
//=============================================================================
/** Fills in vertex & index buffers.
*/
void W3DTerrainBackground::setFlipRecursive(Int xOffset, Int yOffset, Int width)
{

	Int limitX = m_map->getXExtent()-1;
	Int limitY = m_map->getYExtent()-1;
	Int i, j;
	Bool match = true;
	Int minX = m_xOrigin+xOffset;
	Int minY = m_yOrigin+yOffset;
	Int cornerHeight = m_map->getHeight(minX, minY);

	for (i=0; i<=width; i++) {
		for (j=0; j<=width; j++) {
			Int k = minX+i;
			k = k<limitX?k:limitX;
			Int l = minY+j;
			l = l<limitY?l:limitY;
			if (cornerHeight!=m_map->getHeight(k, l)) {
				match = false;
				break;
			}
		}
	}
	if (width==1) {
		match = true;
	}

	if (match) {
		m_map->setFlipState(minX, minY, true);
		m_map->setFlipState(minX+width, minY, true);
		m_map->setFlipState(minX+width, minY+width, true);
		m_map->setFlipState(minX, minY+width, true);
		return;
	}
	Int halfWidth = width/2;


	setFlipRecursive(xOffset, yOffset, halfWidth);
	setFlipRecursive(xOffset, yOffset+halfWidth, halfWidth);
	setFlipRecursive(xOffset+halfWidth, yOffset, halfWidth);
	setFlipRecursive(xOffset+halfWidth, yOffset+halfWidth, halfWidth);

}

//=============================================================================
// W3DTerrainBackground::doTesselatedUpdate
//=============================================================================
/** Updates a partial block of vertices from [x0,y0 to x1,y1]
The coordinates in partialRange are map cell coordinates, relative to the entire map.
The vertex coordinates and texture coordinates, as well as static lighting are updated.
*/
void W3DTerrainBackground::doTesselatedUpdate(const IRegion2D &partialRange, WorldHeightMap *htMap, Bool doTextures )
{
	if (m_map==NULL) return;
	if (htMap) {
		REF_PTR_SET(m_map, htMap);
	}
	if (!m_initialized) {
		return;
	}
	Int minX = m_xOrigin;
	Int minY = m_yOrigin;
	Int maxX = m_xOrigin + m_width;
	Int maxY = m_yOrigin + m_width;
	Int limitX = m_map->getXExtent()-1;
	Int limitY = m_map->getYExtent()-1;

	if (partialRange.lo.x > maxX) return;
	if (partialRange.lo.y > maxY) return;
	if (partialRange.hi.x < minX) return;
	if (partialRange.hi.y < minY) return;

	setFlip(htMap);

	Int count = (m_width+1)*(m_width+1);

	UnsignedShort *ndx = new UnsignedShort[count];

	Int requiredVertex = 0;
	Int i, j;
	for (j=minY; j<=maxY; j++) {
		for (i=minX; i<=maxX; i++) {
			Int ndxNdx = i-minX + (m_width+1)*(j-minY);
			DEBUG_ASSERTCRASH(ndxNdx<count, ("Bad ndxNdx"));
			ndx[ndxNdx] = 0;
			if (m_map->getFlipState(i, j)) {
				requiredVertex++;
			}
		}
	}

	if (m_vertexTerrainSize<requiredVertex || m_vertexTerrain==NULL) {
		m_vertexTerrainSize = requiredVertex;
		REF_PTR_RELEASE(m_vertexTerrain);
		m_vertexTerrain=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZDUV2,m_vertexTerrainSize+4,DX8VertexBufferClass::USAGE_DEFAULT));
	}

	m_curNumTerrainVertices = 0;
	VertexFormatXYZDUV2 *vb;
	// Lock the buffer.
	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexTerrain);
	vb=(VertexFormatXYZDUV2*)lockVtxBuffer.Get_Vertex_Array();
	VertexFormatXYZDUV2 *curVb = vb;
	// Add to the vertex buffer.
	for (j=minY; j<=maxY; j++) {
		for (i=minX; i<=maxX; i++) {
			if (m_map->getFlipState(i, j)) {
				curVb->diffuse = (0<<24)|TheTerrainRenderObject->getStaticDiffuse(i,j);
				Vector3 pos;
				Int k = i<limitX?i:limitX;
				Int l = j<limitY?j:limitY;
				pos.Z = ((float)m_map->getHeight(k,l)*MAP_HEIGHT_SCALE);
				pos.X = (i)*MAP_XY_FACTOR - m_map->getBorderSizeInline()*MAP_XY_FACTOR;
				pos.Y = (j)*MAP_XY_FACTOR - m_map->getBorderSizeInline()*MAP_XY_FACTOR;
				curVb->u1 = (float)(i-minX)/(float)(m_width);
				curVb->v1 = 1.0f - (float)(j-minY)/(float)(m_width);
				curVb->x = pos.X;
				curVb->y = pos.Y;
				curVb->z = pos.Z;
				curVb++;
				Int ndxNdx = i-minX + (m_width+1)*(j-minY);
				DEBUG_ASSERTCRASH(ndxNdx<count, ("Bad ndxNdx"));
				ndx[ndxNdx] = m_curNumTerrainVertices;
				m_curNumTerrainVertices++;
			}
		}
	}

	Int requiredIndex = 0;

	fillVBRecursive(NULL, 0, 0, m_width, ndx, requiredIndex);

	if (m_indexTerrainSize<requiredIndex || m_indexTerrain==NULL) {
		m_indexTerrainSize = requiredIndex;
		REF_PTR_RELEASE(m_indexTerrain);
		m_indexTerrain=NEW_REF(DX8IndexBufferClass,(m_indexTerrainSize+4,DX8IndexBufferClass::USAGE_DEFAULT));
	}

	m_curNumTerrainIndices = 0;

	UnsignedShort *ib;
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexTerrain);
	ib = lockIdxBuffer.Get_Index_Array();
	fillVBRecursive(ib, 0, 0, m_width, ndx, m_curNumTerrainIndices);
	delete[] ndx;
	ndx = NULL;

	MinMaxAABoxClass bounds;
	bounds.Init_Empty();

	for (j=minY; j<=maxY; j+=1) {
		for (i=minX; i<=maxX; i+=1) {
			Vector3 pos;
			Int k = i<limitX?i:limitX;
			Int l = j<limitY?j:limitY;
			pos.Z = ((float)m_map->getHeight(k,l)*MAP_HEIGHT_SCALE);
			pos.X = (i)*MAP_XY_FACTOR - m_map->getBorderSizeInline()*MAP_XY_FACTOR;
			pos.Y = (j)*MAP_XY_FACTOR - m_map->getBorderSizeInline()*MAP_XY_FACTOR;
			bounds.Add_Point(pos);
		}
	}
	m_bounds.Init(bounds);

	if (m_terrainTexture == NULL || doTextures) {
		REF_PTR_RELEASE(m_terrainTexture);
		REF_PTR_RELEASE(m_terrainTexture2X);
		REF_PTR_RELEASE(m_terrainTexture4X);
		m_terrainTexture = m_map->getFlatTexture(m_xOrigin, m_yOrigin, m_width, PIXELS_PER_GRID);
		//	DEBUG ONLY. jba. m_terrainTexture =  (TerrainTextureClass *)NEW_REF(TextureClass, ("TBBib.tga"));
		m_terrainTexture->Get_Filter().Set_U_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
		m_terrainTexture->Get_Filter().Set_V_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
	}

}

//-----------------------------------------------------------------------------
//         Public Functions
//-----------------------------------------------------------------------------

//=============================================================================
// W3DTerrainBackground::~W3DTerrainBackground
//=============================================================================
/** Destructor. Releases w3d assets. */
//=============================================================================
W3DTerrainBackground::~W3DTerrainBackground(void)
{
	freeTerrainBuffers();
	REF_PTR_RELEASE(m_terrainTexture);
	REF_PTR_RELEASE(m_terrainTexture2X);
	REF_PTR_RELEASE(m_terrainTexture4X);
}

//=============================================================================
// W3DTerrainBackground::W3DTerrainBackground
//=============================================================================
/** Constructor. Sets m_initialized to true if it finds the w3d models it needs
for the bibs. */
//=============================================================================
W3DTerrainBackground::W3DTerrainBackground(void):
m_vertexTerrain(NULL),
m_vertexTerrainSize(0),
m_initialized(FALSE),
m_indexTerrain(NULL),
m_indexTerrainSize(0),
m_terrainTexture(NULL),
m_terrainTexture2X(NULL),
m_terrainTexture4X(NULL),
m_cullStatus(CULL_STATUS_UNKNOWN),
m_texMultiplier(TEX1X)
{
}

//=============================================================================
// W3DTerrainBackground::freeTerrainBuffers
//=============================================================================
/** Frees the index and vertex buffers. */
//=============================================================================
void W3DTerrainBackground::freeTerrainBuffers(void)
{
	REF_PTR_RELEASE(m_vertexTerrain);
	REF_PTR_RELEASE(m_indexTerrain);
	m_curNumTerrainVertices=0;
	m_curNumTerrainIndices=0;
	m_initialized = false;
	REF_PTR_RELEASE(m_map);
	REF_PTR_RELEASE(m_map);
}

//=============================================================================
// W3DTerrainBackground::allocateTerrainBuffers
//=============================================================================
/** Allocates the index and vertex buffers. */
//=============================================================================
void W3DTerrainBackground::allocateTerrainBuffers(WorldHeightMap *htMap, Int xOrigin, Int yOrigin, Int width)
{
	if (htMap==NULL) return;
	freeTerrainBuffers(); // in case already allocated. jba [3/24/2003]
	m_curNumTerrainVertices=0;
	m_curNumTerrainIndices=0;
	m_xOrigin = xOrigin;
	m_yOrigin = yOrigin;
	m_width = width;
	m_initialized = true;
	REF_PTR_SET(m_map, htMap);
}


//=============================================================================
// W3DTerrainBackground::updateCenter
//=============================================================================
/** Updates the culling status. */
//=============================================================================
void W3DTerrainBackground::updateCenter(CameraClass *camera)
{
	if (camera->Cull_Box(m_bounds)) {
		m_cullStatus = CULL_STATUS_INVISIBLE;
	}	else {
		m_cullStatus = CULL_STATUS_VISIBLE;
	}

	if (m_cullStatus==CULL_STATUS_INVISIBLE) {
		REF_PTR_RELEASE(m_terrainTexture2X);
		REF_PTR_RELEASE(m_terrainTexture4X);
		m_texMultiplier = TEX1X;
		return;
	}
	Vector3 cameraPos = camera->Get_Position();
	const Real mipDistance = 310;
	const Real mipSlop = 40;
	const Real mip4xDistanceSqr = sqr(mipDistance+mipSlop);
	const Real mip2xDistanceSqr = sqr(2*mipDistance+mipSlop);
	const Real mipLODDistanceSqr = sqr(4*mipDistance+mipSlop);
	Real minDistSqr = 2*mip2xDistanceSqr;
	Int i, j, k;
	for (i=-1; i<2; i++) {
		for (j=-1; j<2; j++) {
			for (k=-1; k<2; k++) {
				Vector3 corner = m_bounds.Center;
				corner.X += m_bounds.Extent.X * i;
				corner.Y += m_bounds.Extent.Y * j;
				corner.Z += m_bounds.Extent.Z * k;
				Real distSqr = (cameraPos-corner).Length2();
				if (distSqr<minDistSqr) minDistSqr = distSqr;
			}
		}
	}
	m_texMultiplier = TEX1X;
	if (minDistSqr<mip4xDistanceSqr) {
		m_texMultiplier = TEX4X;
	} else if (minDistSqr<mip2xDistanceSqr) {
		m_texMultiplier = TEX2X;
	} else {
		REF_PTR_RELEASE(m_terrainTexture4X);
		REF_PTR_RELEASE(m_terrainTexture2X);
		Int LOD = 0;
		if (minDistSqr>mipLODDistanceSqr) {
			LOD = 1;
		}
		m_terrainTexture->setLOD(LOD);
	}

}

//=============================================================================
// W3DTerrainBackground::updateCenter
//=============================================================================
/** Updates the culling status. */
//=============================================================================
void W3DTerrainBackground::updateTexture(void)
{
	if (m_cullStatus==CULL_STATUS_INVISIBLE) {
		REF_PTR_RELEASE(m_terrainTexture2X);
		REF_PTR_RELEASE(m_terrainTexture4X);
		return;
	}

	if (m_texMultiplier == TEX4X) {
		REF_PTR_RELEASE(m_terrainTexture2X);
		if (m_terrainTexture4X == NULL) {
			m_terrainTexture4X = m_map->getFlatTexture(m_xOrigin, m_yOrigin, m_width, 4*PIXELS_PER_GRID);
			m_terrainTexture4X->Get_Filter().Set_U_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
			m_terrainTexture4X->Get_Filter().Set_V_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
		}
	} else if (m_texMultiplier == TEX2X) {
		REF_PTR_RELEASE(m_terrainTexture4X);
		if (m_terrainTexture2X == NULL) {
			m_terrainTexture2X = m_map->getFlatTexture(m_xOrigin, m_yOrigin, m_width, 2*PIXELS_PER_GRID);
			m_terrainTexture2X->Get_Filter().Set_U_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
			m_terrainTexture2X->Get_Filter().Set_V_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
		}
	} else {
		REF_PTR_RELEASE(m_terrainTexture4X);
		REF_PTR_RELEASE(m_terrainTexture2X);
	}

}

//=============================================================================
// W3DTerrainBackground::renderTerrain
//=============================================================================
//=============================================================================
void W3DTerrainBackground::drawVisiblePolys(RenderInfoClass & rinfo, Bool disableTextures)
{
#if 1
	if (m_curNumTerrainIndices == 0) {
		return;
	}
	if (m_cullStatus==CULL_STATUS_INVISIBLE) {
		return;
	}
	// Setup the vertex buffer, shader & texture.
	DX8Wrapper::Set_Index_Buffer(m_indexTerrain,0);
	DX8Wrapper::Set_Vertex_Buffer(m_vertexTerrain);
  if (!disableTextures) {
		if (m_terrainTexture4X) {
			DX8Wrapper::Set_Texture(1, m_terrainTexture4X);
		}	else if (m_terrainTexture2X) {
			DX8Wrapper::Set_Texture(1, m_terrainTexture2X);
		}	else {
			DX8Wrapper::Set_Texture(1, m_terrainTexture);
		}
	}
	DX8Wrapper::Draw_Triangles(	0, m_curNumTerrainIndices/3, 0,	m_curNumTerrainVertices);
#else
	if (m_curNumTerrainIndices == 0) {
		return;
	}
	if (m_cullStatus==CULL_STATUS_INVISIBLE) {
		return;
	}
	// Setup the vertex buffer, shader & texture.
	DX8Wrapper::Set_Index_Buffer(m_indexTerrain,0);
	DX8Wrapper::Set_Vertex_Buffer(m_vertexTerrain);
  if (!disableTextures) {
		if (m_terrainTexture4X) {
			DX8Wrapper::Set_Texture(0, m_terrainTexture4X);
		}	else if (m_terrainTexture2X) {
			DX8Wrapper::Set_Texture(0, m_terrainTexture2X);
		}	else {
			DX8Wrapper::Set_Texture(0, m_terrainTexture);
		}
	}
	DX8Wrapper::Draw_Triangles(	0, m_curNumTerrainIndices/3, 0,	m_curNumTerrainVertices);
#endif
}





