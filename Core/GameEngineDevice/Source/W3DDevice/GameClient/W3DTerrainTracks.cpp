import Graphics.Frame.RenderClock;
#include "W3DDevice/GameClient/W3DRenderServices.h"
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

// FILE: W3DTerrainTracks.cpp ////////////////////////////////////////////////
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
// File name: W3DTerrainTracks.cpp
//
// Created:   Mark Wilczynski, May 2001
//
// Desc:      Draw track marks on the terrain.  Uses a sequence of connected
//			  quads that are oriented to fit the terrain and updated when object
//			  moves.
//-----------------------------------------------------------------------------

#include <array>
#include <span>
#include <vector>
import Graphics.Scene.Tracks.Geometry;
import Graphics.Frame.Runtime;
#include "W3DDevice/GameClient/W3DGraphicsResources.h"
#include "W3DDevice/GameClient/W3DTerrainTracks.h"
#include "W3DDevice/GameClient/BaseHeightMap.h"
#include "W3DDevice/GameClient/WorldHeightMap.h"
#include "Common/PerfTimer.h"
#include "Common/GlobalData.h"
#include "Common/Debug.h"
#include "W3DDevice/GameClient/W3DTextureHandle.h"
#include "WWMath/colmath.h"
#include "W3DDevice/GameClient/W3DCastQuery.h"
#include "W3DDevice/GameClient/W3DRenderContext.h"
#include "W3DDevice/GameClient/W3DCamera.h"
#include "W3DDevice/GameClient/W3DAssetCatalog.h"

#include "W3DDevice/GameClient/W3DSceneClass.h"
#include "GameLogic/TerrainLogic.h"
#include "GameLogic/Object.h"
#include "GameClient/Drawable.h"


#define BRIDGE_OFFSET_FACTOR	0.25f	//amount to raise tracks above bridges.
//=============================================================================
// TerrainTracksRenderObjClass::~TerrainTracksRenderObjClass
//=============================================================================
/** Destructor. Releases w3d assets. */
//=============================================================================
TerrainTracksRenderObjClass::~TerrainTracksRenderObjClass()
{
	freeTerrainTracksResources();
}

//=============================================================================
// TerrainTracksRenderObjClass::TerrainTracksRenderObjClass
//=============================================================================
/** Constructor. Just nulls out some variables. */
//=============================================================================
TerrainTracksRenderObjClass::TerrainTracksRenderObjClass()
{
	m_stageZeroTexture=nullptr;
	m_lastAnchor=Vector3(0,1,2.25);
	m_haveAnchor=false;
	m_haveCap=true;
	m_topIndex=0;
	m_bottomIndex=0;
	m_activeEdgeCount=0;
	m_totalEdgesAdded=0;
	m_bound=false;
	m_ownerDrawable = nullptr;
}

//=============================================================================
// TerrainTracksRenderObjClass::Get_Obj_Space_Bounding_Sphere
//=============================================================================
/** WW3D method that returns object bounding sphere used in frustum culling*/
//=============================================================================
void TerrainTracksRenderObjClass::Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const
{	/// @todo: Add code to cull track marks to screen by constantly updating bounding volumes
	sphere=m_boundingSphere;
}

//=============================================================================
// TerrainTracksRenderObjClass::Get_Obj_Space_Bounding_Box
//=============================================================================
/** WW3D method that returns object bounding box used in collision detection*/
//=============================================================================
void TerrainTracksRenderObjClass::Get_Obj_Space_Bounding_Box(AABoxClass & box) const
{
	box=m_boundingBox;
}

//=============================================================================
// MirrorRenderObjClass::Class_ID
//=============================================================================
/** returns the class id, so the scene can tell what kind of render object it has. */
//=============================================================================
Int TerrainTracksRenderObjClass::Class_ID() const
{
	return W3DRenderObject::CLASSID_IMAGE3D;
}

//=============================================================================
// TerrainTracksRenderObjClass::Clone
//=============================================================================
/** Not used, but required virtual method. */
//=============================================================================
W3DRenderObject *	 TerrainTracksRenderObjClass::Clone() const
{
	assert(false);
	return nullptr;
}

//=============================================================================
// TerrainTracksRenderObjClass::freeTerrainTracksResources
//=============================================================================
/** Free any W3D resources associated with this object */
//=============================================================================
Int TerrainTracksRenderObjClass::freeTerrainTracksResources()
{
	REF_PTR_RELEASE(m_stageZeroTexture);
	m_haveAnchor=false;
	m_haveCap=true;
	m_topIndex=0;
	m_bottomIndex=0;
	m_activeEdgeCount=0;
	m_totalEdgesAdded=0;
	m_ownerDrawable = nullptr;

	return 0;
}

//=============================================================================
// TerrainTracksRenderObjClass::init
//=============================================================================
/** Setup size settings and allocate W3D texture */
//=============================================================================
void TerrainTracksRenderObjClass::init( Real width, Real length, const Char *texturename)
{
	freeTerrainTracksResources();	//free old data and ib/vb

	m_boundingSphere.Init(Vector3(0,0,0),400*MAP_XY_FACTOR);
	m_boundingBox.Center.Set(0.0f, 0.0f, 0.0f);
	m_boundingBox.Extent.Set(400.0f*MAP_XY_FACTOR, 400.0f*MAP_XY_FACTOR, 1.0f);
	m_width=width;
	m_length=length;
	//no sense culling these things since they have very irregular shape and fade
	//out over time.
	Set_Force_Visible(TRUE);
	m_stageZeroTexture=W3DAssetCatalog::Get_Instance()->Get_Texture(texturename);
}

//=============================================================================
// TerrainTracksRenderObjClass::addCapEdgeToTrack
//=============================================================================
/** Cap the current track (adding an feathered edge) so we're ready to resume
		the track at a new location.  Used by objects entering FOW where we need to
		stop adding edges to the track when they enter the fog boundary but resume
		elsewhere if they become visible again.
*/
//=============================================================================
void TerrainTracksRenderObjClass::addCapEdgeToTrack(Real x, Real y)
{
	/// @todo: Have object pass its height and orientation so we can remove extra calls.

	if (m_haveCap)
	{	//we already have a cap or there are no segments to cap
		return;
	}

	if (m_activeEdgeCount == 1)
	{	//if we only have one edge, then it must be the current anchor edge.
		//since achnors are caps, there is not point in adding another.
		m_haveCap=TRUE;
		m_haveAnchor=false;	//recreate a new anchor when track resumes.
		return;
	}

	Vector3	vPos,vZ;
	Coord3D vZTmp;
	PathfindLayerEnum objectLayer;
	Real eHeight;

	if (m_ownerDrawable && (objectLayer=m_ownerDrawable->getObject()->getLayer()) != LAYER_GROUND)
		eHeight=BRIDGE_OFFSET_FACTOR+TheTerrainLogic->getLayerHeight(x,y,objectLayer,&vZTmp);
	else
		eHeight=TheTerrainLogic->getGroundHeight(x,y,&vZTmp);

	vZ.X = vZTmp.x;
	vZ.Y = vZTmp.y;
	vZ.Z = vZTmp.z;

	vPos.X=x;
	vPos.Y=y;
	vPos.Z=eHeight;

	Vector3	vDir=Vector3(x,y,eHeight)-m_lastAnchor;
	Int maxEdgeCount=TheTerrainTracksRenderObjClassSystem->m_maxTankTrackEdges;

	//avoid sqrt() by checking distance squared since last track mark
	if (vDir.Length2() < sqr(m_length))
	{	//not far enough from anchor to add track
		//since this is a  cap, we'll force the previous segment to transparent
		Int lastAddedEdge=m_topIndex-1;
		if (lastAddedEdge < 0)
			lastAddedEdge = maxEdgeCount-1;
		m_edges[lastAddedEdge].alpha=0.0f;	//force the last added edge to transparent.
		m_haveCap=TRUE;
		m_haveAnchor=false;	//recreate a new anchor when track resumes.
		return;
	}

	if (m_activeEdgeCount >= maxEdgeCount)
	{	//no more room in buffer so release oldest edge
		m_bottomIndex++;
		m_activeEdgeCount--;

		if (m_bottomIndex >= maxEdgeCount)
			m_bottomIndex=0;	//roll buffer back to start
	}

	if (m_topIndex >= maxEdgeCount)
		m_topIndex=0;	//roll around buffer

	//we traveled far enough from last point.
	//accept new point
	vDir.Z=0;	//ignore height
	vDir.Normalize();

	Vector3	vX;

	Vector3::Cross_Product(vDir,vZ,&vX);

	//calculate left end point
	edgeInfo& topEdge = m_edges[m_topIndex];

	topEdge.endPointPos[0]=vPos-(m_width*0.5f*vX);	///@todo: try getting height at endpoint
	topEdge.endPointPos[0].Z += 0.2f * MAP_XY_FACTOR;	//raise above terrain slightly

	if (m_totalEdgesAdded&1)	//every other edge has different set of UV's
	{
		topEdge.endPointUV[0].X=0.0f;
		topEdge.endPointUV[0].Y=0.0f;
	}
	else
	{
		topEdge.endPointUV[0].X=0.0f;
		topEdge.endPointUV[0].Y=1.0f;
	}

	//calculate right end point
	topEdge.endPointPos[1]=vPos+(m_width*0.5f*vX);	///@todo: try getting height at endpoint
	topEdge.endPointPos[1].Z += 0.2f * MAP_XY_FACTOR;	//raise above terrain slightly

	if (m_totalEdgesAdded&1)	//every other edge has different set of UV's
	{
		topEdge.endPointUV[1].X=1.0f;
		topEdge.endPointUV[1].Y=0.0f;
	}
	else
	{
		topEdge.endPointUV[1].X=1.0f;
		topEdge.endPointUV[1].Y=1.0f;
	}

	topEdge.timeAdded=Graphics::Get_Render_Clock().Sync_Time();
	topEdge.alpha=0.0f;	//fully transparent at cap.
	m_lastAnchor=vPos;
	m_activeEdgeCount++;
	m_totalEdgesAdded++;
	m_topIndex++;	//make space for new edge
	m_haveCap=TRUE;
	m_haveAnchor=false;
}

//=============================================================================
// TerrainTracksRenderObjClass::addEdgeToTrack
//=============================================================================
/** Try to add an additional segment to track mark.  Will do nothing if distance
* from last edge is too small.  Will overwrite the oldest edge if maximum track
* length is reached.  Oldest edges should by faded out by that time.
*/
//=============================================================================
void TerrainTracksRenderObjClass::addEdgeToTrack(Real x, Real y)
{
	/// @todo: Have object pass its height and orientation so we can remove extra calls.

	if (!m_haveAnchor)
	{	//no anchor yet, make this point an anchor.
		PathfindLayerEnum objectLayer;
		if (m_ownerDrawable && (objectLayer=m_ownerDrawable->getObject()->getLayer()) != LAYER_GROUND)
			m_lastAnchor=Vector3(x,y,TheTerrainLogic->getLayerHeight(x,y,objectLayer)+BRIDGE_OFFSET_FACTOR);
		else
			m_lastAnchor=Vector3(x,y,TheTerrainLogic->getGroundHeight(x,y));

		m_haveAnchor=true;
		m_airborne = true;
		m_haveCap = true;	//single segment tracks are always capped because nothing is drawn.
		return;
	}

	m_haveCap = false;	//have more than 1 segment now so will need to cap if it's interrupted.

	Vector3	vPos,vZ;
	Coord3D vZTmp;
	Real eHeight;
	PathfindLayerEnum objectLayer;

	if (m_ownerDrawable && (objectLayer=m_ownerDrawable->getObject()->getLayer()) != LAYER_GROUND)
		eHeight=BRIDGE_OFFSET_FACTOR+TheTerrainLogic->getLayerHeight(x,y,objectLayer,&vZTmp);
	else
		eHeight=TheTerrainLogic->getGroundHeight(x,y,&vZTmp);

	vZ.X = vZTmp.x;
	vZ.Y = vZTmp.y;
	vZ.Z = vZTmp.z;

	vPos.X=x;
	vPos.Y=y;
	vPos.Z=eHeight;

	Vector3	vDir=Vector3(x,y,eHeight)-m_lastAnchor;

	//avoid sqrt() by checking distance squared since last track mark
	if (vDir.Length2() < sqr(m_length))
		return;	//not far enough from anchor to add track

	Int maxEdgeCount=TheTerrainTracksRenderObjClassSystem->m_maxTankTrackEdges;

	if (m_activeEdgeCount >= maxEdgeCount)
	{	//no more room in buffer so release oldest edge
		m_bottomIndex++;
		m_activeEdgeCount--;

		if (m_bottomIndex >= maxEdgeCount)
			m_bottomIndex=0;	//roll buffer back to start
	}

	if (m_topIndex >= maxEdgeCount)
		m_topIndex=0;	//roll around buffer

	//we traveled far enough from last point.
	//accept new point
	vDir.Z=0;	//ignore height
	vDir.Normalize();

	Vector3	vX;

	Vector3::Cross_Product(vDir,vZ,&vX);

	edgeInfo& topEdge = m_edges[m_topIndex];

	//calculate left end point
	topEdge.endPointPos[0]=vPos-(m_width*0.5f*vX);	///@todo: try getting height at endpoint
	topEdge.endPointPos[0].Z += 0.2f * MAP_XY_FACTOR;	//raise above terrain slightly

	if (m_totalEdgesAdded&1)	//every other edge has different set of UV's
	{
		topEdge.endPointUV[0].X=0.0f;
		topEdge.endPointUV[0].Y=0.0f;
	}
	else
	{
		topEdge.endPointUV[0].X=0.0f;
		topEdge.endPointUV[0].Y=1.0f;
	}

	//calculate right end point
	topEdge.endPointPos[1]=vPos+(m_width*0.5f*vX);	///@todo: try getting height at endpoint
	topEdge.endPointPos[1].Z += 0.2f * MAP_XY_FACTOR;	//raise above terrain slightly

	if (m_totalEdgesAdded&1)	//every other edge has different set of UV's
	{
		topEdge.endPointUV[1].X=1.0f;
		topEdge.endPointUV[1].Y=0.0f;
	}
	else
	{
		topEdge.endPointUV[1].X=1.0f;
		topEdge.endPointUV[1].Y=1.0f;
	}

	topEdge.timeAdded=Graphics::Get_Render_Clock().Sync_Time();
	topEdge.alpha=1.0f;	//fully opaque at start.
	if (m_airborne || m_activeEdgeCount <= 1) {
		topEdge.alpha=0.0f;	//smooth out track restarts by setting transparent
	}
	m_airborne = false;

	m_lastAnchor=vPos;
	m_activeEdgeCount++;
	m_totalEdgesAdded++;
	m_topIndex++;	//make space for new edge
}

//=============================================================================
// TerrainTracksRenderObjClass::Render
//=============================================================================
/** Does nothing.  Just increments a counter of how many track edges were
*  requested for rendering this frame.  Actual rendering is done in flush().
*/
//=============================================================================
void TerrainTracksRenderObjClass::Render(W3DRenderContext & rinfo)
{	///@todo: After adding track mark visibility tests, add visible marks to another list.
	if (TheGlobalData->m_makeTrackMarks && m_activeEdgeCount >= 2)
		TheTerrainTracksRenderObjClassSystem->m_edgesToFlush += m_activeEdgeCount;
}

#define DEFAULT_TRACK_SPACING  (MAP_XY_FACTOR  * 1.4f)
#define DEFAULT_TRACK_WIDTH	4.0f;

/**Find distance between the "trackfx" bones of the model.  This tells us the correct
   width for the trackmarks.
*/
static Real computeTrackSpacing(W3DRenderObject *renderObj)
{
	Real trackSpacing = DEFAULT_TRACK_SPACING;
	Int leftTrack;
	Int rightTrack;

	if ((leftTrack=renderObj->Get_Bone_Index( "TREADFX01" )) != 0 && (rightTrack=renderObj->Get_Bone_Index( "TREADFX02" )) != 0)
	{	//both bones found, determine distance between them.
		Vector3 leftPos,rightPos;
		leftPos=renderObj->Get_Bone_Transform( leftTrack ).Get_Translation();
		rightPos=renderObj->Get_Bone_Transform( rightTrack ).Get_Translation();
		rightPos -= leftPos;	//get distance between centers of tracks
		trackSpacing = rightPos.Length() + DEFAULT_TRACK_WIDTH;	//add width of each track
		///@todo: It's assumed that all tank treads have the same width.
	};

	return trackSpacing;
}

//=============================================================================
//TerrainTracksRenderObjClassSystem::bindTrack
//=============================================================================
/** Grab a track from the free store. If no free tracks exist, return null.
	As long as a track is bound to an object (like a tank) it is ready to accept
	updates with additional edges.  Once it is unbound, it will expire and return
	to the free store once all tracks have faded out.

	Input: width in world units of each track edge (should probably width of vehicle).
		   length in world units between edges.  Shorter lengths produce more edges and
		   smoother curves.
		   texture to use for the tracks - image should be symetrical and include alpha channel.
*/
//=============================================================================
TerrainTracksRenderObjClass *TerrainTracksRenderObjClassSystem::bindTrack( W3DRenderObject *renderObject, Real length, const Char *texturename)
{
	TerrainTracksRenderObjClass *mod;

	mod = m_freeModules;
	if( mod )
	{
		// take module off the free list
		if( mod->m_nextSystem )
			mod->m_nextSystem->m_prevSystem = mod->m_prevSystem;
		if( mod->m_prevSystem )
			mod->m_prevSystem->m_nextSystem = mod->m_nextSystem;
		else
			m_freeModules = mod->m_nextSystem;

		// put module on the used list
		mod->m_prevSystem = nullptr;
		mod->m_nextSystem = m_usedModules;
		if( m_usedModules )
			m_usedModules->m_prevSystem = mod;
		m_usedModules = mod;

		mod->init(computeTrackSpacing(renderObject),length,texturename);
		mod->m_bound=true;
		m_TerrainTracksScene->Add_Render_Object( mod);
	}

	return mod;

}

//=============================================================================
//TerrainTracksRenderObjClassSystem::unbindTrack
//=============================================================================
/** Called when an object (i.e Tank) will not lay down any more tracks and
doesn't need this object anymore.  The track-laying object will be returned
to pool of available tracks as soon as any remaining track edges have faded out.
*/
//=============================================================================
void TerrainTracksRenderObjClassSystem::unbindTrack( TerrainTracksRenderObjClass *mod )
{
	//this object should return to free store as soon as there is nothing
	//left to render.
	mod->m_bound=false;
	mod->m_ownerDrawable = nullptr;
}

//=============================================================================
//TerrainTracksRenderObjClassSystem::releaseTrack
//=============================================================================
/** Returns a track laying object to free store to be used again later.
*/
void TerrainTracksRenderObjClassSystem::releaseTrack( TerrainTracksRenderObjClass *mod )
{
	if (mod==nullptr)
		return;

	DEBUG_ASSERTCRASH(mod->m_bound == false, ("mod is bound."));

	// remove module from used list
	if( mod->m_nextSystem )
		mod->m_nextSystem->m_prevSystem = mod->m_prevSystem;
	if( mod->m_prevSystem )
		mod->m_prevSystem->m_nextSystem = mod->m_nextSystem;
	else
		m_usedModules = mod->m_nextSystem;

	// add module to free list
	mod->m_prevSystem = nullptr;
	mod->m_nextSystem = m_freeModules;
	if( m_freeModules )
		m_freeModules->m_prevSystem = mod;
	m_freeModules = mod;
	mod->freeTerrainTracksResources();
	m_TerrainTracksScene->Remove_Render_Object(mod);
}

//=============================================================================
// TerrainTracksRenderObjClassSystem::TerrainTracksRenderObjClassSystem
//=============================================================================
/** Constructor. Just nulls out some variables. */
//=============================================================================
TerrainTracksRenderObjClassSystem::TerrainTracksRenderObjClassSystem()
{
	m_usedModules = nullptr;
	m_freeModules = nullptr;
	m_TerrainTracksScene = nullptr;
	m_edgesToFlush = 0;

	m_maxTankTrackEdges=TheGlobalData->m_maxTankTrackEdges;
	m_maxTankTrackOpaqueEdges=TheGlobalData->m_maxTankTrackOpaqueEdges;
	m_maxTankTrackFadeDelay=TheGlobalData->m_maxTankTrackFadeDelay;
}

//=============================================================================
// TerrainTracksRenderObjClassSystem::~TerrainTracksRenderObjClassSystem
//=============================================================================
/** Destructor.  Free all pre-allocated track laying render objects*/
//=============================================================================
TerrainTracksRenderObjClassSystem::~TerrainTracksRenderObjClassSystem()
{

	// free all data
	shutdown();

	m_TerrainTracksScene=nullptr;

}

//=============================================================================
// TerrainTracksRenderObjClassSystem::ReAcquireResources
//=============================================================================
/** (Re)allocates all W3D assets after a reset.. */
//=============================================================================
void TerrainTracksRenderObjClassSystem::ReAcquireResources()
{
    // The graphics renderer recreates GPU resources lazily after device reset.
}

//=============================================================================
// TerrainTracksRenderObjClassSystem::ReleaseResources
//=============================================================================
/** (Re)allocates all W3D assets after a reset.. */
//=============================================================================
void TerrainTracksRenderObjClassSystem::ReleaseResources()
{
    Graphics::Get_Surface_Renderer().Destroy_Mesh(m_graphicsMesh);
    m_graphicsMesh = {};
}

//=============================================================================
// TerrainTracksRenderObjClassSystem::init
//=============================================================================
/**  initialize the system, allocate all the render objects we will need */
//=============================================================================
void TerrainTracksRenderObjClassSystem::init( W3DScene *TerrainTracksScene )
{
	const Int numModules=TheGlobalData->m_maxTerrainTracks;

	Int i;
	TerrainTracksRenderObjClass *mod;

	m_TerrainTracksScene=TerrainTracksScene;

	ReAcquireResources();
	//go with a preset material for now.

	//use a multi-texture shader: (text1*diffuse)*text2.

	// we cannot initialize a system that is already initialized
	if( m_freeModules || m_usedModules )
	{

		// system already online!
		assert( 0 );
		return;

	}

	// allocate our modules for this system
	for( i = 0; i < numModules; i++ )
	{

		mod = NEW_REF( TerrainTracksRenderObjClass, () );

		if( mod == nullptr )
		{

			// unable to allocate modules needed
			assert( 0 );
			return;

		}

		mod->m_prevSystem = nullptr;
		mod->m_nextSystem = m_freeModules;
		if( m_freeModules )
			m_freeModules->m_prevSystem = mod;
		m_freeModules = mod;

	}

}

//=============================================================================
// TerrainTracksRenderObjClassSystem::shutdown
//=============================================================================
/** Shutdown and free all memory for this system */
//=============================================================================
void TerrainTracksRenderObjClassSystem::shutdown()
{
    ReleaseResources();
	TerrainTracksRenderObjClass *nextMod,*mod;

	//release unbound tracks that may still be fading out
	mod=m_usedModules;

	while(mod)
	{
		nextMod=mod->m_nextSystem;

		if (!mod->m_bound)
			releaseTrack(mod);

		mod = nextMod;
	}


	// free all attached things and used modules
	assert( m_usedModules == nullptr );

	// free all module storage
	while( m_freeModules )
	{

		nextMod = m_freeModules->m_nextSystem;
		REF_PTR_RELEASE (m_freeModules);
		m_freeModules = nextMod;

	}


}

//=============================================================================
// TerrainTracksRenderObjClassSystem::update
//=============================================================================
/** Update the state of all active track marks - fade, expire, etc. */
//=============================================================================
void TerrainTracksRenderObjClassSystem::update()
{

	Int		iTime=Graphics::Get_Render_Clock().Sync_Time();
	Real	iDiff;
	TerrainTracksRenderObjClass *mod=m_usedModules,*nextMod;

	//first update all the tracks
	while( mod )
	{
		Int i,index;
		Vector3 *endPoint;
		Vector2 *endPointUV;

		nextMod = mod->m_nextSystem;

		if (!TheGlobalData->m_makeTrackMarks)
			mod->m_haveAnchor=false;	//force a track restart next time around.

		for (i=0,index=mod->m_bottomIndex; i<mod->m_activeEdgeCount; i++,index++)
		{
			if (index >= m_maxTankTrackEdges)
				index=0;

			endPoint=&mod->m_edges[index].endPointPos[0];	//left endpoint
			endPointUV=&mod->m_edges[index].endPointUV[0];
			iDiff=(float)(iTime-mod->m_edges[index].timeAdded);
			iDiff = 1.0f - iDiff/(Real)m_maxTankTrackFadeDelay;
			if (iDiff < 0.0)
				iDiff=0.0f;
			if (mod->m_edges[index].alpha>0.0f) {
				mod->m_edges[index].alpha=iDiff;
			}

			if (iDiff == 0.0f)
			{	//this edge was invisible, we can remove it
				mod->m_bottomIndex++;
				mod->m_activeEdgeCount--;

				if (mod->m_bottomIndex >= m_maxTankTrackEdges)
					mod->m_bottomIndex=0;	//roll buffer back to start
			}
			if (mod->m_activeEdgeCount == 0 && !mod->m_bound)
				releaseTrack(mod);
		}
		mod = nextMod;
	}
}


//=============================================================================
// TerrainTracksRenderObjClassSystem::flush
//=============================================================================
/** Draw all active track marks for this frame */
//=============================================================================
void TerrainTracksRenderObjClassSystem::flush(W3DCamera& camera)
{
    if (Get_W3D_Render_Services().Is_Reflection_Render_Pass()) return;
    auto* device = Graphics::Shared_Frame_Device();
    if (!device || !m_usedModules || m_edgesToFlush < 2) {
        m_edgesToFlush = 0;
        return;
    }
    const auto& ambient = TheGlobalData->m_terrainAmbient[0];
    const auto& diffuse = TheGlobalData->m_terrainDiffuse[0];
    const UnsignedInt packed = REAL_TO_INT((ambient.blue + diffuse.blue/2)*255)
        | (REAL_TO_INT((ambient.green + diffuse.green/2)*255) << 8)
        | (REAL_TO_INT((ambient.red + diffuse.red/2)*255) << 16);
    const std::array<float,3> color{((packed>>16)&255)/255.0f,((packed>>8)&255)/255.0f,(packed&255)/255.0f};
    const auto parameters = Make_Surface_Parameters(camera);
    Graphics::SurfaceStyle style;
    style.cull = Graphics::RHICullMode::Back;
    style.front_counter_clockwise = true;
    auto& renderer = Graphics::Get_Surface_Renderer();
    std::vector<Graphics::TrackEdge> edges;
    Graphics::TrackGeometry geometry;
    for (auto* mod=m_usedModules; mod; mod=mod->m_nextSystem) {
        if (mod->m_activeEdgeCount < 2 || !mod->Is_Really_Visible()) continue;
        edges.clear();
        for (Int i=0,index=mod->m_bottomIndex; i<mod->m_activeEdgeCount; ++i,++index) {
            if (index >= m_maxTankTrackEdges) index=0;
            const auto& source = mod->m_edges[index];
            Graphics::TrackEdge edge;
            edge.alpha = source.alpha;
            for (unsigned side=0; side<2; ++side) {
                Vector3 position;
                Matrix3D::Transform_Vector(mod->Transform, source.endPointPos[side], &position);
                edge.positions[side] = {position.X,position.Y,position.Z};
                edge.uv[side] = {source.endPointUV[side].X,source.endPointUV[side].Y};
            }
            edges.push_back(edge);
        }
        geometry.Build(edges,m_maxTankTrackEdges,m_maxTankTrackOpaqueEdges,color);
        if (m_graphicsMesh.Is_Valid()) renderer.Update_Mesh(m_graphicsMesh,geometry.vertices,geometry.indices);
        else m_graphicsMesh = renderer.Create_Mesh(geometry.vertices,geometry.indices);
        const std::array<Graphics::RHITextureHandle,4> textures{Resolve_Graphics_Texture(mod->m_stageZeroTexture),{},{},{}};
        renderer.Draw(device->Immediate_Command_List(),m_graphicsMesh,style,parameters,textures);
    }
    m_edgesToFlush = 0;
}

/**Removes all remaining tracks from the rendering system*/
void TerrainTracksRenderObjClassSystem::Reset()
{
	TerrainTracksRenderObjClass *nextMod,*mod=m_usedModules;

	while(mod)
	{
		nextMod=mod->m_nextSystem;

		releaseTrack(mod);

		mod = nextMod;
	}


	// free all attached things and used modules
	assert( m_usedModules == nullptr );
	m_edgesToFlush=0;
}

/**Clear the treads from each track laying object without freeing the objects.
Mostly used when user changed LOD level*/
void TerrainTracksRenderObjClassSystem::clearTracks()
{
	TerrainTracksRenderObjClass *mod=m_usedModules;

	while(mod)
	{
		mod->m_haveAnchor=false;
		mod->m_haveCap=true;
		mod->m_topIndex=0;
		mod->m_bottomIndex=0;
		mod->m_activeEdgeCount=0;
		mod->m_totalEdgesAdded=0;

		mod = mod->m_nextSystem;
	}

	m_edgesToFlush=0;
}

/**Adjust various paremeters which affect the cost of rendering tracks on the map.
Parameters are passed via GlobalData*/
void TerrainTracksRenderObjClassSystem::setDetail()
{
	//Remove all existing track segments from screen.
	clearTracks();
	ReleaseResources();

	m_maxTankTrackEdges=TheGlobalData->m_maxTankTrackEdges;
	m_maxTankTrackOpaqueEdges=TheGlobalData->m_maxTankTrackOpaqueEdges;
	m_maxTankTrackFadeDelay=TheGlobalData->m_maxTankTrackFadeDelay;

	//We changed the maximum number of visible edges so re-allocate our resources to match.
	ReAcquireResources();
};

TerrainTracksRenderObjClassSystem *TheTerrainTracksRenderObjClassSystem=nullptr;	///< singleton for track drawing system.
