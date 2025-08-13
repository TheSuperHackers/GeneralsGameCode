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

#ifndef __DRAW_OBJECT_H_
#define __DRAW_OBJECT_H_

#include "always.h"
#include "rendobj.h"
#include "w3d_file.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "shader.h"
#include "vertmaterial.h"
#include "Lib/BaseType.h"
#include "Common/AsciiString.h"

// The draw objects draw a circle of diameter 1.0 cells.
#define THE_RADIUS (0.8f*MAP_XY_FACTOR)

class MeshClass;
class PolygonTrigger;
class WaterRenderObjClass;
//
// DrawObject: Draws 3d feedback for tools & objects.
//
//
class DrawObject : public RenderObjClass
{

public:

	DrawObject(void);
	DrawObject(const DrawObject & src);
	DrawObject & operator = (const DrawObject &);
	~DrawObject(void);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface
	/////////////////////////////////////////////////////////////////////////////
	virtual RenderObjClass *	Clone(void) const;
	virtual int						Class_ID(void) const;
	virtual void					Render(RenderInfoClass & rinfo);
//	virtual void					Special_Render(SpecialRenderInfoClass & rinfo);
//	virtual void 					Set_Transform(const Matrix3D &m);
//	virtual void 					Set_Position(const Vector3 &v);
//TODO: MW: do these later - only needed for collision detection
	virtual Bool					Cast_Ray(RayCollisionTestClass & raytest);
//	virtual Bool					Cast_AABox(AABoxCollisionTestClass & boxtest);
//	virtual Bool					Cast_OBBox(OBBoxCollisionTestClass & boxtest);
//	virtual Bool					Intersect_AABox(AABoxIntersectionTestClass & boxtest);
//	virtual Bool					Intersect_OBBox(OBBoxIntersectionTestClass & boxtest);

	virtual void					Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const;
    virtual void					Get_Obj_Space_Bounding_Box(AABoxClass & aabox) const;


//	virtual int					 	Get_Num_Polys(void) const;
//	virtual const char *		 	Get_Name(void) const;
//	virtual void				 	Set_Name(const char * name);

//	unsigned int					Get_Flags(void)  { return Flags; }
//	void								Set_Flags(unsigned int flags) { Flags = flags; }
//	void								Set_Flag(unsigned int flag, Bool onoff) { Flags &= (~flag); if (onoff) Flags |= flag; }

	Int freeMapResources(void);

	void setDrawObjects(Bool val, Bool waypoints, Bool poly) { m_drawObjects = val; m_drawWaypoints=waypoints; m_drawPolygonAreas = poly;}
	static void setDoBrushFeedback(Bool val) { m_toolWantsFeedback = val; m_meshFeedback=false;}
	static void setDoMeshFeedback(Bool val) { m_meshFeedback = val; }
	static void setDoRampFeedback(Bool val) { m_rampFeedback = val; }
	static void setDoBoundaryFeedback(Bool val) { m_boundaryFeedback = val; }
	static void setDoAmbientSoundFeedback(Bool val) { m_ambientSoundFeedback = val; }

	static void setBrushFeedbackParms(Bool square, Int width, Int featherWidth)
														{ m_squareFeedback = square; m_brushWidth=width;
															m_meshFeedback = false; m_brushFeatherWidth = featherWidth;}
	static void disableFeedback(void) {m_disableFeedback = true;};
	static void enableFeedback(void) {m_disableFeedback = false;};
	static Bool isFeedbackEnabled(void) { return !m_disableFeedback;};
	static void setFeedbackPos(Coord3D pos);

	static void setWaypointDragFeedback(const Coord3D &start, const Coord3D &end);
	static void setRampFeedbackParms(const Coord3D *start, const Coord3D *end, Real rampWidth);
	static void stopWaypointDragFeedback();




	MeshClass *peekMesh(void) {return m_moldMesh;};
	void getMeshBounds(SphereClass *pSphere) {*pSphere = m_moldMeshBounds;};

protected:
	enum {MAX_RADIUS = 50, NUM_FEEDBACK_VERTEX = 201*201, NUM_FEEDBACK_INDEX = 101*101*6};
	Int	m_numTriangles;	//dimensions of list

	DX8IndexBufferClass				*m_indexBuffer;	///< indices defining a object icon
	ShaderClass								m_shaderClass; ///< shader or rendering state for heightmap
	VertexMaterialClass	  	  *m_vertexMaterialClass;
	DX8VertexBufferClass			*m_vertexBufferTile1;	///< First vertex buffer.
	DX8VertexBufferClass			*m_vertexBufferTile2;	///< Second vertex buffer.

	DX8VertexBufferClass			*m_vertexBufferWater;	///< Vertex buffer for the water plane.
	DX8IndexBufferClass				*m_indexWater;	///< indices defining a triangle strip for the water on terrain
	Int												m_waterVertexCount;

	WaterRenderObjClass				*m_waterDrawObject;

	Bool											m_drawObjects;
	Bool											m_drawWaypoints;
	Bool											m_drawPolygonAreas;

	DX8VertexBufferClass			*m_vertexFeedback;	///< Vertex buffer for brush feedback.
	DX8IndexBufferClass				*m_indexFeedback;	///< indices defining a triangle strip for the feedback on terrain
	Int												m_feedbackIndexCount;
	Int												m_feedbackVertexCount;

	AsciiString								m_curMeshModelName;  ///< Model name of m_moldMesh.

	MeshClass									*m_moldMesh;		///< W3D mesh model for the mold.
	SphereClass								m_moldMeshBounds;				///< Bounding sphere for mold mesh.

protected: // static state vars.
	static Bool								m_squareFeedback;	///< True for square brush feedback, false for round.
	static Int								m_brushWidth;	///< Width of brush feedback.
	static Int								m_brushFeatherWidth;	///< Width of brush feathered feedback.
	static Bool								m_toolWantsFeedback; ///< True to display brush feedback.
	static Bool								m_disableFeedback; ///< True to disable feedback.
	static Coord3D						m_feedbackPoint;	///< Current brush feedback location.
	static CPoint							m_cellCenter;		///< Cell to show feedback from.
	static Bool								m_meshFeedback;
	static Bool								m_rampFeedback;	///< should we be showing feedback for the ramp tool?
	static Bool								m_boundaryFeedback;
	static Bool								m_ambientSoundFeedback;

	static Bool								m_dragWaypointFeedback; ///< True for the waypoint tool dragging.
	static Coord3D						m_dragWayStart;///< Start drag waypoint feedback.
	static Coord3D						m_dragWayEnd; ///< End drag waypoint.

	static Coord3D						m_rampStartPoint;	///< Beginning ramp point
	static Coord3D						m_rampEndPoint;		///< End ramp point
	static Real								m_rampWidth;

protected:
	int initData(void);
	Int updateVB(DX8VertexBufferClass	*vertexBufferTile, Int color, Bool doArrow, Bool doDiamond);
	void updatePolygonVB(PolygonTrigger *pTrig, Bool selected, Bool isOpen);
	void updateFeedbackVB(void);
	void updateMeshVB(void);
	void updateRampVB(void);
	void updateWaypointVB(void);
	void updateForWater(void);
	void updateBoundaryVB(void);
	void updateAmbientSoundVB(void);

};

void BuildRectFromSegmentAndWidth(const Coord3D* b, const Coord3D* t, Real width,
																	Coord3D* outBL, Coord3D* outTL, Coord3D* outBR, Coord3D* outTR);

#endif  // end __DRAW_OBJECT_H_
