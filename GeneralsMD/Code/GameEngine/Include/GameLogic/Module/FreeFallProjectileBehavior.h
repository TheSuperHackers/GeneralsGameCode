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

// FILE:		FreeFallProjectileBehavior.h
// Author:	Andi W, June 2025
// Desc:		

#pragma once

#ifndef _FreeFallProjectileBehavior_H_
#define _FreeFallProjectileBehavior_H_

#include "Common/GameType.h"
#include "Common/GlobalData.h"
#include "Common/STLTypedefs.h"
#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/CollideModule.h"
#include "GameLogic/Module/UpdateModule.h"
#include "GameLogic/WeaponBonusConditionFlags.h"
#include "Common/INI.h"
#include "WWMath/matrix3d.h"

class ParticleSystem;
class FXList;


//-------------------------------------------------------------------------------------------------
class FreeFallProjectileBehaviorModuleData : public UpdateModuleData
{
public:
	/**
		These four data define a Bezier curve.  The first and last control points are the firer and victim.
	*/
	
	UnsignedInt m_maxLifespan;
	Bool m_tumbleRandomly;
	Real m_courseCorrectionScalar;
	Real m_exitPitchRate;
	Bool m_applyLauncherBonus;
	// Bool m_inheritTransportVelocity;
	Bool m_useWeaponSpeed;
	Bool m_detonateCallsKill;

	Bool m_detonateOnGround;
	Bool m_detonateOnCollide;

	Int				m_garrisonHitKillCount;
	KindOfMaskType	m_garrisonHitKillKindof;			///< the kind(s) of units that can be collided with
	KindOfMaskType	m_garrisonHitKillKindofNot;		///< the kind(s) of units that CANNOT be collided with
	const FXList*   m_garrisonHitKillFX;

	FreeFallProjectileBehaviorModuleData();

	static void buildFieldParse(MultiIniFieldParse& p);

};

//-------------------------------------------------------------------------------------------------
class FreeFallProjectileBehavior : public UpdateModule, public ProjectileUpdateInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( FreeFallProjectileBehavior, "FreeFallProjectileBehavior" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( FreeFallProjectileBehavior, FreeFallProjectileBehaviorModuleData );

public:

	FreeFallProjectileBehavior( Thing *thing, const ModuleData* moduleData );
	// virtual destructor provided by memory pool object

	// UpdateModuleInterface
	virtual UpdateSleepTime update();
	virtual ProjectileUpdateInterface* getProjectileUpdateInterface() { return this; }

	// ProjectileUpdateInterface
	virtual void projectileLaunchAtObjectOrPosition(const Object *victim, const Coord3D* victimPos, const Object *launcher, WeaponSlotType wslot, Int specificBarrelToUse, const WeaponTemplate* detWeap, const ParticleSystemTemplate* exhaustSysOverride);
	virtual void projectileFireAtObjectOrPosition( const Object *victim, const Coord3D *victimPos, const WeaponTemplate *detWeap, const ParticleSystemTemplate* exhaustSysOverride );
	virtual Bool projectileHandleCollision( Object *other );
	virtual Bool projectileIsArmed() const { return true; }
	virtual ObjectID projectileGetLauncherID() const { return m_launcherID; }
	virtual void setFramesTillCountermeasureDiversionOccurs( UnsignedInt frames ) {}
	virtual void projectileNowJammed() {}
	virtual Object* getTargetObject();
	virtual const Coord3D* getTargetPosition();
	virtual bool projectileShouldCollideWithWater() const override;

protected:

	void positionForLaunch(const Object *launcher, WeaponSlotType wslot, Int specificBarrelToUse);
	void detonate();

private:

	ObjectID							m_launcherID;							///< ID of object that launched us (zero if not yet launched)
	ObjectID							m_victimID;								///< ID of object we are targeting (zero if not yet launched)
	Coord3D								m_targetPos;
	const WeaponTemplate*	m_detonationWeaponTmpl;		///< weapon to fire at end (or null)
	UnsignedInt						m_lifespanFrame;					///< if we haven't collided by this frame, blow up anyway
	WeaponBonusConditionFlags		m_extraBonusFlags;
  
    Bool                  m_hasDetonated;           ///< 

};

#endif // _FreeFallProjectileBehavior_H_

