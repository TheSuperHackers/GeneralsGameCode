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
#include "WBHeightMap.h"
#include "Common/GlobalData.h"
#include <tri.h>
#include <colmath.h>
#include <coltest.h>


//-----------------------------------------------------------------------------
//         Private Data
//-----------------------------------------------------------------------------

//=============================================================================
// WBHeightMap::WBHeightMap
//=============================================================================
WBHeightMap::WBHeightMap() :
	m_drawEntireMap(false),
	m_flattenHeights(false)
{
}

//=============================================================================
// WBHeightMap::Render
//=============================================================================
/** Renders (draws) the terrain. */
//=============================================================================
void WBHeightMap::setFlattenHeights(Bool flat)
{
	if (m_flattenHeights != flat) {
		m_flattenHeights = flat;
		m_originX = 0;
		m_originY = 0;
 		updateBlock(0, 0, m_x-1, m_y-1, m_map, nullptr);
	}
}

// THE_Z is just above the water plane, so the flattened terrain doesn't draw
// under water.
#define THE_Z (10)
//=============================================================================
// WBHeightMap::flattenHeights
//=============================================================================
/** Flattens the terrain for the top down view.. */
//=============================================================================
void WBHeightMap::flattenHeights(void) {
	Real theZ = THE_Z;
	Int i, j;
	for (j=0; j<m_numVBTilesY; j++)
		for (i=0; i<m_numVBTilesX; i++)
		{
			static int count = 0;
			count++;
			Int numVertex = (VERTEX_BUFFER_TILE_LENGTH*2)*(VERTEX_BUFFER_TILE_LENGTH*2);
			DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexBufferTiles[j*m_numVBTilesX+i]);
			VERTEX_FORMAT *vbHardware = (VERTEX_FORMAT*)lockVtxBuffer.Get_Vertex_Array();
			Int vtx;
			for (vtx=0; vtx<numVertex; vtx++) {
				vbHardware->z = theZ;
				vbHardware++;
			}
		}
}

//=============================================================================
// WBHeightMap::getMaxCellHeight
//=============================================================================
/** Returns maximum height of the 4 corners containing the given point */
//=============================================================================
Real WBHeightMap::getMaxCellHeight(Real x, Real y)
{
	if (!m_flattenHeights) {
		return HeightMapRenderObjClass::getMaxCellHeight(x,y);
	}
	// If we are flattening the height, all z values aret THE_Z.  jba.
	return THE_Z;
}


//=============================================================================
// WBHeightMap::getHeight
//=============================================================================
/** return the height and normal of the triangle plane containing given location within heightmap. */
//=============================================================================
Real WBHeightMap::getHeightMapHeight(Real x, Real y, Coord3D* normal)
{
	if (!m_flattenHeights) {
		return HeightMapRenderObjClass::getHeightMapHeight(x,y,normal);
	}
	// If we are flattening the height, all z values aret THE_Z.  jba.
	if (normal) {
		normal->x = 0;
		normal->y = 0;
		normal->z = 1;
	}
	return THE_Z;
}



//=============================================================================
// WBHeightMap::Cast_Ray
//=============================================================================
/** Return intersection of a ray with the heightmap mesh.

*/
//=============================================================================
Bool WBHeightMap::Cast_Ray(RayCollisionTestClass & raytest)
{
	if (!m_flattenHeights) {
		return HeightMapRenderObjClass::Cast_Ray(raytest);
	}
	Real theZ = THE_Z;
	TriClass tri;
	Bool hit = false;
	Int X,Y;
	Vector3 normal,P0,P1,P2,P3;

	if (!m_map)
		return false;	//need valid pointer to heightmap samples
//	HeightSampleType *pData = m_map->getDataPtr();
	//Clip ray to extents of heightfield
	AABoxClass hbox;
	LineSegClass lineseg,lineseg2;
	CastResultStruct	result;
	Int StartCellX;
	Int EndCellX;
 	Int StartCellY;
	Int EndCellY;
	const Int overhang = 2*VERTEX_BUFFER_TILE_LENGTH; // Allow picking past the edge for scrolling & objects.
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
			return(false);
		}

		// Take the 2D bounding box of ray and check heights
		// inside this box for intersection.
		if (P0.X > P1.X) {	//flip start/end points
			StartCellX = floor(P1.X/MAP_XY_FACTOR);
			EndCellX = ceil(P0.X/MAP_XY_FACTOR);
		}	else {
			StartCellX = floor(P0.X/MAP_XY_FACTOR);
			EndCellX = ceil(P1.X/MAP_XY_FACTOR);
		}
		if (P0.Y > P1.Y) {	//flip start/end points
			StartCellY=floor(P1.Y/MAP_XY_FACTOR);
			EndCellY=ceil(P0.Y/MAP_XY_FACTOR);
		}	else {
			StartCellY = floor(P0.Y/MAP_XY_FACTOR);
			EndCellY = ceil(P1.Y/MAP_XY_FACTOR);
		}

		Vector3 minPt(MAP_XY_FACTOR*(StartCellX-1), MAP_XY_FACTOR*(StartCellY-1), theZ-1);
		Vector3 maxPt(MAP_XY_FACTOR*(EndCellX+1), MAP_XY_FACTOR*(EndCellY+1), theZ+1);
		MinMaxAABoxClass mmbox(minPt, maxPt);
		hbox.Init(mmbox);
	}

	raytest.Result->ComputeContactPoint=true;	//tell CollisionMath that we need point.

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
				P0.X=X*MAP_XY_FACTOR;
				P0.Y=Y*MAP_XY_FACTOR;
				P0.Z=THE_Z;

				P1.X=(X+1)*MAP_XY_FACTOR;
				P1.Y=Y*MAP_XY_FACTOR;
				P1.Z=THE_Z;

				P2.X=(X+1)*MAP_XY_FACTOR;
				P2.Y=(Y+1)*MAP_XY_FACTOR;
				P2.Z=THE_Z;

				P3.X=X*MAP_XY_FACTOR;
				P3.Y=(Y+1)*MAP_XY_FACTOR;
				P3.Z=THE_Z;


				tri.V[0] = &P0;
				tri.V[1] = &P1;
				tri.V[2] = &P2;

				tri.N = &normal;

				tri.Compute_Normal();

				hit = hit | CollisionMath::Collide(raytest.Ray, tri, raytest.Result);

				if (raytest.Result->StartBad)
					return true;

				//top triangle
				tri.V[0] = &P2;
				tri.V[1] = &P3;
				tri.V[2] = &P0;

				tri.N = &normal;

				tri.Compute_Normal();

				hit = hit | CollisionMath::Collide(raytest.Ray, tri, raytest.Result);

				if (hit)
					raytest.Result->SurfaceType = SURFACE_TYPE_DEFAULT;	///@todo: WW3D uses this to return dirt, grass, etc.  Do we need this?
			}
			if (hit) break;
		}
		if (hit) break;
	}
	return hit;
}


//=============================================================================
// WBHeightMap::Render
//=============================================================================
/** Renders (draws) the terrain. */
//=============================================================================
void WBHeightMap::Render(RenderInfoClass & rinfo)
{
	if (m_flattenHeights) {
		flattenHeights();
	}
	HeightMapRenderObjClass::Render(rinfo);
}


