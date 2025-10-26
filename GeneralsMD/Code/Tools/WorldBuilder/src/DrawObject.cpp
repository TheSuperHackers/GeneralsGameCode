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

#include "StdAfx.h"

#include "DrawObject.h"

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
#include "Common/ThingTemplate.h"
#include "W3DDevice/Common/W3DConvert.h"
#include "render2d.h"
#include "GameLogic/Weapon.h"
#include "Common/AudioEventInfo.h"

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
	REF_PTR_RELEASE(m_waterDrawObject);
	TheWaterRenderObj = NULL;
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
	m_moldMesh(NULL),
	m_lineRenderer(NULL),
  m_drawSoundRanges(false)
{
	m_feedbackPoint.x = 20;
	m_feedbackPoint.y = 20;
	initData();
	m_waterDrawObject = new WaterRenderObjClass;
	m_waterDrawObject->init(0, 0, 0, NULL, WaterRenderObjClass::WATER_TYPE_0_TRANSLUCENT);
	TheWaterRenderObj=m_waterDrawObject;

	//(gth) this was needed to fix the extents bug that is based off water and too small for our maps
	Set_Force_Visible(true);
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
	// (gth) CNC3 these bounds don't actually work for all levels...
	// we set the "force visible" flag for this object since it encapsulates all of the UI
	// gadgets for the whole level anyway.
	Vector3	ObjSpaceCenter(TheGlobalData->m_waterExtentX,TheGlobalData->m_waterExtentY,50*MAP_XY_FACTOR);
	float length = ObjSpaceCenter.Length();
	sphere.Init(ObjSpaceCenter, length);
}

void DrawObject::Get_Obj_Space_Bounding_Box(AABoxClass & box) const
{
	// (gth) CNC3 these bounds don't actually work for all levels...
	// we set the "force visible" flag for this object since it encapsulates all of the UI
	// gadgets for the whole level anyway.
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

	delete m_lineRenderer;
	m_lineRenderer = NULL;

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
	m_indexBuffer=NEW_REF(DX8IndexBufferClass,(m_numTriangles*3, DX8IndexBufferClass::USAGE_DYNAMIC));

	// Fill up the IB
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexBuffer, D3DLOCK_DISCARD);
	UnsignedShort *ib=lockIdxBuffer.Get_Index_Array();

	for (i=0; i<3*m_numTriangles; i+=3)
	{
		ib[0]=i;
		ib[1]=i+1;
		ib[2]=i+2;

		ib+=3;	//skip the 3 indices we just filled
	}

	m_vertexBufferTile1=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZDUV1,m_numTriangles*3,DX8VertexBufferClass::USAGE_DYNAMIC));
	m_vertexBufferTile2=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZDUV1,m_numTriangles*3,DX8VertexBufferClass::USAGE_DYNAMIC));

	m_vertexFeedback=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZDUV1,NUM_FEEDBACK_VERTEX,DX8VertexBufferClass::USAGE_DYNAMIC));
	m_indexFeedback=NEW_REF(DX8IndexBufferClass,(NUM_FEEDBACK_INDEX,DX8IndexBufferClass::USAGE_DYNAMIC));

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
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexFeedback, D3DLOCK_DISCARD);
	UnsignedShort *ib=lockIdxBuffer.Get_Index_Array();
	UnsignedShort *curIb = ib;

	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexFeedback, D3DLOCK_DISCARD);
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
	const TriIndex *pPoly =m_moldMesh->Get_Model()->Get_Polygon_Array();
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
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexFeedback, D3DLOCK_DISCARD);
	UnsignedShort *ib=lockIdxBuffer.Get_Index_Array();
	UnsignedShort *curIb = ib;

	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexFeedback, D3DLOCK_DISCARD);
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
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexFeedback, D3DLOCK_DISCARD);
	UnsignedShort *ib=lockIdxBuffer.Get_Index_Array();
	UnsignedShort *curIb = ib;

	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexFeedback, D3DLOCK_DISCARD);
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

			if (m_feedbackVertexCount + 8 > NUM_FEEDBACK_VERTEX) {
				return;
			}

			if (m_feedbackIndexCount + 12 > NUM_FEEDBACK_INDEX) {
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
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexFeedback, D3DLOCK_DISCARD);
	UnsignedShort *ib=lockIdxBuffer.Get_Index_Array();
	UnsignedShort *curIb = ib;

	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexFeedback, D3DLOCK_DISCARD);
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
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexFeedback, D3DLOCK_DISCARD);
	UnsignedShort *ib=lockIdxBuffer.Get_Index_Array();
	UnsignedShort *curIb = ib;

	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexFeedback, D3DLOCK_DISCARD);
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
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexFeedback, D3DLOCK_DISCARD);
	UnsignedShort *ib=lockIdxBuffer.Get_Index_Array();
	UnsignedShort *curIb = ib;

	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexFeedback, D3DLOCK_DISCARD);
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
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexFeedback, D3DLOCK_DISCARD);
	UnsignedShort *ib=lockIdxBuffer.Get_Index_Array();
	UnsignedShort *curIb = ib;

	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexFeedback, D3DLOCK_DISCARD);
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

		DX8VertexBufferClass::WriteLockClass lockVtxBuffer(pVB, D3DLOCK_DISCARD);
		VertexFormatXYZDUV1 *vb = (VertexFormatXYZDUV1*)lockVtxBuffer.Get_Vertex_Array();

		const Real theZ = 0.0f;
		Real theRadius = THE_RADIUS;
		Real halfLineWidth = 0.03f*MAP_XY_FACTOR;
		if (doDiamond) {
			theRadius *= 5.0;
		}
		else
		{
			theRadius *= 2.0;
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

		if (!doDiamond) {
			theRadius /= 2.0;
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

#define BOUNDING_BOX_LINE_WIDTH 2.0f
/** Draw an object's bounding box into the vertex buffer. **/
// MLL C&C3
void DrawObject::updateVBWithBoundingBox(MapObject *pMapObj, CameraClass* camera)
{
	if (!pMapObj || !m_lineRenderer || !pMapObj->getThingTemplate()) {
		return;
	}

	unsigned long color = 0xFFAA00AA; // Purple

	GeometryInfo ginfo = pMapObj->getThingTemplate()->getTemplateGeometryInfo();

	Coord3D pos = *pMapObj->getLocation();
	if (TheTerrainRenderObject) {
		// Make sure that the position is on the terrain.
		pos.z += TheTerrainRenderObject->getHeightMapHeight(pos.x, pos.y, NULL);
	}

	switch (ginfo.getGeomType())
	{
		//---------------------------------------------------------------------------------------------
		case GEOMETRY_BOX:
		{
			Real angle = pMapObj->getAngle();
			Real c = (Real)cos(angle);
			Real s = (Real)sin(angle);
			Real exc = ginfo.getMajorRadius()*c;
			Real eyc = ginfo.getMinorRadius()*c;
			Real exs = ginfo.getMajorRadius()*s;
			Real eys = ginfo.getMinorRadius()*s;
			Coord3D pts[4];
			pts[0].x = pos.x - exc - eys;
			pts[0].y = pos.y + eyc - exs;
			pts[0].z = 0;
			pts[1].x = pos.x + exc - eys;
			pts[1].y = pos.y + eyc + exs;
			pts[1].z = 0;
			pts[2].x = pos.x + exc + eys;
			pts[2].y = pos.y - eyc + exs;
			pts[2].z = 0;
			pts[3].x = pos.x - exc + eys;
			pts[3].y = pos.y - eyc - exs;
			pts[3].z = 0;
			Real z = pos.z;
			for (int i = 0; i < 2; i++) {
				for (int corner = 0; corner < 4; corner++) {
					ICoord2D start, end;
					pts[corner].z = z;
					pts[(corner+1)&3].z = z;
					bool shouldStart = worldToScreen(&pts[corner], &start, camera);
					bool shouldEnd = worldToScreen(&pts[(corner+1)&3], &end, camera);
					if (shouldStart && shouldEnd) {
						m_lineRenderer->Add_Line(Vector2(start.x, start.y), Vector2(end.x, end.y), BOUNDING_BOX_LINE_WIDTH, color);
					}
				}

				z += ginfo.getMaxHeightAbovePosition();
			}
			break;
		}

		//---------------------------------------------------------------------------------------------
		case GEOMETRY_SPHERE:	// not quite right, but close enough
		case GEOMETRY_CYLINDER:
		{
			Real angle, inc = PI/4.0f;
			Real radius = ginfo.getMajorRadius();
			Coord3D pnt, lastPnt;
			ICoord2D start, end;
			Real z = pos.z;

			bool shouldEnd, shouldStart;
			// Draw the cylinder.
			for (int i=0; i<2; i++) {
				angle = 0.0f;
				lastPnt.x = pos.x + radius * (Real)cos(angle);
				lastPnt.y = pos.y + radius * (Real)sin(angle);
				lastPnt.z = z;
				shouldEnd = worldToScreen(&lastPnt, &end, camera);

				for( angle = inc; angle <= 2.0f * PI; angle += inc ) {
					pnt.x = pos.x + radius * (Real)cos(angle);
					pnt.y = pos.y + radius * (Real)sin(angle);
					pnt.z = z;
					shouldStart = worldToScreen(&pnt, &start, camera);
					if (shouldStart && shouldEnd) {
						m_lineRenderer->Add_Line(Vector2(start.x, start.y), Vector2(end.x, end.y), BOUNDING_BOX_LINE_WIDTH, color);
					}
					lastPnt = pnt;
					end = start;
					shouldEnd = shouldStart;
				}

				// Next time around, draw the top of the cylinder.
				z += ginfo.getMaxHeightAbovePosition();
			}

			// Draw centerline
			pnt.x = pos.x;
			pnt.y = pos.y;
			pnt.z = pos.z;
			shouldStart = worldToScreen( &pnt, &start, camera);
			pnt.z = pos.z + ginfo.getMaxHeightAbovePosition();
			shouldEnd = worldToScreen( &pnt, &end, camera);
			if (shouldStart && shouldEnd) {
				m_lineRenderer->Add_Line(Vector2(start.x, start.y), Vector2(end.x, end.y), BOUNDING_BOX_LINE_WIDTH, color);
			}
			break;
		}
	}
}

/** Draw a "circle" into the m_lineRenderer, e.g. to visualize weapon range, sight range, sound range **/
void DrawObject::addCircleToLineRenderer( const Coord3D & center, Real radius, Real width, unsigned long color, CameraClass* camera )
{
  Real angle, inc = PI/4.0f;
  Coord3D pnt, lastPnt;
  ICoord2D start, end;
  Real z = center.z;

  // Draw the circle.
  angle = 0.0f;
  lastPnt.x = center.x + radius * (Real)cos(angle);
  lastPnt.y = center.y + radius * (Real)sin(angle);
  lastPnt.z = z;
  bool shouldEnd = worldToScreen(&lastPnt, &end, camera);

  for( angle = inc; angle <= 2.0f * PI; angle += inc ) {
    pnt.x = center.x + radius * (Real)cos(angle);
    pnt.y = center.y + radius * (Real)sin(angle);
    pnt.z = z;

    bool shouldStart = worldToScreen(&pnt, &start, camera);
    if (shouldStart && shouldEnd) {
      m_lineRenderer->Add_Line(Vector2(start.x, start.y), Vector2(end.x, end.y), width, color);
    }

    lastPnt = pnt;
    end = start;
    shouldEnd = shouldStart;
  }

}

#define SIGHT_RANGE_LINE_WIDTH 2.0f
/** Draw an object's sight range into the vertex buffer. **/
// MLL C&C3
void DrawObject::updateVBWithSightRange(MapObject *pMapObj, CameraClass* camera)
{
	if (!pMapObj || !m_lineRenderer || !pMapObj->getThingTemplate()) {
		return;
	}

	const unsigned long color = 0xFFF0F0F0; // Light blue.

	Real radius = pMapObj->getThingTemplate()->friend_calcVisionRange();

	Coord3D pos = *pMapObj->getLocation();
	if (TheTerrainRenderObject) {
		// Make sure that the position is on the terrain.
		pos.z += TheTerrainRenderObject->getHeightMapHeight(pos.x, pos.y, NULL);
	}

  addCircleToLineRenderer(pos, radius, SIGHT_RANGE_LINE_WIDTH, color, camera );
}

#define WEAPON_RANGE_LINE_WIDTH 1.0f
/** Draw an object's weapon range into the vertex buffer. **/
// MLL C&C3
void DrawObject::updateVBWithWeaponRange(MapObject *pMapObj, CameraClass* camera)
{
	if (!pMapObj || !m_lineRenderer || !pMapObj->getThingTemplate()) {
		return;
	}

  const unsigned long colors[WEAPONSLOT_COUNT] = {0xFF00FF00, 0xFFE0F00A, 0xFFFF0000}; // Green, Yellow, Red


	Coord3D pos = *pMapObj->getLocation();
	if (TheTerrainRenderObject) {
		// Make sure that the position is on the terrain.
		pos.z += TheTerrainRenderObject->getHeightMapHeight(pos.x, pos.y, NULL);
	}

	const WeaponTemplateSetVector& weapons = pMapObj->getThingTemplate()->getWeaponTemplateSets();

	for (WeaponTemplateSetVector::const_iterator it = weapons.begin(); it != weapons.end(); ++it)	{
		if (it->hasAnyWeapons() == false) {
			continue;
		}

		for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
			const WeaponTemplate* tmpl = it->getNth((WeaponSlotType)i);

			if (tmpl == NULL) {
				continue;
			}

			Real radius = tmpl->getUnmodifiedAttackRange();

      addCircleToLineRenderer(pos, radius, WEAPON_RANGE_LINE_WIDTH, colors[i], camera );
		}
	}
}

#define SOUND_RANGE_LINE_WIDTH 1.0f
/** Draw an object's min & max sound ranges into the vertex buffer. **/
// MLL C&C3
void DrawObject::updateVBWithSoundRanges(MapObject *pMapObj, CameraClass* camera)
{
  if (!pMapObj || !m_lineRenderer) {
    return;
  }

  const unsigned long colors[2] = {0xFF0000FF, 0xFFFF00FF}; // Blue and purple
                                                            // Colors match those used in W3DView.cpp


  Coord3D pos = *pMapObj->getLocation();
  if (TheTerrainRenderObject) {
    // Make sure that the position is on the terrain.
    pos.z += TheTerrainRenderObject->getHeightMapHeight(pos.x, pos.y, NULL);
  }

  // Does this object actually have an attached sound?
  const AudioEventInfo * audioInfo = NULL;

  Dict * properties = pMapObj->getProperties();

  Bool exists = false;
  AsciiString ambientName = properties->getAsciiString( TheKey_objectSoundAmbient, &exists );

  if ( exists )
  {
    if ( ambientName.isEmpty() )
    {
      // User has removed normal sound
      return;
    }
    else
    {
      if ( TheAudio == NULL )
      {
        DEBUG_CRASH( ("TheAudio is NULL! Can't draw sound circles") );
        return;
      }

      audioInfo = TheAudio->findAudioEventInfo( ambientName );

      if ( audioInfo == NULL )
      {
        DEBUG_CRASH( ("Override audio named %s is missing; Can't draw sound circles", ambientName.str() ) );
        return;
      }
    }
  }
  else
  {
    const ThingTemplate * thingTemplate = pMapObj->getThingTemplate();
    if ( thingTemplate == NULL )
    {
      // No sound if no template
      return;
    }

    if ( !thingTemplate->hasSoundAmbient() )
    {
      return;
    }

    const AudioEventRTS * event = thingTemplate->getSoundAmbient();

    if ( event == NULL )
    {
      return;
    }

    audioInfo = event->getAudioEventInfo();

    if ( audioInfo == NULL )
    {
      // May just not be set up yet
      if ( TheAudio == NULL )
      {
        DEBUG_CRASH( ("TheAudio is NULL! Can't draw sound circles") );
        return;
      }

      audioInfo = TheAudio->findAudioEventInfo( event->getEventName() );

      if ( audioInfo == NULL )
      {
        DEBUG_CRASH( ("Default ambient sound %s has no info; Can't draw sound circles", event->getEventName().str() ) );
        return;
      }
    }
  }

  // Should have set up audioInfo or returned by now
  DEBUG_ASSERTCRASH( audioInfo != NULL, ("Managed to finish setting up audio info without setting it?!?" ) );
  if ( audioInfo == NULL )
  {
    return;
  }

  // Get the current radius (could be overridden)
  Real minRadius = audioInfo->m_minDistance;
  Real maxRadius = audioInfo->m_maxDistance;
  Bool customized = properties->getBool( TheKey_objectSoundAmbientCustomized, &exists );
  if ( exists && customized )
  {
    Real valReal;

    valReal = properties->getReal( TheKey_objectSoundAmbientMinRange, &exists );
    if ( exists )
    {
      minRadius = valReal;
    }
    valReal = properties->getReal( TheKey_objectSoundAmbientMaxRange, &exists );
    if ( exists )
    {
      maxRadius = valReal;
    }
  }
  addCircleToLineRenderer(pos, minRadius, SOUND_RANGE_LINE_WIDTH, colors[0], camera );
  addCircleToLineRenderer(pos, maxRadius, SOUND_RANGE_LINE_WIDTH, colors[1], camera );
}


#define TEST_ART_HIGHLIGHT_LINE_WIDTH 5.0f
/** Draw test art with an X on it. **/
// MLL C&C3
void DrawObject::updateVBWithTestArtHighlight(MapObject *pMapObj, CameraClass* camera)
{
	if (!pMapObj || !m_lineRenderer || pMapObj->getThingTemplate() || pMapObj->isScorch()) {
		// It is test art if it doesn't have a ThingTemplate.
		return;
	}

	unsigned long color = 0xFFA000A0; // Purple


	Coord3D pos = *pMapObj->getLocation();
	if (TheTerrainRenderObject) {
		// Make sure that the position is on the terrain.
		pos.z += TheTerrainRenderObject->getHeightMapHeight(pos.x, pos.y, NULL);
	}

	Real angle, inc = PI/2.0f;
	Coord3D pnt, lastPnt;
	ICoord2D start, end;
	Real z = pos.z;
	Real radius = 30.0f;

	// Draw the diamond.
	angle = 0.0f;
	lastPnt.x = pos.x + radius * (Real)cos(angle);
	lastPnt.y = pos.y + radius * (Real)sin(angle);
	lastPnt.z = z;
	bool shouldEnd = worldToScreen(&lastPnt, &end, camera);

	for( angle = inc; angle <= 2.0f * PI; angle += inc ) {
		pnt.x = pos.x + radius * (Real)cos(angle);
		pnt.y = pos.y + radius * (Real)sin(angle);
		pnt.z = z;

		bool shouldStart = worldToScreen(&pnt, &start, camera);
		if (shouldStart && shouldEnd) {
			m_lineRenderer->Add_Line(Vector2(start.x, start.y), Vector2(end.x, end.y), TEST_ART_HIGHLIGHT_LINE_WIDTH, color);
		}

		lastPnt = pnt;
		end = start;
		shouldEnd = shouldStart;
	}

}


/** Transform a 3D Coordinate into 2D screen space **/
// MLL C&C3
bool DrawObject::worldToScreen(const Coord3D *w, ICoord2D *s, CameraClass* camera)
{

	if ((w == NULL) || (s == NULL) || (camera == NULL)) {
		return false;
	}

	Vector3 world;
	Vector3 screen;

	world.Set(w->x, w->y, w->z);
	camera->Project(screen, world);

	//
	// note that the screen coord returned from the project W3D camera
	// gave us a screen coords that range from (-1,-1) bottom left to
	// (1,1) top right ... we are turning that into (0,0) upper left
	// coords now
	//
	W3DLogicalScreenToPixelScreen(screen.X, screen.Y, &s->x, &s->y, m_winSize.x, m_winSize.y);

	if ((screen.X > 2.0f) || (screen.Y > 2.0f) || (screen.X < -2.0f) || (screen.Y < -2.0f)) {
		// Too far off the screen.
		return false;
	}

	return (true);
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


bool _skip_drawobject_render = false;

/** Render draws into the current 3d context. */
void DrawObject::Render(RenderInfoClass & rinfo)
{
//DEBUG!
if (_skip_drawobject_render) {
	return;
}

	if (m_lineRenderer == NULL) {
		// This can't be created in init because the doc hasn't been created yet.
		m_lineRenderer = new Render2DClass();
		ASSERT(m_lineRenderer);
		CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
		ASSERT(pDoc);
		WbView3d *pView = pDoc->Get3DView();
		ASSERT(pView);
		m_winSize = pView->getActualWinSize();
		m_lineRenderer->Set_Coordinate_Range(RectClass(0, 0, m_winSize.x, m_winSize.y));
		m_lineRenderer->Reset();
		m_lineRenderer->Enable_Texturing(FALSE);
	}

	DX8Wrapper::Apply_Render_State_Changes();

	DX8Wrapper::Set_Material(m_vertexMaterialClass);
	DX8Wrapper::Set_Shader(m_shaderClass);
	DX8Wrapper::Set_Texture(0, NULL);
	DX8Wrapper::Set_Index_Buffer(m_indexBuffer,0);
	DX8Wrapper::Apply_Render_State_Changes();
	Int count=0;
	Int i;
	bool linesToRender = false;

	curHighlight++;
	if (curHighlight >= NUM_HIGHLIGHT) {
		curHighlight = 0;
	}
	m_waterDrawObject->update();
	DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferTile1);
  if (m_drawObjects || m_drawWaypoints || m_drawBoundingBoxes || m_drawSightRanges || m_drawWeaponRanges || m_drawSoundRanges || m_drawTestArtHighlight) {
		//Apply the shader and material

		//WST Variables below are for optimization to reduce VB updates which are extremely slow
		// Optimization strategy is to remember last setting and avoid re-updating unless it changed
		int rememberLastSettingVB1 = -99999;
		int rememberLastSettingVB2 = -99999;

		MapObject *pMapObj;
		for (pMapObj = MapObject::getFirstMapObject(); pMapObj; pMapObj = pMapObj->getNext()) {
			// simple Draw test.
			if (pMapObj->getFlags() & FLAG_DONT_RENDER) {
				continue;
			}

// DEBUG!
if (pMapObj->isSelected()) {
 Transform.Get_Translation();
}
			Coord3D loc = *pMapObj->getLocation();
			if (TheTerrainRenderObject) {
				loc.z += TheTerrainRenderObject->getHeightMapHeight(loc.x, loc.y, NULL);
			}
			// Cull.
			//SphereClass bounds(Vector3(loc.x, loc.y, loc.z), THE_RADIUS);
			//if (rinfo.Camera.Cull_Sphere(bounds)) {
			//	continue;
			//}
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
				// MLL C&C3
				if (pMapObj->isSelected()) {
					if (doArrow && m_drawBoundingBoxes) {
						linesToRender = true;
						updateVBWithBoundingBox(pMapObj, &rinfo.Camera);
					}
					if (doArrow && m_drawSightRanges) {
						linesToRender = true;
						updateVBWithSightRange(pMapObj, &rinfo.Camera);
					}
					if (doArrow && m_drawWeaponRanges) {
						linesToRender = true;
						updateVBWithWeaponRange(pMapObj, &rinfo.Camera);
					}
          if (doArrow && m_drawSoundRanges) {
            linesToRender = true;
            updateVBWithSoundRanges(pMapObj, &rinfo.Camera);
          }
				}

				if (doArrow && m_drawTestArtHighlight) {
					linesToRender = true;
					updateVBWithTestArtHighlight(pMapObj, &rinfo.Camera);
				}

				if (!m_drawObjects) {
					continue;
				}
				if (BuildListTool::isActive()) {
					continue;
				}
			}

			if (count&1) {
				int setting = pMapObj->getColor();

				if (doArrow) {
					setting |= (1<<25);
				}
				if (doDiamond) {
					setting |= (1<<26);
				}

				if (setting != rememberLastSettingVB1)	{
					rememberLastSettingVB1 = setting;
					updateVB(m_vertexBufferTile1,pMapObj->getColor(), doArrow, doDiamond);
				}
				DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferTile1);

			} else {
				int setting = pMapObj->getColor();

				if (doArrow) {
					setting |= (1<<25);
				}
				if (doDiamond) {
					setting |= (1<<26);
				}

				if (setting != rememberLastSettingVB2) {
					rememberLastSettingVB2 = setting;
					updateVB(m_vertexBufferTile2, pMapObj->getColor(), doArrow, doDiamond);
				}
				DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferTile2);
			}

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
			if (isTree) {
				DX8Wrapper::Draw_Triangles(	NUM_TRI*3,polyCount, 0,	(m_numTriangles*3));
			} else {
				DX8Wrapper::Draw_Triangles(	0,polyCount, 0,	(m_numTriangles*3));
			}

			count++;
		}
	}
	if (m_drawPolygonAreas) {
 		DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferWater);
		Int selected;
		for (selected = 0; selected < 2; selected++) {
			for (PolygonTrigger *pTrig=PolygonTrigger::getFirstPolygonTrigger(); pTrig; pTrig = pTrig->getNext()) {
				DX8Wrapper::Set_Index_Buffer(m_indexBuffer,0);
				if (!pTrig->getShouldRender()) continue;
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
			DX8Wrapper::Set_Transform(D3DTS_WORLD,tmXX);
			DX8Wrapper::Draw_Triangles(	0,polyCountA, 0,	(m_numTriangles*3));
#endif

		}
	}

	DX8Wrapper::Set_Index_Buffer(m_indexBuffer,0);
 	DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferWater);
	Matrix3D tmReset(Transform);
	DX8Wrapper::Set_Transform(D3DTS_WORLD,tmReset);

	if (m_drawWaypoints) {
		updateWaypointVB();
		if (m_feedbackIndexCount>0) {
 			DX8Wrapper::Set_Vertex_Buffer(m_vertexFeedback);
			DX8Wrapper::Set_Index_Buffer(m_indexFeedback,0);
			DX8Wrapper::Set_Shader(m_shaderClass);
			DX8Wrapper::Draw_Triangles(	0, m_feedbackIndexCount/3, 0,	m_feedbackVertexCount);
			DX8Wrapper::Set_Index_Buffer(m_indexBuffer,0);
 			DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferWater);
		}
	}



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

	if (m_drawLetterbox) {
		int w = m_winSize.x;
		int h = m_winSize.y;
		int size = (int)((h - (9.0f / 16.0f * w)) * 0.5f);
		RectClass rect(0, 0, w, size);
		m_lineRenderer->Add_Quad(rect, 0xFF000000);
		rect.Set(0, h - size, w, h);
		m_lineRenderer->Add_Quad(rect, 0xFF000000);
		linesToRender = true;
	}

	// Render any lines that have been added, like bounding boxes.
	// MLL C&C3
	if (linesToRender && m_lineRenderer) {
		m_lineRenderer->Render();
		// Clear the old lines.
		m_lineRenderer->Reset();
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
