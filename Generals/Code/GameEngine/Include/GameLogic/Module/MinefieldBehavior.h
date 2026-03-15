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

// FILE: MinefieldBehavior.h ////////////////////////////////////////////////////////////////////
// Author: Steven Johnson, June 2002
// Desc:   Minefield behavior
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/CollideModule.h"
#include "GameLogic/Module/UpdateModule.h"
#include "GameLogic/Module/DamageModule.h"
#include "GameLogic/Module/DieModule.h"

//-------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class MinefieldBehaviorModuleData : public UpdateModuleData
{

public:

	MinefieldBehaviorModuleData();

	static void buildFieldParse( MultiIniFieldParse &p );

	const WeaponTemplate*	m_detonationWeapon;								///< what happens when we detonate
	Int										m_detonatedBy;										///< can we be triggered by allies, etc?
	Bool									m_stopsRegenAfterCreatorDies;
	Bool									m_regenerates;										///< if t, can't be killed normally
	Bool									m_workersDetonate;								///< if f, workers don't detonate mines
	UnsignedInt						m_creatorDeathCheckRate;					///< if above is true, how often to check
	UnsignedInt						m_scootFromStartingPointTime;			///< if nonzero, gradually scoot to dest pt
	UnsignedInt						m_numVirtualMines;								///< num of "virtual" mines we have
	Real									m_repeatDetonateMoveThresh;
	Real									m_healthPercentToDrainPerSecond;

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class MinefieldBehavior : public UpdateModule,
													public CollideModuleInterface,
													public DamageModuleInterface,
													public DieModuleInterface,
													public LandMineInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( MinefieldBehavior, "MinefieldBehavior" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( MinefieldBehavior, MinefieldBehaviorModuleData )

public:

	MinefieldBehavior( Thing *thing, const ModuleData *moduleData );
	// virtual destructor prototype provided by memory pool declaration

	static Int getInterfaceMask() { return UpdateModule::getInterfaceMask() | (MODULEINTERFACE_COLLIDE) | (MODULEINTERFACE_DAMAGE) | (MODULEINTERFACE_DIE); }

	// BehaviorModule
	virtual CollideModuleInterface* getCollide() override { return this; }
	virtual LandMineInterface* getLandMineInterface() override { return this; }
	virtual DamageModuleInterface* getDamage() override { return this; }
	virtual DieModuleInterface* getDie() override { return this; }

	// DamageModuleInterface
	virtual void onDamage( DamageInfo *damageInfo ) override;
	virtual void onHealing( DamageInfo *damageInfo ) override;
	virtual void onBodyDamageStateChange(const DamageInfo* damageInfo, BodyDamageType oldState, BodyDamageType newState) override { }

	// DieModuleInterface
	virtual void onDie( const DamageInfo *damageInfo ) override;

	// UpdateModuleInterface
	virtual UpdateSleepTime update() override;

	// CollideModuleInterface
	virtual void onCollide( Object *other, const Coord3D *loc, const Coord3D *normal ) override;
	virtual Bool wouldLikeToCollideWith(const Object* other) const override { return false; }
	virtual Bool isHijackedVehicleCrateCollide() const override { return false; }
	virtual Bool isCarBombCrateCollide() const override { return false; }
	virtual Bool isRailroad() const override { return false;}
	virtual Bool isSalvageCrateCollide() const override { return false; }

	// Minefield specific methods
	virtual void setScootParms(const Coord3D& start, const Coord3D& end) override;
	virtual void disarm() override;

private:

	// mines are small, so we can get by with a small fixed number here
	enum { MAX_IMMUNITY = 3 };
	struct ImmuneInfo
	{
		ObjectID id;
		UnsignedInt collideTime;
	};

	struct DetonatorInfo
	{
		ObjectID id;
		Coord3D where;
	};

	UnsignedInt		m_nextDeathCheckFrame;
	UnsignedInt		m_scootFramesLeft;
	Coord3D				m_scootVel;
	Coord3D				m_scootAccel;
	UnsignedInt		m_virtualMinesRemaining;
	ImmuneInfo		m_immunes[MAX_IMMUNITY];
	std::vector<DetonatorInfo>	m_detonators;
	Bool					m_ignoreDamage;
	Bool					m_regenerates;
	Bool					m_draining;

	void detonateOnce(const Coord3D& position);
	UpdateSleepTime calcSleepTime();

};
