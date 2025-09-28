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

// FILE: ScatterShotUpdate.h //////////////////////////////////////////////////////////////////////
// Author: Andi W, April 2025
// Desc:   Projectile Scatter/Airburst module
///////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#ifndef __SCATTER_SHOT_UPDATE_H_
#define __SCATTER_SHOT_UPDATE_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpdateModule.h"
#include "GameLogic/Object.h"
#include "GameLogic/Weapon.h"
#include "GameLogic/Module/DieModule.h"

class FXList;
enum DeathType;

//-------------------------------------------------------------------------------------------------
class ScatterShotUpdateModuleData : public UpdateModuleData
{
public:
	const WeaponTemplate* m_weaponTemplate;
	UnsignedInt m_numShots;
	Real m_targetSearchRadius;
	Real m_targetMinRadius;
	UnsignedInt m_maxShotsPerTarget;
	Bool m_preferSimilarTargets;
	Bool m_preferNearestTargets;
	Real m_noTargetsScatterRadius;
	Real m_noTargetsScatterMinRadius;
	Real m_noTargetsScatterMaxAngle;
	Bool m_attackGroundWhenNoTargets;

	Real m_triggerDistanceToTarget;
	Real m_triggerDistanceFromSource;
	Real m_triggerDistancePercent;
	UnsignedInt m_triggerLifetime;
	Bool m_triggerOnImpact;
	Bool m_triggerInstantly;

	Bool m_stayAliveAfterTrigger;
	Bool m_avoidOriginalTarget;
	Bool m_avoidPrevTarget;

	DeathType m_triggerDeathType;
	const FXList* m_scatterFX;


	ScatterShotUpdateModuleData();

	static void buildFieldParse(MultiIniFieldParse& p);

private:

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class ScatterShotUpdate : public UpdateModule, public DieModuleInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(ScatterShotUpdate, "ScatterShotUpdate")
		MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA(ScatterShotUpdate, ScatterShotUpdateModuleData)

public:

	ScatterShotUpdate(Thing* thing, const ModuleData* moduleData);
	// virtual destructor prototype provided by memory pool declaration

	virtual UpdateSleepTime update();

	static Int getInterfaceMask() { return MODULEINTERFACE_DIE; }

	// BehaviorModule
	virtual DieModuleInterface* getDie() { return this; }

	void onDie(const DamageInfo* damageInfo);

protected:

	Weapon* m_weapon;
	UnsignedInt m_initialDelayFrame;
	Bool m_hasTriggered;
	Bool m_isInitialized; // are goal pos/obj set

	Coord3D m_goalPos;
	Object* m_goalObj;
	ObjectID m_prevTargetID;  //< target of previous attack. only used for special case when chaining multiple targets

	Real m_totalTargetDistance;

	void triggerScatterShot(void);
	Bool isValidTarget(const Object* victim) const;

private:
	Real getTargetDistance() const;

};

#endif

