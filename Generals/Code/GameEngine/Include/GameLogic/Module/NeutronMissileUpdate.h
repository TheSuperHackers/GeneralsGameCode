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

// FILE:		NeutronMissileUpdate.h
// Author:	Michael S. Booth, December 2001
// Desc:		Missile behavior

#pragma once

#ifndef _MISSILE_UPDATE_H_
#define _MISSILE_UPDATE_H_

#include "GameClient/RadiusDecal.h"
#include "Common/GameType.h"
#include "Common/GlobalData.h"
#include "GameLogic/Module/UpdateModule.h"
#include "GameLogic/Module/DieModule.h"
#include "Common/INI.h"
#include "WWMath/matrix3d.h"

enum ParticleSystemID;
class FXList;

//-------------------------------------------------------------------------------------------------
class NeutronMissileUpdateModuleData : public UpdateModuleData
{
public:
	Real					m_initialDist;
	Real					m_maxTurnRate;		
	Real					m_forwardDamping;
	Real					m_relativeSpeed;
	Real					m_targetFromDirectlyAbove;	///< aim first for dest+offset, then dest
	Real					m_specialAccelFactor;
	UnsignedInt		m_specialSpeedTime;
	Real					m_specialSpeedHeight;
	Real					m_specialJitterDistance;
	const FXList*	m_launchFX;			///< FXList to do when missile 'launches'
	const FXList*	m_ignitionFX;			///< FXList to do when missile 'ignites'
	RadiusDecalTemplate	m_deliveryDecalTemplate;
	Real					m_deliveryDecalRadius;

	NeutronMissileUpdateModuleData();

	static void buildFieldParse(MultiIniFieldParse& p);

};

//-------------------------------------------------------------------------------------------------
/**
 * This module encapsulates missile behavior.
 */
class NeutronMissileUpdate : public UpdateModule, 
	public DieModuleInterface,
	public ProjectileUpdateInterface
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( NeutronMissileUpdate, "NeutronMissileUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( NeutronMissileUpdate, NeutronMissileUpdateModuleData );

public:
	NeutronMissileUpdate( Thing *thing, const ModuleData* moduleData );

	static Int getInterfaceMask() { return UpdateModule::getInterfaceMask() | (MODULEINTERFACE_DIE); }

	// BehaviorModule
	virtual DieModuleInterface* getDie() { return this; }

	// DieModuleInterface
	virtual void onDie( const DamageInfo *damageInfo );

	virtual ProjectileUpdateInterface* getProjectileUpdateInterface() { return this; }

	enum MissileStateType
	{
		PRELAUNCH,
		LAUNCH,			///< released from plane, falling
		ATTACK,			///< fly toward victim
		DEAD
	};

	virtual void projectileLaunchAtObjectOrPosition(const Object *victim, const Coord3D* victimPos, const Object *launcher, WeaponSlotType wslot, Int specificBarrelToUse, const WeaponTemplate* detWeap, const ParticleSystemTemplate* exhaustSysOverride);
	virtual void projectileFireAtObjectOrPosition( const Object *victim, const Coord3D *victimPos, const WeaponTemplate *detWeap, const ParticleSystemTemplate* exhaustSysOverride );
	virtual Bool projectileIsArmed() const { return m_isArmed; }											///< return true if the missile is armed and ready to explode
	virtual ObjectID projectileGetLauncherID() const { return m_launcherID; }				///< Return firer of missile. Returns 0 if not yet fired.
	virtual Bool projectileHandleCollision( Object *other );
	virtual const Coord3D *getVelocity() const { return &m_vel; }		///< get current velocity

	virtual UpdateSleepTime update();
	virtual void onDelete( void );

private:

	MissileStateType m_state;						///< the behavior state of the missile
	Coord3D m_targetPos;								///< the position of the target
	Coord3D m_intermedPos;

	ObjectID m_launcherID;							///< ID of object that launched us (zero if not yet launched)
	WeaponSlotType m_attach_wslot;			///< where to fire the missile from
	Int m_attach_specificBarrelToUse;							///< where to fire the missile from

	Coord3D m_accel;
	Coord3D m_vel;

	UnsignedInt m_stateTimestamp;				///< time of state change
	Bool m_isLaunched;							
	Bool m_isArmed;											///< if true, missile will explode on contact
	Real m_noTurnDistLeft;				///< when zero, ok to start turning
	Bool m_reachedIntermediatePos;
	UnsignedInt m_frameAtLaunch;
	Real m_heightAtLaunch;
	RadiusDecal	m_deliveryDecal;

	const ParticleSystemTemplate* m_exhaustSysTmpl;

	void doLaunch( void );							///< implement LAUNCH state
	void doAttack( void );							///< implement ATTACK state
	void detonate();												///< blow it up. (usually only called by MissileCollide)


};

#endif // _MISSILE_UPDATE_H_

