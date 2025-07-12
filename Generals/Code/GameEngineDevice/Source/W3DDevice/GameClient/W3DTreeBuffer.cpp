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

// FILE: W3DTreeBuffer.cpp ////////////////////////////////////////////////
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
// File name: W3DTreeBuffer.cpp
//
// Created:   John Ahlquist, May 2001
//
// Desc:      Draw buffer to handle all the trees in a scene.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//         Includes                                                      
//-----------------------------------------------------------------------------
#include "W3DDevice/GameClient/W3DTreeBuffer.h"

#include <stdio.h>
#include <string.h>
#include <assetmgr.h>
#include <texture.h>
#include "Common/GlobalData.h"
#include "GameClient/ClientRandomValue.h"
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
	ShaderClass::ALPHATEST_DISABLE, ShaderClass::CULL_MODE_DISABLE, \
	ShaderClass::DETAILCOLOR_DISABLE, ShaderClass::DETAILALPHA_DISABLE) )

static ShaderClass detailAlphaShader(SC_ALPHA_DETAIL);


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
// W3DTreeBuffer::cull
//=============================================================================
/** Culls the trees, marking the visible flag.  If a tree becomes visible, it sets
it's sortKey */
//=============================================================================
void W3DTreeBuffer::cull(CameraClass * camera)
{
	Int curTree;

	m_anythingChanged = false;
	// Calulate the vector direction that the camera is looking at.
	Matrix3D camera_matrix = camera->Get_Transform();
	float zmod = -1;
	float x = zmod * camera_matrix[0][2] ;
	float y = zmod * camera_matrix[1][2] ;
	float z = zmod * camera_matrix[2][2] ;
	m_cameraLookAtVector.Set(x,y,z);

	for (curTree=0; curTree<m_numTrees; curTree++) {
		Bool doKey = false;	// We calculate the key when a tree becomes visible.
		Bool visible = !camera->Cull_Sphere(m_trees[curTree].bounds);
		if (visible!=m_trees[curTree].visible) {
			m_trees[curTree].visible=visible;
			m_anythingChanged = true;
			if (visible) {
				doKey = true;
			}
		}
		// Also calculate sort key if a tree is visible, and the view changed setting m_updateAllKeys to true.
		if (doKey || (visible&&m_updateAllKeys)) {
			// The sort key is essentially the distance of location in the direction of the 
			// camera look at.
			m_trees[curTree].sortKey = Vector3::Dot_Product(m_trees[curTree].location, m_cameraLookAtVector); 
		}
	}
	m_updateAllKeys = false;
}

//=============================================================================
// W3DTreeBuffer::cullMirror
//=============================================================================
/** Culls the trees, marking the visible flag for the mirror view. Unlike cull(),
doesn't update anything except the visible flag. */
//=============================================================================
void W3DTreeBuffer::cullMirror(CameraClass * camera)
{
	Int curTree;
	m_anythingChanged = false;
	for (curTree=0; curTree<m_numTrees; curTree++) {
		if (!m_trees[curTree].mirrorVisible) {
			if (m_trees[curTree].visible) {
				m_anythingChanged = true;
			}
			m_trees[curTree].visible = false;
			continue;
		}
		Bool visible = !camera->Cull_Sphere(m_trees[curTree].bounds);
		m_trees[curTree].visible=visible;
	}
}

//=============================================================================
// W3DTreeBuffer::sort
//=============================================================================
/** Sorts the trees.  Does num_iterations of a bubble sort.  This is good because
it ends immediately if the trees are already sorted (which is most of the time)
and will perform a fixed amount of work each frame until it becomes sorted. */
//=============================================================================
void W3DTreeBuffer::sort(Int numIterations)
{
	// sort in descending order.
	Int iter;
	Bool swap = false;
	for (iter = 0; iter<numIterations; iter++) {
		Int cur = 0;
		// Note - only sorts the visible trees.
		while (cur<m_numTrees-iter && !m_trees[cur].visible) {
			cur++;
		}
		Int i;
		for (i=cur+1; i<m_numTrees-iter; i++) {
			if (m_trees[i].visible) {
				if (m_trees[cur].sortKey > m_trees[i].sortKey) {
					TTree tmp = m_trees[cur];
					m_trees[cur] = m_trees[i];
					m_trees[i] = tmp;
					swap = true;
				}
				cur = i;
			}
		}
		if (!swap) {
			return;
		}
		m_anythingChanged = true;
	}
}

//=============================================================================
// W3DTreeBuffer::doLighting
//=============================================================================
/** Calculates the diffuse lighting as affected by dynamic lighting. */
//=============================================================================
Int W3DTreeBuffer::doLighting(Vector3 *loc, Real r, Real g, Real b, SphereClass &bounds, RefRenderObjListIterator *pDynamicLightsIterator)
{
	if (pDynamicLightsIterator == NULL) {
		return(REAL_TO_INT(b) | (REAL_TO_INT(g) << 8) | (REAL_TO_INT(r) << 16) | (255 << 24));
	}
	Real shadeR, shadeG, shadeB;
	shadeR = r;
	shadeG = g;
	shadeB = b;
	Bool didLight = false;
	for (pDynamicLightsIterator->First(); !pDynamicLightsIterator->Is_Done(); pDynamicLightsIterator->Next())
	{		
		W3DDynamicLight *pLight = (W3DDynamicLight*)pDynamicLightsIterator->Peek_Obj();
		if (!pLight->isEnabled()) {
			continue; // he is turned off.
		}
		if (CollisionMath::Overlap_Test(bounds, pLight->Get_Bounding_Sphere()) == CollisionMath::OUTSIDE) {
			continue; // this tree is outside of the light's influence.
		}
		Vector3 lightDirection = *loc;
		Real factor;
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
			} 
			break;
		case LightClass::DIRECTIONAL:
			pLight->Get_Spot_Direction(lightDirection);
			factor = 1.0;
			break;
		};
		Real shade = 0.5f * factor; // Assume adjustment for diffuse.
		Vector3 vDiffuse;
		pLight->Get_Diffuse(&vDiffuse);
		Vector3 ambient;
		pLight->Get_Ambient(&ambient);
		if (shade > 1.0) shade = 1.0;
		if(shade < 0.0f) shade = 0.0f;
		shadeR += shade*vDiffuse.X;
		shadeG += shade*vDiffuse.Y;
		shadeB += shade*vDiffuse.Z;		
		shadeR += factor*ambient.X;
		shadeG += factor*ambient.Y;
		shadeB += factor*ambient.Z;		
		didLight = true;
	} 
	if (!didLight) {
		return(REAL_TO_INT(b) | (REAL_TO_INT(g) << 8) | (REAL_TO_INT(r) << 16) | (255 << 24));
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
	return(REAL_TO_INT(shadeB) | (REAL_TO_INT(shadeG) << 8) | (REAL_TO_INT(shadeR) << 16) | (255 << 24));
}

//=============================================================================
// W3DTreeBuffer::loadTreesInVertexAndIndexBuffers
//=============================================================================
/** Loads the trees into the vertex buffer for drawing. */
//=============================================================================
void W3DTreeBuffer::loadTreesInVertexAndIndexBuffers(RefRenderObjListIterator *pDynamicLightsIterator)
{
	if (!m_indexTree || !m_vertexTree || !m_initialized) {
		return;
	}
	if (!m_anythingChanged) {
		//return;
	}
	m_curNumTreeVertices = 0;
	m_curNumTreeIndices = 0;
	VertexFormatXYZDUV1 *vb;
	UnsignedShort *ib;
	// Lock the buffers.
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexTree);
	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexTree);
	vb=(VertexFormatXYZDUV1*)lockVtxBuffer.Get_Vertex_Array();
	ib = lockIdxBuffer.Get_Index_Array();
	// Add to the index buffer & vertex buffer.
	Vector2 lookAtVector(m_cameraLookAtVector.X, m_cameraLookAtVector.Y);
	lookAtVector.Normalize();
	// We draw from back to front, so we put the indexes in the buffer 
	// from back to front.
	UnsignedShort *curIb = ib+MAX_TREE_INDEX;

	VertexFormatXYZDUV1 *curVb = vb;

	Int curTree;

	// Calculate a static lighting value to use for all the trees.
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

	for (curTree=0; curTree<m_numTrees; curTree++) {

		if (!m_trees[curTree].visible) continue;
		Real scale = m_trees[curTree].scale;
		Vector3 loc = m_trees[curTree].location;
		Int type = m_trees[curTree].treeType;
		Real theSin = m_trees[curTree].sin;
		Real theCos = m_trees[curTree].cos;
		if (type<0 || m_typeMesh[type] == 0) {
			type = 0;
		}
		Bool doVertexLighting = false;

#if 0 // no dynamic lighting.
		for (pDynamicLightsIterator->First(); !pDynamicLightsIterator->Is_Done(); pDynamicLightsIterator->Next())
		{		
			W3DDynamicLight *pLight = (W3DDynamicLight*)pDynamicLightsIterator->Peek_Obj();
			if (!pLight->isEnabled()) {
				continue; // he is turned off.
			}
			if (CollisionMath::Overlap_Test(m_trees[curTree].bounds, pLight->Get_Bounding_Sphere()) == CollisionMath::OUTSIDE) {
				continue; // this tree is outside of the light's influence.
			}
			doVertexLighting = true;
		}
#endif

		Int diffuse = 0;
		if (!doVertexLighting) {
			diffuse = doLighting(&loc, shadeR, shadeG, shadeB, m_trees[curTree].bounds, NULL);
		}

		Real typeOffset = type*0.5;
		Int startVertex = m_curNumTreeVertices;
		Int i, j;
		Int numVertex = m_typeMesh[type]->Peek_Model()->Get_Vertex_Count();
		Vector3 *pVert = m_typeMesh[type]->Peek_Model()->Get_Vertex_Array();
		// If we happen to have too many trees, stop.
		if (m_curNumTreeVertices+numVertex+2>= MAX_TREE_VERTEX) {
			break;
		}
		Int numIndex = m_typeMesh[type]->Get_Model()->Get_Polygon_Count();
		const Vector3i *pPoly = m_typeMesh[type]->Get_Model()->Get_Polygon_Array();
		if (m_curNumTreeIndices+3*numIndex+6 >= MAX_TREE_INDEX) {
			break;
		}

		const Vector2*uvs=m_typeMesh[type]->Get_Model()->Get_UV_Array_By_Index(0);

		// If we are doing reduced resolution terrain, do reduced
		// poly trees.
		Bool doPanel = (TheGlobalData->m_useHalfHeightMap || TheGlobalData->m_stretchTerrain);

		if (doPanel) {
			if (m_trees[curTree].rotates) {
				theSin = -lookAtVector.X;
				theCos = lookAtVector.Y;
			}
			// panel start is index offset, there are 3 index per triangle.
			if (m_trees[curTree].panelStart/3 + 2 > numIndex) {
				continue; // not enought polygons for the offset.  jba.
			}
			for (j=0; j<6; j++) {
				i = ((Int *)pPoly)[j+m_trees[curTree].panelStart];
				if (m_curNumTreeVertices >= MAX_TREE_VERTEX) 
					break;

				// Update the uv values.  The W3D models each have their own texture, and 
				// we use one texture with all images in one, so we have to change the uvs to 
				// match.
				Real U, V;
				if (type==SHRUB) {
					// shrub texture is tucked in the corner
					U = ((512-64)+uvs[i].U*64.0f)/512.0f;
					V = ((256-64)+uvs[i].V*64.0f)/256.0f;
				} else if (type==FENCE) {
					U = uvs[i].U*0.5f; 
					V = 1.0f + uvs[i].V;		
				} else {
					U = typeOffset+uvs[i].U*0.5f; 
					V = uvs[i].V;		
				}
				curVb->u1 = U;
				curVb->v1 = V/2.0;
				Vector3 vLoc;
				vLoc.X = pVert[i].X*scale*theCos - pVert[i].Z*scale*theSin;
				vLoc.Y = pVert[i].Z*scale*theCos + pVert[i].X*scale*theSin;

				vLoc.X += loc.X;
				vLoc.Y += loc.Y;
				vLoc.Z = loc.Z + pVert[i].Y*scale; // In W3D z is up, in 3dMax y is up.

				curVb->x = vLoc.X;
				curVb->y = vLoc.Y;
				curVb->z = vLoc.Z;	 
				if (doVertexLighting) {
					curVb->diffuse = doLighting(&vLoc, shadeR, shadeG, shadeB, m_trees[curTree].bounds, pDynamicLightsIterator);
				} else {
					curVb->diffuse = diffuse;
				}
				curVb++;
				m_curNumTreeVertices++;
			}

			for (i=0; i<6; i++) {
				if (m_curNumTreeIndices+4 > MAX_TREE_INDEX) 
					break;
				curIb--;
				*curIb = startVertex + i;
				m_curNumTreeIndices++;
			}
		} else {
			for (i=0; i<numVertex; i++) {
				if (m_curNumTreeVertices >= MAX_TREE_VERTEX) 
					break;

				// Update the uv values.  The W3D models each have their own texture, and 
				// we use one texture with all images in one, so we have to change the uvs to 
				// match.
				Real U, V;
				if (type==SHRUB) {
					// shrub texture is tucked in the corner
					U = ((512-64)+uvs[i].U*64.0f)/512.0f;
					V = ((256-64)+uvs[i].V*64.0f)/256.0f;
				} else if (type==FENCE) {
					U = uvs[i].U*0.5f; 
					V = 1.0f + uvs[i].V;		
				} else {
					U = typeOffset+uvs[i].U*0.5f; 
					V = uvs[i].V;		
				}
				curVb->u1 = U;
				curVb->v1 = V/2.0;
				Vector3 vLoc;
				vLoc.X = pVert[i].X*scale*theCos - pVert[i].Z*scale*theSin;
				vLoc.Y = pVert[i].Z*scale*theCos + pVert[i].X*scale*theSin;

				vLoc.X += loc.X;
				vLoc.Y += loc.Y;
				vLoc.Z = loc.Z + pVert[i].Y*scale; // In W3D z is up, in 3dMax y is up.

				curVb->x = vLoc.X;
				curVb->y = vLoc.Y;
				curVb->z = vLoc.Z;	 
				if (doVertexLighting) {
					curVb->diffuse = doLighting(&vLoc, shadeR, shadeG, shadeB, m_trees[curTree].bounds, pDynamicLightsIterator);
				} else {
					curVb->diffuse = diffuse;
				}
				curVb++;
				m_curNumTreeVertices++;
			}

			for (i=0; i<numIndex; i++) {
				if (m_curNumTreeIndices+4 > MAX_TREE_INDEX) 
					break;
				curIb-=3;
				*curIb++ = startVertex + pPoly[i].I;
				*curIb++ = startVertex + pPoly[i].J;
				*curIb++ = startVertex + pPoly[i].K;
 				curIb-=3;
				m_curNumTreeIndices+=3;
			}
		}		
	}
	m_curTreeIndexOffset = curIb - ib;	
}

//-----------------------------------------------------------------------------
//         Public Functions                                                
//-----------------------------------------------------------------------------

//=============================================================================
// W3DTreeBuffer::~W3DTreeBuffer
//=============================================================================
/** Destructor. Releases w3d assets. */
//=============================================================================
W3DTreeBuffer::~W3DTreeBuffer(void)
{
	freeTreeBuffers();
	REF_PTR_RELEASE(m_treeTexture);
	Int i;
	for (i=0; i<MAX_TYPES; i++) {
		REF_PTR_RELEASE(m_typeMesh[i]);
	}
}

//=============================================================================
// W3DTreeBuffer::W3DTreeBuffer
//=============================================================================
/** Constructor. Sets m_initialized to true if it finds the w3d models it needs
for the trees. */
//=============================================================================
W3DTreeBuffer::W3DTreeBuffer(void)
{
	m_initialized = false;
	///@toto - reactivate this optimization if useful.  jba.
	return;
	m_vertexTree = NULL;
	m_indexTree = NULL;
	m_treeTexture = NULL;
	m_curNumTreeVertices=0;
	m_curNumTreeIndices=0;
	clearAllTrees();
	allocateTreeBuffers();
	Int i;
	for (i=0; i<MAX_TYPES; i++) {
		m_typeMesh[i] = 0;
	}
	if (WW3DAssetManager::Get_Instance()==NULL)
		return;  // WorldBuilderTool doesn't initialize the asset manager.  jba.

	m_treeTexture = NEW_REF(TextureClass, ("trees.tga"));
	m_treeTexture->Get_Filter().Set_U_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
	m_treeTexture->Get_Filter().Set_V_Addr_Mode(TextureFilterClass::TEXTURE_ADDRESS_CLAMP);
	for (i=0; i<MAX_TYPES; i++) {
		switch(i) {
		case 0:
			m_typeMesh[i] = (MeshClass*)WW3DAssetManager::Get_Instance()->Create_Render_Obj("ALPINETREE.TREE 1" );
			break;
		case 1:
			m_typeMesh[i] = (MeshClass*)WW3DAssetManager::Get_Instance()->Create_Render_Obj("DECIDUOUS.TREE 1" );
			break;
		case 2:
			m_typeMesh[i] = (MeshClass*)WW3DAssetManager::Get_Instance()->Create_Render_Obj("SHRUB.TREE 1" );
			break;
		case 3:
			m_typeMesh[i] = (MeshClass*)WW3DAssetManager::Get_Instance()->Create_Render_Obj("FENCE.PLANE09" );
			break;
		}
		if (m_typeMesh[i] == NULL) continue;
		Int numVertex = m_typeMesh[i]->Peek_Model()->Get_Vertex_Count();
		Vector3 *pVert = m_typeMesh[i]->Peek_Model()->Get_Vertex_Array();

		const Matrix3D xfm = m_typeMesh[i]->Get_Transform();
		SphereClass bounds(pVert, numVertex);
		// Model is in y up format, so swap y and z.
		Real tmp;
		tmp = bounds.Center.Y;
		bounds.Center.Y = bounds.Center.Z;
		bounds.Center.Z = tmp;
		m_typeBounds[i] = bounds;
	}
	if (m_typeMesh[0] == NULL) {

		//DEBUG_LOG("!!!!!!!!!!!!*************** W3DTreeBuffer failed to initialize.");
		return;  // didn't initialize.
	}
	m_initialized = true;
}


//=============================================================================
// W3DTreeBuffer::freeTreeBuffers
//=============================================================================
/** Frees the index and vertex buffers. */
//=============================================================================
void W3DTreeBuffer::freeTreeBuffers(void)
{
	REF_PTR_RELEASE(m_vertexTree);
	REF_PTR_RELEASE(m_indexTree);
}

//=============================================================================
// W3DTreeBuffer::allocateTreeBuffers
//=============================================================================
/** Allocates the index and vertex buffers. */
//=============================================================================
void W3DTreeBuffer::allocateTreeBuffers(void)
{
	m_vertexTree=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZDUV1,MAX_TREE_VERTEX+4,DX8VertexBufferClass::USAGE_DYNAMIC));
	m_indexTree=NEW_REF(DX8IndexBufferClass,(2*MAX_TREE_INDEX+4, DX8IndexBufferClass::USAGE_DYNAMIC));
	m_curNumTreeVertices=0;
	m_curNumTreeIndices=0;
}

//=============================================================================
// W3DTreeBuffer::clearAllTrees
//=============================================================================
/** Removes all trees. */
//=============================================================================
void W3DTreeBuffer::clearAllTrees(void)
{
	m_numTrees=0;
}

//=============================================================================
// W3DTreeBuffer::addTree
//=============================================================================
/** Adds a tree.  Name is the W3D model name, supported models are
ALPINE, DECIDUOUS and SHRUB. */
//=============================================================================
void W3DTreeBuffer::addTree(Coord3D loc, Real scale, Real angle, AsciiString name, Bool mirrorVisible)
{
	if (m_numTrees >= MAX_TREES) {
		return;  
	}

	if (!m_initialized) {
		return;  
	}

	TTreeType treeType = ALPINE_TREE;
	if (!name.compare("DECIDUOUS")) {
		treeType = DECIDUOUS_TREE;
	}	else if (!name.compare("SHRUB")) {
		treeType = SHRUB;
	}	else if (!name.compare("FENCE")) {
		treeType = FENCE;
	}
	m_trees[m_numTrees].panelStart = 0;
	Real randomScale = GameClientRandomValueReal( 0.7f, 1.3f );
	if (treeType == FENCE) {
		// Fences don't randomly scale & orient
		m_trees[m_numTrees].sin = WWMath::Sin(angle);
		m_trees[m_numTrees].scale = scale;
		m_trees[m_numTrees].cos = WWMath::Cos(angle);
		m_trees[m_numTrees].rotates = false;
		m_trees[m_numTrees].panelStart = 48;
	} else {
		// Randomizes the scale and orientation of trees.
		m_trees[m_numTrees].sin = GameClientRandomValueReal( -1.0f, 1.0f );
		m_trees[m_numTrees].scale = scale*randomScale;
		m_trees[m_numTrees].cos = WWMath::Sqrt(1.0 - m_trees[m_numTrees].sin*m_trees[m_numTrees].sin);
		m_trees[m_numTrees].rotates = true;
	}
	m_trees[m_numTrees].location = Vector3(loc.x, loc.y, loc.z);
	m_trees[m_numTrees].treeType = treeType;
	// Translate the bounding sphere of the model.
	m_trees[m_numTrees].bounds = m_typeBounds[treeType];
	m_trees[m_numTrees].bounds.Center *= scale;
	m_trees[m_numTrees].bounds.Radius *= scale;
	m_trees[m_numTrees].bounds.Center += m_trees[m_numTrees].location;
	// Initially set it invisible.  cull will update it's visiblity flag.
	m_trees[m_numTrees].visible = false;
	m_trees[m_numTrees].mirrorVisible = mirrorVisible;
	m_numTrees++;
}


//=============================================================================
// W3DTreeBuffer::drawTrees
//=============================================================================
/** Draws the trees.  Uses camera to cull. */
//=============================================================================
void W3DTreeBuffer::drawTrees(CameraClass * camera, RefRenderObjListIterator *pDynamicLightsIterator)
{
	if (!m_isTerrainPass) {
		return;
	}
	m_isTerrainPass = false;

	if (ShaderClass::Is_Backface_Culling_Inverted()) { 
		// Mirror inverts backface culling.
		cullMirror(camera);
	} else  { 
		// Normal draw.
		cull(camera);
		// Only sort once per frame.
		sort(SORT_ITERATIONS_PER_FRAME);
	}	
	loadTreesInVertexAndIndexBuffers(pDynamicLightsIterator);

	if (m_curNumTreeIndices == 0) {
		return;
	}
	// Setup the vertex buffer, shader & texture.
	DX8Wrapper::Set_Index_Buffer(m_indexTree,0);
	DX8Wrapper::Set_Vertex_Buffer(m_vertexTree);
	DX8Wrapper::Set_Shader(detailAlphaShader);
	DX8Wrapper::Set_Texture(0,m_treeTexture);
	// Draw all the trees.
	DX8Wrapper::Draw_Triangles(	m_curTreeIndexOffset, m_curNumTreeIndices/3, 0,	m_curNumTreeVertices);
}


