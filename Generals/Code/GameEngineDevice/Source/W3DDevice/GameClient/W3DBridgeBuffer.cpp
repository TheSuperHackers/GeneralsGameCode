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

// FILE: W3DBridgeBuffer.cpp ////////////////////////////////////////////////
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
// File name: W3DBridgeBuffer.cpp
//
// Created:   John Ahlquist, May 2001
//
// Desc:      Draw buffer to handle all the bridges in a scene.
//
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//         Includes                                                      
//-----------------------------------------------------------------------------
#include "W3DDevice/GameClient/W3DBridgeBuffer.h"

#include <stdio.h>
#include <string.h>
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include <texture.h>
#include "Common/GlobalData.h"
#include "Common/RandomValue.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "GameClient/TerrainRoads.h"
#include "GameLogic/Damage.h"
#include "GameLogic/Module/BodyModule.h"
#include "W3DDevice/GameLogic/W3DTerrainLogic.h"
#include "W3DDevice/GameClient/TerrainTex.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DDynamicLight.h"
#include "W3DDevice/GameClient/Module/W3DModelDraw.h"
#include "W3DDevice/GameClient/W3DShaderManager.h"
#include "W3DDevice/GameClient/W3DShroud.h"
#include "WW3D2/camera.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/dx8renderer.h"
#include "WW3D2/mesh.h"
#include "WW3D2/meshmdl.h"
#include "WW3D2/scene.h"

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

//-----------------------------------------------------------------------------
//         Private Data                                                     
//-----------------------------------------------------------------------------
// A W3D shader that does alpha, texturing, tests zbuffer, doesn't update zbuffer.
#define SC_ALPHA_DETAIL ( SHADE_CNST(ShaderClass::PASS_LEQUAL, ShaderClass::DEPTH_WRITE_ENABLE, ShaderClass::COLOR_WRITE_ENABLE, ShaderClass::SRCBLEND_SRC_ALPHA, \
	ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA, ShaderClass::FOG_DISABLE, ShaderClass::GRADIENT_MODULATE, ShaderClass::SECONDARY_GRADIENT_DISABLE, ShaderClass::TEXTURING_ENABLE, \
	ShaderClass::ALPHATEST_ENABLE, ShaderClass::CULL_MODE_DISABLE, \
	ShaderClass::DETAILCOLOR_DISABLE, ShaderClass::DETAILALPHA_DISABLE) )

static ShaderClass detailAlphaShader(SC_ALPHA_DETAIL);


#define SC_ALPHA_MIRROR ( SHADE_CNST(ShaderClass::PASS_LEQUAL, ShaderClass::DEPTH_WRITE_ENABLE, ShaderClass::COLOR_WRITE_ENABLE, ShaderClass::SRCBLEND_ONE, \
	ShaderClass::DSTBLEND_ZERO, ShaderClass::FOG_DISABLE, ShaderClass::GRADIENT_MODULATE, ShaderClass::SECONDARY_GRADIENT_DISABLE, ShaderClass::TEXTURING_ENABLE, \
	ShaderClass::ALPHATEST_DISABLE, ShaderClass::CULL_MODE_DISABLE, \
	ShaderClass::DETAILCOLOR_DISABLE, ShaderClass::DETAILALPHA_DISABLE) )

static ShaderClass detailShader(SC_ALPHA_MIRROR);

#define NO_USE_BRIDGE_NORMALS

//-----------------------------------------------------------------------------
//         Private Classes                                               
//-----------------------------------------------------------------------------
//=============================================================================
// W3DBridge constructor.
//=============================================================================
/** Initializes pointers & values.  */
//=============================================================================
W3DBridge::W3DBridge() :
m_bridgeTexture(NULL),
m_leftMesh(NULL),
m_sectionMesh(NULL),
m_rightMesh(NULL),
m_visible(false),
m_curDamageState(BODY_PRISTINE),
m_scale(1.0)
{
}

//=============================================================================
// W3DBridge destructor.
//=============================================================================
/** Frees objects.  */
//=============================================================================
W3DBridge::~W3DBridge(void)
{
	clearBridge();
}

//=============================================================================
// W3DBridge::renderBridge
//=============================================================================
/** Renders the bride.  It is assumed that the shared vertex and index buffers
are already set.  */
//=============================================================================
void W3DBridge::renderBridge(Bool wireframe)
{
	if (m_visible && m_numPolygons && m_numVertex) {
		if (!wireframe) DX8Wrapper::Set_Texture(0,m_bridgeTexture);
		// Draw all the bridges.
		DX8Wrapper::Draw_Triangles(	m_firstIndex, m_numPolygons, m_firstVertex,	m_numVertex);
	}
}

//=============================================================================
// W3DBridge::clearBridge
//=============================================================================
/** Frees all bridge objects (meshes & texture).  */
//=============================================================================
void W3DBridge::clearBridge(void)
{
	m_visible = false;
	REF_PTR_RELEASE(m_bridgeTexture);
	REF_PTR_RELEASE(m_leftMesh);
	REF_PTR_RELEASE(m_sectionMesh);
	REF_PTR_RELEASE(m_rightMesh);
}

//=============================================================================
// W3DBridge::cullBridge
//=============================================================================
/** Culls bridge to camera.  */
//=============================================================================
Bool W3DBridge::cullBridge(CameraClass * camera)
{
	///@todo - cull bridges. 
	Bool wasVisible = m_visible;

	m_visible = true;

	return(wasVisible != m_visible);

}

#define BRIDGE_FLOAT_AMT (0.25f)

//=============================================================================
// W3DBridge::init
//=============================================================================
/** Inits a bridges location & type so it can be load'ed.  */
//=============================================================================
void W3DBridge::init(Vector3 fromLoc, Vector3 toLoc, AsciiString bridgeTemplateName)
{
	m_start = fromLoc;
	m_end = toLoc;
	m_templateName = bridgeTemplateName;
	m_enabled = true;
}

//=============================================================================
// W3DBridge::init
//=============================================================================
/** Loads a bridge model(if not already loaded) and gets meshes for use at 
specified location.  */
//=============================================================================
Bool W3DBridge::load(enum BodyDamageType curDamageState)
{
	REF_PTR_RELEASE(m_bridgeTexture);
	REF_PTR_RELEASE(m_leftMesh);
	REF_PTR_RELEASE(m_sectionMesh);
	REF_PTR_RELEASE(m_rightMesh);

	Real scale, width, length;
	char textureFile[_MAX_PATH] = "No Texture";
	char modelName[_MAX_PATH] = "BRIDGESECTIONAL";

	/// @todo, should these be defaults in INI??? CBD
	scale = 0.7f;
	width = 34;
	length = 170;

	// try to find bridge in INI
	TerrainRoadType *bridge = TheTerrainRoads->findBridge( m_templateName );
	if (!bridge) return false;

	scale = bridge->getBridgeScale();
	switch (curDamageState) {
		default: return false;

		case 	BODY_PRISTINE:
			strcpy( textureFile, bridge->getTexture().str() );
			strcpy( modelName, bridge->getBridgeModel().str() );
			break;
		case BODY_DAMAGED:
			strcpy( textureFile, bridge->getTextureDamaged().str() );
			strcpy( modelName, bridge->getBridgeModelNameDamaged().str() );
			break;
		case BODY_REALLYDAMAGED:
			strcpy( textureFile, bridge->getTextureReallyDamaged().str() );
			strcpy( modelName, bridge->getBridgeModelNameReallyDamaged().str() );
			break;
		case BODY_RUBBLE:
			strcpy( textureFile, bridge->getTextureBroken().str() );
			strcpy( modelName, bridge->getBridgeModelNameBroken().str() );
			break;
	}

	WW3DAssetManager *pMgr = W3DAssetManager::Get_Instance();
	char left[_MAX_PATH];
	char section[_MAX_PATH];
	char right[_MAX_PATH];

	strcpy(left, modelName);
	strcat(left, ".BRIDGE_LEFT");
	strcpy(section, modelName);
	strcat(section, ".BRIDGE_SPAN");
	strcpy(right, modelName);
	strcat(right, ".BRIDGE_RIGHT");

	m_bridgeTexture = pMgr->Get_Texture(textureFile,  MIP_LEVELS_3); 
	m_leftMtx.Make_Identity();
	m_rightMtx.Make_Identity();
	m_sectionMtx.Make_Identity();

	RenderObjClass *pObj = pMgr->Create_Render_Obj(modelName );
	if (!pObj) return false;
	Int i;
	for (i=0; i<pObj->Get_Num_Sub_Objects(); i++) {
		RenderObjClass *pSub = pObj->Get_Sub_Object(i);
		Matrix3D mtx = pSub->Get_Transform();
		if (0==strnicmp(left, pSub->Get_Name(), strlen(left))) {
			m_leftMtx = mtx;
			strcpy(left, pSub->Get_Name());
		}
		if (0==strnicmp(section, pSub->Get_Name(), strlen(section))) {
			m_sectionMtx = mtx;
			strcpy(section, pSub->Get_Name());
		}
		if (0==strnicmp(right, pSub->Get_Name(), strlen(right))) {
			m_rightMtx = mtx;
			strcpy(right, pSub->Get_Name());
		}
		REF_PTR_RELEASE(pSub);
		//DEBUG_LOG(("Sub obj name %s\n", pSub->Get_Name()));
	}

	REF_PTR_RELEASE(pObj);

	m_leftMesh = (MeshClass*)pMgr->Create_Render_Obj(left );
	m_sectionMesh = (MeshClass*)pMgr->Create_Render_Obj(section);
	m_rightMesh = (MeshClass*)pMgr->Create_Render_Obj(right);
	m_scale = scale;


	if (m_leftMesh == NULL) {
		clearBridge();
		return(false);
	}
	m_bridgeType = SECTIONAL_BRIDGE;

	if (m_rightMesh == NULL || m_sectionMesh == NULL) {
		m_bridgeType = FIXED_BRIDGE;
	}

	Int numVertex = m_leftMesh->Peek_Model()->Get_Vertex_Count();
	Vector3 *pVert = m_leftMesh->Peek_Model()->Get_Vertex_Array();
	m_leftMinX = FLT_MAX;
	m_leftMaxX = -FLT_MAX;
	m_minY = FLT_MAX;
	m_maxY = -FLT_MAX;
	for (i=0; i<numVertex; i++) {
		Vector3 vert;
		Matrix3D::Transform_Vector(m_leftMtx, pVert[i], &vert);
		if (m_leftMinX > vert.X) m_leftMinX = vert.X;
		if (m_minY > vert.Y) m_minY = vert.Y;
		if (vert.X > m_leftMaxX) m_leftMaxX = vert.X;
		if (vert.Y > m_maxY) m_maxY = vert.Y;	 // Note - we assume all sections are the same width, so we only do maxY for first section.
	}
	if (m_bridgeType == SECTIONAL_BRIDGE) {
		numVertex = m_sectionMesh->Peek_Model()->Get_Vertex_Count();
		pVert = m_sectionMesh->Peek_Model()->Get_Vertex_Array();
		m_sectionMinX = FLT_MAX;
		m_sectionMaxX = -FLT_MAX;
		for (i=0; i<numVertex; i++) {
			Vector3 vert;
			Matrix3D::Transform_Vector(m_sectionMtx, pVert[i], &vert);
			if (m_sectionMinX > vert.X) m_sectionMinX = vert.X;
			if (vert.X > m_sectionMaxX) m_sectionMaxX = vert.X;
		}

		numVertex = m_rightMesh->Peek_Model()->Get_Vertex_Count();
		pVert = m_rightMesh->Peek_Model()->Get_Vertex_Array();
		m_rightMinX = FLT_MAX;
		m_rightMaxX = -FLT_MAX;
		for (i=0; i<numVertex; i++) {
			Vector3 vert;
			Matrix3D::Transform_Vector(m_rightMtx, pVert[i], &vert);
			if (m_rightMinX > vert.X) m_rightMinX = vert.X;
			if (vert.X > m_rightMaxX) m_rightMaxX = vert.X;
		}
	} else {
		m_sectionMinX = m_leftMaxX;
		m_sectionMaxX = m_leftMaxX;
		m_rightMinX = m_leftMaxX;
		m_rightMaxX = m_leftMaxX;
	}
	length = m_rightMaxX - m_leftMinX;
	if (length < 1) length = 1;
	m_length = length;
	if (m_bridgeType == SECTIONAL_BRIDGE) {
		Real allowableError = 0.05f*length;
		// make sure the sections align.

		if (m_leftMaxX>m_sectionMinX+allowableError) {
			m_bridgeType = FIXED_BRIDGE;
		}

		if (m_rightMinX<m_sectionMaxX-allowableError) {
			m_bridgeType = FIXED_BRIDGE;
		}

	}
	return(true);
}


//=============================================================================
// W3DBridge::getBridgeInfo
//=============================================================================
/** Gets the location info for the bridge.  */
//=============================================================================
void W3DBridge::getBridgeInfo(BridgeInfo *pInfo)
{

	pInfo->from.x = m_start.X;
	pInfo->from.y = m_start.Y;
	pInfo->from.z = m_start.Z;
	pInfo->to.x = m_end.X;
	pInfo->to.y = m_end.Y;
	pInfo->to.z = m_end.Z;
	pInfo->bridgeWidth = (m_maxY - m_minY) *m_scale;

	Vector3 vec = 	m_end-m_start;
	Vector3 vecNormal(-vec.Y, vec.X, 0);
	vecNormal.Normalize();

	// From left = from + vecNormal*maxY*scale
	pInfo->fromLeft.x = m_start.X + vecNormal.X * m_maxY * m_scale;
	pInfo->fromLeft.y = m_start.Y + vecNormal.Y * m_maxY * m_scale;
	pInfo->fromLeft.z = m_start.Z + vecNormal.Z * m_maxY * m_scale;

	// From right = from + vecNormal*minY*scale
	pInfo->fromRight.x = m_start.X + vecNormal.X * m_minY * m_scale;
	pInfo->fromRight.y = m_start.Y + vecNormal.Y * m_minY * m_scale;
	pInfo->fromRight.z = m_start.Z + vecNormal.Z * m_minY * m_scale;

	// to left = to + vecNormal*maxY*scale
	pInfo->toLeft.x = m_end.X + vecNormal.X * m_maxY * m_scale;
	pInfo->toLeft.y = m_end.Y + vecNormal.Y * m_maxY * m_scale;
	pInfo->toLeft.z = m_end.Z + vecNormal.Z * m_maxY * m_scale;

	// to right = to + vecNormal*minY*scale
	pInfo->toRight.x = m_end.X + vecNormal.X * m_minY * m_scale;
	pInfo->toRight.y = m_end.Y + vecNormal.Y * m_minY * m_scale;
	pInfo->toRight.z = m_end.Z + vecNormal.Z * m_minY * m_scale;

}



//=============================================================================
// W3DBridge::getModelVertices
//=============================================================================
/** Gets the vertex values for a section of a bridge.  */
//=============================================================================
Int W3DBridge::getModelVertices(VertexFormatXYZNDUV1 *destination_vb, Int curVertex, Real xOffset,
																Vector3 &vec, Vector3 &vecNormal, Vector3 &vecZ, Vector3 &offset, 
																const Matrix3D &mtx, 
																MeshClass *pMesh, RefRenderObjListIterator *pLightsIterator)
{
	if (pMesh == NULL) 
		return(0);

	Int i;
	Int numVertex = pMesh->Peek_Model()->Get_Vertex_Count();
	Vector3 *pVert = pMesh->Peek_Model()->Get_Vertex_Array();

	const Vector3 *pNormal = 	pMesh->Peek_Model()->Get_Vertex_Normal_Array();

	// If we happen to have too many bridges, stop.
	if (curVertex+numVertex+2>= W3DBridgeBuffer::MAX_BRIDGE_VERTEX) {
		return(0);
	}

	Vector3 lightRay[MAX_GLOBAL_LIGHTS];
	const Coord3D *lightPos;

	for (Int lightIndex=0; lightIndex < TheGlobalData->m_numGlobalLights; lightIndex++)
	{
		lightPos=&TheGlobalData->m_terrainLightPos[lightIndex];
		lightRay[lightIndex].Set(-lightPos->x,-lightPos->y,	-lightPos->z);
//		__asm {int 3}; //see if it really needs normalization!!
		lightRay[lightIndex].Normalize();
	}

	const Vector2*uvs=pMesh->Peek_Model()->Get_UV_Array_By_Index(0);
	VertexFormatXYZNDUV1 *curVb = destination_vb+curVertex;

	for (i=0; i<numVertex; i++) {
		curVb->u1 = uvs[i].U;
		curVb->v1 = uvs[i].V;
		Vector3 vLoc;
		Vector3 vertex;
		Matrix3D::Transform_Vector(mtx, pVert[i], &vertex);
		vLoc = (vertex.X+xOffset) * vec + vertex.Y*vecNormal + vertex.Z*vecZ;

		vLoc.X += m_start.X;
		vLoc.Y += m_start.Y;
		vLoc.Z += m_start.Z; 

		curVb->x = vLoc.X;
		curVb->y = vLoc.Y;
		curVb->z = vLoc.Z;
		
		VERTEX_FORMAT vb;
		vb.x = vLoc.X;
		vb.y = vLoc.Y;
		vb.z = vLoc.Z;

		Vector3 normal;
		Matrix3D::Rotate_Vector(mtx, pNormal[i], &normal);
		curVb->diffuse = 0xFF000000;
#ifdef USE_BRIDGE_NORMALS
		curVb->nx = normal.X;
		curVb->ny = normal.Y;
		curVb->nz = normal.Z;
#else
		normal = (normal.X) * vec + normal.Y*vecNormal + normal.Z*vecZ;
		normal.Normalize();	
		TheTerrainRenderObject->doTheLight(&vb, lightRay, &normal, NULL, 1.0f);
		curVb->diffuse = vb.diffuse | 0xFF000000;
#endif
		curVb++;
	}
	return(numVertex);
}

//=============================================================================
// W3DBridge::getModelVerticesFixed
//=============================================================================
/** Gets the vertex values for a section of a fixed bridge.  */
//=============================================================================
Int W3DBridge::getModelVerticesFixed(VertexFormatXYZNDUV1 *destination_vb, Int curVertex, 
																const Matrix3D &mtx, MeshClass *pMesh, RefRenderObjListIterator *pLightsIterator)
{
	if (pMesh == NULL) 
		return(0);

	Vector3 vec = m_end - m_start;
	if (vec.Length2() < 1.0f) {
		vec.Normalize();
	}
	Vector3 vecNormal(-vec.Y, vec.X, 0);
	vecNormal.Normalize();
	Real deltaZ = m_end.Z - m_start.Z;
	deltaZ /= vec.Length();
	Real deltaX = sqrt(1.0 - deltaZ*deltaZ);
	Vector3 vecZ(-deltaZ, 0, deltaX);
	vec /= m_length;
	vecNormal *= m_scale;
	vecZ *= m_scale;
	Real xOffset = -m_leftMinX;
	return(getModelVertices(destination_vb, curVertex, xOffset, vec, vecNormal, vecZ, m_start, mtx, pMesh, pLightsIterator));
}

//=============================================================================
// W3DBridge::getIndicesNVertices
//=============================================================================
/** Gets the index values and vertex values for a bridge.  */
//=============================================================================
void W3DBridge::getIndicesNVertices(UnsignedShort *destination_ib, VertexFormatXYZNDUV1 *destination_vb, 
																		Int *curIndexP, Int *curVertexP, RefRenderObjListIterator *pLightsIterator)
{
	Int numI;
	Int numV;
	m_firstVertex = *curVertexP;
	m_firstIndex = *curIndexP;
	m_numVertex = 0;
	m_numPolygons = 0;
	if (m_sectionMesh == NULL) {
		numV = getModelVerticesFixed(destination_vb, *curVertexP, m_leftMtx, m_leftMesh, pLightsIterator);
		if (!numV)
		{	//not enough room for vertices
			DEBUG_ASSERTCRASH( numV, ("W3DBridge::GetIndicesNVertices(). Vertex overflow.\n") );
			return;
		}
		numI = getModelIndices( destination_ib, *curIndexP, *curVertexP, m_leftMesh);
		if (!numI)
		{	//not enough room for indices
			DEBUG_ASSERTCRASH( numI, ("W3DBridge::GetIndicesNVertices(). Index overflow.\n") );
			return;
		}
		*curIndexP += numI;
		*curVertexP += numV;
		m_numVertex += numV;
		m_numPolygons += numI/3;
		return;
	}

	Vector3 vec = m_end - m_start;
	if (vec.Length2() < 1.0f) {
		vec.Normalize();
	}

	Vector3 vecNormal(-vec.Y, vec.X, 0);
	vecNormal.Normalize();
	vecNormal *= m_scale;

	// Rotate along the y axis to get the appropriate Z height adjustment.
	Real deltaZ = m_end.Z - m_start.Z;
	Real desiredLength = vec.Length();
	deltaZ /= desiredLength;
	Real deltaX = sqrt(1.0 - deltaZ*deltaZ);
	Vector3 vecZ(-deltaZ, 0, deltaX);
	vecZ *= m_scale;

	Real spanLength = m_rightMinX - m_leftMaxX; 
	Int numSpans = 1;
	if (m_bridgeType != FIXED_BRIDGE) {
		Real spannable = desiredLength - (m_length-spanLength);
		numSpans = REAL_TO_INT_FLOOR( (spannable + spanLength/2)/spanLength);
		if (numSpans<0) numSpans = 0;
	}

	Real bridgeLength = m_length + (numSpans-1)*spanLength;
	Real xOffset = -m_leftMinX;
	
	// Draw the left end.
	vec /= bridgeLength;
	numV = getModelVertices(destination_vb, *curVertexP, xOffset, vec, vecNormal, vecZ, m_start, 
		m_leftMtx, m_leftMesh, pLightsIterator);
	if (!numV)
	{	//not enough room for vertices
		DEBUG_ASSERTCRASH( numV, ("W3DBridge::GetIndicesNVertices(). Vertex overflow.\n") );
		return;
	}
	numI = getModelIndices( destination_ib, *curIndexP, *curVertexP, m_leftMesh);
	if (!numI)
	{	//not enough room for indices
		DEBUG_ASSERTCRASH( numI, ("W3DBridge::GetIndicesNVertices(). Index overflow.\n") );
		return;
	}
	*curIndexP += numI;
	*curVertexP += numV;
	m_numVertex += numV;
	m_numPolygons += numI/3;

	Int i;
	// draw the spans.
	for (i=0; i<numSpans; i++) {
		numV = getModelVertices(destination_vb, *curVertexP, xOffset+i*spanLength, vec, vecNormal, vecZ, m_start, 
			m_sectionMtx, m_sectionMesh, pLightsIterator);
		if (!numV)
		{	//not enough room for vertices
			DEBUG_ASSERTCRASH( numV, ("W3DBridge::GetIndicesNVertices(). Vertex overflow.\n") );
			return;
		}
		numI = getModelIndices( destination_ib, *curIndexP, *curVertexP, m_sectionMesh);
		if (!numI)
		{	//not enough room for indices
			DEBUG_ASSERTCRASH( numI, ("W3DBridge::GetIndicesNVertices(). Index overflow.\n") );
			return;
		}
		*curIndexP += numI;
		*curVertexP += numV;
		m_numVertex += numV;
		m_numPolygons += numI/3;
	}
		
	// Draw the right end.
	numV = getModelVertices(destination_vb, *curVertexP, xOffset+(numSpans-1)*spanLength, vec, vecNormal, vecZ, m_start,
		m_rightMtx, m_rightMesh, pLightsIterator);
	if (!numV)
	{	//not enough room for vertices
		DEBUG_ASSERTCRASH( numV, ("W3DBridge::GetIndicesNVertices(). Vertex overflow.\n") );
		return;
	}
	numI = getModelIndices( destination_ib, *curIndexP, *curVertexP, m_rightMesh);
	if (!numI)
	{	//not enough room for indices
		DEBUG_ASSERTCRASH( numI, ("W3DBridge::GetIndicesNVertices(). Index overflow.\n") );
		return;
	}
	*curIndexP += numI;
	*curVertexP += numV;
	m_numVertex += numV;
	m_numPolygons += numI/3;
	return;
}

//=============================================================================
// W3DBridge::getModelIndices
//=============================================================================
/** Gets the index values for a particular mesh section of the bridge.  */
//=============================================================================
Int W3DBridge::getModelIndices(UnsignedShort *destination_ib, Int curIndex, Int vertexOffset, MeshClass *pMesh)
{
	if (pMesh == NULL) 
		return(0);
	Int numPoly = pMesh->Peek_Model()->Get_Polygon_Count();
	const TriIndex *pPoly =pMesh->Peek_Model()->Get_Polygon_Array();
	if (curIndex+3*numPoly+6 >= W3DBridgeBuffer::MAX_BRIDGE_INDEX) {
		return(0);
	}
	UnsignedShort *curIb = destination_ib+curIndex;
	Int i;
	for (i=0; i<numPoly; i++) {
		*curIb++ = vertexOffset + pPoly[i].I;
		*curIb++ = vertexOffset + pPoly[i].J;
		*curIb++ = vertexOffset + pPoly[i].K;
	}
	return(numPoly*3);
}

//-----------------------------------------------------------------------------
//         Private Functions                                               
//-----------------------------------------------------------------------------

///@todo - Sort bridges by texture for better performance.

//=============================================================================
// W3DBridgeBuffer::cull
//=============================================================================
/** Culls the bridges, marking the visible flag.  If a bridge changes visibility, it sets
m_anythingChanged */
//=============================================================================
void W3DBridgeBuffer::cull(CameraClass * camera)
{
	Int curBridge;

	m_anythingChanged = m_updateVis;

	for (curBridge=0; curBridge<m_numBridges; curBridge++) {
		if (m_bridges[curBridge].cullBridge(camera)) {
			m_anythingChanged = true;
		}
	}
}


//=============================================================================
// W3DBridgeBuffer::loadBridgesInVertexAndIndexBuffers
//=============================================================================
/** Loads the bridges into the vertex buffer for drawing. */
//=============================================================================
void W3DBridgeBuffer::loadBridgesInVertexAndIndexBuffers(RefRenderObjListIterator *pLightsIterator)
{
	if (!m_indexBridge || !m_vertexBridge || !m_initialized) {
		return;
	}
	m_curNumBridgeVertices = 0;
	m_curNumBridgeIndices = 0;
	VertexFormatXYZNDUV1 *vb;
	UnsignedShort *ib;
	// Lock the buffers.
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexBridge);
	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexBridge);
	vb=(VertexFormatXYZNDUV1*)lockVtxBuffer.Get_Vertex_Array();
	ib = lockIdxBuffer.Get_Index_Array();

//	UnsignedShort *curIb = ib;

//	VertexFormatXYZNDUV1 *curVb = vb;

	Int curBridge;

	for (curBridge=0; curBridge<m_numBridges; curBridge++) {
		m_bridges[curBridge].getIndicesNVertices(ib, vb, &m_curNumBridgeIndices, 
			&m_curNumBridgeVertices, pLightsIterator);
	}
}

//-----------------------------------------------------------------------------
//         Public Functions                                                
//-----------------------------------------------------------------------------

//=============================================================================
// W3DBridgeBuffer::~W3DBridgeBuffer
//=============================================================================
/** Destructor. Releases w3d assets. */
//=============================================================================
W3DBridgeBuffer::~W3DBridgeBuffer(void)
{
	freeBridgeBuffers();
}

//=============================================================================
// W3DBridgeBuffer::W3DBridgeBuffer
//=============================================================================
/** Constructor. Sets m_initialized to true if it finds the w3d models it needs
for the bridges. */
//=============================================================================
W3DBridgeBuffer::W3DBridgeBuffer(void)
{
	m_initialized = false;
	m_vertexMaterial = NULL;
	m_vertexBridge = NULL;
	m_indexBridge = NULL;
	m_bridgeTexture = NULL;
	m_curNumBridgeVertices=0;
	m_curNumBridgeIndices=0;
	clearAllBridges();
	allocateBridgeBuffers();
	m_initialized = true;
}


//=============================================================================
// W3DBridgeBuffer::freeBridgeBuffers
//=============================================================================
/** Frees the index and vertex buffers. */
//=============================================================================
void W3DBridgeBuffer::freeBridgeBuffers(void)
{
	REF_PTR_RELEASE(m_vertexBridge);
	REF_PTR_RELEASE(m_indexBridge);
	REF_PTR_RELEASE(m_vertexMaterial);
}

//=============================================================================
// W3DBridgeBuffer::allocateBridgeBuffers
//=============================================================================
/** Allocates the index and vertex buffers. */
//=============================================================================
void W3DBridgeBuffer::allocateBridgeBuffers(void)
{
	if (TheGlobalData->m_headless)
		return;
	m_vertexBridge=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZNDUV1,MAX_BRIDGE_VERTEX+4,DX8VertexBufferClass::USAGE_DYNAMIC));
	m_indexBridge=NEW_REF(DX8IndexBufferClass,(MAX_BRIDGE_INDEX+4, DX8IndexBufferClass::USAGE_DYNAMIC));
	m_vertexMaterial=VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);
#ifdef USE_BRIDGE_NORMALS
	m_vertexMaterial= NEW VertexMaterialClass();
	m_vertexMaterial->Set_Shininess(0.0);
	m_vertexMaterial->Set_Ambient(1,1,1);		  
	m_vertexMaterial->Set_Diffuse(1,1,1);
	m_vertexMaterial->Set_Specular(0,0,0);
	m_vertexMaterial->Set_Emissive(0,0,0);
	m_vertexMaterial->Set_Opacity(1);
	m_vertexMaterial->Set_Lighting(true);
	m_vertexMaterial->Set_Diffuse_Color_Source(VertexMaterialClass::COLOR1);
#endif
	m_curNumBridgeVertices=0;
	m_curNumBridgeIndices=0;
}

//=============================================================================
// W3DBridgeBuffer::clearAllBridges
//=============================================================================
/** Removes all bridges. */
//=============================================================================
void W3DBridgeBuffer::clearAllBridges(void)
{
	Int curBridge;
	for (curBridge=0; curBridge<m_numBridges; curBridge++) {
		m_bridges[curBridge].clearBridge();
	}
	m_curNumBridgeIndices = 0;
	m_numBridges=0;
}

//=============================================================================
// W3DBridgeBuffer::loadBridges
//=============================================================================
/** loadBridges.  When loaded, tell the terrain logic where the bridge is. */
//=============================================================================
void W3DBridgeBuffer::loadBridges(W3DTerrainLogic *pTerrainLogic, Bool saveGame)
{
	// TheSuperHackers @logic-client-separation helmutbuhler 26/4/2025
	// W3DBridgeBuffer shouldn't add objects to W3DTerrainLogic
	clearAllBridges();
	MapObject *pMapObj;
	MapObject *pMapObj2;
	for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
		if (pMapObj->getFlag(FLAG_BRIDGE_POINT1)) {
			pMapObj2 = pMapObj->getNext();
			if ( !pMapObj2 || !pMapObj2->getFlag(FLAG_BRIDGE_POINT2)) {
				DEBUG_LOG(("Missing second bridge point.  Ignoring first.\n"));
			}
			if (pMapObj2==NULL) break;
			if (!pMapObj2->getFlag(FLAG_BRIDGE_POINT2)) continue;
			Vector3 from, to;
			from.Set(pMapObj->getLocation()->x, pMapObj->getLocation()->y, 0);
			from.Z = TheTerrainRenderObject->getHeightMapHeight(from.X, from.Y, NULL) + BRIDGE_FLOAT_AMT;
			to.Set(pMapObj2->getLocation()->x, pMapObj2->getLocation()->y, 0);
			to.Z = TheTerrainRenderObject->getHeightMapHeight(to.X, to.Y, NULL) + BRIDGE_FLOAT_AMT;
			addBridge(from, to, pMapObj->getName(), pTerrainLogic, pMapObj->getProperties());
			pMapObj = pMapObj2;
		} 
	}
	if (pTerrainLogic) {
		pTerrainLogic->updateBridgeDamageStates();
	}
}

//=============================================================================
//=============================================================================
static RenderObjClass* createTower( SimpleSceneClass *scene,
																		W3DAssetManager *assetManager,
																		MapObject *mapObject, 
																	  BridgeTowerType type, 
																	  BridgeInfo *bridgeInfo )
{
	RenderObjClass* tower = NULL;

	// sanity
	if( scene == NULL ||
			assetManager == NULL || 
			mapObject == NULL || 
			bridgeInfo == NULL || 
			type < 0 || type >= BRIDGE_MAX_TOWERS )
		return NULL;

	// get template for this bridge
	DEBUG_ASSERTCRASH( TheTerrainRoads, ("createTower: TheTerrainRoads is NULL\n") );
	TerrainRoadType *bridgeTemplate = TheTerrainRoads->findBridge( mapObject->getName() );
	if( bridgeTemplate == NULL )
		return NULL;

	// given the type of tower (corner position) find the appropriate spot to put the tower
	Coord3D towerPos;
	switch( type )
	{

		case BRIDGE_TOWER_FROM_LEFT:	towerPos = bridgeInfo->fromLeft;		break;
		case BRIDGE_TOWER_FROM_RIGHT: towerPos = bridgeInfo->fromRight;		break;
		case BRIDGE_TOWER_TO_LEFT:		towerPos = bridgeInfo->toLeft;			break;
		case BRIDGE_TOWER_TO_RIGHT:		towerPos = bridgeInfo->toRight;			break;
		default: return NULL;

	}  // end switch

	// set the Z position to that of the terrain
	towerPos.z = TheTerrainRenderObject->getHeightMapHeight( towerPos.x, towerPos.y, NULL);

	// find the thing template for the tower we want to construct
	AsciiString towerTemplateName = bridgeTemplate->getTowerObjectName( type );
	DEBUG_ASSERTCRASH( TheThingFactory, ("createTower: TheThingFactory is NULL\n") );
	const ThingTemplate *towerTemplate = TheThingFactory->findTemplate( towerTemplateName );
	if( towerTemplate == NULL )
		return NULL;
		
	// find the name of the render object to show	
	const ModuleInfo& mi = towerTemplate->getDrawModuleInfo( );
	if( mi.getCount() <= 0 )
		return NULL;
	const ModuleData* mdd = mi.getNthData(0);
	const W3DModelDrawModuleData* md = mdd ? mdd->getAsW3DModelDrawModuleData() : NULL;
	if( md == NULL )
		return NULL;
	ModelConditionFlags state;
	state.clear();
	AsciiString modelName = md->getBestModelNameForWB( state );

	// create the render object
	Int playerColor = 0xFFFFFF;
	tower = assetManager->Create_Render_Obj( modelName.str(), 1.0f, playerColor );

	// tie the render object into the map object
	mapObject->setBridgeRenderObject( type, tower );

	// set the position of the tower render object to the position in the world
	Matrix3D transform;
	transform.Make_Identity();
	transform.Set_X_Translation( towerPos.x );
	transform.Set_Y_Translation( towerPos.y );
	transform.Set_Z_Translation( towerPos.z );
	tower->Set_Transform( transform );

	// set the angle for the tower
	/// @todo --> write me

	// add tower render object to the scene
	scene->Add_Render_Object( tower );

	// return the render object of the tower created
	return tower;

}

//=============================================================================
//=============================================================================
static void updateTowerPos( RenderObjClass* tower, 
														BridgeTowerType type, 
														BridgeInfo* bridgeInfo )
{

	// sanity
	if( tower == NULL || type < 0 || type >= BRIDGE_MAX_TOWERS || bridgeInfo == NULL )
		return;

	//
	// compute the angle of the bridge ... we consider the angle of the bridge to be
	// from 'from' to 'to' in the bridge info ... and so does the game
	//
	Coord2D v;
	v.x = bridgeInfo->toLeft.x - bridgeInfo->fromLeft.x;
	v.y = bridgeInfo->toLeft.y - bridgeInfo->fromLeft.y;
	Real angle = v.toAngle();

	//
	// given the type of tower (corner position) find the appropriate spot to put the tower
	// NOTE that we're also adjusting the angle for the from side to point the
	// opposite way the "bridge is pointing"
	//
	Coord3D towerPos;
	switch( type )
	{

		case BRIDGE_TOWER_FROM_LEFT:	towerPos = bridgeInfo->fromLeft;		angle += PI; break;
		case BRIDGE_TOWER_FROM_RIGHT: towerPos = bridgeInfo->fromRight;		angle += PI; break;
		case BRIDGE_TOWER_TO_LEFT:		towerPos = bridgeInfo->toLeft;			break;
		case BRIDGE_TOWER_TO_RIGHT:		towerPos = bridgeInfo->toRight;			break;
		default: return;

	}  // end switch

	// set the position of the tower render object to the position in the world
	Matrix3D transform;
	transform.Make_Identity();
	transform.Set_X_Translation( towerPos.x );
	transform.Set_Y_Translation( towerPos.y );
	transform.Set_Z_Translation( towerPos.z );
	transform.Rotate_Z( angle );
	tower->Set_Transform( transform );

	// set the angle for the tower
//	tower->setAngle( angle );

}

//=============================================================================
// W3DBridgeBuffer::worldBuilderUpdateBridgeTowers
//=============================================================================
/** loadBridges.  When loaded, tell the terrain logic where the bridge is. */
//=============================================================================
void W3DBridgeBuffer::worldBuilderUpdateBridgeTowers( W3DAssetManager *assetManager,
																											SimpleSceneClass *scene )
{
	MapObject *pMapObj;
	MapObject *pMapObj2;

	for( pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext() ) 
	{

		if( pMapObj->getFlag( FLAG_BRIDGE_POINT1 ) ) 
		{

			pMapObj2 = pMapObj->getNext();
			if( !pMapObj2 || !pMapObj2->getFlag( FLAG_BRIDGE_POINT2 ) ) 
				DEBUG_LOG(("Missing second bridge point.  Ignoring first.\n"));

			if( pMapObj2 == NULL ) 
				break;
			if( !pMapObj2->getFlag( FLAG_BRIDGE_POINT2 ) ) 
				continue;

			//
			// now that we've got the two map objects that are bridge point 1 and 2, get the
			// bridge info that has been stored
			//
			for( Int i = 0; i < m_numBridges; ++i )
			{

				//
				// find the bridge with the matching name and position ... note we're just matching
				// (x,y) here cause name and location (without the additional complication of Z) is
				// really all we have to match bridges.
				/// @todo integrate the editor with the game ... will never happen tho ... 
				//
				if( m_bridges[ i ].getTemplateName() == pMapObj->getName() &&
						m_bridges[ i ].getStart()->X == pMapObj->getLocation()->x &&
						m_bridges[ i ].getStart()->Y == pMapObj->getLocation()->y &&
						m_bridges[ i ].getEnd()->X == pMapObj2->getLocation()->x &&
						m_bridges[ i ].getEnd()->Y == pMapObj2->getLocation()->y )
				{
					RenderObjClass *towerRenderObj;

					// get the bridge info
					BridgeInfo bridgeInfo;
					m_bridges[ i ].getBridgeInfo( &bridgeInfo );

					// go through all bridge tower render objects
					Bool created;
					for( Int j = 0; j < BRIDGE_MAX_TOWERS; ++j )
					{

						// create render object if needed
						created = FALSE;
						towerRenderObj = pMapObj->getBridgeRenderObject( (BridgeTowerType)j );
						if( towerRenderObj == NULL )
						{

							towerRenderObj = createTower( scene, assetManager, pMapObj, (BridgeTowerType)j, &bridgeInfo );
							created = TRUE;

						}  // end if

						// sanity
						DEBUG_ASSERTCRASH( towerRenderObj != NULL, ("worldBuilderUpdateBridgeTowers: unable to create tower for bridge '%s'\n",
															 m_bridges[ i ].getTemplateName().str()) );
															  
						// update the position of the towers
						updateTowerPos( towerRenderObj, (BridgeTowerType)j, &bridgeInfo );

						// release the initial ref count of 1 for a newly created tower
						if( created )
							REF_PTR_RELEASE( towerRenderObj );

					}  // end for j

				}  // end if

			}  // end for i

			// skip the 2nd map object representing the second half of the bridgef
			pMapObj = pMapObj2;

		} 

	}

}

//=============================================================================
// W3DBridgeBuffer::addBridge
//=============================================================================
/** Adds a bridge.  Name is the GDF object name. */
//=============================================================================
void W3DBridgeBuffer::addBridge(Vector3 fromLoc, Vector3 toLoc, AsciiString name, W3DTerrainLogic *pTerrainLogic, Dict *props)
{
	if (m_numBridges >= MAX_BRIDGES) {
		return;  
	}

	if (!m_initialized) {
		return;  
	}
	m_bridges[m_numBridges].init(fromLoc, toLoc, name);
	if (m_bridges[m_numBridges].load(BODY_PRISTINE)) {
		W3DBridge *pBridge = m_bridges+m_numBridges;
		if (pTerrainLogic) {
			BridgeInfo info;
			pBridge->getBridgeInfo(&info);
			info.bridgeIndex = m_numBridges;
			pTerrainLogic->addBridgeToLogic(&info, props, name);
		}
		m_numBridges++;
	}
}

//=============================================================================
// W3DBridgeBuffer::updateCenter
//=============================================================================
/** Updates the drawing buffer, based on the camera position. */
//=============================================================================
void W3DBridgeBuffer::updateCenter(CameraClass *camera, RefRenderObjListIterator *pLightsIterator)
{
	cull(camera);
	if (m_anythingChanged || m_curNumBridgeIndices == 0) {
		loadBridgesInVertexAndIndexBuffers(pLightsIterator);
	}
	m_updateVis = false;
}

//=============================================================================
// W3DBridgeBuffer::drawBridges
//=============================================================================
/** Draws the bridges. */
//=============================================================================
void W3DBridgeBuffer::drawBridges(CameraClass * camera, Bool wireframe, TextureClass *cloudTexture)
{

	Int curBridge;
	if (TheTerrainLogic) {
		for (curBridge=0; curBridge<m_numBridges; curBridge++) {
			m_bridges[curBridge].setEnabled(false);
		}
		/* Check for any changed damage states. */
		Bool changed = false;
		for (Bridge *bridge = TheTerrainLogic->getFirstBridge(); bridge; bridge = bridge->getNext()) {
			BridgeInfo info;
			bridge->getBridgeInfo(&info);
			if (info.bridgeIndex<0 || info.bridgeIndex>=m_numBridges) {
				continue;
			}
			m_bridges[info.bridgeIndex].setEnabled(true);
			if (m_bridges[info.bridgeIndex].getDamageState() != info.curDamageState) {
				changed = true;
				enum BodyDamageType curState = m_bridges[info.bridgeIndex].getDamageState();
				m_bridges[info.bridgeIndex].setDamageState(info.curDamageState);
				if (!m_bridges[info.bridgeIndex].load(info.curDamageState)) {
					// put the old model back.
					m_bridges[info.bridgeIndex].load(curState);
					m_bridges[info.bridgeIndex].setDamageState(info.curDamageState);
				}
			}
		}
		if (changed) {
			loadBridgesInVertexAndIndexBuffers(NULL);
		}
	}	else {
		// In wb, all are enabled.
		for (curBridge=0; curBridge<m_numBridges; curBridge++) {
			m_bridges[curBridge].setEnabled(true);
		}
	}



	if (m_curNumBridgeIndices == 0) {
		return;
	}

	DX8Wrapper::Set_Material(m_vertexMaterial);
	// Setup the vertex buffer, shader & texture.
	DX8Wrapper::Set_Index_Buffer(m_indexBridge,0);
	DX8Wrapper::Set_Vertex_Buffer(m_vertexBridge);
	DX8Wrapper::Set_Shader(detailAlphaShader);
#ifdef RTS_DEBUG
	//DX8Wrapper::Set_Shader(detailShader); // shows alpha clipping.
#endif

	DX8Wrapper::Apply_Render_State_Changes();

	if (!wireframe && cloudTexture)
	{	//Force a cloud texture projection into stage 1
		W3DShaderManager::setTexture(1,cloudTexture);
		W3DShaderManager::setShader(W3DShaderManager::ST_CLOUD_TEXTURE,1);
	}

	for (curBridge=0; curBridge<m_numBridges; curBridge++) {
		if (m_bridges[curBridge].isEnabled() && m_bridges[curBridge].isVisible()) {
			m_bridges[curBridge].renderBridge(wireframe);
		}
	}

	if (!wireframe && cloudTexture)
		//Force a cloud texture projection into stage 1
		W3DShaderManager::resetShader(W3DShaderManager::ST_CLOUD_TEXTURE);

	//Render shroud pass over all the bridges
	if (!wireframe && TheTerrainRenderObject->getShroud())
	{
		//Reset to a known shader.
		DX8Wrapper::Invalidate_Cached_Render_States();
		DX8Wrapper::Set_Shader(ShaderClass::_PresetOpaqueShader);
		DX8Wrapper::Set_Material(m_vertexMaterial);
		DX8Wrapper::Set_Index_Buffer(m_indexBridge,0);
		DX8Wrapper::Set_Vertex_Buffer(m_vertexBridge);
		DX8Wrapper::Apply_Render_State_Changes();
		//Apply custom shroud projection shader.
		W3DShaderManager::setTexture(0,TheTerrainRenderObject->getShroud()->getShroudTexture());
		W3DShaderManager::setShader(W3DShaderManager::ST_SHROUD_TEXTURE, 0);
		for (curBridge=0; curBridge<m_numBridges; curBridge++) {
			if (m_bridges[curBridge].isEnabled() && m_bridges[curBridge].isVisible()) {
				//Pretend we're in wireframe so function doesn't reset the shroud texture.
				m_bridges[curBridge].renderBridge(TRUE);
			}
		}
		W3DShaderManager::resetShader(W3DShaderManager::ST_SHROUD_TEXTURE);
	}
}


