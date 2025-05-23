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

// FILE: Heightmap.cpp ////////////////////////////////////////////////
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
// File name: Heightmap.cpp
//
// Created:   Mark W., John Ahlquist, April/May 2001
//
// Desc:      Draw the terrain and scorchmarks in a scene.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//         Includes                                                      
//-----------------------------------------------------------------------------
#include "W3DDevice/GameClient/HeightMap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assetmgr.h>
#include <texture.h>
#include <tri.h>
#include <colmath.h>
#include <coltest.h>
#include <rinfo.h>
#include <camera.h>
#include <d3dx8core.h>
#include "Common/GlobalData.h"
#include "Common/PerfTimer.h"

#include "GameClient/TerrainVisual.h"
#include "GameClient/View.h"
#include "GameClient/Water.h"

#include "GameLogic/AIPathfind.h"
#include "GameLogic/TerrainLogic.h"
#include "W3DDevice/GameClient/TerrainTex.h"
#include "W3DDevice/GameClient/W3DDynamicLight.h"
#include "W3DDevice/GameClient/W3DScene.h"
#include "W3DDevice/GameClient/W3DTerrainTracks.h"
#include "W3DDevice/GameClient/W3DBibBuffer.h"
#include "W3DDevice/GameClient/W3DTreeBuffer.h"
#include "W3DDevice/GameClient/W3DRoadBuffer.h"
#include "W3DDevice/GameClient/W3DBridgeBuffer.h"
#include "W3DDevice/GameClient/W3DWaypointBuffer.h"
#include "W3DDevice/GameClient/W3DCustomEdging.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "W3DDevice/GameClient/W3DShaderManager.h"
#include "W3DDevice/GameClient/W3DShadow.h"
#include "W3DDevice/GameClient/W3DWater.h"
#include "W3DDevice/GameClient/W3DShroud.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/light.h"
#include "WW3D2/scene.h"
#include "W3DDevice/GameClient/W3DPoly.h"
#include "W3DDevice/GameClient/W3DCustomScene.h"

#include "Common/PerfTimer.h"
#include "Common/UnitTimings.h" //Contains the DO_UNIT_TIMINGS define jba.		 

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

#define no_OPTIMIZED_HEIGHTMAP_LIGHTING	01
// Doesn't work well.  jba.

//-----------------------------------------------------------------------------
//         Private Data                                                     
//-----------------------------------------------------------------------------
#define SC_DETAIL_BLEND ( SHADE_CNST(ShaderClass::PASS_LEQUAL, ShaderClass::DEPTH_WRITE_ENABLE, ShaderClass::COLOR_WRITE_ENABLE, ShaderClass::SRCBLEND_ONE, \
	ShaderClass::DSTBLEND_ZERO, ShaderClass::FOG_DISABLE, ShaderClass::GRADIENT_MODULATE, ShaderClass::SECONDARY_GRADIENT_DISABLE, ShaderClass::TEXTURING_ENABLE, \
	ShaderClass::ALPHATEST_DISABLE, ShaderClass::CULL_MODE_ENABLE, ShaderClass::DETAILCOLOR_SCALE, ShaderClass::DETAILALPHA_DISABLE) )

static ShaderClass detailOpaqueShader(SC_DETAIL_BLEND);

//-----------------------------------------------------------------------------
//         Global Functions & Data                                              
//-----------------------------------------------------------------------------
/// The one-of for the terrain rendering object.
HeightMapRenderObjClass *TheTerrainRenderObject=NULL;

/** Entry point so that trees can be drawn at the appropriate point in the rendering pipe for 
    transparent objects. */
void DoTrees(RenderInfoClass & rinfo)
{
	if (TheTerrainRenderObject) {
		TheTerrainRenderObject->renderTrees(&rinfo.Camera);
	}
}

void oversizeTheTerrain(Int amount)
{
	if (TheTerrainRenderObject) 
	{
		TheTerrainRenderObject->oversizeTerrain(amount);
	}
}

#define DEFAULT_MAX_FRAME_EXTRABLEND_TILES		256	//default number of terrain tiles rendered per call (must fit in one VB)
#define DEFAULT_MAX_MAP_EXTRABLEND_TILES		2048	//default size of array allocated to hold all map extra blend tiles.
#define DEFAULT_MAX_BATCH_SHORELINE_TILES		512	//maximum number of terrain tiles rendered per call (must fit in one VB)
#define DEFAULT_MAX_MAP_SHORELINE_TILES		4096	//default size of array allocated to hold all map shoreline tiles.

#define ADJUST_FROM_INDEX_TO_REAL(k) ((k-m_map->getBorderSize())*MAP_XY_FACTOR)
inline Int IABS(Int x) {	if (x>=0) return x; return -x;};

//-----------------------------------------------------------------------------
//         Private Functions                                               
//-----------------------------------------------------------------------------

//=============================================================================
// HeightMapRenderObjClass::freeMapResources
//=============================================================================
/** Frees the w3d resources used to draw the terrain. */
//=============================================================================
Int HeightMapRenderObjClass::freeMapResources(void)
{
#ifdef DO_SCORCH
	freeScorchBuffers();
#endif
	REF_PTR_RELEASE(m_indexBuffer);
	if (m_vertexBufferTiles) {
		for (int i=0; i<m_numVertexBufferTiles; i++)
			REF_PTR_RELEASE(m_vertexBufferTiles[i]);
		delete[] m_vertexBufferTiles;
		m_vertexBufferTiles = NULL;
	}
	if (m_vertexBufferBackup) {
		for (int i=0; i<m_numVertexBufferTiles; i++)
			delete[] m_vertexBufferBackup[i];
		delete[] m_vertexBufferBackup;
		m_vertexBufferBackup = NULL;
	}
	m_numVertexBufferTiles = 0;
#ifdef PRE_TRANSFORM_VERTEX
	if (m_xformedVertexBuffer) {
		for (int i=0; i<m_numVertexBufferTiles; i++)
			m_xformedVertexBuffer[i]->Release();
		delete[] m_xformedVertexBuffer;
		m_xformedVertexBuffer = NULL;
	}
#endif
	REF_PTR_RELEASE(m_vertexMaterialClass);
	REF_PTR_RELEASE(m_stageZeroTexture);
	REF_PTR_RELEASE(m_stageOneTexture);
	REF_PTR_RELEASE(m_stageTwoTexture);
	REF_PTR_RELEASE(m_stageThreeTexture);
	REF_PTR_RELEASE(m_destAlphaTexture);
	REF_PTR_RELEASE(m_map);

	return 0;
}

//=============================================================================
// HeightMapRenderObjClass::doTheLight
//=============================================================================
/** Calculates the diffuse lighting for a vertex in the terrain, taking all of the
static lights into account as well.  It is possible to just use the normal in the 
vertex and let D3D do the lighting, but it is slower to render, and can only 
handle 4 lights at this point. */
//=============================================================================
void HeightMapRenderObjClass::doTheLight(VERTEX_FORMAT *vb, Vector3*light, Vector3*normal, RefRenderObjListIterator *pLightsIterator, UnsignedByte alpha)
{
#ifdef USE_NORMALS
	vb->nx = normal->X;
	vb->ny = normal->Y;
	vb->nz = normal->Z;
#else
	Real shadeR, shadeG, shadeB;
	Real shade;
	shadeR = TheGlobalData->m_terrainAmbient[0].red;	//only the first terrain light contributes to ambient
	shadeG = TheGlobalData->m_terrainAmbient[0].green;
	shadeB = TheGlobalData->m_terrainAmbient[0].blue;

	if (pLightsIterator) {
		for (pLightsIterator->First(); !pLightsIterator->Is_Done(); pLightsIterator->Next())
		{		
			LightClass *pLight = (LightClass*)pLightsIterator->Peek_Obj();
			Vector3 lightDirection(vb->x, vb->y, vb->z);
			Real factor = 1.0f;
			switch(pLight->Get_Type()) {
			case LightClass::POINT:
			case LightClass::SPOT: {
					Vector3 lightLoc = pLight->Get_Position();
					lightDirection -= lightLoc;
					double range, midRange;
					pLight->Get_Far_Attenuation_Range(midRange, range);
					if (vb->x < lightLoc.X-range) continue;
					if (vb->x > lightLoc.X+range) continue;
					if (vb->y < lightLoc.Y-range) continue;
					if (vb->y > lightLoc.Y+range) continue;
					Real dist = lightDirection.Length();
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
				} 
				break;
			case LightClass::DIRECTIONAL:
				lightDirection = pLight->Get_Transform().Get_Z_Vector();
				factor = 1.0;
				break;
			};
			lightDirection.Normalize();
			Vector3 lightRay(-lightDirection.X, -lightDirection.Y, -lightDirection.Z);
			shade = Vector3::Dot_Product(lightRay, *normal); 
			shade *= factor;
			Vector3 diffuse;
			pLight->Get_Diffuse(&diffuse);
			Vector3 ambient;
			pLight->Get_Ambient(&ambient);
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
	// Add in global diffuse value.
	const RGBColor *terrainDiffuse;
	for (Int lightIndex=0; lightIndex < TheGlobalData->m_numGlobalLights; lightIndex++)
	{
		shade = Vector3::Dot_Product(light[lightIndex], *normal); 
		if (shade > 1.0) shade = 1.0;
		if(shade < 0.0f) shade = 0.0f;
		terrainDiffuse=&TheGlobalData->m_terrainDiffuse[lightIndex];
		shadeR += shade*terrainDiffuse->red;
		shadeG += shade*terrainDiffuse->green;
		shadeB += shade*terrainDiffuse->blue;
	}

	if (shadeR > 1.0) shadeR = 1.0;
	if(shadeR < 0.0f) shadeR = 0.0f;
	if (shadeG > 1.0) shadeG = 1.0;
	if(shadeG < 0.0f) shadeG = 0.0f;
	if (shadeB > 1.0) shadeB = 1.0;
	if(shadeB < 0.0f) shadeB = 0.0f;

	if (m_useDepthFade && vb->z <= TheGlobalData->m_waterPositionZ)
	{	//height is below water level
		//reduce lighting values based on light fall off as it travels through water.
		float depthScale = (1.4f - vb->z)/TheGlobalData->m_waterPositionZ;
		shadeR *= 1.0f - depthScale * (1.0f-m_depthFade.X);
		shadeG *= 1.0f - depthScale * (1.0f-m_depthFade.Y);
		shadeB *= 1.0f - depthScale * (1.0f-m_depthFade.Z);
	}

	shadeR*=255.0f;
	shadeG*=255.0f;
	shadeB*=255.0f;
	vb->diffuse = REAL_TO_INT(shadeB) | (REAL_TO_INT(shadeG) << 8) | (REAL_TO_INT(shadeR) << 16) | ((Int)alpha << 24);
#endif
}

//=============================================================================
// HeightMapRenderObjClass::doTheDynamicLight
//=============================================================================
/** Calculates the diffuse lighting as affected by dynamic lighting. */
//=============================================================================
UnsignedInt HeightMapRenderObjClass::doTheDynamicLight(VERTEX_FORMAT *vb, VERTEX_FORMAT *vbMirror, Vector3*light, Vector3*normal,  W3DDynamicLight *pLights[], Int numLights)
{
#ifdef USE_NORMALS
	return;
#else
	Real shadeR, shadeG, shadeB;
	Int diffuse = vbMirror->diffuse;
#ifdef RTS_DEBUG
	//vbMirror->diffuse += 30;	// Shows which vertexes are geting touched by dynamic light. debug only.
#endif
	
	// (gth) avoiding the extra divides (compiler unfortunately didn't do this automatically...)
	const float oo255 = (1.0f/255.0f);  
	shadeR = ((diffuse>>16)&0x00FF) * oo255;
	shadeG = ((diffuse>>8)&0x00FF) * oo255;
	shadeB = (diffuse&0x00FF) * oo255;

	Int alpha = (diffuse>>24)&0x00FF;
	Int k;
	for (k=0; k<numLights; k++) {
		W3DDynamicLight *pLight = pLights[k];
		if (!pLight->isEnabled()) {
			continue; // he is turned off.
		}
		Vector3 lightDirection(vbMirror->x, vbMirror->y, vbMirror->z);
		Real factor = 1.0f;
		switch(pLight->Get_Type()) {	  
		case LightClass::POINT:
		case LightClass::SPOT: {
				Vector3 lightLoc = pLight->Get_Position();
				lightDirection -= lightLoc;
				double range, midRange;
				pLight->Get_Far_Attenuation_Range(midRange, range);
				Real dist = lightDirection.Length();
				if (dist >= range) continue;
				if (midRange < 0.1) continue;
				factor = 1.0f - (dist - midRange) / (range - midRange);
				factor = WWMath::Clamp(factor,0.0f,1.0f);

				// (gth) normalize here since we have the length
				lightDirection /= dist;
			} 
			break;
		case LightClass::DIRECTIONAL:
			pLight->Get_Spot_Direction(lightDirection);
			factor = 1.0;
			break;
		};
		
		// (gth) unneeded due to above normalization
		//lightDirection.Normalize();

		Vector3 lightRay(-lightDirection.X, -lightDirection.Y, -lightDirection.Z);
		Real shade = Vector3::Dot_Product(lightRay, *normal); 
		shade *= factor;
		Vector3 diffuse;
		pLight->Get_Diffuse(&diffuse);
		Vector3 ambient;
		pLight->Get_Ambient(&ambient);
		if (shade > 1.0) shade = 1.0;
		if(shade < 0.0f) shade = 0.0f;
		shadeR += shade*diffuse.X;
		shadeG += shade*diffuse.Y;
		shadeB += shade*diffuse.Z;		
		shadeR += factor*ambient.X;
		shadeG += factor*ambient.Y;
		shadeB += factor*ambient.Z;		

	} 
	if (shadeR > 1.0) shadeR = 1.0;
	if(shadeR < 0.0f) shadeR = 0.0f;
	if (shadeG > 1.0) shadeG = 1.0;
	if(shadeG < 0.0f) shadeG = 0.0f;
	if (shadeB > 1.0) shadeB = 1.0;
	if(shadeB < 0.0f) shadeB = 0.0f;
	shadeR*=255.0f;
	shadeG*=255.0f;
	shadeB*=255.0f;

//	(gth) faster float to int conversion, return the result so we can re-use it.
//	vb->diffuse=REAL_TO_INT(shadeB) | (REAL_TO_INT(shadeG) << 8) | (REAL_TO_INT(shadeR) << 16) | ((int)alpha << 24);
	UnsignedInt light_val = WWMath::Float_To_Int_Chop(shadeB) | (WWMath::Float_To_Int_Chop(shadeG) << 8) | (WWMath::Float_To_Int_Chop(shadeR) << 16) | ((int)alpha << 24);
	vb->diffuse = light_val;
	return light_val;

#endif
}

//=============================================================================
// HeightMapRenderObjClass::getXWithOrigin
//=============================================================================
/** Gets the x index that corresponds to the data.  For example, if the columns
are shifted by 3, index 3 is actually the first row of polygons, or 0.  Yes it
is confusing, but it makes sliding the map 10x faster.  */
//=============================================================================
Int HeightMapRenderObjClass::getXWithOrigin(Int x)
{
	x -= m_originX;
	if (x<0) x+= m_x-1;
	if (x>= m_x-1) x-=m_x-1;
#ifdef RTS_DEBUG
	DEBUG_ASSERTCRASH (x>=0, ("X out of range."));
	DEBUG_ASSERTCRASH (x<m_x-1, ("X out of range."));
#endif
	if (x<0) x = 0;
	if (x>= m_x-1) x=m_x-1;
	return x;
}

//=============================================================================
// HeightMapRenderObjClass::getYWithOrigin
//=============================================================================
/** Gets the y index that corresponds to the data.  For example, if the rows
are shifted by 3, index 3 is actually the first row of polygons, or 0.  Yes it
is confusing, but it makes sliding the map 10x faster.  */
//=============================================================================
Int HeightMapRenderObjClass::getYWithOrigin(Int y)
{
	y -= m_originY;
	if (y<0) y+= m_y-1;
	if (y>= m_y-1) y-=m_y-1;
#ifdef RTS_DEBUG
	DEBUG_ASSERTCRASH (y>=0, ("Y out of range."));
	DEBUG_ASSERTCRASH (y<m_y-1, ("Y out of range."));
#endif
	if (y<0) y = 0;
	if (y>= m_y-1) y=m_y-1;
	return y;
}

//=============================================================================
// HeightMapRenderObjClass::updateVB
//=============================================================================
/** Update a rectangular block of the given Vertex Buffer. 
data is expected to be an array same dimensions as current heightmap
mapped into this VB.
*/
//=============================================================================
Int HeightMapRenderObjClass::updateVB(DX8VertexBufferClass	*pVB, char *data, Int x0, Int y0, Int x1, Int y1, Int originX, Int originY, WorldHeightMap *pMap, RefRenderObjListIterator *pLightsIterator)
{
	Int i,j;
	Vector3 lightRay[MAX_GLOBAL_LIGHTS];
	const Coord3D *lightPos;
	Int xCoord, yCoord;
	Int vn0,un0,vp1,up1;
	Vector3 l2r,n2f,normalAtTexel;
	Int	vertsPerRow=(VERTEX_BUFFER_TILE_LENGTH)*4;	//vertices per row of VB

	Int cellOffset = 1;
	if (m_halfResMesh) {
		cellOffset = 2;
	}

	REF_PTR_SET(m_map, pMap);	//update our heightmap pointer in case it changed since last call.
	if (m_vertexBufferTiles && pMap)
	{
#ifdef RTS_DEBUG
		assert(x0 >= originX && y0 >= originY && x1>x0 && y1>y0 && x1<=originX+VERTEX_BUFFER_TILE_LENGTH && y1<=originY+VERTEX_BUFFER_TILE_LENGTH);
#endif 

		DX8VertexBufferClass::WriteLockClass lockVtxBuffer(pVB);
		VERTEX_FORMAT *vbHardware = (VERTEX_FORMAT*)lockVtxBuffer.Get_Vertex_Array();
		VERTEX_FORMAT *vBase = (VERTEX_FORMAT*)data;
		// Note that we are building the vertex buffer data in the memory buffer, data.
		// At the bottom, we will copy the final vertex data for one cell into the 
		// hardware vertex buffer. 
		
		for (j=y0; j<y1; j++)
		{
			VERTEX_FORMAT *vb = vBase;
			if (m_halfResMesh) {
				if (j&1) continue;
				vb += ((j-originY)/2)*vertsPerRow/2;	//skip to correct row in vertex buffer
				vb += ((x0-originX)/2)*4;		//skip to correct vertex in row.
			} else {
				vb += (j-originY)*vertsPerRow;	//skip to correct row in vertex buffer
				vb += (x0-originX)*4;		//skip to correct vertex in row.
			}
			vn0 = getYWithOrigin(j)-cellOffset;
			if (vn0 < -pMap->getDrawOrgY())
				vn0=-pMap->getDrawOrgY();
			vp1 = getYWithOrigin(j+cellOffset)+cellOffset;
			if (vp1 >= pMap->getYExtent()-pMap->getDrawOrgY())
				vp1=pMap->getYExtent()-pMap->getDrawOrgY()-1;

			yCoord = getYWithOrigin(j)+pMap->getDrawOrgY();
			for (i=x0; i<x1; i++)
			{
				if (m_halfResMesh) {
					if (i&1) continue;
				}
				un0 = getXWithOrigin(i)-cellOffset;
				if (un0 < -pMap->getDrawOrgX())
					un0=-pMap->getDrawOrgX();
				up1 = getXWithOrigin(i+cellOffset)+cellOffset;
				if (up1 >= pMap->getXExtent()-pMap->getDrawOrgX())
					up1=pMap->getXExtent()-pMap->getDrawOrgX()-1;
				xCoord = getXWithOrigin(i)+pMap->getDrawOrgX();

				//update the 4 vertices in this block
				float U[4], V[4];
				UnsignedByte alpha[4];
				float UA[4], VA[4];
				Bool flipForBlend = false;			 // True if the blend needs the triangles flipped.

				if (pMap) {
					pMap->getUVData(getXWithOrigin(i),getYWithOrigin(j),U, V, m_halfResMesh);
					pMap->getAlphaUVData(getXWithOrigin(i),getYWithOrigin(j), UA, VA, alpha, &flipForBlend, m_halfResMesh);
				} 


				for (Int lightIndex=0; lightIndex < TheGlobalData->m_numGlobalLights; lightIndex++)
				{
					lightPos=&TheGlobalData->m_terrainLightPos[lightIndex];
					lightRay[lightIndex].Set(-lightPos->x,-lightPos->y,	-lightPos->z);
				}

				//top-left sample
				l2r.Set(2*MAP_XY_FACTOR,0,MAP_HEIGHT_SCALE*(pMap->getDisplayHeight(getXWithOrigin(i)+cellOffset, getYWithOrigin(j)) - pMap->getDisplayHeight(un0, getYWithOrigin(j))));
				n2f.Set(0,2*MAP_XY_FACTOR,MAP_HEIGHT_SCALE*(pMap->getDisplayHeight(getXWithOrigin(i), (getYWithOrigin(j)+cellOffset)) - pMap->getDisplayHeight(getXWithOrigin(i), vn0)));
				
#ifdef ALLOW_TEMPORARIES
				normalAtTexel= Normalize(Vector3::Cross_Product(l2r,n2f));
#else
				Vector3::Normalized_Cross_Product(l2r, n2f, &normalAtTexel);
#endif

				vb->x=xCoord;
				vb->y=yCoord;
				vb->z=  ((float)pMap->getDisplayHeight(getXWithOrigin(i), getYWithOrigin(j)))*MAP_HEIGHT_SCALE;
				vb->x = ADJUST_FROM_INDEX_TO_REAL(vb->x);
				vb->y = ADJUST_FROM_INDEX_TO_REAL(vb->y);
				vb->u1=U[0];
				vb->v1=V[0];
				vb->u2=UA[0];
				vb->v2=VA[0];
				doTheLight(vb, lightRay, &normalAtTexel, pLightsIterator, alpha[0]);
				vb++;

				//top-right sample
				l2r.Set(2*MAP_XY_FACTOR,0,MAP_HEIGHT_SCALE*(pMap->getDisplayHeight(up1 , getYWithOrigin(j) ) - pMap->getDisplayHeight(getXWithOrigin(i) , getYWithOrigin(j) )));
				n2f.Set(0,2*MAP_XY_FACTOR,MAP_HEIGHT_SCALE*(pMap->getDisplayHeight(getXWithOrigin(i)+cellOffset , (getYWithOrigin(j)+cellOffset) ) - pMap->getDisplayHeight(getXWithOrigin(i)+cellOffset , vn0 )));
				
#ifdef ALLOW_TEMPORARIES
				normalAtTexel= Normalize(Vector3::Cross_Product(l2r,n2f));
#else
				Vector3::Normalized_Cross_Product(l2r, n2f, &normalAtTexel);
#endif

				vb->x=xCoord+cellOffset;
				vb->y=yCoord;
				vb->z=  ((float)pMap->getDisplayHeight(getXWithOrigin(i)+cellOffset, getYWithOrigin(j)))*MAP_HEIGHT_SCALE;
				vb->x = ADJUST_FROM_INDEX_TO_REAL(vb->x);
				vb->y = ADJUST_FROM_INDEX_TO_REAL(vb->y);
				vb->u1=U[1];
				vb->v1=V[1];
				vb->u2=UA[1];
				vb->v2=VA[1];
				doTheLight(vb, lightRay, &normalAtTexel, pLightsIterator, alpha[1]);
				vb++;

				//bottom-right sample
				l2r.Set(2*MAP_XY_FACTOR,0,MAP_HEIGHT_SCALE*(pMap->getDisplayHeight(up1 , (getYWithOrigin(j)+cellOffset) ) - pMap->getDisplayHeight(getXWithOrigin(i) , (getYWithOrigin(j)+cellOffset) )));
				n2f.Set(0,2*MAP_XY_FACTOR,MAP_HEIGHT_SCALE*(pMap->getDisplayHeight(getXWithOrigin(i)+cellOffset , vp1 ) - pMap->getDisplayHeight(getXWithOrigin(i)+cellOffset , getYWithOrigin(j) )));
				
#ifdef ALLOW_TEMPORARIES
				normalAtTexel= Normalize(Vector3::Cross_Product(l2r,n2f));
#else
				Vector3::Normalized_Cross_Product(l2r, n2f, &normalAtTexel);
#endif

				vb->x=xCoord+cellOffset;
				if (yCoord + 1 == pMap->getDrawOrgY() + m_y - 1) { 
					vb->y=yCoord+1;
				} else {
					vb->y=yCoord+cellOffset;
				}
				vb->z=  ((float)pMap->getDisplayHeight(getXWithOrigin(i)+cellOffset, getYWithOrigin(j)+cellOffset))*MAP_HEIGHT_SCALE;
				vb->x = ADJUST_FROM_INDEX_TO_REAL(vb->x);
				vb->y = ADJUST_FROM_INDEX_TO_REAL(vb->y);
				vb->u1=U[2];
				vb->v1=V[2];
				vb->u2=UA[2];
				vb->v2=VA[2];
				doTheLight(vb, lightRay, &normalAtTexel, pLightsIterator, alpha[2]);
				vb++;

				//bottom-left sample
				l2r.Set(2*MAP_XY_FACTOR,0,MAP_HEIGHT_SCALE*(pMap->getDisplayHeight(getXWithOrigin(i)+cellOffset , (getYWithOrigin(j)+cellOffset) ) - pMap->getDisplayHeight(un0 , (getYWithOrigin(j)+cellOffset) )));
				n2f.Set(0,2*MAP_XY_FACTOR,MAP_HEIGHT_SCALE*(pMap->getDisplayHeight(getXWithOrigin(i) , vp1 ) - pMap->getDisplayHeight(getXWithOrigin(i) , getYWithOrigin(j) )));
				
#ifdef ALLOW_TEMPORARIES
				normalAtTexel= Normalize(Vector3::Cross_Product(l2r,n2f));
#else
				Vector3::Normalized_Cross_Product(l2r, n2f, &normalAtTexel);
#endif

				if (xCoord == pMap->getDrawOrgX()) { 
					vb->x=xCoord;
					//if (vb->x < 0) vb->x = 0;
				} else {
					vb->x=xCoord;
				}
				if (yCoord + 1 == pMap->getDrawOrgY() + m_y - 1) { 
					vb->y=yCoord+1;
				} else {
					vb->y=yCoord+cellOffset;
				}
				vb->z=  ((float)pMap->getDisplayHeight(getXWithOrigin(i), getYWithOrigin(j)+cellOffset))*MAP_HEIGHT_SCALE;
				vb->x = ADJUST_FROM_INDEX_TO_REAL(vb->x);
				vb->y = ADJUST_FROM_INDEX_TO_REAL(vb->y);
				vb->u1=U[3];
				vb->v1=V[3];
				vb->u2=UA[3];
				vb->v2=VA[3];
				doTheLight(vb, lightRay, &normalAtTexel, pLightsIterator, alpha[3]);
				vb++;

				VERTEX_FORMAT *pCurVertices = vb-4;
#ifdef FLIP_TRIANGLES // jba - reduces "diamonding" in some cases, not others.  Better cliffs, though.
				VERTEX_FORMAT tmpVertex;
				if (flipForBlend) {
					tmpVertex = pCurVertices[0];
					pCurVertices[0] = pCurVertices[1];
					pCurVertices[1] = pCurVertices[2];
					pCurVertices[2] = pCurVertices[3];
					pCurVertices[3] = tmpVertex;
				}
#endif

				if (m_showImpassableAreas) {
					// Color impassable cells "red"
					DEBUG_ASSERTCRASH(PATHFIND_CELL_SIZE_F == MAP_XY_FACTOR, ("Pathfind must be terrain cell size, or this code needs reworking.  John A."));
					Real borderHiX = (pMap->getXExtent()-2*pMap->getBorderSize())*MAP_XY_FACTOR;
					Real borderHiY = (pMap->getYExtent()-2*pMap->getBorderSize())*MAP_XY_FACTOR;
					Bool border = pCurVertices[0].x == -MAP_XY_FACTOR || pCurVertices[0].y == -MAP_XY_FACTOR;
					Bool cliffMapped = pMap->isCliffMappedTexture(getXWithOrigin(i), getYWithOrigin(j));
					if (pCurVertices[0].x == borderHiX) {
						border = true;
					}
					if (pCurVertices[0].y == borderHiY) {
						border = true;
					}
					Bool isCliff = pMap->getCliffState(getXWithOrigin(i)+pMap->getDrawOrgX(), getYWithOrigin(j)+pMap->getDrawOrgY())
												 || showAsVisibleCliff(getXWithOrigin(i) + pMap->getDrawOrgX(), getYWithOrigin(j)+pMap->getDrawOrgY());

					if ( isCliff || border || cliffMapped) {
						Int cellX, cellY;
						for (cellX=0; cellX<2; cellX++) {
							for (cellY=0; cellY<2; cellY++) {
								Int vertex = cellX+2*cellY;
								if (border) {
									Bool doBorder = false;
									if (pCurVertices[vertex].y >= 0 && pCurVertices[vertex].y <= borderHiY) {
										if (pCurVertices[vertex].x == 0 || pCurVertices[vertex].x == borderHiX) {
											doBorder = true;
										}
									}
									if (pCurVertices[vertex].x >= 0 && pCurVertices[vertex].x <= borderHiX) {
										if (pCurVertices[vertex].y == 0 || pCurVertices[vertex].y == borderHiY) {
											doBorder = true;
										}
									}
									if (doBorder) {
										pCurVertices[vertex].diffuse &= 0xFF0000ff; // blue with alpha.
									}
								} else if (isCliff) {
									pCurVertices[vertex].diffuse &= 0xFFFF0000; // red with alpha.
								}
								if (cliffMapped && vertex==0) {
									pCurVertices[vertex].diffuse &= 0xFF000000; // Black.
									pCurVertices[vertex].diffuse |= 0xff00; // Add green.
								}
							}
						}
					}
				}

				// Note - We have been building the vertex buffer in the memory location.
				// Now copy the set of vertices into the hardware buffer.
				// We don't copy the whole vertex buffer because we often update only
				// a couple of rows and its a lot faster to just copy the ones that change.
				Int offset = pCurVertices - vBase;
				memcpy(vbHardware+offset, pCurVertices, 4*sizeof(VERTEX_FORMAT));
			}
		}
		return 0; //success.
	}
	return -1;
}

//=============================================================================
// HeightMapRenderObjClass::updateVBForLight
//=============================================================================
/** Update the dynamic lighting values only in a rectangular block of the given Vertex Buffer. 
The vertex locations and texture coords are unchanged.
*/
Int HeightMapRenderObjClass::updateVBForLight(DX8VertexBufferClass	*pVB, char *data, Int x0, Int y0, Int x1, Int y1, Int originX, Int originY, W3DDynamicLight *pLights[], Int numLights)
{

#if (OPTIMIZED_HEIGHTMAP_LIGHTING)	// (gth) if optimizations are enabled, jump over to the "optimized" version of this function.
	return updateVBForLightOptimized( pVB, data, x0, y0, x1, y1, originX, originY, pLights, numLights );
#endif	
	
	Int i,j,k;
	Int vn0,un0,vp1,up1;
	Vector3 l2r,n2f,normalAtTexel;
	Int	vertsPerRow=(VERTEX_BUFFER_TILE_LENGTH)*4;	//vertices per row of VB

	if (m_vertexBufferTiles && m_map)
	{
#ifdef RTS_DEBUG
		assert(x0 >= originX && y0 >= originY && x1>x0 && y1>y0 && x1<=originX+VERTEX_BUFFER_TILE_LENGTH && y1<=originY+VERTEX_BUFFER_TILE_LENGTH);
#endif 

		DX8VertexBufferClass::WriteLockClass lockVtxBuffer(pVB);
		VERTEX_FORMAT *vBase = (VERTEX_FORMAT*)lockVtxBuffer.Get_Vertex_Array();
		VERTEX_FORMAT *vb;
		
		for (j=y0; j<y1; j++)
		{
			if (m_halfResMesh) {
				if (j&1) continue;
			}
			Int yCoord = getYWithOrigin(j)+m_map->getDrawOrgY()-m_map->getBorderSize();
			Bool intersect = false;
			for (k=0; k<numLights; k++) {
				if (pLights[k]->m_minY <= yCoord+1 && 
					pLights[k]->m_maxY >= yCoord) {
					intersect = true;
				}
				if (pLights[k]->m_prevMinY <= yCoord+1 && 
					pLights[k]->m_prevMaxY >= yCoord) {
					intersect = true;
				}
			}
			if (!intersect) {
				continue;
			}
			vn0 = getYWithOrigin(j)-1;
			if (vn0 < -m_map->getDrawOrgY())
				vn0=-m_map->getDrawOrgY();
			vp1 = getYWithOrigin(j+1)+1;
			if (vp1 >= m_map->getYExtent()-m_map->getDrawOrgY())
				vp1=m_map->getYExtent()-m_map->getDrawOrgY()-1;

			for (i=x0; i<x1; i++)
			{
				if (m_halfResMesh) {
					if (i&1) continue;
				}
				Int xCoord = getXWithOrigin(i)+m_map->getDrawOrgX()-m_map->getBorderSize();
				Bool intersect = false;
				for (k=0; k<numLights; k++) {
					if (pLights[k]->m_minX <= xCoord+1 && 
						pLights[k]->m_maxX >= xCoord &&
						pLights[k]->m_minY <= yCoord+1 && 
						pLights[k]->m_maxY >= yCoord) {
						intersect = true;
					}
					if (pLights[k]->m_prevMinX <= xCoord+1 && 
						pLights[k]->m_prevMaxX >= xCoord &&
						pLights[k]->m_prevMinY <= yCoord+1 && 
						pLights[k]->m_prevMaxY >= yCoord) {
						intersect = true;
					}
				}
				if (!intersect) {
					continue;
				}
				// vb is the pointer to the vertex in the hardware dx8 vertex buffer.
				Int offset = (j-originY)*vertsPerRow+4*(i-originX);
				if (m_halfResMesh) {
					offset = (j-originY)*vertsPerRow/4+2*(i-originX);
				}
				vb = vBase + offset;	//skip to correct row in vertex buffer
				// vbMirror is the pointer to the vertex in our memory based copy.
				// The important point is that we can read out of our copy to get the original
				// diffuse color, and xyz location.  It is VERY SLOW to read out of the 
				// hardware vertex buffer, possibly worse... jba.
				VERTEX_FORMAT *vbMirror = ((VERTEX_FORMAT*)data)  + offset;
				un0 = getXWithOrigin(i)-1;
				if (un0 < -m_map->getDrawOrgX())
					un0=-m_map->getDrawOrgX();
				up1 = getXWithOrigin(i+1)+1;
				if (up1 >= m_map->getXExtent()-m_map->getDrawOrgX())
					up1=m_map->getXExtent()-m_map->getDrawOrgX()-1;

				Vector3 lightRay(0,0,0);

				//top-left sample
				l2r.Set(2*MAP_XY_FACTOR,0,MAP_HEIGHT_SCALE*(m_map->getDisplayHeight(getXWithOrigin(i)+1, getYWithOrigin(j)) - m_map->getDisplayHeight(un0, getYWithOrigin(j))));
				n2f.Set(0,2*MAP_XY_FACTOR,MAP_HEIGHT_SCALE*(m_map->getDisplayHeight(getXWithOrigin(i), (getYWithOrigin(j)+1)) - m_map->getDisplayHeight(getXWithOrigin(i), vn0)));
				
#ifdef ALLOW_TEMPORARIES
				normalAtTexel= Normalize(Vector3::Cross_Product(l2r,n2f));
#else
				Vector3::Normalized_Cross_Product(l2r, n2f, &normalAtTexel);
#endif

				doTheDynamicLight(vb, vbMirror, &lightRay, &normalAtTexel, pLights, numLights);
				vb++;	vbMirror++;

				//top-right sample
				l2r.Set(2*MAP_XY_FACTOR,0,MAP_HEIGHT_SCALE*(m_map->getDisplayHeight(up1 , getYWithOrigin(j) ) - m_map->getDisplayHeight(getXWithOrigin(i) , getYWithOrigin(j) )));
				n2f.Set(0,2*MAP_XY_FACTOR,MAP_HEIGHT_SCALE*(m_map->getDisplayHeight(getXWithOrigin(i)+1 , (getYWithOrigin(j)+1) ) - m_map->getDisplayHeight(getXWithOrigin(i)+1 , vn0 )));
				
#ifdef ALLOW_TEMPORARIES
				normalAtTexel= Normalize(Vector3::Cross_Product(l2r,n2f));
#else
				Vector3::Normalized_Cross_Product(l2r, n2f, &normalAtTexel);
#endif

				doTheDynamicLight(vb, vbMirror, &lightRay, &normalAtTexel, pLights, numLights);
				vb++;	vbMirror++;

				//bottom-right sample
				l2r.Set(2*MAP_XY_FACTOR,0,MAP_HEIGHT_SCALE*(m_map->getDisplayHeight(up1 , (getYWithOrigin(j)+1) ) - m_map->getDisplayHeight(getXWithOrigin(i) , (getYWithOrigin(j)+1) )));
				n2f.Set(0,2*MAP_XY_FACTOR,MAP_HEIGHT_SCALE*(m_map->getDisplayHeight(getXWithOrigin(i)+1 , vp1 ) - m_map->getDisplayHeight(getXWithOrigin(i)+1 , getYWithOrigin(j) )));
				
#ifdef ALLOW_TEMPORARIES
				normalAtTexel= Normalize(Vector3::Cross_Product(l2r,n2f));
#else
				Vector3::Normalized_Cross_Product(l2r, n2f, &normalAtTexel);
#endif

				doTheDynamicLight(vb, vbMirror, &lightRay, &normalAtTexel, pLights, numLights);
				vb++;	vbMirror++;

				//bottom-left sample
				l2r.Set(2*MAP_XY_FACTOR,0,MAP_HEIGHT_SCALE*(m_map->getDisplayHeight(getXWithOrigin(i)+1 , (getYWithOrigin(j)+1) ) - m_map->getDisplayHeight(un0 , (getYWithOrigin(j)+1) )));
				n2f.Set(0,2*MAP_XY_FACTOR,MAP_HEIGHT_SCALE*(m_map->getDisplayHeight(getXWithOrigin(i) , vp1 ) - m_map->getDisplayHeight(getXWithOrigin(i) , getYWithOrigin(j) )));
				
#ifdef ALLOW_TEMPORARIES
				normalAtTexel= Normalize(Vector3::Cross_Product(l2r,n2f));
#else
				Vector3::Normalized_Cross_Product(l2r, n2f, &normalAtTexel);
#endif

				doTheDynamicLight(vb, vbMirror, &lightRay, &normalAtTexel, pLights, numLights);
				vb++;	vbMirror++;
			}
		}
		return 0; //success.
	}
	return -1;
}


Int HeightMapRenderObjClass::updateVBForLightOptimized(DX8VertexBufferClass	*pVB, char *data, Int x0, Int y0, Int x1, Int y1, Int originX, Int originY, W3DDynamicLight *pLights[], Int numLights)
{
	Int i,j,k;
	Int vn0,un0,vp1,up1;
	Vector3 l2r,n2f,normalAtTexel;
	Int	vertsPerRow=(VERTEX_BUFFER_TILE_LENGTH)*4;	//vertices per row of VB

	if (m_vertexBufferTiles && m_map)
	{
#ifdef RTS_DEBUG
		assert(x0 >= originX && y0 >= originY && x1>x0 && y1>y0 && x1<=originX+VERTEX_BUFFER_TILE_LENGTH && y1<=originY+VERTEX_BUFFER_TILE_LENGTH);
#endif 

		DX8VertexBufferClass::WriteLockClass lockVtxBuffer(pVB);
		VERTEX_FORMAT *vBase = (VERTEX_FORMAT*)lockVtxBuffer.Get_Vertex_Array();
		VERTEX_FORMAT *vb;
		
		//
		// (gth) the optimization in this function is to take advantage of verts in the same
		// x,y position who have already computed their lighting.  To do this, we need to set up
		// some offsets in the vertex buffer.  I've computed these offsets to be consistent with
		// the formula's that Generals is using but in the case of the "half-res-mesh" I'm not
		// sure things are correct...
		//
		Int quad_right_offset;
		Int quad_below_offset;
		Int quad_below_right_offset;

		if (m_halfResMesh == false) {
			// offset = (j-originY)*vertsPerRow+4*(i-originX);
			quad_right_offset = 4;
			quad_below_offset = vertsPerRow;
			quad_below_right_offset = vertsPerRow + 4;

		} else {
			// offset = (j-originY)*vertsPerRow/4+2*(i-originX);
			quad_right_offset = 2;
			quad_below_offset = vertsPerRow/4;
			quad_below_right_offset = vertsPerRow/4 + 2;
		}

		// 
		// i,j loop over the quads affected by the light.  Each quad has its *own* 4 vertices.  This
		// means that for any vertex position on the map, there are actually 4 copies of the vertex. 
		//
		for (j=y0; j<y1; j++)
		{
			if (m_halfResMesh) {
				if (j&1) continue;
			}
			Int yCoord = getYWithOrigin(j)+m_map->getDrawOrgY()-m_map->getBorderSize();
			Bool intersect = false;
			for (k=0; k<numLights; k++) {
				if (pLights[k]->m_minY <= yCoord+1 && 
					pLights[k]->m_maxY >= yCoord) {
					intersect = true;
				}
				if (pLights[k]->m_prevMinY <= yCoord+1 && 
					pLights[k]->m_prevMaxY >= yCoord) {
					intersect = true;
				}
			}
			if (!intersect) {
				continue;
			}
			vn0 = getYWithOrigin(j)-1;
			if (vn0 < -m_map->getDrawOrgY())
				vn0=-m_map->getDrawOrgY();
			vp1 = getYWithOrigin(j+1)+1;
			if (vp1 >= m_map->getYExtent()-m_map->getDrawOrgY())
				vp1=m_map->getYExtent()-m_map->getDrawOrgY()-1;

			for (i=x0; i<x1; i++)
			{
				if (m_halfResMesh) {
					if (i&1) continue;
				}
				Int xCoord = getXWithOrigin(i)+m_map->getDrawOrgX()-m_map->getBorderSize();
				Bool intersect = false;
				for (k=0; k<numLights; k++) {
					if (pLights[k]->m_minX <= xCoord+1 && 
						pLights[k]->m_maxX >= xCoord &&
						pLights[k]->m_minY <= yCoord+1 && 
						pLights[k]->m_maxY >= yCoord) {
						intersect = true;
					}
					if (pLights[k]->m_prevMinX <= xCoord+1 && 
						pLights[k]->m_prevMaxX >= xCoord &&
						pLights[k]->m_prevMinY <= yCoord+1 && 
						pLights[k]->m_prevMaxY >= yCoord) {
						intersect = true;
					}
				}
				if (!intersect) {
					continue;
				}
				// vb is the pointer to the vertex in the hardware dx8 vertex buffer.
				Int offset = (j-originY)*vertsPerRow+4*(i-originX);
				if (m_halfResMesh) {
					offset = (j-originY)*vertsPerRow/4+2*(i-originX);
				}
				vb = vBase + offset;	//skip to correct row in vertex buffer
				// vbMirror is the pointer to the vertex in our memory based copy.
				// The important point is that we can read out of our copy to get the original
				// diffuse color, and xyz location.  It is VERY SLOW to read out of the 
				// hardware vertex buffer, possibly worse... jba.
				VERTEX_FORMAT *vbMirror = ((VERTEX_FORMAT*)data)  + offset;
				VERTEX_FORMAT *vbaseMirror = ((VERTEX_FORMAT*)data);
				un0 = getXWithOrigin(i)-1;
				if (un0 < -m_map->getDrawOrgX())
					un0=-m_map->getDrawOrgX();
				up1 = getXWithOrigin(i+1)+1;
				if (up1 >= m_map->getXExtent()-m_map->getDrawOrgX())
					up1=m_map->getXExtent()-m_map->getDrawOrgX()-1;

				Vector3 lightRay(0,0,0);

				//
				// (gth) Following the set of rules below lets us take advantage of lighting values that have
				// been previously computed.  The idea is to copy them ahead to future quads that will need them
				// and then not compute them when we get to those quads.  This also avoids having to read-back
				// from the vertex buffer but we do jump around in memory... probably bad anyway, maybe we should
				// compute into a temporary buffer and copy all at once...
				//
				unsigned long light_copy;

				// top-left sample -> only compute when i==0 and j==0 
				if ((i==x0) && (j==y0)) {
					l2r.Set(2*MAP_XY_FACTOR,0,MAP_HEIGHT_SCALE*(m_map->getDisplayHeight(getXWithOrigin(i)+1, getYWithOrigin(j)) - m_map->getDisplayHeight(un0, getYWithOrigin(j))));
					n2f.Set(0,2*MAP_XY_FACTOR,MAP_HEIGHT_SCALE*(m_map->getDisplayHeight(getXWithOrigin(i), (getYWithOrigin(j)+1)) - m_map->getDisplayHeight(getXWithOrigin(i), vn0)));
					Vector3::Normalized_Cross_Product(l2r, n2f, &normalAtTexel);
					doTheDynamicLight(vb, vbMirror, &lightRay, &normalAtTexel, pLights, numLights);
				} 
				vb++;	vbMirror++;

				//top-right sample -> compute when j==0, then copy to (right,0)
				if (j==y0) {
					l2r.Set(2*MAP_XY_FACTOR,0,MAP_HEIGHT_SCALE*(m_map->getDisplayHeight(up1 , getYWithOrigin(j) ) - m_map->getDisplayHeight(getXWithOrigin(i) , getYWithOrigin(j) )));
					n2f.Set(0,2*MAP_XY_FACTOR,MAP_HEIGHT_SCALE*(m_map->getDisplayHeight(getXWithOrigin(i)+1 , (getYWithOrigin(j)+1) ) - m_map->getDisplayHeight(getXWithOrigin(i)+1 , vn0 )));
					Vector3::Normalized_Cross_Product(l2r, n2f, &normalAtTexel);
					light_copy = doTheDynamicLight(vb, vbMirror, &lightRay, &normalAtTexel, pLights, numLights);
				
					if (i < x1-1) {
						// copy light to (right,0)
						(vBase + offset + quad_right_offset)->diffuse = (light_copy&0x00FFFFFF) | ((vbaseMirror + offset + quad_right_offset)->diffuse&0xff000000) ;
					}
				} 
				vb++;	vbMirror++;

				//bottom-right sample -> always compute, then copy to (right,3), (down,1), (down+right,0)
				l2r.Set(2*MAP_XY_FACTOR,0,MAP_HEIGHT_SCALE*(m_map->getDisplayHeight(up1 , (getYWithOrigin(j)+1) ) - m_map->getDisplayHeight(getXWithOrigin(i) , (getYWithOrigin(j)+1) )));
				n2f.Set(0,2*MAP_XY_FACTOR,MAP_HEIGHT_SCALE*(m_map->getDisplayHeight(getXWithOrigin(i)+1 , vp1 ) - m_map->getDisplayHeight(getXWithOrigin(i)+1 , getYWithOrigin(j) )));
				Vector3::Normalized_Cross_Product(l2r, n2f, &normalAtTexel);
				light_copy = doTheDynamicLight(vb, vbMirror, &lightRay, &normalAtTexel, pLights, numLights);
				
				if (i < x1-1) {
					// copy light to (right,3)
					//(vBase + offset + quad_right_offset + 3)->diffuse = light_copy;
					(vBase + offset + quad_right_offset + 3)->diffuse = (light_copy&0x00FFFFFF) | ((vbaseMirror + offset + quad_right_offset + 3)->diffuse&0xff000000) ;
				}
				if (j < y1-1) {
					// copy light to (down,1)
					//(vBase + offset + quad_below_offset + 1)->diffuse = light_copy;
					(vBase + offset + quad_right_offset + 1)->diffuse = (light_copy&0x00FFFFFF) | ((vbaseMirror + offset + quad_right_offset + 1)->diffuse&0xff000000) ;
				}
				if ((i < x1-1) && (j < y1-1)) {
					// copy light to (right+down,0)
					//(vBase + offset + quad_below_right_offset)->diffuse = light_copy;
					(vBase + offset + quad_right_offset)->diffuse = (light_copy&0x00FFFFFF) | ((vbaseMirror + offset + quad_right_offset)->diffuse&0xff000000) ;
				}
				vb++;	vbMirror++;

				//bottom-left sample -> compute when i==0, otherwise copy from (left,2)
				if (i==x0) {
					l2r.Set(2*MAP_XY_FACTOR,0,MAP_HEIGHT_SCALE*(m_map->getDisplayHeight(getXWithOrigin(i)+1 , (getYWithOrigin(j)+1) ) - m_map->getDisplayHeight(un0 , (getYWithOrigin(j)+1) )));
					n2f.Set(0,2*MAP_XY_FACTOR,MAP_HEIGHT_SCALE*(m_map->getDisplayHeight(getXWithOrigin(i) , vp1 ) - m_map->getDisplayHeight(getXWithOrigin(i) , getYWithOrigin(j) )));
					Vector3::Normalized_Cross_Product(l2r, n2f, &normalAtTexel);
					light_copy = doTheDynamicLight(vb, vbMirror, &lightRay, &normalAtTexel, pLights, numLights);

					if (j < y1-1) {
						// copy light to (down,0)
						//(vBase + offset + quad_below_offset)->diffuse = light_copy;
						(vBase + offset + quad_below_offset)->diffuse = (light_copy&0x00FFFFFF) | ((vbaseMirror + offset + quad_below_offset)->diffuse&0xff000000) ;
					}
				} 
				vb++;	vbMirror++;
			}
		}
		return 0; //success.
	}
	return -1;
}



//=============================================================================
// HeightMapRenderObjClass::doPartialUpdate
//=============================================================================
/** Updates a partial block of vertices from [x0,y0 to x1,y1]
The coordinates in partialRange are map cell coordinates, relative to the entire map.
The vertex coordinates and texture coordinates, as well as static lighting are updated.
*/
void HeightMapRenderObjClass::doPartialUpdate(const IRegion2D &partialRange, WorldHeightMap *htMap, RefRenderObjListIterator *pLightsIterator)
{	
	// Adjust range into the current drawn map range.
	Int minX = partialRange.lo.x - htMap->getDrawOrgX();
	Int maxX = partialRange.hi.x - htMap->getDrawOrgX();
	Int minY = partialRange.lo.y - htMap->getDrawOrgY();
	Int maxY = partialRange.hi.y - htMap->getDrawOrgY();
	if (minX<0) minX = 0;
	if (minY<0) minY = 0;
	if (maxX > m_x-1) maxX = m_x-1;
	if (maxY > m_y-1) maxY = m_y-1;
	if (maxX < minX) return;
	if (maxY < minY) return;
	if (m_originX == 0 && m_originY == 0) {
		// simple case.
		updateBlock(minX, minY, maxX, maxY,
								htMap, pLightsIterator);
	}
	else
	{
		minY = minY+m_originY;
		maxY = maxY+m_originY;

		if (minY> m_y-1) {
			minY -= m_y-1;
 			maxY -= m_y-1;
		}
		if (maxY > m_y-1) {
			maxY -= m_y-1;
			updateBlock(0, minY, m_x-1, m_y-1, htMap, pLightsIterator);
			updateBlock(0, 0, m_x-1, maxY, htMap, pLightsIterator);
		} else {
			updateBlock(0, minY, m_x-1, maxY, htMap, pLightsIterator);
		}
	}

	if (!m_extraBlendTilePositions)
	{	//Need to allocate memory
		m_extraBlendTilePositions = NEW Int[DEFAULT_MAX_MAP_EXTRABLEND_TILES];
		m_extraBlendTilePositionsSize = DEFAULT_MAX_MAP_EXTRABLEND_TILES;
	}
	
	//Find list of all extra blend tiles used on map.  These are tiles with 3 materials/textures
	//over the same tile and require an extra render pass.

	Int i, j;
	//First remove any existing extra blend tiles within this partial region
	for (j=0; j<m_numExtraBlendTiles; j++)
	{	Int x = m_extraBlendTilePositions[j] & 0xffff;
		Int y = m_extraBlendTilePositions[j] >> 16;
		if (x >= partialRange.lo.x && x < partialRange.hi.x &&
			y >= partialRange.lo.y && y < partialRange.hi.y)
		{	//this tile is inside region being updated so remove it by shifting tile array
			memcpy(m_extraBlendTilePositions+j,m_extraBlendTilePositions+j+1,(m_numExtraBlendTiles-1-j)*sizeof(Int));
			m_numExtraBlendTiles--;
			j--;	//need to look at index j again because this tile was removed
		}
	}

	for (j=partialRange.lo.y; j<partialRange.hi.y; j++)
		for (i=partialRange.lo.x; i<partialRange.hi.x; i++)
		{
			if (j<0 || i<0) continue;
			Real U[4],V[4];
			UnsignedByte alpha[4];
			Bool flipState,cliffState;
			if (htMap->getExtraAlphaUVData(i,j,U,V,alpha,&flipState, &cliffState))
			{	if (m_numExtraBlendTiles >= m_extraBlendTilePositionsSize)
				{	//no more room to store extra blend tiles so enlarge the buffer.
					Int *tempPositions=NEW Int[m_extraBlendTilePositionsSize+512];
					memcpy(tempPositions, m_extraBlendTilePositions, m_extraBlendTilePositionsSize*sizeof(Int));
					delete [] m_extraBlendTilePositions;
					//enlarge by more tiles to reduce memory trashing
					m_extraBlendTilePositions = tempPositions;
					m_extraBlendTilePositionsSize += 512;
				}
				//Pack x and y position into single integer since maps are limited in size
				m_extraBlendTilePositions[m_numExtraBlendTiles]=i | (j <<16);
				m_numExtraBlendTiles++;
			}
		}
	updateShorelineTiles(partialRange.lo.x,partialRange.lo.y,partialRange.hi.x,partialRange.hi.y,htMap);

	updateViewImpassableAreas(TRUE, minX, maxX, minY, maxY);
}

//=============================================================================
// HeightMapRenderObjClass::updateBlock
//=============================================================================
/** Updates a block of vertices from [x0,y0 to x1,y1]
The vertex coordinates and texture coordinates, as well as static lighting are updated.
*/
Int HeightMapRenderObjClass::updateBlock(Int x0, Int y0, Int x1, Int y1,  WorldHeightMap *pMap, RefRenderObjListIterator *pLightsIterator)
{	
#ifdef RTS_DEBUG
	DEBUG_ASSERTCRASH(x0>=0&&y0>=0 && x1<m_x && y1<m_y && x0<=x1 && y0<=y1, ("Invalid updates."));
#endif
	Invalidate_Cached_Bounding_Volumes();
	if (pMap && m_treeBuffer != NULL) {
		REF_PTR_SET(m_stageZeroTexture, pMap->getTerrainTexture());
		REF_PTR_SET(m_stageOneTexture, pMap->getAlphaTerrainTexture());
	}

	Int i,j;
	DX8VertexBufferClass	**pVB;
	Int originX,originY;
	//step through each vertex buffer that needs updating
	for (j=0; j<m_numVBTilesY; j++)
	{
		originY=j*VERTEX_BUFFER_TILE_LENGTH;	//location of this VB on the large full-size heightmap
		Int yMin, yMax;
		yMin = originY;
		if (y0>yMin) yMin = y0;
		yMax = originY+VERTEX_BUFFER_TILE_LENGTH;
		if (y1<yMax) yMax = y1;
		if (yMin >= yMax) {
			continue;
		}
		for (i=0; i<m_numVBTilesX; i++)
		{	
			originX=i*VERTEX_BUFFER_TILE_LENGTH;	//location of this VB on the large full-size heightmap
			Int xMin, xMax;
			xMin = originX;
			if (xMin<x0) xMin = x0;
			xMax = originX+VERTEX_BUFFER_TILE_LENGTH;
			if (xMax>x1) xMax = x1;
			if (xMin >= xMax) {
				continue;
			}
			pVB=m_vertexBufferTiles+j*m_numVBTilesX+i;	//point to correct row/column of vertex buffers 
			char **pData = m_vertexBufferBackup+j*m_numVBTilesX+i;
			updateVB(*pVB, *pData, xMin, yMin, xMax, yMax, originX, originY, pMap, pLightsIterator);
		}
	}

	return 0;
}

//=============================================================================
// HeightMapRenderObjClass::getTileBoundingBox
//=============================================================================
/** Calculate the bounding box for given terrain tile.  Right now each terrain tile is equivalent
to one of the vertex buffers holding heightmap vertices. */
//=============================================================================
AABoxClass & HeightMapRenderObjClass::getTileBoundingBox(AABoxClass *aabox, Int x, Int y)
{
	Vector3 vmin;
	Vector3 vmax;
	Int xOffset = 0;
	Int yOffset = 0;
	if (m_map) {
		xOffset = m_map->getDrawOrgX();
		yOffset = m_map->getDrawOrgY();
	}

	vmin.Set(x*VERTEX_BUFFER_TILE_LENGTH+xOffset,y*VERTEX_BUFFER_TILE_LENGTH+yOffset,m_minHeight);
	vmax.Set((x+1)*VERTEX_BUFFER_TILE_LENGTH+xOffset,(y+1)*VERTEX_BUFFER_TILE_LENGTH+yOffset,m_maxHeight);

	aabox->Init_Min_Max(vmin,vmax);
	return *aabox;
}

#ifdef DO_SCORCH
//=============================================================================
// HeightMapRenderObjClass::drawScorches
//=============================================================================
/** Draws the scorch marks. */
//=============================================================================
void HeightMapRenderObjClass::drawScorches(void)
{
	updateScorches();
	if (m_curNumScorchIndices == 0) {
		return;
	}
	DX8Wrapper::Set_Index_Buffer(m_indexScorch,0);
	DX8Wrapper::Set_Vertex_Buffer(m_vertexScorch);

	DX8Wrapper::Set_Texture(0,m_scorchTexture);
	if (Is_Hidden() == 0) {
		DX8Wrapper::Draw_Triangles(	0,m_curNumScorchIndices/3, 0,	m_curNumScorchVertices);
	}
}
#endif


//-----------------------------------------------------------------------------
//         Public Functions                                                
//-----------------------------------------------------------------------------

//=============================================================================
// HeightMapRenderObjClass::~HeightMapRenderObjClass
//=============================================================================
/** Destructor. Releases w3d assets. */
//=============================================================================
HeightMapRenderObjClass::~HeightMapRenderObjClass(void)
{
	freeMapResources();
	if (m_treeBuffer) {
		delete m_treeBuffer;
 		m_treeBuffer = NULL;
	}
	if (m_bibBuffer) {
		delete m_bibBuffer;
 		m_bibBuffer = NULL;
	}
#ifdef TEST_CUSTOM_EDGING
	if (m_customEdging) {
		delete m_customEdging;
 		m_customEdging = NULL;
	}
#endif
#ifdef DO_ROADS
	if (m_roadBuffer) {
		delete m_roadBuffer;
 		m_roadBuffer = NULL;
	}
#endif
	if (m_bridgeBuffer) {
		delete m_bridgeBuffer;
	}

	if( m_waypointBuffer )
	{
		delete m_waypointBuffer;
	}
	if (m_shroud) {
		delete m_shroud;
	}
	if (m_extraBlendTilePositions)
		delete [] m_extraBlendTilePositions;
	if (m_shoreLineTilePositions)
		delete [] m_shoreLineTilePositions;
}

//=============================================================================
// HeightMapRenderObjClass::HeightMapRenderObjClass
//=============================================================================
/** Constructor. Mostly nulls out the member variables. */
//=============================================================================
HeightMapRenderObjClass::HeightMapRenderObjClass(void)
{
	m_x=0;
	m_y=0;
	m_needFullUpdate = false;
	m_showImpassableAreas = false;
	m_originX = 0;
	m_originY = 0;
	m_updating = false;
	//Set height to the maximum value that can be stored.
	//We should refine this with actual value.
	m_maxHeight=(pow(256.0, sizeof(HeightSampleType))-1.0)*MAP_HEIGHT_SCALE;
	m_minHeight=0;
	m_numExtraBlendTiles=0;
	m_extraBlendTilePositions=NULL;
	m_extraBlendTilePositionsSize=0;
	m_shoreLineTilePositions=NULL;
	m_numShoreLineTiles=0;
	m_shoreLineTilePositionsSize=0;
	m_currentMinWaterOpacity = -1.0f;

	m_numVertexBufferTiles=0;
	m_indexBuffer=NULL;
	m_vertexMaterialClass=NULL;
#ifdef PRE_TRANSFORM_VERTEX
	m_xformedVertexBuffer = NULL;
#endif
	m_stageZeroTexture=NULL;
	m_stageOneTexture=NULL;
	m_stageTwoTexture=NULL;
	m_stageThreeTexture=NULL;
	m_destAlphaTexture=NULL;
	m_vertexBufferTiles=NULL;
	m_vertexBufferBackup=NULL;
	m_map=NULL;
	m_depthFade.X = 0.0f;
	m_depthFade.Y = 0.0f;
	m_depthFade.Z = 0.0f;
	m_useDepthFade = false;
	m_disableTextures = false;
	TheTerrainRenderObject = this;

	m_treeBuffer = NULL;
	m_bibBuffer = NULL;
#ifdef TEST_CUSTOM_EDGING
	m_customEdging = NULL;
#endif
	m_bridgeBuffer = NULL;
	m_waypointBuffer = NULL;
#ifdef DO_ROADS
	m_roadBuffer = NULL;
#endif
#ifdef DO_SCORCH
	m_vertexScorch = NULL;
	m_indexScorch = NULL;
	m_scorchTexture = NULL;
	clearAllScorches();
	m_shroud = NULL;
#endif
	m_bridgeBuffer = NEW W3DBridgeBuffer;

	if (TheGlobalData->m_headless)
		return;

	m_treeBuffer = NEW W3DTreeBuffer;

	m_bibBuffer = NEW W3DBibBuffer;

	m_curImpassableSlope = 45.0f;	// default to 45 degrees.

#ifdef TEST_CUSTOM_EDGING
	m_customEdging = NEW W3DCustomEdging;
#endif
	m_waypointBuffer = NEW W3DWaypointBuffer;
#ifdef DO_ROADS
	m_roadBuffer = NEW W3DRoadBuffer;
#endif

#if defined(RTS_DEBUG) || defined(RTS_INTERNAL)
	if (TheGlobalData->m_shroudOn)
		m_shroud = NEW W3DShroud;
#else
	m_shroud = NEW W3DShroud;
#endif
	DX8Wrapper::SetCleanupHook(this);
}


//=============================================================================
// HeightMapRenderObjClass::adjustTerrainLOD
//=============================================================================
/** Adjust the terrain Level Of Detail.  If adj > 0 , increases LOD 1 step, if 
adj < 0 decreases it one step, if adj==0, then just sets up for the current LOD */
//=============================================================================
void HeightMapRenderObjClass::adjustTerrainLOD(Int adj) 
{
	if (adj>0 && TheGlobalData->m_terrainLOD<TERRAIN_LOD_MAX) TheWritableGlobalData->m_terrainLOD=(TerrainLOD)(TheGlobalData->m_terrainLOD+1);
	if (adj<0 && TheGlobalData->m_terrainLOD>TERRAIN_LOD_MIN) TheWritableGlobalData->m_terrainLOD=(TerrainLOD)(TheGlobalData->m_terrainLOD-1);

	switch (TheGlobalData->m_terrainLOD) {
		case	TERRAIN_LOD_MIN: TheWritableGlobalData->m_useCloudMap = false;
									TheWritableGlobalData->m_useLightMap = false ;
									TheWritableGlobalData->m_useWaterPlane = false;
									TheWritableGlobalData->m_stretchTerrain = false;
									TheWritableGlobalData->m_useHalfHeightMap = true;
									break;
		case TERRAIN_LOD_HALF_CLOUDS: TheWritableGlobalData->m_useCloudMap = true;
									TheWritableGlobalData->m_useLightMap = true;
									TheWritableGlobalData->m_useWaterPlane = false;
									TheWritableGlobalData->m_stretchTerrain = false;
									TheWritableGlobalData->m_useHalfHeightMap = true;
									break;
		case TERRAIN_LOD_STRETCH_NO_CLOUDS: TheWritableGlobalData->m_useCloudMap = false;
									TheWritableGlobalData->m_useLightMap = false;
									TheWritableGlobalData->m_useWaterPlane = false;
									TheWritableGlobalData->m_stretchTerrain = true;
									TheWritableGlobalData->m_useHalfHeightMap = false;
									break;
		case TERRAIN_LOD_STRETCH_CLOUDS: TheWritableGlobalData->m_useCloudMap = true;
									TheWritableGlobalData->m_useLightMap = true;
									TheWritableGlobalData->m_useWaterPlane = false;
									TheWritableGlobalData->m_stretchTerrain = true;
									TheWritableGlobalData->m_useHalfHeightMap = false;
									break;
		case TERRAIN_LOD_NO_CLOUDS: TheWritableGlobalData->m_useCloudMap = false;
									TheWritableGlobalData->m_useLightMap = false;
									TheWritableGlobalData->m_useWaterPlane = false;
									TheWritableGlobalData->m_stretchTerrain = false;
									TheWritableGlobalData->m_useHalfHeightMap = false;
									break;
		default:
		case TERRAIN_LOD_NO_WATER: TheWritableGlobalData->m_useCloudMap = true;
									TheWritableGlobalData->m_useLightMap = true;
									TheWritableGlobalData->m_useWaterPlane = false;
									TheWritableGlobalData->m_stretchTerrain = false;
									TheWritableGlobalData->m_useHalfHeightMap = false;
									break;
		case TERRAIN_LOD_MAX: TheWritableGlobalData->m_useCloudMap = true;
									TheWritableGlobalData->m_useLightMap = true;
									TheWritableGlobalData->m_useWaterPlane = true;
									TheWritableGlobalData->m_stretchTerrain = false;
									TheWritableGlobalData->m_useHalfHeightMap = false;
									break;
	}
	if (m_map==NULL) return;
	m_map->setDrawOrg(m_map->getDrawOrgX(), m_map->getDrawOrgX());
	if (m_shroud)
		m_shroud->reset();	//need reset here since initHeightData will load new shroud.
	this->initHeightData(m_map->getDrawWidth(), 
											m_map->getDrawHeight(), m_map, NULL);
	staticLightingChanged();
	if (TheTacticalView) {
		TheTacticalView->setAngle(TheTacticalView->getAngle() + 1);
		TheTacticalView->setAngle(TheTacticalView->getAngle() - 1);
	}
}

//=============================================================================
// HeightMapRenderObjClass::ReleaseResources
//=============================================================================
/** Releases all w3d assets, to prepare for Reset device call. */
//=============================================================================
void HeightMapRenderObjClass::ReleaseResources(void)
{
	if (m_treeBuffer) {
		m_treeBuffer->freeTreeBuffers();
	}
	if (m_bibBuffer) {
		m_bibBuffer->freeBibBuffers();
	}
#ifdef TEST_CUSTOM_EDGING
	if (m_customEdging)
		m_customEdging ->freeEdgingBuffers();
#endif
	if (m_bridgeBuffer) {
		m_bridgeBuffer->freeBridgeBuffers();
	}

	if( m_waypointBuffer )
	{
		m_waypointBuffer->freeWaypointBuffers();
	}
	// We need to save the map.
	WorldHeightMap *pMap=NULL;
	REF_PTR_SET(pMap, m_map);
	freeMapResources();
	m_map = pMap; // ref_ptr_set has already incremented the ref count.
	if (TheWaterRenderObj)
		TheWaterRenderObj->ReleaseResources();
	if (TheTerrainTracksRenderObjClassSystem)
		TheTerrainTracksRenderObjClassSystem->ReleaseResources();
	if (TheW3DShadowManager)
		TheW3DShadowManager->ReleaseResources();
	if (m_shroud)
	{	m_shroud->reset();
		m_shroud->ReleaseResources();
	}

	//Release any resources that may be used by custom pixel/vertex shaders
	W3DShaderManager::shutdown();
#ifdef DO_ROADS
	if (m_roadBuffer) {
		m_roadBuffer->freeRoadBuffers();
	}		  
#endif
}

//=============================================================================
// HeightMapRenderObjClass::ReAcquireResources
//=============================================================================
/** Reallocates all W3D assets after a reset.. */
//=============================================================================
void HeightMapRenderObjClass::ReAcquireResources(void)
{
	W3DShaderManager::init();	//reaquire resources which may be needed by custom shaders

	if (TheWaterRenderObj)
		TheWaterRenderObj->ReAcquireResources();

	if (TheTerrainTracksRenderObjClassSystem)
		TheTerrainTracksRenderObjClassSystem->ReAcquireResources();

	if (TheW3DShadowManager)
		TheW3DShadowManager->ReAcquireResources();
	if (m_shroud)
		m_shroud->ReAcquireResources();

	Int x = m_x;
	Int y = m_y;

	if (m_map)
	{
		this->initHeightData(x,y,m_map, NULL);
		// Tell lights to update next time through.
		m_needFullUpdate = true;
	}

	if (m_treeBuffer) {
		m_treeBuffer->allocateTreeBuffers();
	}
	if (m_bibBuffer) {
		m_bibBuffer->allocateBibBuffers();
	}
#ifdef TEST_CUSTOM_EDGING
	if (m_customEdging)
		m_customEdging ->allocateEdgingBuffers();
#endif
	if (m_bridgeBuffer) {
		m_bridgeBuffer->allocateBridgeBuffers();
	}

	//Waypoint buffers are done dynamically. One line, one node (just rendered multiple times accessing other data).
	//Internally creates it if needed.

#ifdef DO_ROADS
	if (m_roadBuffer) {
		m_roadBuffer->allocateRoadBuffers();
		m_roadBuffer->loadRoads();
	}
#endif

	if (TheTacticalView)
	{	TheTacticalView->forceRedraw();	//force map to update itself for the current camera position.
		//for some reason we need to do it twice otherwise we sometimes end up with a black map until
		//the player moves.
		TheTacticalView->forceRedraw();
	}
}

//=============================================================================
// HeightMapRenderObjClass::updateMacroTexture
//=============================================================================
/** Updates the macro noise/lightmap texture (pass 3) */
//=============================================================================
void HeightMapRenderObjClass::updateMacroTexture(AsciiString textureName)
{
	m_macroTextureName = textureName;
	// Release texture.
	REF_PTR_RELEASE(m_stageThreeTexture);
	// Reallocate texture.
	m_stageThreeTexture=NEW LightMapTerrainTextureClass(m_macroTextureName);	
}

//=============================================================================
// HeightMapRenderObjClass::reset
//=============================================================================
/** Updates the macro noise/lightmap texture (pass 3) */
//=============================================================================
void HeightMapRenderObjClass::reset(void)
{
	if (m_treeBuffer) {
		m_treeBuffer->clearAllTrees();
	}
	clearAllScorches();
#ifdef TEST_CUSTOM_EDGING
	if (m_customEdging)
		m_customEdging ->clearAllEdging();
#endif
#ifdef DO_ROADS
	if (m_roadBuffer) {
		m_roadBuffer->clearAllRoads();
	}
#endif
	if (m_bridgeBuffer) {
		m_bridgeBuffer->clearAllBridges();
	}

	if (m_bibBuffer) {
		m_bibBuffer->clearAllBibs();
	}

	m_showAsVisibleCliff.clear();

	if (m_shroud)
	{	m_shroud->reset();
		m_shroud->setBorderShroudLevel((W3DShroudLevel)TheGlobalData->m_shroudAlpha);	//assume border is always black at start.
	}
}

/**@todo: Ray intersection needs to be optimized with some sort of grid-tracing
(ala line drawing).  We should also try making the search in a front->back order
relative to the ray so we can early exit as soon as we have a hit.
*
//=============================================================================
// HeightMapRenderObjClass::Cast_Ray
//=============================================================================
/** Return intersection of a ray with the heightmap mesh.
This is a quick version that just checks every polygon inside
a 2D bounding rectangle of the ray projected onto the heightfield plane.
For most of our view-picking cases the ray in almost perpendicular to the
map plane so this is very quick (small bounding box).  But it can become slow
for arbitrary rays such as those used in AI visbility checks.(2 units on
opposite corners of the map would check every polygon in the map).
*/
//=============================================================================
bool HeightMapRenderObjClass::Cast_Ray(RayCollisionTestClass & raytest)
{
	TriClass tri;
	Bool hit = false;
	Int X,Y;
	Vector3 normal,P0,P1,P2,P3;

	if (!m_map)
		return false;	//need valid pointer to heightmap samples
//HeightSampleType *pData = m_map->getDataPtr();
	//Clip ray to extents of heightfield
	AABoxClass hbox;
	LineSegClass lineseg,lineseg2;
	CastResultStruct	result;
	Int StartCellX = 0;
	Int EndCellX = 0;
 	Int StartCellY = 0;
	Int EndCellY = 0;
	const Int overhang = 2*VERTEX_BUFFER_TILE_LENGTH+m_map->getBorderSize(); // Allow picking past the edge for scrolling & objects.
 	Vector3 minPt(MAP_XY_FACTOR*(-overhang), MAP_XY_FACTOR*(-overhang), -MAP_XY_FACTOR);
	Vector3 maxPt(MAP_XY_FACTOR*(m_map->getXExtent()+overhang), 
		MAP_XY_FACTOR*(m_map->getYExtent()+overhang), MAP_HEIGHT_SCALE*m_map->getMaxHeightValue()+MAP_XY_FACTOR);
	MinMaxAABoxClass mmbox(minPt, maxPt);
	hbox.Init(mmbox);

	lineseg=raytest.Ray;

	//Set initial ray endpoints
	P0 = raytest.Ray.Get_P0();
	P1 = raytest.Ray.Get_P1();
	result.ComputeContactPoint=true;

	Int p;
	for (p=0; p<3; p++) {
		//find intersection point of ray and terrain bounding box
		result.Reset();
		result.ComputeContactPoint=true;
		if (CollisionMath::Collide(lineseg,hbox,&result))
		{	//ray intersects terrain or starts inside the terrain.
			if (!result.StartBad)	//check if start point inside terrain
				P0 = result.ContactPoint;			//make intersection point the new start of the ray.

			//reverse direction of original ray and clip again to extent of
			//heightmap
			result.Fraction=1.0f;	//reset the result
			result.StartBad=false;
			lineseg2.Set(lineseg.Get_P1(),lineseg.Get_P0());	//reverse line segment
			if (CollisionMath::Collide(lineseg2,hbox,&result))
			{	if (!result.StartBad)	//check if end point inside terrain
					P1 = result.ContactPoint;	//make intersection point the new end pont of ray
			}
		} else {
			if (p==0) return(false);
			break;
		}

		// Take the 2D bounding box of ray and check heights
		// inside this box for intersection.
		if (P0.X > P1.X) {	//flip start/end points
			StartCellX = REAL_TO_INT_FLOOR(P1.X/MAP_XY_FACTOR);
			EndCellX = REAL_TO_INT_CEIL(P0.X/MAP_XY_FACTOR);
		}	else {
			StartCellX = REAL_TO_INT_FLOOR(P0.X/MAP_XY_FACTOR);
			EndCellX = REAL_TO_INT_CEIL(P1.X/MAP_XY_FACTOR);
		}
		if (P0.Y > P1.Y) {	//flip start/end points
			StartCellY = REAL_TO_INT_FLOOR(P1.Y/MAP_XY_FACTOR);
			EndCellY = REAL_TO_INT_CEIL(P0.Y/MAP_XY_FACTOR);
		}	else {
			StartCellY = REAL_TO_INT_FLOOR(P0.Y/MAP_XY_FACTOR);
			EndCellY = REAL_TO_INT_CEIL(P1.Y/MAP_XY_FACTOR);
		}

		Int i, j, minHt, maxHt;

		minHt = m_map->getMaxHeightValue();
		maxHt = 0;

		for (j=StartCellY; j<=EndCellY; j++) {
			for (i=StartCellX; i<=EndCellX; i++) {
				Short cur = getClipHeight(i+m_map->getBorderSize(),j+m_map->getBorderSize());
				if (cur<minHt) minHt = cur;
				if (maxHt<cur) maxHt = cur;
			}
		}
		Vector3 minPt(MAP_XY_FACTOR*(StartCellX-1), MAP_XY_FACTOR*(StartCellY-1), MAP_HEIGHT_SCALE*(minHt-1));
		Vector3 maxPt(MAP_XY_FACTOR*(EndCellX+1), MAP_XY_FACTOR*(EndCellY+1), MAP_HEIGHT_SCALE*(maxHt+1));
		MinMaxAABoxClass mmbox(minPt, maxPt);
		hbox.Init(mmbox);
	}

	raytest.Result->ComputeContactPoint=true;	//tell CollisionMath that we need point.

	// Adjust indexes into the bordered height map.

	StartCellX += m_map->getBorderSize();
	EndCellX += m_map->getBorderSize();
	StartCellY += m_map->getBorderSize();
	EndCellY += m_map->getBorderSize();

	Int offset;
	for (offset = 1; offset < 5; offset *= 3) {
		for (Y=StartCellY-offset; Y<=EndCellY+offset; Y++) { 
			//if (Y<0) continue;
			//if (Y>=m_map->getYExtent()-1) continue;

			for (X=StartCellX-offset; X<=EndCellX+offset; X++) {
				//test the 2 triangles in this cell
				//	3-----2
				//  |    /|
				//  |  /  |
				//	|/    |
				//  0-----1

				//bottom triangle first
				P0.X=ADJUST_FROM_INDEX_TO_REAL(X);
				P0.Y=ADJUST_FROM_INDEX_TO_REAL(Y);
				P0.Z=MAP_HEIGHT_SCALE*(float)getClipHeight(X, Y);

				P1.X=ADJUST_FROM_INDEX_TO_REAL(X+1);
				P1.Y=ADJUST_FROM_INDEX_TO_REAL(Y);
				P1.Z=MAP_HEIGHT_SCALE*(float)getClipHeight(X+1, Y);

				P2.X=ADJUST_FROM_INDEX_TO_REAL(X+1);
				P2.Y=ADJUST_FROM_INDEX_TO_REAL(Y+1);
				P2.Z=MAP_HEIGHT_SCALE*(float)getClipHeight(X+1, Y+1);

				P3.X=ADJUST_FROM_INDEX_TO_REAL(X);
				P3.Y=ADJUST_FROM_INDEX_TO_REAL(Y+1);
				P3.Z=MAP_HEIGHT_SCALE*(float)getClipHeight(X, Y+1);


				tri.V[0] = &P0; 
				tri.V[1] = &P1;
				tri.V[2] = &P2;
				
				tri.N = &normal;

				tri.Compute_Normal();

				hit = hit || (Bool)CollisionMath::Collide(raytest.Ray, tri, raytest.Result);

				if (raytest.Result->StartBad)
					return true;

				//top triangle
				tri.V[0] = &P2; 
				tri.V[1] = &P3;
				tri.V[2] = &P0;
				
				tri.N = &normal;

				tri.Compute_Normal();

				hit = hit || (Bool)CollisionMath::Collide(raytest.Ray, tri, raytest.Result);

				if (hit)
					raytest.Result->SurfaceType = SURFACE_TYPE_DEFAULT;	///@todo: WW3D uses this to return dirt, grass, etc.  Do we need this?
			}
			// Don't break.  It is possible to intersect 2 triangles, and the second is closer. if (hit) break;
		}
		// Don't break.  It is possible to intersect 2 triangles, and the second is closer. if (hit) break;
	}
	return hit;
}

//=============================================================================
// HeightMapRenderObjClass::getHeightMapHeight
//=============================================================================
/** return the height and normal of the triangle plane containing given location within heightmap. */
//=============================================================================
Real HeightMapRenderObjClass::getHeightMapHeight(Real x, Real y, Coord3D* normal) const
{
	if (m_map == NULL)
	{
		if (normal)
		{	
			// return a default normal pointing up
			normal->x = 0.0f;
			normal->y = 0.0f;
			normal->z = 1.0f;
		}
		return 0;
	}

	float height;

	//	3-----2
	//  |    /|
	//  |  /  |
	//	|/    |
	//  0-----1
	//Find surrounding grid points
	
	const Real MAP_XY_FACTOR_INV = 1.0f / MAP_XY_FACTOR;

	float xdiv = x * MAP_XY_FACTOR_INV;
	float ydiv = y * MAP_XY_FACTOR_INV;

	float ixf = floorf(xdiv);
	float iyf = floorf(ydiv);

	float fx = xdiv - ixf; //get fraction
	float fy = ydiv - iyf; //get fraction

	// since ixf & iyf are already floor'ed, we can use the fastest f->i conversion we have...
	Int	ix = REAL_TO_INT_FLOOR(ixf) + m_map->getBorderSize();
	Int	iy = REAL_TO_INT_FLOOR(iyf) + m_map->getBorderSize();
	Int xExtent = m_map->getXExtent();

	// Check for extent-3, not extent-1: we go into the next row/column of data for smoothed triangle points, so extent-1
	// goes off the end...
	if (ix > (xExtent-3) || iy > (m_map->getYExtent()-3) || iy < 1 || ix < 1)
	{	
		// sample point is not on the heightmap
		if (normal)
		{	
			// return a default normal pointing up
			normal->x = 0.0f;
			normal->y = 0.0f;
			normal->z = 1.0f;
		}
		return getClipHeight(ix, iy) * MAP_HEIGHT_SCALE;
	}

	const UnsignedByte* data = m_map->getDataPtr();
	int idx = ix + iy*xExtent;
	float p0 = data[idx];
	float p2 = data[idx + xExtent + 1];
	if (fy > fx) // test if we are in the upper triangle
	{	
		float p3 = data[idx + xExtent];
		height = (p3 + (1.0f-fy)*(p0-p3) + fx*(p2-p3)) * MAP_HEIGHT_SCALE;
	}
	else
	{	
		// we are in the lower triangle
		float p1 = data[idx + 1];
		height = (p1 + fy*(p2-p1) + (1.0f-fx)*(p0-p1)) * MAP_HEIGHT_SCALE;
	}
	if (normal) {
		//		9		  8
		//
		//10	3-----2		7
		//	  |    /|
		//	  |  /  |
		//		|/    |
		//11	0-----1		6
		//
		//		4			5
		//Find surrounding grid points for smoothed normals.
 		int idx4 = ix + (iy-1)*xExtent;
 		int idx0 = ix + iy*xExtent;
 		int idx3 = ix + iy*xExtent+xExtent;
		int idx9 = ix + (iy+2)*xExtent;
		UnsignedByte d0, d1, d2, d3, d4, d5, d6, d7, d8, d9, d10, d11;
		d0 = data[idx0];
		d1 = data[idx0+1];
		d2 = data[idx3+1];
		d3 = data[idx3];
		d4 = data[idx4];
		d5 = data[idx4+1];
		d6 = data[idx0+2];
		d7 = data[idx3+2];
		d8 = data[idx9+1];
		d9 = data[idx9];
		d10 = data[idx3-1];
		d11 = data[idx0-1];

		Real deltaZ_X0 = d1-d11;
		Real deltaZ_X1 = d6-d0;
		Real deltaZ_X2 = d7-d3;
		Real deltaZ_X3 = d6-d0;

		Real deltaZ_Y0 = d3-d4;
		Real deltaZ_Y1 = d2-d5;
		Real deltaZ_Y2 = d8-d1;
		Real deltaZ_Y3 = d9-d0;

		// Interpolate to get the smoothed valued.
		Real deltaZ_X_Left = deltaZ_X0*(1.0f-fx) + fx*deltaZ_X3;
		Real deltaZ_X_Right = deltaZ_X1*(1.0f-fx) + fx*deltaZ_X2;
		Real deltaZ_X = deltaZ_X_Left*(1.0-fy) + fy*deltaZ_X_Right;

		Real deltaZ_Y_Left = deltaZ_Y0*(1.0f-fx) + fx*deltaZ_Y3;
		Real deltaZ_Y_Right = deltaZ_Y1*(1.0f-fx) + fx*deltaZ_Y2;
		Real deltaZ_Y = deltaZ_Y_Left*(1.0-fy) + fy*deltaZ_Y_Right;



			Vector3 l2r, n2f, normalAtTexel;
			l2r.Set(2*MAP_XY_FACTOR/MAP_HEIGHT_SCALE, 0, deltaZ_X);
			n2f.Set(0, 2*MAP_XY_FACTOR/MAP_HEIGHT_SCALE, deltaZ_Y);
			Vector3::Normalized_Cross_Product(l2r,n2f, &normalAtTexel);
			normal->x = normalAtTexel.X;
			normal->y = normalAtTexel.Y;
			normal->z = normalAtTexel.Z;

	}


	return height;
}

//=============================================================================
Bool HeightMapRenderObjClass::isClearLineOfSight(const Coord3D& pos, const Coord3D& posOther) const
{
	if (m_map == NULL)
		return false;	// doh. should not happen.

#define DO_BRESENHAM
#ifdef DO_BRESENHAM

	/*
		this is WAY faster, though not quite as accurate... however, the inaccuracy
		is pretty minimal, so we really should force other code to live with it. (srj)
	*/
	const Real MAP_XY_FACTOR_INV = 1.0f / MAP_XY_FACTOR;

	Int borderSize = m_map->getBorderSize();
	Int start_x = REAL_TO_INT_FLOOR(pos.x * MAP_XY_FACTOR_INV) + borderSize;
	Int start_y = REAL_TO_INT_FLOOR(pos.y * MAP_XY_FACTOR_INV) + borderSize;
	Int end_x = REAL_TO_INT_FLOOR(posOther.x * MAP_XY_FACTOR_INV) + borderSize;
	Int end_y = REAL_TO_INT_FLOOR(posOther.y * MAP_XY_FACTOR_INV) + borderSize;
	Int delta_x = abs(end_x - start_x);			// The difference between the x's
	Int delta_y = abs(end_y - start_y);			// The difference between the y's
	Int x = start_x;												// Start x off at the first pixel
	Int y = start_y;												// Start y off at the first pixel

	Int xinc1, xinc2;
	if (end_x >= start_x)								// The x-values are increasing
	{
		xinc1 = 1;
		xinc2 = 1;
	}
	else																// The x-values are decreasing
	{
		xinc1 = -1;
		xinc2 = -1;
	}

	Int yinc1, yinc2;
	if (end_y >= start_y)               // The y-values are increasing
	{
		yinc1 = 1;
		yinc2 = 1;
	}
	else																// The y-values are decreasing
	{
		yinc1 = -1;
		yinc2 = -1;
	}

	Int den, num, numadd, numpixels;

	Bool checkY = true;
	if (delta_x >= delta_y)							// There is at least one x-value for every y-value
	{
		xinc1 = 0;												// Don't change the x when numerator >= denominator
		yinc2 = 0;												// Don't change the y for every iteration
		den = delta_x;
		num = delta_x / 2;
		numadd = delta_y;
		numpixels = delta_x;							// There are more x-values than y-values
	}
	else																// There is at least one y-value for every x-value
	{
		checkY = false;
		xinc2 = 0;												// Don't change the x for every iteration
		yinc1 = 0;												// Don't change the y when numerator >= denominator
		den = delta_y;
		num = delta_y / 2;
		numadd = delta_x;
		numpixels = delta_y;							// There are more y-values than x-values
	}

	Real nsInv = 1.0f / numpixels;
	Real z = pos.z;
	Real dz = posOther.z - z;
	Real zinc = dz * nsInv;

	Bool result = true;
	const UnsignedByte* data = m_map->getDataPtr();
	Int xExtent = m_map->getXExtent();
	Int yExtent = m_map->getYExtent();
	for (Int curpixel = 0; curpixel < numpixels; curpixel++)
	{
		if (x < 0 || 
				y < 0 ||
				x >= xExtent-1 ||
				y >= yExtent-1)
		{
			// once we go off the map, we're done
			break;
		}

		Int idx = x + y*xExtent;
		float height = data[idx];
		height = __max(height, data[idx + 1]);
		height = __max(height, data[idx + xExtent]);
		height = __max(height, data[idx + xExtent + 1]);
		height *= MAP_HEIGHT_SCALE;

		// if terrainHeight > z, we can't see, so punt.
		// add a little fudge to account for slop.
		const Real LOS_FUDGE = 0.5f;
		if (height > z + LOS_FUDGE)
		{
			result = false;
			break;
		}

		// we're above the max height of the terrain and still looking up, so we're done.
		// (don't bother for reverse test, since that doesn't generally happen)
		if (z >= getMaxHeight() && zinc > 0.0f)
		{
			break;
		}

		z += zinc;

		// continue with the maintenance.
		num += numadd;										// Increase the numerator by the top of the fraction
		if (num >= den)										// Check if numerator >= denominator
		{
			num -= den;											// Calculate the new numerator value
			x += xinc1;											// Change the x as appropriate
			y += yinc1;											// Change the y as appropriate
		}
		x += xinc2;												// Change the x as appropriate
		y += yinc2;												// Change the y as appropriate
	}
	
	return result;

#else

	// walk a line from obj to objOther and
	// find the highest point in between 'em. while
	// we're doing this, also estimate the point on the
	// line at the same x,y as the high-terrain-point.

	Real fx = pos.x;
	Real fy = pos.y;
	Real fz = pos.z;
	Real fdx = posOther.x - fx;
	Real fdy = posOther.y - fy;
	Real fdz = posOther.z - fz;

	// What's the largest step size that will be accurate enough?
	// Currently we use a step size of about 2 "feet", which
	// seems acceptable accuracy. If performance here is inadequate,
	// we can try increasing the step size, but be sure to retest
	// accuracy.
	Real len = ceilf(sqrtf(fdx*fdx + fdy*fdy));
	const Real STEP_LEN = 2.0f;
	Int numSteps = REAL_TO_INT_CEIL(len / STEP_LEN);
	if (numSteps < 1) numSteps = 1;
	Real fnsInv = 1.0f / numSteps;
	Real fxinc = fdx * fnsInv;
	Real fyinc = fdy * fnsInv;
	Real fzinc = fdz * fnsInv;
	while (numSteps--)
	{
		Real terrainHeight = getHeightMapHeight( fx, fy, NULL );

		// if terrainHeight > fz, we can't see, so punt.
		// add a little fudge to account for slop.
		const Real LOS_FUDGE = 0.5f;
		if (terrainHeight > fz + LOS_FUDGE)
		{
			return false;
		}

		// we're above the max height of the terrain and still looking up, so we're done.
		// (don't bother for reverse test, since that doesn't generally happen)
		if (fz >= getMaxHeight() && fzinc > 0.0f)
		{
			return true;
		}

		fx += fxinc;
		fy += fyinc;
		fz += fzinc;

	}

	return true;
#endif
}

//=============================================================================
// HeightMapRenderObjClass::getMaxCellHeight
//=============================================================================
/** Returns maximum height of the 4 corners containing the given point */
//=============================================================================
Real HeightMapRenderObjClass::getMaxCellHeight(Real x, Real y) const
{
	float p0,p1,p2,p3;
	float height;

	//	3-----2
	//  |    /|
	//  |  /  |
	//	|/    |
	//  0-----1
	//Find surrounding grid points

	if (m_map == NULL)
	{	//sample point is not on the heightmap
		return 0.0f;	//return default height
	}
	Int offset = 1;
	Int iX = x/MAP_XY_FACTOR;
	Int iY = y/MAP_XY_FACTOR;
	iX += m_map->getBorderSize();
	iY += m_map->getBorderSize();
	if (iX<0) iX = 0;
	if (iY<0) iY = 0;
	if (iX >= (m_map->getXExtent()-1)) {
		iX = m_map->getXExtent()-2;
	}
	if (iY >= (m_map->getYExtent()-1)) {
		iY = m_map->getYExtent()-2;
	}
	if (m_halfResMesh) {
		offset = 2;
		iX = (iX/2)*2;
		iY = (iY/2)*2;
	}
	UnsignedByte *data = m_map->getDataPtr();
	p0=data[iX+iY*m_map->getXExtent()]*MAP_HEIGHT_SCALE;
	p1=data[(iX+offset)+iY*m_map->getXExtent()]*MAP_HEIGHT_SCALE;
	p2=data[(iX+offset)+(iY+offset)*m_map->getXExtent()]*MAP_HEIGHT_SCALE;
	p3=data[iX+(iY+offset)*m_map->getXExtent()]*MAP_HEIGHT_SCALE;

	height=p0;
	height=__max(height,p1);
	height=__max(height,p2);
	height=__max(height,p3);

	return height;
}

//=============================================================================
// HeightMapRenderObjClass::isCliffCell
//=============================================================================
/** Returns true if the cell containing the point is a cliff cell */
//=============================================================================
Bool HeightMapRenderObjClass::isCliffCell(Real x, Real y)
{

	if (m_map == NULL)
	{	//sample point is not on the heightmap
		return false;
	}
	Int iX = x/MAP_XY_FACTOR;
	Int iY = y/MAP_XY_FACTOR;
	iX += m_map->getBorderSize();
	iY += m_map->getBorderSize();
	if (iX<0) iX = 0;
	if (iY<0) iY = 0;
	if (iX >= (m_map->getXExtent()-1)) {
		iX = m_map->getXExtent()-2;
	}
	if (iY >= (m_map->getYExtent()-1)) {
		iY = m_map->getYExtent()-2;
	}
	return m_map->getCliffState(iX, iY);
}

//=============================================================================
//=============================================================================
Bool HeightMapRenderObjClass::showAsVisibleCliff(Int xIndex, Int yIndex) const
{
	Int xSize = m_map->getXExtent();

	return m_showAsVisibleCliff[xIndex + yIndex * xSize];
}

//=============================================================================
//=============================================================================
Bool HeightMapRenderObjClass::evaluateAsVisibleCliff(Int xIndex, Int yIndex, Real valuesGreaterThanRad)
{
	// This one never changes, so don't bother recomputing it.
	static const Real distance[4] = 
	{
		0.0f,
		1.0 * MAP_XY_FACTOR,
		sqrt(2.0f) * MAP_XY_FACTOR,
		1.0 * MAP_XY_FACTOR,
	};

	// Note: getHeight will protect us from going out of bounds by returning 0 if we give it
	// a value outside of its bounds.
	UnsignedByte bytes[4] = 
	{ 
		m_map->getHeight(xIndex + 0, yIndex + 0), 
		m_map->getHeight(xIndex + 1, yIndex + 0), 
		m_map->getHeight(xIndex + 1, yIndex + 1), 
		m_map->getHeight(xIndex + 0, yIndex + 1),
	};

	Real heights[4] = 
	{
		INT_TO_REAL(bytes[0]) * MAP_HEIGHT_SCALE,
		INT_TO_REAL(bytes[1]) * MAP_HEIGHT_SCALE,
		INT_TO_REAL(bytes[2]) * MAP_HEIGHT_SCALE,
		INT_TO_REAL(bytes[3]) * MAP_HEIGHT_SCALE,
	};

	Bool anyImpassable = FALSE;	

	for (Int i = 1; i < 4 && !anyImpassable; ++i) {
		if (fabs((heights[i] - heights[0]) / distance[i]) > valuesGreaterThanRad) {
			anyImpassable = TRUE;
		}
	}

	return anyImpassable;
}

//=============================================================================
// HeightMapRenderObjClass::oversizeTerrain
//=============================================================================
/** Sets the terrain oversize amount. */
//=============================================================================
void HeightMapRenderObjClass::oversizeTerrain(Int tilesToOversize) 
{
	Int width = WorldHeightMap::NORMAL_DRAW_WIDTH;
	Int height = WorldHeightMap::NORMAL_DRAW_HEIGHT;
	if (tilesToOversize>0 && tilesToOversize<5) 
	{
		width += 32*tilesToOversize;
		height += 32*tilesToOversize;
		if (width>m_map->getXExtent()) 
			width = m_map->getXExtent();
		if (height>m_map->getYExtent()) 
			height = m_map->getYExtent();
	}
	Int dx = width-m_map->getDrawWidth();
	Int dy = height-m_map->getDrawHeight();
 	m_map->setDrawWidth(width);
	m_map->setDrawHeight(height);
	dx /= 2;
	dy /= 2;
	Int newOrgX = m_map->getDrawOrgX()-dx;
	Int newOrgy = m_map->getDrawOrgY()-dy;
	if (newOrgX<0) newOrgX=0;
	if (newOrgy<0) newOrgy=0;
	m_map->setDrawOrg(newOrgX,newOrgy);
	m_originX = 0;
	m_originY = 0;
	if (m_shroud)
		m_shroud->reset();
	//delete m_shroud;
	//m_shroud = NULL;
	initHeightData(m_map->getDrawWidth(), m_map->getDrawHeight(), m_map, NULL);		 
	m_needFullUpdate = true;
}



//=============================================================================
// HeightMapRenderObjClass::Get_Obj_Space_Bounding_Sphere
//=============================================================================
/** WW3D method that returns object bounding sphere used in frustum culling*/
//=============================================================================
void HeightMapRenderObjClass::Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const
{
	Vector3	ObjSpaceCenter((float)m_x*0.5f*MAP_XY_FACTOR,(float)m_y*0.5f*MAP_XY_FACTOR,(float)m_minHeight+(m_maxHeight-m_minHeight)*0.5f);
	float length = ObjSpaceCenter.Length();
	
	if (m_map) {
		ObjSpaceCenter.X += m_map->getDrawOrgX()*MAP_XY_FACTOR;
		ObjSpaceCenter.Y += m_map->getDrawOrgY()*MAP_XY_FACTOR;
	}
	sphere.Init(ObjSpaceCenter, length);
}

//=============================================================================
// HeightMapRenderObjClass::Get_Obj_Space_Bounding_Box
//=============================================================================
/** WW3D method that returns object bounding box used in collision detection*/
//=============================================================================
void HeightMapRenderObjClass::Get_Obj_Space_Bounding_Box(AABoxClass & box) const
{
	Vector3	minPt(0,0,m_minHeight);
	Vector3	maxPt((float)m_x*MAP_XY_FACTOR,(float)m_y*MAP_XY_FACTOR,(float)m_maxHeight);
	if (m_map) {
		minPt.X += m_map->getDrawOrgX()*MAP_XY_FACTOR;
		minPt.Y += m_map->getDrawOrgY()*MAP_XY_FACTOR;
		maxPt.X += m_map->getDrawOrgX()*MAP_XY_FACTOR;
		maxPt.Y += m_map->getDrawOrgY()*MAP_XY_FACTOR;
	}
	MinMaxAABoxClass minMaxBox(minPt, maxPt);

	box.Init(minMaxBox);
}

//-------------------------------------------------------------------------------------------------
/** Get the 3D extent of the terrain visible through the camera.  Return value
	is false if no part of terrain is visible.  This function returns a worse
	case bounding volume based on lowest/highest points in entire terrain.  It
	does not optimize the volume to heights actually visible.  Unlike some of
	the other methods, this function is guaranteed not to miss any visible
	polygons.  The ignoreMaxHeight flag is used to return a box that uses the
	camera position as the maximum height instead of the terrain - good for getting
	a volume enclosing things that can float above terrain.
 */
//-------------------------------------------------------------------------------------------------
Bool HeightMapRenderObjClass::getMaximumVisibleBox(const FrustumClass &frustum, AABoxClass *box, Bool ignoreMaxHeight)
{
	//create a plane from the lowest point on the terrain
	PlaneClass	groundPlane(Vector3(0,0,1), m_minHeight);

	//clip each side of the view frustum to ground plane
	float clipFraction;
	Vector3 ClippedCorners[8];
	ClippedCorners[0]=frustum.Corners[0];
	for (Int i=0; i<4; i++)
	{	ClippedCorners[i]=frustum.Corners[i];
		if (groundPlane.Compute_Intersection(frustum.Corners[i],frustum.Corners[i+4],&clipFraction))
		{	//edge intersects the terrain
			ClippedCorners[i+4]=frustum.Corners[i]+(frustum.Corners[i+4]-frustum.Corners[i])*clipFraction;
		}
		else
			ClippedCorners[i+4]=frustum.Corners[i+4];
	}

	if (box)
		box->Init(ClippedCorners,8);

	return TRUE;
}

//=============================================================================
// HeightMapRenderObjClass::Class_ID
//=============================================================================
/** returns the class id, so the scene can tell what kind of render object it has. */
//=============================================================================
Int HeightMapRenderObjClass::Class_ID(void) const
{
	return RenderObjClass::CLASSID_TILEMAP;
}

//=============================================================================
// HeightMapRenderObjClass::Clone
//=============================================================================
/** Not used, but required virtual method. */
//=============================================================================
RenderObjClass *	 HeightMapRenderObjClass::Clone(void) const
{
	assert(false);
	return NULL;
}

//=============================================================================
// HeightMapRenderObjClass::loadRoadsAndBridges
//=============================================================================
/** Loads the roads from the map objects. */
//=============================================================================
void HeightMapRenderObjClass::loadRoadsAndBridges(W3DTerrainLogic *pTerrainLogic, Bool saveGame)
{	
	if (DX8Wrapper::_Get_D3D_Device8() && (DX8Wrapper::_Get_D3D_Device8()->TestCooperativeLevel()) != D3D_OK)
		return;	//device not ready to render anything

#ifdef DO_ROADS
	if (m_roadBuffer) {
		m_roadBuffer->loadRoads();
	}
#endif
	if (m_bridgeBuffer) {
		m_bridgeBuffer->loadBridges(pTerrainLogic, saveGame);
	}
}

// ============================================================================
// HeightMapRenderObjClass::worldBuilderUpdateBridgeTowers
// ============================================================================
/** The worldbuilder has it's own method here to update the visual representation
	* of the bridge towers */
// ============================================================================
void HeightMapRenderObjClass::worldBuilderUpdateBridgeTowers( W3DAssetManager *assetManager,
																															SimpleSceneClass *scene )
{

	if( m_bridgeBuffer )
		m_bridgeBuffer->worldBuilderUpdateBridgeTowers( assetManager, scene );

}

void HeightMapRenderObjClass::setShoreLineDetail(void)
{
	if (!m_map)
		return;

	Int m_mapDX=m_map->getXExtent();
	Int m_mapDY=m_map->getYExtent();

	//Find all shoreline tiles so they can get extra alpha blend
	updateShorelineTiles(0,0,m_mapDX-1,m_mapDY-1,m_map);
}

/**Scan through our map and record all tiles which cross a water plane and are within visible depth under
water.*/
void HeightMapRenderObjClass::updateShorelineTiles(Int minX, Int minY, Int maxX, Int maxY, WorldHeightMap *pMap)
{
	Int border = pMap->getBorderSize();

	//Clamp region to valid terrain tiles
	if (minX<0)
		minX = 0;
	if (minY<0)
		minY = 0;
	if (maxX > (pMap->getXExtent() - 1))
		maxX = (pMap->getXExtent() - 1);
	if (maxY > (pMap->getYExtent() - 1))
		maxY = (pMap->getYExtent() - 1);

	if (!m_shoreLineTilePositions)
	{	//Need to allocate memory
		m_shoreLineTilePositions = NEW shoreLineTileInfo[DEFAULT_MAX_MAP_SHORELINE_TILES];
		m_shoreLineTilePositionsSize = DEFAULT_MAX_MAP_SHORELINE_TILES;
	}
	
	//Find list of all extra blend tiles used on map.  These are tiles with 3 materials/textures
	//over the same tile and require an extra render pass.

	//First remove any existing extra blend tiles within this partial region
	Int j=0;
	for (; j<m_numShoreLineTiles; j++)
	{	Int x = m_shoreLineTilePositions[j].m_xy & 0xffff;
		Int y = m_shoreLineTilePositions[j].m_xy >> 16;
		if (x >= minX && x < maxX &&
			y >= minY && y < maxY)
		{	//this tile is inside region being updated so remove it by shifting tile array
			memcpy(m_shoreLineTilePositions+j,m_shoreLineTilePositions+j+1,(m_numShoreLineTiles-1-j)*sizeof(shoreLineTileInfo));
			m_numShoreLineTiles--;
			j--;	//look at current tile again since it was removed.
		}
	}

	if (TheWaterTransparency->m_transparentWaterDepth == 0 || !TheGlobalData->m_showSoftWaterEdge)
		return;

	Int waterSide;
	Real waterZ0,waterZ1,waterZ2,waterZ3;
	Real terrainZ0, terrainZ1, terrainZ2, terrainZ3;
	//Figure out maximum depth of water before we reach the m_minWaterOpacity value.  Depths greater than this don't need
	//custom shoreline tiles because they will get their opacity from the default value stored in the frame buffer during
	//a screen clear operation.
	Real transparentDepth=TheWaterTransparency->m_transparentWaterDepth*TheWaterTransparency->m_minWaterOpacity;
	Real depthScaleFactor = 1.0f/transparentDepth;

	for (j=minY; j<maxY; j++)
		for (Int i=minX; i<maxX; i++)
		{
				waterSide=(waterZ0=TheWaterRenderObj->getWaterHeight((i-border)*MAP_XY_FACTOR,(j-border)*MAP_XY_FACTOR)) > ((terrainZ0=MAP_HEIGHT_SCALE*pMap->getHeight(i,j)));
				waterSide |=((waterZ1=TheWaterRenderObj->getWaterHeight((i-border+1)*MAP_XY_FACTOR,(j-border)*MAP_XY_FACTOR)) > ((terrainZ1=MAP_HEIGHT_SCALE*pMap->getHeight(i+1,j)))) << 1;
				waterSide |=((waterZ2=TheWaterRenderObj->getWaterHeight((i-border+1)*MAP_XY_FACTOR,(j-border+1)*MAP_XY_FACTOR)) > ((terrainZ2=MAP_HEIGHT_SCALE*pMap->getHeight(i+1,j+1)))) << 2;
				waterSide |=((waterZ3=TheWaterRenderObj->getWaterHeight((i-border)*MAP_XY_FACTOR,(j-border+1)*MAP_XY_FACTOR)) > ((terrainZ3=MAP_HEIGHT_SCALE*pMap->getHeight(i,j+1)))) << 3;

				if (!waterSide || (waterZ0*waterZ1*waterZ2*waterZ3) <= 0)
					continue;	//all verts are on positive (surface) side of water so don't need blending.  Or one of them is outside the water plane bounds (waterHeight <= 0!)

				//Check if mix of under/over water vertices or some vertices within depth fade region.
				if (waterSide < 0xf || (waterZ0 - terrainZ0) < transparentDepth ||
					(waterZ1 - terrainZ1) < transparentDepth || (waterZ2 - terrainZ2) < transparentDepth
					|| (waterZ3 - terrainZ3) < transparentDepth)
				{	//add tile to set that needs shoreline blending.
					if (m_numShoreLineTiles >= m_shoreLineTilePositionsSize)
					{	//no more room to store extra blend tiles so enlarge the buffer.
						shoreLineTileInfo *tempPositions=NEW shoreLineTileInfo[m_shoreLineTilePositionsSize+512];
						memcpy(tempPositions, m_shoreLineTilePositions, m_shoreLineTilePositionsSize*sizeof(shoreLineTileInfo));
						delete [] m_shoreLineTilePositions;
						//enlarge by more tiles to reduce memory trashing
						m_shoreLineTilePositions = tempPositions;
						m_shoreLineTilePositionsSize += 512;
					}
					//Pack x and y position into single integer since maps are limited in size
					shoreLineTileInfo *shoreInfo=&m_shoreLineTilePositions[m_numShoreLineTiles];
					shoreInfo->m_xy=i | (j <<16);
					shoreInfo->t0=(waterZ0 - terrainZ0)*depthScaleFactor;
					shoreInfo->t1=(waterZ1 - terrainZ1)*depthScaleFactor;
					shoreInfo->t2=(waterZ2 - terrainZ2)*depthScaleFactor;
					shoreInfo->t3=(waterZ3 - terrainZ3)*depthScaleFactor;
					m_numShoreLineTiles++;
				}
		}
}

/** Generate a lookup table for arbitrary angled impassable area viewing. */
void HeightMapRenderObjClass::updateViewImpassableAreas(Bool partial, Int minX, Int maxX, Int minY, Int maxY)
{
	Int xSize = m_map->getXExtent();
	Int ySize = m_map->getYExtent();
	if (m_showAsVisibleCliff.size() != xSize * ySize) {
		m_showAsVisibleCliff.resize(xSize * ySize);
	}

	if (!partial) {
		minX = 0;
		minY = 0;
		maxX = xSize;
		maxY = ySize;
	}

	// save calculating the tangent over and over again.
	Real tanImpassableRad = tan(m_curImpassableSlope / 360.f * 2 * PI);
	for (Int j = minY; j < maxY; ++j) {
		for (Int i = minX; i < maxX; ++i) {
			m_showAsVisibleCliff[i + j * xSize] = evaluateAsVisibleCliff(i, j, tanImpassableRad);
		}
	}
}

/** Generate a lookup table which can be used to generate an
alpha value from a given set of uv coordinates.  Currently used
for smoothing water/terrain border*/
void HeightMapRenderObjClass::initDestAlphaLUT(void)
{
	if (!m_destAlphaTexture)
		return;

	SurfaceClass *surf=m_destAlphaTexture->Get_Surface_Level();

	if (surf)
	{
		Int pitch;
		UnsignedInt *pData=(UnsignedInt*)surf->Lock(&pitch);

		Int maxOpacity=(Int)(TheWaterTransparency->m_minWaterOpacity * 255.0f);
		Int alpha;

		if (pData)
		{
			//Fill texture with alpha gradient
			for (Int x=0; x<256; x++)
			{	
				alpha = x;
				if (alpha > maxOpacity)
					alpha = maxOpacity;
				*pData=(alpha<<24)|0x00ffffff;
				pData++;
			}
			surf->Unlock();
		}

		m_destAlphaTexture->Get_Filter().Set_U_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
		m_destAlphaTexture->Get_Filter().Set_V_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
		REF_PTR_RELEASE(surf);
		m_currentMinWaterOpacity = TheWaterTransparency->m_minWaterOpacity;
	}
}

//=============================================================================
// HeightMapRenderObjClass::initHeightData
//=============================================================================
/** Allocate a heightmap of x by y vertices and fill with initial height values.
Also allocates all rendering resources such as vertex buffers, index buffers, 
shaders, and materials.*/
//=============================================================================
Int HeightMapRenderObjClass::initHeightData(Int x, Int y, WorldHeightMap *pMap, RefRenderObjListIterator *pLightsIterator)
{	
	Int i,j;
//	Int	vertsPerRow=x*2-2;
//	Int	vertsPerColumn=y*2-2;

	REF_PTR_SET(m_map,pMap);	//update our heightmap pointer in case it changed since last call.

	if (m_shroud)
		m_shroud->init(m_map,TheGlobalData->m_partitionCellSize,TheGlobalData->m_partitionCellSize);
#ifdef DO_ROADS
	if (m_roadBuffer)
		m_roadBuffer->setMap(m_map);
#endif
	HeightSampleType *data = NULL;
	if (pMap) {
		data = pMap->getDataPtr();
	}

	m_numExtraBlendTiles = 0;
	m_numShoreLineTiles = 0;
	//Do some preprocessing on map to extract useful data
	if (pMap)
	{
		//Find min/max values for all terrain heights, useful for rendering optimization
		Int m_mapDX=pMap->getXExtent();
		Int m_mapDY=pMap->getYExtent();
		Int i, j, minHt, maxHt;

		minHt = pMap->getMaxHeightValue();
		maxHt = 0;

		for (j=0; j<m_mapDY; j++) {
			for (i=0; i<m_mapDX; i++) {
				Short cur = pMap->getHeight(i,j);
				if (cur<minHt) minHt = cur;
				if (maxHt<cur) maxHt = cur;
			}
		}
		m_minHeight = minHt * MAP_HEIGHT_SCALE;
		m_maxHeight = maxHt * MAP_HEIGHT_SCALE;

		if (!m_extraBlendTilePositions)
		{	//Need to allocate memory
			m_extraBlendTilePositions = NEW Int[DEFAULT_MAX_MAP_EXTRABLEND_TILES];
			m_extraBlendTilePositionsSize = DEFAULT_MAX_MAP_EXTRABLEND_TILES;
		}
		
		//Find list of all extra blend tiles used on map.  These are tiles with 3 materials/textures
		//over the same tile and require an extra render pass.
		for (j=0; j<(m_mapDY-1); j++)
			for (i=0; i<(m_mapDX-1); i++)
			{
				Real U[4],V[4];
				UnsignedByte alpha[4];
				Bool flipState,cliffState;
				if (pMap->getExtraAlphaUVData(i,j,U,V,alpha,&flipState, &cliffState))
				{	if (m_numExtraBlendTiles >= m_extraBlendTilePositionsSize)
					{	//no more room to store extra blend tiles so enlarge the buffer.
						Int *tempPositions=NEW Int[m_extraBlendTilePositionsSize+512];
						memcpy(tempPositions, m_extraBlendTilePositions, m_extraBlendTilePositionsSize*sizeof(Int));
						delete [] m_extraBlendTilePositions;
						//enlarge by more tiles to reduce memory trashing
						m_extraBlendTilePositions = tempPositions;
						m_extraBlendTilePositionsSize += 512;
					}
					//Pack x and y position into single integer since maps are limited in size
					m_extraBlendTilePositions[m_numExtraBlendTiles]=i | (j <<16);
					m_numExtraBlendTiles++;
				}
			}

		//Find all shoreline tiles so they can get extra alpha blend
		updateShorelineTiles(0,0,m_mapDX-1,m_mapDY-1,pMap);
		if (TheWaterTransparency->m_minWaterOpacity != m_currentMinWaterOpacity)
			initDestAlphaLUT();
	}

	Set_Force_Visible(TRUE);	//terrain is always visible.
	m_halfResMesh = TheGlobalData->m_useHalfHeightMap;
	m_originX = 0;
	m_originY = 0;
	m_needFullUpdate = true;

	m_scorchesInBuffer = 0; 
	m_curNumScorchVertices=0;
	m_curNumScorchIndices=0;
	// If the size changed, we need to allocate.
	Bool needToAllocate = (x != m_x || y != m_y);
	// If the textures aren't allocated (usually because of a hardware reset) need to allocate.
	if (m_stageOneTexture == NULL && m_treeBuffer) {
		needToAllocate = true;
	}
	if (data && needToAllocate && m_treeBuffer != NULL)
	{	//requested heightmap different from old one.
		//allocate a new one.
		freeMapResources();	//free old data and ib/vb
		REF_PTR_SET(m_map,pMap);	//update our heightmap pointer in case it changed since last call.
		m_stageTwoTexture=NEW CloudMapTerrainTextureClass;
		m_stageThreeTexture=NEW LightMapTerrainTextureClass(m_macroTextureName);
		m_destAlphaTexture=MSGNEW("TextureClass") TextureClass(256,1,WW3D_FORMAT_A8R8G8B8,MIP_LEVELS_1);
		initDestAlphaLUT();
#ifdef DO_SCORCH
		allocateScorchBuffers();
#endif

		//Create static index buffers.  These will index the vertex buffers holding the map.
		m_indexBuffer=NEW_REF(DX8IndexBufferClass,(VERTEX_BUFFER_TILE_LENGTH*VERTEX_BUFFER_TILE_LENGTH*2*3));

		// Fill up the IB
		DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexBuffer);
		UnsignedShort *ib=lockIdxBuffer.Get_Index_Array();
			
		for (j=0; j<(VERTEX_BUFFER_TILE_LENGTH*VERTEX_BUFFER_TILE_LENGTH*4); j+=VERTEX_BUFFER_TILE_LENGTH*4)
		{
			for (i=j; i<(j+VERTEX_BUFFER_TILE_LENGTH*4); i+=4)	//4 vertices per 2x2 block
			{
				ib[0]=i;
				ib[1]=i+2;
				ib[2]=i+3;

				ib[3]=i;
				ib[4]=i+1;
				ib[5]=i+2;

				ib+=6;	//skip the 6 indices we just filled
			}
		}

		//Get number of vertex buffers needed to hold current map
		//First round dimensions to next multiple of VERTEX_BUFFER_TILE_LENGTH since that's our
		//blocksize
		m_numVBTilesX=1;
		for (i=VERTEX_BUFFER_TILE_LENGTH+1; i<x;)
		{	i+=VERTEX_BUFFER_TILE_LENGTH;
			m_numVBTilesX++;
		}
		m_numVBTilesY=1;
		for (j=VERTEX_BUFFER_TILE_LENGTH+1; j<y;)
		{	j+=VERTEX_BUFFER_TILE_LENGTH;
			m_numVBTilesY++;
		}

		m_numBlockColumnsInLastVB=(x-1)%VERTEX_BUFFER_TILE_LENGTH;	//right border within last VB
		m_numBlockRowsInLastVB=(y-1)%VERTEX_BUFFER_TILE_LENGTH;	//bottom border within last VB

		m_numVertexBufferTiles=m_numVBTilesX*m_numVBTilesY;
		m_x=x;
		m_y=y;
		m_vertexBufferTiles = NEW DX8VertexBufferClass*[m_numVertexBufferTiles];
		m_vertexBufferBackup = NEW char *[m_numVertexBufferTiles];

		Int numVertex = VERTEX_BUFFER_TILE_LENGTH*2*VERTEX_BUFFER_TILE_LENGTH*2;

#ifdef PRE_TRANSFORM_VERTEX
		D3DDEVICE_CREATION_PARAMETERS parms;
		DX8Wrapper::_Get_D3D_Device8()->GetCreationParameters(&parms);
		Bool softwareVertexProcessing = 0!=(parms.BehaviorFlags&D3DCREATE_SOFTWARE_VERTEXPROCESSING);
		if (m_xformedVertexBuffer == NULL && softwareVertexProcessing) {
			m_xformedVertexBuffer = NEW IDirect3DVertexBuffer8*[m_numVertexBufferTiles];
		}
#endif

		for (i=0; i<m_numVertexBufferTiles; i++) {
#ifdef USE_NORMALS	 
			m_vertexBufferTiles[i]=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZNUV2,numVertex,DX8VertexBufferClass::USAGE_DEFAULT));
#else
			m_vertexBufferTiles[i]=NEW_REF(DX8VertexBufferClass,(DX8_VERTEX_FORMAT,numVertex,DX8VertexBufferClass::USAGE_DEFAULT));
#endif
			m_vertexBufferBackup[i] = NEW char[numVertex*sizeof(VERTEX_FORMAT)];
#ifdef PRE_TRANSFORM_VERTEX
			if (m_xformedVertexBuffer) {
				DX8Wrapper::_Get_D3D_Device8()->CreateVertexBuffer(
					D3DXGetFVFVertexSize(D3DFVF_XYZRHW |D3DFVF_DIFFUSE|D3DFVF_TEX2)*numVertex,
					D3DUSAGE_WRITEONLY|D3DUSAGE_DYNAMIC,
					D3DFVF_XYZRHW |D3DFVF_DIFFUSE|D3DFVF_TEX2,
					D3DPOOL_DEFAULT,
					&m_xformedVertexBuffer[i]);
			}
#endif
		} 

		//go with a preset material for now.
#ifdef USE_NORMALS
		m_vertexMaterialClass= NEW VertexMaterialClass();
		m_vertexMaterialClass->Set_Shininess(0.0);
		m_vertexMaterialClass->Set_Ambient(1,1,1);		  
		m_vertexMaterialClass->Set_Diffuse(1,1,1);
		m_vertexMaterialClass->Set_Specular(0,0,0);
		m_vertexMaterialClass->Set_Opacity(0);
		m_vertexMaterialClass->Set_Lighting(true);
#else
		m_vertexMaterialClass=VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
#endif

		m_shaderClass = detailOpaqueShader;	//		ShaderClass::_PresetOpaqueShader;
	}

	updateBlock(0,0,x-1,y-1,pMap,pLightsIterator);

	return 0;
}

#ifdef DO_SCORCH
//=============================================================================
// HeightMapRenderObjClass::freeScorchBuffers
//=============================================================================
/** Frees the vertex buffers for scorches.*/
//=============================================================================
void HeightMapRenderObjClass::freeScorchBuffers(void)
{
	REF_PTR_RELEASE(m_vertexScorch);
	REF_PTR_RELEASE(m_indexScorch);
	REF_PTR_RELEASE(m_scorchTexture);
}

//=============================================================================
// HeightMapRenderObjClass::allocateScorchBuffers
//=============================================================================
/** Allocates the vertex buffer and texture for scorches.*/
//=============================================================================
void HeightMapRenderObjClass::allocateScorchBuffers(void)
{
	m_vertexScorch=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZDUV1,MAX_SCORCH_VERTEX,DX8VertexBufferClass::USAGE_DEFAULT));
	m_indexScorch=NEW_REF(DX8IndexBufferClass,(MAX_SCORCH_INDEX));
	m_scorchTexture=NEW ScorchTextureClass;
	m_scorchesInBuffer = 0; // If we just allocated the buffers, we got no scorches in the buffer.
	m_curNumScorchVertices=0;
	m_curNumScorchIndices=0;
#ifdef RTS_DEBUG
	Vector3 loc(4*MAP_XY_FACTOR,4*MAP_XY_FACTOR,0);
	addScorch(loc, 1*MAP_XY_FACTOR, SCORCH_1);
	loc.Y += 10*MAP_XY_FACTOR;
	loc.X += 5*MAP_XY_FACTOR;
	addScorch(loc, 3*MAP_XY_FACTOR, SCORCH_1);
#endif

}

//=============================================================================
// HeightMapRenderObjClass::updateScorches
//=============================================================================
/** Builds the vertex buffer data for drawing the scorches.*/
//=============================================================================
void HeightMapRenderObjClass::updateScorches(void)
{
	if (m_scorchesInBuffer > 1) {
		return;
	}
	if (!m_indexScorch || !m_vertexScorch) {
		return;
	}
	m_curNumScorchVertices = 0;
	m_curNumScorchIndices = 0;
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexScorch);
	UnsignedShort *ib=lockIdxBuffer.Get_Index_Array();
	UnsignedShort *curIb = ib;

	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexScorch);
	VertexFormatXYZDUV1 *vb = (VertexFormatXYZDUV1*)lockVtxBuffer.Get_Vertex_Array();
	VertexFormatXYZDUV1 *curVb = vb;

	Int curScorch;
	Real shadeR, shadeG, shadeB;
	shadeR = TheGlobalData->m_terrainAmbient[0].red;
	shadeG = TheGlobalData->m_terrainAmbient[0].green;
	shadeB = TheGlobalData->m_terrainAmbient[0].blue;
	shadeR += TheGlobalData->m_terrainDiffuse[0].red/2;
	shadeG += TheGlobalData->m_terrainDiffuse[0].green/2;
	shadeB += TheGlobalData->m_terrainDiffuse[0].blue/2;
	shadeR*=255.0f;
	shadeG*=255.0f;
	shadeB*=255.0f;
	Int diffuse=REAL_TO_INT(shadeB) | (REAL_TO_INT(shadeG) << 8) | (REAL_TO_INT(shadeR) << 16) | ((int)255 << 24);
	m_scorchesInBuffer = 0;
	for (curScorch=m_numScorches-1; curScorch>=0; curScorch--) {
		m_scorchesInBuffer++;
		Real radius = m_scorches[curScorch].radius;
		Vector3 loc = m_scorches[curScorch].location;
		Int type = m_scorches[curScorch].scorchType;
		if (type<0) {
			type = 0;
		}
		if (type >= SCORCH_MARKS_IN_TEXTURE) {
			type = 0;
		}
		Real amtToFloat = 0;
		if (m_halfResMesh) {
			amtToFloat = MAP_HEIGHT_SCALE/8;
		}

		Int minX = REAL_TO_INT_FLOOR((loc.X-radius)/MAP_XY_FACTOR);
		Int minY = REAL_TO_INT_FLOOR((loc.Y-radius)/MAP_XY_FACTOR);
		if (minX<-m_map->getBorderSize()) minX=-m_map->getBorderSize();
		if (minY<-m_map->getBorderSize()) minY=-m_map->getBorderSize();
		Int maxX = REAL_TO_INT_CEIL((loc.X+radius)/MAP_XY_FACTOR);
		Int maxY = REAL_TO_INT_CEIL((loc.Y+radius)/MAP_XY_FACTOR);
		maxX++; maxY++;
		if (maxX > m_map->getXExtent()-m_map->getBorderSize()) {
			maxX = m_map->getXExtent()-m_map->getBorderSize();
		}
		if (maxY > m_map->getYExtent()-m_map->getBorderSize()) {
			maxY = m_map->getYExtent()-m_map->getBorderSize();
		}
		Int startVertex = m_curNumScorchVertices;
		Int i, j;
		for (j=minY; j<maxY; j++) {
			for (i=minX; i<maxX; i++) {
				if (m_curNumScorchVertices >= MAX_SCORCH_VERTEX) return;
				curVb->diffuse = diffuse;
				Real theZ; 
				theZ = amtToFloat+((float)getClipHeight(i+m_map->getBorderSize(),j+m_map->getBorderSize())*MAP_HEIGHT_SCALE);
				if (m_halfResMesh) {
					theZ = amtToFloat + this->getMaxCellHeight(i, j);
					Real amt2 = amtToFloat + getMaxCellHeight(i-1, j-1);
					if (amt2 > theZ) {
						theZ = amt2;
					}
				}
				// The scorchmarks are spaced out by 1.5 in the texture.
				Real uOffset = (type%SCORCH_PER_ROW) * 1.5f;
				Real vOffset = (type/SCORCH_PER_ROW) * 1.5f;
				Real X = i*MAP_XY_FACTOR; 
				Real Y = j*MAP_XY_FACTOR;
				curVb->u1 = (uOffset + 0.5f + (X - loc.X)/(2*radius)) / (SCORCH_PER_ROW+1);
				curVb->v1 = (vOffset + 0.5f + (Y - loc.Y)/(2*radius)) / (SCORCH_PER_ROW+1);
				curVb->x = X;
				curVb->y = Y;
				curVb->z = theZ;
				curVb++;
				m_curNumScorchVertices++;
			}
		}
		Int yOffset = maxX-minX;
		for (j=0; j<maxY-minY-1; j++) {
			for (i=0; i<maxX-minX-1; i++) {
				if (m_curNumScorchIndices+6 > MAX_SCORCH_INDEX) return;
				Int xNdx = i+minX+m_map->getBorderSize();
				Int yNdx = j+minY+m_map->getBorderSize();
				Bool flipForBlend = m_map->getFlipState(xNdx, yNdx);
#if 0
				UnsignedByte alpha[4];
				float UA[4], VA[4];
				m_map->getAlphaUVData(xNdx, yNdx, UA, VA, alpha, &flipForBlend, false);
#endif
				if (flipForBlend) {
					*curIb++ = startVertex + j*yOffset + i+1;
 					*curIb++ = startVertex + j*yOffset + i+yOffset;
					*curIb++ = startVertex + j*yOffset + i;
 					*curIb++ = startVertex + j*yOffset + i+1;
 					*curIb++ = startVertex + j*yOffset + i+1+yOffset;
					*curIb++ = startVertex + j*yOffset + i+yOffset;
				}	
				else 
				{
					*curIb++ = startVertex + j*yOffset + i;
					*curIb++ = startVertex + j*yOffset + i+1+yOffset;
					*curIb++ = startVertex + j*yOffset + i+yOffset;
					*curIb++ = startVertex + j*yOffset + i;
					*curIb++ = startVertex + j*yOffset + i+1;
					*curIb++ = startVertex + j*yOffset + i+1+yOffset;
				}
				m_curNumScorchIndices+=6;
			}
		}
	}

}

#endif

//=============================================================================
// HeightMapRenderObjClass::clearAllScorches
//=============================================================================
/** Removes all scorches. */
//=============================================================================
void HeightMapRenderObjClass::clearAllScorches(void)
{
#ifdef DO_SCORCH
	m_numScorches=0;
	m_scorchesInBuffer=0;	
#endif	
}

//=============================================================================
// HeightMapRenderObjClass::addScorch
//=============================================================================
/** Adds a scorch mark. */
//=============================================================================
void HeightMapRenderObjClass::addScorch(Vector3 location, Real radius, Scorches type)
{
#ifdef DO_SCORCH
	if (m_numScorches >= MAX_SCORCH_MARKS) {
		Int i;
		for (i=0; i<MAX_SCORCH_MARKS-1; i++) {
			m_scorches[i] = m_scorches[i+1];
		}
		m_numScorches--;
	}

	Int i;
	Real limit = radius/4;
	for (i=0; i<m_numScorches; i++) {		
		if ( abs(location.X-m_scorches[i].location.X) < limit &&  
				 abs(location.Y-m_scorches[i].location.Y) < limit && 
				 abs(radius - m_scorches[i].radius) < limit &&
				 m_scorches[i].scorchType == type) {
			return; // basically a duplicate.
		}
	}

	m_scorches[m_numScorches].location = location;
	m_scorches[m_numScorches].radius = radius;
	m_scorches[m_numScorches].scorchType = type;
	m_numScorches++;
	m_scorchesInBuffer = 0; // force buffer regenerations.
#endif	
}


//=============================================================================
// HeightMapRenderObjClass::getStaticDiffuse
//=============================================================================
/** Gets the static diffuse color value for a terrain vertex.*/
//=============================================================================
Int HeightMapRenderObjClass::getStaticDiffuse(Int x, Int y)
{	

	if (x<0) x = 0;
	if (y<0) y = 0;
	if (x >= m_map->getXExtent())
		x=m_map->getXExtent()-1;
	if (y >= m_map->getYExtent())
		y=m_map->getYExtent()-1;
	if (m_halfResMesh) {
		x&=0xffffffe;
		y&=0xffffffe;
	}

	if (m_map == NULL) {
		return(0);
	}

	Vector3 l2r,n2f,normalAtTexel;
	Int vn0,un0,vp1,up1;
	Int cellOffset = 1;
	if (m_halfResMesh) {
		cellOffset = 2;
	}

	vn0 = y-cellOffset;
	vp1 = y+cellOffset;
	if (vp1 >= m_map->getYExtent())
		vp1=m_map->getYExtent()-1;
	if (vn0<0) vn0 = 0;
	un0 = x-cellOffset;
	up1 = x+cellOffset;
	if (un0 < 0)
		un0=0;
	if (up1 >= m_map->getXExtent())
		up1=m_map->getXExtent()-1;

	Vector3 lightRay[MAX_GLOBAL_LIGHTS];
	const Coord3D *lightPos;

	for (Int lightIndex=0; lightIndex < TheGlobalData->m_numGlobalLights; lightIndex++)
	{
		lightPos=&TheGlobalData->m_terrainLightPos[lightIndex];
		lightRay[lightIndex].Set(-lightPos->x,-lightPos->y,	-lightPos->z);
	}

	//top-left sample
	l2r.Set(2*MAP_XY_FACTOR,0,MAP_HEIGHT_SCALE*(m_map->getHeight(up1, y) - m_map->getHeight(un0, y)));
	n2f.Set(0,2*MAP_XY_FACTOR,MAP_HEIGHT_SCALE*(m_map->getHeight(x, vp1) - m_map->getHeight(x, vn0)));
	
#ifdef ALLOW_TEMPORARIES
	normalAtTexel= Normalize(Vector3::Cross_Product(l2r,n2f));
#else
	Vector3::Normalized_Cross_Product(l2r,n2f, &normalAtTexel);
#endif

	VERTEX_FORMAT vertex;
	vertex.x=ADJUST_FROM_INDEX_TO_REAL(x);
	vertex.y=ADJUST_FROM_INDEX_TO_REAL(y);

	vertex.z=  ((float)m_map->getHeight(x,y))*MAP_HEIGHT_SCALE;
	vertex.u1=0;
	vertex.v1=0;
	vertex.u2=1;
	vertex.v2=1;

	RTS3DScene *pMyScene = (RTS3DScene *)Scene;
	if (pMyScene) {
		RefRenderObjListIterator *it = pMyScene->createLightsIterator();
		doTheLight(&vertex, lightRay, &normalAtTexel, it, 1.0f);
		if (it) {
		 pMyScene->destroyLightsIterator(it);
		 it = NULL;
		}
	} else {
		doTheLight(&vertex, lightRay, &normalAtTexel, NULL, 1.0f);
	}
#ifdef USE_NORMALS
	return(0xffffffff);
#else 
	return vertex.diffuse;
#endif

#define not_VERTS_MATCH
#ifdef VERTS_MATCH
	Int i,j;
	if (m_halfResMesh) {
		x&=0xffffffe;
		y&=0xffffffe;
	}
	Real X = x*MAP_XY_FACTOR;
	Real Y = y*MAP_XY_FACTOR;
	// This code digs the diffuse out of the vertex buffer.
	// It makes sense if the road vertexes match the terrain.
	// However, they don't match anymore. jba.

	Int yCoordMin = m_map->getDrawOrgY();
	Int yCoordMax = m_y+m_map->getDrawOrgY()-1;
	Int xCoordMin = m_map->getDrawOrgX();
	Int xCoordMax = m_x+m_map->getDrawOrgX()-1;	
	if (x<xCoordMin || y<yCoordMin || x> xCoordMax || y>yCoordMax) {
		return(0);
	}
	if (x==xCoordMax) x--;
	if (y==yCoordMax) y--;
	x -= xCoordMin;
	y -= yCoordMin;

	y += m_originY;
	if (y<0) y+= m_y-1;
	if (y> m_y-1) y-=m_y-1;
	if (y<0) y = 0;
	if (y>= m_y-1) y=m_y-1;
												  
	x += m_originX;
	if (x<0) x+= m_x-1;
	if (x> m_x-1) x-=m_x-1;
	if (x<0) x = 0;
	if (x>= m_x-1) x=m_x-1;
	i = 0;
	while (x>VERTEX_BUFFER_TILE_LENGTH) {
		i++;
		x -= VERTEX_BUFFER_TILE_LENGTH;
	}
	if (x==VERTEX_BUFFER_TILE_LENGTH) x--;
	j = 0;
	while (y>VERTEX_BUFFER_TILE_LENGTH) {
		j++;
		y -= VERTEX_BUFFER_TILE_LENGTH;
	}
	if (y==VERTEX_BUFFER_TILE_LENGTH) y--;

	char **pData = m_vertexBufferBackup+j*m_numVBTilesX+i;
	Int	vertsPerRow=(VERTEX_BUFFER_TILE_LENGTH)*4;	//vertices per row of VB

	if (m_halfResMesh) {
		x/=2;
		y/=2;
		vertsPerRow /= 2;
	}

	VERTEX_FORMAT *vbMirror = ((VERTEX_FORMAT*)(*pData))  + (y)*vertsPerRow+4*(x);
	if ( vbMirror[0].x==X && vbMirror[0].y==Y) {
		return(vbMirror[0].diffuse);
	}
	if ( vbMirror[3].x==X && vbMirror[3].y==Y) {
		return(vbMirror[3].diffuse);
	}
	if ( vbMirror[1].x==X && vbMirror[1].y==Y) {
		return(vbMirror[1].diffuse);
	}
	if ( vbMirror[2].x==X && vbMirror[2].y==Y) {
		return(vbMirror[2].diffuse);
	}
#ifdef RTS_DEBUG
	char buf[256];
	sprintf(buf, "(%f,%f) -> mirror (%f, %f)\n", X, Y, vbMirror->x, vbMirror->y);
	::OutputDebugString(buf);
#endif
	return(vbMirror->diffuse);
#endif
}

//=============================================================================
// HeightMapRenderObjClass::On_Frame_Update
//=============================================================================
/** Updates the diffuse color values in the vertices as affected by the dynamic lights.*/
//=============================================================================
void HeightMapRenderObjClass::On_Frame_Update(void)
{	

	Int i,j,k;
	DX8VertexBufferClass	**pVB;
	Int originX,originY;
	if (Scene==NULL) return;
	RTS3DScene *pMyScene = (RTS3DScene *)Scene;


	RefRenderObjListIterator pDynamicLightsIterator(pMyScene->getDynamicLights());
	if (m_map == NULL) {
		return;
	}

#ifdef DO_UNIT_TIMINGS  		  
#pragma MESSAGE("*** WARNING *** DOING DO_UNIT_TIMINGS!!!!")
	return;
#endif

#ifdef EXTENDED_STATS
	if (DX8Wrapper::stats.m_disableTerrain) {
		return;
	}
#endif

	Int numDynaLights=0;
	W3DDynamicLight *enabledLights[MAX_ENABLED_DYNAMIC_LIGHTS];

	Int yCoordMin = m_map->getDrawOrgY();
	Int yCoordMax = m_y+m_map->getDrawOrgY();
	Int xCoordMin = m_map->getDrawOrgX();
	Int xCoordMax = m_x+m_map->getDrawOrgX();

	for (pDynamicLightsIterator.First(); !pDynamicLightsIterator.Is_Done(); pDynamicLightsIterator.Next())
	{		
		W3DDynamicLight *pLight = (W3DDynamicLight*)pDynamicLightsIterator.Peek_Obj();
		pLight->m_processMe = false;
		if (pLight->m_enabled || pLight->m_priorEnable) {
			Real range = pLight->Get_Attenuation_Range();
			if (pLight->m_priorEnable) {
				pLight->m_prevMinX = pLight->m_minX;
				pLight->m_prevMinY = pLight->m_minY;
				pLight->m_prevMaxX = pLight->m_maxX;
				pLight->m_prevMaxY = pLight->m_maxY;
			} 
			Vector3	pos = pLight->Get_Position();
			pLight->m_minX = (pos.X-range)/MAP_XY_FACTOR;
			pLight->m_maxX = (pos.X+range)/MAP_XY_FACTOR+1.0f;
			pLight->m_minY = (pos.Y-range)/MAP_XY_FACTOR;
			pLight->m_maxY = (pos.Y+range)/MAP_XY_FACTOR+1.0f;
			if (!pLight->m_priorEnable) {
				pLight->m_prevMinX = pLight->m_minX;
				pLight->m_prevMinY = pLight->m_minY;
				pLight->m_prevMaxX = pLight->m_maxX;
				pLight->m_prevMaxY = pLight->m_maxY;
			} 

			if (pLight->m_minX < xCoordMax && 
					pLight->m_minY < yCoordMax &&
					pLight->m_maxX > xCoordMin && 
					pLight->m_maxY > yCoordMin) {
				pLight->m_processMe = TRUE;
			} else if (pLight->m_prevMinX < xCoordMax && 
					pLight->m_prevMinY < yCoordMax &&
					pLight->m_prevMaxX > xCoordMin && 
					pLight->m_prevMaxY > yCoordMin) {
				pLight->m_processMe = TRUE;
			} else {
				pLight->m_processMe = false;
			}
			if (pLight->m_processMe) {
				enabledLights[numDynaLights] = pLight;
				numDynaLights++;
				if (numDynaLights == MAX_ENABLED_DYNAMIC_LIGHTS) {
					break;
				}
			}
		}
		pLight->m_priorEnable = pLight->m_enabled;
	}
	if (numDynaLights > 0) {
		//step through each vertex buffer that needs updating
		for (j=0; j<m_numVBTilesY; j++)
		{
			originY=j*VERTEX_BUFFER_TILE_LENGTH;	//location of this VB on the large full-size heightmap
			Int yMin, yMax;
			yMin = originY;
			yMax = originY+VERTEX_BUFFER_TILE_LENGTH;
			Bool intersect = false;
			Int yCoordMin = getYWithOrigin(yMin)+m_map->getDrawOrgY()-m_map->getBorderSize();
			Int yCoordMax = getYWithOrigin(yMax-1)+m_map->getDrawOrgY()+1-m_map->getBorderSize();
			if (yCoordMax>yCoordMin) {
				// no wrap occurred.
				for (k=0; k<numDynaLights; k++) {
					if (enabledLights[k]->m_minY < yCoordMax && 
						enabledLights[k]->m_maxY > yCoordMin) {
						intersect = true;
						break;
					}
					if (enabledLights[k]->m_prevMinY < yCoordMax && 
						enabledLights[k]->m_prevMaxY > yCoordMin) {
						intersect = true;
						break;
					}
				}
			} else {
				// wrap occurred, so we are outside of this range.
				int tmp=yCoordMin;
				yCoordMin = yCoordMax;
				yCoordMax = tmp;
				for (k=0; k<numDynaLights; k++) {
					if (enabledLights[k]->m_minY <=  yCoordMin || 
						enabledLights[k]->m_maxY >= yCoordMax) {
						intersect = true;
						break;
					}
					if (enabledLights[k]->m_prevMinY <=  yCoordMin || 
						enabledLights[k]->m_prevMaxY >= yCoordMax) {
						intersect = true;
						break;
					}
				}
			}
			if (!intersect) {
				continue;
			}

			for (i=0; i<m_numVBTilesX; i++)
			{	
				originX=i*VERTEX_BUFFER_TILE_LENGTH;	//location of this VB on the large full-size heightmap
				Int xMin, xMax;
				xMin = originX;
				xMax = originX+VERTEX_BUFFER_TILE_LENGTH;

				Bool intersect = false;
				Int xCoordMin = getXWithOrigin(xMin)+m_map->getDrawOrgX()-m_map->getBorderSize();
				Int xCoordMax = getXWithOrigin(xMax-1)+m_map->getDrawOrgX()+1-m_map->getBorderSize();
				if (xCoordMax>xCoordMin) {
					// no wrap occurred.
					for (k=0; k<numDynaLights; k++) {
						if (enabledLights[k]->m_minX < xCoordMax && 
							enabledLights[k]->m_maxX > xCoordMin) {
							intersect = true;
							break;
						}
						if (enabledLights[k]->m_prevMinX < xCoordMax && 
							enabledLights[k]->m_prevMaxX > xCoordMin) {
							intersect = true;
							break;
						}
					}
				} else {
					// wrap occurred, so we are outside of this range.
					int tmp=xCoordMin;
					xCoordMin = xCoordMax;
					xCoordMax = tmp;
					for (k=0; k<numDynaLights; k++) {
						if (enabledLights[k]->m_minX <=  xCoordMin || 
							enabledLights[k]->m_maxX >= xCoordMax) {
							intersect = true;
							break;
						}
						if (enabledLights[k]->m_prevMinX <=  xCoordMin ||
							enabledLights[k]->m_prevMaxX >= xCoordMax) {
							intersect = true;
							break;
						}
					}
				}
				if (!intersect) {
					continue;
				}
				pVB=m_vertexBufferTiles+j*m_numVBTilesX+i;	//point to correct row/column of vertex buffers 
				char **pData = m_vertexBufferBackup+j*m_numVBTilesX+i;
				updateVBForLight(*pVB, *pData, xMin, yMin, xMax, yMax, originX,originY, enabledLights, numDynaLights);
			}
		}
	}
}

//=============================================================================
// HeightMapRenderObjClass::addTree
//=============================================================================
/** Adds a tree to the tree buffer.*/
//=============================================================================
void HeightMapRenderObjClass::addTree(Coord3D location, Real scale, Real angle, 
																			AsciiString name, Bool visibleInMirror)
{
	m_treeBuffer->addTree(location, scale, angle, name, visibleInMirror); 
};

//=============================================================================
// HeightMapRenderObjClass::addTerrainBib
//=============================================================================
/** Adds a terrainBib to the bib buffer.*/
//=============================================================================
void HeightMapRenderObjClass::addTerrainBib(Vector3 corners[4], 
																						ObjectID id, Bool highlight)
{
	if (m_bibBuffer)
		m_bibBuffer->addBib(corners, id, highlight); 
};

//=============================================================================
// HeightMapRenderObjClass::addTerrainBib
//=============================================================================
/** Adds a terrainBib to the bib buffer.*/
//=============================================================================
void HeightMapRenderObjClass::addTerrainBibDrawable(Vector3 corners[4], 
																						DrawableID id, Bool highlight)
{
	if (m_bibBuffer)
		m_bibBuffer->addBibDrawable(corners, id, highlight); 
};

//=============================================================================
// HeightMapRenderObjClass::removeAllTerrainBibs
//=============================================================================
/** Removes all terrainBib highlighting from the bib buffer.*/
//=============================================================================
void HeightMapRenderObjClass::removeTerrainBibHighlighting()
{
	if (m_bibBuffer)
		m_bibBuffer->removeHighlighting(  ); 
};

//=============================================================================
// HeightMapRenderObjClass::removeAllTerrainBibs
//=============================================================================
/** Removes all terrainBibs from the bib buffer.*/
//=============================================================================
void HeightMapRenderObjClass::removeAllTerrainBibs()
{
	if (m_bibBuffer)
		m_bibBuffer->clearAllBibs(  ); 
};

//=============================================================================
// HeightMapRenderObjClass::removeTerrainBib
//=============================================================================
/** Removes a terrainBib from the bib buffer.*/
//=============================================================================
void HeightMapRenderObjClass::removeTerrainBib(ObjectID id)
{
	if (m_bibBuffer)
		m_bibBuffer->removeBib( id ); 
};

//=============================================================================
// HeightMapRenderObjClass::removeTerrainBib
//=============================================================================
/** Removes a terrainBib from the bib buffer.*/
//=============================================================================
void HeightMapRenderObjClass::removeTerrainBibDrawable(DrawableID id)
{
	if (m_bibBuffer)
		m_bibBuffer->removeBibDrawable( id ); 
};

//=============================================================================
// HeightMapRenderObjClass::staticLightingChanged
//=============================================================================
/** Notification that all lighting needs to be recalculated. */
//=============================================================================
void HeightMapRenderObjClass::staticLightingChanged( void )
{
	// Cause the terrain to get updated with new lighting.
	m_needFullUpdate = true;

	// Cause the scorches to get updated with new lighting.
	m_scorchesInBuffer = 0; // If we just allocated the buffers, we got no scorches in the buffer.
	m_curNumScorchVertices=0;
	m_curNumScorchIndices=0;

}

//=============================================================================
// HeightMapRenderObjClass::setTimeOfDay
//=============================================================================
/** When the time of day changes, the lighting changes and we need to update. */
//=============================================================================
void HeightMapRenderObjClass::setTimeOfDay( TimeOfDay tod )
{		 
	staticLightingChanged();
}

//=============================================================================
// HeightMapRenderObjClass::Notify_Added
//=============================================================================
/** W3D render object method, we use it to add ourselves to tthe update 
list, so On_Frame_Update gets called. */
//=============================================================================
void HeightMapRenderObjClass::Notify_Added(SceneClass * scene)
{
	RenderObjClass::Notify_Added(scene);
	scene->Register(this,SceneClass::ON_FRAME_UPDATE);
}



#define CENTER_LIMIT 2
#define BIG_JUMP 16
#define WIDE_STEP 32

static Int visMinX, visMinY, visMaxX, visMaxY;
static Bool check(const FrustumClass & frustum, WorldHeightMap *pMap, Int x, Int y) 
{
	if (x<0 || y<0) return(false);
	if (x>= pMap->getXExtent() || y>= pMap->getYExtent()) return(false);
	if (x >= visMinX && y >= visMinY && x <=visMaxX && y <= visMaxY) {
		return(true);
	}
	Int height = pMap->getHeight(x, y);
	Vector3 loc((x-pMap->getBorderSize())*MAP_XY_FACTOR, (y-pMap->getBorderSize())*MAP_XY_FACTOR, height*MAP_HEIGHT_SCALE);
	if (CollisionMath::Overlap_Test(frustum,loc) == CollisionMath::INSIDE) {
		if (x<visMinX) visMinX=x;
		if (x>visMaxX) visMaxX=x;
		if (y<visMinY) visMinY=y;
		if (y>visMaxY) visMaxY=y;
		return(true);
	}
	return(false);
}

static void calcVis(const FrustumClass & frustum, WorldHeightMap *pMap, Int minX, Int minY, Int maxX, Int maxY, Int limit)
{
	if (maxX-minX<2) return;
	if (maxY-minY<2) return;
	if (minX >=visMinX && minY >= visMinY && maxX <=visMaxX && maxY <= visMaxY) {
		return;
	}
	Int midX = (minX+maxX)/2;
	Int midY = (minY+maxY)/2;
	Bool recurse1 = maxX-minX>=limit;
	Bool recurse2 = recurse1;
	Bool recurse3 = recurse1;
	Bool recurse4 = recurse1;	 
	/* boxes are:

			1     2


			3			4 */

	if (check(frustum, pMap, midX, maxY)) {
		recurse1=true;
		recurse2=true;
	}
	if (check(frustum, pMap, midX, minY)) {
		recurse3=true;
		recurse4=true;
	}
	if (check(frustum, pMap, midX, midY)) {
		recurse1=true;
		recurse2=true;
		recurse3=true;
		recurse4=true;
	}
	if (check(frustum, pMap, minX, midY)) {
		recurse1=true;
		recurse3=true;
	}
	if (check(frustum, pMap, maxX, midY)) {
		recurse2=true;
		recurse4=true;
	}
	if (recurse1) {
		calcVis(frustum, pMap, minX, midY, midX, maxY, limit);
	}
	if (recurse2) {
		calcVis(frustum, pMap, midX, midY, maxX, maxY, limit);
	}
	if (recurse3) {
		calcVis(frustum, pMap, minX, minY, midX, midY, limit);
	}
	if (recurse4) {
		calcVis(frustum, pMap, midX, minY, maxX, midY, limit);
	}
}


//=============================================================================
// HeightMapRenderObjClass::updateCenter
//=============================================================================
/** Updates the positioning of the drawn portion of the height map in the 
heightmap.  As the view slides around, this determines what is the actually
rendered portion of the terrain.  Only a 96x96 section is rendered at any time, 
even though maps can be up to 1024x1024.  This function determines which subset
is rendered. */
//=============================================================================
void HeightMapRenderObjClass::updateCenter(CameraClass *camera , RefRenderObjListIterator *pLightsIterator)
{
	if (m_map==NULL) {
		return;
	}
	if (m_updating) {
		return;
	}

	if (m_vertexBufferTiles ==NULL)
		return;		//did not initialize resources yet.

	m_treeBuffer->doFullUpdate();	// Tell the trees to update for view change.

#ifdef TEST_CUSTOM_EDGING
	m_customEdging->doFullUpdate();
#endif

	m_updating = true;
	if (m_needFullUpdate) {
		m_needFullUpdate = false;
		updateBlock(0, 0, m_x-1, m_y-1, m_map, pLightsIterator);
#ifdef DO_ROADS
		if (m_roadBuffer) {
			m_roadBuffer->updateLighting();
		}
#endif
		m_bridgeBuffer->doFullUpdate();
		m_bridgeBuffer->updateCenter(camera, pLightsIterator);
		m_updating = false;
		return;
	}
	m_bridgeBuffer->updateCenter(camera, pLightsIterator);

	if (m_x >= m_map->getXExtent() && m_y >= m_map->getYExtent()) {
		m_updating = false;
		return; // no need to center. 
	}

	Int cellOffset = 1;
	if (m_halfResMesh) {
		cellOffset = 2;
	}
	// determine the ray corresponding to the camera and distance to projection plane
	Matrix3D camera_matrix = camera->Get_Transform();
	
	Vector3 camera_location  = camera->Get_Position();

	Vector3 rayLocation;
	Vector3 rayDirection;
	Vector3 rayDirectionPt;
	// the projected ray has the same origin as the camera
	rayLocation = camera_location; 
	// determine the location of the screen coordinate in camera-model space
	const ViewportClass &viewport = camera->Get_Viewport();
	Int i, j, minHt;

	Real intersectionZ;
	minHt = m_map->getMaxHeightValue();
	for (i=0; i<m_x; i++) {
		for (j=0; j<m_y; j++) {
			Short cur = m_map->getDisplayHeight(i,j);
			if (cur<minHt) minHt = cur;
		}
	}
	intersectionZ = (float)minHt;
//	float aspect = camera->Get_Aspect_Ratio();

	Vector2 min,max;
	camera->Get_View_Plane(min,max);
	float xscale = (max.X - min.X);
	float yscale = (max.Y - min.Y);

	float zmod = -1.0; // Scene->vpd; // Note: view plane distance is now always 1.0 from the camera
	float minX = 200000;
	float maxX = -minX;
	float minY = 200000;
	float maxY = -minY;
	for (i=0; i<2; i++) {
		for (j=0; j<2; j++) {
			float xmod = (-i + 0.5 + viewport.Min.X) * zmod * xscale;// / aspect;
			float ymod = (j - 0.5 - viewport.Min.Y) * zmod * yscale;// * aspect;

			// transform the screen coordinates by the camera's matrix into world coordinates.
			float x = zmod * camera_matrix[0][2] + xmod * camera_matrix[0][0] + ymod * camera_matrix[0][1];
			float y = zmod * camera_matrix[1][2] + xmod * camera_matrix[1][0] + ymod * camera_matrix[1][1];
			float z = zmod * camera_matrix[2][2] + xmod * camera_matrix[2][0] + ymod * camera_matrix[2][1];

			rayDirection.Set(x,y,z);
			rayDirection.Normalize();
			rayDirectionPt = rayLocation+rayDirection;

			x = Vector3::Find_X_At_Z(intersectionZ, rayLocation, rayDirectionPt);
			y = Vector3::Find_Y_At_Z(intersectionZ, rayLocation, rayDirectionPt);
			if (x<minX) minX = x;
			if (x>maxX) maxX = x;
			if (y<minY) minY = y;
			if (y>maxY) maxY = y;
		}
	}

	// convert back to cell indexes.
	minX /= MAP_XY_FACTOR;
	maxX /= MAP_XY_FACTOR;
	minY /= MAP_XY_FACTOR;
	maxY /= MAP_XY_FACTOR;

	minX += m_map->getBorderSize();
	maxX += m_map->getBorderSize();
	minY += m_map->getBorderSize();
	maxY += m_map->getBorderSize();

	visMinX = m_map->getXExtent();
	visMinY = m_map->getYExtent();
	visMaxX = 0;
	visMaxY = 0;

	///< @todo find out why values go out of range
	if (minX<0) minX=0;
	if (minY<0) minY=0;
	if (maxX > visMinX) maxX = visMinX;
	if (maxY > visMinY) maxY = visMinY;

	const FrustumClass & frustum = camera->Get_Frustum();
	Int limit = (maxX-minX)/2;
	if (limit > WIDE_STEP/2) {
		limit=WIDE_STEP/2;
	}
	calcVis(frustum, m_map, minX-WIDE_STEP/2, minY-WIDE_STEP/2, maxX+WIDE_STEP/2, maxY+WIDE_STEP/2, limit);

	if (m_map) {
		Int newOrgX;
		if (visMaxX-visMinX > m_x) {
			newOrgX = (maxX+minX)/2-m_x/2.0;	
		} else {
			newOrgX = (visMaxX+visMinX)/2-m_x/2.0;
		}

		Int newOrgY;
		if (visMaxY - visMinY > m_y) {
			newOrgY = visMinY+1;	
		}	else {
			newOrgY = (visMaxY+visMinY)/2-m_y/2.0;
		}																															  
		if (TheTacticalView->getFieldOfView() != 0) {
			newOrgX = (visMaxX+visMinX)/2-m_x/2.0;
			newOrgY = (visMaxY+visMinY)/2-m_y/2.0;
		}
		if (m_halfResMesh) {
			newOrgX &= 0xFFFFFFFE;
			newOrgY &= 0xFFFFFFFE;
		}
		Int deltaX = newOrgX - m_map->getDrawOrgX();
		Int deltaY = newOrgY - m_map->getDrawOrgY();
		if (IABS(deltaX) > m_x/2 || IABS(deltaY)>m_x/2) {
			m_map->setDrawOrg(newOrgX, newOrgY);
			m_originY = 0;
			m_originX = 0;
			updateBlock(0, 0, m_x-1, m_y-1, m_map, pLightsIterator); 
			m_updating = false;
			return;
		}

		if (abs(deltaX)>CENTER_LIMIT || abs(deltaY)>CENTER_LIMIT) {
			if (abs(deltaY) >= CENTER_LIMIT) {
				if (m_map->setDrawOrg(m_map->getDrawOrgX(), newOrgY)) {
					Int minY = 0;
					Int maxY = 0;
					deltaY -= newOrgY - m_map->getDrawOrgY(); 
					m_originY += deltaY;
					if (m_originY >= m_y-1) m_originY -= m_y-1;
					if (deltaY<0) {
						minY = m_originY;
						maxY = m_originY-deltaY;
					} else {
						minY = m_originY - deltaY;
						maxY = m_originY;
					}
					minY-=cellOffset;
					if (m_originY < 0) m_originY += m_y-1;
					if (minY<0) {
						minY += m_y-1;
						if (minY<0) minY = 0;
						updateBlock(0, minY, m_x-1, m_y-1, m_map, pLightsIterator);
						updateBlock(0, 0, m_x-1, maxY, m_map, pLightsIterator);
					} else {
						updateBlock(0, minY, m_x-1, maxY, m_map, pLightsIterator);
					}
				}
				// It is much more efficient to update a cople of columns one frame, and then
				// a couple of rows.  So if we aren't "jumping" to a new view, and have done X
				// recently, return.
				if (abs(deltaX) < BIG_JUMP && !m_doXNextTime) {
					m_updating = false;
					m_doXNextTime = true;
					return;	// Only do the y this frame.  Do x next frame.  jba.
				}
			}
			if (abs(deltaX) > CENTER_LIMIT) {
				m_doXNextTime = false;
				newOrgX = m_map->getDrawOrgX() + deltaX;
				if (m_map->setDrawOrg(newOrgX, m_map->getDrawOrgY())) {
					Int minX = 0;
					Int maxX = 0;
					deltaX -= newOrgX - m_map->getDrawOrgX(); 
					m_originX += deltaX;
					if (m_originX >= m_x-1) m_originX -= m_x-1;
					if (deltaX<0) {
						minX = m_originX;
						maxX = m_originX-deltaX;
					} else {
						minX = m_originX - deltaX;
						maxX = m_originX;
					}
					minX-=cellOffset;
					maxX+=cellOffset;
					if (m_originX < 0) m_originX += m_x-1;
					if (minX<0) {
						minX += m_x-1;
						if (minX<0) minX = 0;
						updateBlock(minX,0,m_x-1, m_y-1, m_map, pLightsIterator);
						updateBlock(0,0,maxX, m_y-1, m_map, pLightsIterator);
					} else {
						updateBlock(minX,0,maxX, m_y-1, m_map, pLightsIterator);
					}
				}
			} 
		}
	}
	m_updating = false;
}

//=============================================================================
// HeightMapRenderObjClass::Render
//=============================================================================
/** Renders (draws) the terrain. */
//=============================================================================
//DECLARE_PERF_TIMER(Terrain_Render)

void HeightMapRenderObjClass::Render(RenderInfoClass & rinfo)
{
	//USE_PERF_TIMER(Terrain_Render)
	
	Int i,j,devicePasses;
	W3DShaderManager::ShaderTypes st;
	Bool doCloud = TheGlobalData->m_useCloudMap;

	Matrix3D tm(Transform);
#if 0 // There is some weirdness sometimes with the dx8 static buffers.
			// This usually fixes terrain flashing.  jba.
	static Int delay = 1;
	delay --;
	if (delay<1) {
		delay = 1;
		static Int ndx = -1;
		ndx++;
		if (ndx>=m_numVertexBufferTiles) {
			ndx = 0;
		}
		DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexBufferTiles[ndx]);
		VERTEX_FORMAT *vb = (VERTEX_FORMAT*)lockVtxBuffer.Get_Vertex_Array();	
		vb = 0;
	}
#endif
	// If there are trees, tell them to draw at the transparent time to draw.
	if (m_treeBuffer) {
		m_treeBuffer->setIsTerrain();
	}


#ifdef DO_UNIT_TIMINGS
#pragma MESSAGE("*** WARNING *** DOING DO_UNIT_TIMINGS!!!!")
	return;
#endif

#ifdef EXTENDED_STATS
	if (DX8Wrapper::stats.m_disableTerrain) {
		return;
	}
#endif

	DX8Wrapper::Set_Light_Environment(rinfo.light_environment);

	// Force shaders to update.
	m_stageTwoTexture->restore();
	DX8Wrapper::Set_Texture(0,NULL);
	DX8Wrapper::Set_Texture(1,NULL);
	ShaderClass::Invalidate();

	//	tm.Scale(ObjSpaceExtent);
	DX8Wrapper::Set_Transform(D3DTS_WORLD,tm);

	//Apply the shader and material

	DX8Wrapper::Set_Index_Buffer(m_indexBuffer,0);

	Bool doMultiPassWireFrame=FALSE;

	if (((RTS3DScene *)rinfo.Camera.Get_User_Data())->getCustomPassMode() == SCENE_PASS_ALPHA_MASK ||
		((SceneClass *)rinfo.Camera.Get_User_Data())->Get_Extra_Pass_Polygon_Mode() == SceneClass::EXTRA_PASS_CLEAR_LINE)
	{
			if (WW3D::Is_Texturing_Enabled())
			{	//first pass where we just fill the z-buffer

				devicePasses=1;	//one pass solid, next in wireframe.
				doMultiPassWireFrame=TRUE;

				if (rinfo.Additional_Pass_Count())
				{
					rinfo.Peek_Additional_Pass(0)->Install_Materials();
					renderTerrainPass(&rinfo.Camera);
					rinfo.Peek_Additional_Pass(0)->UnInstall_Materials();
					return;
				}
			}
			else
			{	//wireframe pass
				//Set to vertex diffuse lighting
				DX8Wrapper::Set_Material(m_vertexMaterialClass);
				//Set shader to non-textured solid color from vertex
				DX8Wrapper::Set_Shader(ShaderClass::_PresetOpaqueSolidShader);
				devicePasses=1;	//one pass solid, next in wireframe.
				DX8Wrapper::Apply_Render_State_Changes();
				DX8Wrapper::Set_DX8_Texture_Stage_State( 0, D3DTSS_COLORARG2, D3DTA_TFACTOR );
				DX8Wrapper::Set_DX8_Render_State(D3DRS_TEXTUREFACTOR,0xff808080);
				doMultiPassWireFrame=TRUE;
				renderTerrainPass(&rinfo.Camera);
				DX8Wrapper::Set_DX8_Render_State(D3DRS_TEXTUREFACTOR,0xff008000);
				return;
			}
	}
	else
	{
		DX8Wrapper::Set_Material(m_vertexMaterialClass);
		DX8Wrapper::Set_Shader(m_shaderClass);

		if (TheGlobalData->m_timeOfDay == TIME_OF_DAY_NIGHT) {
			doCloud = false;
		}

 		st=W3DShaderManager::ST_TERRAIN_BASE; //set default shader
 		
 		//set correct shader based on current settings
 		if (!ShaderClass::Is_Backface_Culling_Inverted())
 		{	//not reflection pass
 			if (TheGlobalData->m_useLightMap && doCloud)
 			{	st=W3DShaderManager::ST_TERRAIN_BASE_NOISE12;
 			}
 			else
 			if (TheGlobalData->m_useLightMap)
 			{	//lightmap only
 				st=W3DShaderManager::ST_TERRAIN_BASE_NOISE2;
 			}
 			else
 			if (doCloud)
 			{	//cloudmap only
 				st=W3DShaderManager::ST_TERRAIN_BASE_NOISE1;
 			}
 		}
 		else
 		{	//reflection pass, just do base texture
 			st=W3DShaderManager::ST_TERRAIN_BASE;
 		}
 
 		//Find number of passes required to render current shader
 		devicePasses=W3DShaderManager::getShaderPasses(st);
 
 		if (m_disableTextures)
 			devicePasses=1;	//force to 1 lighting-only pass
 
 		//Specify all textures that this shader may need.
 		W3DShaderManager::setTexture(0,m_stageZeroTexture);
 		W3DShaderManager::setTexture(1,m_stageZeroTexture);
 		W3DShaderManager::setTexture(2,m_stageTwoTexture);	//cloud
 		W3DShaderManager::setTexture(3,m_stageThreeTexture);//noise
		//Disable writes to destination alpha channel (if there is one)
		if (DX8Wrapper::getBackBufferFormat() == WW3D_FORMAT_A8R8G8B8)
			DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE,D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_RED);
	}

	Int pass;
 	for (pass=0; pass<devicePasses; pass++) {
#ifdef TIMING_TESTS
#endif
		if (!doMultiPassWireFrame)	//multi-pass wireframe doesn't use regular shaders.
		{
 			if (m_disableTextures ) {
 				DX8Wrapper::Set_Shader(ShaderClass::_PresetOpaque2DShader);
 				DX8Wrapper::Set_Texture(0,NULL);
   			} else {
 				W3DShaderManager::setShader(st, pass);
			}
		}

		for (j=0; j<m_numVBTilesY; j++)
			for (i=0; i<m_numVBTilesX; i++)
			{
				static int count = 0;
				count++;
				Int numPolys = VERTEX_BUFFER_TILE_LENGTH*VERTEX_BUFFER_TILE_LENGTH*2;
				Int numVertex = (VERTEX_BUFFER_TILE_LENGTH*2)*(VERTEX_BUFFER_TILE_LENGTH*2);
				if (m_halfResMesh) {
					numPolys /= 4;
					numVertex /= 4;
				}
				DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferTiles[j*m_numVBTilesX+i]);
#ifdef PRE_TRANSFORM_VERTEX
				if (m_xformedVertexBuffer && pass==0) {
					// Note - m_xformedVertexBuffer should only be used for non T&L hardware.  jba.
					DX8Wrapper::Apply_Render_State_Changes();
					int code = DX8Wrapper::_Get_D3D_Device8()->ProcessVertices(0, 0, numVertex, m_xformedVertexBuffer[j*m_numVBTilesX+i], 0); 
					::OutputDebugString("did process vertex\n");
				}
				if (m_xformedVertexBuffer) {
					// Note - m_xformedVertexBuffer should only be used for non T&L hardware.  jba.
					DX8Wrapper::Apply_Render_State_Changes();
					DX8Wrapper::_Get_D3D_Device8()->SetStreamSource(
						0,
						m_xformedVertexBuffer[j*m_numVBTilesX+i],
						D3DXGetFVFVertexSize(D3DFVF_XYZRHW |D3DFVF_DIFFUSE|D3DFVF_TEX2));
					DX8Wrapper::_Get_D3D_Device8()->SetVertexShader(D3DFVF_XYZRHW |D3DFVF_DIFFUSE|D3DFVF_TEX2);
				}
#endif				
				if (Is_Hidden() == 0) {
					DX8Wrapper::Draw_Triangles(	0,numPolys, 0,	numVertex);
				}

			}
	}		

	if (!doMultiPassWireFrame)
	{
		if (pass)	//shader was applied at least once?
 			W3DShaderManager::resetShader(st);

		//Draw feathered shorelines
		renderShoreLines(&rinfo.Camera);

		//Do additional pass over any tiles that have 3 textures blended together.
		if (TheGlobalData->m_use3WayTerrainBlends)
			renderExtraBlendTiles();

	#ifdef TEST_CUSTOM_EDGING
		// Draw edging just before last pass.
		DX8Wrapper::Set_Texture(0,NULL);
		DX8Wrapper::Set_Texture(1,NULL);
		m_stageTwoTexture->restore();
		Int yCoordMin = m_map->getDrawOrgY();
		Int yCoordMax = m_y+m_map->getDrawOrgY()-1;
		Int xCoordMin = m_map->getDrawOrgX();
		Int xCoordMax = m_x+m_map->getDrawOrgX()-1;	
		m_customEdging->drawEdging(m_map, xCoordMin, xCoordMax, yCoordMin, yCoordMax, 
			m_stageZeroTexture, doCloud?m_stageTwoTexture:NULL, TheGlobalData->m_useLightMap?m_stageThreeTexture:NULL);
	#endif
	#ifdef DO_ROADS
		DX8Wrapper::Set_Texture(0,NULL);
		DX8Wrapper::Set_Texture(1,NULL);
		m_stageTwoTexture->restore();

		ShaderClass::Invalidate();
		if (!ShaderClass::Is_Backface_Culling_Inverted()) {
			DX8Wrapper::Set_Material(m_vertexMaterialClass);
			if (Scene) {
				RTS3DScene *pMyScene = (RTS3DScene *)Scene;
				RefRenderObjListIterator pDynamicLightsIterator(pMyScene->getDynamicLights());
				m_roadBuffer->drawRoads(&rinfo.Camera, doCloud?m_stageTwoTexture:NULL, TheGlobalData->m_useLightMap?m_stageThreeTexture:NULL,
					m_disableTextures,xCoordMin-m_map->getBorderSize(), xCoordMax-m_map->getBorderSize(), yCoordMin-m_map->getBorderSize(), yCoordMax-m_map->getBorderSize(), &pDynamicLightsIterator);
			}
		}
	#endif

	#ifdef DO_SCORCH
		DX8Wrapper::Set_Texture(0,NULL);
		DX8Wrapper::Set_Texture(1,NULL);
		m_stageTwoTexture->restore();

		ShaderClass::Invalidate();
		if (!ShaderClass::Is_Backface_Culling_Inverted()) {
			drawScorches();
		}
	#endif
		DX8Wrapper::Set_Texture(0,NULL);
		DX8Wrapper::Set_Texture(1,NULL);
		m_stageTwoTexture->restore();
		ShaderClass::Invalidate();
		DX8Wrapper::Apply_Render_State_Changes();

		m_bridgeBuffer->drawBridges(&rinfo.Camera, m_disableTextures, m_stageTwoTexture);

		if (TheTerrainTracksRenderObjClassSystem)
			TheTerrainTracksRenderObjClassSystem->flush();

		if (m_shroud && rinfo.Additional_Pass_Count())
		{
			rinfo.Peek_Additional_Pass(0)->Install_Materials();
			renderTerrainPass(&rinfo.Camera);
			rinfo.Peek_Additional_Pass(0)->UnInstall_Materials();
		}

		ShaderClass::Invalidate();
		DX8Wrapper::Apply_Render_State_Changes();
	}
	else
			m_bridgeBuffer->drawBridges(&rinfo.Camera, m_disableTextures, m_stageTwoTexture);

	m_waypointBuffer->drawWaypoints(rinfo);

	m_bibBuffer->renderBibs();

	// We do some custom blending, so tell the shader class to reset everything.
	DX8Wrapper::Set_Texture(0,NULL);
	DX8Wrapper::Set_Texture(1,NULL);
	m_stageTwoTexture->restore();
	ShaderClass::Invalidate();
	DX8Wrapper::Set_Material(NULL);

}

/**Render parts of terrain that are along the coast line and have vertices directly under the
water plane.  Applying a custom render to these polygons allows for a smoother land->water
transition*/
void HeightMapRenderObjClass::renderShoreLines(CameraClass *pCamera)
{
	if (!TheGlobalData->m_showSoftWaterEdge || TheWaterTransparency->m_transparentWaterDepth==0 || m_numShoreLineTiles == 0)
		return;

	//Check if video card is capable of using this effect
	if (DX8Wrapper::getBackBufferFormat() != WW3D_FORMAT_A8R8G8B8)
		return;	//can't apply effect on cards without destination alpha

	Int vertexCount = 0;
	Int indexCount = 0;
	Int xExtent = m_map->getXExtent();
	Int border = m_map->getBorderSize();
	Int drawEdgeY=m_map->getDrawOrgY()+m_map->getDrawHeight()-1;
	Int drawEdgeX=m_map->getDrawOrgX()+m_map->getDrawWidth()-1;
	if (drawEdgeX > (m_map->getXExtent()-1))
		drawEdgeX = m_map->getXExtent()-1;
	if (drawEdgeY > (m_map->getYExtent()-1))
		drawEdgeY = m_map->getYExtent()-1;
	Int drawStartX=m_map->getDrawOrgX();
	Int drawStartY=m_map->getDrawOrgY();
	const UnsignedByte* data = m_map->getDataPtr();
	Int j=0;

	ShaderClass unlitShader=ShaderClass::_PresetOpaque2DShader;
	unlitShader.Set_Depth_Compare(ShaderClass::PASS_LEQUAL);
	DX8Wrapper::Set_Shader(unlitShader);
	VertexMaterialClass *vmat=VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
	DX8Wrapper::Set_Material(vmat);
	REF_PTR_RELEASE(vmat);
	DX8Wrapper::Set_Texture(0,m_destAlphaTexture);
	DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix3D(1));
	//Enabled writes to destination alpha only
	DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE,D3DCOLORWRITEENABLE_ALPHA);
	DX8Wrapper::Set_DX8_Texture_Stage_State(0,  D3DTSS_TEXCOORDINDEX, 0);

	while (j != m_numShoreLineTiles)
	{
		{	//Need to put this in another code block so vb/ib gets automatically locked/unlocked by destructors
			DynamicVBAccessClass vb_access(BUFFER_TYPE_DYNAMIC_DX8,dynamic_fvf_type,DEFAULT_MAX_BATCH_SHORELINE_TILES*4);
			DynamicIBAccessClass ib_access(BUFFER_TYPE_DYNAMIC_DX8,DEFAULT_MAX_BATCH_SHORELINE_TILES*6);

			DynamicVBAccessClass::WriteLockClass lock(&vb_access);
			VertexFormatXYZNDUV2 *vb= lock.Get_Formatted_Vertex_Array();
			DynamicIBAccessClass::WriteLockClass lockib(&ib_access);
			UnsignedShort *ib=lockib.Get_Index_Array();
			if (!ib || !vb)
			{	DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE,D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_RED);
				return;
			}

			//Loop over visible terrain and extract all the tiles that need shoreline blend
			for (; j<m_numShoreLineTiles; j++)
			{
				if (vertexCount >= (DEFAULT_MAX_BATCH_SHORELINE_TILES*4))
					break;	//no room in vertex buffer

				shoreLineTileInfo *shoreInfo=&m_shoreLineTilePositions[j];

				Int x = shoreInfo->m_xy & 0xffff;
				Int y = shoreInfo->m_xy >> 16;

				if (x >= drawStartX && x < drawEdgeX &&	y >= drawStartY && y < drawEdgeY)
				{	//this tile is inside visible region

					Int idx = x+y*xExtent;

					Real p0=data[idx]*MAP_HEIGHT_SCALE;
					Real p1=data[idx+1]*MAP_HEIGHT_SCALE;
					Real p2=data[idx + 1 + xExtent]*MAP_HEIGHT_SCALE;
					Real p3=data[idx + xExtent]*MAP_HEIGHT_SCALE;

					vb->x=(x-border)*MAP_XY_FACTOR;	 
					vb->y=(y-border)*MAP_XY_FACTOR;
					vb->z=p0;
					vb->u1=shoreInfo->t0;
					vb->v1=0;
					vb++;

					vb->x=(x+1-border)*MAP_XY_FACTOR;	 
					vb->y=(y-border)*MAP_XY_FACTOR;
					vb->z=p1;
					vb->u1=shoreInfo->t1;
					vb->v1=0;
					vb++;

					vb->x=(x+1-border)*MAP_XY_FACTOR;	 
					vb->y=(y+1-border)*MAP_XY_FACTOR;
					vb->z=p2;
					vb->u1=shoreInfo->t2;
					vb->v1=0;
					vb++;

					vb->x=(x-border)*MAP_XY_FACTOR;	 
					vb->y=(y+1-border)*MAP_XY_FACTOR;
					vb->z=p3;
					vb->u1=shoreInfo->t3;
					vb->v1=0;
					vb++;
					
					if (m_map->getFlipState(x,y))
					{
						ib[0]=1+vertexCount;
						ib[1]=3+vertexCount;
						ib[2]=0+vertexCount;
						ib[3]=1+vertexCount;
						ib[4]=2+vertexCount;
						ib[5]=3+vertexCount;
					}
					else
					{
						ib[0]=0+vertexCount;
						ib[1]=2+vertexCount;
						ib[2]=3+vertexCount;
						ib[3]=0+vertexCount;
						ib[4]=1+vertexCount;
						ib[5]=2+vertexCount;
					}
					ib += 6;
					vertexCount +=4;
					indexCount +=6;
				}
			}

			DX8Wrapper::Set_Index_Buffer(ib_access,0);
			DX8Wrapper::Set_Vertex_Buffer(vb_access);
		}//lock and fill ib/vb

		if (indexCount > 0 && vertexCount > 0)
			DX8Wrapper::Draw_Triangles(	0,indexCount/3, 0,	vertexCount);	//draw a quad, 2 triangles, 4 verts
		vertexCount=0;
		indexCount=0;
	}//for all shore tiles

	//Disable writes to destination alpha
	DX8Wrapper::Set_DX8_Render_State(D3DRS_COLORWRITEENABLE,D3DCOLORWRITEENABLE_BLUE|D3DCOLORWRITEENABLE_GREEN|D3DCOLORWRITEENABLE_RED);
	ShaderClass::Invalidate();
}

///Performs additional terrain rendering pass, blending in the black shroud texture.
void HeightMapRenderObjClass::renderTerrainPass(CameraClass *pCamera)
{
	DX8Wrapper::Set_Transform(D3DTS_WORLD,Matrix3D(1));

	//Apply the shader and material
	
	DX8Wrapper::Set_Index_Buffer(m_indexBuffer,0);

	for (Int j=0; j<m_numVBTilesY; j++)
		for (Int i=0; i<m_numVBTilesX; i++)
		{
			static int count = 0;
			count++;
			Int numPolys = VERTEX_BUFFER_TILE_LENGTH*VERTEX_BUFFER_TILE_LENGTH*2;
			Int numVertex = (VERTEX_BUFFER_TILE_LENGTH*2)*(VERTEX_BUFFER_TILE_LENGTH*2);
			if (m_halfResMesh) {
				numPolys /= 4;
				numVertex /= 4;
			}
			DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferTiles[j*m_numVBTilesX+i]);
#ifdef PRE_TRANSFORM_VERTEX
			if (m_xformedVertexBuffer && pass==0) {
				// Note - m_xformedVertexBuffer should only be used for non T&L hardware.  jba.
				DX8Wrapper::Apply_Render_State_Changes();
				int code = DX8Wrapper::_Get_D3D_Device8()->ProcessVertices(0, 0, numVertex, m_xformedVertexBuffer[j*m_numVBTilesX+i], 0); 
				::OutputDebugString("did process vertex\n");
			}
			if (m_xformedVertexBuffer) {
				// Note - m_xformedVertexBuffer should only be used for non T&L hardware.  jba.
				DX8Wrapper::Apply_Render_State_Changes();
				DX8Wrapper::_Get_D3D_Device8()->SetStreamSource(
					0,
					m_xformedVertexBuffer[j*m_numVBTilesX+i],
					D3DXGetFVFVertexSize(D3DFVF_XYZRHW |D3DFVF_DIFFUSE|D3DFVF_TEX2));
				DX8Wrapper::_Get_D3D_Device8()->SetVertexShader(D3DFVF_XYZRHW |D3DFVF_DIFFUSE|D3DFVF_TEX2);
			}
#endif				
			if (Is_Hidden() == 0) {
				DX8Wrapper::Draw_Triangles(	0,numPolys, 0,	numVertex);
			}
		}
}

//=============================================================================
// HeightMapRenderObjClass::renderTrees
//=============================================================================
/** Renders (draws) the trees. Since the trees are transparent, this has to be
called after flush. */
//=============================================================================
void HeightMapRenderObjClass::renderTrees(CameraClass * camera)
{
#ifdef EXTENDED_STATS
	if (DX8Wrapper::stats.m_disableObjects) {
		return;
	}
#endif
	if (m_map==NULL) return;
	if (Scene==NULL) return;
	if (m_treeBuffer) {
		Matrix3D tm(Transform);
		DX8Wrapper::Set_Transform(D3DTS_WORLD,tm);
		DX8Wrapper::Set_Material(m_vertexMaterialClass);
		RTS3DScene *pMyScene = (RTS3DScene *)Scene;
		RefRenderObjListIterator pDynamicLightsIterator(pMyScene->getDynamicLights());
		m_treeBuffer->drawTrees(camera, &pDynamicLightsIterator);
	}
}

/** Renders an additoinal terrain pass including only those tiles which have more than 2 textures
blended together.  Used primarily for corner cases where 3 different textures meet.*/
void HeightMapRenderObjClass::renderExtraBlendTiles(void)
{
	Int vertexCount = 0;
	Int indexCount = 0;
	Int xExtent = m_map->getXExtent();
	Int border = m_map->getBorderSize();
	static Int maxBlendTiles = DEFAULT_MAX_FRAME_EXTRABLEND_TILES;

	if (!m_numExtraBlendTiles)
		return;	//nothing to draw

	if (maxBlendTiles > 10000)	//we can only fit about 10000 tiles into a single VB.
		maxBlendTiles = 10000;

	DynamicVBAccessClass vb_access(BUFFER_TYPE_DYNAMIC_DX8,DX8_FVF_XYZNDUV2,maxBlendTiles*4);
	DynamicIBAccessClass ib_access(BUFFER_TYPE_DYNAMIC_DX8,maxBlendTiles*6);
	{

		DynamicVBAccessClass::WriteLockClass lock(&vb_access);
		VertexFormatXYZNDUV2* vb= lock.Get_Formatted_Vertex_Array();
		DynamicIBAccessClass::WriteLockClass lockib(&ib_access);
		UnsignedShort *ib=lockib.Get_Index_Array();

		if (!vb || !ib) return;

		const UnsignedByte* data = m_map->getDataPtr();

		//Loop over visible terrain and extract all the tiles that need extra blend
		Int drawEdgeY=m_map->getDrawOrgY()+m_map->getDrawHeight()-1;
		Int drawEdgeX=m_map->getDrawOrgX()+m_map->getDrawWidth()-1;
		if (drawEdgeX > (m_map->getXExtent()-1))
			drawEdgeX = m_map->getXExtent()-1;
		if (drawEdgeY > (m_map->getYExtent()-1))
			drawEdgeY = m_map->getYExtent()-1;
		Int drawStartX=m_map->getDrawOrgX();
		Int drawStartY=m_map->getDrawOrgY();

		for (Int j=0; j<m_numExtraBlendTiles; j++)
		{
			if (vertexCount >= (maxBlendTiles*4))
				break;	//no room in vertex buffer

			Real U[4],V[4];
			UnsignedByte alpha[4];
			Bool flipState,cliffState;
			Int x = m_extraBlendTilePositions[j] & 0xffff;
			Int y = m_extraBlendTilePositions[j] >> 16;

			if (x >= drawStartX && x < drawEdgeX &&
				y >= drawStartY && y < drawEdgeY &&
				m_map->getExtraAlphaUVData(x,y,U,V,alpha,&flipState, &cliffState))
			{	//this tile is inside visible region and has 3rd blend layer.

				Int idx = x+y*xExtent;

				Real p0=data[idx]*MAP_HEIGHT_SCALE;
				Real p1=data[idx+1]*MAP_HEIGHT_SCALE;
				Real p2=data[idx + 1 + xExtent]*MAP_HEIGHT_SCALE;
				Real p3=data[idx + xExtent]*MAP_HEIGHT_SCALE;
				if (cliffState && abs(p0-p2) > abs(p1-p3))	//cliffs sometimes force a flip
					flipState = TRUE;

				vb->x=(x-border)*MAP_XY_FACTOR;	 
				vb->y=(y-border)*MAP_XY_FACTOR;
				vb->z=p0;
				vb->diffuse=(alpha[0]<<24)|(getStaticDiffuse(x,y) & 0x00ffffff);
				vb->u1=U[0];
				vb->v1=V[0];
				vb++;

				vb->x=(x+1-border)*MAP_XY_FACTOR;	 
				vb->y=(y-border)*MAP_XY_FACTOR;
				vb->z=p1;
				vb->diffuse=(alpha[1]<<24)|(getStaticDiffuse(x+1,y) & 0x00ffffff);
				vb->u1=U[1];
				vb->v1=V[1];
				vb++;

				vb->x=(x+1-border)*MAP_XY_FACTOR;	 
				vb->y=(y+1-border)*MAP_XY_FACTOR;
				vb->z=p2;
				vb->diffuse=(alpha[2]<<24)|(getStaticDiffuse(x+1,y+1) & 0x00ffffff);
				vb->u1=U[2];
				vb->v1=V[2];
				vb++;

				vb->x=(x-border)*MAP_XY_FACTOR;	 
				vb->y=(y+1-border)*MAP_XY_FACTOR;
				vb->z=p3;
				vb->diffuse=(alpha[3]<<24)|(getStaticDiffuse(x,y+1) & 0x00ffffff);
				vb->u1=U[3];
				vb->v1=V[3];
				vb++;
				
				if (flipState)
				{
					ib[0]=1+vertexCount;
					ib[1]=3+vertexCount;
					ib[2]=0+vertexCount;
					ib[3]=1+vertexCount;
					ib[4]=2+vertexCount;
					ib[5]=3+vertexCount;
				}
				else
				{
					ib[0]=0+vertexCount;
					ib[1]=2+vertexCount;
					ib[2]=3+vertexCount;
					ib[3]=0+vertexCount;
					ib[4]=1+vertexCount;
					ib[5]=2+vertexCount;
				}
				ib += 6;
				vertexCount +=4;
				indexCount +=6;
			}//tile has 3rd blend layer and is visible
		}	//for all extre blend tiles
	}//unlock vertex buffer

	if (vertexCount)
	{
		//Check if we couldn't fit all blend tiles into vertex buffer so we can enlarge it for next frame.
		if (vertexCount == (maxBlendTiles*4))
			maxBlendTiles += 16;	//enlarge by 16 to reduce trashing.
		
		ShaderClass::Invalidate();	//invalidate to force shader to reset since we directly changed states
		DX8Wrapper::Set_Index_Buffer(ib_access,0);
		DX8Wrapper::Set_Vertex_Buffer(vb_access);
		VertexMaterialClass *vmat=VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
		DX8Wrapper::Set_Material(vmat);
		REF_PTR_RELEASE(vmat);
		ShaderClass shader=ShaderClass::_PresetOpaqueShader;
		shader.Set_Depth_Mask(ShaderClass::DEPTH_WRITE_DISABLE);	//disable writes to z
		DX8Wrapper::Set_Shader(shader);

		if (TheGlobalData->m_use3WayTerrainBlends == 2)
		{
			shader.Set_Primary_Gradient(ShaderClass::GRADIENT_DISABLE);	//disable lighting.
			shader.Set_Texturing(ShaderClass::TEXTURING_DISABLE);		//disable texturing.
			DX8Wrapper::Set_Shader(shader);
			DX8Wrapper::Set_Texture(0,NULL);	//debug mode which draws terrain tiles in white.
			if (Is_Hidden() == 0) {
				DX8Wrapper::Draw_Triangles(	0,indexCount/3, 0,	vertexCount);	//draw a quad, 2 triangles, 4 verts
			}
		}
		else
		{
			W3DShaderManager::setTexture(0,m_stageOneTexture);
			W3DShaderManager::setTexture(1,m_stageTwoTexture);	//cloud
			W3DShaderManager::setTexture(2,m_stageThreeTexture);	//noise/lightmap

			W3DShaderManager::ShaderTypes st = W3DShaderManager::ST_ROAD_BASE;

			Bool doCloud = TheGlobalData->m_useCloudMap;
			if (TheGlobalData->m_timeOfDay == TIME_OF_DAY_NIGHT) {
				doCloud = false;
			}

			if (TheGlobalData->m_useLightMap && doCloud)
 			{	
				st = W3DShaderManager::ST_ROAD_BASE_NOISE12;
 			}
 			else if (TheGlobalData->m_useLightMap)
 			{	//lightmap only
 				st = W3DShaderManager::ST_ROAD_BASE_NOISE2;
 			}
 			else if (doCloud)
 			{	//cloudmap only
 				st = W3DShaderManager::ST_ROAD_BASE_NOISE1;
 			}

			Int devicePasses=W3DShaderManager::getShaderPasses(st);

			for (Int pass=0; pass < devicePasses; pass++)
			{
				W3DShaderManager::setShader(st, pass);
				//Draw all this road type.
				if (Is_Hidden() == 0) {
					DX8Wrapper::Draw_Triangles(	0,indexCount/3, 0,	vertexCount);	//draw a quad, 2 triangles, 4 verts
				}
			}
			W3DShaderManager::resetShader(st);
		}
  }
}
