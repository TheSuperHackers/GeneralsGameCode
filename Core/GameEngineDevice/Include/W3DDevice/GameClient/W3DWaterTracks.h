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

#include <array>
#include <span>
#include <vector>
#include "W3DDevice/GameClient/WaterMaterial.h"
#include "WWMath/aabox.h"
#include "WWMath/sphere.h"
#include "WWMath/vector2.h"

enum waveType CPP_11(: Int);	//forward reference

/// Custom render object that draws animated tracks/waves on the water.
/**
	This is an object which draws a small breaking wave or splash animation.  These objects are
	to be managed/accessed only by the WaterTracksRenderSystem
*/
class WaterTracksObj
{
	friend class WaterTracksRenderSystem;

public:

	WaterTracksObj();
	~WaterTracksObj();

	Int freeWaterTracksResources();	///<free W3D assets used for this track
	void init( Real width, Real length, const Vector2 &start, const Vector2 &end, const Char *texturename, Int waveTimeOffset);	///<allocate W3D resources and set size
	void init( Real width, const Vector2 &start, const Vector2 &end, const Char *texturename);	///<allocate W3D resources and set size
	Int	update(Int msElapsed);	///< update animation state
	void render(WaterMaterialClass& material, Graphics::WaterMeshHandle& mesh,
        std::vector<WaterSurfaceVertex>& vertices, std::span<const unsigned short> indices);	///<draw this object

protected:
	W3DTextureHandle *m_stageZeroTexture;	///<primary texture
	waveType	m_type;					///<used for render state sorting (set this to texture pointer for now).
	Int			m_x;					///<vertex count
	Int			m_y;					///<vertex count
	Bool		m_bound;				///<object is bound to owner and accepts new edges
	Vector2		m_startPos;				///<starting position of wave
//	Vector2		m_endPos;				///<ending position of wave
	Vector2		m_waveDir;				///<direction of wave travel
	Vector2		m_perpDir;				///<direction perpendicular to wave travel
	Vector2		m_initStartPos;			///<original settings used to create wave
	Vector2		m_initEndPos;			///<original settings used to create wave
	Int			m_initTimeOffset;		///<time offset when wave is added into the system
	Int		m_fadeMs;				///<time for wave to fade out after it stops moving
	Int		m_totalMs;				///<amount of time to complete full motion
	Int			m_elapsedMs;			///<amount of time since start of motion

	//New version
	Real	m_waveInitialWidth;				///<width of wave segment when it first appears
	Real	m_waveInitialHeight;			///<height of wave segment when it first appears
	Real	m_waveFinalWidth;				///<width of wave segment at full size
	Real	m_waveFinalWidthPeakFrac;		///<fraction along path when wave reaches full width
	Real	m_waveFinalHeight;				///<final height of unstretched wave

	Real m_initialVelocity;	//initial velocity in world units per ms.
	Real m_waveDistance;		//<total distance traveled by wave front.
	Real m_timeToReachBeach;
	Real m_frontSlowDownAcc;
	Real m_timeToStop;
	Real m_timeToRetreat;
	Real m_backSlowDownAcc;
	Real m_timeToCompress;
	Real m_flipU;			///<force uv coordinates to flip

	WaterTracksObj	*m_nextSystem;			///<next track in system
	WaterTracksObj *m_prevSystem;			///<previous track in system
};

/// System for drawing, updating, and re-using water mark render objects.
/**
This system keeps track of all the active track mark objects and reuses them
when they expire.  It also renders all the track marks that were submitted in
this frame.
*/

class WaterTracksRenderSystem
{
	friend class WaterTracksObj;

public:

	WaterTracksRenderSystem();
	~WaterTracksRenderSystem();

	void ReleaseResources();	///< Release all backend resources so the device can be reset.
	void ReAcquireResources();  ///< Reacquire all resources after device reset.

	void flush (W3DRenderContext & rinfo);	///<draw all tracks that were requested for rendering.
	void update();	///<update the state of all edges (fade alpha, remove old, etc.)

	void init();	///< pre-allocate track objects
	void shutdown();		///< release all pre-allocated track objects, called by destructor
	void reset();			///< free all map dependent items.

	WaterTracksObj *bindTrack(waveType type);	///<track object to be controlled by owner
	void unbindTrack( WaterTracksObj *mod );	///<releases control of track object
	void saveTracks();									///<save all used tracks to disk
	void loadTracks();									///<load tracks from disk
	WaterTracksObj *findTrack(Vector2 &start, Vector2 &end, waveType type);

protected:
    std::vector<WaterSurfaceVertex> m_vertices;
    std::vector<UnsignedShort> m_indices;
    Graphics::WaterMeshHandle m_graphicsMesh;
	WaterMaterialClass m_material;	///<explicit programmable track material

	WaterTracksObj *m_usedModules;	///<active objects being rendered in the scene
	WaterTracksObj *m_freeModules;	//<unused modules that are free to use again

	Int		m_stripSizeX;			///< resolution (vertex count) of wave strip
	Int		m_stripSizeY;			///< resolution (vertex count) of wave strip
	Real	m_level;				///< water level
	void releaseTrack( WaterTracksObj *mod );	///<returns track object to free store.
};
