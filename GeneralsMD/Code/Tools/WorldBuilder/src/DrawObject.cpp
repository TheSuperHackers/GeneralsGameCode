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

// This is used to allow sounds to be played via PlaySound
#include <mmsystem.h>

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
#include "TileTool.h"
#include "BuildListTool.h"
#include "LayersList.h"
#include "Common/WellKnownKeys.h"
#include "Common/BorderColors.h"
#include "Common/ThingTemplate.h"
#include "W3DDevice/Common/W3DConvert.h"
#include "render2d.h"
#include "GameLogic/Weapon.h"
#include "Common/AudioEventInfo.h"

#ifdef _DEBUG
#define NO_INTENSE_DEBUG 1
#endif

const Real LINE_THICKNESS = 2.0f;
const Real HANDLE_SIZE = (2.0f) * LINE_THICKNESS;
const Real LINE_THICKNESS_GRID = 1.5f;
#define ADJUST_FROM_INDEX_TO_REAL(k) ((k-pMap->getBorderSize())*MAP_XY_FACTOR)

#define MAX_LINE_RENDER_SAFE_LIMIT 20000 // If we hit this dont do render at all


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


int DrawObject::m_defaultIconColor = 0x00FFFF; // or whatever the default value is
int DrawObject::m_waypointIconColor = 0x00FF00;
int DrawObject::m_unitIconColor = 0xFF00FF;
int DrawObject::m_treeIconColor = 0x00FF00;
int DrawObject::m_roadIconColor = 0xFFFF00;

Bool DrawObject::m_squareFeedback = false;
Int	DrawObject::m_brushWidth = 3;
Int	DrawObject::m_brushFeatherWidth = 3;
Int DrawObject::m_brushHeight = 0;
Bool	DrawObject::m_toolWantsFeedback = true;
Bool	DrawObject::m_disableFeedback = false;
Bool	DrawObject::m_meshFeedback = false;
Bool	DrawObject::m_rampFeedback = false;
Bool	DrawObject::m_boundaryFeedback = false;
Bool	DrawObject::m_rulerGridFeedback = true;
Bool	DrawObject::m_showTracingOverlay = false;
Bool	DrawObject::m_ambientSoundFeedback = false;
Coord3D	DrawObject::m_feedbackPoint;
CPoint DrawObject::m_cellCenter;

Coord3D	DrawObject::m_rampStartPoint;
Coord3D	DrawObject::m_rampEndPoint;
Real DrawObject::m_rampWidth = 0.0f;


Bool DrawObject::m_dragWaypointFeedback = false; 
Coord3D DrawObject::m_dragWayStart;
Coord3D DrawObject::m_dragWayEnd;

Bool DrawObject::m_terrainPasteFeedback = false;
Coord3D DrawObject::m_terrainPasteCenter;
Int DrawObject::m_terrainPasteFeedbackRotation = 0;

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
	// m_roadIconColor     = 0xFFFF00; // yellow
	// m_unitIconColor     = 0xFF00FF; // pink
	// m_waypointIconColor = 0x00FF00; // green
	// m_defaultIconColor  = 0x00FFFF; // cyan

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
	if (m_lineRenderer) {
		delete m_lineRenderer;
		m_lineRenderer = NULL;
	}
	return 0;
}

// Total number of triangles // Responsible for the middle point
#define NUM_TRI 16	 // 18 is octagon // 26 is original
// Number of triangles in the arrow.
#define NUM_ARROW_TRI 4
// Number of triangles in the selection pyramid. // The selection circle
#define NUM_SELECT_TRI 6
// Height of selection pyramid.
#define SELECT_PYRAMID_HEIGHT (1.0f)

// // Total number of triangles
// #define NUM_TRI (3 + NUM_ARROW_TRI + NUM_SELECT_TRI)


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
	m_shaderClass = ShaderClass::ShaderClass(SC_OPAQUE);//_PresetOpaque2DShader;//ShaderClass(SC_OPAQUE); //_PresetOpaqueShader;

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

/** Returns the water height if the point is underwater, or -FLT_MAX if not */
Real getWaterHeightIfUnderwater(Real x, Real y)
{
    ICoord3D iLoc;
    iLoc.x = (floor(x + 0.5f));
    iLoc.y = (floor(y + 0.5f));
    iLoc.z = 0;

    for (PolygonTrigger *pTrig = PolygonTrigger::getFirstPolygonTrigger(); pTrig; pTrig = pTrig->getNext()) {
        if (!pTrig->isWaterArea()) {
            continue;
        }

        if (pTrig->pointInTrigger(iLoc)) {
            Real waterZ = pTrig->getPoint(0)->z;
            Real terrainZ = TheTerrainRenderObject->getHeightMapHeight(x, y, NULL);
            if (terrainZ < waterZ) {
                return waterZ;
            }
        }
    }

    return -FLT_MAX; // Not underwater
}

void DrawObject::updateBoundaryVB(void)
{
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

	const float stepSize = 10.0f * MAP_XY_FACTOR; // Adjust smoothness here

	for (Int i = 0; i < numBoundaries; ++i) {
		ICoord2D curBoundary;
		pDoc->getBoundary(i, &curBoundary);
		if (curBoundary.x == 0 || curBoundary.y == 0) {
			continue; // Skip defunct boundaries
		}

		// Define the 4 corner points of the boundary rectangle in order:
		// (0,0), (0, y), (x, y), (x, 0)
		Coord3D corners[4];
		// Corner 0: (0,0)
		corners[0].x = 0 * MAP_XY_FACTOR;
		corners[0].y = 0 * MAP_XY_FACTOR;
		corners[0].z = TheTerrainRenderObject->getHeightMapHeight(corners[0].x, corners[0].y, NULL);
		// Corner 1: (0, y)
		corners[1].x = 0 * MAP_XY_FACTOR;
		corners[1].y = curBoundary.y * MAP_XY_FACTOR;
		corners[1].z = TheTerrainRenderObject->getHeightMapHeight(corners[1].x, corners[1].y, NULL);
		// Corner 2: (x, y)
		corners[2].x = curBoundary.x * MAP_XY_FACTOR;
		corners[2].y = curBoundary.y * MAP_XY_FACTOR;
		corners[2].z = TheTerrainRenderObject->getHeightMapHeight(corners[2].x, corners[2].y, NULL);
		// Corner 3: (x, 0)
		corners[3].x = curBoundary.x * MAP_XY_FACTOR;
		corners[3].y = 0 * MAP_XY_FACTOR;
		corners[3].z = TheTerrainRenderObject->getHeightMapHeight(corners[3].x, corners[3].y, NULL);

		// Loop over edges (corner j → corner (j+1)%4)
		for (int j = 0; j < 4; ++j) {
			Coord3D startPt = corners[j];
			Coord3D endPt = corners[(j + 1) % 4];

			Vector3 edgeVec(endPt.x - startPt.x, endPt.y - startPt.y, 0);
			float edgeLength = sqrtf(edgeVec.X * edgeVec.X + edgeVec.Y * edgeVec.Y);
			int segments = max(1, (int)(edgeLength / stepSize));


			for (int s = 0; s < segments; ++s) {
				float t1 = (float)s / segments;
				float t2 = (float)(s + 1) / segments;

				Coord3D p1, p2;
				p1.x = startPt.x + (endPt.x - startPt.x) * t1;
				p1.y = startPt.y + (endPt.y - startPt.y) * t1;
				p1.z = TheTerrainRenderObject->getHeightMapHeight(p1.x, p1.y, NULL);
				if(m_showWater) {
					Real waterHeight1 = getWaterHeightIfUnderwater(p1.x, p1.y);
					if (waterHeight1 != -FLT_MAX) {
						p1.z = waterHeight1 + 4.5f; // Draw slightly above water
					}
				}
				p2.x = startPt.x + (endPt.x - startPt.x) * t2;
				p2.y = startPt.y + (endPt.y - startPt.y) * t2;
				p2.z = TheTerrainRenderObject->getHeightMapHeight(p2.x, p2.y, NULL);
				if(m_showWater) {
					Real waterHeight2 = getWaterHeightIfUnderwater(p2.x, p2.y);
					if (waterHeight2 != -FLT_MAX) {
						p2.z = waterHeight2 + 4.5f;
					}
				}
				// Calculate perpendicular normal for thickness
				Vector3 dir(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
				dir.Normalize();
				dir *= LINE_THICKNESS;
				dir.Rotate_Z(PI / 2);

				// Check buffer capacity
				if (m_feedbackVertexCount + 4 > NUM_FEEDBACK_VERTEX || m_feedbackIndexCount + 6 > NUM_FEEDBACK_INDEX)
					return;

				DWORD color = BORDER_COLORS[i % BORDER_COLORS_SIZE].m_borderColor;

				#define ADD_VERT(px, py, pz) \
					curVb->x = px; curVb->y = py; curVb->z = pz; \
					curVb->u1 = 0; curVb->v1 = 0; curVb->diffuse = color; ++curVb; ++m_feedbackVertexCount;

				ADD_VERT(p1.x + dir.X, p1.y + dir.Y, p1.z);
				ADD_VERT(p1.x - dir.X, p1.y - dir.Y, p1.z);
				ADD_VERT(p2.x + dir.X, p2.y + dir.Y, p2.z);
				ADD_VERT(p2.x - dir.X, p2.y - dir.Y, p2.z);

				*curIb++ = m_feedbackVertexCount - 4;
				*curIb++ = m_feedbackVertexCount - 2;
				*curIb++ = m_feedbackVertexCount - 3;
				*curIb++ = m_feedbackVertexCount - 4;
				*curIb++ = m_feedbackVertexCount - 1;
				*curIb++ = m_feedbackVertexCount - 2;
				m_feedbackIndexCount += 6;
			}
			
		}

		// Optional: You can still draw the handles ("little nuggets") here if needed,
		// but now the edges follow terrain better.

	}
}

void DrawObject::updateGridVB(void)
{
	m_feedbackVertexCount = 0;
	m_feedbackIndexCount = 0;

	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexFeedback, D3DLOCK_DISCARD);
	UnsignedShort *ib = lockIdxBuffer.Get_Index_Array();
	UnsignedShort *curIb = ib;

	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexFeedback, D3DLOCK_DISCARD);
	VertexFormatXYZDUV1 *vb = (VertexFormatXYZDUV1 *)lockVtxBuffer.Get_Vertex_Array();
	VertexFormatXYZDUV1 *curVb = vb;

	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
	WorldHeightMapEdit *pMap = pDoc->GetHeightMap();

	const float stepSize = 10.0f * MAP_XY_FACTOR;

	// #define ADJUST_FROM_INDEX_TO_REAL(k) ((k - pMap->getBorderSize()) * MAP_XY_FACTOR)
	const float worldX0 = ADJUST_FROM_INDEX_TO_REAL(1);
	const float worldY0 = ADJUST_FROM_INDEX_TO_REAL(1);
	const float worldX1 = ADJUST_FROM_INDEX_TO_REAL(pMap->getXExtent() - 2);
	const float worldY1 = ADJUST_FROM_INDEX_TO_REAL(pMap->getYExtent() - 2);

	const int targetLines = 10;

	float mapWidth  = worldX1 - worldX0;
	float mapHeight = worldY1 - worldY0;

	float spacingX = mapWidth / targetLines;
	float spacingY = mapHeight / targetLines;

	const float Z_OFFSET = 1.0f;
	const float centerX = (worldX0 + worldX1) * 0.5f;
	const float centerY = (worldY0 + worldY1) * 0.5f;
	const float epsilon = 0.1f;

	#define ADD_GRID_VERT(px, py, pz, clr) \
		curVb->x = px; curVb->y = py; curVb->z = pz; \
		curVb->u1 = 0; curVb->v1 = 0; curVb->diffuse = clr; ++curVb; ++m_feedbackVertexCount;

	int i;

	// --- Draw Concentric Circles (auto-scaled to map size) ---
	const int NUM_CIRCLES = 3;
	const int SEGMENTS_PER_CIRCLE = 96; // smooth circles

	// use smaller dimension to fit within map bounds
	float baseRadius = min(mapWidth, mapHeight) * 0.5f * 0.8f; // 80% of half-size
	float circleRadii[NUM_CIRCLES];

	// distribute evenly: inner = 1/3, mid = 2/3, outer = full
	for (int ca = 0; ca < NUM_CIRCLES; ++ca)
		circleRadii[ca] = baseRadius * ((ca + 1) / (float)NUM_CIRCLES);

	for (int c = 0; c < NUM_CIRCLES; ++c)
	{
		float radius = circleRadii[c];
		DWORD color;
		if (c == 0) color = 0xFF00FFFF;    // inner - cyan
		else if (c == 1) color = 0xFFFFFF00; // middle - yellow
		else color = 0xFFFF00FF;             // outer - magenta

		for (int s = 0; s < SEGMENTS_PER_CIRCLE; ++s)
		{
			float angle1 = (s / (float)SEGMENTS_PER_CIRCLE) * 2.0f * PI;
			float angle2 = ((s + 1) / (float)SEGMENTS_PER_CIRCLE) * 2.0f * PI;

			float x1 = centerX + cosf(angle1) * radius;
			float y1 = centerY + sinf(angle1) * radius;
			float x2 = centerX + cosf(angle2) * radius;
			float y2 = centerY + sinf(angle2) * radius;

			Real z1 = TheTerrainRenderObject->getHeightMapHeight(x1, y1, NULL);
			Real z2 = TheTerrainRenderObject->getHeightMapHeight(x2, y2, NULL);
			Real waterZ1 = getWaterHeightIfUnderwater(x1, y1);
			Real waterZ2 = getWaterHeightIfUnderwater(x2, y2);

			if (waterZ1 != -FLT_MAX && waterZ1 + 4.5f > z1 + Z_OFFSET)
				z1 = waterZ1 + 4.5f;
			else
				z1 += Z_OFFSET;

			if (waterZ2 != -FLT_MAX && waterZ2 + 4.5f > z2 + Z_OFFSET)
				z2 = waterZ2 + 4.5f;
			else
				z2 += Z_OFFSET;

			Coord3D a = { x1, y1, z1 };
			Coord3D b = { x2, y2, z2 };

			Vector3 dir(b.x - a.x, b.y - a.y, b.z - a.z);
			dir.Normalize();
			dir *= LINE_THICKNESS_GRID;
			dir.Rotate_Z(PI / 2);

			if (m_feedbackVertexCount + 4 > NUM_FEEDBACK_VERTEX || m_feedbackIndexCount + 6 > NUM_FEEDBACK_INDEX)
				return;

			ADD_GRID_VERT(a.x + dir.X, a.y + dir.Y, a.z, color);
			ADD_GRID_VERT(a.x - dir.X, a.y - dir.Y, a.z, color);
			ADD_GRID_VERT(b.x + dir.X, b.y + dir.Y, b.z, color);
			ADD_GRID_VERT(b.x - dir.X, b.y - dir.Y, b.z, color);

			*curIb++ = m_feedbackVertexCount - 4; // v0
			*curIb++ = m_feedbackVertexCount - 3; // v1
			*curIb++ = m_feedbackVertexCount - 2; // v2
			*curIb++ = m_feedbackVertexCount - 2; // v2
			*curIb++ = m_feedbackVertexCount - 3; // v1
			*curIb++ = m_feedbackVertexCount - 1; // v3
			m_feedbackIndexCount += 6;
		}
	}

	// Vertical lines
	for (i = 0;; ++i) {
		bool didDraw = false;
		float x1 = centerX + i * spacingX;
		float x2 = centerX - i * spacingX;

		if (x1 <= worldX1 + 0.001f) {
			DWORD color = (fabs(x1 - centerX) < epsilon) ? 0xFFFF0000 : 0xFF808080;
			float y;
			for (y = worldY0; y < worldY1; y += stepSize) {
				float nextY = y + stepSize;
				if (nextY > worldY1) nextY = worldY1;

				Real z1 = TheTerrainRenderObject->getHeightMapHeight(x1, y, NULL);
				Real waterZ1 = getWaterHeightIfUnderwater(x1, y);
				if (waterZ1 != -FLT_MAX && waterZ1 + 4.5f > z1 + Z_OFFSET)
					z1 = waterZ1 + 4.5f;
				else
					z1 += Z_OFFSET;

				Real z2 = TheTerrainRenderObject->getHeightMapHeight(x1, nextY, NULL);
				Real waterZ2 = getWaterHeightIfUnderwater(x1, nextY);
				if (waterZ2 != -FLT_MAX && waterZ2 + 4.5f > z2 + Z_OFFSET)
					z2 = waterZ2 + 4.5f;
				else
					z2 += Z_OFFSET;

				Coord3D a = { x1, y, z1 };
				Coord3D b = { x1, nextY, z2 };

				Vector3 dir(LINE_THICKNESS_GRID, 0, 0); // Extrude in +X and -X

				if (m_feedbackVertexCount + 4 > NUM_FEEDBACK_VERTEX || m_feedbackIndexCount + 6 > NUM_FEEDBACK_INDEX)
					return;

				ADD_GRID_VERT(a.x + dir.X, a.y + dir.Y, a.z, color);
				ADD_GRID_VERT(a.x - dir.X, a.y - dir.Y, a.z, color);
				ADD_GRID_VERT(b.x + dir.X, b.y + dir.Y, b.z, color);
				ADD_GRID_VERT(b.x - dir.X, b.y - dir.Y, b.z, color);

				*curIb++ = m_feedbackVertexCount - 4; // v0
				*curIb++ = m_feedbackVertexCount - 3; // v1
				*curIb++ = m_feedbackVertexCount - 2; // v2

				*curIb++ = m_feedbackVertexCount - 2; // v2
				*curIb++ = m_feedbackVertexCount - 3; // v1
				*curIb++ = m_feedbackVertexCount - 1; // v3
				m_feedbackIndexCount += 6;
			}
			didDraw = true;
		}

		if (i != 0 && x2 >= worldX0 - 0.001f) {
			DWORD color = (fabs(x2 - centerX) < epsilon) ? 0xFFFF0000 : 0xFF808080;
			float y;
			for (y = worldY0; y < worldY1; y += stepSize) {
				float nextY = y + stepSize;
				if (nextY > worldY1) nextY = worldY1;

				Real z1 = TheTerrainRenderObject->getHeightMapHeight(x2, y, NULL);
				Real waterZ1 = getWaterHeightIfUnderwater(x2, y);
				if (waterZ1 != -FLT_MAX && waterZ1 + 4.5f > z1 + Z_OFFSET)
					z1 = waterZ1 + 4.5f;
				else
					z1 += Z_OFFSET;

				Real z2 = TheTerrainRenderObject->getHeightMapHeight(x2, nextY, NULL);
				Real waterZ2 = getWaterHeightIfUnderwater(x2, nextY);
				if (waterZ2 != -FLT_MAX && waterZ2 + 4.5f > z2 + Z_OFFSET)
					z2 = waterZ2 + 4.5f;
				else
					z2 += Z_OFFSET;

				Coord3D a = { x2, y, z1 };
				Coord3D b = { x2, nextY, z2 };

				Vector3 dir(b.x - a.x, b.y - a.y, b.z - a.z);
				dir.Normalize();
				dir *= LINE_THICKNESS_GRID;
				dir.Rotate_Z(PI / 2);

				if (m_feedbackVertexCount + 4 > NUM_FEEDBACK_VERTEX || m_feedbackIndexCount + 6 > NUM_FEEDBACK_INDEX)
					return;

				ADD_GRID_VERT(a.x + dir.X, a.y + dir.Y, a.z, color);
				ADD_GRID_VERT(a.x - dir.X, a.y - dir.Y, a.z, color);
				ADD_GRID_VERT(b.x + dir.X, b.y + dir.Y, b.z, color);
				ADD_GRID_VERT(b.x - dir.X, b.y - dir.Y, b.z, color);

				*curIb++ = m_feedbackVertexCount - 4; // v0
				*curIb++ = m_feedbackVertexCount - 3; // v1
				*curIb++ = m_feedbackVertexCount - 2; // v2

				*curIb++ = m_feedbackVertexCount - 2; // v2
				*curIb++ = m_feedbackVertexCount - 3; // v1
				*curIb++ = m_feedbackVertexCount - 1; // v3
				m_feedbackIndexCount += 6;
			}
			didDraw = true;
		}

		if (!didDraw)
			break;
	}

	// Horizontal lines
	for (i = 0;; ++i) {
		bool didDraw = false;
		float y1 = centerY + i * spacingY;
		float y2 = centerY - i * spacingY;

		if (y1 <= worldY1 + 0.001f) {
			DWORD color = (fabs(y1 - centerY) < epsilon) ? 0xFFFF0000 : 0xFF808080;
			float x;
			for (x = worldX0; x < worldX1; x += stepSize) {
				float nextX = x + stepSize;
				if (nextX > worldX1) nextX = worldX1;

				Real z1 = TheTerrainRenderObject->getHeightMapHeight(x, y1, NULL);
				Real waterZ1 = getWaterHeightIfUnderwater(x, y1);
				if (waterZ1 != -FLT_MAX && waterZ1 + 4.5f > z1 + Z_OFFSET)
					z1 = waterZ1 + 4.5f;
				else
					z1 += Z_OFFSET;

				Real z2 = TheTerrainRenderObject->getHeightMapHeight(nextX, y1, NULL);
				Real waterZ2 = getWaterHeightIfUnderwater(nextX, y1);
				if (waterZ2 != -FLT_MAX && waterZ2 + 4.5f > z2 + Z_OFFSET)
					z2 = waterZ2 + 4.5f;
				else
					z2 += Z_OFFSET;

				Coord3D a = { x, y1, z1 };
				Coord3D b = { nextX, y1, z2 };

				Vector3 dir(0, LINE_THICKNESS_GRID, 0); // Extrude in +Y and -Y

				if (m_feedbackVertexCount + 4 > NUM_FEEDBACK_VERTEX || m_feedbackIndexCount + 6 > NUM_FEEDBACK_INDEX)
					return;

				ADD_GRID_VERT(a.x + dir.X, a.y + dir.Y, a.z, color);
				ADD_GRID_VERT(a.x - dir.X, a.y - dir.Y, a.z, color);
				ADD_GRID_VERT(b.x + dir.X, b.y + dir.Y, b.z, color);
				ADD_GRID_VERT(b.x - dir.X, b.y - dir.Y, b.z, color);

				*curIb++ = m_feedbackVertexCount - 4; // v0
				*curIb++ = m_feedbackVertexCount - 3; // v1
				*curIb++ = m_feedbackVertexCount - 2; // v2

				*curIb++ = m_feedbackVertexCount - 2; // v2
				*curIb++ = m_feedbackVertexCount - 3; // v1
				*curIb++ = m_feedbackVertexCount - 1; // v3
				m_feedbackIndexCount += 6;
			}
			didDraw = true;
		}

		if (i != 0 && y2 >= worldY0 - 0.001f) {
			DWORD color = (fabs(y2 - centerY) < epsilon) ? 0xFFFF0000 : 0xFF808080;
			float x;
			for (x = worldX0; x < worldX1; x += stepSize) {
				float nextX = x + stepSize;
				if (nextX > worldX1) nextX = worldX1;

				Real z1 = TheTerrainRenderObject->getHeightMapHeight(x, y2, NULL);
				Real waterZ1 = getWaterHeightIfUnderwater(x, y2);
				if (waterZ1 != -FLT_MAX && waterZ1 + 4.5f > z1 + Z_OFFSET)
					z1 = waterZ1 + 4.5f;
				else
					z1 += Z_OFFSET;

				Real z2 = TheTerrainRenderObject->getHeightMapHeight(nextX, y2, NULL);
				Real waterZ2 = getWaterHeightIfUnderwater(nextX, y2);
				if (waterZ2 != -FLT_MAX && waterZ2 + 4.5f > z2 + Z_OFFSET)
					z2 = waterZ2 + 4.5f;
				else
					z2 += Z_OFFSET;

				Coord3D a = { x, y2, z1 };
				Coord3D b = { nextX, y2, z2 };

				Vector3 dir(b.x - a.x, b.y - a.y, b.z - a.z);
				dir.Normalize();
				dir *= LINE_THICKNESS_GRID;
				dir.Rotate_Z(PI / 2);

				if (m_feedbackVertexCount + 4 > NUM_FEEDBACK_VERTEX || m_feedbackIndexCount + 6 > NUM_FEEDBACK_INDEX)
					return;

				ADD_GRID_VERT(a.x + dir.X, a.y + dir.Y, a.z, color);
				ADD_GRID_VERT(a.x - dir.X, a.y - dir.Y, a.z, color);
				ADD_GRID_VERT(b.x + dir.X, b.y + dir.Y, b.z, color);
				ADD_GRID_VERT(b.x - dir.X, b.y - dir.Y, b.z, color);

				*curIb++ = m_feedbackVertexCount - 4; // v0
				*curIb++ = m_feedbackVertexCount - 3; // v1
				*curIb++ = m_feedbackVertexCount - 2; // v2

				*curIb++ = m_feedbackVertexCount - 2; // v2
				*curIb++ = m_feedbackVertexCount - 3; // v1
				*curIb++ = m_feedbackVertexCount - 1; // v3
				m_feedbackIndexCount += 6;
			}
			didDraw = true;
		}

		if (!didDraw)
			break;
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

void DrawObject::updateWaypointVB(RenderInfoClass & rinfo)
{
	m_feedbackVertexCount = 0;
	m_feedbackIndexCount = 0;
	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexFeedback, D3DLOCK_DISCARD);
	UnsignedShort *ib = lockIdxBuffer.Get_Index_Array();
	UnsignedShort *curIb = ib;

	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexFeedback, D3DLOCK_DISCARD);
	VertexFormatXYZDUV1 *vb = (VertexFormatXYZDUV1*)lockVtxBuffer.Get_Vertex_Array();
	VertexFormatXYZDUV1 *curVb = vb;

	CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();

	// Map group label to color
	std::map<AsciiString, uint32> groupColorMap;
	int colorSeed = 0;

	for (Int i = 0; i <= pDoc->getNumWaypointLinks(); i++) {
		Bool gotLocation = false;
		Coord3D loc1;
		Coord3D loc2;
		Bool exists;
		Int waypointID1, waypointID2;

		for (Int k = 0; k < 2; k++) {
			Bool ok = false;
			pDoc->getWaypointLink(i, &waypointID1, &waypointID2);
			if (k == 0 || i == pDoc->getNumWaypointLinks()) {
				ok = (k == 0);
			} else {
				MapObject *pWay = pDoc->getWaypointByID(waypointID1);
				if (pWay) {
					Bool biDirectional = pWay->getProperties()->getBool(TheKey_waypointPathBiDirectional, &exists);
					if (biDirectional) {
						ok = true;
						pDoc->getWaypointLink(i, &waypointID2, &waypointID1);
					}
				}
			}

			if (i == pDoc->getNumWaypointLinks()) {
				if (m_dragWaypointFeedback) {
					loc1 = m_dragWayStart;
					loc2 = m_dragWayEnd;
					gotLocation = true;
				}
			} else {
				MapObject *pWay1 = pDoc->getWaypointByID(waypointID1);
				MapObject *pWay2 = pDoc->getWaypointByID(waypointID2);
				if (pWay1 && pWay2) {
					gotLocation = true;
					loc1 = *pWay1->getLocation();
					loc2 = *pWay2->getLocation();
					AsciiString wayLayer;

					wayLayer = pWay1->getProperties()->getAsciiString(TheKey_objectLayer, &exists);
					if (exists && TheLayersList->isLayerHidden(wayLayer)) gotLocation = false;

					wayLayer = pWay2->getProperties()->getAsciiString(TheKey_objectLayer, &exists);
					if (exists && TheLayersList->isLayerHidden(wayLayer)) gotLocation = false;
				}
			}

			// if (gotLocation) {
				//
                // ✅ Cull the waypoint segment before adding vertices
                //
                // Vector3 center(
                //     (loc1.x + loc2.x) * 0.5f,
                //     (loc1.y + loc2.y) * 0.5f,
                //     (loc1.z + loc2.z) * 0.5f
                // );
                // float radius = sqrtf(
                //     (loc2.x - loc1.x) * (loc2.x - loc1.x) +
                //     (loc2.y - loc1.y) * (loc2.y - loc1.y) +
                //     (loc2.z - loc1.z) * (loc2.z - loc1.z)
                // ) * 0.5f;

                // SphereClass bounds(center, radius);
                // if (rinfo.Camera.Cull_Sphere(bounds)) {
                //     continue; // completely outside view, skip this segment
                // }
			if (gotLocation) {
				// === GROUP COLOR SECTION ===
				AsciiString groupLabel = "default";
				MapObject* pWay1 = pDoc->getWaypointByID(waypointID1);
				if (pWay1) {
					groupLabel = pWay1->getProperties()->getAsciiString(TheKey_waypointPathLabel1, &exists);
					if (!exists) groupLabel = "default";
				}

				uint32 groupColor = 0xFF00FF00; // default green

				if (m_useFixedColoredWaypoints) {
					AsciiString labelLower = groupLabel;
					labelLower.toLower(); // assuming AsciiString has toLower(), else write helper

					if (labelLower.startsWith("flank"))
						groupColor = 0xFFFFFF00; // yellow
					else if (labelLower.startsWith("center"))
						groupColor = 0xFFFF6666; // red
					else if (labelLower.startsWith("backdoor"))
						groupColor = 0xFF00FFFF; // cyan
					else if (labelLower.startsWith("special"))
						groupColor = 0xFFCC66FF; // softer violet
					else
						groupColor = 0xFF00FF00; // fallback green
				}
				else {
					if (groupLabel == "default") {
						groupColor = 0xFF00FF00;
						groupColorMap[groupLabel] = groupColor;
					} else if (groupColorMap.find(groupLabel) == groupColorMap.end()) {
						uint32 hue = (colorSeed * 137) % 360;
						float s = 0.6f;
						float v = 0.95f;

						float c = v * s;
						float x = c * (1 - fabs(fmod(hue / 60.0f, 2) - 1));
						float m = v - c;

						float rf, gf, bf;
						if (hue < 60) { rf = c; gf = x; bf = 0; }
						else if (hue < 120) { rf = x; gf = c; bf = 0; }
						else if (hue < 180) { rf = 0; gf = c; bf = x; }
						else if (hue < 240) { rf = 0; gf = x; bf = c; }
						else if (hue < 300) { rf = x; gf = 0; bf = c; }
						else { rf = c; gf = 0; bf = x; }

						uint32 r = static_cast<uint32>((rf + m) * 255);
						uint32 g = static_cast<uint32>((gf + m) * 255);
						uint32 b = static_cast<uint32>((bf + m) * 255);

						groupColor = (0xFF << 24) | (r << 16) | (g << 8) | b;
						groupColorMap[groupLabel] = groupColor;
						colorSeed++;
					} else {
						groupColor = groupColorMap[groupLabel];
					}
				}
				// === END GROUP COLOR SECTION ===
								

				Vector3 normal(loc2.x - loc1.x, loc2.y - loc1.y, loc2.z - loc1.z);
				normal.Normalize();
				normal *= 0.5f;
				normal.Rotate_Z(PI / 2);
				loc1.z = TheTerrainRenderObject->getHeightMapHeight(loc1.x, loc1.y, NULL);
				loc2.z = TheTerrainRenderObject->getHeightMapHeight(loc2.x, loc2.y, NULL);

				if (m_feedbackVertexCount + 9 >= NUM_FEEDBACK_VERTEX) return;

				// Stem vertices - use group color
				curVb->u1 = 0; curVb->v1 = 0;
				curVb->x = loc1.x + normal.X; curVb->y = loc1.y + normal.Y; curVb->z = loc1.z;
				curVb->diffuse = groupColor; curVb++; m_feedbackVertexCount++;

				curVb->u1 = 0; curVb->v1 = 0;
				curVb->x = loc1.x - normal.X; curVb->y = loc1.y - normal.Y; curVb->z = loc1.z;
				curVb->diffuse = groupColor; curVb++; m_feedbackVertexCount++;

				// End vertices - stay red
				curVb->u1 = 0; curVb->v1 = 0;
				curVb->x = loc2.x + normal.X; curVb->y = loc2.y + normal.Y; curVb->z = loc2.z;
				curVb->diffuse = groupColor; curVb++; m_feedbackVertexCount++;

				curVb->u1 = 0; curVb->v1 = 0;
				curVb->x = loc2.x - normal.X; curVb->y = loc2.y - normal.Y; curVb->z = loc2.z;
				curVb->diffuse = groupColor; curVb++; m_feedbackVertexCount++;

				if (m_feedbackIndexCount + 12 >= NUM_FEEDBACK_INDEX) return;

				*curIb++ = m_feedbackVertexCount - 3;
				*curIb++ = m_feedbackVertexCount - 1;
				*curIb++ = m_feedbackVertexCount - 2;
				*curIb++ = m_feedbackVertexCount - 4;
				*curIb++ = m_feedbackVertexCount - 3;
				*curIb++ = m_feedbackVertexCount - 2;
				m_feedbackIndexCount += 6;

				// Arrowhead
				Vector3 vec(loc2.x - loc1.x, loc2.y - loc1.y, loc2.z - loc1.z);
				vec.Normalize();
				const Real ARROWHEAD_LEN = 16.0f;
				const Real NORMAL_SHIFT = 10.0f;
				vec *= ARROWHEAD_LEN;
				Coord3D arrowBase;
				arrowBase.x = loc2.x - vec.X;
				arrowBase.y = loc2.y - vec.Y;
				arrowBase.z = loc2.z - vec.Z;

				if (m_feedbackVertexCount + 3 >= NUM_FEEDBACK_VERTEX) return;

				// Arrow base vertex 1
				curVb->u1 = 0; curVb->v1 = 0;
				curVb->x = arrowBase.x + NORMAL_SHIFT * normal.X;
				curVb->y = arrowBase.y + NORMAL_SHIFT * normal.Y;
				curVb->z = arrowBase.z;
				curVb->diffuse = groupColor; curVb++; m_feedbackVertexCount++;

				// Arrow base vertex 2
				curVb->u1 = 0; curVb->v1 = 0;
				curVb->x = arrowBase.x - NORMAL_SHIFT * normal.X;
				curVb->y = arrowBase.y - NORMAL_SHIFT * normal.Y;
				curVb->z = arrowBase.z;
				curVb->diffuse = groupColor; curVb++; m_feedbackVertexCount++;

				// Arrow tip vertex
				curVb->u1 = 0; curVb->v1 = 0;
				curVb->x = loc2.x; curVb->y = loc2.y; curVb->z = loc2.z;
				curVb->diffuse = groupColor; curVb++; m_feedbackVertexCount++;

				if (m_feedbackIndexCount + 3 >= NUM_FEEDBACK_INDEX) return;

				*curIb++ = m_feedbackVertexCount - 3;
				*curIb++ = m_feedbackVertexCount - 2;
				*curIb++ = m_feedbackVertexCount - 1;
				m_feedbackIndexCount += 3;
			}
		}
	}
}

/** updateMeshVB puts polygon trigger triangles into m_vertexFeedback. */

/** updateMeshVB puts polygon trigger triangles into m_vertexFeedback. */
void DrawObject::updatePolygonVB(PolygonTrigger *pTrig, Bool selected, Bool isOpen)
{
	Int green = 0;
	if (selected) {
		green = (255 * curHighlight) / (NUM_HIGHLIGHT - 1);
	}
	green = green << 8;
	m_feedbackVertexCount = 0;
	m_feedbackIndexCount = 0;

	DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexFeedback, D3DLOCK_DISCARD);
	UnsignedShort *ib = lockIdxBuffer.Get_Index_Array();
	UnsignedShort *curIb = ib;

	DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexFeedback, D3DLOCK_DISCARD);
	VertexFormatXYZDUV1 *vb = (VertexFormatXYZDUV1*)lockVtxBuffer.Get_Vertex_Array();
	VertexFormatXYZDUV1 *curVb = vb;

	AsciiString triggerName = pTrig->getTriggerName();
	// DEBUG_LOG(("triggername %s\n", triggerName.str()));

	// Determine base color depending on trigger name
	unsigned int baseColor = 0xFFFF0000; // default red
	const char *tstr = triggerName.str();
	if (_strnicmp(tstr, "inner", 5) == 0) {
		// e.g. InnerPerimeter*
		baseColor = 0xFFFF0000; // red
	}
	else if (_strnicmp(tstr, "outer", 5) == 0) {
		// e.g. OuterPerimeter*
		baseColor = 0xFF00FF00; // green
	}
	else if (_strnicmp(tstr, "combat", 6) == 0) {
		// e.g. CombatZone*
		baseColor = 0xFFFFFF00; // yellow (A=FF,R=FF,G=FF,B=00)
	}
	else if (pTrig->isWaterArea()) {
		baseColor = 0xFF0000FF; // blue for water
	}
	
	for (Int i = 0; i < pTrig->getNumPoints(); i++) {
		Coord3D loc1;
		Coord3D loc2;
		ICoord3D iLoc = *pTrig->getPoint(i);
		loc1.x = iLoc.x;
		loc1.y = iLoc.y;
		loc1.z = TheTerrainRenderObject->getHeightMapHeight(loc1.x, loc1.y, NULL);

		if (i < pTrig->getNumPoints() - 1) {
			iLoc = *pTrig->getPoint(i + 1);
		} else {
			if (isOpen) break;
			iLoc = *pTrig->getPoint(0);
		}

		loc2.x = iLoc.x;
		loc2.y = iLoc.y;
		loc2.z = TheTerrainRenderObject->getHeightMapHeight(loc2.x, loc2.y, NULL);

		Vector3 normal(loc2.x - loc1.x, loc2.y - loc1.y, loc2.z - loc1.z);
		normal.Normalize();
		normal *= 0.5f;
		normal.Rotate_Z(PI / 2);

		if (m_feedbackVertexCount + 9 >= NUM_FEEDBACK_VERTEX) {
			return;
		}

		Int diffuse = baseColor + green;

		// First vertex
		curVb->u1 = 0; curVb->v1 = 0;
		curVb->x = loc1.x + normal.X;
		curVb->y = loc1.y + normal.Y;
		curVb->z = loc1.z;
		curVb->diffuse = diffuse;
		curVb++; m_feedbackVertexCount++;

		// Second vertex
		curVb->u1 = 0; curVb->v1 = 0;
		curVb->x = loc1.x - normal.X;
		curVb->y = loc1.y - normal.Y;
		curVb->z = loc1.z;
		curVb->diffuse = diffuse;
		curVb++; m_feedbackVertexCount++;

		// Third vertex
		curVb->u1 = 0; curVb->v1 = 0;
		curVb->x = loc2.x + normal.X;
		curVb->y = loc2.y + normal.Y;
		curVb->z = loc2.z;
		curVb->diffuse = diffuse;
		curVb++; m_feedbackVertexCount++;

		// Fourth vertex
		curVb->u1 = 0; curVb->v1 = 0;
		curVb->x = loc2.x - normal.X;
		curVb->y = loc2.y - normal.Y;
		curVb->z = loc2.z;
		curVb->diffuse = diffuse;
		curVb++; m_feedbackVertexCount++;

		if (m_feedbackIndexCount + 12 >= NUM_FEEDBACK_INDEX) {
			return;
		}

		*curIb++ = m_feedbackVertexCount - 3;
		*curIb++ = m_feedbackVertexCount - 1;
		*curIb++ = m_feedbackVertexCount - 2;
		*curIb++ = m_feedbackVertexCount - 4;
		*curIb++ = m_feedbackVertexCount - 3;
		*curIb++ = m_feedbackVertexCount - 2;
		m_feedbackIndexCount += 6;
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

Int DrawObject::updateVB(DX8VertexBufferClass	*pVB, Int color, Bool doArrow, Bool doDiamond, Bool disableColoring)
{
	Int i, k;

	// Real factor = TheGlobalData->m_terrainAmbient[0].red +
	// 			  TheGlobalData->m_terrainAmbient[0].green +
	// 		      TheGlobalData->m_terrainAmbient[0].blue;
	// if (factor > 1.0f) factor = 1.0f;

	Real factor = 1.0f;
	
	Int r = color&0xFF;
	Int g = (color&0x00FF00)>>8;
	Int b = (color&0xFF0000)>>16;

	r *= factor;
	g *= factor;
	b *= factor;


	// r = 0;
	// g = 255;
	// b = 255; // cyan
	// r = 255;
	// g = 105;
	// b = 180; // hot pink!
	const Int theAlpha = 127;

	Int highlightColors[NUM_HIGHLIGHT] = {
		(255<<8) + (255<<16),
		(255<<16),
		(255<<8)
	};

	if(disableColoring){
		highlightColors[0] = (255) + (255<<8) + (255<<16) + (255<<24); // White
		highlightColors[1] = (255) + (255<<8) + (255<<16) + (255<<24); // White
		highlightColors[2] = (255) + (255<<8) + (255<<16) + (255<<24); // White
	}
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

	unsigned long color = 0xFFFFFF00; // Yellow
	
	GeometryInfo ginfo = pMapObj->getThingTemplate()->getTemplateGeometryInfo();

	// Bool isSmall =  ginfo.getIsSmall() || pMapObj->getThingTemplate()->isKindOf(KINDOF_LOW_OVERLAPPABLE) || pMapObj->getThingTemplate()->isKindOf(KINDOF_STRUCTURE) ;
	Bool isSmall =  !pMapObj->getThingTemplate()->isKindOf(KINDOF_STRUCTURE);
		
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
			Real exc = ginfo.getMajorRadius() * c;
			Real eyc = ginfo.getMinorRadius() * c;
			Real exs = ginfo.getMajorRadius() * s;
			Real eys = ginfo.getMinorRadius() * s;

			// Original 4 corners (base)
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
		
			// Margin size for outer box
			const Real margin = 18.0f;
		
			// Expanded corners for the green box
			Coord3D expandedPts[4];
			Coord3D center = pos;
		
			for (int i = 0; i < 4; ++i) {
				Coord3D dir;
				dir.x = pts[i].x - center.x;
				dir.y = pts[i].y - center.y;
				Real len = (Real)sqrt(dir.x * dir.x + dir.y * dir.y);
				if (len != 0) {
					dir.x /= len;
					dir.y /= len;
				}
				expandedPts[i].x = pts[i].x + dir.x * margin;
				expandedPts[i].y = pts[i].y + dir.y * margin;
				expandedPts[i].z = 0;
			}
		
			const Real cutSize = 10.0f; // Fixed size of beveled corner cut in world units

			for (int boxLayer = 0; boxLayer < 2; ++boxLayer) {
				if (boxLayer == 1 && isSmall)
					continue; // Skip green box for small objects

				unsigned long lineColor = (boxLayer == 0) ? 0xFFFFFF00 : 0xFF00C8C8; // Yellow or Green
				Real z = pos.z;

				for (int heightStep = 0; heightStep < 2; ++heightStep) {
					if (boxLayer == 1 && heightStep == 1)
						continue; // Skip top layer for green box

					if (boxLayer == 0) {
						// Draw purple box normally
						for (int corner = 0; corner < 4; ++corner) {
							int next = (corner + 1) & 3;
							pts[corner].z = z;
							pts[next].z = z;

							ICoord2D start, end;
							if (worldToScreen(&pts[corner], &start, camera) &&
								worldToScreen(&pts[next], &end, camera)) {
								m_lineRenderer->Add_Line(Vector2(start.x, start.y), Vector2(end.x, end.y), BOUNDING_BOX_LINE_WIDTH, lineColor);
							}
						}
					} else {
						// Draw green box with beveled corners (bottom layer only)
						Coord3D trimmed[4][2]; // [corner][0=prev bevel, 1=next bevel]

						// Draw main trimmed sides
						for (int corner = 0; corner < 4; ++corner) {
							int next = (corner + 1) & 3;

							Coord3D start3D = expandedPts[corner];
							Coord3D end3D = expandedPts[next];

							// Compute direction vector
							Coord3D dir;
							dir.x = end3D.x - start3D.x;
							dir.y = end3D.y - start3D.y;
							dir.z = 0;

							// Normalize
							Real len = sqrt(dir.x * dir.x + dir.y * dir.y);
							if (len < cutSize * 2.0f) {
								// Too short to bevel properly, skip beveling
								dir.x = dir.y = 0.0f;
							} else {
								dir.x /= len;
								dir.y /= len;
							}

							// Compute trimmed ends
							Coord3D trimmedStart, trimmedEnd;
							trimmedStart.x = start3D.x + dir.x * cutSize;
							trimmedStart.y = start3D.y + dir.y * cutSize;
							trimmedStart.z = z;

							trimmedEnd.x = end3D.x - dir.x * cutSize;
							trimmedEnd.y = end3D.y - dir.y * cutSize;
							trimmedEnd.z = z;

							trimmed[corner][1] = trimmedStart;
							trimmed[next][0] = trimmedEnd;

							ICoord2D s, e;
							if (worldToScreen(&trimmedStart, &s, camera) && worldToScreen(&trimmedEnd, &e, camera)) {
								m_lineRenderer->Add_Line(Vector2(s.x, s.y), Vector2(e.x, e.y), BOUNDING_BOX_LINE_WIDTH, lineColor);
							}
						}

						// Draw beveled corners
						for (int cornerx = 0; cornerx < 4; ++cornerx) {
							ICoord2D a, b;
							if (worldToScreen(&trimmed[cornerx][0], &a, camera) &&
								worldToScreen(&trimmed[cornerx][1], &b, camera)) {
								m_lineRenderer->Add_Line(Vector2(a.x, a.y), Vector2(b.x, b.y), BOUNDING_BOX_LINE_WIDTH, lineColor);
							}
						}
					}

					z += ginfo.getMaxHeightAbovePosition(); // Top layer (only used for purple)
				}
			}
			break;
		}
		//---------------------------------------------------------------------------------------------
		case GEOMETRY_SPHERE: // not quite right, but close enough
		case GEOMETRY_CYLINDER:
		{ 
			Real angle, inc = PI / 4.0f;
			Real radius = ginfo.getMajorRadius();
			Coord3D pnt, lastPnt;
			ICoord2D start, end;
			Real z = pos.z;
		
			bool shouldEnd, shouldStart;
		
			// Draw the cylinder.
			for (int i = 0; i < 2; i++) {
				angle = 0.0f;
				lastPnt.x = pos.x + radius * (Real)cos(angle);
				lastPnt.y = pos.y + radius * (Real)sin(angle);
				lastPnt.z = z;
				shouldEnd = worldToScreen(&lastPnt, &end, camera);
		
				for (angle = inc; angle <= 2.0f * PI; angle += inc) {
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
			shouldStart = worldToScreen(&pnt, &start, camera);
			pnt.z = pos.z + ginfo.getMaxHeightAbovePosition();
			shouldEnd = worldToScreen(&pnt, &end, camera);
			if (shouldStart && shouldEnd) {
				m_lineRenderer->Add_Line(Vector2(start.x, start.y), Vector2(end.x, end.y), BOUNDING_BOX_LINE_WIDTH, color);
			}
		
			// Draw green outer margin (similar to GEOMETRY_BOX's green box)
			if (!isSmall) {
				const Real margin = 14.0f;
				const Real outerRadius = radius + margin;
				const unsigned long greenColor = 0xFF00C8C8;
				const Real fineInc = PI / 4.0f; // Reduced segments for performance
		
				Coord3D pnt, lastPnt;
				ICoord2D start, end;
				bool shouldStart, shouldEnd;
		
				angle = 0.0f;
				lastPnt.x = pos.x + outerRadius * (Real)cos(angle);
				lastPnt.y = pos.y + outerRadius * (Real)sin(angle);
				lastPnt.z = pos.z;
				shouldEnd = worldToScreen(&lastPnt, &end, camera);
		
				for (angle = fineInc; angle <= 2.0f * PI + fineInc; angle += fineInc) {
					pnt.x = pos.x + outerRadius * (Real)cos(angle);
					pnt.y = pos.y + outerRadius * (Real)sin(angle);
					pnt.z = pos.z;
					shouldStart = worldToScreen(&pnt, &start, camera);
		
					if (shouldStart && shouldEnd) {
						m_lineRenderer->Add_Line(Vector2(start.x, start.y), Vector2(end.x, end.y), BOUNDING_BOX_LINE_WIDTH, greenColor);
					}
		
					lastPnt = pnt;
					end = start;
					shouldEnd = shouldStart;
				}
			}
			break;
		}
	} 
}

/** Draw a "circle" into the m_lineRenderer, e.g. to visualize weapon range, sight range, sound range **/
void DrawObject::addCircleToLineRenderer( const Coord3D & center, Real radius, Real width, unsigned long color, CameraClass* camera )
{
    Real angle, inc = PI / 24.0f; // smoother circle with more segments
    Coord3D pnt, lastPnt;
    ICoord2D screenStart, screenEnd;

    // First point
    angle = 0.0f;
    lastPnt.x = center.x + radius * (Real)cos(angle);
    lastPnt.y = center.y + radius * (Real)sin(angle);

    // Adjust Z using terrain and optionally water height
    lastPnt.z = TheTerrainRenderObject->getHeightMapHeight(lastPnt.x, lastPnt.y, NULL);
    if (m_showWater) {
        Real waterZ = getWaterHeightIfUnderwater(lastPnt.x, lastPnt.y);
        if (waterZ != -FLT_MAX) {
            lastPnt.z = waterZ + 4.5f;
        }
    }

    bool shouldEnd = worldToScreen(&lastPnt, &screenEnd, camera);

    for (angle = inc; angle <= 2.0f * PI + 0.001f; angle += inc) {
        pnt.x = center.x + radius * (Real)cos(angle);
        pnt.y = center.y + radius * (Real)sin(angle);

        pnt.z = TheTerrainRenderObject->getHeightMapHeight(pnt.x, pnt.y, NULL);
        if (m_showWater) {
            Real waterZ = getWaterHeightIfUnderwater(pnt.x, pnt.y);
            if (waterZ != -FLT_MAX) {
                pnt.z = waterZ + 4.5f;
            }
        }

        bool shouldStart = worldToScreen(&pnt, &screenStart, camera);
        if (shouldStart && shouldEnd) {
            m_lineRenderer->Add_Line(Vector2(screenStart.x, screenStart.y), Vector2(screenEnd.x, screenEnd.y), width, color);
        }

        lastPnt = pnt;
        screenEnd = screenStart;
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

	// AudioEventRTS event;
	// event.setEventName(comboText);
	// event.setAudioEventInfo(TheAudio->findAudioEventInfo(comboText));
	// event.generateFilename();
	
	// if (!event.getFilename().isEmpty()) {
	// 	PlaySound(event.getFilename().str(), NULL, SND_ASYNC | SND_FILENAME | SND_PURGE);
	// }
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

	// AudioEventRTS eventToPlay;
	// eventToPlay.setEventName(event->getEventName());
	// eventToPlay.setAudioEventInfo(audioInfo);
	// eventToPlay.generateFilename();
	
	// if (!eventToPlay.getFilename().isEmpty()) {
	// 	PlaySound(eventToPlay.getFilename().str(), NULL, SND_ASYNC | SND_FILENAME | SND_PURGE);
	// }
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

void DrawObject::updateTerrainPasteVB(void)
{
    TileTool::TerrainCopyBuffer &buf = TileTool::s_copyBuffer;

    m_feedbackVertexCount = 0;
    m_feedbackIndexCount = 0;

    DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexFeedback, D3DLOCK_DISCARD);
    UnsignedShort *ib = lockIdxBuffer.Get_Index_Array();
    UnsignedShort *curIb = ib;

    DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexFeedback, D3DLOCK_DISCARD);
    VertexFormatXYZDUV1 *vb = (VertexFormatXYZDUV1*)lockVtxBuffer.Get_Vertex_Array();
    VertexFormatXYZDUV1 *curVb = vb;

    int w = buf.width;
    int h = buf.height;
    if (w <= 0 || h <= 0) return;

    const int PAD_VERT = 9;   // match mesh code's small padding
    const int PAD_IDX  = 12;  // some extra indices like mesh code
    const int MAX_VERTS = NUM_FEEDBACK_VERTEX;
    const int MAX_IDX   = NUM_FEEDBACK_INDEX;
    const int MAX_16BIT = 65535;

    int expectedVerts = w * h;
    int expectedIdx = (w - 1) * (h - 1) * 6;

    // Defensive caps:
    if (expectedVerts + PAD_VERT >= MAX_VERTS) {
        DEBUG_LOG(("Paste preview too many vertices (%d) — skipping preview", expectedVerts));
        return;
    }
    if (expectedIdx + PAD_IDX >= MAX_IDX) {
        DEBUG_LOG(("Paste preview too many indices (%d) — skipping preview", expectedIdx));
        return;
    }
    if (expectedVerts > MAX_16BIT) {
        DEBUG_LOG(("Paste preview exceeds 16-bit index limit (%d verts) — skipping preview", expectedVerts));
        return;
    }

    DWORD color = 0x80FFFFFF;  /* semi-transparent white */

    /* Convert rotation to radians */
    float angle = 0.0f;
    switch (m_terrainPasteFeedbackRotation)
    {
        case 90:  angle = 3.1415926f * 0.5f; break;
        case 180: angle = 3.1415926f; break;
        case 270: angle = 3.1415926f * 1.5f; break;
        default: angle = 0.0f; break;
    }

    float cosA = cosf(angle);
    float sinA = sinf(angle);

    /* Center offset for rotation pivot */
    float cx = (float)(w - 1) * 0.5f;
    float cy = (float)(h - 1) * 0.5f;

    /* Fill vertex buffer with rotated positions */
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            /* Local coordinates centered at pivot */
            float localX = (x - cx) * MAP_XY_FACTOR;
            float localY = (y - cy) * MAP_XY_FACTOR;

            /* Apply rotation */
            float rotX = localX * cosA - localY * sinA;
            float rotY = localX * sinA + localY * cosA;

			/* Compute world position */
			Coord3D tmp;
			tmp.x = m_terrainPasteCenter.x + rotX;
			tmp.y = m_terrainPasteCenter.y + rotY;
			// tmp.z = (float)buf.heightData[y][x] * MAP_HEIGHT_SCALE;

			float baseZ = (float)buf.heightData[y][x] * MAP_HEIGHT_SCALE;
			tmp.z = baseZ + m_brushHeight;  

			// Snap to grid if enabled
			CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
			WbView3d *pView = pDoc->Get3DView(); 
			pView->snapPoint(&tmp);

			curVb->x = tmp.x;
			curVb->y = tmp.y;
			curVb->z = tmp.z;

            curVb->u1 = 0.0f;
            curVb->v1 = 0.0f;
            curVb->diffuse = color;

            ++curVb;
            ++m_feedbackVertexCount;
        }
    }

    /* Build index buffer (unchanged) */
    for (int b = 0; b < h - 1; ++b)
    {
        for (int x = 0; x < w - 1; ++x)
        {
            int idx = b * w + x;
            *curIb++ = (UnsignedShort)idx;
            *curIb++ = (UnsignedShort)(idx + 1);
            *curIb++ = (UnsignedShort)(idx + w);
            *curIb++ = (UnsignedShort)(idx + 1);
            *curIb++ = (UnsignedShort)(idx + w + 1);
            *curIb++ = (UnsignedShort)(idx + w);
            m_feedbackIndexCount += 6;
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


// This routine fails to draw poly triggers in some cases when optimized.
// So just shut it off for now.  The failure case was new doc, add a poly trigger.
// Adding any other object fixed the problem.	jba

#pragma optimize("", off)

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
  if (m_drawObjects || m_drawWaypoints || m_drawBoundingBoxes || m_drawSightRanges || m_drawWeaponRanges || m_drawSoundRanges || m_drawTestArtHighlight || m_drawObjectsSelected) {
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
			// if (pMapObj->isSelected()) {
			// 	Transform.Get_Translation();
			// }

			/**
			 * Adriane [Deathscythe] -- this check already existed, not sure why they commented it out.
			 * It actually works too -- significantly reduces lag when rendering object icons
			 * with lots of objects placed on the map.
			 *
			 * Cull the mfs
			 */
			Coord3D loc = *pMapObj->getLocation();

			SphereClass bounds(Vector3(loc.x, loc.y, loc.z), THE_RADIUS); 
			if (rinfo.Camera.Cull_Sphere(bounds)) {
				continue;
			}

			if (TheTerrainRenderObject) {
				loc.z += TheTerrainRenderObject->getHeightMapHeight(loc.x, loc.y, NULL);
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
				if (!m_drawObjects && !pMapObj->isSelected()) {
					continue;
				}
				if (BuildListTool::isActive()) {
					continue;
				}
			}

			// Makes the icons look 3d -- we set it to true by default since it looks cool -- Adriane
			Bool isTree = true;

			int settingColor;
			
			if (doDiamond) { // Waypoint
				settingColor = m_waypointIconColor;
			} else if ( pMapObj->getFlag(FLAG_ROAD_FLAGS)) {
				settingColor = m_roadIconColor;
			} else if ( pMapObj->getThingTemplate() && (pMapObj->getThingTemplate()->getEditorSorting() == ES_INFANTRY || pMapObj->getThingTemplate()->getEditorSorting() == ES_VEHICLE) ) {
				settingColor = m_unitIconColor;
			} else if ( pMapObj->getThingTemplate() && pMapObj->getThingTemplate()->getEditorSorting() == ES_SHRUBBERY) {
				settingColor = m_treeIconColor;
				// isTree = true; 
			} else { // Everything else
				settingColor = m_defaultIconColor;
			}
			
			// Now build the setting
			int setting = settingColor;
			if (doArrow) {
				setting |= (1 << 25);
			}
			if (doDiamond) {
				setting |= (1 << 26);
			}
			
			// Now push into vertex buffers like before
			if (count & 1) {
				if (setting != rememberLastSettingVB1) {
					rememberLastSettingVB1 = setting;
					updateVB(m_vertexBufferTile1, settingColor, doArrow, doDiamond);
				}
				DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferTile1);
			} else {
				if (setting != rememberLastSettingVB2) {
					rememberLastSettingVB2 = setting;
					updateVB(m_vertexBufferTile2, settingColor, doArrow, doDiamond);
				}
				DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferTile2);
			}
			

			Vector3 vec(loc.x, loc.y, loc.z);
			Matrix3D tm(Transform);
			Matrix3x3 rot(true);
			if (!(pMapObj->getFlag(FLAG_ROAD_FLAGS) || 
				pMapObj->getFlag(FLAG_BRIDGE_FLAGS))) 
			{
				rot.Rotate_Z(pMapObj->getAngle());
			}

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
				updateVB(m_vertexBufferTile1, GREEN, true, false, false);
				DX8Wrapper::Set_Vertex_Buffer(m_vertexBufferTile1);
			} else {
				updateVB(m_vertexBufferTile2, GREEN, true, false, false);
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
		updateWaypointVB(rinfo);
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
		if (m_feedbackIndexCount > 0) {
			DX8Wrapper::Set_DX8_Render_State(D3DRS_ZENABLE, TRUE);
			DX8Wrapper::Set_DX8_Render_State(D3DRS_ZWRITEENABLE, TRUE);
			DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE, FALSE);

			DX8Wrapper::Set_Vertex_Buffer(m_vertexFeedback);
			DX8Wrapper::Set_Index_Buffer(m_indexFeedback,0);
			DX8Wrapper::Set_Shader(m_shaderClass);
			DX8Wrapper::Set_DX8_Render_State(D3DRS_CULLMODE, D3DCULL_NONE);
			DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE,D3DFILL_SOLID);	// we want a solid ramp
			DX8Wrapper::Set_DX8_Render_State(D3DRS_LIGHTING, FALSE);				// disable lighting
			DX8Wrapper::Draw_Triangles(	0, m_feedbackIndexCount/3, 0,	m_feedbackVertexCount);
		}
	}

	if (m_rulerGridFeedback) {
		updateGridVB();
		if (m_feedbackIndexCount > 0) {
			DX8Wrapper::Set_DX8_Render_State(D3DRS_ZENABLE, TRUE);
			DX8Wrapper::Set_DX8_Render_State(D3DRS_ZWRITEENABLE, TRUE);
			DX8Wrapper::Set_DX8_Render_State(D3DRS_ALPHABLENDENABLE, FALSE);

			DX8Wrapper::Set_Vertex_Buffer(m_vertexFeedback);
			DX8Wrapper::Set_Index_Buffer(m_indexFeedback, 0);
			DX8Wrapper::Set_Shader(SC_OPAQUE_Z); // or any shader that fits
			DX8Wrapper::Set_DX8_Render_State(D3DRS_CULLMODE, D3DCULL_NONE);
			DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE, D3DFILL_SOLID); // or D3DFILL_WIREFRAME if you prefer
			DX8Wrapper::Set_DX8_Render_State(D3DRS_LIGHTING, FALSE);
			DX8Wrapper::Draw_Triangles(0, m_feedbackIndexCount / 3, 0, m_feedbackVertexCount);
		}
	}

#if 1
	if (m_terrainPasteFeedback && !m_disableFeedback) {
		updateTerrainPasteVB();
		if (m_feedbackIndexCount > 0) {
			DX8Wrapper::Set_Vertex_Buffer(m_vertexFeedback);
			DX8Wrapper::Set_Index_Buffer(m_indexFeedback,0);
			DX8Wrapper::Set_Shader(SC_OPAQUE_Z);
			DX8Wrapper::Set_DX8_Render_State(D3DRS_FILLMODE,D3DFILL_WIREFRAME);
			DX8Wrapper::Draw_Triangles(	0, m_feedbackIndexCount/3, 0,	m_feedbackVertexCount);
		}
	} else if (m_meshFeedback) {
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

	if (m_showTracingOverlay) {
		CWorldBuilderDoc *pDoc = CWorldBuilderDoc::GetActiveDoc();
		WorldHeightMapEdit *pMap = pDoc->GetHeightMap();

		// Grid resolution (increase for more detail)
		const int gridX = 64;
		const int gridY = 64;

		float left   = ADJUST_FROM_INDEX_TO_REAL(1);
		float top    = ADJUST_FROM_INDEX_TO_REAL(1);
		float right  = ADJUST_FROM_INDEX_TO_REAL(pMap->getXExtent() - 2);
		float bottom = ADJUST_FROM_INDEX_TO_REAL(pMap->getYExtent() - 2);

		float dx = (right - left) / (gridX - 1);
		float dy = (bottom - top) / (gridY - 1);

		DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexFeedback, D3DLOCK_DISCARD);
		VertexFormatXYZDUV1* vb = (VertexFormatXYZDUV1*)lockVtxBuffer.Get_Vertex_Array();

		int vtxCount = 0;
		for (int y = 0; y < gridY; ++y) {
			float fy = top + y * dy;
			float v = (float)y / (gridY - 1);
			for (int x = 0; x < gridX; ++x) {
				float fx = left + x * dx;
				float u = (float)x / (gridX - 1);
				float fz = TheTerrainRenderObject->getHeightMapHeight(fx, fy, NULL) + 2.0f; // Slightly above terrain

				vb[vtxCount].x = fx;
				vb[vtxCount].y = fy;
				vb[vtxCount].z = fz;
				vb[vtxCount].u1 = u;
				vb[vtxCount].v1 = v;
				vb[vtxCount].diffuse = 0xFFFFFFFF;
				vtxCount++;
			}
		}

		DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexFeedback, D3DLOCK_DISCARD);
		UnsignedShort* ib = lockIdxBuffer.Get_Index_Array();

		int idxCount = 0;
		for (int b = 0; b < gridY - 1; ++b) {
			for (int x = 0; x < gridX - 1; ++x) {
				int i0 = b * gridX + x;
				int i1 = i0 + 1;
				int i2 = i0 + gridX;
				int i3 = i2 + 1;
				// First triangle
				ib[idxCount++] = i0;
				ib[idxCount++] = i1;
				ib[idxCount++] = i2;
				// Second triangle
				ib[idxCount++] = i1;
				ib[idxCount++] = i3;
				ib[idxCount++] = i2;
			}
		}

		m_feedbackVertexCount = vtxCount;
		m_feedbackIndexCount = idxCount;

		DX8Wrapper::Set_Vertex_Buffer(m_vertexFeedback);
		DX8Wrapper::Set_Index_Buffer(m_indexFeedback, 0);
		DX8Wrapper::Set_Shader(ShaderClass::_PresetAlpha2DShader);
		DX8Wrapper::Set_Material(m_vertexMaterialClass);
		DX8Wrapper::Set_Texture(0, W3DAssetManager::Get_Instance()->Get_Texture("data\\editor\\trace_overlay.dds"));
		DX8Wrapper::Draw_Triangles(0, idxCount / 3, 0, vtxCount);
		DX8Wrapper::Set_Texture(0, NULL);
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

	if (m_waterDrawObject && m_showWater) {
		m_waterDrawObject->renderWater();
	}

	if (m_drawLetterbox) {
		int w = m_winSize.x;
		int h = m_winSize.y;
		int size = 200; // or whatever fixed height you want
		RectClass rect(0, 0, w, size);
		m_lineRenderer->Add_Quad(rect, 0xFF000000);
		rect.Set(0, h - size, w, h);
		m_lineRenderer->Add_Quad(rect, 0xFF000000);
		linesToRender = true;
	}

	// Render any lines that have been added, like bounding boxes.
	// MLL C&C3 - guarded to prevent Render2D overflow crash
	if (m_lineRenderer) {
		int vCount = m_lineRenderer->Get_Color_Array().Count();
		if (linesToRender && vCount > 0) {
			// 20k base its working -- 30k is working
			if (vCount > MAX_LINE_RENDER_SAFE_LIMIT) {
				// DEBUG_LOG(("m_lineRenderer overflow detected (%d colors) — resetting\n", vCount));
				m_lineRenderer->Reset();
			} else {
				m_lineRenderer->Render();
				m_lineRenderer->Reset();
			}
		}
	}
}
#pragma optimize("", on)

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
