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

// PhysicsUpdate.h
// Simple rigid body physics
// Author: Michael S. Booth, November 2001

#pragma once

#ifndef _PHYSICSUPDATE_H_
#define _PHYSICSUPDATE_H_

#include "Common/AudioEventRTS.h"
#include "Common/GameAudio.h"
#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/UpdateModule.h"
#include "GameLogic/Module/CollideModule.h"

enum ObjectID CPP_11(: Int);

enum PhysicsTurningType CPP_11(: Int)
{
	TURN_NEGATIVE = -1,
	TURN_NONE = 0,
	TURN_POSITIVE = 1
};

//-------------------------------------------------------------------------------------------------
class PhysicsBehaviorModuleData : public UpdateModuleData
{
public:
	Real	m_mass;
	Real	m_forwardFriction;
	Real	m_lateralFriction;
	Real	m_ZFriction;
	Real	m_aerodynamicFriction;	// The percent of the wind resistance effect you suffer from
	Real	m_centerOfMassOffset;	// Distance the center of mass is from the center of geometry, to control pitch rate
	Bool	m_killWhenRestingOnGround;	// when airborne==false and vel==0, kill it.
	Bool	m_allowBouncing;
	Bool	m_allowCollideForce;
	Real	m_minFallSpeedForDamage;
	Real	m_fallHeightDamageFactor;
	Real	m_pitchRollYawFactor;
	const WeaponTemplate* m_vehicleCrashesIntoBuildingWeaponTemplate;
	const WeaponTemplate* m_vehicleCrashesIntoNonBuildingWeaponTemplate;

	PhysicsBehaviorModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

//-------------------------------------------------------------------------------------------------
/**
 * Simple rigid body physics update module
 */
class PhysicsBehavior : public UpdateModule,
												public CollideModuleInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( PhysicsBehavior, "PhysicsBehavior" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( PhysicsBehavior, PhysicsBehaviorModuleData )

public:
	PhysicsBehavior( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype defined by MemoryPoolObject

	static Int getInterfaceMask() { return UpdateModule::getInterfaceMask() | (MODULEINTERFACE_COLLIDE); }

	virtual void onObjectCreated();

	// BehaviorModule
	virtual CollideModuleInterface* getCollide() { return this; }

	// CollideModuleInterface
	virtual void onCollide( Object *other, const Coord3D *loc, const Coord3D *normal );
	virtual Bool wouldLikeToCollideWith(const Object* other) const { return false; }
	virtual Bool isCarBombCrateCollide() const { return false; }
	virtual Bool isHijackedVehicleCrateCollide() const { return false; }
	virtual Bool isRailroad() const { return false;}
	virtual Bool isSalvageCrateCollide() const { return false; }

	// UpdateModuleInterface
	virtual UpdateSleepTime update();
	// Disabled conditions to process -- all
	virtual DisabledMaskType getDisabledTypesToProcess() const { return DISABLEDMASK_ALL; }

	void applyForce( const Coord3D *force );		///< apply a force at the object's CG
	void addVelocityTo(const Coord3D* vel) ;

	/**
		identical to applyForce, except that no forward friction will be applied to the physics
		this frame. (lateral and z-friction will still be applied.)
	*/
	void applyMotiveForce( const Coord3D *force );

	/** This is a force clear for when objects are going out of bounds, so the locomotor
		can push them back into legal space. */
	void clearAcceleration() { m_accel.zero(); }

	/**
		add the velocity of 'this' to 'that'... useful when a unit disgorges another unit
		and you want to maintain relative velocities
	*/
	void transferVelocityTo(PhysicsBehavior* that) const;

	/// @todo Rotations should be handled by this system as well (MSB)

	void setAngles( Real yaw, Real pitch, Real roll );
	Real getMass() const;
	void setMass( Real mass ) { m_mass = mass; }
	Real getCenterOfMassOffset() const { return getPhysicsBehaviorModuleData()->m_centerOfMassOffset; }

	const Coord3D *getAcceleration() const { return &m_prevAccel; }		///< get last frame's acceleration
	const Coord3D *getVelocity() const { return &m_vel; }			///< get current velocity
	Real getVelocityMagnitude() const;																		///< return velocity magnitude (speed)
	Real getForwardSpeed2D() const;															///< compute speed along object's 2d direction vector
	Real getForwardSpeed3D() const;															///< compute speed along object's 3d direction vector

	ObjectID getCurrentOverlap() const;					///< return object(s) being overlapped
	ObjectID getPreviousOverlap() const;					///< return object(s) that were overlapped last frame
	ObjectID getLastCollidee() const;					///< return object that was last collided with... can be quite old
	Bool isCurrentlyOverlapped(Object *obj) const;
	Bool wasPreviouslyOverlapped(Object *obj) const;
	void addOverlap(Object *obj);

	Bool isMotive() const;

	PhysicsTurningType getTurning(void) const { return m_turning; }		///< 0 = not turning, -1 = turn negative, 1 = turn positive.
	void setTurning(PhysicsTurningType turning) { m_turning = turning; }

	/** This is a force scrub for velocity when ai objects are colliding. */
	void scrubVelocity2D( Real desiredVelocity );

	// note that, unlike scrubVelocity2D(), this is a signed limit
	void scrubVelocityZ( Real desiredVelocity );

	void setPitchRate(Real pitch);
	void setRollRate(Real roll);
	void setYawRate(Real yaw);

	/*
		stickToGround and allowToFall seem contradictory... here's the deal.

		if stickToGround is true, you stick to the ground, otherwise your z isn't messed with.

		HOWEVER, if allowToFall is true, stickToGround is ignored, and z is never messed with.
		Also, allowToFall is reset to "false" when you hit the ground, so you can set it with
		(relatively) little fear.
	*/
	void setAllowToFall(Bool allow) { setFlag(ALLOW_TO_FALL, allow); }
	void setStickToGround(Bool allow) { setFlag(STICK_TO_GROUND, allow); }
	void setAllowBouncing(Bool allow) { setFlag(ALLOW_BOUNCE, allow); }
	void setAllowCollideForce(Bool allow) { setFlag(ALLOW_COLLIDE_FORCE, allow); }
	void setAllowAirborneFriction(Bool allow) { setFlag(APPLY_FRICTION2D_WHEN_AIRBORNE, allow); }
	void setImmuneToFallingDamage(Bool allow) { setFlag(IMMUNE_TO_FALLING_DAMAGE, allow); }

	Bool getAllowToFall() const { return getFlag(ALLOW_TO_FALL); }

	void setIsInFreeFall(Bool allow) { setFlag(IS_IN_FREEFALL, allow); }
	Bool getIsInFreeFall() const { return getFlag(IS_IN_FREEFALL); }

	void setExtraBounciness(Real b) { m_extraBounciness = b; }
	void setExtraFriction(Real b) { m_extraFriction = b; }

	void setBounceSound(const AudioEventRTS* bounceSound);
	const AudioEventRTS* getBounceSound() { return m_bounceSound ? &m_bounceSound->m_event : TheAudio->getValidSilentAudioEvent(); }

	/**
		Reset all values (vel, accel, etc) to starting values.
		You should ALMOST NEVER use this; it's intended for cases where you need
		to nuke everything, like when an existing object goes off the map and you
		want to bring it back on with all-new values, and want to start with a clean
		slate. Rule of thumb: if the object was in existence on the map in the previous
		frame, don't call this... apply forces instead!
	*/
	void resetDynamicPhysics();

	void setIgnoreCollisionsWith(const Object* obj);
	Bool isIgnoringCollisionsWith(ObjectID id) const;

	inline Bool getAllowCollideForce() const { return getFlag(ALLOW_COLLIDE_FORCE); }

protected:

	/*
		Physics runs in its own phase, after AI, but before all others.
		It's actually quite important that AI (the thing that drives Locomotors) and Physics
		run in the same order, relative to each other, for a given object; otherwise,
		interesting oscillations can occur in some situations, with friction being applied
		either before or after the locomotive force, making for huge stuttery messes. (srj)
	*/
	virtual SleepyUpdatePhase getUpdatePhase() const { return PHASE_PHYSICS; }

	Real getAerodynamicFriction() const;
	Real getForwardFriction() const;
	Real getLateralFriction() const;
	Real getZFriction() const;

	void applyGravitationalForces();
	void applyFrictionalForces();
	Bool handleBounce(Real oldZ, Real newZ, Real groundZ, Coord3D* bounceForce);
	void applyYPRDamping(Real factor);
	UpdateSleepTime calcSleepTime() const;

	void doBounceSound(const Coord3D& prevPos);

	Bool checkForOverlapCollision(Object *other);

private:

	enum PhysicsFlagsType
	{
		// Note - written out in save/load xfer; don't change these numbers.
		STICK_TO_GROUND									= 0x0001,
		ALLOW_BOUNCE										= 0x0002,
		APPLY_FRICTION2D_WHEN_AIRBORNE	= 0x0004,
		UPDATE_EVER_RUN									= 0x0008,
		WAS_AIRBORNE_LAST_FRAME					= 0x0010,
		ALLOW_COLLIDE_FORCE							= 0x0020,
		ALLOW_TO_FALL										= 0x0040,
		HAS_PITCHROLLYAW								= 0x0080,
		IMMUNE_TO_FALLING_DAMAGE				= 0x0100,
		IS_IN_FREEFALL									= 0x0200,
		IS_IN_UPDATE										= 0x0400
	};

	/*
		Note: these are private because you should never manipulate these directly,
		even if you are a subclass... if you want to change the acceleration, you
		MUST call applyForce().
	*/
	Real												m_yawRate;								///< rate of rotation around up vector
	Real												m_rollRate;								///< rate of rotation around forward vector
	Real												m_pitchRate;							///< rate or rotation around side vector
	DynamicAudioEventRTS*				m_bounceSound;						///< The sound for when this thing bounces, or NULL
	Coord3D											m_accel;									///< current acceleration
	Coord3D											m_prevAccel;							///< last frame's acceleration
	Coord3D											m_vel;										///< current velocity
	PhysicsTurningType					m_turning;								///< 0 = not turning, -1 = turn negative, 1 = turn positive.
	ObjectID										m_ignoreCollisionsWith;
	Int													m_flags;
	Real												m_mass;
	ObjectID										m_currentOverlap;					///< object(s) being overlapped, if any
	ObjectID										m_previousOverlap;				///< last frame's object(s) being overlapped
	ObjectID										m_lastCollidee;						///< ID of the last object I collided with, can be quite old.
	UnsignedInt									m_motiveForceExpires;			///< frames at which "recent" applyMotiveForce is no longer considered
	Real												m_extraBounciness;				///< modifier to ground stiffness
	Real												m_extraFriction;					///< modifier to friction(s)
	ProjectileUpdateInterface*	m_pui;
	mutable Real								m_velMag;									///< magnitude of cur vel (recalced when m_vel changes)

	inline void setFlag(PhysicsFlagsType f, Bool set) { if (set) m_flags |= f; else m_flags &= ~f; }
	inline Bool getFlag(PhysicsFlagsType f) const { return (m_flags & f) != 0; }


};

//-------------------------------------------------------------------------------------------------
inline ObjectID PhysicsBehavior::getCurrentOverlap() const
{
	return m_currentOverlap;
}

//-------------------------------------------------------------------------------------------------
inline ObjectID PhysicsBehavior::getPreviousOverlap() const
{
	return m_previousOverlap;
}

inline ObjectID PhysicsBehavior::getLastCollidee() const
{
	return m_lastCollidee;
}

#endif // _PHYSICSUPDATE_H_

