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

#pragma once

#ifndef __W3DTERRAINTRACKS_H_
#define __W3DTERRAINTRACKS_H_

#include "always.h"
#include "rendobj.h"
#include "w3d_file.h"
#include "dx8vertexbuffer.h"
#include "dx8indexbuffer.h"
#include "shader.h"
#include "vertmaterial.h"
#include "Lib/BaseType.h"

#define MAX_TRACK_EDGE_COUNT	100	//maximum number of edges or divisions in track mark
#define MAX_TRACK_OPAQUE_EDGE	25	//linear fade of edges will begin at this edge
#define FADE_TIME_FRAMES 300000	// 300 seconds at 30 fps - time to fade out an edge and remove it from the system.

class TerrainTracksRenderObjClassSystem;
class Drawable;

/// Custom render object that draws tracks on the terrain.
/**
This render object handles drawing tracks left by objects moving on the terrain.
*/
class TerrainTracksRenderObjClass : public W3DMPO, public RenderObjClass
{
	W3DMPO_GLUE(TerrainTracksRenderObjClass)

	friend class TerrainTracksRenderObjClassSystem;

public:

	TerrainTracksRenderObjClass(void);
	~TerrainTracksRenderObjClass(void);

	/////////////////////////////////////////////////////////////////////////////
	// Render Object Interface (W3D methods)
	/////////////////////////////////////////////////////////////////////////////
	virtual RenderObjClass *	Clone(void) const;
	virtual int						Class_ID(void) const;
	virtual void					Render(RenderInfoClass & rinfo);
	virtual void					Get_Obj_Space_Bounding_Sphere(SphereClass & sphere) const;
    virtual void					Get_Obj_Space_Bounding_Box(AABoxClass & aabox) const;

	Int freeTerrainTracksResources(void);	///<free W3D assets used for this track
	void init( Real width, Real length, const Char *texturename);	///<allocate W3D resources and set size
	void addEdgeToTrack(Real x, Real y);	///< add a new segment to the track
	void addCapEdgeToTrack(Real x, Real y);	///< cap the existing segment so we can resume at an unconnected position.
	void setAirborne(void) {m_airborne = true; }	///< Starts a new section of track, generally after going airborne.
	void setOwnerDrawable(const Drawable *owner) {m_ownerDrawable = owner;}

protected:
	TextureClass *m_stageZeroTexture;	///<primary texture
	SphereClass	m_boundingSphere;		///<bounding sphere of TerrainTracks
	AABoxClass	m_boundingBox;			///<bounding box of TerrainTracks
	Int			m_activeEdgeCount;			///<number of active edges in segment list
	Int			m_totalEdgesAdded;		///<number of edges ever added to this track
	const Drawable	*m_ownerDrawable;	///<logical object that's laying down tread marks.

	struct edgeInfo{
		Vector3	endPointPos[2];			///<the 2 endpoints on the edge
		Vector2	endPointUV[2];			///< uv coordinates at each end point
		Int		timeAdded;				///< time when edge was created.
		Real	alpha;					///< current alpha value for rendering
	};

	edgeInfo	m_edges[MAX_TRACK_EDGE_COUNT];	///<edges at each segment break
	Vector3		m_lastAnchor;						///<location of last edge center
	Int			m_bottomIndex;						///<points at oldest edge on track
	Int			m_topIndex;						///<points to newest edge on track
	Bool		m_haveAnchor;					///<set to false until first edge is added
	Bool		m_bound;						///<object is bound to owner and accepts new edges
	Real		m_width;						///<track width
	Real		m_length;						///<length of each track segment
	Bool		m_airborne;					///< Did the vehicle bounce up into the air?
	Bool		m_haveCap;					///< is the segment capped so we can stop and resume at new location.
	TerrainTracksRenderObjClass	*m_nextSystem;			///<next track system
	TerrainTracksRenderObjClass *m_prevSystem;			///<previous track system
};

/// System for drawing, updating, and re-using tread mark render objects.
/**
This system keeps track of all the active track mark objects and reuses them
when they expire.  It also renders all the track marks that were submitted in
this frame.
*/

class TerrainTracksRenderObjClassSystem
{
	friend class TerrainTracksRenderObjClass;

public:

	TerrainTracksRenderObjClassSystem( void );
	~TerrainTracksRenderObjClassSystem( void );

	void ReleaseResources(void);	///< Release all dx8 resources so the device can be reset.
	void ReAcquireResources(void);  ///< Reacquire all resources after device reset.

	void setDetail(void);

	void flush (void);	///<draw all tracks that were requested for rendering.
	void update(void);	///<update the state of all edges (fade alpha, remove old, etc.)

	void init( SceneClass *TerrainTracksScene);	///< pre-allocate track objects
	void shutdown( void );		///< release all pre-allocated track objects, called by destructor
	void Reset(void);	///<empties the system, ready for a new scene.

	TerrainTracksRenderObjClass *bindTrack(RenderObjClass *renderObject, Real length, const Char *texturename);	///<track object to be controlled by owner
	void unbindTrack( TerrainTracksRenderObjClass *mod );	///<releases control of track object

protected:
	DX8VertexBufferClass		*m_vertexBuffer;	///<vertex buffer used to draw all tracks
	DX8IndexBufferClass			*m_indexBuffer;	///<indices defining triangles in maximum length track
	VertexMaterialClass	  	  *m_vertexMaterialClass;	///< vertex lighting material
	ShaderClass m_shaderClass; ///<shader or rendering state for heightmap

	TerrainTracksRenderObjClass *m_usedModules;	///<active objects being rendered in the scene
	TerrainTracksRenderObjClass *m_freeModules;	//<unused modules that are free to use again
	SceneClass	*m_TerrainTracksScene;		///<scene that will contain all the TerrainTracks

	Int	m_edgesToFlush;			///< number of edges to flush on next render.

	void releaseTrack( TerrainTracksRenderObjClass *mod );	///<returns track object to free store.
	void clearTracks(void);	///<reset the amount of visible track marks of each object.

	Int m_maxTankTrackEdges;	///<maximum length of tank track
	Int m_maxTankTrackOpaqueEdges;	///<maximum length of tank track before it starts fading.
	Int m_maxTankTrackFadeDelay;	///<maximum amount of time a tank track segment remains visible.

};  // end class TerrainTracksRenderObjClassSystem

extern TerrainTracksRenderObjClassSystem *TheTerrainTracksRenderObjClassSystem; ///< singleton for track drawing system.

#endif  // end __W3DTERRAINTRACKS_H_
