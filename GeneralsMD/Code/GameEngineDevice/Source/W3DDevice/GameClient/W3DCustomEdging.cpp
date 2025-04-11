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

// FILE: W3DCustomEdging.cpp ////////////////////////////////////////////////
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
// File name: W3DCustomEdging.cpp
//
// Created:   John Ahlquist, May 2001
//
// Desc:      Draw buffer to handle all the custom tile edges in a scene.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//         Includes                                                      
//-----------------------------------------------------------------------------
#include "W3DDevice/GameClient/W3DCustomEdging.h"

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
#define SC_ALPHA_DETAIL ( SHADE_CNST(ShaderClass::PASS_LEQUAL, ShaderClass::DEPTH_WRITE_DISABLE, ShaderClass::COLOR_WRITE_ENABLE, ShaderClass::SRCBLEND_SRC_ALPHA, \
	ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA, ShaderClass::FOG_DISABLE, ShaderClass::GRADIENT_MODULATE, ShaderClass::SECONDARY_GRADIENT_DISABLE, ShaderClass::TEXTURING_ENABLE, \
	ShaderClass::ALPHATEST_ENABLE, ShaderClass::CULL_MODE_DISABLE, \
	ShaderClass::DETAILCOLOR_DISABLE, ShaderClass::DETAILALPHA_DISABLE) )

static ShaderClass detailAlphaTestShader(SC_ALPHA_DETAIL);


#define SC_NO_ALPHA ( SHADE_CNST(ShaderClass::PASS_ALWAYS, ShaderClass::DEPTH_WRITE_DISABLE, ShaderClass::COLOR_WRITE_ENABLE, ShaderClass::SRCBLEND_ONE, \
	ShaderClass::DSTBLEND_ZERO, ShaderClass::FOG_DISABLE, ShaderClass::GRADIENT_MODULATE, ShaderClass::SECONDARY_GRADIENT_DISABLE, ShaderClass::TEXTURING_ENABLE, \
	ShaderClass::ALPHATEST_DISABLE, ShaderClass::CULL_MODE_DISABLE, \
	ShaderClass::DETAILCOLOR_DISABLE, ShaderClass::DETAILALPHA_DISABLE) )

static ShaderClass detailShader(SC_NO_ALPHA);

#define SC_DETAIL_BLEND ( SHADE_CNST(ShaderClass::PASS_LEQUAL, ShaderClass::DEPTH_WRITE_ENABLE, ShaderClass::COLOR_WRITE_ENABLE, ShaderClass::SRCBLEND_ONE, \
	ShaderClass::DSTBLEND_ZERO, ShaderClass::FOG_DISABLE, ShaderClass::GRADIENT_MODULATE, ShaderClass::SECONDARY_GRADIENT_DISABLE, ShaderClass::TEXTURING_ENABLE, \
	ShaderClass::ALPHATEST_DISABLE, ShaderClass::CULL_MODE_ENABLE, ShaderClass::DETAILCOLOR_SCALE, ShaderClass::DETAILALPHA_DISABLE) )

static ShaderClass detailOpaqueShader(SC_DETAIL_BLEND);

/*
#define SC_ALPHA_MIRROR ( SHADE_CNST(ShaderClass::PASS_LEQUAL, ShaderClass::DEPTH_WRITE_DISABLE, ShaderClass::COLOR_WRITE_ENABLE, ShaderClass::SRCBLEND_SRC_ALPHA, \
	ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA, ShaderClass::FOG_DISABLE, ShaderClass::GRADIENT_MODULATE, ShaderClass::SECONDARY_GRADIENT_DISABLE, ShaderClass::TEXTURING_ENABLE, \
	ShaderClass::DETAILCOLOR_DISABLE, ShaderClass::DETAILALPHA_DISABLE, ShaderClass::ALPHATEST_DISABLE, ShaderClass::CULL_MODE_DISABLE, \
	ShaderClass::DETAILCOLOR_DISABLE, ShaderClass::DETAILALPHA_DISABLE) )

static ShaderClass mirrorAlphaShader(SC_ALPHA_DETAIL);

// ShaderClass::PASS_ALWAYS, 

#define SC_ALPHA_2D ( SHADE_CNST(PASS_ALWAYS, DEPTH_WRITE_DISABLE, COLOR_WRITE_ENABLE, \
	SRCBLEND_SRC_ALPHA, DSTBLEND_ONE_MINUS_SRC_ALPHA, FOG_DISABLE, GRADIENT_DISABLE, \
	SECONDARY_GRADIENT_DISABLE, TEXTURING_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE, \
	ALPHATEST_DISABLE, CULL_MODE_ENABLE, DETAILCOLOR_DISABLE, DETAILALPHA_DISABLE) )
ShaderClass ShaderClass::_PresetAlpha2DShader(SC_ALPHA_2D);
*/
//-----------------------------------------------------------------------------
//         Private Functions                                               
//-----------------------------------------------------------------------------

//=============================================================================
// W3DCustomEdging::loadEdgingsInVertexAndIndexBuffers
//=============================================================================
/** Loads the trees into the vertex buffer for drawing. */
//=============================================================================
void W3DCustomEdging::loadEdgingsInVertexAndIndexBuffers(WorldHeightMap *pMap, Int minX, Int maxX, Int minY, Int maxY)
{
	if (!m_indexEdging || !m_vertexEdging || !m_initialized) {
		return;
	}
	if (!m_anythingChanged) {
		return;
	}
	m_anythingChanged = false;
	m_curNumEdgingVertices = 0;
	m_curNumEdgingIndices = 0;
	VertexFormatXYZDUV2 *vb;
	UnsignedShort *ib;
	// Lock the buffers.
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexEdging);
	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexEdging);
	vb=(VertexFormatXYZDUV2*)lockVtxBuffer.Get_Vertex_Array();
	ib = lockIdxBuffer.Get_Index_Array();

	UnsignedShort *curIb = ib;

	VertexFormatXYZDUV2 *curVb = vb;

	if (minX<0) minX = 0;
	if (minY<0) minY = 0;
	if (maxX >= pMap->getXExtent()) maxX = pMap->getXExtent()-1;
	if (maxY >= pMap->getYExtent()) maxY = pMap->getYExtent()-1;
	Int row;
	Int column;
	for (row=minY; row<maxY-1; row++) {
		for (column = minX; column < maxX-1; column++) {
			Int cellNdx = column+row*pMap->getXExtent();
			Int blend = pMap->m_blendTileNdxes[cellNdx];
			if (blend == 0) continue; // no blend.

			if (pMap->m_blendedTiles[blend].customBlendEdgeClass<0) continue; // alpha blend.
			Int i, j;
			Real uOffset;
			Real vOffset;

			if (pMap->m_blendedTiles[blend].horiz) {
				uOffset = 0;
				vOffset = 0.25f * (1 + (row&1));
				if (pMap->m_blendedTiles[blend].inverted) {
					uOffset = 0.75f;
				}
			} else if (pMap->m_blendedTiles[blend].vert) {
				vOffset = 0.75;
				uOffset = 0.25f * (1 + (column&1));
				if (pMap->m_blendedTiles[blend].inverted) {
					vOffset = 0.0f;
				}
			} else if (pMap->m_blendedTiles[blend].rightDiagonal) {
				if (pMap->m_blendedTiles[blend].longDiagonal) {
					vOffset = 0.25;
					uOffset = 0.5;
					if (pMap->m_blendedTiles[blend].inverted) {
						uOffset = 0.5f;
						vOffset = 0.5f;
					}
				} else {
					vOffset = .75;
					uOffset = 0;
					if (pMap->m_blendedTiles[blend].inverted) {
						uOffset = 0.0f;
						vOffset = 0.0f;
					}
				}
			} else if (pMap->m_blendedTiles[blend].leftDiagonal) {
				if (pMap->m_blendedTiles[blend].longDiagonal) {
					uOffset = 0.25f;
					vOffset = 0.25f;
					if (pMap->m_blendedTiles[blend].inverted) {
						uOffset = 0.25f;
						vOffset = 0.5f;
					}
				} else {
					vOffset = 0.75;
					uOffset = 0.75f;
					if (pMap->m_blendedTiles[blend].inverted) {
						uOffset = 0.75f;
						vOffset = 0.0f;
					}
				}
			}	else {
				continue;
			}
			Region2D range;
			pMap->getUVForBlend(pMap->m_blendedTiles[blend].customBlendEdgeClass, &range);

			uOffset = range.lo.x + range.width()*uOffset;
			vOffset = range.lo.y + range.height()*vOffset;

			UnsignedByte alpha[4];
			float UA[4], VA[4];
			Bool flipForBlend;
			pMap->getAlphaUVData(column-pMap->getDrawOrgX(), row-pMap->getDrawOrgY(), UA, VA, alpha, &flipForBlend, false);


			Int startVertex = m_curNumEdgingVertices;
			for (j=0; j<2; j++) {
				for (i=0; i<2; i++) {
					if (m_curNumEdgingVertices >= MAX_EDGE_VERTEX) return;
					cellNdx = column+i+(row+j)*pMap->getXExtent();

					Int diffuse = TheTerrainRenderObject->getStaticDiffuse(column+i, row+j);
					curVb->diffuse = 0x80000000 + (diffuse&0x00FFFFFF); // set alpha to 5b.
					Real theZ; 
					theZ = ((float)pMap->getDataPtr()[cellNdx])*MAP_HEIGHT_SCALE;
					Real X = (column+i)*MAP_XY_FACTOR; 
					Real Y = (row+j)*MAP_XY_FACTOR;
					curVb->u2 = uOffset + i*0.25f*range.width();
					curVb->v2 = vOffset + (1-j)*0.25f*range.height();
					Int ndx;
					if (j==0) ndx=i;
					if (j==1) ndx = 3-i;
					curVb->u1 = UA[ndx];
					curVb->v1 = VA[ndx];
					curVb->x = X;
					curVb->y = Y;
					curVb->z = theZ;
					curVb++;
					m_curNumEdgingVertices++;
				}
			}
			Int yOffset = 2;
			if (m_curNumEdgingIndices+6 > MAX_EDGE_INDEX) return;
#ifdef FLIP_TRIANGLES // jba - reduces "diamonding" in some cases, not others.  Better cliffs, though.
			if (flipForBlend) {
				*curIb++ = startVertex + 1;
 				*curIb++ = startVertex + yOffset;
				*curIb++ = startVertex ;
 				*curIb++ = startVertex + 1;
 				*curIb++ = startVertex + 1+yOffset;
				*curIb++ = startVertex + yOffset;
			}	
			else 
#endif
			{
				*curIb++ = startVertex;
				*curIb++ = startVertex + 1+yOffset;
				*curIb++ = startVertex + yOffset;
				*curIb++ = startVertex ;
				*curIb++ = startVertex + 1;
				*curIb++ = startVertex + 1+yOffset;
			}
			m_curNumEdgingIndices+=6;
		}
	}
	m_anythingChanged = false;
}

//-----------------------------------------------------------------------------
//         Public Functions                                                
//-----------------------------------------------------------------------------

//=============================================================================
// W3DCustomEdging::~W3DCustomEdging
//=============================================================================
/** Destructor. Releases w3d assets. */
//=============================================================================
W3DCustomEdging::~W3DCustomEdging(void)
{
	freeEdgingBuffers();
}

//=============================================================================
// W3DCustomEdging::W3DCustomEdging
//=============================================================================
/** Constructor. Sets m_initialized to true if it finds the w3d models it needs
for the trees. */
//=============================================================================
W3DCustomEdging::W3DCustomEdging(void)
{
	m_initialized = false;
	m_vertexEdging = NULL;
	m_indexEdging = NULL;
	clearAllEdging();
	allocateEdgingBuffers();
	m_initialized = true;
}


//=============================================================================
// W3DCustomEdging::freeEdgingBuffers
//=============================================================================
/** Frees the index and vertex buffers. */
//=============================================================================
void W3DCustomEdging::freeEdgingBuffers(void)
{
	REF_PTR_RELEASE(m_vertexEdging);
	REF_PTR_RELEASE(m_indexEdging);
}

//=============================================================================
// W3DCustomEdging::allocateEdgingBuffers
//=============================================================================
/** Allocates the index and vertex buffers. */
//=============================================================================
void W3DCustomEdging::allocateEdgingBuffers(void)
{
	m_vertexEdging=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZDUV2,MAX_EDGE_VERTEX+4,DX8VertexBufferClass::USAGE_DYNAMIC));
	m_indexEdging=NEW_REF(DX8IndexBufferClass,(2*MAX_EDGE_INDEX+4, DX8IndexBufferClass::USAGE_DYNAMIC));
	m_curNumEdgingVertices=0;
	m_curNumEdgingIndices=0;
	//m_edgeTexture = MSGNEW("TextureClass") TextureClass("EdgingTemplate.tga","EdgingTemplate.tga", MIP_LEVELS_3);
}

//=============================================================================
// W3DCustomEdging::clearAllEdging
//=============================================================================
/** Removes all trees. */
//=============================================================================
void W3DCustomEdging::clearAllEdging(void)
{
	m_curNumEdgingVertices=0;				  
	m_curNumEdgingIndices=0;
	m_anythingChanged = true;
}




//=============================================================================
// W3DCustomEdging::drawEdging
//=============================================================================
/** Draws the trees.  Uses camera to cull. */
//=============================================================================
void W3DCustomEdging::drawEdging(WorldHeightMap *pMap, Int minX, Int maxX, Int minY, Int maxY,  
		TextureClass * terrainTexture, TextureClass * cloudTexture, TextureClass * noiseTexture) 
{
	static Bool foo = false;
	if (foo) {
		return;
	}
	//m_anythingChanged = true;
	loadEdgingsInVertexAndIndexBuffers(pMap, minX, maxX, minY, maxY);

	if (m_curNumEdgingIndices == 0) {
		return;
	}
	TextureClass *edgeTex = pMap->getEdgeTerrainTexture();
	// Setup the vertex buffer, shader & texture.
	DX8Wrapper::Set_Index_Buffer(m_indexEdging,0);
	DX8Wrapper::Set_Vertex_Buffer(m_vertexEdging);
	DX8Wrapper::Set_Shader(detailAlphaTestShader);
#ifdef _DEBUG
	//DX8Wrapper::Set_Shader(detailShader); // shows clipping.
#endif	
	
	DX8Wrapper::Set_Texture(0,terrainTexture);
	DX8Wrapper::Set_Texture(1,edgeTex);
	DX8Wrapper::Apply_Render_State_Changes();

	DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHAREF,0x7B);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHAFUNC,D3DCMP_LESSEQUAL);	//pass pixels who's alpha is not zero
	DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHATESTENABLE, true);	//test pixels if transparent(clipped) before rendering.
	DX8Wrapper::Draw_Triangles(	m_curEdgingIndexOffset, m_curNumEdgingIndices/3, 0,	m_curNumEdgingVertices);

	DX8Wrapper::Set_Texture(0,edgeTex);
	DX8Wrapper::Set_Texture(1, NULL);
	// Draw the custom edge.
	DX8Wrapper::Apply_Render_State_Changes();

	DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHAREF,0x84);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHAFUNC,D3DCMP_GREATEREQUAL);	//pass pixels who's alpha is not zero
	DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHATESTENABLE, true);	//test pixels if transparent(clipped) before rendering.
	DX8Wrapper::Draw_Triangles(	m_curEdgingIndexOffset, m_curNumEdgingIndices/3, 0,	m_curNumEdgingVertices);

#if 0 // Dumps out unmasked data.
	DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE,false);
	DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHATESTENABLE, false);	//test pixels if transparent(clipped) before rendering.
	DX8Wrapper::Draw_Triangles(	m_curEdgingIndexOffset, m_curNumEdgingIndices/3, 0,	m_curNumEdgingVertices);
#endif
	DX8Wrapper::Set_Texture(1, NULL);
	if (cloudTexture) {
		DX8Wrapper::Set_Shader(detailOpaqueShader);
		DX8Wrapper::Apply_Render_State_Changes();
		DX8Wrapper::Set_Texture(1,edgeTex);
		DX8Wrapper::Apply_Render_State_Changes();
		DX8Wrapper::Set_Texture(0,cloudTexture);
		DX8Wrapper::Apply_Render_State_Changes();
#if 1
		DX8Wrapper::Set_DX8_Texture_Stage_State( 0, D3DTSS_ALPHAARG1,   D3DTA_CURRENT );
		DX8Wrapper::Set_DX8_Texture_Stage_State( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG1 );

		DX8Wrapper::Set_DX8_Texture_Stage_State( 1, D3DTSS_COLORARG1, D3DTA_CURRENT );
		DX8Wrapper::Set_DX8_Texture_Stage_State( 1, D3DTSS_COLORARG2, D3DTA_TEXTURE );
		DX8Wrapper::Set_DX8_Texture_Stage_State( 1, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );
		DX8Wrapper::Set_DX8_Texture_Stage_State( 1, D3DTSS_ALPHAARG1,   D3DTA_CURRENT );
		DX8Wrapper::Set_DX8_Texture_Stage_State( 1, D3DTSS_ALPHAARG2,   D3DTA_TEXTURE );
		DX8Wrapper::Set_DX8_Texture_Stage_State( 1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
		DX8Wrapper::Set_DX8_Texture_Stage_State( 1, D3DTSS_TEXCOORDINDEX, 1 );
#endif
		DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHAREF,0x80);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHAFUNC,D3DCMP_NOTEQUAL);	//pass pixels who's alpha is not zero
		DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHATESTENABLE, true);	//test pixels if transparent(clipped) before rendering.
		DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE,true);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND,D3DBLEND_DESTCOLOR);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND,D3DBLEND_ZERO);
		DX8Wrapper::Draw_Triangles(	m_curEdgingIndexOffset, m_curNumEdgingIndices/3, 0,	m_curNumEdgingVertices);
	}
	if (noiseTexture) {
		DX8Wrapper::Set_Texture(1, NULL);
		DX8Wrapper::Set_Texture(0,noiseTexture);
		DX8Wrapper::Apply_Render_State_Changes();
		DX8Wrapper::Set_Texture(1,edgeTex);
		DX8Wrapper::Apply_Render_State_Changes();

		DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHAREF,0x80);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHAFUNC,D3DCMP_NOTEQUAL);	//pass pixels who's alpha is not zero
		DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHATESTENABLE, true);	//test pixels if transparent(clipped) before rendering.
		DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE,true);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_SRCBLEND,D3DBLEND_DESTCOLOR);
		DX8Wrapper::Set_DX8_Render_State(D3DRS_DESTBLEND,D3DBLEND_ZERO);
		DX8Wrapper::Draw_Triangles(	m_curEdgingIndexOffset, m_curNumEdgingIndices/3, 0,	m_curNumEdgingVertices);
	}
}


