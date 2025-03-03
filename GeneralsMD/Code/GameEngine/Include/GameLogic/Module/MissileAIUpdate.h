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

// FILE:		MissileAIUpdate.h
// Author:	Michael S. Booth, December 2001
// Desc:		Missile behavior

#pragma once

#ifndef _MISSILE_AI_UPDATE_H_
#define _MISSILE_AI_UPDATE_H_

#include "Common/GameType.h"
#include "Common/GlobalData.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/WeaponBonusConditionFlags.h"
#include "Common/INI.h"
#include "WWMath/matrix3d.h"

enum ParticleSystemID;
class FXList;


//-------------------------------------------------------------------------------------------------
class MissileAIUpdateModuleData : public AIUpdateModuleData
{
public:
	Bool						m_tryToFollowTarget;	///< if true, attack object, not pos
	UnsignedInt			m_fuelLifetime;				///< num frames till missile runs out of motive power (0 == inf)
	UnsignedInt			m_ignitionDelay;			///< delay in frames from when missile is 'fired', to when it starts moving		15
	Real						m_initialVel;			
	Real						m_initialDist;
	Real						m_diveDistance;				///< If I get this close to my target, start ignoring my preferred height
	const FXList*		m_ignitionFX;					///< FXList to do when missile 'ignites'
	Bool						m_useWeaponSpeed;			///< if true, limit speed of projectile to the Weapon's info
	Bool						m_detonateOnNoFuel;		///< If true, don't just stop thrusting, blow up when out of gas
	Int							m_garrisonHitKillCount;
	KindOfMaskType	m_garrisonHitKillKindof;			///< the kind(s) of units that can be collided with
	KindOfMaskType	m_garrisonHitKillKindofNot;		///< the kind(s) of units that CANNOT be collided with
	const FXList*		m_garrisonHitKillFX;
	Real						m_distanceScatterWhenJammed;	///< How far I scatter when Jammed

	Real						m_lockDistance;				///< If I get this close to my target, guaranteed hit.
	Bool						m_detonateCallsKill;			///< if true, kill() will be called, instead of KILL_SELF state, which calls destroy.
  Int             m_killSelfDelay;      ///< If I have detonated and entered the KILL-SELF state, how ling do I wait before I Kill/destroy self?
	MissileAIUpdateModuleData();

	static void buildFieldParse(MultiIniFieldParse& p);

};

//-------------------------------------------------------------------------------------------------
class MissileAIUpdate : public AIUpdateInterface, public ProjectileUpdateInterface
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( MissileAIUpdate, "MissileAIUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( MissileAIUpdate, MissileAIUpdateModuleData );

public:
	MissileAIUpdate( Thing *thing, const ModuleData* moduleData );

	enum MissileStateType	 // Stored in save file, don't renumber.
	{
		PRELAUNCH			= 0,
		LAUNCH				= 1, ///< released from launcher, falling
		IGNITION			= 2, ///< engines ignite
		ATTACK_NOTURN	= 3, ///< fly toward victim
		ATTACK				= 4, ///< fly toward victim
		DEAD					= 5,
		KILL					= 6, ///< Hit victim (cheat).
		KILL_SELF			= 7, ///< Destroy self.
	};

	virtual ProjectileUpdateInterface* getProjectileUpdateInterface() { return this; }
	virtual void projectileFireAtObjectOrPosition( const Object *victim, const Coord3D *victimPos, const WeaponTemplate *detWeap, const ParticleSystemTemplate* exhaustSysOverride );
	virtual void projectileLaunchAtObjectOrPosition(const Object *victim, const Coord3D* victimPos, const Object *launcher, WeaponSlotType wslot, Int specificBarrelToUse, const WeaponTemplate* detWeap, const ParticleSystemTemplate* exhaustSysOverride);
	virtual Bool projectileHandleCollision( Object *other );
	virtual Bool projectileIsArmed() const { return m_isArmed; }
	virtual ObjectID projectileGetLauncherID() const { return m_launcherID; }
	virtual void setFramesTillCountermeasureDiversionOccurs( UnsignedInt frames ); ///< Number of frames till missile diverts to countermeasures.
	virtual void projectileNowJammed();///< We lose our Object target and scatter to the ground

	virtual Bool processCollision(PhysicsBehavior *physics, Object *other); ///< Returns true if the physics collide should apply the force.  Normally not.  jba.

	virtual UpdateSleepTime update();
	virtual void onDelete( void );


protected:

	void detonate();

private:

	MissileStateType			m_state;									///< the behavior state of the missile
	UnsignedInt						m_stateTimestamp;					///< time of state change
	UnsignedInt						m_nextTargetTrackTime;		///< if nonzero, how often we update our target pos
	ObjectID							m_launcherID;							///< ID of object that launched us (INVALID_ID if not yet launched)
	ObjectID							m_victimID;								///< ID of object that I am rocketing towards (INVALID_ID if not yet launched)
	UnsignedInt						m_fuelExpirationDate;			///< how long 'til we run out of fuel
	Real									m_noTurnDistLeft;					///< when zero, ok to start turning
	Real									m_maxAccel;
	Coord3D								m_originalTargetPos;			///< When firing uphill, we aim high to clear the brow of the hill.  jba.
	Coord3D								m_prevPos;
	WeaponBonusConditionFlags		m_extraBonusFlags;
	const WeaponTemplate*	m_detonationWeaponTmpl;		///< weapon to fire at end (or null)
	const ParticleSystemTemplate* m_exhaustSysTmpl;
	ParticleSystemID			m_exhaustID;								///< our exhaust particle system (if any)
	UnsignedInt						m_framesTillDecoyed;			///< Number of frames before missile will get distracted by decoy countermeasures.
	Bool									m_isTrackingTarget;				///< Was I originally shot at a moving object?
	Bool									m_isArmed;								///< if true, missile will explode on contact
	Bool									m_noDamage;								///< if true, missile will not cause damage when it detonates. (Used for flares).
	Bool									m_isJammed;								///< No target, just shooting at a scattered position
	
	void doPrelaunchState();
	void doLaunchState();
	void doIgnitionState();
	void doAttackState(Bool turnOK);
	void doKillState();
	void doKillSelfState();
	void doDeadState();

	void airborneTargetGone();											///< My airborne target has died, so I have to do something cool to make up for that

	void tossExhaust();
	void switchToState(MissileStateType s);


};

#endif // _MISSILE_AI_UPDATE_H_

