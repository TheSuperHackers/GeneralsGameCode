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

#include "StdAfx.h"

#include "DrawObject.h"

#include <stdlib.h>
#include <assetmgr.h>
#include <texture.h>
#include <tri.h>
#include <colmath.h>
#include <coltest.h>
#include <rinfo.h>
#include <camera.h>
#include "Common/GlobalData.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "W3DDevice/GameClient/TerrainTex.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/W3DWater.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/mesh.h"
#include "WW3D2/meshmdl.h"
#include "WW3D2/shader.h"
#include "Common/MapObject.h"
#include "Common/ThingTemplate.h"
#include "GameLogic/PolygonTrigger.h"
#include "GameLogic/SidesList.h"
#include "resource.h"
#include "wbview3d.h"
#include "WorldBuilderDoc.h"
#include "WHeightMapEdit.h"
#include "MeshMoldOptions.h"
#include "WaterTool.h"
#include "BuildListTool.h"
#include "LayersList.h"
#include "Common/WellKnownKeys.h"
#include "Common/BorderColors.h"

#ifdef RTS_DEBUG
#define NO_INTENSE_DEBUG 1
#endif

const Real LINE_THICKNESS = 2.0f;
const Real HANDLE_SIZE = (2.0f) * LINE_THICKNESS;


// Texturing, no zbuffer, disabled zbuffer write, primary gradient, alpha blending
#define SC_OPAQUE ( SHADE_CNST(ShaderClass::PASS_ALWAYS, ShaderClass::DEPTH_WRITE_DISABLE, ShaderClass::COLOR_WRITE_ENABLE, ShaderClass::SRCBLEND_ONE, \
	ShaderClass::DSTBLEND_ZERO, ShaderClass::FOG_DISABLE, ShaderClass::GRADIENT_DISABLE, ShaderClass::SECONDARY_GRADIENT_DISABLE, ShaderClass::TEXTURING_ENABLE, \
	ShaderClass::ALPHATEST_DISABLE, ShaderClass::CULL_MODE_DISABLE, \
	ShaderClass::DETAILCOLOR_DISABLE, ShaderClass::DETAILALPHA_DISABLE) )

// Texturing, no zbuffer, disabled zbuffer write, primary gradient, alpha blending
#define SC_ALPHA ( SHADE_CNST(ShaderClass::PASS_ALWAYS, ShaderClass::DEPTH_WRITE_DISABLE, ShaderClass::COLOR_WRITE_ENABLE, ShaderClass::SRCBLEND_SRC_ALPHA, \
	ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA, ShaderClass::FOG_DISABLE, ShaderClass::GRADIENT_MODULATE, ShaderClass::SECONDARY_GRADIENT_DISABLE, ShaderClass::TEXTURING_ENABLE, \
	ShaderClass::ALPHATEST_DISABLE, ShaderClass::CULL_MODE_ENABLE, \
	ShaderClass::DETAILCOLOR_DISABLE, ShaderClass::DETAILALPHA_DISABLE) )

// Texturing, no zbuffer, disabled zbuffer write, primary gradient, alpha blending
#define SC_ALPHA_Z ( SHADE_CNST(ShaderClass::PASS_LEQUAL, ShaderClass::DEPTH_WRITE_DISABLE, ShaderClass::COLOR_WRITE_ENABLE, ShaderClass::SRCBLEND_SRC_ALPHA, \
	ShaderClass::DSTBLEND_ONE_MINUS_SRC_ALPHA, ShaderClass::FOG_DISABLE, ShaderClass::GRADIENT_MODULATE, ShaderClass::SECONDARY_GRADIENT_DISABLE, ShaderClass::TEXTURING_ENABLE, \
	ShaderClass::ALPHATEST_DISABLE, ShaderClass::CULL_MODE_DISABLE, \
	ShaderClass::DETAILCOLOR_DISABLE, ShaderClass::DETAILALPHA_DISABLE) )

// Texturing, no zbuffer, disabled zbuffer write, primary gradient, alpha blending
#define SC_OPAQUE_Z ( SHADE_CNST(ShaderClass::PASS_LEQUAL, ShaderClass::DEPTH_WRITE_DISABLE, ShaderClass::COLOR_WRITE_ENABLE, ShaderClass::SRCBLEND_ONE, \
	ShaderClass::DSTBLEND_ZERO, ShaderClass::FOG_DISABLE, ShaderClass::GRADIENT_DISABLE, ShaderClass::SECONDARY_GRADIENT_DISABLE, ShaderClass::TEXTURING_ENABLE, \
	ShaderClass::ALPHATEST_DISABLE, ShaderClass::CULL_MODE_DISABLE, \
	ShaderClass::DETAILCOLOR_DISABLE, ShaderClass::DETAILALPHA_DISABLE) )


Bool DrawObject::m_squareFeedback = false;
Int	DrawObject::m_brushWidth = 3;
Int	DrawObject::m_brushFeatherWidth = 3;
Bool	DrawObject::m_toolWantsFeedback = true;
Bool	DrawObject::m_disableFeedback = false;
Bool	DrawObject::m_meshFeedback = false;
Bool	DrawObject::m_rampFeedback = false;
Bool	DrawObject::m_boundaryFeedback = false;
Bool	DrawObject::m_ambientSoundFeedback = false;

Coord3D	DrawObject::m_feedbackPoint;
CPoint DrawObject::m_cellCenter;

Coord3D	DrawObject::m_rampStartPoint;
Coord3D	DrawObject::m_rampEndPoint;
Real DrawObject::m_rampWidth = 0.0f;


Bool DrawObject::m_dragWaypointFeedback = false;
Coord3D DrawObject::m_dragWayStart;
Coord3D DrawObject::m_dragWayEnd;

static Int curHighlight = 0;
static const Int NUM_HIGHLIGHT = 3;




void DrawObject::setWaypointDragFeedback(const Coord3D &start, const Coord3D &end)
{
	m_dragWaypointFeedback = true;
	m_dragWayStart = start;
	m_dragWayEnd = end;
}

void DrawObject::stopWaypointDragFeedback()
{
	m_dragWaypointFeedback = false;
}



DrawObject::~DrawObject(void)
{
	freeMapResources();
}

DrawObject::DrawObject(void) :
	m_drawObjects(true),
	m_drawPolygonAreas(true),
	m_indexBuffer(NULL),
	m_vertexMaterialClass(NULL),
	m_vertexBufferTile1(NULL),
	m_vertexBufferTile2(NULL),
	m_vertexBufferWater(NULL),
	m_vertexFeedback(NULL),
	m_indexFeedback(NULL),
	m_indexWater(NULL),
	m_moldMesh(NULL)

{
	m_feedbackPoint.x = 20;
	m_feedbackPoint.y = 20;
	initData();
	m_waterDrawObject = new WaterRenderObjClass;
	m_waterDrawObject->init(0, 0, 0, NULL, WaterRenderObjClass::WATER_TYPE_0_TRANSLUCENT);
	TheWaterRenderObj=m_waterDrawObject;
}


Bool DrawObject::Cast_Ray(RayCollisionTestClass & raytest)
{

	return false;

}


//@todo: MW Handle both of these properly!!
DrawObject::DrawObject(const DrawObject & src)
{
	*this = src;
}

DrawObject & DrawObject::operator = (const DrawObject & that)
{
	DEBUG_CRASH(("oops"));
	return *this;
}

void DrawObject::Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const
{
	Vector3	ObjSpaceCenter(TheGlobalData->m_waterExtentX,TheGlobalData->m_waterExtentY,50*MAP_XY_FACTOR);
	float length = ObjSpaceCenter.Length();

	sphere.Init(ObjSpaceCenter, length);
}

void DrawObject::Get_Obj_Space_Bounding_Box(AABoxClass & box) const
{
	Vector3	minPt(-2*TheGlobalData->m_waterExtentX,-2*TheGlobalData->m_waterExtentY,0);
	Vector3	maxPt(2*TheGlobalData->m_waterExtentX,2*TheGlobalData->m_waterExtentY,100*MAP_XY_FACTOR);
	box.Init(minPt,maxPt);
}

Int DrawObject::Class_ID(void) const
{
	return RenderObjClass::CLASSID_UNKNOWN;
}

RenderObjClass * DrawObject::Clone(void) const
{
	return new DrawObject(*this);
}


Int DrawObject::freeMapResources(void)
{

	REF_PTR_RELEASE(m_indexBuffer);
	REF_PTR_RELEASE(m_vertexBufferTile1);
	REF_PTR_RELEASE(m_vertexBufferTile2);
	REF_PTR_RELEASE(m_vertexBufferWater);
	REF_PTR_RELEASE(m_vertexMaterialClass);
	REF_PTR_RELEASE(m_vertexFeedback);
	REF_PTR_RELEASE(m_indexFeedback);
	REF_PTR_RELEASE(m_indexWater);
	REF_PTR_RELEASE(m_moldMesh);
	REF_PTR_RELEASE(m_waterDrawObject);
	TheWaterRenderObj = NULL;

	return 0;
}

// Total number of triangles
#define NUM_TRI 26
// Number of triangles in the arrow.
#define NUM_ARROW_TRI 4
// Number of triangles in the selection pyramid.
#define NUM_SELECT_TRI 16
// Height of selection pyramid.
#define SELECT_PYRAMID_HEIGHT (1.0f)


Int DrawObject::initData(void)
{
	Int i;

	freeMapResources();	//free old data and ib/vb

	m_numTriangles = 2*NUM_TRI;
	m_indexBuffer=NEW_REF(DX8IndexBufferClass,(m_numTriangles*3));

	// Fill up the IB
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexBuffer);
	UnsignedShort *ib=lockIdxBuffer.Get_Index_Array();

	for (i=0; i<3*m_numTriangles; i+=3)
	{
		ib[0]=i;
		ib[1]=i+1;
		ib[2]=i+2;

		ib+=3;	//skip the 3 indices we just filled
	}

	m_vertexBufferTile1=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZDUV1,m_numTriangles*3,DX8VertexBufferClass::USAGE_DEFAULT));
	m_vertexBufferTile2=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZDUV1,m_numTriangles*3,DX8VertexBufferClass::USAGE_DEFAULT));

	m_vertexFeedback=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZDUV1,NUM_FEEDBACK_VERTEX,DX8VertexBufferClass::USAGE_DEFAULT));
	m_indexFeedback=NEW_REF(DX8IndexBufferClass,(NUM_FEEDBACK_INDEX));

	//go with a preset material for now.
	m_vertexMaterialClass=VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);

	//use a multi-texture shader: (text1*diffuse)*text2.
	m_shaderClass = ShaderClass(SC_OPAQUE);//_PresetOpaque2DShader;//ShaderClass(SC_OPAQUE); //_PresetOpaqueShader;

	m_shaderClass = ShaderClass::_PresetOpaque2DShader;
	updateForWater();
	updateVB(m_vertexBufferTile1, 255<<8, true, false);

	return 0;
}


/** updateMeshVB puts mesh mold triangles into m_vertexFeedback. */

void DrawObject::updateMeshVB(void)
{
	const Int theAlpha = 64;

	if (m_curMeshModelName != MeshMoldOptions::getModelName()) {
		REF_PTR_RELEASE(m_moldMesh);
		m_curMeshModelName = MeshMoldOptions::getModelName();
	}
	if (m_moldMesh == NULL) {
 		WW3DAssetManager *pMgr = W3DAssetManager::Get_Instance();
		pMgr->Set_WW3D_Load_On_Demand(false);	 // We don't want it fishing for these assets in the game assets.
		m_moldMesh = (MeshClass*)pMgr->Create_Render_Obj(m_curMeshModelName.str());
		if (m_moldMesh == NULL) {
			// Try loading the mold asset.
			AsciiString path("data\\editor\\molds\\");
			path.concat(m_curMeshModelName);
			path.concat(".w3d");
			pMgr->Load_3D_Assets(path.str());
			m_moldMesh = (MeshClass*)pMgr->Create_Render_Obj(m_curMeshModelName.str());
		}
		if (m_moldMesh) {
			m_moldMeshBounds = m_moldMesh->Get_Bounding_Sphere();
		}
		pMgr->Set_WW3D_Load_On_Demand(true);
	}
	if (m_moldMesh == NULL) {
		return;
	}


	m_feedbackVertexCount = 0;
	m_feedbackIndexCount = 0;
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexFeedback);
	UnsignedShort *ib=lockIdxBuffer.Get_Index_Array();
	UnsignedShort *curIb = ib;

	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexFeedback);
	VertexFormatXYZDUV1 *vb = (VertexFormatXYZDUV1*)lockVtxBuffer.Get_Vertex_Array();
	VertexFormatXYZDUV1 *curVb = vb;

	if (m_moldMesh == NULL) {
		return;
	}
	Int i;
	Int numVertex = m_moldMesh->Peek_Model()->Get_Vertex_Count();
	Vector3 *pVert = m_moldMesh->Peek_Model()->Get_Vertex_Array();

//	const Vector3 *pNormal = 	m_moldMesh->Peek_Model()->Get_Vertex_Normal_Array();

	// If we happen to have too many vertex, stop.
	if (numVertex+9>= NUM_FEEDBACK_VERTEX) {
		return;
	}

#if 0	//this wasn't being used (see below) so I commented it out. -MW
	Vector3 lightRay=Normalize(Vector3(-TheGlobalData->m_terrainLightPos[0].x,
		-TheGlobalData->m_terrainLightPos[0].y, -TheGlobalData->m_terrainLightPos[0].z));
#endif

	for (i=0; i<numVertex; i++) {
		curVb->u1 = 0;
		curVb->v1 = 0;
		Vector3 vLoc(pVert[i]);
		vLoc *= MeshMoldOptions::getScale();
		vLoc.Rotate_Z(MeshMoldOptions::getAngle()*PI/180.0f);
		vLoc.X += m_feedbackPoint.x;
		vLoc.Y += m_feedbackPoint.y;
		vLoc.Z += m_feedbackPoint.z;
		curVb->x = vLoc.X;
		curVb->y = vLoc.Y;
		curVb->z = vLoc.Z;

		VertexFormatXYZDUV2 vb;
		vb.x = vLoc.X;
		vb.y = vLoc.Y;
		vb.z = vLoc.Z;

#if 1
		curVb->diffuse = 0x0000ffff | (theAlpha << 24);		// bright cyan.
#else
		TheTerrainRenderObject->doTheLight(&vb, &lightRay, (Vector3 *)(&pNormal[i]), NULL, 1.0f);
		vb.diffuse &= 0x0000ffff;
		curVb->diffuse = vb.diffuse | (theAlpha << 24);
#endif
		curVb++;
		m_feedbackVertexCount++;
	}
	// Put in the "center anchor"

	curVb->u1 = 0;
	curVb->v1 = 0;
	curVb->x = m_feedbackPoint.x;
	curVb->y = m_feedbackPoint.y;
	curVb->z = 0;
	curVb->diffuse = 0xFFFF0000;  // red.
	curVb++;
	m_feedbackVertexCount++;
	curVb->u1 = 0;
	curVb->v1 = 0;
	curVb->x = m_feedbackPoint.x+1;
	curVb->y = m_feedbackPoint.y+1;
	curVb->z = m_feedbackPoint.z;
	curVb->diffuse = 0xFFFF0000;  // red.
	curVb++;
	m_feedbackVertexCount++;
	curVb->u1 = 0;
	curVb->v1 = 0;
	curVb->x = m_feedbackPoint.x;
	curVb->y = m_feedbackPoint.y;
	curVb->z = m_feedbackPoint.z-500;
	curVb->diffuse = 0xFFFF0000;  // red.
	curVb++;
	m_feedbackVertexCount++;
	curVb->u1 = 0;
	curVb->v1 = 0;
	curVb->x = m_feedbackPoint.x+1;
	curVb->y = m_feedbackPoint.y+1;
	curVb->z = m_feedbackPoint.z-500;
	curVb->diffuse = 0xFFFF0000;  // red.
	curVb++;
	m_feedbackVertexCount++;


	Int numPoly = m_moldMesh->Get_Model()->Get_Polygon_Count();
	const Vector3i *pPoly =m_moldMesh->Get_Model()->Get_Polygon_Array();
	if (3*numPoly+9 >= NUM_FEEDBACK_INDEX) {
		return;
	}

	for (i=0; i<numPoly; i++) {
		*curIb++ = pPoly[i].I;
		*curIb++ = pPoly[i].J;
		*curIb++ = pPoly[i].K;
		m_feedbackIndexCount+=3;
	}
	*curIb++ = m_feedbackVertexCount-2;
	*curIb++ = m_feedbackVertexCount-1;
	*curIb++ = m_feedbackVertexCount-3;
	*curIb++ = m_feedbackVertexCount-2;
	*curIb++ = m_feedbackVertexCount-3;
	*curIb++ = m_feedbackVertexCount-4;

	*curIb++ = m_feedbackVertexCount-3;
	*curIb++ = m_feedbackVertexCount-1;
	*curIb++ = m_feedbackVertexCount-2;
	*curIb++ = m_feedbackVertexCount-4;
	*curIb++ = m_feedbackVertexCount-3;
	*curIb++ = m_feedbackVertexCount-2;
	m_feedbackIndexCount+=12;

}

/** updateRampVB puts the ramps into a vertex buffer. */

void DrawObject::updateRampVB(void)
{
	const Int theAlpha = 64;

	m_feedbackVertexCount = 0;
	m_feedbackIndexCount = 0;
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexFeedback);
	UnsignedShort *ib=lockIdxBuffer.Get_Index_Array();
	UnsignedShort *curIb = ib;

	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexFeedback);
	VertexFormatXYZDUV1 *vb = (VertexFormatXYZDUV1*)lockVtxBuffer.Get_Vertex_Array();
	VertexFormatXYZDUV1 *curVb = vb;

	Int i, j;
	Int widthVerts = 8;
	Int lengthVerts = 8;
	Int numVertex = widthVerts * lengthVerts;

/*
	Generate the rectangle via the function BuildRectFromSegmentAndWidth(...).
	Note that for the rectangular case, this is easy, as we simply step along the line at
	pre-determined step sizes, with no additional calculation. (IE, we can simply perform
	linear interpolation.) However, with the curved case, we will need to recalculate the
	value every step along the way.


	Ultimately, what I'd like to do is to precompute what the terrain is actually going to
	do, and then use the faux-adjusted vertices, but this is much easier to start from. jkmcd
*/
	Coord3D coordBL, coordTL, coordBR, coordTR;
	BuildRectFromSegmentAndWidth(&m_rampStartPoint, &m_rampEndPoint, m_rampWidth,
															 &coordBL, &coordTL, &coordBR, &coordTR);

	Vector3 bl(coordBL.x, coordBL.y, coordBL.z);
	Vector3 tl(coordTL.x, coordTL.y, coordTL.z);
	Vector3 br(coordBR.x, coordBR.y, coordBR.z);
	Vector3 tr(coordTR.x, coordTR.y, coordTR.z);

	for (i = 0; i < numVertex; i++) {
		curVb->u1 = INT_TO_REAL(i % widthVerts) / widthVerts;
		curVb->v1 = INT_TO_REAL(i / lengthVerts) / lengthVerts;

		curVb->diffuse = curVb->diffuse = 0x0000ffff | (theAlpha << 24);		// bright cyan.

		Vector3 vLoc;
		vLoc.X = (br.X - bl.X) * INT_TO_REAL(i % widthVerts) / (widthVerts  - 1) +
						 (tl.X - bl.X) * INT_TO_REAL(i / lengthVerts) / (lengthVerts - 1) + bl.X;

		vLoc.Y = (br.Y - bl.Y) * INT_TO_REAL(i % widthVerts) / (widthVerts - 1) +
						 (tl.Y - bl.Y) * INT_TO_REAL(i / lengthVerts) / (lengthVerts - 1) + bl.Y;

		vLoc.Z = (br.Z - bl.Z) * INT_TO_REAL(i % widthVerts) / (widthVerts - 1) +
						 (tl.Z - bl.Z) * INT_TO_REAL(i / lengthVerts) / (lengthVerts - 1) + bl.Z;

		curVb->x = vLoc.X;
		curVb->y = vLoc.Y;
		curVb->z = vLoc.Z;

		curVb++;
		m_feedbackVertexCount++;
	}

	// Now do the indices
	for (i = 0; i < lengthVerts - 1; ++i) {
		for (j = 0; j < widthVerts - 1; ++j) {
			(*curIb++) = i * lengthVerts + j;
			(*curIb++) = (i + 1) * lengthVerts + j;
			(*curIb++) = (i + 1) * lengthVerts + j + 1;

			(*curIb++) = i * lengthVerts + j;
			(*curIb++) = (i + 1) * lengthVerts + j + 1;
			(*curIb++) = (i) * lengthVerts + j + 1;
			m_feedbackIndexCount += 6;
		}

	}
#if 0
	// Put in the "center anchor"

	curVb->u1 = 0;
	curVb->v1 = 0;
	curVb->x = m_feedbackPoint.x;
	curVb->y = m_feedbackPoint.y;
	curVb->z = 0;
	curVb->diffuse = 0xFFFF0000;  // red.
	curVb++;
	m_feedbackVertexCount++;
	curVb->u1 = 0;
	curVb->v1 = 0;
	curVb->x = m_feedbackPoint.x+1;
	curVb->y = m_feedbackPoint.y+1;
	curVb->z = m_feedbackPoint.z;
	curVb->diffuse = 0xFFFF0000;  // red.
	curVb++;
	m_feedbackVertexCount++;
	curVb->u1 = 0;
	curVb->v1 = 0;
	curVb->x = m_feedbackPoint.x;
	curVb->y = m_feedbackPoint.y;
	curVb->z = m_feedbackPoint.z-500;
	curVb->diffuse = 0xFFFF0000;  // red.
	curVb++;
	m_feedbackVertexCount++;
	curVb->u1 = 0;
	curVb->v1 = 0;
	curVb->x = m_feedbackPoint.x+1;
	curVb->y = m_feedbackPoint.y+1;
	curVb->z = m_feedbackPoint.z-500;
	curVb->diffuse = 0xFFFF0000;  // red.
	curVb++;
	m_feedbackVertexCount++;
#endif
}

/** updateBoundaryVB puts boundaries into m_vertexFeedback. */
void DrawObject::updateBoundaryVB(void)
{
//	const Int theAlpha = 64;

	m_feedbackVertexCount = 0;
	m_feedbackIndexCount = 0;
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexFeedback);
	UnsignedShort *ib=lockIdxBuffer.Get_Index_Array();
	UnsignedShort *curIb = ib;

	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexFeedback);
	VertexFormatXYZDUV1 *vb = (VertexFormatXYZDUV1*)lockVtxBuffer.Get_Vertex_Array();
	VertexFormatXYZDUV1 *curVb = vb;

 	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	Int numBoundaries = pDoc->getNumBoundaries();

	Int i, j;
	for (i = 0; i < numBoundaries; ++i) {
		ICoord2D curBoundary;
		pDoc->getBoundary(i, &curBoundary);
		if (curBoundary.x == 0 || curBoundary.y == 0) {
			// do not show feedback, this is a defunct boundary
			continue;
		}

		for (j = 0; j < 4; ++j) {
			Coord3D startPt, endPt;

			if (j == 0) {
				startPt.x = startPt.y = 0;
				startPt.x *= MAP_XY_FACTOR;
				startPt.y *= MAP_XY_FACTOR;
				startPt.z = TheTerrainRenderObject->getHeightMapHeight(startPt.x, startPt.y, NULL);
				endPt.x = 0;
				endPt.y = curBoundary.y;
				endPt.x *= MAP_XY_FACTOR;
				endPt.y *= MAP_XY_FACTOR;
				endPt.z = TheTerrainRenderObject->getHeightMapHeight(endPt.x, endPt.y, NULL);
			} else if (j == 1) {
				startPt = endPt;
				endPt.x = curBoundary.x;
				endPt.y = curBoundary.y;
				endPt.x *= MAP_XY_FACTOR;
				endPt.y *= MAP_XY_FACTOR;
				endPt.z = TheTerrainRenderObject->getHeightMapHeight(endPt.x, endPt.y, NULL);
			} else if (j == 2) {
				startPt = endPt;
				endPt.x = curBoundary.x;
				endPt.y = 0;
				endPt.x *= MAP_XY_FACTOR;
				endPt.y *= MAP_XY_FACTOR;
				endPt.z = TheTerrainRenderObject->getHeightMapHeight(endPt.x, endPt.y, NULL);
			} else if (j == 3) {
				startPt = endPt;
				endPt.x = 0;
				endPt.y = 0;
				endPt.x *= MAP_XY_FACTOR;
				endPt.y *= MAP_XY_FACTOR;
				endPt.z = TheTerrainRenderObject->getHeightMapHeight(endPt.x, endPt.y, NULL);
			}

			if ((m_feedbackVertexCount + 8) > NUM_FEEDBACK_VERTEX) {
				return;
			}

			if ((m_feedbackIndexCount + 12) > NUM_FEEDBACK_INDEX) {
				return;
			}

			Vector3 normal(endPt.x - startPt.x, endPt.y - startPt.y, endPt.z - startPt.z);
			normal.Normalize();
			normal *= LINE_THICKNESS;
			normal.Rotate_Z(PI/2);

			curVb->u1 = 0;
			curVb->v1 = 0;
			curVb->x = startPt.x+normal.X;
			curVb->y = startPt.y+normal.Y;
			curVb->z = startPt.z;
			curVb->diffuse = BORDER_COLORS[i % BORDER_COLORS_SIZE ].m_borderColor;
			curVb++;
			m_feedbackVertexCount++;
			curVb->u1 = 0;
			curVb->v1 = 0;
			curVb->x = startPt.x-normal.X;
			curVb->y = startPt.y-normal.Y;
			curVb->z = startPt.z;
			curVb->diffuse = BORDER_COLORS[i % BORDER_COLORS_SIZE ].m_borderColor;
			curVb++;
			m_feedbackVertexCount++;
			curVb->u1 = 0;
			curVb->v1 = 0;
			curVb->x = endPt.x+normal.X;
			curVb->y = endPt.y+normal.Y;
			curVb->z = endPt.z;
			curVb->diffuse = BORDER_COLORS[i % BORDER_COLORS_SIZE ].m_borderColor;
			curVb++;
			m_feedbackVertexCount++;
			curVb->u1 = 0;
			curVb->v1 = 0;
			curVb->x = endPt.x-normal.X;
			curVb->y = endPt.y-normal.Y;
			curVb->z = endPt.z;
			curVb->diffuse = BORDER_COLORS[i % BORDER_COLORS_SIZE ].m_borderColor;
			curVb++;
			m_feedbackVertexCount++;

			*curIb++ = m_feedbackVertexCount-3;
			*curIb++ = m_feedbackVertexCount-1;
			*curIb++ = m_feedbackVertexCount-2;
			*curIb++ = m_feedbackVertexCount-4;
			*curIb++ = m_feedbackVertexCount-3;
			*curIb++ = m_feedbackVertexCount-2;
			m_feedbackIndexCount+=6;

			// draw a little nugget
			curVb->u1 = 0;
			curVb->v1 = 0;
			curVb->x = startPt.x;
			curVb->y = startPt.y - HANDLE_SIZE;
			curVb->z = startPt.z;
			curVb->diffuse = BORDER_COLORS[i % BORDER_COLORS_SIZE ].m_borderColor;
			curVb++;
			m_feedbackVertexCount++;

			curVb->u1 = 0;
			curVb->v1 = 0;
			curVb->x = startPt.x - HANDLE_SIZE;
			curVb->y = startPt.y;
			curVb->z = startPt.z;
			curVb->diffuse = BORDER_COLORS[i % BORDER_COLORS_SIZE ].m_borderColor;
			curVb++;
			m_feedbackVertexCount++;

			curVb->u1 = 0;
			curVb->v1 = 0;
			curVb->x = startPt.x;
			curVb->y = startPt.y + HANDLE_SIZE;
			curVb->z = startPt.z;
			curVb->diffuse = BORDER_COLORS[i % BORDER_COLORS_SIZE ].m_borderColor;
			curVb++;
			m_feedbackVertexCount++;

			curVb->u1 = 0;
			curVb->v1 = 0;
			curVb->x = startPt.x + HANDLE_SIZE;
			curVb->y = startPt.y;
			curVb->z = startPt.z;
			curVb->diffuse = BORDER_COLORS[i % BORDER_COLORS_SIZE ].m_borderColor;
			curVb++;
			m_feedbackVertexCount++;

			*curIb++ = m_feedbackVertexCount - 4;
			*curIb++ = m_feedbackVertexCount - 2;
			*curIb++ = m_feedbackVertexCount - 3;
			*curIb++ = m_feedbackVertexCount - 4;
			*curIb++ = m_feedbackVertexCount - 1;
			*curIb++ = m_feedbackVertexCount - 2;
			m_feedbackIndexCount+=6;
		}
		// need to push handles in heie.
	}
}

// update the ambient sound Vertex buffers.
// We basically just draw a flag using 12 verts.
//	|\
//	|  \
//	|	 /
//	|/
//	||
//	||
static const Int poleHeight = 20;
static const Int poleWidth = 2;
static const Int flagHeight = 10;
static const Int flagWidth = 10;

void DrawObject::updateAmbientSoundVB(void)
{
	m_feedbackVertexCount = 0;
	m_feedbackIndexCount = 0;
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexFeedback);
	UnsignedShort *ib=lockIdxBuffer.Get_Index_Array();
	UnsignedShort *curIb = ib;

	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexFeedback);
	VertexFormatXYZDUV1 *vb = (VertexFormatXYZDUV1*)lockVtxBuffer.Get_Vertex_Array();
	VertexFormatXYZDUV1 *curVb = vb;

	MapObject* mo = MapObject::getFirstMapObject();

	while (mo) {
		if (!mo->getThingTemplate() || (mo->getThingTemplate()->getEditorSorting() != ES_AUDIO)) {
			mo = mo->getNext();
			continue;
		}

		Coord3D startPt = *mo->getLocation();
		startPt.z = TheTerrainRenderObject->getHeightMapHeight(startPt.x, startPt.y, NULL);

		if (m_feedbackVertexCount + 6 > NUM_FEEDBACK_VERTEX) {
			return;
		}

		if (m_feedbackIndexCount + 12 > NUM_FEEDBACK_INDEX) {
			return;
		}

		curVb->u1 = 0;
		curVb->v1 = 0;
		curVb->x = startPt.x;
		curVb->y = startPt.y;
		curVb->z = startPt.z;
		curVb->diffuse = 0xFF2525EF;
		++curVb;
		++m_feedbackVertexCount;

		curVb->u1 = 0;
		curVb->v1 = 0;
		curVb->x = startPt.x;
		curVb->y = startPt.y;
		curVb->z = startPt.z + poleHeight;
		curVb->diffuse = 0xFF2525EF;
		++curVb;
		++m_feedbackVertexCount;

		curVb->u1 = 0;
		curVb->v1 = 0;
		curVb->x = startPt.x + poleWidth;
		curVb->y = startPt.y;
		curVb->z = startPt.z + poleHeight;
		curVb->diffuse = 0xFF2525EF;
		++curVb;
		++m_feedbackVertexCount;

		curVb->u1 = 0;
		curVb->v1 = 0;
		curVb->x = startPt.x + poleWidth;
		curVb->y = startPt.y;
		curVb->z = startPt.z;
		curVb->diffuse = 0xFF2525EF;
		++curVb;
		++m_feedbackVertexCount;

		curVb->u1 = 0;
		curVb->v1 = 0;
		curVb->x = startPt.x;
		curVb->y = startPt.y;
		curVb->z = startPt.z + poleHeight + flagHeight;
		curVb->diffuse = 0xFF2525EF;
		++curVb;
		++m_feedbackVertexCount;

		curVb->u1 = 0;
		curVb->v1 = 0;
		curVb->x = startPt.x + flagWidth;
		curVb->y = startPt.y;
		curVb->z = startPt.z + poleHeight + (flagHeight / 2);
		curVb->diffuse = 0xFF2525EF;
		++curVb;
		++m_feedbackVertexCount;

		*curIb++ = m_feedbackVertexCount-6;
		*curIb++ = m_feedbackVertexCount-4;
		*curIb++ = m_feedbackVertexCount-5;

		*curIb++ = m_feedbackVertexCount-6;
		*curIb++ = m_feedbackVertexCount-3;
		*curIb++ = m_feedbackVertexCount-4;

		*curIb++ = m_feedbackVertexCount-5;
		*curIb++ = m_feedbackVertexCount-1;
		*curIb++ = m_feedbackVertexCount-2;

		*curIb++ = m_feedbackVertexCount-5;
		*curIb++ = m_feedbackVertexCount-4;
		*curIb++ = m_feedbackVertexCount-1;
		m_feedbackIndexCount += 12;

		mo = mo->getNext();
	}
}


/** updateMeshVB puts waypoint path triangles into m_vertexFeedback. */

void DrawObject::updateWaypointVB(void)
{
//	const Int theAlpha = 64;

	m_feedbackVertexCount = 0;
	m_feedbackIndexCount = 0;
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexFeedback);
	UnsignedShort *ib=lockIdxBuffer.Get_Index_Array();
	UnsignedShort *curIb = ib;

	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexFeedback);
	VertexFormatXYZDUV1 *vb = (VertexFormatXYZDUV1*)lockVtxBuffer.Get_Vertex_Array();
	VertexFormatXYZDUV1 *curVb = vb;

 	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	Int i;
	for (i = 0; i<=pDoc->getNumWaypointLinks(); i++) {
		Bool gotLocation=false;
		Coord3D loc1;
		Coord3D loc2;
 		Bool exists;
		Int waypointID1, waypointID2;

		Int k;
		for (k=0; k<2; k++) {
			Bool ok = false;
			pDoc->getWaypointLink(i, &waypointID1, &waypointID2);
			if (k==0 || i==pDoc->getNumWaypointLinks()) {
				ok = (k==0);
			}	else {
				MapObject *pWay = pDoc->getWaypointByID(waypointID1);
				if (pWay) {
					Bool biDirectional = pWay->getProperties()->getBool(TheKey_waypointPathBiDirectional, &exists);
					if (biDirectional) {
						ok = true;
						pDoc->getWaypointLink(i, &waypointID2, &waypointID1);
					}
				}
			}

			if (i==pDoc->getNumWaypointLinks()) {
				if (m_dragWaypointFeedback) {
					loc1 = m_dragWayStart;
					loc2 = m_dragWayEnd;
					gotLocation = true;
				}
			} else {
				MapObject *pWay1, *pWay2;
				pWay1 = pDoc->getWaypointByID(waypointID1);
				pWay2 = pDoc->getWaypointByID(waypointID2);
				if (pWay1 && pWay2) {
					gotLocation = true;
					loc1 = *pWay1->getLocation();
					loc2 = *pWay2->getLocation();
					AsciiString wayLayer;
					wayLayer = pWay1->getProperties()->getAsciiString(TheKey_objectLayer, &exists);
					if (exists && TheLayersList->isLayerHidden(wayLayer)) {
						gotLocation = false;
					}

					wayLayer = pWay2->getProperties()->getAsciiString(TheKey_objectLayer, &exists);
					if (exists && TheLayersList->isLayerHidden(wayLayer)) {
						gotLocation = false;
					}
				}
			}
			if (gotLocation) {

				Vector3 normal(loc2.x-loc1.x, loc2.y-loc1.y, loc2.z-loc1.z);
				normal.Normalize();
				normal *= 0.5f;
				// Rotate the normal 90 degrees.
				normal.Rotate_Z(PI/2);
				loc1.z = TheTerrainRenderObject->getHeightMapHeight(loc1.x, loc1.y, NULL);
				loc2.z = TheTerrainRenderObject->getHeightMapHeight(loc2.x, loc2.y, NULL);

				if (m_feedbackVertexCount+9>= NUM_FEEDBACK_VERTEX) {
					return;
				}
				curVb->u1 = 0;
				curVb->v1 = 0;
				curVb->x = loc1.x+normal.X;
				curVb->y = loc1.y+normal.Y;
				curVb->z = loc1.z;
				curVb->diffuse = 0xFF000000;  // black.
				curVb++;
				m_feedbackVertexCount++;
				curVb->u1 = 0;
				curVb->v1 = 0;
				curVb->x = loc1.x-normal.X;
				curVb->y = loc1.y-normal.Y;
				curVb->z = loc1.z;
				curVb->diffuse = 0xFF000000;  // black.
				curVb++;
				m_feedbackVertexCount++;
				curVb->u1 = 0;
				curVb->v1 = 0;
				curVb->x = loc2.x+normal.X;
				curVb->y = loc2.y+normal.Y;
				curVb->z = loc2.z;
				curVb->diffuse = 0xFFFF0000;  // red.
				curVb++;
				m_feedbackVertexCount++;
				curVb->u1 = 0;
				curVb->v1 = 0;
				curVb->x = loc2.x-normal.X;
				curVb->y = loc2.y-normal.Y;
				curVb->z = loc2.z;
				curVb->diffuse = 0xFFFF0000;  // red.
				curVb++;
				m_feedbackVertexCount++;

				if (m_feedbackIndexCount+12 >= NUM_FEEDBACK_INDEX) {
					return;
				}
				*curIb++ = m_feedbackVertexCount-3;
				*curIb++ = m_feedbackVertexCount-1;
				*curIb++ = m_feedbackVertexCount-2;
				*curIb++ = m_feedbackVertexCount-4;
				*curIb++ = m_feedbackVertexCount-3;
				*curIb++ = m_feedbackVertexCount-2;
				m_feedbackIndexCount+=6;

				// Do arrowhead.
				Vector3 vec(loc2.x-loc1.x, loc2.y-loc1.y, loc2.z-loc1.z);
				vec.Normalize();
				const Real ARROWHEAD_LEN = 10.0f;
				const Real NORMAL_SHIFT = 6.0f;

				vec *=ARROWHEAD_LEN;
				loc1.x = loc2.x - vec.X;
				loc1.y = loc2.y - vec.Y;
				loc1.z = loc2.z - vec.Z;
				if (m_feedbackVertexCount+9>= NUM_FEEDBACK_VERTEX) {
					return;
				}
				curVb->u1 = 0;
				curVb->v1 = 0;
				curVb->x = loc1.x+NORMAL_SHIFT*normal.X+normal.X;
				curVb->y = loc1.y+NORMAL_SHIFT*normal.Y+normal.Y;
				curVb->z = loc1.z;
				curVb->diffuse = 0xFFFF0000;  // red.
				curVb++;
				m_feedbackVertexCount++;
				curVb->u1 = 0;
				curVb->v1 = 0;
				curVb->x = loc1.x+NORMAL_SHIFT*normal.X;
				curVb->y = loc1.y+NORMAL_SHIFT*normal.Y;
				curVb->z = loc1.z;
				curVb->diffuse = 0xFFFF0000;  // red.
				curVb++;
				m_feedbackVertexCount++;
				curVb->u1 = 0;
				curVb->v1 = 0;
				curVb->x = loc2.x+normal.X;
				curVb->y = loc2.y+normal.Y;
				curVb->z = loc2.z;
				curVb->diffuse = 0xFFFF0000;  // red.
				curVb++;
				m_feedbackVertexCount++;
				curVb->u1 = 0;
				curVb->v1 = 0;
				curVb->x = loc2.x-normal.X;
				curVb->y = loc2.y-normal.Y;
				curVb->z = loc2.z;
				curVb->diffuse = 0xFFFF0000;  // red.
				curVb++;
				m_feedbackVertexCount++;

				if (m_feedbackIndexCount+12 >= NUM_FEEDBACK_INDEX) {
					return;
				}
				*curIb++ = m_feedbackVertexCount-3;
				*curIb++ = m_feedbackVertexCount-1;
				*curIb++ = m_feedbackVertexCount-2;
				*curIb++ = m_feedbackVertexCount-4;
				*curIb++ = m_feedbackVertexCount-3;
				*curIb++ = m_feedbackVertexCount-2;
				m_feedbackIndexCount+=6;

				if (m_feedbackVertexCount+9>= NUM_FEEDBACK_VERTEX) {
					return;
				}
				curVb->u1 = 0;
				curVb->v1 = 0;
				curVb->x = loc1.x-NORMAL_SHIFT*normal.X;
				curVb->y = loc1.y-NORMAL_SHIFT*normal.Y;
				curVb->z = loc1.z;
				curVb->diffuse = 0xFFFF0000;  // red.
				curVb++;
				m_feedbackVertexCount++;
				curVb->u1 = 0;
				curVb->v1 = 0;
				curVb->x = loc1.x-NORMAL_SHIFT*normal.X-normal.X;
				curVb->y = loc1.y-NORMAL_SHIFT*normal.Y-normal.Y;
				curVb->z = loc1.z;
				curVb->diffuse = 0xFFFF0000;  // red.
				curVb++;
				m_feedbackVertexCount++;
				curVb->u1 = 0;
				curVb->v1 = 0;
				curVb->x = loc2.x+normal.X;
				curVb->y = loc2.y+normal.Y;
				curVb->z = loc2.z;
				curVb->diffuse = 0xFFFF0000;  // red.
				curVb++;
				m_feedbackVertexCount++;
				curVb->u1 = 0;
				curVb->v1 = 0;
				curVb->x = loc2.x-normal.X;
				curVb->y = loc2.y-normal.Y;
				curVb->z = loc2.z;
				curVb->diffuse = 0xFFFF0000;  // red.
				curVb++;
				m_feedbackVertexCount++;

				if (m_feedbackIndexCount+12 >= NUM_FEEDBACK_INDEX) {
					return;
				}
				*curIb++ = m_feedbackVertexCount-3;
				*curIb++ = m_feedbackVertexCount-1;
				*curIb++ = m_feedbackVertexCount-2;
				*curIb++ = m_feedbackVertexCount-4;
				*curIb++ = m_feedbackVertexCount-3;
				*curIb++ = m_feedbackVertexCount-2;
				m_feedbackIndexCount+=6;
			}
		}
	}
}

/** updateMeshVB puts polygon trigger triangles into m_vertexFeedback. */

void DrawObject::updatePolygonVB(PolygonTrigger *pTrig, Bool selected, Bool isOpen)
{
//	const Int theAlpha = 64;

	Int green = 0;
	if (selected) {
		green = (255*curHighlight) / (NUM_HIGHLIGHT-1);
	}
	green = green<<8;
	m_feedbackVertexCount = 0;
	m_feedbackIndexCount = 0;
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexFeedback);
	UnsignedShort *ib=lockIdxBuffer.Get_Index_Array();
	UnsignedShort *curIb = ib;

	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexFeedback);
	VertexFormatXYZDUV1 *vb = (VertexFormatXYZDUV1*)lockVtxBuffer.Get_Vertex_Array();
	VertexFormatXYZDUV1 *curVb = vb;

	Int i;
	for (i=0; i<pTrig->getNumPoints(); i++) {
		Coord3D loc1;
		Coord3D loc2;
		ICoord3D iLoc = *pTrig->getPoint(i);
		loc1.x = iLoc.x;
		loc1.y = iLoc.y;
		loc1.z = TheTerrainRenderObject->getHeightMapHeight(loc1.x, loc1.y, NULL);
		if (i<pTrig->getNumPoints()-1) {
			iLoc = *pTrig->getPoint(i+1);
		} else {
			if (isOpen) break;
			iLoc = *pTrig->getPoint(0);
		}
		loc2.x = iLoc.x;
		loc2.y = iLoc.y;
		loc2.z = TheTerrainRenderObject->getHeightMapHeight(loc2.x, loc2.y, NULL);
		Vector3 normal(loc2.x-loc1.x, loc2.y-loc1.y, loc2.z-loc1.z);
		normal.Normalize();
		normal *= 0.5f;
		// Rotate the normal 90 degrees.
		normal.Rotate_Z(PI/2);
		// Put in the "center anchor"

		if (m_feedbackVertexCount+9>= NUM_FEEDBACK_VERTEX) {
			return;
		}
		Int diffuse = 0xFFFF0000+green;
		if (pTrig->isWaterArea()) {
			diffuse = 0xFF0000FF+green;
		}
		curVb->u1 = 0;
		curVb->v1 = 0;
		curVb->x = loc1.x+normal.X;
		curVb->y = loc1.y+normal.Y;
		curVb->z = loc1.z;
		curVb->diffuse = diffuse;
		curVb++;
		m_feedbackVertexCount++;
		curVb->u1 = 0;
		curVb->v1 = 0;
		curVb->x = loc1.x-normal.X;
		curVb->y = loc1.y-normal.Y;
		curVb->z = loc1.z;
		curVb->diffuse = diffuse;
		curVb++;
		m_feedbackVertexCount++;
		curVb->u1 = 0;
		curVb->v1 = 0;
		curVb->x = loc2.x+normal.X;
		curVb->y = loc2.y+normal.Y;
		curVb->z = loc2.z;
		curVb->diffuse = diffuse;
		curVb++;
		m_feedbackVertexCount++;
		curVb->u1 = 0;
		curVb->v1 = 0;
		curVb->x = loc2.x-normal.X;
		curVb->y = loc2.y-normal.Y;
		curVb->z = loc2.z;
		curVb->diffuse = diffuse;
		curVb++;
		m_feedbackVertexCount++;

		if (m_feedbackIndexCount+12 >= NUM_FEEDBACK_INDEX) {
			return;
		}
		*curIb++ = m_feedbackVertexCount-3;
		*curIb++ = m_feedbackVertexCount-1;
		*curIb++ = m_feedbackVertexCount-2;
		*curIb++ = m_feedbackVertexCount-4;
		*curIb++ = m_feedbackVertexCount-3;
		*curIb++ = m_feedbackVertexCount-2;
		m_feedbackIndexCount+=6;

	}
}


/** updateFeedbackVB puts brush feedback triangles into m_vertexFeedback. */

void DrawObject::updateFeedbackVB(void)
{
	const Int theAlpha = 64;
	m_feedbackVertexCount = 0;
	m_feedbackIndexCount = 0;
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexFeedback);
	UnsignedShort *ib=lockIdxBuffer.Get_Index_Array();
	UnsignedShort *curIb = ib;

	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexFeedback);
	VertexFormatXYZDUV1 *vb = (VertexFormatXYZDUV1*)lockVtxBuffer.Get_Vertex_Array();
	VertexFormatXYZDUV1 *curVb = vb;

	Bool doubleResolution = 0;
	Int brushWidth = m_brushWidth;
	Int featherWidth = m_brushFeatherWidth;

	Int shadeR, shadeG, shadeB;
	shadeR = 0;
	shadeG = 125;
	shadeB = 255;
	Int diffuse=shadeB | (shadeG << 8) | (shadeR << 16) | (theAlpha << 24);
	Int featherDiffuse = (shadeG << 8) ;
	Real radius = m_brushWidth/2.0 + m_brushFeatherWidth;

	if (!m_squareFeedback) {
		if (radius < MAX_RADIUS/2) {
			brushWidth = brushWidth*2;
			featherWidth = featherWidth*2;
			doubleResolution = true;
			radius = brushWidth/2.0 + featherWidth;
		}
		radius++;
	}

	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	WorldHeightMapEdit *pMap = pDoc->GetHeightMap();
#define ADJUST_FROM_INDEX_TO_REAL(k) ((k-pMap->getBorderSize())*MAP_XY_FACTOR)

	if (radius > MAX_RADIUS) radius = MAX_RADIUS;
	Real offset = 0;
	if (m_brushWidth&1) offset = 0.5f;
	Int minX = floor(m_cellCenter.x-radius+offset);
	Int minY = floor(m_cellCenter.y-radius+offset);
	Int maxX = minX+2*radius;
	Int maxY = minY+2*radius;
	maxX++; maxY++;
	Int i, j;
//	int sub = m_brushWidth/2;
//	int add = m_brushWidth-sub;
	for (j=minY; j<maxY; j++) {
		for (i=minX; i<maxX; i++) {
			if (m_feedbackVertexCount >= NUM_FEEDBACK_VERTEX) return;
			if (m_squareFeedback) {
				curVb->diffuse = diffuse;
			} else {
				Real blendFactor = Tool::calcRoundBlendFactor(m_cellCenter, i, j, brushWidth, featherWidth);
				if (blendFactor > 0.99) {
					curVb->diffuse = diffuse;
				} else if (blendFactor > 0.05) {
					curVb->diffuse = featherDiffuse | (theAlpha<<24);
				}	else {
					curVb->diffuse = 0;
				}
			}
			Real X, Y, theZ;
			if (doubleResolution) {
				X = ADJUST_FROM_INDEX_TO_REAL(i)/2.0f + ADJUST_FROM_INDEX_TO_REAL(2*offset+m_cellCenter.x)  / 2.0;
				Y = ADJUST_FROM_INDEX_TO_REAL(j)/2.0f + ADJUST_FROM_INDEX_TO_REAL(2*offset+m_cellCenter.y)  / 2.0;
				theZ = TheTerrainRenderObject->getHeightMapHeight(X, Y, NULL);
			} else {
				X = ADJUST_FROM_INDEX_TO_REAL(i);
				Y = ADJUST_FROM_INDEX_TO_REAL(j);
				theZ = TheTerrainRenderObject->getHeightMapHeight(X, Y, NULL);
			}
			curVb->u1 = 0;
			curVb->v1 = 0;
			curVb->x = X;
			curVb->y = Y;
			curVb->z = theZ;
			curVb++;
			m_feedbackVertexCount++;
		}
	}
	Int yOffset = maxX-minX;
	Int halfWidth = yOffset/2;
	for (j=0; j<maxY-minY-1; j++) {
		for (i=0; i<maxX-minX-1; i++) {
			if (m_feedbackIndexCount+6 > NUM_FEEDBACK_INDEX) return;
			{
				Bool flipForBlend = false;
				if (i>=halfWidth && j>=halfWidth) flipForBlend = true;
				if (i<halfWidth && j<halfWidth) flipForBlend = true;
				if (flipForBlend) {
					*curIb++ = j*yOffset + i+1;
 					*curIb++ = j*yOffset + i+yOffset;
					*curIb++ = j*yOffset + i;
 					*curIb++ = j*yOffset + i+1;
 					*curIb++ = j*yOffset + i+1+yOffset;
					*curIb++ = j*yOffset + i+yOffset;
				} else {
					*curIb++ = j*yOffset + i;
					*curIb++ = j*yOffset + i+1+yOffset;
					*curIb++ = j*yOffset + i+yOffset;
					*curIb++ = j*yOffset + i;
					*curIb++ = j*yOffset + i+1;
					*curIb++ = j*yOffset + i+1+yOffset;
				}
			}
			m_feedbackIndexCount+=6;
		}
	}
}


/** Calculate the sign of the cross product.  If the tails of the vectors are both placed
at 0,0, then the cross product can be interpreted as -1 means v2 is to the right of v1,
1 means v2 is to the left of v1, and 0 means v2 is parallel to v1. */

static Int xpSign(const ICoord3D &v1, const ICoord3D &v2) {
	Real xpdct = (Real)v1.x*v2.y - (Real)v1.y*v2.x;
	if (xpdct<0) return -1;
	if (xpdct>0) return 1;
	return 0;
}


/** updateForWater puts a blue rectangle into the vertex buffer. */

void DrawObject::updateForWater(void)
{
}

/* This is a code snippet that starts to attempt to solve the concave area problem,
but doesn't, really.
				const Int maxPoints = 256;
				Bool pointFlags[256];
				Int numPoints = pTrig->getNumPoints();
				ICoord3D pLL1, pLL2, pLL3;
				if (numPoints < 3) continue;
				pLL1 = *pTrig->getPoint(numPoints-1);
				pLL2 = *pTrig->getPoint(0);
				pLL3 = *pTrig->getPoint(1);
				pointFlags[0] = true;
				for (k=1; k<numPoints; k++) {
					pointFlags[k] = true;
					ICoord3D pt = *pTrig->getPoint(k);
					if (pt.y < pLL2.y || (pt.y==pLL2.y && pt.x<pLL2.x) ) {
						pLL2 = pt;
						pLL1 = *pTrig->getPoint(k-1);
						if (k<numPoints-1) {
							pLL3 = *pTrig->getPoint(k+1);
						} else {
							pLL3 = *pTrig->getPoint(0);
						}
					}
				}
				ICoord3D v1, v2;
				v1.x = pLL2.x-pLL1.x;
				v1.y = pLL2.y-pLL1.y;
				v1.z = 0;
				v2.x = pLL3.x-pLL2.x;
				v2.y = pLL3.y-pLL2.y;
				v2.z = 0;
				Int windingXpdct = xpSign(v1, v2);
				if (windingXpdct == 0) windingXpdct = -1;
				Bool didSomething = true;
				while (didSomething) {
					didSomething = false;

					for (k=0; k<pTrig->getNumPoints()-1; k++) {
						if (!pointFlags[k]) continue;
						Int kPlus1;
						for (kPlus1 = k+1; kPlus1 < pTrig->getNumPoints()-1; kPlus1++) {
							if (pointFlags[kPlus1]) break;
						}
						if (kPlus1 >= pTrig->getNumPoints()-1) continue;
						Int kPlus2 = kPlus1+1;
						for (kPlus2 = kPlus1+1; kPlus2 < pTrig->getNumPoints(); kPlus2++) {
							if (pointFlags[kPlus2]) break;
						}

						ICoord3D pt1 = *pTrig->getPoint(k);
						ICoord3D pt2 = *pTrig->getPoint(kPlus1);
						ICoord3D pt3 = *pTrig->getPoint(kPlus2);


*/


/** updateVB puts a circle with an arrow into the vertex buffer. */

Int DrawObject::updateVB(DX8VertexBufferClass	*pVB, Int color, Bool doArrow, Bool doDiamond)
{
	Int i, k;

	Real factor = TheGlobalData->m_terrainAmbient[0].red +
								TheGlobalData->m_terrainAmbient[0].green +
								TheGlobalData->m_terrainAmbient[0].blue;
	if (factor > 1.0f) factor = 1.0f;
	Int r = color&0xFF;
	Int g = (color&0x00FF00)>>8;
	Int b = (color&0xFF0000)>>16;

	r *= factor;
	g *= factor;
	b *= factor;
	const Int theAlpha = 127;
	static const Int highlightColors[NUM_HIGHLIGHT] = { ((255<<8) + (255<<16)) ,
				((255<<16)), (255<<8) };
	Int diffuse =  b + (g<<8) + (r<<16) + (theAlpha<<24);	 // b g<<8 r<<16 a<<24.
	if (pVB )
	{

		DX8VertexBufferClass::WriteLockClass lockVtxBuffer(pVB);
		VertexFormatXYZDUV1 *vb = (VertexFormatXYZDUV1*)lockVtxBuffer.Get_Vertex_Array();

		const Real theZ = 0.0f;
		Real theRadius = THE_RADIUS;
		Real halfLineWidth = 0.03f*MAP_XY_FACTOR;
		if (doDiamond) {
			theRadius *= 5.0;
		}

		Int limit = NUM_TRI-(NUM_ARROW_TRI+NUM_SELECT_TRI);
		float curAngle = 0;
		float deltaAngle = 2*PI/limit;
		if (doDiamond) {
			deltaAngle = PI/2;
		}
		for (i=0; i<limit; i++)
		{
			for (k=0; k<3; k++) {
				vb->z=  theZ;
				if (k==0) {
					vb->x=	0;
					vb->y=	0;

					Vector3 vec(0,0,theZ);
					vec.Rotate_Z(curAngle+(deltaAngle/2));
					vb->x=	vec.X;
					vb->y=	vec.Y;
				} else if (k==1) {
					Vector3 vec(theRadius/10,0,theZ);
					vec.Rotate_Z(curAngle);
					vb->x=	vec.X;
					vb->y=	vec.Y;
				} else if (k==2) {
					Real angle = curAngle+deltaAngle;
					if (i==limit-1) {
						angle = 0;
					}
					Vector3 vec(theRadius/10,0,theZ);
					vec.Rotate_Z(angle);
					vb->x=	vec.X;
					vb->y=	vec.Y;
				}
				vb->diffuse=diffuse;
				vb->u1=0;
				vb->v1=0;
				vb[3*NUM_TRI] = *vb;
				if (k==0) {
					vb[3*NUM_TRI].z += 3.0;
					vb[3*NUM_TRI].diffuse = diffuse;
				}
				vb++;
			}
			curAngle += deltaAngle;

		}
		if (!doArrow) {
			theRadius /= 20;
			halfLineWidth /= 20;
		}
		/* Now do the arrow. */
		for (k=0; k<3; k++) {
			vb->x=	(k&1)?2*theRadius:0.0f;
			vb->y=	-halfLineWidth + ((k&2)?2*halfLineWidth:0);
			vb->z=  theZ;
			vb->diffuse=highlightColors[curHighlight] + (theAlpha<<24);
			vb->u1=0;
			vb->v1=0;
			vb[3*NUM_TRI] = *vb;
			vb++;
		}
		for (k=0; k<3; k++) {
			vb->x=	(k&1)?0.0f:2*theRadius;
			vb->y=	halfLineWidth - ((k&2)?2*halfLineWidth:0);
			vb->z=  theZ;
			vb->diffuse=highlightColors[curHighlight] + (theAlpha<<24);
			vb->u1=0;
			vb->v1=0;
			vb++;
		}
		for (k=0; k<3; k++) {
			if (k==0) { vb->x=theRadius; vb->y = 0;}
			else if (k==1) { vb->x=2*theRadius + 2*halfLineWidth; vb->y = 0;}
			else { vb->x=theRadius; vb->y = 2*halfLineWidth;}
			vb->z=  theZ;
			vb->diffuse=highlightColors[curHighlight] + (theAlpha<<24);
			vb->u1=0;
			vb->v1=0;
			vb[3*NUM_TRI] = *vb;
			vb++;
		}
		for (k=0; k<3; k++) {
			if (k==0) { vb->x=theRadius; vb->y = 0;}
			else if (k==1) { vb->x=theRadius; vb->y = -2*halfLineWidth;}
			else { vb->x=2*theRadius + 2*halfLineWidth; vb->y = 0;}
			vb->z=  theZ;
			vb->diffuse=highlightColors[curHighlight] + (theAlpha<<24);
			vb->u1=0;
			vb->v1=0;
			vb[3*NUM_TRI] = *vb;
			vb++;
		}

		if (!doArrow) {
			theRadius *= 20;
			halfLineWidth *= 20;
		}


		limit = NUM_SELECT_TRI;
		curAngle = 0;
		deltaAngle = 2*PI/limit;
		if (doDiamond) {
			theRadius/=5.0f;
		}
		for (i=0; i<limit; i++)
		{
			for (k=0; k<3; k++) {
				vb->z=  theZ;
				if (k==0) {
					vb->x=	0;
					vb->y=	0;

					Vector3 vec(theRadius*4/5,0,theZ);
					vec.Rotate_Z(curAngle+(deltaAngle/2));
					vb->x=	vec.X;
					vb->y=	vec.Y;
				} else if (k==1) {
					Vector3 vec(theRadius,0,theZ);
					vec.Rotate_Z(curAngle);
					vb->x=	vec.X;
					vb->y=	vec.Y;
				} else if (k==2) {
					Real angle = curAngle+deltaAngle;
					if (i==limit-1) {
						angle = 0;
					}
					Vector3 vec(theRadius,0,theZ);
					vec.Rotate_Z(angle);
					vb->x=	vec.X;
					vb->y=	vec.Y;
				}
				vb->diffuse = highlightColors[curHighlight] + (theAlpha<<24);
				vb->u1=0;
				vb->v1=0;
				vb[3*NUM_TRI] = *vb;
				if (k==0) {
					vb[3*NUM_TRI].z += 3.0;
					vb[3*NUM_TRI].diffuse = highlightColors[curHighlight] + (theAlpha<<24);	 // b g<<8 r<<16 a<<24.
				}
				vb++;
			}
			curAngle += deltaAngle;

		}
#if 0
		// Now do the highlight triangle.  This is in yellow.
		for (k=0; k<3; k++) {
			vb->x = k==0?theRadius:0;
			vb->y = k==1?theRadius:0;
			vb->z=  k==2?theZ+SELECT_PYRAMID_HEIGHT:theZ;
			vb->diffuse= highlightColors[curHighlight] + (theAlpha<<24);	 // b g<<8 r<<16 a<<24.
			vb->u1=0;
			vb->v1=0;
			vb[3*NUM_TRI] = *vb;
			vb++;
		}
		for (k=0; k<3; k++) {
			vb->x = k==1?-theRadius:0;
			vb->y = k==0?theRadius:0;
			vb->z=  k==2?theZ+SELECT_PYRAMID_HEIGHT:theZ;
			vb->diffuse= highlightColors[curHighlight] + (theAlpha<<24);	 // b g<<8 r<<16 a<<24.
			vb->u1=0;
			vb->v1=0;
			vb[3*NUM_TRI] = *vb;
			vb++;
		}

		for (k=0; k<3; k++) {
			vb->x = k==1?theRadius:0;
			vb->y = k==0?-theRadius:0;
			vb->z=  k==2?theZ+SELECT_PYRAMID_HEIGHT:theZ;
			vb->diffuse= highlightColors[curHighlight] + (theAlpha<<24);	 // b g<<8 r<<16 a<<24.
			vb->u1=0;
			vb->v1=0;
			vb[3*NUM_TRI] = *vb;
			vb++;
		}
		for (k=0; k<3; k++) {
			vb->x = k==0?-theRadius:0;
			vb->y = k==1?-theRadius:0;
			vb->z=  k==2?theZ+SELECT_PYRAMID_HEIGHT:theZ;
			vb->diffuse= highlightColors[curHighlight] + (theAlpha<<24);	 // b g<<8 r<<16 a<<24.
			vb->u1=0;
			vb->v1=0;
			vb[3*NUM_TRI] = *vb;
			vb++;
		}
#endif
		return 0; //success.
	}
	return -1;
}

/** Tells drawobject where the tool is located, so it can draw feedback. */
void DrawObject::setFeedbackPos(Coord3D pos)
{
	m_feedbackPoint = pos;
	// center on half pixel for even widths.
	if (!(m_brushWidth&1)) {
		pos.x += MAP_XY_FACTOR/2;
		pos.y += MAP_XY_FACTOR/2;
	}
	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	if (!pDoc) return;
	CPoint ndx;
	pDoc->getCellIndexFromCoord(pos, &ndx);
	if (ndx.x != m_cellCenter.x || ndx.y != m_cellCenter.y) {
		m_cellCenter = ndx;
		if (m_toolWantsFeedback && !m_disableFeedback) {
			WbView3d *pView = pDoc->Get3DView();
			if (pView) {
				pView->Invalidate(false);
			}
		}
	}
}

void DrawObject::setRampFeedbackParms(const Coord3D *start, const Coord3D *end, Real rampWidth)
{
	DEBUG_ASSERTCRASH(start && end, ("Parameter passed into setRampFeedbackParms was NULL. Not allowed"));
	if (!(start && end)) {
		return;
	}

	m_rampStartPoint = *start;
	m_rampEndPoint = *end;
	m_rampWidth = rampWidth;

}


/** Render draws into the current 3d context. */
void DrawObject::Render(RenderInfoClass & rinfo)
{
	DX8Wrapper::Apply_Render_State_Changes();

	DX8Wrapper::Set_Material(m_vertexMaterialClass);
	DX8Wrapper::Set_Shader(m_shaderClass);
	DX8Wrapper::Set_Texture(0, NULL);
	DX8Wrapper::Apply_Render_State_Changes();
	Int count=0;
	Int i;
	curHighlight++;
	if (curHighlight >= NUM_HIGHLIGHT) {
		curHighlight = 0;
	}
	m_waterDrawObject->update();
	if (m_drawObjects || m_drawWaypoints) {
		//Apply the shader and material

		MapObject *pMapObj;
		for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
			// simple Draw test.
			if (pMapObj->getFlags() & FLAG_DONT_RENDER) {
				continue;
			}

			Coord3D loc = *pMapObj->getLocation();
			if (TheTerrainRenderObject) {
				loc.z += TheTerrainRenderObject->getHeightMapHeight(loc.x, loc.y, NULL);
			}
			// Cull.
			SphereClass bounds(Vector3(loc.x, loc.y, loc.z), THE_RADIUS);
			if (rinfo.Camera.Cull_Sphere(bounds)) {
				continue;
			}
			Bool doArrow = true;
			if (pMapObj->getFlag(FLAG_ROAD_FLAGS) || pMapObj->getFlag(FLAG_BRIDGE_FLAGS) || pMapObj->isWaypoint())
			{
				doArrow = false;
			}

			Bool doDiamond = pMapObj->isWaypoint();
			if (doDiamond) {
				if (!m_drawWaypoints) {
					continue;
				}
			}	else {
				if (!m_drawObjects) {
					continue;
				}
				if (BuildListTool::isActive()) {
					continue;
				}
			}

			if (count&1) {
				updateVB(m_vertexBufferTile1, pMapObj->getColor(), doArrow, doDiamond);
				DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferTile1);
			} else {
				updateVB(m_vertexBufferTile2, pMapObj->getColor(), doArrow, doDiamond);
				DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferTile2);
			}
			count++;
			///@todo - remove the istree stuff, or get the info from the thing template.  jba.
			Bool isTree = false;

			Vector3 vec(loc.x, loc.y, loc.z);
			Matrix3D tm(Transform);
			Matrix3x3 rot(true);
			rot.Rotate_Z(pMapObj->getAngle());

			tm.Set_Translation(vec);
			tm.Set_Rotation(rot);
			int polyCount = NUM_TRI;
			if (!pMapObj->isSelected()) {
				polyCount -= NUM_ARROW_TRI+NUM_SELECT_TRI;
			}

			DX8Wrapper::Set_Transform(D3DTS_WORLD,tm);
			DX8Wrapper::Set_Index_Buffer(m_indexBuffer,0);

			if (isTree) {
				DX8Wrapper::Draw_Triangles(	NUM_TRI*3,polyCount, 0,	(m_numTriangles*3));
			} else {
				DX8Wrapper::Draw_Triangles(	0,polyCount, 0,	(m_numTriangles*3));
			}
		}
	}

	DX8Wrapper::Set_Vertex_Buffer(NULL);	//release reference to vertex buffer
	DX8Wrapper::Set_Index_Buffer(NULL,0);	//release reference to vertex buffer

	if (m_drawPolygonAreas) {
		Int selected;
		for (selected = 0; selected < 2; selected++) {
			for (PolygonTrigger *pTrig=PolygonTrigger::getFirstPolygonTrigger(); pTrig; pTrig = pTrig->getNext()) {
				DX8Wrapper::Set_Index_Buffer(m_indexBuffer,0);
				Bool polySelected = PolygonTool::isSelected(pTrig);
				if (polySelected && !selected) continue;
				if (!polySelected && selected) continue;
				for (i=0; i<pTrig->getNumPoints(); i++) {
					Bool pointSelected = (polySelected && PolygonTool::getSelectedPointNdx()==i);
					ICoord3D iLoc = *pTrig->getPoint(i);
					Coord3D loc;
					loc.x = iLoc.x;
					loc.y = iLoc.y;
					loc.z = TheTerrainRenderObject->getHeightMapHeight(loc.x, loc.y, NULL);
					SphereClass bounds(Vector3(loc.x, loc.y, loc.z), THE_RADIUS);
					if (rinfo.Camera.Cull_Sphere(bounds)) {
						continue;
					}
					const Bool ARROW=false;
					const Bool DIAMOND=true;
					const Int RED = 0x0000FF; // red in BGR.
					const Int BLUE = 0xFF7f00; // bright blue.
					Int color = RED;
					if (pTrig->isWaterArea()) {
						color = BLUE;
					}
					if (count&1) {
						updateVB(m_vertexBufferTile1, color, ARROW, DIAMOND);
						DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferTile1);
					} else {
						updateVB(m_vertexBufferTile2, color, ARROW, DIAMOND);
						DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferTile2);
					}
					count++;

					Vector3 vec(loc.x, loc.y, loc.z);
					Matrix3D tm(Transform);
					tm.Set_Translation(vec);

					int polyCount = NUM_TRI;
					if (!pointSelected) {
						polyCount -= NUM_ARROW_TRI+NUM_SELECT_TRI;
					}

					DX8Wrapper::Set_Index_Buffer(m_indexBuffer,0);
					DX8Wrapper::Set_Transform(D3DTS_WORLD,tm);
					DX8Wrapper::Draw_Triangles(	0,polyCount, 0,	(m_numTriangles*3));
				}
				Matrix3D tmReset(Transform);
				DX8Wrapper::Set_Transform(D3DTS_WORLD,tmReset);
				DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferTile1);
				updatePolygonVB(pTrig, polySelected, polySelected && PolygonTool::isSelectedOpen());
 				DX8Wrapper::Set_Vertex_Buffer(m_vertexFeedback);
				if (m_feedbackIndexCount>0) {
					DX8Wrapper::Set_Index_Buffer(m_indexFeedback,0);
					DX8Wrapper::Draw_Triangles(	0, m_feedbackIndexCount/3, 0,	m_feedbackVertexCount);
				}
			}
			DX8Wrapper::Set_Index_Buffer(m_indexBuffer,0);
		}
	}

	DX8Wrapper::Set_Vertex_Buffer(NULL);	//release reference to vertex buffer
	DX8Wrapper::Set_Index_Buffer(NULL,0);	//release reference to vertex buffer

 	if (BuildListTool::isActive()) for (i=0; i<TheSidesList->getNumSides(); i++) {
		SidesInfo *pSide = TheSidesList->getSideInfo(i);
		for (BuildListInfo *pBuild = pSide->getBuildList(); pBuild; pBuild = pBuild->getNext()) {
			Coord3D loc = *pBuild->getLocation();
			if (TheTerrainRenderObject) {
				loc.z += TheTerrainRenderObject->getHeightMapHeight(loc.x, loc.y, NULL);
			}
			// Cull.
			SphereClass bounds(Vector3(loc.x, loc.y, loc.z), THE_RADIUS);
			if (rinfo.Camera.Cull_Sphere(bounds)) {
				continue;
			}
			if (!m_drawObjects) {
				continue;
			}
			const Int GREEN = 0x00FF00; // GREEN in BGR.
			if (count&1) {
				updateVB(m_vertexBufferTile1, GREEN, true, false);
				DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferTile1);
			} else {
				updateVB(m_vertexBufferTile2, GREEN, true, false);
				DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferTile2);
			}
			count++;
// ok to here.
			Vector3 vec(loc.x, loc.y, loc.z);
			Matrix3D tmXX(Transform);
			Matrix3x3 rot(true);
			rot.Rotate_Z(pBuild->getAngle());

			tmXX.Set_Translation(vec);
			tmXX.Set_Rotation(rot);
			int polyCountA = NUM_TRI;
			if (!pBuild->isSelected()) {
				polyCountA -= NUM_ARROW_TRI+NUM_SELECT_TRI;
			}

#if 1
			DX8Wrapper::Set_Index_Buffer(m_indexBuffer,0);
			DX8Wrapper::Set_Transform(D3DTS_WORLD,tmXX);
			DX8Wrapper::Draw_Triangles(	0,polyCountA, 0,	(m_numTriangles*3));
#endif
		}
	}

	DX8Wrapper::Set_Vertex_Buffer(NULL);	//release reference to vertex buffer
	DX8Wrapper::Set_Index_Buffer(NULL,0);	//release reference to vertex buffer

	Matrix3D tmReset(Transform);
	DX8Wrapper::Set_Transform(D3DTS_WORLD,tmReset);

	if (m_drawWaypoints) {
		updateWaypointVB();
		if (m_feedbackIndexCount>0) {
 			DX8Wrapper::Set_Vertex_Buffer(m_vertexFeedback);
			DX8Wrapper::Set_Index_Buffer(m_indexFeedback,0);
			DX8Wrapper::Set_Shader(m_shaderClass);
			DX8Wrapper::Draw_Triangles(	0, m_feedbackIndexCount/3, 0,	m_feedbackVertexCount);
//			DX8Wrapper::Set_Index_Buffer(m_indexBuffer,0);
 //			DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferWater);
		}
	}

	DX8Wrapper::Set_Vertex_Buffer(NULL);	//release reference to vertex buffer
	DX8Wrapper::Set_Index_Buffer(NULL,0);	//release reference to vertex buffer

#if 1
	if (m_meshFeedback) {
		updateMeshVB();
		if (m_feedbackIndexCount>0) {
 			DX8Wrapper::Set_Vertex_Buffer(m_vertexFeedback);
			DX8Wrapper::Set_Index_Buffer(m_indexFeedback,0);
			DX8Wrapper::Set_Shader(SC_OPAQUE_Z);
			DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE,D3DFILL_WIREFRAME);
			DX8Wrapper::Draw_Triangles(	0, m_feedbackIndexCount/3, 0,	m_feedbackVertexCount);
		}
	} else if (m_toolWantsFeedback && !m_disableFeedback) {
		updateFeedbackVB();
		if (m_feedbackIndexCount>0) {
 			DX8Wrapper::Set_Vertex_Buffer(m_vertexFeedback);
			DX8Wrapper::Set_Index_Buffer(m_indexFeedback,0);
			DX8Wrapper::Set_Shader(ShaderClass::_PresetAlpha2DShader);
			DX8Wrapper::Draw_Triangles(	0, m_feedbackIndexCount/3, 0,	m_feedbackVertexCount);
		}
	}
#endif

	DX8Wrapper::Set_Vertex_Buffer(NULL);	//release reference to vertex buffer
	DX8Wrapper::Set_Index_Buffer(NULL,0);	//release reference to vertex buffer

#if 1
	if (m_rampFeedback) {
		updateRampVB();
		if (m_feedbackIndexCount>0) {
 			DX8Wrapper::Set_Vertex_Buffer(m_vertexFeedback);
			DX8Wrapper::Set_Index_Buffer(m_indexFeedback,0);
			DX8Wrapper::Set_Shader(SC_OPAQUE_Z);
			DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE,D3DFILL_WIREFRAME);	// we want a solid ramp
			DX8Wrapper::Set_DX8_Render_State(D3DRS_LIGHTING, FALSE);				// disable lighting
			DX8Wrapper::Draw_Triangles(	0, m_feedbackIndexCount/3, 0,	m_feedbackVertexCount);
		}
	}
#endif

	DX8Wrapper::Set_Vertex_Buffer(NULL);	//release reference to vertex buffer
	DX8Wrapper::Set_Index_Buffer(NULL,0);	//release reference to vertex buffer

#if 1
	if (m_boundaryFeedback) {
		updateBoundaryVB();
		if (m_feedbackIndexCount>0) {
 			DX8Wrapper::Set_Vertex_Buffer(m_vertexFeedback);
			DX8Wrapper::Set_Index_Buffer(m_indexFeedback,0);
			DX8Wrapper::Set_Shader(m_shaderClass);
			DX8Wrapper::Set_DX8_Render_State(D3DRS_CULLMODE, D3DCULL_NONE);
			DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE,D3DFILL_SOLID);	// we want a solid ramp
			DX8Wrapper::Set_DX8_Render_State(D3DRS_LIGHTING, FALSE);				// disable lighting
			DX8Wrapper::Draw_Triangles(	0, m_feedbackIndexCount/3, 0,	m_feedbackVertexCount);
		}
	}
#endif

	DX8Wrapper::Set_Vertex_Buffer(NULL);	//release reference to vertex buffer
	DX8Wrapper::Set_Index_Buffer(NULL,0);	//release reference to vertex buffer


	if (m_ambientSoundFeedback) {
		updateAmbientSoundVB();
		if (m_feedbackIndexCount>0) {
 			DX8Wrapper::Set_Vertex_Buffer(m_vertexFeedback);
			DX8Wrapper::Set_Index_Buffer(m_indexFeedback,0);
			DX8Wrapper::Set_Shader(m_shaderClass);
			DX8Wrapper::Set_DX8_Render_State(D3DRS_CULLMODE, D3DCULL_NONE);
			DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE,D3DFILL_SOLID);	// we want a solid ramp
			DX8Wrapper::Set_DX8_Render_State(D3DRS_LIGHTING, FALSE);				// disable lighting
			DX8Wrapper::Draw_Triangles(	0, m_feedbackIndexCount/3, 0,	m_feedbackVertexCount);
		}
	}

	DX8Wrapper::Set_Index_Buffer(m_indexBuffer,0);
 	DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferWater);

	if (m_waterDrawObject) {
		m_waterDrawObject->renderWater();
	}

}


void BuildRectFromSegmentAndWidth(const Coord3D* start, const Coord3D* end, Real width,
																	Coord3D* outBL, Coord3D* outTL, Coord3D* outBR, Coord3D* outTR)
{
/*
	Here's how we're generating the surface to render:
 		1) Assign longSeg to be the segment from rampStartPoint to rampStopPoint
 		2) Cross product with the segment (0, 0, 1)
 		3) Normalize to get the unit vector (which is in the XY plane.)
 		4) Multiply the unit vector by the ramp width / 2
 		5) Store the four corners of the ramp as startPoint + unit, startPoint - unit,
 			 endPoint + unit and endPoint - unit.
 		6) This gives us a surface that has endpoints which always lie flat along the ground.
*/

	if (!(start && end && outBL && outTL && outBR && outTR)) {
		return;
	}

	// 1)
	Vector3 longSeg;
	if (start->length() > end->length()) {
		longSeg.X = end->x - start->x;
		longSeg.Y = end->y - start->y;
		longSeg.Z = end->z - start->z;
	} else {
		longSeg.X = start->x - end->x;
		longSeg.Y = start->y - end->y;
		longSeg.Z = start->z - end->z;
	}

	// 2)
	Vector3 upSeg(0.0f, 0.0f, 1.0f);
	Vector3 unitVec;

	Vector3::Cross_Product(longSeg, upSeg, &unitVec);

	// 3)
	unitVec.Normalize();

	// 4)
	unitVec.Scale(Vector3(width, width, width));

	Coord3D bl = { start->x + unitVec.X, start->y + unitVec.Y, start->z + unitVec.Z };
	Coord3D tl = { end->x + unitVec.X, end->y + unitVec.Y, end->z + unitVec.Z };
	Coord3D br = { start->x - unitVec.X, start->y - unitVec.Y, start->z - unitVec.Z };
	Coord3D tr = { end->x - unitVec.X, end->y - unitVec.Y, end->z - unitVec.Z };


	// 5)
	(*outBL) = bl;
	(*outTL) = tl;
	(*outBR) = br;
	(*outTR) = tr;

	// 6)
	// all done
}
