import Assets.Math;
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
#include "W3DDevice/GameClient/W3DGraphicsResources.h"
import Graphics.Scene.Bridges.Renderer;
import Graphics.Frame.Runtime;

#include "W3DDevice/GameClient/W3DAssetManager.h"
#include <W3DDevice/GameClient/W3DTextureHandle.h>
#include "Common/GlobalData.h"
#include "Common/RandomValue.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "GameClient/TerrainRoads.h"
#include "GameLogic/Damage.h"
#include "GameLogic/Module/BodyModule.h"
#include "W3DDevice/GameLogic/W3DTerrainLogic.h"
#include "W3DDevice/GameClient/TerrainTex.h"
#include "W3DDevice/GameClient/BaseHeightMap.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "W3DDevice/GameClient/W3DDynamicLight.h"
#include "W3DDevice/GameClient/Module/W3DModelDraw.h"

#include "W3DDevice/GameClient/W3DShroud.h"
#include "W3DDevice/GameClient/W3DCamera.h"
import Graphics.Scene.Surfaces.Geometry;

#include "W3DDevice/GameClient/W3DMeshRenderObject.h"
#include "W3DDevice/GameClient/W3DMeshResource.h"
#include "W3DDevice/GameClient/W3DSceneClass.h"
import Assets.Identity;
#include <string>


//-----------------------------------------------------------------------------
//         Private Data
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//         Private Classes
//-----------------------------------------------------------------------------
//=============================================================================
// W3DBridge constructor.
//=============================================================================
/** Initializes pointers & values.  */
//=============================================================================
W3DBridge::W3DBridge() :
m_bridgeTexture(nullptr),
m_leftMesh(nullptr),
m_sectionMesh(nullptr),
m_rightMesh(nullptr),
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
W3DBridge::~W3DBridge()
{
	clearBridge();
}

//=============================================================================
// W3DBridge::renderBridge
//=============================================================================
/** Renders the bride.  It is assumed that the shared vertex and index buffers
are already set.  */
//=============================================================================
void W3DBridge::releaseGeometry()
{
    Graphics::Get_Surface_Renderer().Destroy_Mesh(m_graphicsMesh);
    m_graphicsMesh = {};
}

bool W3DBridge::uploadGeometry(std::span<const Graphics::SurfaceVertex> vertices,
    std::span<const UnsignedShort> indices)
{
    if (m_firstVertex < 0 || m_numVertex < 0 || m_firstIndex < 0 || m_numPolygons < 0
        || static_cast<std::size_t>(m_firstVertex + m_numVertex) > vertices.size()
        || static_cast<std::size_t>(m_firstIndex + m_numPolygons * 3) > indices.size()) return false;
    const auto output = vertices.subspan(m_firstVertex,m_numVertex);
    std::vector<std::uint32_t> triangles(m_numPolygons * 3);
    for (std::size_t index = 0; index < triangles.size(); ++index)
        triangles[index] = indices[m_firstIndex + index] - m_firstVertex;
    auto &renderer = Graphics::Get_Surface_Renderer();
    if (m_graphicsMesh.Is_Valid()) return renderer.Update_Mesh(m_graphicsMesh, output, triangles);
    m_graphicsMesh = renderer.Create_Mesh(output, triangles);
    return m_graphicsMesh.Is_Valid();
}

//=============================================================================
// W3DBridge::clearBridge
//=============================================================================
/** Frees all bridge objects (meshes & texture).  */
//=============================================================================
void W3DBridge::clearBridge()
{
    releaseGeometry();
    m_firstIndex = m_firstVertex = m_numVertex = m_numPolygons = 0;
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
Bool W3DBridge::cullBridge(W3DCamera * camera)
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
Bool W3DBridge::load(BodyDamageType curDamageState)
{
	REF_PTR_RELEASE(m_bridgeTexture);
	REF_PTR_RELEASE(m_leftMesh);
	REF_PTR_RELEASE(m_sectionMesh);
	REF_PTR_RELEASE(m_rightMesh);

	Real scale, width, length;
	std::string textureFile = "No Texture";
	std::string modelName = "BRIDGESECTIONAL";

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
			textureFile = bridge->getTexture().str();
			modelName = bridge->getBridgeModel().str();
			break;
		case BODY_DAMAGED:
			textureFile = bridge->getTextureDamaged().str();
			modelName = bridge->getBridgeModelNameDamaged().str();
			break;
		case BODY_REALLYDAMAGED:
			textureFile = bridge->getTextureReallyDamaged().str();
			modelName = bridge->getBridgeModelNameReallyDamaged().str();
			break;
		case BODY_RUBBLE:
			textureFile = bridge->getTextureBroken().str();
			modelName = bridge->getBridgeModelNameBroken().str();
			break;
	}

	W3DAssetCatalog *catalog = W3DAssetCatalog::Get_Instance();
	if (catalog == nullptr)
		return false;
	std::string left = modelName + ".BRIDGE_LEFT";
	std::string section = modelName + ".BRIDGE_SPAN";
	std::string right = modelName + ".BRIDGE_RIGHT";

	m_bridgeTexture = catalog->Get_Texture(textureFile.c_str(), MIP_LEVELS_3);
	m_leftMtx.Make_Identity();
	m_rightMtx.Make_Identity();
	m_sectionMtx.Make_Identity();

	W3DRenderObject *pObj = catalog->Create_Render_Obj(modelName.c_str());
	if (!pObj) return false;
	Int i;
	for (i=0; i<pObj->Get_Num_Sub_Objects(); i++) {
		W3DRenderObject *pSub = pObj->Get_Sub_Object(i);
		Matrix3D mtx = pSub->Get_Transform();
		if (Assets::Asset_Name_Prefix_Equals_No_Case(left.c_str(), pSub->Get_Name(), left.size())) {
			m_leftMtx = mtx;
			left = pSub->Get_Name();
		}
		if (Assets::Asset_Name_Prefix_Equals_No_Case(section.c_str(), pSub->Get_Name(), section.size())) {
			m_sectionMtx = mtx;
			section = pSub->Get_Name();
		}
		if (Assets::Asset_Name_Prefix_Equals_No_Case(right.c_str(), pSub->Get_Name(), right.size())) {
			m_rightMtx = mtx;
			right = pSub->Get_Name();
		}
		REF_PTR_RELEASE(pSub);
		//DEBUG_LOG(("Sub obj name %s", pSub->Get_Name()));
	}

	REF_PTR_RELEASE(pObj);

	m_leftMesh = (W3DMeshRenderObject*)catalog->Create_Render_Obj(left.c_str());
	m_sectionMesh = (W3DMeshRenderObject*)catalog->Create_Render_Obj(section.c_str());
	m_rightMesh = (W3DMeshRenderObject*)catalog->Create_Render_Obj(right.c_str());
	m_scale = scale;


	if (m_leftMesh == nullptr) {
		clearBridge();
		return(false);
	}
	m_bridgeType = SECTIONAL_BRIDGE;

	if (m_rightMesh == nullptr || m_sectionMesh == nullptr) {
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
Int W3DBridge::getModelVertices(Graphics::SurfaceVertex *destination_vb, Int curVertex, Real xOffset,
																Vector3 &vec, Vector3 &vecNormal, Vector3 &vecZ, Vector3 &offset,
																const Matrix3D &mtx,
																W3DMeshRenderObject *pMesh, Graphics::SceneObjectList<W3DRenderObject>::Cursor *pLightsIterator)
{
	if (pMesh == nullptr)
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
	Graphics::SurfaceVertex *curVb = destination_vb+curVertex;

	for (i=0; i<numVertex; i++) {
		Vector3 vLoc;
		Vector3 vertex;
		Matrix3D::Transform_Vector(mtx, pVert[i], &vertex);
		vLoc = (vertex.X+xOffset) * vec + vertex.Y*vecNormal + vertex.Z*vecZ;

		vLoc.X += m_start.X;
		vLoc.Y += m_start.Y;
		vLoc.Z += m_start.Z;

		curVb->position[0] = vLoc.X;
		curVb->position[1] = vLoc.Y;
		curVb->position[2] = vLoc.Z;


		Vector3 normal;
		Matrix3D::Rotate_Vector(mtx, pNormal[i], &normal);
		normal = (normal.X) * vec + normal.Y*vecNormal + normal.Z*vecZ;
		normal.Normalize();
		const auto diffuse = TheTerrainRenderObject->computeVertexLighting(vLoc, lightRay, &normal, nullptr, 1);
		curVb->color = Assets::Color_From_ARGB(diffuse | 0xFF000000).To_Array();

		curVb->uv[0] = uvs[i].U;
		curVb->uv[1] = uvs[i].V;
		curVb++;
	}
	return(numVertex);
}

//=============================================================================
// W3DBridge::getModelVerticesFixed
//=============================================================================
/** Gets the vertex values for a section of a fixed bridge.  */
//=============================================================================
Int W3DBridge::getModelVerticesFixed(Graphics::SurfaceVertex *destination_vb, Int curVertex,
																const Matrix3D &mtx, W3DMeshRenderObject *pMesh, Graphics::SceneObjectList<W3DRenderObject>::Cursor *pLightsIterator)
{
	if (pMesh == nullptr)
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
void W3DBridge::getIndicesNVertices(UnsignedShort *destination_ib, Graphics::SurfaceVertex *destination_vb,
																		Int *curIndexP, Int *curVertexP, Graphics::SceneObjectList<W3DRenderObject>::Cursor *pLightsIterator)
{
	Int numI;
	Int numV;
	m_firstVertex = *curVertexP;
	m_firstIndex = *curIndexP;
	m_numVertex = 0;
	m_numPolygons = 0;
	if (m_sectionMesh == nullptr) {
		numV = getModelVerticesFixed(destination_vb, *curVertexP, m_leftMtx, m_leftMesh, pLightsIterator);
		if (!numV)
		{	//not enough room for vertices
			DEBUG_ASSERTCRASH( numV, ("W3DBridge::GetIndicesNVertices(). Vertex overflow.") );
			return;
		}
		numI = getModelIndices( destination_ib, *curIndexP, *curVertexP, m_leftMesh);
		if (!numI)
		{	//not enough room for indices
			DEBUG_ASSERTCRASH( numI, ("W3DBridge::GetIndicesNVertices(). Index overflow.") );
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
		DEBUG_ASSERTCRASH( numV, ("W3DBridge::GetIndicesNVertices(). Vertex overflow.") );
		return;
	}
	numI = getModelIndices( destination_ib, *curIndexP, *curVertexP, m_leftMesh);
	if (!numI)
	{	//not enough room for indices
		DEBUG_ASSERTCRASH( numI, ("W3DBridge::GetIndicesNVertices(). Index overflow.") );
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
			DEBUG_ASSERTCRASH( numV, ("W3DBridge::GetIndicesNVertices(). Vertex overflow.") );
			return;
		}
		numI = getModelIndices( destination_ib, *curIndexP, *curVertexP, m_sectionMesh);
		if (!numI)
		{	//not enough room for indices
			DEBUG_ASSERTCRASH( numI, ("W3DBridge::GetIndicesNVertices(). Index overflow.") );
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
		DEBUG_ASSERTCRASH( numV, ("W3DBridge::GetIndicesNVertices(). Vertex overflow.") );
		return;
	}
	numI = getModelIndices( destination_ib, *curIndexP, *curVertexP, m_rightMesh);
	if (!numI)
	{	//not enough room for indices
		DEBUG_ASSERTCRASH( numI, ("W3DBridge::GetIndicesNVertices(). Index overflow.") );
		return;
	}
	*curIndexP += numI;
	*curVertexP += numV;
	m_numVertex += numV;
	m_numPolygons += numI/3;
}

//=============================================================================
// W3DBridge::getModelIndices
//=============================================================================
/** Gets the index values for a particular mesh section of the bridge.  */
//=============================================================================
Int W3DBridge::getModelIndices(UnsignedShort *destination_ib, Int curIndex, Int vertexOffset, W3DMeshRenderObject *pMesh)
{
	if (pMesh == nullptr)
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
void W3DBridgeBuffer::cull(W3DCamera * camera)
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
void W3DBridgeBuffer::loadBridgesInVertexAndIndexBuffers(Graphics::SceneObjectList<W3DRenderObject>::Cursor *pLightsIterator)
{
    if (!m_initialized || m_vertices.empty() || m_indices.empty()) return;
    m_curNumBridgeVertices = 0;
    m_curNumBridgeIndices = 0;
    auto *vb = m_vertices.data();
    auto *ib = m_indices.data();
	Int curBridge;

	for (curBridge=0; curBridge<m_numBridges; curBridge++) {
		m_bridges[curBridge].getIndicesNVertices(ib, vb, &m_curNumBridgeIndices,
			&m_curNumBridgeVertices, pLightsIterator);
        if (!m_bridges[curBridge].uploadGeometry(m_vertices, m_indices)) {
            m_bridges[curBridge].releaseGeometry();
            DEBUG_LOG(("Bridge geometry upload failed.\n"));
        }
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
W3DBridgeBuffer::~W3DBridgeBuffer()
{
	freeBridgeBuffers();
}

//=============================================================================
// W3DBridgeBuffer::W3DBridgeBuffer
//=============================================================================
/** Constructor. Sets m_initialized to true if it finds the w3d models it needs
for the bridges. */
//=============================================================================
W3DBridgeBuffer::W3DBridgeBuffer()
{
	m_initialized = false;
	m_numBridges = 0;
	m_bridgeTexture = nullptr;
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
void W3DBridgeBuffer::freeBridgeBuffers()
{
    for (int index = 0; index < m_numBridges; ++index) m_bridges[index].releaseGeometry();
    m_vertices.clear();
    m_indices.clear();
    m_curNumBridgeVertices = m_curNumBridgeIndices = 0;
}

//=============================================================================
// W3DBridgeBuffer::allocateBridgeBuffers
//=============================================================================
/** Allocates the index and vertex buffers. */
//=============================================================================
void W3DBridgeBuffer::allocateBridgeBuffers()
{
    if (TheGlobalData->m_headless) return;
    m_vertices.resize(MAX_BRIDGE_VERTEX + 4);
    m_indices.resize(MAX_BRIDGE_INDEX + 4);
    m_curNumBridgeVertices = m_curNumBridgeIndices = 0;
}

//=============================================================================
// W3DBridgeBuffer::clearAllBridges
//=============================================================================
/** Removes all bridges. */
//=============================================================================
void W3DBridgeBuffer::clearAllBridges()
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
				DEBUG_LOG(("Missing second bridge point.  Ignoring first."));
			}
			if (pMapObj2==nullptr) break;
			if (!pMapObj2->getFlag(FLAG_BRIDGE_POINT2)) continue;
			Vector3 from, to;
			from.Set(pMapObj->getLocation()->x, pMapObj->getLocation()->y, 0);
			from.Z = TheTerrainRenderObject->getHeightMapHeight(from.X, from.Y, nullptr) + BRIDGE_FLOAT_AMT;
			to.Set(pMapObj2->getLocation()->x, pMapObj2->getLocation()->y, 0);
			to.Z = TheTerrainRenderObject->getHeightMapHeight(to.X, to.Y, nullptr) + BRIDGE_FLOAT_AMT;
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
static W3DRenderObject* createTower( W3DSimpleScene *scene,
																		W3DAssetManager *assetManager,
																		MapObject *mapObject,
																	  BridgeTowerType type,
																	  BridgeInfo *bridgeInfo )
{
	W3DRenderObject* tower = nullptr;

	// sanity
	if( scene == nullptr ||
			assetManager == nullptr ||
			mapObject == nullptr ||
			bridgeInfo == nullptr ||
			type < 0 || type >= BRIDGE_MAX_TOWERS )
		return nullptr;

	// get template for this bridge
	DEBUG_ASSERTCRASH( TheTerrainRoads, ("createTower: TheTerrainRoads is null") );
	TerrainRoadType *bridgeTemplate = TheTerrainRoads->findBridge( mapObject->getName() );
	if( bridgeTemplate == nullptr )
		return nullptr;

	// given the type of tower (corner position) find the appropriate spot to put the tower
	Coord3D towerPos;
	switch( type )
	{

		case BRIDGE_TOWER_FROM_LEFT:	towerPos = bridgeInfo->fromLeft;		break;
		case BRIDGE_TOWER_FROM_RIGHT: towerPos = bridgeInfo->fromRight;		break;
		case BRIDGE_TOWER_TO_LEFT:		towerPos = bridgeInfo->toLeft;			break;
		case BRIDGE_TOWER_TO_RIGHT:		towerPos = bridgeInfo->toRight;			break;
		default: return nullptr;

	}

	// set the Z position to that of the terrain
	towerPos.z = TheTerrainRenderObject->getHeightMapHeight( towerPos.x, towerPos.y, nullptr);

	// find the thing template for the tower we want to construct
	AsciiString towerTemplateName = bridgeTemplate->getTowerObjectName( type );
	DEBUG_ASSERTCRASH( TheThingFactory, ("createTower: TheThingFactory is null") );
	const ThingTemplate *towerTemplate = TheThingFactory->findTemplate( towerTemplateName );
	if( towerTemplate == nullptr )
		return nullptr;

	// find the name of the render object to show
	const ModuleInfo& mi = towerTemplate->getDrawModuleInfo();
	if( mi.getCount() <= 0 )
		return nullptr;
	const ModuleData* mdd = mi.getNthData(0);
	const W3DModelDrawModuleData* md = mdd ? mdd->getAsW3DModelDrawModuleData() : nullptr;
	if( md == nullptr )
		return nullptr;
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
static void updateTowerPos( W3DRenderObject* tower,
														BridgeTowerType type,
														BridgeInfo* bridgeInfo )
{

	// sanity
	if( tower == nullptr || type < 0 || type >= BRIDGE_MAX_TOWERS || bridgeInfo == nullptr )
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

	}

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
																											W3DSimpleScene *scene )
{
	MapObject *pMapObj;
	MapObject *pMapObj2;

	for( pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext() )
	{

		if( pMapObj->getFlag( FLAG_BRIDGE_POINT1 ) )
		{

			pMapObj2 = pMapObj->getNext();
			if( !pMapObj2 || !pMapObj2->getFlag( FLAG_BRIDGE_POINT2 ) )
				DEBUG_LOG(("Missing second bridge point.  Ignoring first."));

			if( pMapObj2 == nullptr )
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
					W3DRenderObject *towerRenderObj;

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
						if( towerRenderObj == nullptr )
						{

							towerRenderObj = createTower( scene, assetManager, pMapObj, (BridgeTowerType)j, &bridgeInfo );
							created = TRUE;

						}

						// sanity
						DEBUG_ASSERTCRASH( towerRenderObj != nullptr, ("worldBuilderUpdateBridgeTowers: unable to create tower for bridge '%s'",
															 m_bridges[ i ].getTemplateName().str()) );

						// update the position of the towers
						updateTowerPos( towerRenderObj, (BridgeTowerType)j, &bridgeInfo );

						// release the initial ref count of 1 for a newly created tower
						if( created )
							REF_PTR_RELEASE( towerRenderObj );

					}

				}

			}

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
void W3DBridgeBuffer::updateCenter(W3DCamera *camera, Graphics::SceneObjectList<W3DRenderObject>::Cursor *pLightsIterator)
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
void W3DBridgeBuffer::drawBridges(W3DCamera * camera, Bool wireframe, W3DTextureHandle *cloudTexture)
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
				BodyDamageType curState = m_bridges[info.bridgeIndex].getDamageState();
				m_bridges[info.bridgeIndex].setDamageState(info.curDamageState);
				if (!m_bridges[info.bridgeIndex].load(info.curDamageState)) {
					// put the old model back.
					m_bridges[info.bridgeIndex].load(curState);
					m_bridges[info.bridgeIndex].setDamageState(info.curDamageState);
				}
			}
		}
		if (changed) {
			loadBridgesInVertexAndIndexBuffers(nullptr);
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

    auto *device = Graphics::Shared_Frame_Device();
    if (device == nullptr || camera == nullptr) return;
    auto parameters = Make_Surface_Parameters(*camera);
    parameters.textured = wireframe ? 0 : 1;
    Graphics::RHITextureHandle cloud;
    if (!wireframe && cloudTexture != nullptr) {
        const auto *source = static_cast<CloudMapTerrainTextureClass *>(cloudTexture);
        constexpr float stretch = 1.0f / (63.0f * MAP_XY_FACTOR / 2.0f);
        parameters.cloud_projection = {stretch, stretch, source->Get_X_Offset(), source->Get_Y_Offset()};
        parameters.cloud = 1;
        cloud = Resolve_Graphics_Texture(cloudTexture);
    }
    const auto shroud = !wireframe && TheTerrainRenderObject != nullptr
        ? Set_Surface_Shroud(parameters, TheTerrainRenderObject->getShroud()) : Graphics::RHITextureHandle{};
    std::vector<Graphics::BridgeDraw> draws;
    draws.reserve(m_numBridges);
    for (int index = 0; index < m_numBridges; ++index) {
        const auto mesh = m_bridges[index].getGraphicsMesh();
        if (m_bridges[index].isEnabled() && m_bridges[index].isVisible() && mesh.Is_Valid())
            draws.push_back({mesh, Resolve_Graphics_Texture(m_bridges[index].getTexture())});
    }
    if (!Graphics::Draw_Bridges(Graphics::Get_Surface_Renderer(), device->Immediate_Command_List(),
        draws, parameters, cloud, shroud))
        DEBUG_LOG(("Bridge graphics submission failed.\n"));
}
