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

// FILE: PoisonedBehavior.h /////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, July 2002
// Desc:   Behavior that reacts to poison Damage by continuously damaging us further in an Update
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __Poisoned_Behavior_H_
#define __Poisoned_Behavior_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/DamageModule.h"
#include "GameLogic/Module/UpdateModule.h"


//-------------------------------------------------------------------------------------------------
class PoisonedBehaviorModuleData : public UpdateModuleData
{
public:
	UnsignedInt m_poisonDamageIntervalData; // How often I retake poison damage dealt me
	UnsignedInt m_poisonDurationData;				// And how long after the last poison dose I am poisoned

	PoisonedBehaviorModuleData();

	static void buildFieldParse(MultiIniFieldParse& p);

private:

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class PoisonedBehavior : public UpdateModule,
												 public DamageModuleInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( PoisonedBehavior, "PoisonedBehavior" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( PoisonedBehavior, PoisonedBehaviorModuleData )

public:

	PoisonedBehavior( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	static Int getInterfaceMask() { return UpdateModule::getInterfaceMask() | (MODULEINTERFACE_DAMAGE); }

	// BehaviorModule
	virtual DamageModuleInterface* getDamage() { return this; }

	// DamageModuleInterface
	virtual void onDamage( DamageInfo *damageInfo );
	virtual void onHealing( DamageInfo *damageInfo );
	virtual void onBodyDamageStateChange(const DamageInfo* damageInfo, BodyDamageType oldState, BodyDamageType newState) { }

	// UpdateInterface
	virtual UpdateSleepTime update();
	// Disabled conditions to process (we should still poison disabled things)
	virtual DisabledMaskType getDisabledTypesToProcess() const { return DISABLEDMASK_ALL; }

protected:

	void startPoisonedEffects( const DamageInfo *damageInfo );
	void stopPoisonedEffects();
	UpdateSleepTime calcSleepTime();

private:
	UnsignedInt		m_poisonDamageFrame;
	UnsignedInt		m_poisonOverallStopFrame;
	Real					m_poisonDamageAmount;
	ObjectID			m_poisonSource;
	DeathType			m_deathType;

};

#endif // __Poisoned_Behavior_H_

