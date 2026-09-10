import Assets.Math;
import Graphics.Renderer2D;
import Graphics.Frame.AttachmentBindings;
import Graphics.Frame.Runtime;
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
#include <algorithm>
import Graphics.Scene.Views.CameraMatrices;
import Graphics.Scene.DrawParameters;

#include "DrawObject.h"
#include <span>
#include "WWMath/matrix4.h"
import Graphics.Scene.Surfaces.Geometry;

#include <stdlib.h>
#include <WW3D2/AssetMgr.h>
#include <WW3D2/Texture.h>
#include <WWMath/tri.h>
#include <WWMath/colmath.h>
#include <WW3D2/ColTest.h>
#include <WW3D2/RInfo.h>
#include <WW3D2/Camera.h>
#include "Common/GlobalData.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "W3DDevice/GameClient/TerrainTex.h"
#include "W3DDevice/GameClient/BaseHeightMap.h"
#include "W3DDevice/GameClient/W3DAssetManager.h"
#include "W3DDevice/GameClient/W3DWater.h"
#include "W3DDevice/GameClient/W3DMeshRenderObject.h"
#include "W3DDevice/GameClient/W3DMeshResource.h"
import Graphics.Materials.State;
#include "WW3D2/WW3D.h"
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
#include "GameLogic/Weapon.h"
#include "Common/AudioEventInfo.h"

#ifdef RTS_DEBUG
#define NO_INTENSE_DEBUG 1
#endif

const Real LINE_THICKNESS = 2.0f;
const Real HANDLE_SIZE = (2.0f) * LINE_THICKNESS;


// Texturing, no zbuffer, disabled zbuffer write, primary gradient, alpha blending
#define SC_OPAQUE ( Graphics::MaterialState::Make_Bits(Graphics::MaterialState::PASS_ALWAYS, Graphics::MaterialState::DEPTH_WRITE_DISABLE, Graphics::MaterialState::COLOR_WRITE_ENABLE, Graphics::MaterialState::SRCBLEND_ONE, \
	Graphics::MaterialState::DSTBLEND_ZERO, Graphics::MaterialState::FOG_DISABLE, Graphics::MaterialState::GRADIENT_DISABLE, Graphics::MaterialState::SECONDARY_GRADIENT_DISABLE, Graphics::MaterialState::TEXTURING_ENABLE, \
	Graphics::MaterialState::ALPHATEST_DISABLE, Graphics::MaterialState::CULL_MODE_DISABLE, \
	Graphics::MaterialState::DETAILCOLOR_DISABLE, Graphics::MaterialState::DETAILALPHA_DISABLE) )

// Texturing, no zbuffer, disabled zbuffer write, primary gradient, alpha blending
#define SC_ALPHA ( Graphics::MaterialState::Make_Bits(Graphics::MaterialState::PASS_ALWAYS, Graphics::MaterialState::DEPTH_WRITE_DISABLE, Graphics::MaterialState::COLOR_WRITE_ENABLE, Graphics::MaterialState::SRCBLEND_SRC_ALPHA, \
	Graphics::MaterialState::DSTBLEND_ONE_MINUS_SRC_ALPHA, Graphics::MaterialState::FOG_DISABLE, Graphics::MaterialState::GRADIENT_MODULATE, Graphics::MaterialState::SECONDARY_GRADIENT_DISABLE, Graphics::MaterialState::TEXTURING_ENABLE, \
	Graphics::MaterialState::ALPHATEST_DISABLE, Graphics::MaterialState::CULL_MODE_ENABLE, \
	Graphics::MaterialState::DETAILCOLOR_DISABLE, Graphics::MaterialState::DETAILALPHA_DISABLE) )

// Texturing, no zbuffer, disabled zbuffer write, primary gradient, alpha blending
#define SC_ALPHA_Z ( Graphics::MaterialState::Make_Bits(Graphics::MaterialState::PASS_LEQUAL, Graphics::MaterialState::DEPTH_WRITE_DISABLE, Graphics::MaterialState::COLOR_WRITE_ENABLE, Graphics::MaterialState::SRCBLEND_SRC_ALPHA, \
	Graphics::MaterialState::DSTBLEND_ONE_MINUS_SRC_ALPHA, Graphics::MaterialState::FOG_DISABLE, Graphics::MaterialState::GRADIENT_MODULATE, Graphics::MaterialState::SECONDARY_GRADIENT_DISABLE, Graphics::MaterialState::TEXTURING_ENABLE, \
	Graphics::MaterialState::ALPHATEST_DISABLE, Graphics::MaterialState::CULL_MODE_DISABLE, \
	Graphics::MaterialState::DETAILCOLOR_DISABLE, Graphics::MaterialState::DETAILALPHA_DISABLE) )

// Texturing, no zbuffer, disabled zbuffer write, primary gradient, alpha blending
#define SC_OPAQUE_Z ( Graphics::MaterialState::Make_Bits(Graphics::MaterialState::PASS_LEQUAL, Graphics::MaterialState::DEPTH_WRITE_DISABLE, Graphics::MaterialState::COLOR_WRITE_ENABLE, Graphics::MaterialState::SRCBLEND_ONE, \
	Graphics::MaterialState::DSTBLEND_ZERO, Graphics::MaterialState::FOG_DISABLE, Graphics::MaterialState::GRADIENT_DISABLE, Graphics::MaterialState::SECONDARY_GRADIENT_DISABLE, Graphics::MaterialState::TEXTURING_ENABLE, \
	Graphics::MaterialState::ALPHATEST_DISABLE, Graphics::MaterialState::CULL_MODE_DISABLE, \
	Graphics::MaterialState::DETAILCOLOR_DISABLE, Graphics::MaterialState::DETAILALPHA_DISABLE) )


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



DrawObject::~DrawObject()
{
	freeMapResources();
	delete m_waterDrawObject;
	m_waterDrawObject = nullptr;
	TheWaterRenderSystem = nullptr;
}

DrawObject::DrawObject() :
	m_drawObjects(true),
	m_drawPolygonAreas(true),
	m_moldMesh(nullptr),
  m_drawSoundRanges(false)
{
	m_feedbackPoint.x = 20;
	m_feedbackPoint.y = 20;
	initData();
	m_waterDrawObject = new WaterRenderSystem;
	m_waterDrawObject->init(0, 0, 0, nullptr, WaterRenderSystem::WATER_TYPE_SURFACE);
	TheWaterRenderSystem=m_waterDrawObject;

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

Int DrawObject::Class_ID() const
{
	return RenderObjClass::CLASSID_UNKNOWN;
}

RenderObjClass * DrawObject::Clone() const
{
	return new DrawObject(*this);
}


Int DrawObject::freeMapResources()
{

	m_indexBuffer.clear();
	m_vertexBufferTile1.clear();
	m_vertexBufferTile2.clear();


	m_vertexFeedback.clear();
	m_indexFeedback.clear();

	REF_PTR_RELEASE(m_moldMesh);


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


Int DrawObject::initData()
{
	Int i;

	freeMapResources();	//free old data and ib/vb

	m_numTriangles = 2*NUM_TRI;
	m_indexBuffer.resize(m_numTriangles*3);

	// Fill up the IB
	unsigned *ib=m_indexBuffer.data();

	for (i=0; i<3*m_numTriangles; i+=3)
	{
		ib[0]=i;
		ib[1]=i+1;
		ib[2]=i+2;

		ib+=3;	//skip the 3 indices we just filled
	}

	m_vertexBufferTile1.resize(m_numTriangles*3);
	m_vertexBufferTile2.resize(m_numTriangles*3);

	m_vertexFeedback.resize(NUM_FEEDBACK_VERTEX);
	m_indexFeedback.resize(NUM_FEEDBACK_INDEX);

	//go with a preset material for now.

	//use a multi-texture shader: (text1*diffuse)*text2.
	m_shaderClass = Graphics::MaterialState(SC_OPAQUE);//_PresetOpaque2DShader;//Graphics::MaterialState(SC_OPAQUE); //_PresetOpaqueShader;

	m_shaderClass = Graphics::MaterialState::Opaque2D();
	updateForWater();
	updateVB(m_vertexBufferTile1, 255<<8, true, false);

	return 0;
}


/** updateMeshVB puts mesh mold triangles into m_vertexFeedback. */

void DrawObject::updateMeshVB()
{
	const Int theAlpha = 64;

	if (m_curMeshModelName != MeshMoldOptions::getModelName()) {
		REF_PTR_RELEASE(m_moldMesh);
		m_curMeshModelName = MeshMoldOptions::getModelName();
	}
	if (m_moldMesh == nullptr) {
 		WW3DAssetManager *pMgr = W3DAssetManager::Get_Instance();
		pMgr->Set_WW3D_Load_On_Demand(false);	 // We don't want it fishing for these assets in the game assets.
		m_moldMesh = (W3DMeshRenderObject*)pMgr->Create_Render_Obj(m_curMeshModelName.str());
		if (m_moldMesh == nullptr) {
			// Try loading the mold asset.
			AsciiString path("data\\editor\\molds\\");
			path.concat(m_curMeshModelName);
			path.concat(".w3d");
			pMgr->Load_3D_Assets(path.str());
			m_moldMesh = (W3DMeshRenderObject*)pMgr->Create_Render_Obj(m_curMeshModelName.str());
		}
		if (m_moldMesh) {
			m_moldMeshBounds = m_moldMesh->Get_Bounding_Sphere();
		}
		pMgr->Set_WW3D_Load_On_Demand(true);
	}
	if (m_moldMesh == nullptr) {
		return;
	}


	m_feedbackVertexCount = 0;
	m_feedbackIndexCount = 0;
	unsigned *ib=m_indexFeedback.data();
	unsigned *curIb = ib;

	Graphics::SurfaceVertex *vb = m_vertexFeedback.data();
	Graphics::SurfaceVertex *curVb = vb;

	if (m_moldMesh == nullptr) {
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
		curVb->uv[0] = 0;
		curVb->uv[1] = 0;
		Vector3 vLoc(pVert[i]);
		vLoc *= MeshMoldOptions::getScale();
		vLoc.Rotate_Z(MeshMoldOptions::getAngle()*PI/180.0f);
		vLoc.X += m_feedbackPoint.x;
		vLoc.Y += m_feedbackPoint.y;
		vLoc.Z += m_feedbackPoint.z;
		curVb->position[0] = vLoc.X;
		curVb->position[1] = vLoc.Y;
		curVb->position[2] = vLoc.Z;

		curVb->color = Assets::Color_From_ARGB(0x0000ffff | (theAlpha << 24)).To_Array(); // bright cyan.

		curVb++;
		m_feedbackVertexCount++;
	}
	// Put in the "center anchor"

	curVb->uv[0] = 0;
	curVb->uv[1] = 0;
	curVb->position[0] = m_feedbackPoint.x;
	curVb->position[1] = m_feedbackPoint.y;
	curVb->position[2] = 0;
	curVb->color = Assets::Color_From_ARGB(0xFFFF0000).To_Array();  // red.
	curVb++;
	m_feedbackVertexCount++;
	curVb->uv[0] = 0;
	curVb->uv[1] = 0;
	curVb->position[0] = m_feedbackPoint.x+1;
	curVb->position[1] = m_feedbackPoint.y+1;
	curVb->position[2] = m_feedbackPoint.z;
	curVb->color = Assets::Color_From_ARGB(0xFFFF0000).To_Array();  // red.
	curVb++;
	m_feedbackVertexCount++;
	curVb->uv[0] = 0;
	curVb->uv[1] = 0;
	curVb->position[0] = m_feedbackPoint.x;
	curVb->position[1] = m_feedbackPoint.y;
	curVb->position[2] = m_feedbackPoint.z-500;
	curVb->color = Assets::Color_From_ARGB(0xFFFF0000).To_Array();  // red.
	curVb++;
	m_feedbackVertexCount++;
	curVb->uv[0] = 0;
	curVb->uv[1] = 0;
	curVb->position[0] = m_feedbackPoint.x+1;
	curVb->position[1] = m_feedbackPoint.y+1;
	curVb->position[2] = m_feedbackPoint.z-500;
	curVb->color = Assets::Color_From_ARGB(0xFFFF0000).To_Array();  // red.
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

void DrawObject::updateRampVB()
{
	const Int theAlpha = 64;

	m_feedbackVertexCount = 0;
	m_feedbackIndexCount = 0;
	unsigned *ib=m_indexFeedback.data();
	unsigned *curIb = ib;

	Graphics::SurfaceVertex *vb = m_vertexFeedback.data();
	Graphics::SurfaceVertex *curVb = vb;

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
		curVb->uv[0] = INT_TO_REAL(i % widthVerts) / widthVerts;
		curVb->uv[1] = INT_TO_REAL(i / lengthVerts) / lengthVerts;

		curVb->color = Assets::Color_From_ARGB(curVb->diffuse = 0x0000ffff | (theAlpha << 24)).To_Array();		// bright cyan.

		Vector3 vLoc;
		vLoc.X = (br.X - bl.X) * INT_TO_REAL(i % widthVerts) / (widthVerts  - 1) +
						 (tl.X - bl.X) * INT_TO_REAL(i / lengthVerts) / (lengthVerts - 1) + bl.X;

		vLoc.Y = (br.Y - bl.Y) * INT_TO_REAL(i % widthVerts) / (widthVerts - 1) +
						 (tl.Y - bl.Y) * INT_TO_REAL(i / lengthVerts) / (lengthVerts - 1) + bl.Y;

		vLoc.Z = (br.Z - bl.Z) * INT_TO_REAL(i % widthVerts) / (widthVerts - 1) +
						 (tl.Z - bl.Z) * INT_TO_REAL(i / lengthVerts) / (lengthVerts - 1) + bl.Z;

		curVb->position[0] = vLoc.X;
		curVb->position[1] = vLoc.Y;
		curVb->position[2] = vLoc.Z;

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

	curVb->uv[0] = 0;
	curVb->uv[1] = 0;
	curVb->position[0] = m_feedbackPoint.x;
	curVb->position[1] = m_feedbackPoint.y;
	curVb->position[2] = 0;
	curVb->color = Assets::Color_From_ARGB(0xFFFF0000).To_Array();  // red.
	curVb++;
	m_feedbackVertexCount++;
	curVb->uv[0] = 0;
	curVb->uv[1] = 0;
	curVb->position[0] = m_feedbackPoint.x+1;
	curVb->position[1] = m_feedbackPoint.y+1;
	curVb->position[2] = m_feedbackPoint.z;
	curVb->color = Assets::Color_From_ARGB(0xFFFF0000).To_Array();  // red.
	curVb++;
	m_feedbackVertexCount++;
	curVb->uv[0] = 0;
	curVb->uv[1] = 0;
	curVb->position[0] = m_feedbackPoint.x;
	curVb->position[1] = m_feedbackPoint.y;
	curVb->position[2] = m_feedbackPoint.z-500;
	curVb->color = Assets::Color_From_ARGB(0xFFFF0000).To_Array();  // red.
	curVb++;
	m_feedbackVertexCount++;
	curVb->uv[0] = 0;
	curVb->uv[1] = 0;
	curVb->position[0] = m_feedbackPoint.x+1;
	curVb->position[1] = m_feedbackPoint.y+1;
	curVb->position[2] = m_feedbackPoint.z-500;
	curVb->color = Assets::Color_From_ARGB(0xFFFF0000).To_Array();  // red.
	curVb++;
	m_feedbackVertexCount++;
#endif
}

/** updateBoundaryVB puts boundaries into m_vertexFeedback. */
void DrawObject::updateBoundaryVB()
{
//	const Int theAlpha = 64;

	m_feedbackVertexCount = 0;
	m_feedbackIndexCount = 0;
	unsigned *ib=m_indexFeedback.data();
	unsigned *curIb = ib;

	Graphics::SurfaceVertex *vb = m_vertexFeedback.data();
	Graphics::SurfaceVertex *curVb = vb;

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
				startPt.z = TheTerrainRenderObject->getHeightMapHeight(startPt.x, startPt.y, nullptr);
				endPt.x = 0;
				endPt.y = curBoundary.y;
				endPt.x *= MAP_XY_FACTOR;
				endPt.y *= MAP_XY_FACTOR;
				endPt.z = TheTerrainRenderObject->getHeightMapHeight(endPt.x, endPt.y, nullptr);
			} else if (j == 1) {
				startPt = endPt;
				endPt.x = curBoundary.x;
				endPt.y = curBoundary.y;
				endPt.x *= MAP_XY_FACTOR;
				endPt.y *= MAP_XY_FACTOR;
				endPt.z = TheTerrainRenderObject->getHeightMapHeight(endPt.x, endPt.y, nullptr);
			} else if (j == 2) {
				startPt = endPt;
				endPt.x = curBoundary.x;
				endPt.y = 0;
				endPt.x *= MAP_XY_FACTOR;
				endPt.y *= MAP_XY_FACTOR;
				endPt.z = TheTerrainRenderObject->getHeightMapHeight(endPt.x, endPt.y, nullptr);
			} else if (j == 3) {
				startPt = endPt;
				endPt.x = 0;
				endPt.y = 0;
				endPt.x *= MAP_XY_FACTOR;
				endPt.y *= MAP_XY_FACTOR;
				endPt.z = TheTerrainRenderObject->getHeightMapHeight(endPt.x, endPt.y, nullptr);
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

			curVb->uv[0] = 0;
			curVb->uv[1] = 0;
			curVb->position[0] = startPt.x+normal.X;
			curVb->position[1] = startPt.y+normal.Y;
			curVb->position[2] = startPt.z;
			curVb->color = Assets::Color_From_ARGB(BORDER_COLORS[i % BORDER_COLORS_SIZE ].m_borderColor).To_Array();
			curVb++;
			m_feedbackVertexCount++;
			curVb->uv[0] = 0;
			curVb->uv[1] = 0;
			curVb->position[0] = startPt.x-normal.X;
			curVb->position[1] = startPt.y-normal.Y;
			curVb->position[2] = startPt.z;
			curVb->color = Assets::Color_From_ARGB(BORDER_COLORS[i % BORDER_COLORS_SIZE ].m_borderColor).To_Array();
			curVb++;
			m_feedbackVertexCount++;
			curVb->uv[0] = 0;
			curVb->uv[1] = 0;
			curVb->position[0] = endPt.x+normal.X;
			curVb->position[1] = endPt.y+normal.Y;
			curVb->position[2] = endPt.z;
			curVb->color = Assets::Color_From_ARGB(BORDER_COLORS[i % BORDER_COLORS_SIZE ].m_borderColor).To_Array();
			curVb++;
			m_feedbackVertexCount++;
			curVb->uv[0] = 0;
			curVb->uv[1] = 0;
			curVb->position[0] = endPt.x-normal.X;
			curVb->position[1] = endPt.y-normal.Y;
			curVb->position[2] = endPt.z;
			curVb->color = Assets::Color_From_ARGB(BORDER_COLORS[i % BORDER_COLORS_SIZE ].m_borderColor).To_Array();
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
			curVb->uv[0] = 0;
			curVb->uv[1] = 0;
			curVb->position[0] = startPt.x;
			curVb->position[1] = startPt.y - HANDLE_SIZE;
			curVb->position[2] = startPt.z;
			curVb->color = Assets::Color_From_ARGB(BORDER_COLORS[i % BORDER_COLORS_SIZE ].m_borderColor).To_Array();
			curVb++;
			m_feedbackVertexCount++;

			curVb->uv[0] = 0;
			curVb->uv[1] = 0;
			curVb->position[0] = startPt.x - HANDLE_SIZE;
			curVb->position[1] = startPt.y;
			curVb->position[2] = startPt.z;
			curVb->color = Assets::Color_From_ARGB(BORDER_COLORS[i % BORDER_COLORS_SIZE ].m_borderColor).To_Array();
			curVb++;
			m_feedbackVertexCount++;

			curVb->uv[0] = 0;
			curVb->uv[1] = 0;
			curVb->position[0] = startPt.x;
			curVb->position[1] = startPt.y + HANDLE_SIZE;
			curVb->position[2] = startPt.z;
			curVb->color = Assets::Color_From_ARGB(BORDER_COLORS[i % BORDER_COLORS_SIZE ].m_borderColor).To_Array();
			curVb++;
			m_feedbackVertexCount++;

			curVb->uv[0] = 0;
			curVb->uv[1] = 0;
			curVb->position[0] = startPt.x + HANDLE_SIZE;
			curVb->position[1] = startPt.y;
			curVb->position[2] = startPt.z;
			curVb->color = Assets::Color_From_ARGB(BORDER_COLORS[i % BORDER_COLORS_SIZE ].m_borderColor).To_Array();
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

void DrawObject::updateAmbientSoundVB()
{
	m_feedbackVertexCount = 0;
	m_feedbackIndexCount = 0;
	unsigned *ib=m_indexFeedback.data();
	unsigned *curIb = ib;

	Graphics::SurfaceVertex *vb = m_vertexFeedback.data();
	Graphics::SurfaceVertex *curVb = vb;

	MapObject* mo = MapObject::getFirstMapObject();

	while (mo) {
		if (!mo->getThingTemplate() || (mo->getThingTemplate()->getEditorSorting() != ES_AUDIO)) {
			mo = mo->getNext();
			continue;
		}

		Coord3D startPt = *mo->getLocation();
		startPt.z = TheTerrainRenderObject->getHeightMapHeight(startPt.x, startPt.y, nullptr);

		if (m_feedbackVertexCount + 6 > NUM_FEEDBACK_VERTEX) {
			return;
		}

		if (m_feedbackIndexCount + 12 > NUM_FEEDBACK_INDEX) {
			return;
		}

		curVb->uv[0] = 0;
		curVb->uv[1] = 0;
		curVb->position[0] = startPt.x;
		curVb->position[1] = startPt.y;
		curVb->position[2] = startPt.z;
		curVb->color = Assets::Color_From_ARGB(0xFF2525EF).To_Array();
		++curVb;
		++m_feedbackVertexCount;

		curVb->uv[0] = 0;
		curVb->uv[1] = 0;
		curVb->position[0] = startPt.x;
		curVb->position[1] = startPt.y;
		curVb->position[2] = startPt.z + poleHeight;
		curVb->color = Assets::Color_From_ARGB(0xFF2525EF).To_Array();
		++curVb;
		++m_feedbackVertexCount;

		curVb->uv[0] = 0;
		curVb->uv[1] = 0;
		curVb->position[0] = startPt.x + poleWidth;
		curVb->position[1] = startPt.y;
		curVb->position[2] = startPt.z + poleHeight;
		curVb->color = Assets::Color_From_ARGB(0xFF2525EF).To_Array();
		++curVb;
		++m_feedbackVertexCount;

		curVb->uv[0] = 0;
		curVb->uv[1] = 0;
		curVb->position[0] = startPt.x + poleWidth;
		curVb->position[1] = startPt.y;
		curVb->position[2] = startPt.z;
		curVb->color = Assets::Color_From_ARGB(0xFF2525EF).To_Array();
		++curVb;
		++m_feedbackVertexCount;

		curVb->uv[0] = 0;
		curVb->uv[1] = 0;
		curVb->position[0] = startPt.x;
		curVb->position[1] = startPt.y;
		curVb->position[2] = startPt.z + poleHeight + flagHeight;
		curVb->color = Assets::Color_From_ARGB(0xFF2525EF).To_Array();
		++curVb;
		++m_feedbackVertexCount;

		curVb->uv[0] = 0;
		curVb->uv[1] = 0;
		curVb->position[0] = startPt.x + flagWidth;
		curVb->position[1] = startPt.y;
		curVb->position[2] = startPt.z + poleHeight + (flagHeight / 2);
		curVb->color = Assets::Color_From_ARGB(0xFF2525EF).To_Array();
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

void DrawObject::updateWaypointVB()
{
//	const Int theAlpha = 64;

	m_feedbackVertexCount = 0;
	m_feedbackIndexCount = 0;
	unsigned *ib=m_indexFeedback.data();
	unsigned *curIb = ib;

	Graphics::SurfaceVertex *vb = m_vertexFeedback.data();
	Graphics::SurfaceVertex *curVb = vb;

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
				loc1.z = TheTerrainRenderObject->getHeightMapHeight(loc1.x, loc1.y, nullptr);
				loc2.z = TheTerrainRenderObject->getHeightMapHeight(loc2.x, loc2.y, nullptr);

				if (m_feedbackVertexCount+9>= NUM_FEEDBACK_VERTEX) {
					return;
				}
				curVb->uv[0] = 0;
				curVb->uv[1] = 0;
				curVb->position[0] = loc1.x+normal.X;
				curVb->position[1] = loc1.y+normal.Y;
				curVb->position[2] = loc1.z;
				curVb->color = Assets::Color_From_ARGB(0xFF000000).To_Array();  // black.
				curVb++;
				m_feedbackVertexCount++;
				curVb->uv[0] = 0;
				curVb->uv[1] = 0;
				curVb->position[0] = loc1.x-normal.X;
				curVb->position[1] = loc1.y-normal.Y;
				curVb->position[2] = loc1.z;
				curVb->color = Assets::Color_From_ARGB(0xFF000000).To_Array();  // black.
				curVb++;
				m_feedbackVertexCount++;
				curVb->uv[0] = 0;
				curVb->uv[1] = 0;
				curVb->position[0] = loc2.x+normal.X;
				curVb->position[1] = loc2.y+normal.Y;
				curVb->position[2] = loc2.z;
				curVb->color = Assets::Color_From_ARGB(0xFFFF0000).To_Array();  // red.
				curVb++;
				m_feedbackVertexCount++;
				curVb->uv[0] = 0;
				curVb->uv[1] = 0;
				curVb->position[0] = loc2.x-normal.X;
				curVb->position[1] = loc2.y-normal.Y;
				curVb->position[2] = loc2.z;
				curVb->color = Assets::Color_From_ARGB(0xFFFF0000).To_Array();  // red.
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
				curVb->uv[0] = 0;
				curVb->uv[1] = 0;
				curVb->position[0] = loc1.x+NORMAL_SHIFT*normal.X+normal.X;
				curVb->position[1] = loc1.y+NORMAL_SHIFT*normal.Y+normal.Y;
				curVb->position[2] = loc1.z;
				curVb->color = Assets::Color_From_ARGB(0xFFFF0000).To_Array();  // red.
				curVb++;
				m_feedbackVertexCount++;
				curVb->uv[0] = 0;
				curVb->uv[1] = 0;
				curVb->position[0] = loc1.x+NORMAL_SHIFT*normal.X;
				curVb->position[1] = loc1.y+NORMAL_SHIFT*normal.Y;
				curVb->position[2] = loc1.z;
				curVb->color = Assets::Color_From_ARGB(0xFFFF0000).To_Array();  // red.
				curVb++;
				m_feedbackVertexCount++;
				curVb->uv[0] = 0;
				curVb->uv[1] = 0;
				curVb->position[0] = loc2.x+normal.X;
				curVb->position[1] = loc2.y+normal.Y;
				curVb->position[2] = loc2.z;
				curVb->color = Assets::Color_From_ARGB(0xFFFF0000).To_Array();  // red.
				curVb++;
				m_feedbackVertexCount++;
				curVb->uv[0] = 0;
				curVb->uv[1] = 0;
				curVb->position[0] = loc2.x-normal.X;
				curVb->position[1] = loc2.y-normal.Y;
				curVb->position[2] = loc2.z;
				curVb->color = Assets::Color_From_ARGB(0xFFFF0000).To_Array();  // red.
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
				curVb->uv[0] = 0;
				curVb->uv[1] = 0;
				curVb->position[0] = loc1.x-NORMAL_SHIFT*normal.X;
				curVb->position[1] = loc1.y-NORMAL_SHIFT*normal.Y;
				curVb->position[2] = loc1.z;
				curVb->color = Assets::Color_From_ARGB(0xFFFF0000).To_Array();  // red.
				curVb++;
				m_feedbackVertexCount++;
				curVb->uv[0] = 0;
				curVb->uv[1] = 0;
				curVb->position[0] = loc1.x-NORMAL_SHIFT*normal.X-normal.X;
				curVb->position[1] = loc1.y-NORMAL_SHIFT*normal.Y-normal.Y;
				curVb->position[2] = loc1.z;
				curVb->color = Assets::Color_From_ARGB(0xFFFF0000).To_Array();  // red.
				curVb++;
				m_feedbackVertexCount++;
				curVb->uv[0] = 0;
				curVb->uv[1] = 0;
				curVb->position[0] = loc2.x+normal.X;
				curVb->position[1] = loc2.y+normal.Y;
				curVb->position[2] = loc2.z;
				curVb->color = Assets::Color_From_ARGB(0xFFFF0000).To_Array();  // red.
				curVb++;
				m_feedbackVertexCount++;
				curVb->uv[0] = 0;
				curVb->uv[1] = 0;
				curVb->position[0] = loc2.x-normal.X;
				curVb->position[1] = loc2.y-normal.Y;
				curVb->position[2] = loc2.z;
				curVb->color = Assets::Color_From_ARGB(0xFFFF0000).To_Array();  // red.
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
	unsigned *ib=m_indexFeedback.data();
	unsigned *curIb = ib;

	Graphics::SurfaceVertex *vb = m_vertexFeedback.data();
	Graphics::SurfaceVertex *curVb = vb;

	Int i;
	for (i=0; i<pTrig->getNumPoints(); i++) {
		Coord3D loc1;
		Coord3D loc2;
		ICoord3D iLoc = *pTrig->getPoint(i);
		loc1.x = iLoc.x;
		loc1.y = iLoc.y;
		loc1.z = TheTerrainRenderObject->getHeightMapHeight(loc1.x, loc1.y, nullptr);
		if (i<pTrig->getNumPoints()-1) {
			iLoc = *pTrig->getPoint(i+1);
		} else {
			if (isOpen) break;
			iLoc = *pTrig->getPoint(0);
		}
		loc2.x = iLoc.x;
		loc2.y = iLoc.y;
		loc2.z = TheTerrainRenderObject->getHeightMapHeight(loc2.x, loc2.y, nullptr);
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
		curVb->uv[0] = 0;
		curVb->uv[1] = 0;
		curVb->position[0] = loc1.x+normal.X;
		curVb->position[1] = loc1.y+normal.Y;
		curVb->position[2] = loc1.z;
		curVb->color = Assets::Color_From_ARGB(diffuse).To_Array();
		curVb++;
		m_feedbackVertexCount++;
		curVb->uv[0] = 0;
		curVb->uv[1] = 0;
		curVb->position[0] = loc1.x-normal.X;
		curVb->position[1] = loc1.y-normal.Y;
		curVb->position[2] = loc1.z;
		curVb->color = Assets::Color_From_ARGB(diffuse).To_Array();
		curVb++;
		m_feedbackVertexCount++;
		curVb->uv[0] = 0;
		curVb->uv[1] = 0;
		curVb->position[0] = loc2.x+normal.X;
		curVb->position[1] = loc2.y+normal.Y;
		curVb->position[2] = loc2.z;
		curVb->color = Assets::Color_From_ARGB(diffuse).To_Array();
		curVb++;
		m_feedbackVertexCount++;
		curVb->uv[0] = 0;
		curVb->uv[1] = 0;
		curVb->position[0] = loc2.x-normal.X;
		curVb->position[1] = loc2.y-normal.Y;
		curVb->position[2] = loc2.z;
		curVb->color = Assets::Color_From_ARGB(diffuse).To_Array();
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

void DrawObject::updateFeedbackVB()
{
	const Int theAlpha = 64;
	m_feedbackVertexCount = 0;
	m_feedbackIndexCount = 0;
	unsigned *ib=m_indexFeedback.data();
	unsigned *curIb = ib;

	Graphics::SurfaceVertex *vb = m_vertexFeedback.data();
	Graphics::SurfaceVertex *curVb = vb;

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
				curVb->color = Assets::Color_From_ARGB(diffuse).To_Array();
			} else {
				Real blendFactor = Tool::calcRoundBlendFactor(m_cellCenter, i, j, brushWidth, featherWidth);
				if (blendFactor > 0.99) {
					curVb->color = Assets::Color_From_ARGB(diffuse).To_Array();
				} else if (blendFactor > 0.05) {
					curVb->color = Assets::Color_From_ARGB(featherDiffuse | (theAlpha<<24)).To_Array();
				}	else {
					curVb->color = Assets::Color_From_ARGB(0).To_Array();
				}
			}
			Real X, Y, theZ;
			if (doubleResolution) {
				X = ADJUST_FROM_INDEX_TO_REAL(i)/2.0f + ADJUST_FROM_INDEX_TO_REAL(2*offset+m_cellCenter.x)  / 2.0;
				Y = ADJUST_FROM_INDEX_TO_REAL(j)/2.0f + ADJUST_FROM_INDEX_TO_REAL(2*offset+m_cellCenter.y)  / 2.0;
				theZ = TheTerrainRenderObject->getHeightMapHeight(X, Y, nullptr);
			} else {
				X = ADJUST_FROM_INDEX_TO_REAL(i);
				Y = ADJUST_FROM_INDEX_TO_REAL(j);
				theZ = TheTerrainRenderObject->getHeightMapHeight(X, Y, nullptr);
			}
			curVb->uv[0] = 0;
			curVb->uv[1] = 0;
			curVb->position[0] = X;
			curVb->position[1] = Y;
			curVb->position[2] = theZ;
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

void DrawObject::updateForWater()
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

Int DrawObject::updateVB(std::vector<Graphics::SurfaceVertex>& pVB, Int color, Bool doArrow, Bool doDiamond)
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
	if (!pVB.empty())
	{

	Graphics::SurfaceVertex *vb = pVB.data();

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
				vb->position[2]=  theZ;
				if (k==0) {
					vb->position[0]=	0;
					vb->position[1]=	0;

					Vector3 vec(0,0,theZ);
					vec.Rotate_Z(curAngle+(deltaAngle/2));
					vb->position[0]=	vec.X;
					vb->position[1]=	vec.Y;
				} else if (k==1) {
					Vector3 vec(theRadius/10,0,theZ);
					vec.Rotate_Z(curAngle);
					vb->position[0]=	vec.X;
					vb->position[1]=	vec.Y;
				} else if (k==2) {
					Real angle = curAngle+deltaAngle;
					if (i==limit-1) {
						angle = 0;
					}
					Vector3 vec(theRadius/10,0,theZ);
					vec.Rotate_Z(angle);
					vb->position[0]=	vec.X;
					vb->position[1]=	vec.Y;
				}
				vb->color = Assets::Color_From_ARGB(diffuse).To_Array();
				vb->uv[0]=0;
				vb->uv[1]=0;
				vb[3*NUM_TRI] = *vb;
				if (k==0) {
					vb[3*NUM_TRI].position[2] += 3.0;
					vb[3*NUM_TRI].color = Assets::Color_From_ARGB(diffuse).To_Array();
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
			vb->position[0]=	(k&1)?2*theRadius:0.0f;
			vb->position[1]=	-halfLineWidth + ((k&2)?2*halfLineWidth:0);
			vb->position[2]=  theZ;
			vb->color = Assets::Color_From_ARGB(highlightColors[curHighlight] + (theAlpha<<24)).To_Array();
			vb->uv[0]=0;
			vb->uv[1]=0;
			vb[3*NUM_TRI] = *vb;
			vb++;
		}
		for (k=0; k<3; k++) {
			vb->position[0]=	(k&1)?0.0f:2*theRadius;
			vb->position[1]=	halfLineWidth - ((k&2)?2*halfLineWidth:0);
			vb->position[2]=  theZ;
			vb->color = Assets::Color_From_ARGB(highlightColors[curHighlight] + (theAlpha<<24)).To_Array();
			vb->uv[0]=0;
			vb->uv[1]=0;
			vb++;
		}
		for (k=0; k<3; k++) {
			if (k==0) { vb->position[0]=theRadius; vb->position[1] = 0;}
			else if (k==1) { vb->position[0]=2*theRadius + 2*halfLineWidth; vb->position[1] = 0;}
			else { vb->position[0]=theRadius; vb->position[1] = 2*halfLineWidth;}
			vb->position[2]=  theZ;
			vb->color = Assets::Color_From_ARGB(highlightColors[curHighlight] + (theAlpha<<24)).To_Array();
			vb->uv[0]=0;
			vb->uv[1]=0;
			vb[3*NUM_TRI] = *vb;
			vb++;
		}
		for (k=0; k<3; k++) {
			if (k==0) { vb->position[0]=theRadius; vb->position[1] = 0;}
			else if (k==1) { vb->position[0]=theRadius; vb->position[1] = -2*halfLineWidth;}
			else { vb->position[0]=2*theRadius + 2*halfLineWidth; vb->position[1] = 0;}
			vb->position[2]=  theZ;
			vb->color = Assets::Color_From_ARGB(highlightColors[curHighlight] + (theAlpha<<24)).To_Array();
			vb->uv[0]=0;
			vb->uv[1]=0;
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
				vb->position[2]=  theZ;
				if (k==0) {
					vb->position[0]=	0;
					vb->position[1]=	0;

					Vector3 vec(theRadius*4/5,0,theZ);
					vec.Rotate_Z(curAngle+(deltaAngle/2));
					vb->position[0]=	vec.X;
					vb->position[1]=	vec.Y;
				} else if (k==1) {
					Vector3 vec(theRadius,0,theZ);
					vec.Rotate_Z(curAngle);
					vb->position[0]=	vec.X;
					vb->position[1]=	vec.Y;
				} else if (k==2) {
					Real angle = curAngle+deltaAngle;
					if (i==limit-1) {
						angle = 0;
					}
					Vector3 vec(theRadius,0,theZ);
					vec.Rotate_Z(angle);
					vb->position[0]=	vec.X;
					vb->position[1]=	vec.Y;
				}
				vb->color = Assets::Color_From_ARGB(highlightColors[curHighlight] + (theAlpha<<24)).To_Array();
				vb->uv[0]=0;
				vb->uv[1]=0;
				vb[3*NUM_TRI] = *vb;
				if (k==0) {
					vb[3*NUM_TRI].position[2] += 3.0;
					vb[3*NUM_TRI].color = Assets::Color_From_ARGB(highlightColors[curHighlight] + (theAlpha<<24)).To_Array();	 // b g<<8 r<<16 a<<24.
				}
				vb++;
			}
			curAngle += deltaAngle;

		}

#if 0
		// Now do the highlight triangle.  This is in yellow.
		for (k=0; k<3; k++) {
			vb->position[0] = k==0?theRadius:0;
			vb->position[1] = k==1?theRadius:0;
			vb->position[2]=  k==2?theZ+SELECT_PYRAMID_HEIGHT:theZ;
			vb->color = Assets::Color_From_ARGB(highlightColors[curHighlight] + (theAlpha<<24)).To_Array();	 // b g<<8 r<<16 a<<24.
			vb->uv[0]=0;
			vb->uv[1]=0;
			vb[3*NUM_TRI] = *vb;
			vb++;
		}
		for (k=0; k<3; k++) {
			vb->position[0] = k==1?-theRadius:0;
			vb->position[1] = k==0?theRadius:0;
			vb->position[2]=  k==2?theZ+SELECT_PYRAMID_HEIGHT:theZ;
			vb->color = Assets::Color_From_ARGB(highlightColors[curHighlight] + (theAlpha<<24)).To_Array();	 // b g<<8 r<<16 a<<24.
			vb->uv[0]=0;
			vb->uv[1]=0;
			vb[3*NUM_TRI] = *vb;
			vb++;
		}

		for (k=0; k<3; k++) {
			vb->position[0] = k==1?theRadius:0;
			vb->position[1] = k==0?-theRadius:0;
			vb->position[2]=  k==2?theZ+SELECT_PYRAMID_HEIGHT:theZ;
			vb->color = Assets::Color_From_ARGB(highlightColors[curHighlight] + (theAlpha<<24)).To_Array();	 // b g<<8 r<<16 a<<24.
			vb->uv[0]=0;
			vb->uv[1]=0;
			vb[3*NUM_TRI] = *vb;
			vb++;
		}
		for (k=0; k<3; k++) {
			vb->position[0] = k==0?-theRadius:0;
			vb->position[1] = k==1?-theRadius:0;
			vb->position[2]=  k==2?theZ+SELECT_PYRAMID_HEIGHT:theZ;
			vb->color = Assets::Color_From_ARGB(highlightColors[curHighlight] + (theAlpha<<24)).To_Array();	 // b g<<8 r<<16 a<<24.
			vb->uv[0]=0;
			vb->uv[1]=0;
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
	if (!pMapObj || !pMapObj->getThingTemplate()) {
		return;
	}

	unsigned long color = 0xFFAA00AA; // Purple

	GeometryInfo ginfo = pMapObj->getThingTemplate()->getTemplateGeometryInfo();

	Coord3D pos = *pMapObj->getLocation();
	if (TheTerrainRenderObject) {
		// Make sure that the position is on the terrain.
		pos.z += TheTerrainRenderObject->getHeightMapHeight(pos.x, pos.y, nullptr);
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
						Graphics::Get_Renderer2D().Add_Line({start.x - 0.5f, start.y - 0.5f},
                        {end.x - 0.5f, end.y - 0.5f}, BOUNDING_BOX_LINE_WIDTH, Graphics::Color2D::From_ARGB(color));
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
						Graphics::Get_Renderer2D().Add_Line({start.x - 0.5f, start.y - 0.5f},
                        {end.x - 0.5f, end.y - 0.5f}, BOUNDING_BOX_LINE_WIDTH, Graphics::Color2D::From_ARGB(color));
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
				Graphics::Get_Renderer2D().Add_Line({start.x - 0.5f, start.y - 0.5f},
                        {end.x - 0.5f, end.y - 0.5f}, BOUNDING_BOX_LINE_WIDTH, Graphics::Color2D::From_ARGB(color));
			}
			break;
		}
	}
}

/** Draw a "circle" into the graphics overlay batch, e.g. to visualize weapon range, sight range, sound range **/
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
      Graphics::Get_Renderer2D().Add_Line({start.x - 0.5f, start.y - 0.5f},
                        {end.x - 0.5f, end.y - 0.5f}, width, Graphics::Color2D::From_ARGB(color));
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
	if (!pMapObj || !pMapObj->getThingTemplate()) {
		return;
	}

	const unsigned long color = 0xFFF0F0F0; // Light blue.

	Real radius = pMapObj->getThingTemplate()->friend_calcVisionRange();

	Coord3D pos = *pMapObj->getLocation();
	if (TheTerrainRenderObject) {
		// Make sure that the position is on the terrain.
		pos.z += TheTerrainRenderObject->getHeightMapHeight(pos.x, pos.y, nullptr);
	}

  addCircleToLineRenderer(pos, radius, SIGHT_RANGE_LINE_WIDTH, color, camera );
}

#define WEAPON_RANGE_LINE_WIDTH 1.0f
/** Draw an object's weapon range into the vertex buffer. **/
// MLL C&C3
void DrawObject::updateVBWithWeaponRange(MapObject *pMapObj, CameraClass* camera)
{
	if (!pMapObj || !pMapObj->getThingTemplate()) {
		return;
	}

  const unsigned long colors[WEAPONSLOT_COUNT] = {0xFF00FF00, 0xFFE0F00A, 0xFFFF0000}; // Green, Yellow, Red


	Coord3D pos = *pMapObj->getLocation();
	if (TheTerrainRenderObject) {
		// Make sure that the position is on the terrain.
		pos.z += TheTerrainRenderObject->getHeightMapHeight(pos.x, pos.y, nullptr);
	}

	const WeaponTemplateSetVector& weapons = pMapObj->getThingTemplate()->getWeaponTemplateSets();

	for (WeaponTemplateSetVector::const_iterator it = weapons.begin(); it != weapons.end(); ++it)	{
		if (it->hasAnyWeapons() == false) {
			continue;
		}

		for (int i = 0; i < WEAPONSLOT_COUNT; i++) {
			const WeaponTemplate* tmpl = it->getNth((WeaponSlotType)i);

			if (tmpl == nullptr) {
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
  if (!pMapObj) {
    return;
  }

  const unsigned long colors[2] = {0xFF0000FF, 0xFFFF00FF}; // Blue and purple
                                                            // Colors match those used in W3DView.cpp


  Coord3D pos = *pMapObj->getLocation();
  if (TheTerrainRenderObject) {
    // Make sure that the position is on the terrain.
    pos.z += TheTerrainRenderObject->getHeightMapHeight(pos.x, pos.y, nullptr);
  }

  // Does this object actually have an attached sound?
  const AudioEventInfo * audioInfo = nullptr;

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
      if ( TheAudio == nullptr )
      {
        DEBUG_CRASH( ("TheAudio is null! Can't draw sound circles") );
        return;
      }

      audioInfo = TheAudio->findAudioEventInfo( ambientName );

      if ( audioInfo == nullptr )
      {
        DEBUG_CRASH( ("Override audio named %s is missing; Can't draw sound circles", ambientName.str() ) );
        return;
      }
    }
  }
  else
  {
    const ThingTemplate * thingTemplate = pMapObj->getThingTemplate();
    if ( thingTemplate == nullptr )
    {
      // No sound if no template
      return;
    }

    if ( !thingTemplate->hasSoundAmbient() )
    {
      return;
    }

    const AudioEventRTS * event = thingTemplate->getSoundAmbient();

    if ( event == nullptr )
    {
      return;
    }

    audioInfo = event->getAudioEventInfo();

    if ( audioInfo == nullptr )
    {
      // May just not be set up yet
      if ( TheAudio == nullptr )
      {
        DEBUG_CRASH( ("TheAudio is null! Can't draw sound circles") );
        return;
      }

      audioInfo = TheAudio->findAudioEventInfo( event->getEventName() );

      if ( audioInfo == nullptr )
      {
        DEBUG_CRASH( ("Default ambient sound %s has no info; Can't draw sound circles", event->getEventName().str() ) );
        return;
      }
    }
  }

  // Should have set up audioInfo or returned by now
  DEBUG_ASSERTCRASH( audioInfo != nullptr, ("Managed to finish setting up audio info without setting it?!?" ) );
  if ( audioInfo == nullptr )
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
	if (!pMapObj || pMapObj->getThingTemplate() || pMapObj->isScorch()) {
		// It is test art if it doesn't have a ThingTemplate.
		return;
	}

	unsigned long color = 0xFFA000A0; // Purple


	Coord3D pos = *pMapObj->getLocation();
	if (TheTerrainRenderObject) {
		// Make sure that the position is on the terrain.
		pos.z += TheTerrainRenderObject->getHeightMapHeight(pos.x, pos.y, nullptr);
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
			Graphics::Get_Renderer2D().Add_Line({start.x - 0.5f, start.y - 0.5f},
                        {end.x - 0.5f, end.y - 0.5f}, TEST_ART_HIGHLIGHT_LINE_WIDTH, Graphics::Color2D::From_ARGB(color));
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

	if ((w == nullptr) || (s == nullptr) || (camera == nullptr)) {
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
	DEBUG_ASSERTCRASH(start && end, ("Parameter passed into setRampFeedbackParms was null. Not allowed"));
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

	if (Graphics::Shared_Frame_Device() == nullptr)
		return;

	const auto viewport = Graphics::Get_Attachment_Bindings().Default().viewport;
	m_winSize = CPoint(static_cast<int>(viewport.width), static_cast<int>(viewport.height));


	std::span<const Graphics::SurfaceVertex> vertices;
    std::span<const unsigned> indices;
    Graphics::MaterialState shader = m_shaderClass;
    Matrix4x4 world(Transform), view, projection;
    std::copy_n(Graphics::Get_Camera_Matrices().view.values.data(), 16, &view[0][0]);
    std::copy_n(Graphics::Get_Camera_Matrices().projection.values.data(), 16, &projection[0][0]);
    const auto draw = [&](unsigned vertex_count, unsigned first_index, unsigned triangle_count) {
        if (vertex_count > vertices.size() || first_index > indices.size()
            || triangle_count > (indices.size() - first_index) / 3) {
            DEBUG_CRASH(("Editor overlay geometry exceeds its CPU batch."));
            return;
        }
        if (!Draw_Graphics_Prelit_Geometry(vertices.first(vertex_count),
            indices.subspan(first_index, triangle_count * 3), projection * view * world, shader, nullptr))
            DEBUG_LOG(("Editor overlay graphics submission failed.\n"));
    };
	shader = Graphics::MaterialState(m_shaderClass);
	indices = m_indexBuffer;

	Int count=0;
	Int i;

	curHighlight++;
	if (curHighlight >= NUM_HIGHLIGHT) {
		curHighlight = 0;
	}
	m_waterDrawObject->update();
	vertices = m_vertexBufferTile1;
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
				loc.z += TheTerrainRenderObject->getHeightMapHeight(loc.x, loc.y, nullptr);
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
						updateVBWithBoundingBox(pMapObj, &rinfo.Camera);
					}
					if (doArrow && m_drawSightRanges) {
						updateVBWithSightRange(pMapObj, &rinfo.Camera);
					}
					if (doArrow && m_drawWeaponRanges) {
						updateVBWithWeaponRange(pMapObj, &rinfo.Camera);
					}
          if (doArrow && m_drawSoundRanges) {
            updateVBWithSoundRanges(pMapObj, &rinfo.Camera);
          }
				}

				if (doArrow && m_drawTestArtHighlight) {
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
				vertices = m_vertexBufferTile1;

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
				vertices = m_vertexBufferTile2;
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

			world = Matrix4x4(tm);
			if (isTree) {
				draw(m_numTriangles * 3, NUM_TRI * 3, polyCount);
			} else {
				draw(m_numTriangles * 3, 0, polyCount);
			}

			count++;
		}
	}
	if (m_drawPolygonAreas) {
		vertices = {};
		Int selected;
		for (selected = 0; selected < 2; selected++) {
			for (PolygonTrigger *pTrig=PolygonTrigger::getFirstPolygonTrigger(); pTrig; pTrig = pTrig->getNext()) {
				indices = m_indexBuffer;
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
					loc.z = TheTerrainRenderObject->getHeightMapHeight(loc.x, loc.y, nullptr);
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
						vertices = m_vertexBufferTile1;
					} else {
						updateVB(m_vertexBufferTile2, color, ARROW, DIAMOND);
						vertices = m_vertexBufferTile2;
					}
					count++;

					Vector3 vec(loc.x, loc.y, loc.z);
					Matrix3D tm(Transform);
					tm.Set_Translation(vec);

					int polyCount = NUM_TRI;
					if (!pointSelected) {
						polyCount -= NUM_ARROW_TRI+NUM_SELECT_TRI;
					}

					indices = m_indexBuffer;
					world = Matrix4x4(tm);
					draw(m_numTriangles * 3, 0, polyCount);
				}
				Matrix3D tmReset(Transform);
				world = Matrix4x4(tmReset);
				vertices = m_vertexBufferTile1;
				updatePolygonVB(pTrig, polySelected, polySelected && PolygonTool::isSelectedOpen());
				vertices = m_vertexFeedback;
				if (m_feedbackIndexCount>0) {
					indices = m_indexFeedback;
					draw(m_feedbackVertexCount, 0, m_feedbackIndexCount / 3);
				}
			}
			indices = m_indexBuffer;
		}
	}


 	if (BuildListTool::isActive()) for (i=0; i<TheSidesList->getNumSides(); i++) {
		SidesInfo *pSide = TheSidesList->getSideInfo(i);
		for (BuildListInfo *pBuild = pSide->getBuildList(); pBuild; pBuild = pBuild->getNext()) {
			Coord3D loc = *pBuild->getLocation();
			if (TheTerrainRenderObject) {
				loc.z += TheTerrainRenderObject->getHeightMapHeight(loc.x, loc.y, nullptr);
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
				vertices = m_vertexBufferTile1;
			} else {
				updateVB(m_vertexBufferTile2, GREEN, true, false);
				vertices = m_vertexBufferTile2;
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
			world = Matrix4x4(tmXX);
			draw(m_numTriangles * 3, 0, polyCountA);
#endif

		}
	}

	indices = m_indexBuffer;
	vertices = {};
	Matrix3D tmReset(Transform);
	world = Matrix4x4(tmReset);

	if (m_drawWaypoints) {
		updateWaypointVB();
		if (m_feedbackIndexCount>0) {
				vertices = m_vertexFeedback;
			indices = m_indexFeedback;
			shader = Graphics::MaterialState(m_shaderClass);
			draw(m_feedbackVertexCount, 0, m_feedbackIndexCount / 3);
			indices = m_indexBuffer;
			vertices = {};
		}
	}



#if 1
	if (m_meshFeedback) {
		updateMeshVB();
		if (m_feedbackIndexCount>0) {
				vertices = m_vertexFeedback;
			indices = m_indexFeedback;
			shader = Graphics::MaterialState(SC_OPAQUE_Z);
			Graphics::Get_Scene_Draw_Parameters().wireframe = true;
			draw(m_feedbackVertexCount, 0, m_feedbackIndexCount / 3);
		}
	} else if (m_toolWantsFeedback && !m_disableFeedback) {
		updateFeedbackVB();
		if (m_feedbackIndexCount>0) {
				vertices = m_vertexFeedback;
			indices = m_indexFeedback;
			shader = Graphics::MaterialState(Graphics::MaterialState::Alpha2D());
			draw(m_feedbackVertexCount, 0, m_feedbackIndexCount / 3);
		}
	}
#endif

#if 1
	if (m_rampFeedback) {
		updateRampVB();
		if (m_feedbackIndexCount>0) {
				vertices = m_vertexFeedback;
			indices = m_indexFeedback;
			shader = Graphics::MaterialState(SC_OPAQUE_Z);
			Graphics::Get_Scene_Draw_Parameters().wireframe = true;	// we want a solid ramp
			draw(m_feedbackVertexCount, 0, m_feedbackIndexCount / 3);
		}
	}
#endif

#if 1
	if (m_boundaryFeedback) {
		updateBoundaryVB();
		if (m_feedbackIndexCount>0) {
				vertices = m_vertexFeedback;
			indices = m_indexFeedback;
			shader = Graphics::MaterialState(m_shaderClass);
			shader.Set_Cull_Mode(Graphics::MaterialState::CULL_MODE_DISABLE);
			Graphics::Get_Scene_Draw_Parameters().wireframe = false;	// we want a solid ramp
			draw(m_feedbackVertexCount, 0, m_feedbackIndexCount / 3);
		}
	}
#endif

	vertices = {};	//release reference to vertex buffer
	indices = {};	//release reference to vertex buffer


	if (m_ambientSoundFeedback) {
		updateAmbientSoundVB();
		if (m_feedbackIndexCount>0) {
				vertices = m_vertexFeedback;
			indices = m_indexFeedback;
			shader = Graphics::MaterialState(m_shaderClass);
			shader.Set_Cull_Mode(Graphics::MaterialState::CULL_MODE_DISABLE);
			Graphics::Get_Scene_Draw_Parameters().wireframe = false;	// we want a solid ramp
			draw(m_feedbackVertexCount, 0, m_feedbackIndexCount / 3);
		}
	}

	  indices = m_indexBuffer;
		vertices = {};

	if (m_waterDrawObject) {
		m_waterDrawObject->renderWater();
	}

	if (m_drawLetterbox) {
		int w = m_winSize.x;
		int h = m_winSize.y;
		int size = (int)((h - (9.0f / 16.0f * w)) * 0.5f);
		if (size > 0) {
			Graphics::Get_Renderer2D().Add_Rect({-0.5f, -0.5f, w - 0.5f, size - 0.5f}, {0, 0, 0, 1});
			Graphics::Get_Renderer2D().Add_Rect({-0.5f, h - size - 0.5f, w - 0.5f, h - 0.5f}, {0, 0, 0, 1});
		}
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
