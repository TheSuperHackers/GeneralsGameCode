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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// ParticleSys.h //////////////////////////////////////////////////////////////////////////////////
// Particle System type definitions
// Author: Michael S. Booth, November 2001
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef _PARTICLE_SYS_H_
#define _PARTICLE_SYS_H_

#include <stdio.h>
#include "Common/AsciiString.h"
#include "Common/GameMemory.h"
#include "Common/GameType.h"
#include "Common/Snapshot.h"
#include "Common/SubsystemInterface.h"
#include "GameClient/ClientRandomValue.h"

#include "WWMath/matrix3d.h"		///< @todo Replace with our own matrix library
#include "Common/STLTypedefs.h"

 
/// @todo Once the client framerate is decoupled, the frame counters within will have to become time-based
 
class Particle;
class ParticleSystem;
class ParticleSystemManager;
class Drawable;
class Object;
struct FieldParse;
class INI;
class DebugWindowDialog;		// really ParticleEditorDialog
class RenderInfoClass;			// ick

enum ParticleSystemID CPP_11(: Int)
{
	INVALID_PARTICLE_SYSTEM_ID = 0
};

#define MAX_VOLUME_PARTICLE_DEPTH ( 16 )
#define DEFAULT_VOLUME_PARTICLE_DEPTH ( 0 )//The Default is not to do the volume thing!
#define OPTIMUM_VOLUME_PARTICLE_DEPTH ( 6 )


//--------------------------------------------------------------------------------------------------------------

enum { MAX_KEYFRAMES=8 };

struct Keyframe
{
	Real value;
	UnsignedInt frame;
};

struct RGBColorKeyframe
{
	RGBColor color;
	UnsignedInt frame;
};

enum ParticlePriorityType CPP_11(: Int)
{
	INVALID_PRIORITY = 0, 
	PARTICLE_PRIORITY_LOWEST = 1,
//	FLUFF = PARTICLE_PRIORITY_LOWEST,		///< total and absolute fluff
//	DEBRIS,		///< debris related particles
//	NATURE,	///< neato effects we just might see in the world
//	WEAPON,		///< Weapons firing and flying in the air
//	DAMAGE,		///< taking damage/dying explosions
//	SPECIAL,	///< super special top priority like a superweapon

	WEAPON_EXPLOSION = PARTICLE_PRIORITY_LOWEST,
	SCORCHMARK,
	DUST_TRAIL,
	BUILDUP,
	DEBRIS_TRAIL,
	UNIT_DAMAGE_FX,
	DEATH_EXPLOSION,
	SEMI_CONSTANT,
	CONSTANT,
	WEAPON_TRAIL,
	AREA_EFFECT,
	CRITICAL,				///< super special top priority like a superweapon
	ALWAYS_RENDER,	///< used for logically important display (not just fluff), so must never be culled, regardless of particle cap, lod, etc
	// !!! *Noting* goes here ... special is the top priority !!!
	PARTICLE_PRIORITY_HIGHEST = ALWAYS_RENDER,
	NUM_PARTICLE_PRIORITIES  ///< Keep this last
};

/**
 * This structure is filled out and passed to the constructor of a Particle to initialize it
 */
class ParticleInfo : public Snapshot
{

public:

	ParticleInfo( void );

	Coord3D m_vel;															///< initial velocity
	Coord3D m_pos;															///< initial position
	Coord3D m_emitterPos;												///< position of the emitter
	Real m_velDamping;													///< velocity damping coefficient

	Real m_angleX;															///< initial angle around X axis
	Real m_angleY;															///< initial angle around Y axis
	Real m_angleZ;															///< initial angle around Z axis
	Real m_angularRateX;												///< initial angle around X axis
	Real m_angularRateY;												///< initial angle around Y axis
	Real m_angularRateZ;												///< initial angle around Z axis
	Real m_angularDamping;											///< angular velocity damping coefficient

	UnsignedInt m_lifetime;											///< lifetime of this particle

	Real m_size;																///< size of the particle
	Real m_sizeRate;														///< rate of change of size
	Real m_sizeRateDamping;											///< damping of size change rate

	Keyframe m_alphaKey[ MAX_KEYFRAMES ];
	RGBColorKeyframe m_colorKey[ MAX_KEYFRAMES ];

	Real m_colorScale;													///< color "scaling" coefficient

	Real m_windRandomness;											///< multiplier for wind randomness per particle

	Bool m_particleUpTowardsEmitter;						///< if this is true, then the 0.0 Z rotation should actually
																							///< correspond to the direction of the emitter.

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

};


/**
 * An individual particle created by a ParticleSystem.
 * NOTE: Particles cannot exist without a parent particle system.
 */
class Particle : public MemoryPoolObject, 
								 public ParticleInfo
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( Particle, "ParticlePool" )		

public:

	Particle( ParticleSystem *system, const ParticleInfo *data );

	Bool update( void );												///< update this particle's behavior - return false if dead
	void doWindMotion( void );									///< do wind motion (if present) from particle system

	void applyForce( const Coord3D *force );		///< add the given acceleration
	void detachDrawable( void ) { m_drawable = NULL; }	///< detach the Drawable pointer from this particle

	inline const Coord3D *getPosition( void ) { return &m_pos; }
	inline Real getSize( void ) { return m_size; }
	inline Real getAngle( void ) { return m_angleZ; }
	inline Real getAlpha( void ) { return m_alpha; }
	inline const RGBColor *getColor( void ) { return &m_color; }
	inline void setColor( RGBColor *color ) { m_color = *color; }

	Bool isInvisible( void );										///< return true if this particle is invisible
	inline Bool isCulled (void) {return m_isCulled;}				///< return true if the particle falls off the edge of the screen
	inline void setIsCulled (Bool enable) { m_isCulled = enable;}		///< set particle to not visible because it's outside view frustum

	void controlParticleSystem( ParticleSystem *sys ) { m_systemUnderControl = sys; }
	void detachControlledParticleSystem( void ) { m_systemUnderControl = NULL; }
	
	// get priority of this particle ... which is the priority of the system it belongs to
	ParticlePriorityType getPriority( void );

	UnsignedInt getPersonality(void) { return m_personality; };
	void setPersonality(UnsignedInt p) { m_personality = p; };

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

	void computeAlphaRate( void );							///< compute alpha rate to get to next key
	void computeColorRate( void );							///< compute color change to get to next key

public:
	Particle *				m_systemNext;
	Particle *				m_systemPrev;
	Particle *				m_overallNext;
	Particle *				m_overallPrev;

protected:
	ParticleSystem *	m_system;										///< the particle system this particle belongs to
	UnsignedInt				m_personality;							    ///< each new particle assigned a number one higher than the previous

	// most of the particle data is derived from ParticleInfo

	Coord3D						m_accel;														///< current acceleration
	Coord3D						m_lastPos;													///< previous position
	UnsignedInt				m_lifetimeLeft;									///< lifetime remaining, if zero -> destroy
	UnsignedInt				m_createTimestamp;							///< frame this particle was created

	Real							m_alpha;																///< current alpha of this particle
	Real							m_alphaRate;														///< current rate of alpha change
	Int								m_alphaTargetKey;												///< next index into key array

	RGBColor					m_color;														///< current color of this particle
	RGBColor					m_colorRate;												///< current rate of color change
	Int								m_colorTargetKey;												///< next index into key array

	Drawable *				m_drawable;												///< drawable associated with this particle

	Bool							m_isCulled;														///< status of particle relative to screen bounds
public:
	Bool							m_inSystemList;
	Bool							m_inOverallList;

	union
	{
		ParticleSystem *	m_systemUnderControl;			///< the particle system attached to this particle (not the system that created us)
		ParticleSystemID	m_systemUnderControlID;	///< id of system attached to this particle (not the system that created us);
	};

};


//--------------------------------------------------------------------------------------------------------------

#ifdef DEFINE_PARTICLE_SYSTEM_NAMES

/**** NOTE: These MUST be kept in sync with the enumerations below *****/

static const char *ParticleShaderTypeNames[] = 
{
	"NONE", "ADDITIVE", "ALPHA", "ALPHA_TEST", "MULTIPLY", NULL
};

static const char *ParticleTypeNames[] = 
{
	"NONE", "PARTICLE", "DRAWABLE", "STREAK", "VOLUME_PARTICLE", NULL
};

static const char *EmissionVelocityTypeNames[] =
{
	"NONE", "ORTHO", "SPHERICAL", "HEMISPHERICAL", "CYLINDRICAL", "OUTWARD", NULL
};

static const char *EmissionVolumeTypeNames[] =
{
	"NONE", "POINT", "LINE", "BOX", "SPHERE", "CYLINDER", NULL
};

//"NONE", "FLUFF", "DEBRIS", "NATURE", "WEAPON", "DAMAGE", "SPECIAL"
static const char *ParticlePriorityNames[] = 
{
	"NONE", "WEAPON_EXPLOSION","SCORCHMARK","DUST_TRAIL","BUILDUP","DEBRIS_TRAIL","UNIT_DAMAGE_FX","DEATH_EXPLOSION","SEMI_CONSTANT","CONSTANT","WEAPON_TRAIL","AREA_EFFECT","CRITICAL", "ALWAYS_RENDER", NULL
};

static const char *WindMotionNames[] = 
{
	"NONE", "Unused", "PingPong", "Circular", NULL
};

#endif

/**
 * All of the properties of a particle system, used by both ParticleSystemTemplates
 * and ParticleSystem classes.
 */
class ParticleSystemInfo : public Snapshot
{

public:

	ParticleSystemInfo();												///< to set defaults.

	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

	Bool m_isOneShot;														///< if true, destroy system after one burst has occurred

	enum ParticleShaderType
	{
		INVALID_SHADER=0, ADDITIVE, ALPHA, ALPHA_TEST, MULTIPLY
	} 
	m_shaderType;																///< how this particle is rendered

	enum ParticleType
	{
		INVALID_TYPE=0, PARTICLE, DRAWABLE, STREAK, VOLUME_PARTICLE	 ///< is a particle a 2D-screen-facing particle, or a Drawable, or a Segment in a streak?
	}
	m_particleType;

	AsciiString m_particleTypeName;							///< if PARTICLE, texture filename, if DRAWABLE, Drawable name

	GameClientRandomVariable m_angleX;										///< initial angle around X axis
	GameClientRandomVariable m_angleY;										///< initial angle around Y axis
	GameClientRandomVariable m_angleZ;										///< initial angle around Z axis
	GameClientRandomVariable m_angularRateX;							///< initial angle around X axis
	GameClientRandomVariable m_angularRateY;							///< initial angle around Y axis
	GameClientRandomVariable m_angularRateZ;							///< initial angle around Z axis
	GameClientRandomVariable m_angularDamping;						///< angular velocity damping coefficient

	GameClientRandomVariable m_velDamping;								///< velocity damping factor

	GameClientRandomVariable m_lifetime;									///< lifetime of emitted particles
	UnsignedInt m_systemLifetime;								///< lifetime of the particle system

	GameClientRandomVariable m_startSize;									///< initial size of emitted particles
	GameClientRandomVariable m_startSizeRate;							///< change in start size of emitted particles
	GameClientRandomVariable m_sizeRate;									///< rate of change of size
	GameClientRandomVariable m_sizeRateDamping;						///< damping of size change


	UnsignedInt m_volumeParticleDepth;								///< how many layers deep to draw the particle, if >1

	struct RandomKeyframe
	{
		GameClientRandomVariable var;												///< the range of values at this keyframe
		UnsignedInt frame;												///< the frame number
	};


	RandomKeyframe m_alphaKey[ MAX_KEYFRAMES ];
	RGBColorKeyframe m_colorKey[ MAX_KEYFRAMES ];	///< color of particle

	typedef Int Color;

	void tintAllColors( Color tintColor );
	
	GameClientRandomVariable m_colorScale;								///< color coefficient

	GameClientRandomVariable m_burstDelay;								///< time between particle emissions
	GameClientRandomVariable m_burstCount;								///< number of particles emitted per burst

	GameClientRandomVariable m_initialDelay;							///< delay before particles begin emitting

	Coord3D m_driftVelocity;										///< additional velocity added to all particles
	Real m_gravity;															///< gravity acceleration (global Z) for particles in this system

	AsciiString m_slaveSystemName;							///< if non-empty, create a system whose particles track this system's
	Coord3D m_slavePosOffset;										///< positional offset of slave particles relative to master's

	AsciiString m_attachedSystemName;						///< if non-empty, create a system attached to each particle of this system

	//-------------------------------------------------------
	// The direction and speed at which particles are emitted
	enum EmissionVelocityType
	{
		INVALID_VELOCITY=0, ORTHO, SPHERICAL, HEMISPHERICAL, CYLINDRICAL, OUTWARD
	}
	m_emissionVelocityType;

	ParticlePriorityType m_priority;

	union
	{
		struct
		{
			GameClientRandomVariable x;												///< initial speed of particle along X axis
			GameClientRandomVariable y;												///< initial speed of particle along Y axis
			GameClientRandomVariable z;												///< initial speed of particle along Z axis
		}
		ortho;

		struct
		{
			GameClientRandomVariable speed;										///< initial speed of particle along random radial direction
		}
		spherical, hemispherical;

		struct
		{
			GameClientRandomVariable radial;									///< initial speed of particle in the disk
			GameClientRandomVariable normal;									///< initial speed of particle perpendicular to disk
		}
		cylindrical;

		struct
		{
			GameClientRandomVariable speed;										///< speed outward from emission volume
			GameClientRandomVariable otherSpeed;							///< speed along "other" axis, such as cylinder length
		}
		outward;
	}
	m_emissionVelocity;

	//----------------------------------------------------------
	// The volume of space where particles are initially created
	// Note that the volume is relative to the system's position and orientation
	enum EmissionVolumeType
	{
		INVALID_VOLUME=0, POINT, LINE, BOX, SPHERE, CYLINDER
	}
	m_emissionVolumeType;												///< the type of volume where particles are created
	
	union emissionVolumeUnion
	{
		// point just uses system's position

		// line
		struct
		{
			Coord3D start, end;
		}
		line;

		// box
		struct
		{
			Coord3D halfSize;
		}
		box;

		// sphere
		struct
		{
			Real radius;
		}
		sphere;

		// cylinder
		struct
		{
			Real radius;
			Real length;
		}
		cylinder;
	}
	m_emissionVolume;														///< the dimensions of the emission volume

	Bool m_isEmissionVolumeHollow;							///< if true, only create particles at boundary of volume
	Bool m_isGroundAligned;											///< if true, align with the ground. if false, then do the normal billboarding. 
	Bool m_isEmitAboveGroundOnly;								///< if true, only emit particles when the system is above ground.
	Bool m_isParticleUpTowardsEmitter;					///< if true, align the up direction to be towards the emitter.

	enum WindMotion
	{
		WIND_MOTION_INVALID = 0,
		WIND_MOTION_NOT_USED,
		WIND_MOTION_PING_PONG,
		WIND_MOTION_CIRCULAR
	};
	WindMotion m_windMotion;				///< motion of the wind angle updating
	Real m_windAngle;								///< angle of the "wind" associated with this system
	Real m_windAngleChange;					///< current how fast the angle changes (higher=faster change)
	Real m_windAngleChangeMin;			///< min for angle change
	Real m_windAngleChangeMax;			///< max for angle change
	Real m_windMotionStartAngle;						///< (for ping pong) angle 1 of the ping pong
	Real m_windMotionStartAngleMin;					///< (for ping pong) min angle for angle 1
	Real m_windMotionStartAngleMax;					///< (for ping pong) max angle for angle 1
	Real m_windMotionEndAngle;							///< (for ping pong) angle 2 of the ping pong
	Real m_windMotionEndAngleMin;						///< (for ping pong) min angle for angle 2
	Real m_windMotionEndAngleMax;						///< (for ping pong) max angel for angle 2
	Byte m_windMotionMovingToEndAngle;			///< (for ping pong) TRUE if we're moving "towards" the end angle

};


/**
 * A ParticleSystemTemplate, used by the ParticleSystemManager to instantiate ParticleSystems.
 */
class ParticleSystemTemplate : public MemoryPoolObject, protected ParticleSystemInfo
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( ParticleSystemTemplate, "ParticleSystemTemplatePool" )		

public:
	ParticleSystemTemplate( const AsciiString &name );

	AsciiString getName( void ) const { return m_name; }

	// This function was made const because of update modules' module data being all const.
	ParticleSystem *createSlaveSystem( Bool createSlaves = TRUE ) const ;					///< if returns non-NULL, it is a slave system for use

	const FieldParse *getFieldParse( void ) const { return m_fieldParseTable; }	///< Parsing from INI access
	static void parseRGBColorKeyframe( INI* ini, void *instance, void *store, const void* /*userData*/ );
	static void parseRandomKeyframe( INI* ini, void *instance, void *store, const void* /*userData*/ );
	static void parseRandomRGBColor( INI* ini, void *instance, void *store, const void* /*userData*/ );
	static void parseRandomRGBColorRate( INI* ini, void *instance, void *store, const void* /*userData*/ );

protected:
	friend class ParticleSystemManager;					///< @todo remove this friendship
	friend class ParticleSystem;								///< @todo remove this friendship
	
	// These friendships are naughty but necessary for particle editing.
	friend class DebugWindowDialog;							///< @todo remove this friendship when no longer editing particles
	friend void _updateAsciiStringParmsToSystem(ParticleSystemTemplate *particleTemplate);
	friend void _updateAsciiStringParmsFromSystem(ParticleSystemTemplate *particleTemplate);
	friend void _writeSingleParticleSystem( File *out, ParticleSystemTemplate *templ );

	static const FieldParse m_fieldParseTable[];			///< the parse table for INI definition

protected:
	AsciiString								m_name;													///< the name of this template

	// This has to be mutable because of the delayed initialization thing in createSlaveSystem
	mutable const ParticleSystemTemplate *m_slaveTemplate;		///< if non-NULL, use this to create a slave system

	// template attribute data inherited from ParticleSystemInfo class
};

/**
 * A particle system, responsible for creating Particles.
 * If a particle system has finished, but still has particles "in the air", it must wait
 * before destroying itself in order to ensure everything can be cleaned up if the system
 * is reset.
 */
class ParticleSystem : public MemoryPoolObject, 
											 public ParticleSystemInfo
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( ParticleSystem, "ParticleSystemPool" )		

public:

	ParticleSystem( const ParticleSystemTemplate *sysTemplate, 
									ParticleSystemID id,
									Bool createSlaves );			///< create a particle system from a template and assign it this ID

	inline ParticleSystemID getSystemID( void ) const { return m_systemID; }	///< get unique system ID

	void setPosition( const Coord3D *pos );			///< set the position of the particle system
	void getPosition( Coord3D *pos );				///< get the position of the particle system
	void setLocalTransform( const Matrix3D *matrix );	///< set the system's local transform
	void rotateLocalTransformX( Real x );				///< rotate local transform matrix
	void rotateLocalTransformY( Real y );				///< rotate local transform matrix
	void rotateLocalTransformZ( Real z );				///< rotate local transform matrix
		
	const Coord3D *getDriftVelocity( void ) { return &m_driftVelocity; }	///< get the drift velocity of the system

	void attachToDrawable( const Drawable *draw );							///< attach this particle system to a Drawable
	void attachToObject( const Object *obj );									///< attach this particle system to an Object

	virtual Bool update( Int localPlayerIndex );								///< update this particle system, return false if dead
	void updateWindMotion( void );							///< update wind motion

	void setControlParticle( Particle *p );			///< set control particle

	void start( void );													///< (re)start a stopped particle system
	void stop( void );													///< stop a particle system from emitting
	void destroy( void );												///< stop emitting, and wait for particles to die, then destroy self

	void setVelocityMultiplier( const Coord3D *value ) { m_velCoeff = *value; }	///< set the velocity multiplier coefficient
	const Coord3D *getVelocityMultiplier( void ) { return &m_velCoeff; }					///< get the velocity multiplier coefficient

	void setBurstCountMultiplier( Real value ) { m_countCoeff = value; }
	Real getBurstCountMultiplier( void ) { return m_countCoeff; }

	void setBurstDelayMultiplier( Real value ) { m_delayCoeff = value; }
	Real getBurstDelayMultiplier( void ) { return m_delayCoeff; }

	void setSizeMultiplier( Real value ) { m_sizeCoeff = value; }
	Real getSizeMultiplier( void ) { return m_sizeCoeff; }

	/// Force a burst of particles to be emitted immediately.
	void trigger (void) { m_burstDelayLeft = 0; m_delayLeft = 0;}

	void setInitialDelay( UnsignedInt delay ) { m_delayLeft = delay; }

	AsciiString getParticleTypeName( void ) { return m_particleTypeName; }	///< return the name of the particles
	Bool isUsingDrawables( void ) { return (m_particleType == DRAWABLE) ? true : false; }
	Bool isUsingStreak( void ) { return (m_particleType == STREAK) ? true : false; }
	UnsignedInt getVolumeParticleDepth( void ) { return ( m_particleType == VOLUME_PARTICLE ) ? OPTIMUM_VOLUME_PARTICLE_DEPTH : 0; }

	Bool shouldBillboard( void ) { return !m_isGroundAligned; }

	ParticleShaderType getShaderType( void ) { return m_shaderType; }

	void setSlave( ParticleSystem *slave );			///< set a slave system for us
	ParticleSystem *getSlave( void ) { return m_slaveSystem; }
	void setMaster( ParticleSystem *master );  ///< make this a slave with a master
	ParticleSystem *getMaster( void ) { return m_masterSystem; }
	const Coord3D *getSlavePositionOffset( void ) { return &m_slavePosOffset; }
	
	void setSystemLifetime( UnsignedInt frames ) { m_systemLifetimeLeft = frames; }; ///< not the particle life, the system!... Lorenzen
	void setLifetimeRange( Real min, Real max );
	Bool isSystemForever() const {return m_isForever;}

	UnsignedInt getStartFrame( void ) { return m_startTimestamp; }	///< return frame when this system was created
	Bool isDestroyed( void ) const { return m_isDestroyed; }

	void setSaveable( Bool b );
	Bool isSaveable( void ) const { return m_isSaveable; }

	/// called when the particle this system is controlled by dies
	void detachControlParticle( Particle *p ) { m_controlParticle = NULL; }

	/// called to merge two systems info. If slaveNeedsFullPromotion is true, then the slave needs to be aware of how many particles
	/// to generate as well.
	static ParticleInfo mergeRelatedParticleSystems( ParticleSystem *masterParticleSystem, ParticleSystem *slaveParticleSystem, Bool slaveNeedsFullPromotion);

	/// @todo Const this (MSB)
	const ParticleSystemTemplate *getTemplate( void ) { return m_template; }	///< get the template used to create this system

	// @todo Const this jkmcd
	Particle *getFirstParticle( void ) { return m_systemParticlesHead; }

	void addParticle( Particle *particleToAdd );
	/// when a particle dies, it calls this method - ONLY FOR USE BY PARTICLE
	void removeParticle( Particle *p );
	UnsignedInt getParticleCount( void ) const { return m_particleCount; }

	inline ObjectID getAttachedObject( void ) { return m_attachedToObjectID; }
	inline DrawableID getAttachedDrawable( void ) { return m_attachedToDrawableID; }

	// Access to dynamically changing part of a particle system.
	void setEmissionVolumeSphereRadius( Real newRadius ) { if (m_emissionVolumeType == SPHERE) m_emissionVolume.sphere.radius = newRadius; }
	void setEmissionVolumeCylinderRadius( Real newRadius ) { if (m_emissionVolumeType == CYLINDER) m_emissionVolume.cylinder.radius = newRadius; }
	EmissionVolumeType getEmisionVolumeType() const { return m_emissionVolumeType; }
	ParticlePriorityType getPriority() const { return m_priority; }

	// Access to wind motoin
	Real getWindAngle( void ) { return m_windAngle; }
	WindMotion getWindMotion( void ) { return m_windMotion; }

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

	virtual Particle *createParticle( const ParticleInfo *data,
																		ParticlePriorityType priority,
																		Bool forceCreate = FALSE );	///< factory method for particles


	const ParticleInfo *generateParticleInfo( Int particleNum, Int particleCount );	///< generate a new, random set of ParticleInfo
	const Coord3D *computeParticlePosition( void );		///< compute a position based on emission properties
	const Coord3D *computeParticleVelocity( const Coord3D *pos );	///< compute a velocity vector based on emission properties
	const Coord3D *computePointOnUnitSphere( void );	///< compute a random point on a unit sphere

protected:
	Particle *				m_systemParticlesHead;
	Particle *				m_systemParticlesTail;

	UnsignedInt				m_particleCount;								///< current count of particles for this system
	ParticleSystemID	m_systemID;											///< unique id given to this system from the particle system manager

	DrawableID				m_attachedToDrawableID;					///< if non-zero, system is parented to this Drawable
	ObjectID					m_attachedToObjectID;						///< if non-zero, system is parented to this Object

	Matrix3D					m_localTransform;								///< local orientation & position of system
	Matrix3D					m_transform;										///< composite transform of parent Drawable and local

	UnsignedInt				m_burstDelayLeft;								///< when zero, emit a particle burst
	UnsignedInt				m_delayLeft;										///< decremented until zero

	UnsignedInt				m_startTimestamp;								///< timestamp when this particle system was (re)started
	UnsignedInt				m_systemLifetimeLeft;						///< lifetime remaining for entire particle system
	UnsignedInt				m_personalityStore;							///< increments each time it is aggigned to each new particle
																										///< so that each particle gets an ever greater number

	Real							m_accumulatedSizeBonus;					///< For a system that wants to make particles start bigger and bigger.  StartSizeRate

	Coord3D						m_velCoeff;											///< scalar value multiplied by all velocity parameters
	Real							m_countCoeff;										///< scalar value multiplied by burst count
	Real							m_delayCoeff;										///< scalar value multiplied by burst delay
	Real							m_sizeCoeff;										///< scalar value multiplied by initial size

	Coord3D						m_pos;													///< this is the position to emit at.
	Coord3D						m_lastPos;											///< this is the previous position we emitted at.

	ParticleSystem *	m_slaveSystem;									///< if non-NULL, another system this one has control of
	ParticleSystemID	m_slaveSystemID;								///< id of slave system (if present)

	ParticleSystem *	m_masterSystem;									///< if non-NULL, the system that controls this one
	ParticleSystemID	m_masterSystemID;								///< master system id (if present);

	const ParticleSystemTemplate *	m_template;						///< the template this system was constructed from
	Particle *											m_controlParticle;		///< if non-NULL, this system is controlled by this particle

	Bool							m_isLocalIdentity;										///< if true, the matrix can be ignored
	Bool							m_isIdentity;													///< if true, the matrix can be ignored
	Bool							m_isForever;													///< System has infinite lifetime
	Bool							m_isStopped;													///< if stopped, do not emit particles
	Bool							m_isDestroyed;												///< are we destroyed and waiting for particles to die
	Bool							m_isFirstPos;													///< true if this system hasn't been drawn before.
	Bool							m_isSaveable;													///< true if this system should be saved/loaded


	// the actual particle system data is inherited from ParticleSystemInfo

};


//--------------------------------------------------------------------------------------------------------------
/**
 * The particle system manager, responsible for maintaining all ParticleSystems
 */
class ParticleSystemManager : public SubsystemInterface,
															public Snapshot
{

public:

	typedef std::list<ParticleSystem*> ParticleSystemList;
	typedef std::list<ParticleSystem*>::iterator ParticleSystemListIt;
	typedef std::hash_map<AsciiString, ParticleSystemTemplate *, rts::hash<AsciiString>, rts::equal_to<AsciiString> > TemplateMap;

	ParticleSystemManager( void );
	virtual ~ParticleSystemManager();

	virtual void init( void );									///< initialize the manager
	virtual void reset( void );									///< reset the manager and all particle systems
	virtual void update( void );								///< update all particle systems

	virtual Int getOnScreenParticleCount( void ) = 0;   ///< returns the number of particles on screen
  virtual void setOnScreenParticleCount(int count);

	ParticleSystemTemplate *findTemplate( const AsciiString &name ) const;
	ParticleSystemTemplate *findParentTemplate( const AsciiString &name, int parentNum ) const;
	ParticleSystemTemplate *newTemplate( const AsciiString &name );

	/// given a template, instantiate a particle system
	ParticleSystem *createParticleSystem( const ParticleSystemTemplate *sysTemplate,
																				Bool createSlaves = TRUE );	

	/** given a template, instantiate a particle system. 
		if attachTo is not null, attach the particle system to the given object.
		return the particle system's ID, NOT its pointer.
	*/
	ParticleSystemID createAttachedParticleSystemID( const ParticleSystemTemplate *sysTemplate,
																				Object* attachTo,
																				Bool createSlaves = TRUE );	

	/// find a particle system given a unique system identifier
	ParticleSystem *findParticleSystem( ParticleSystemID id );

	/// destroy the particle system with the given id (if it still exists)
	void destroyParticleSystemByID(ParticleSystemID id);

	/// return iterators to the particle system template
	TemplateMap::iterator beginParticleSystemTemplate() { return m_templateMap.begin(); }
	TemplateMap::iterator endParticleSystemTemplate() { return m_templateMap.end(); }
	TemplateMap::const_iterator beginParticleSystemTemplate() const { return m_templateMap.begin(); }
	TemplateMap::const_iterator endParticleSystemTemplate() const { return m_templateMap.end(); }
	
	/// destroy attached systems to object
	void destroyAttachedSystems( Object *obj );

	void setLocalPlayerIndex(Int index)	{m_localPlayerIndex=index;}
	void addParticle( Particle *particleToAdd, ParticlePriorityType priority );
	void removeParticle( Particle *particleToRemove );
	Int removeOldestParticles( UnsignedInt count, ParticlePriorityType priorityCap );
	UnsignedInt getParticleCount( void ) const { return m_particleCount; }

	UnsignedInt getFieldParticleCount( void )     const { return m_fieldParticleCount; }

	UnsignedInt getParticleSystemCount( void ) const { return m_particleSystemCount; }

	// @todo const this jkmcd
	ParticleSystemList &getAllParticleSystems( void ) { return m_allParticleSystemList; }
	
	virtual void doParticles(RenderInfoClass &rinfo) = 0;
	virtual void queueParticleRender() = 0;
	
	virtual void preloadAssets( TimeOfDay timeOfDay );

	// these are only for use by partcle systems to link and unlink themselves
	void friend_addParticleSystem( ParticleSystem *particleSystemToAdd );
	void friend_removeParticleSystem( ParticleSystem *particleSystemToRemove );

protected:

	// snapshot methods
	virtual void crc( Xfer *xfer );
	virtual void xfer( Xfer *xfer );
	virtual void loadPostProcess( void );

	Particle *m_allParticlesHead[ NUM_PARTICLE_PRIORITIES ];
	Particle *m_allParticlesTail[ NUM_PARTICLE_PRIORITIES ];

	ParticleSystemID m_uniqueSystemID;					///< unique system ID to assign to each system created

	ParticleSystemList m_allParticleSystemList;

	UnsignedInt m_particleCount;
	UnsignedInt m_fieldParticleCount; ///< this does not need to be xfered, since it is evaluated every frame
	UnsignedInt m_particleSystemCount;
	Int m_onScreenParticleCount;                ///< number of particles displayed on screen per frame
	UnsignedInt m_lastLogicFrameUpdate;
	Int m_localPlayerIndex;	///<used to tell particle systems which particles can be skipped due to player shroud status

private:
	TemplateMap m_templateMap;		///< a hash map of all particle system templates
};

/// The particle system manager singleton
extern ParticleSystemManager *TheParticleSystemManager;

class DebugDisplayInterface;
extern void ParticleSystemDebugDisplay( DebugDisplayInterface *dd, void *, FILE *fp = NULL );


#endif // _PARTICLE_SYS_H_
