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

#include "W3DDevice/GameClient/W3DTerrainTracks.h"
#include "W3DDevice/GameClient/HeightMap.h"
#include "Common/PerfTimer.h"
#include "Common/GlobalData.h"
#include "Common/Debug.h"
#include "texture.h"
#include "colmath.h"
#include "coltest.h"
#include "rinfo.h"
#include "camera.h"
#include "assetmgr.h"
#include "WW3D2/dx8wrapper.h"
#include "WW3D2/scene.h"
#include "GameLogic/TerrainLogic.h"
#include "GameLogic/Object.h"
#include "GameClient/Drawable.h"

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

#define BRIDGE_OFFSET_FACTOR	0.25f	//amount to raise tracks above bridges.
//=============================================================================
// TerrainTracksRenderObjClass::~TerrainTracksRenderObjClass
//=============================================================================
/** Destructor. Releases w3d assets. */
//=============================================================================
TerrainTracksRenderObjClass::~TerrainTracksRenderObjClass(void)
{
	freeTerrainTracksResources();
}

//=============================================================================
// TerrainTracksRenderObjClass::TerrainTracksRenderObjClass
//=============================================================================
/** Constructor. Just nulls out some variables. */
//=============================================================================
TerrainTracksRenderObjClass::TerrainTracksRenderObjClass(void)
{
	m_stageZeroTexture=NULL;
	m_lastAnchor=Vector3(0,1,2.25);
	m_haveAnchor=false;
	m_haveCap=true;
	m_topIndex=0;
	m_bottomIndex=0;
	m_activeEdgeCount=0;
	m_totalEdgesAdded=0;
	m_bound=false;
	m_ownerDrawable = NULL;
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
Int TerrainTracksRenderObjClass::Class_ID(void) const
{
	return RenderObjClass::CLASSID_IMAGE3D;
}

//=============================================================================
// TerrainTracksRenderObjClass::Clone
//=============================================================================
/** Not used, but required virtual method. */
//=============================================================================
RenderObjClass *	 TerrainTracksRenderObjClass::Clone(void) const
{
	assert(false);
	return NULL;
}

//=============================================================================
// TerrainTracksRenderObjClass::freeTerrainTracksResources
//=============================================================================
/** Free any W3D resources associated with this object */
//=============================================================================
Int TerrainTracksRenderObjClass::freeTerrainTracksResources(void)
{
	REF_PTR_RELEASE(m_stageZeroTexture);
	m_haveAnchor=false;
	m_haveCap=true;
	m_topIndex=0;
	m_bottomIndex=0;
	m_activeEdgeCount=0;
	m_totalEdgesAdded=0;
	m_ownerDrawable = NULL;

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
	m_stageZeroTexture=WW3DAssetManager::Get_Instance()->Get_Texture(texturename);
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

	topEdge.timeAdded=WW3D::Get_Sync_Time();
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

	topEdge.timeAdded=WW3D::Get_Sync_Time();
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
void TerrainTracksRenderObjClass::Render(RenderInfoClass & rinfo)
{	///@todo: After adding track mark visibility tests, add visible marks to another list.
	if (TheGlobalData->m_makeTrackMarks && m_activeEdgeCount >= 2)
		TheTerrainTracksRenderObjClassSystem->m_edgesToFlush += m_activeEdgeCount;
}

#define DEFAULT_TRACK_SPACING  (MAP_XY_FACTOR  * 1.4f)
#define DEFAULT_TRACK_WIDTH	4.0f;

/**Find distance between the "trackfx" bones of the model.  This tells us the correct
   width for the trackmarks.
*/
static Real computeTrackSpacing(RenderObjClass *renderObj)
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
/** Grab a track from the free store. If no free tracks exist, return NULL.
	As long as a track is bound to an object (like a tank) it is ready to accept
	updates with additional edges.  Once it is unbound, it will expire and return
	to the free store once all tracks have faded out.

	Input: width in world units of each track edge (should probably width of vehicle).
		   length in world units between edges.  Shorter lengths produce more edges and
		   smoother curves.
		   texture to use for the tracks - image should be symetrical and include alpha channel.
*/
//=============================================================================
TerrainTracksRenderObjClass *TerrainTracksRenderObjClassSystem::bindTrack( RenderObjClass *renderObject, Real length, const Char *texturename)
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
		mod->m_prevSystem = NULL;
		mod->m_nextSystem = m_usedModules;
		if( m_usedModules )
			m_usedModules->m_prevSystem = mod;
		m_usedModules = mod;

		mod->init(computeTrackSpacing(renderObject),length,texturename);
		mod->m_bound=true;
		m_TerrainTracksScene->Add_Render_Object( mod);
	}  // end if

	return mod;

}  //end bindTrack

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
	mod->m_ownerDrawable = NULL;
}

//=============================================================================
//TerrainTracksRenderObjClassSystem::releaseTrack
//=============================================================================
/** Returns a track laying object to free store to be used again later.
*/
void TerrainTracksRenderObjClassSystem::releaseTrack( TerrainTracksRenderObjClass *mod )
{
	if (mod==NULL)
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
	mod->m_prevSystem = NULL;
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
	m_usedModules = NULL;
	m_freeModules = NULL;
	m_TerrainTracksScene = NULL;
	m_edgesToFlush = 0;
	m_indexBuffer = NULL;
	m_vertexMaterialClass = NULL;
	m_vertexBuffer = NULL;

	m_maxTankTrackEdges=TheGlobalData->m_maxTankTrackEdges;
	m_maxTankTrackOpaqueEdges=TheGlobalData->m_maxTankTrackOpaqueEdges;
	m_maxTankTrackFadeDelay=TheGlobalData->m_maxTankTrackFadeDelay;
}

//=============================================================================
// TerrainTracksRenderObjClassSystem::~TerrainTracksRenderObjClassSystem
//=============================================================================
/** Destructor.  Free all pre-allocated track laying render objects*/
//=============================================================================
TerrainTracksRenderObjClassSystem::~TerrainTracksRenderObjClassSystem( void )
{

	// free all data
	shutdown();

	m_vertexMaterialClass=NULL;
	m_TerrainTracksScene=NULL;

}

//=============================================================================
// TerrainTracksRenderObjClassSystem::ReAcquireResources
//=============================================================================
/** (Re)allocates all W3D assets after a reset.. */
//=============================================================================
void TerrainTracksRenderObjClassSystem::ReAcquireResources(void)
{
	Int i;
	const Int numModules=TheGlobalData->m_maxTerrainTracks;

	// just for paranoia's sake.
	REF_PTR_RELEASE(m_indexBuffer);
	REF_PTR_RELEASE(m_vertexBuffer);

	//Create static index buffers.  These will index the vertex buffers holding the track segments
	m_indexBuffer=NEW_REF(DX8IndexBufferClass,((m_maxTankTrackEdges-1)*6));

	// Fill up the IB
	{
		DX8IndexBufferClass::WriteLockClass lockIdxBuffer(m_indexBuffer);
		UnsignedShort *ib=lockIdxBuffer.Get_Index_Array();

		for (i=0; i<(m_maxTankTrackEdges-1); i++)
		{
			ib[3]=ib[0]=i*2;
			ib[1]=i*2+1;
			ib[4]=ib[2]=(i+1)*2+1;
			ib[5]=(i+1)*2;
			ib+=6;	//skip the 6 indices we just filled
		}
	}

	DEBUG_ASSERTCRASH(numModules*m_maxTankTrackEdges*2 < 65535, ("Too many terrain track edges"));

	m_vertexBuffer=NEW_REF(DX8VertexBufferClass,(DX8_FVF_XYZDUV1,numModules*m_maxTankTrackEdges*2,DX8VertexBufferClass::USAGE_DYNAMIC));
}

//=============================================================================
// TerrainTracksRenderObjClassSystem::ReleaseResources
//=============================================================================
/** (Re)allocates all W3D assets after a reset.. */
//=============================================================================
void TerrainTracksRenderObjClassSystem::ReleaseResources(void)
{
	REF_PTR_RELEASE(m_indexBuffer);
	REF_PTR_RELEASE(m_vertexBuffer);
	// Note - it is ok to not release the material, as it is a w3d object that
	// has no dx8 resources. jba.
}

//=============================================================================
// TerrainTracksRenderObjClassSystem::init
//=============================================================================
/**  initialize the system, allocate all the render objects we will need */
//=============================================================================
void TerrainTracksRenderObjClassSystem::init( SceneClass *TerrainTracksScene )
{
	const Int numModules=TheGlobalData->m_maxTerrainTracks;

	Int i;
	TerrainTracksRenderObjClass *mod;

	m_TerrainTracksScene=TerrainTracksScene;

	ReAcquireResources();
	//go with a preset material for now.
	m_vertexMaterialClass=VertexMaterialClass::Get_Preset(VertexMaterialClass::PRELIT_DIFFUSE);

	//use a multi-texture shader: (text1*diffuse)*text2.
	m_shaderClass = ShaderClass::_PresetAlphaShader;//_PresetATestSpriteShader;//_PresetOpaqueShader;

	// we cannot initialize a system that is already initialized
	if( m_freeModules || m_usedModules )
	{

		// system already online!
		assert( 0 );
		return;

	}  // end if

	// allocate our modules for this system
	for( i = 0; i < numModules; i++ )
	{

		mod = NEW_REF( TerrainTracksRenderObjClass, () );

		if( mod == NULL )
		{

			// unable to allocate modules needed
			assert( 0 );
			return;

		}  // end if

		mod->m_prevSystem = NULL;
		mod->m_nextSystem = m_freeModules;
		if( m_freeModules )
			m_freeModules->m_prevSystem = mod;
		m_freeModules = mod;

	}  // end for i

}  // end init

//=============================================================================
// TerrainTracksRenderObjClassSystem::shutdown
//=============================================================================
/** Shutdown and free all memory for this system */
//=============================================================================
void TerrainTracksRenderObjClassSystem::shutdown( void )
{
	TerrainTracksRenderObjClass *nextMod,*mod;

	//release unbound tracks that may still be fading out
	mod=m_usedModules;

	while(mod)
	{
		nextMod=mod->m_nextSystem;

		if (!mod->m_bound)
			releaseTrack(mod);

		mod = nextMod;
	}  // end while


	// free all attached things and used modules
	assert( m_usedModules == NULL );

	// free all module storage
	while( m_freeModules )
	{

		nextMod = m_freeModules->m_nextSystem;
		REF_PTR_RELEASE (m_freeModules);
		m_freeModules = nextMod;

	}  // end while

	REF_PTR_RELEASE(m_indexBuffer);
	REF_PTR_RELEASE(m_vertexMaterialClass);
	REF_PTR_RELEASE(m_vertexBuffer);

}  // end shutdown

//=============================================================================
// TerrainTracksRenderObjClassSystem::update
//=============================================================================
/** Update the state of all active track marks - fade, expire, etc. */
//=============================================================================
void TerrainTracksRenderObjClassSystem::update()
{

	Int		iTime=WW3D::Get_Sync_Time();
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
	}  // end while
}


//=============================================================================
// TerrainTracksRenderObjClassSystem::flush
//=============================================================================
/** Draw all active track marks for this frame */
//=============================================================================
void TerrainTracksRenderObjClassSystem::flush()
{
/** @todo: Optimize system by drawing tracks as triangle strips and use dynamic vertex buffer access.
May also try rendering all tracks with one call to W3D/D3D by grouping them by texture.
Try improving the fit to vertical surfaces like cliffs.
*/

	Int	diffuseLight;
	TerrainTracksRenderObjClass *mod=m_usedModules;
	if (!mod)
		return;	//nothing to render

	Int	trackStartIndex;
	Real distanceFade;

	if (ShaderClass::Is_Backface_Culling_Inverted())
		return;	//don't render track marks in reflections.

	// adjust shading for time of day.
	Real shadeR, shadeG, shadeB;
	shadeR = TheGlobalData->m_terrainAmbient[0].red;
	shadeG = TheGlobalData->m_terrainAmbient[0].green;
	shadeB = TheGlobalData->m_terrainAmbient[0].blue;
	shadeR += TheGlobalData->m_terrainDiffuse[0].red/2;
	shadeG += TheGlobalData->m_terrainDiffuse[0].green/2;
	shadeB += TheGlobalData->m_terrainDiffuse[0].blue/2;
	shadeR*=255.0f;
	shadeG*=255.0f;
	shadeB*=255.0f;

	diffuseLight = REAL_TO_INT(shadeB) | (REAL_TO_INT(shadeG) << 8) | (REAL_TO_INT(shadeR) << 16);
	Real numFadedEdges=m_maxTankTrackEdges-m_maxTankTrackOpaqueEdges;

	//check if there is anything to draw and fill vertex buffer
	if (m_edgesToFlush >= 2)
	{
		DX8VertexBufferClass::WriteLockClass lockVtxBuffer(m_vertexBuffer);
		VertexFormatXYZDUV1 *verts = (VertexFormatXYZDUV1*)lockVtxBuffer.Get_Vertex_Array();
		trackStartIndex=0;

		mod=m_usedModules;
		//Fill our vertex buffer with all the tracks
		while( mod )
		{
			Int i,index;
			Vector3 *endPoint;
			Vector2 *endPointUV;

			if (mod->m_activeEdgeCount >= 2 && mod->Is_Really_Visible())
			{
				for (i=0,index=mod->m_bottomIndex; i<mod->m_activeEdgeCount; i++,index++)
				{
					if (index >= m_maxTankTrackEdges)
						index=0;

					endPoint=&mod->m_edges[index].endPointPos[0];	//left endpoint
					endPointUV=&mod->m_edges[index].endPointUV[0];
		
					distanceFade=1.0f;

					if ((mod->m_activeEdgeCount -1 -i) >= m_maxTankTrackOpaqueEdges)// && i < (MAX_PER_TRACK_EDGE_COUNT-FORCE_FADE_AT_EDGE))
					{	//we're getting close to the limit on the number of track pieces allowed
						//so force it to fade out.
						distanceFade=1.0f-(float)((mod->m_activeEdgeCount -i)-m_maxTankTrackOpaqueEdges)/numFadedEdges;
					}

					distanceFade *= mod->m_edges[index].alpha;	//adjust fade with distance from start of track

					verts->x=endPoint->X;
					verts->y=endPoint->Y;
					verts->z=endPoint->Z;
					
					verts->u1=endPointUV->X;
					verts->v1=endPointUV->Y;

					//fade the alpha channel with distance
					verts->diffuse=diffuseLight | ( REAL_TO_INT(distanceFade*255.0f) <<24);
					verts++;

					endPoint=&mod->m_edges[index].endPointPos[1];	//right endpoint
					endPointUV=&mod->m_edges[index].endPointUV[1];

					verts->x=endPoint->X;
					verts->y=endPoint->Y;
					verts->z=endPoint->Z;

					verts->u1=endPointUV->X;
					verts->v1=endPointUV->Y;			///@todo: Add diffuse lighting.

					verts->diffuse=diffuseLight | ( REAL_TO_INT(distanceFade*255.0f) <<24);
					verts++;
				}//for
			}// mod has edges to render
			mod = mod->m_nextSystem;
		}	//while (mod)
	}//edges to flush

	//draw the filled vertex buffers
	if (m_edgesToFlush >= 2)
	{
		ShaderClass::Invalidate();
		DX8Wrapper::Set_Material(m_vertexMaterialClass);
		DX8Wrapper::Set_Shader(m_shaderClass);
		DX8Wrapper::Set_Index_Buffer(m_indexBuffer,0);
		DX8Wrapper::Set_Vertex_Buffer(m_vertexBuffer);

		trackStartIndex=0;
		mod=m_usedModules;
		Matrix3D tm(mod->Transform);
		DX8Wrapper::Set_Transform(D3DTS_WORLD,tm);
		while (mod)
		{
			if (mod->m_activeEdgeCount >= 2 && mod->Is_Really_Visible())
			{
				DX8Wrapper::Set_Texture(0,mod->m_stageZeroTexture);
				DX8Wrapper::Set_Index_Buffer_Index_Offset(trackStartIndex);
				DX8Wrapper::Draw_Triangles(	0,(mod->m_activeEdgeCount-1)*2, 0, mod->m_activeEdgeCount*2);

				trackStartIndex += mod->m_activeEdgeCount*2;
			}
			mod=mod->m_nextSystem;
		}
	}	//there are some edges to render in pool.

	m_edgesToFlush=0;	//reset count for next flush
}

/**Removes all remaining tracks from the rendering system*/
void TerrainTracksRenderObjClassSystem::Reset(void)
{
	TerrainTracksRenderObjClass *nextMod,*mod=m_usedModules;

	while(mod)
	{
		nextMod=mod->m_nextSystem;

		releaseTrack(mod);

		mod = nextMod;
	}  // end while


	// free all attached things and used modules
	assert( m_usedModules == NULL );
	m_edgesToFlush=0;
}

/**Clear the treads from each track laying object without freeing the objects.
Mostly used when user changed LOD level*/
void TerrainTracksRenderObjClassSystem::clearTracks(void)
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
	}  // end while

	m_edgesToFlush=0;
}

/**Adjust various paremeters which affect the cost of rendering tracks on the map.
Parameters are passed via GlobalData*/
void TerrainTracksRenderObjClassSystem::setDetail(void)
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

TerrainTracksRenderObjClassSystem *TheTerrainTracksRenderObjClassSystem=NULL;	///< singleton for track drawing system.
