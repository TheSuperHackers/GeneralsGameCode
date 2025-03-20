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

// FILE: W3DBibBuffer.cpp ////////////////////////////////////////////////
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
// File name: W3DBibBuffer.cpp
//
// Created:   John Ahlquist, May 2001
//
// Desc:      Draw buffer to handle all the bibs in a scene.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//         Includes                                                      
//-----------------------------------------------------------------------------
#include "W3DDevice/GameClient/W3DBibBuffer.h"

#include <stdio.h>
#include <string.h>
#include <assetmgr.h>
#include <texture.h>
#include "Common/GlobalData.h"
#include "Common/RandomValue.h"
#include "W3DDevice/GameClient/TerrainTex.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DDynamicLight.h"
#include "WW3D2/camera.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/dx8renderer.h"
#include "WW3D2/mesh.h"
#include "WW3D2/meshmdl.h"

//-----------------------------------------------------------------------------
//         Private Data                                                     
//-----------------------------------------------------------------------------
// A W3D shader that does alpha, texturing, tests zbuffer, doesn't update zbuffer.
#define SC_ALPHA_DETAIL ( SHADE_CNST(ShaderClass::PASS_ALWAYS, ShaderClass::DEPTH_WRITE_DISABLE, ShaderClass::COLOR_WRITE_ENABLE, ShaderClass::SRCBLEND_SRC_ALPHA, \
	ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA, ShaderClass::FOG_DISABLE, ShaderClass::GRADIENT_MODULATE, ShaderClass::SECONDARY_GRADIENT_DISABLE, ShaderClass::TEXTURING_ENABLE, \
	ShaderClass::ALPHATEST_DISABLE, ShaderClass::CULL_MODE_DISABLE, \
	ShaderClass::DETAILCOLOR_DISABLE, ShaderClass::DETAILALPHA_DISABLE) )

static ShaderClass detailAlphaShader(SC_ALPHA_DETAIL);


//-----------------------------------------------------------------------------
//         Private Functions                                               
//-----------------------------------------------------------------------------


//=============================================================================
// W3DBibBuffer::loadBibsInVertexAndIndexBuffers
//=============================================================================
/** Loads the bibs into the vertex buffer for drawing. */
//=============================================================================
void W3DBibBuffer::loadBibsInVertexAndIndexBuffers(void)
{
	if (!m_indexBib || !m_vertexBib || !m_initialized) {
		return;
	}
	if (!m_anythingChanged) {
		return;
	}

	m_curNumBibVertices = 0;
	m_curNumBibIndices = 0;
	m_curNumNormalBibIndices = 0;
	m_curNumNormalBibVertex = 0;

	if (m_numBibs==0) {
		return;	
	}

	VertexFormatXYZDUV1 *vb;
	UnsignedShort *ib;
	// Lock the buffers.
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexBib, D3DLOCK_DISCARD);
	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexBib, D3DLOCK_DISCARD);
	vb=(VertexFormatXYZDUV1*)lockVtxBuffer.Get_Vertex_Array();
	ib = lockIdxBuffer.Get_Index_Array();
	// Add to the index buffer & vertex buffer.
	UnsignedShort *curIb = ib;

	VertexFormatXYZDUV1 *curVb = vb;

	Int curBib;

	// Calculate a static lighting value to use for all the bibs.
	Real shadeR, shadeG, shadeB;
	shadeR = TheGlobalData->m_terrainAmbient[0].red;
	shadeG = TheGlobalData->m_terrainAmbient[0].green;
	shadeB = TheGlobalData->m_terrainAmbient[0].blue;
	shadeR += TheGlobalData->m_terrainDiffuse[0].red;
	shadeG += TheGlobalData->m_terrainDiffuse[0].green;
	shadeB += TheGlobalData->m_terrainDiffuse[0].blue;
	if (shadeR>1.0f) shadeR=1.0f;
	if (shadeG>1.0f) shadeG=1.0f;
	if (shadeB>1.0f) shadeB=1.0f;
	shadeR*=255.0f;
	shadeG*=255.0f;
	shadeB*=255.0f;

	Int diffuse = (REAL_TO_INT(shadeB) | (REAL_TO_INT(shadeG) << 8) | (REAL_TO_INT(shadeR) << 16) | (255 << 24));
	Int doHighlight;
	try {
	for (doHighlight=0; doHighlight<=1; doHighlight++) 
	{
		if (doHighlight==1) 
		{
			m_curNumNormalBibIndices = m_curNumBibIndices;
			m_curNumNormalBibVertex = m_curNumBibVertices;
		}
		for (curBib=0; curBib<m_numBibs; curBib++) {
			if (m_bibs[curBib].m_unused) continue;
			if (m_bibs[curBib].m_highlight != (Bool)doHighlight) continue;
			Int startVertex = m_curNumBibVertices;
			Int i;
			Int numVertex = 4;
			if (m_curNumBibVertices+numVertex+2>= m_vertexBibSize) {
				break;
			}
			Int numIndex = 6;
			if (m_curNumBibIndices+numIndex+6 >= m_indexBibSize) {
				break;
			}

			for (i=0; i<numVertex; i++) {

				// Update the uv values.  The W3D models each have their own texture, and 
				// we use one texture with all images in one, so we have to change the uvs to 
				// match.
				Real U, V;
				Vector3 vLoc=m_bibs[curBib].m_corners[i];
				switch (i) {
					case 0 :
						U=0;V=1;
						break;
					case 1:
						U=1;V=1;
						break;
					case 2:
						U=1;V=0;
						break;
					case 3:
						U=0;V=0;
						break;
				}

				curVb->u1 = U;
				curVb->v1 = V;
				curVb->x = vLoc.X;
				curVb->y = vLoc.Y;
				curVb->z = vLoc.Z;	 
				curVb->diffuse = diffuse;
				curVb++;
				m_curNumBibVertices++;
			}

			*curIb++ = startVertex + 0;
			*curIb++ = startVertex + 1;
			*curIb++ = startVertex + 2;
			*curIb++ = startVertex + 0;
			*curIb++ = startVertex + 2;
			*curIb++ = startVertex + 3;
			m_curNumBibIndices+=6;
		}		
	}
	IndexBufferExceptionFunc();
	} catch(...) {
		IndexBufferExceptionFunc();
	}
}

//-----------------------------------------------------------------------------
//         Public Functions                                                
//-----------------------------------------------------------------------------

//=============================================================================
// W3DBibBuffer::~W3DBibBuffer
//=============================================================================
/** Destructor. Releases w3d assets. */
//=============================================================================
W3DBibBuffer::~W3DBibBuffer(void)
{
	freeBibBuffers();
	REF_PTR_RELEASE(m_bibTexture);
	REF_PTR_RELEASE(m_highlightBibTexture);
}

//=============================================================================
// W3DBibBuffer::W3DBibBuffer
//=============================================================================
/** Constructor. Sets m_initialized to true if it finds the w3d models it needs
for the bibs. */
//=============================================================================
W3DBibBuffer::W3DBibBuffer(void)
{
	m_initialized = false;
	m_vertexBib = NULL;
	m_indexBib = NULL;
	m_bibTexture = NULL;
	m_curNumBibVertices=0;
	m_curNumBibIndices=0;
	clearAllBibs();
	m_indexBibSize = INITIAL_BIB_INDEX;
	m_vertexBibSize = INITIAL_BIB_VERTEX;
	allocateBibBuffers();

	m_bibTexture = NEW_REF(TextureClass, ("TBBib.tga"));
	m_highlightBibTexture = NEW_REF(TextureClass, ("TBRedBib.tga"));
	m_bibTexture->Get_Filter().Set_U_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
	m_bibTexture->Get_Filter().Set_V_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
	m_highlightBibTexture->Get_Filter().Set_U_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
	m_highlightBibTexture->Get_Filter().Set_V_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
	m_initialized = true;
}


//=============================================================================
// W3DBibBuffer::freeBibBuffers
//=============================================================================
/** Frees the index and vertex buffers. */
//=============================================================================
void W3DBibBuffer::freeBibBuffers(void)
{
	REF_PTR_RELEASE(m_vertexBib);
	REF_PTR_RELEASE(m_indexBib);
}

//=============================================================================
// W3DBibBuffer::allocateBibBuffers
//=============================================================================
/** Allocates the index and vertex buffers. */
//=============================================================================
void W3DBibBuffer::allocateBibBuffers(void)
{
	m_vertexBib=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZDUV1,m_vertexBibSize+4,DX8VertexBufferClass::USAGE_DYNAMIC));
	m_indexBib=NEW_REF(DX8IndexBufferClass,(m_indexBibSize+4, DX8IndexBufferClass::USAGE_DYNAMIC));
	m_curNumBibVertices=0;
	m_curNumBibIndices=0;
}

//=============================================================================
// W3DBibBuffer::clearAllBibs
//=============================================================================
/** Removes all bibs. */
//=============================================================================
void W3DBibBuffer::clearAllBibs(void)
{
	m_numBibs=0;
	m_anythingChanged = true;
/* test bib
	Vector3 corners[4];
	corners[0].Set(0, 0, 20);
	corners[1].Set(100, 0, 20);
	corners[2].Set(100,100,20);
	corners[3].Set(0,100,20);
	addBib(corners, 1, false);
*/
}

//=============================================================================
// W3DBibBuffer::removeHighlighting
//=============================================================================
/** Clears highlighting flag.   */
//=============================================================================
void W3DBibBuffer::removeHighlighting(void)
{
	Int bibIndex;
	for (bibIndex=0; bibIndex<m_numBibs; bibIndex++) {
		m_bibs[bibIndex].m_highlight = false;
	}
}

//=============================================================================
// W3DBibBuffer::addBib
//=============================================================================
/** Adds a bib.   */
//=============================================================================
void W3DBibBuffer::addBib(Vector3 corners[4], ObjectID id, Bool highlight)
{
	Int bibIndex;
	for (bibIndex=0; bibIndex<m_numBibs; bibIndex++) {
		if (!m_bibs[bibIndex].m_unused && m_bibs[bibIndex].m_objectID == id) {
			break;
		}
	}
	if (bibIndex==m_numBibs) {
		for (bibIndex=0; bibIndex<m_numBibs; bibIndex++) {
			if (m_bibs[bibIndex].m_unused) {
				break;
			}
		}
	}
	if (bibIndex==m_numBibs) {
		if (m_numBibs >= MAX_BIBS) {
			return;  
		}
		m_numBibs++;
	}
	m_anythingChanged = true;
	m_bibs[bibIndex].m_corners[0] = corners[0];
	m_bibs[bibIndex].m_corners[1] = corners[1];
	m_bibs[bibIndex].m_corners[2] = corners[2];
	m_bibs[bibIndex].m_corners[3] = corners[3];
	m_bibs[bibIndex].m_highlight = highlight;
	m_bibs[bibIndex].m_color = 0; // for now.
	m_bibs[bibIndex].m_unused = false; // for now.
	m_bibs[bibIndex].m_objectID = id;
	m_bibs[bibIndex].m_drawableID = INVALID_DRAWABLE_ID;
}

//=============================================================================
// W3DBibBuffer::addBib
//=============================================================================
/** Adds a bib.   */
//=============================================================================
void W3DBibBuffer::addBibDrawable(Vector3 corners[4], DrawableID id, Bool highlight)
{
	Int bibIndex;
	for (bibIndex=0; bibIndex<m_numBibs; bibIndex++) {
		if (!m_bibs[bibIndex].m_unused && m_bibs[bibIndex].m_drawableID == id) {
			break;
		}
	}
	if (bibIndex==m_numBibs) {
		for (bibIndex=0; bibIndex<m_numBibs; bibIndex++) {
			if (m_bibs[bibIndex].m_unused) {
				break;
			}
		}
	}
	if (bibIndex==m_numBibs) {
		if (m_numBibs >= MAX_BIBS) {
			return;  
		}
		m_numBibs++;
	}
	m_anythingChanged = true;
	m_bibs[bibIndex].m_corners[0] = corners[0];
	m_bibs[bibIndex].m_corners[1] = corners[1];
	m_bibs[bibIndex].m_corners[2] = corners[2];
	m_bibs[bibIndex].m_corners[3] = corners[3];
	m_bibs[bibIndex].m_highlight = highlight;
	m_bibs[bibIndex].m_color = 0; // for now.
	m_bibs[bibIndex].m_unused = false; // for now.
	m_bibs[bibIndex].m_objectID = INVALID_ID;
	m_bibs[bibIndex].m_drawableID = id;
}

//=============================================================================
// W3DBibBuffer::removeBib
//=============================================================================
/** Removes a bib.  */
//=============================================================================
void W3DBibBuffer::removeBib(ObjectID id)
{
	Int bibIndex;
	for (bibIndex=0; bibIndex<m_numBibs; bibIndex++) {
		if (m_bibs[bibIndex].m_objectID == id) {
			m_bibs[bibIndex].m_unused = true;
			m_bibs[bibIndex].m_objectID = INVALID_ID;
			m_bibs[bibIndex].m_drawableID = INVALID_DRAWABLE_ID;
			m_anythingChanged = true;
		}
	}
}

//=============================================================================
// W3DBibBuffer::removeBib
//=============================================================================
/** Removes a bib.  */
//=============================================================================
void W3DBibBuffer::removeBibDrawable(DrawableID id)
{
	Int bibIndex;
	for (bibIndex=0; bibIndex<m_numBibs; bibIndex++) {
		if (m_bibs[bibIndex].m_drawableID == id) {
			m_bibs[bibIndex].m_unused = true;
			m_bibs[bibIndex].m_objectID = INVALID_ID;
			m_bibs[bibIndex].m_drawableID = INVALID_DRAWABLE_ID;
			m_anythingChanged = true;
		}
	}
}


//=============================================================================
// W3DBibBuffer::drawBibs
//=============================================================================
/** Draws the bibs.  Uses camera to cull. */
//=============================================================================
void W3DBibBuffer::renderBibs()
{

	loadBibsInVertexAndIndexBuffers();

	if (m_curNumBibIndices == 0) {
		return;
	}
	// Setup the vertex buffer, shader & texture.
	DX8Wrapper::Set_Index_Buffer(m_indexBib,0);
	DX8Wrapper::Set_Vertex_Buffer(m_vertexBib);
	DX8Wrapper::Set_Shader(detailAlphaShader);
	if (m_curNumNormalBibIndices) {
		DX8Wrapper::Set_Texture(0,m_bibTexture);
		DX8Wrapper::Draw_Triangles(	0, m_curNumNormalBibIndices/3, 0,	m_curNumNormalBibVertex);
	}
	if (m_curNumBibIndices>m_curNumNormalBibIndices) {
		DX8Wrapper::Set_Texture(0,m_highlightBibTexture);
		DX8Wrapper::Draw_Triangles(	m_curNumNormalBibIndices, (m_curNumBibIndices-m_curNumNormalBibIndices)/3, 
						m_curNumNormalBibVertex,	m_curNumBibVertices-m_curNumNormalBibVertex);
	}
}


