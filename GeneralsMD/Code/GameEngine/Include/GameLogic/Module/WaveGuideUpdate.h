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

// FILE: WaveGuideUpdate.h ////////////////////////////////////////////////////////////////////////
// Author: Colin Day, April 2002
// Desc:   Update module for the waver wave objects to create flooding
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __WATERWAVE_H_
#define __WATERWAVE_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpdateModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
struct FieldParse;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class WaveGuideUpdateModuleData : public UpdateModuleData
{

public:

	WaveGuideUpdateModuleData( void );

	static void buildFieldParse(MultiIniFieldParse& p);

	Real m_waveDelay;													///< delay in frames to start the wave from when we become enabled
	Real m_ySize;															///< size of object in Y
	Real m_linearWaveSpacing;									///< linear waves get created at this resolution across the object
	Real m_waveBendMagnitude;									///< for curved waveshape, larger # = more straight
	Real m_waterVelocity;											///< (in distance per frame)this amount of force is applied to the water
	Real m_preferredHeight;										///< this is our preferred water height after the wave passes
	Real m_shorelineEffectDistance;						///< this far behind the wave we "hit" the shore
	Real m_damageRadius;											///< damage distance from sample points to do damage
	Real m_damageAmount;											///< amount of damage to do
	Real m_toppleForce;												///< force strength we topple things with
	AudioEventRTS m_randomSplashSound;				///< random splash sound to play sometimes during the wave
	Int m_randomSplashSoundFrequency;					///< number from 1-100 that must be above to play
	const ParticleSystemTemplate *m_bridgeParticle;	///< particle system to play when the wave hits a bridge
	Real m_bridgeParticleAngleFudge;					///< angle to offset the bridge particle system
	AudioEventRTS m_loopingSound;							///< Sound that plays once wave guide is triggered.

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class WaveGuideUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( WaveGuideUpdate, "WaveGuideUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( WaveGuideUpdate, WaveGuideUpdateModuleData )

public:

  WaveGuideUpdate( Thing *thing, const ModuleData *moduleData );
	// virtual destructor prototype provided by MemoryPoolObject

	virtual UpdateSleepTime update( void );		///< the update implementation

protected:

	Bool initWaveGuide( void );						///< initialize the waveguide on motion start
	Bool startMoving( void );							///< start the waveguide moving

	void computeWaveShapePoints( void );	///< compute the wave shape points

	void transformWaveShape( void );			///< build an array we can re-use of the wave shape points

	void doShapeEffects( void );					///< maintenance for the wave shape effects
	void doWaterMotion( void );						///< do the push up water motion
	void doShoreEffects( void );					///< do any effects on the shorelines
	void doDamage( void );								///< do damage to things that have crossed our path

	UnsignedInt m_activeFrame;						///< frame we became active on
	Bool m_needDisable;
	Bool m_initialized;										///< set to TRUE after we're enabled and in motion

	enum { MAX_WAVEGUIDE_SHAPE_POINTS = 64 };
	Coord3D m_shapePoints[ MAX_WAVEGUIDE_SHAPE_POINTS ];						///< array that will be our "shape" of sample points to affect the water
	Coord3D m_transformedShapePoints[ MAX_WAVEGUIDE_SHAPE_POINTS ];	///< m_shapePoints transformed into current world coords
	enum { MAX_SHAPE_EFFECTS = 3 };
	ParticleSystemID m_shapeEffects[ MAX_WAVEGUIDE_SHAPE_POINTS ][ MAX_SHAPE_EFFECTS ];		///< particle effects on the wavefront
	Int m_shapePointCount;								///< number of points in m_shape
	UnsignedInt m_splashSoundFrame;				///< frame we last tried to play a splash sound on

	Coord3D m_finalDestination;						///< the final destination of the waveguide path

};

#endif  // end __WATERWAVE_H_
