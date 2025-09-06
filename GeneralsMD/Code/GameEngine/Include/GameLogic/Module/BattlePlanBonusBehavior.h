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

// FILE: BattlePlanBonusBehavior.h /////////////////////////////////////////////////////////////////////////
// Author: Andi W, September 2025
// Desc:   Behavior to react to battlePlans being applied or removed to this object
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __BATTLEPLANBONUSBEHAVIOR_H_
#define __BATTLEPLANBONUSBEHAVIOR_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////

#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/UpgradeModule.h"
#include "GameLogic/Module/BattlePlanUpdate.h"

enum BattlePlanStatus CPP_11(: Int);
enum WeaponBonusConditionType CPP_11(: Int);
enum WeaponSetType CPP_11(: Int);
enum ArmorSetType CPP_11(: Int);
enum ObjectStatusTypes CPP_11(: Int);

enum { BATTLE_PLAN_COUNT = PLANSTATUS_COUNT-1 };

//-------------------------------------------------------------------------------------------------
class BattlePlanBonusBehaviorModuleData : public BehaviorModuleData
{
public:
	UpgradeMuxData				m_upgradeMuxData;

	Bool						m_initiallyActive;   // Apply upgrade immediately (Does this make sense?)
	Bool						m_overrideGlobal;    // Do not apply effects from global BattlePlan bonus
	Bool						m_shouldParalyze;    // Paralyze this unit when applying BattlePlans

	WeaponBonusConditionType m_weaponBonusEntries[BATTLE_PLAN_COUNT];
	WeaponSetType m_weaponSetFlagEntries[BATTLE_PLAN_COUNT];
	ArmorSetType m_armorSetFlagEntries[BATTLE_PLAN_COUNT];
	Real m_armorDamageScalarEntries[BATTLE_PLAN_COUNT];
	Real m_sightRangeScalarEntries[BATTLE_PLAN_COUNT];
	//Real m_movementSpeedScalarEntries[BATTLE_PLAN_COUNT];
	ObjectStatusTypes m_statusToSetEntries[BATTLE_PLAN_COUNT];
	ObjectStatusTypes m_statusToClearEntries[BATTLE_PLAN_COUNT];

	//std::vector<std::vector<WeaponBonusConditionType>> m_weaponBonusEntries;
	//std::vector<std::vector<WeaponSetType>> m_weaponSetFlagEntries;
	//std::vector<std::vector<WeaponBonusConditionType>> m_armorSetFlagEntries;
	//std::vector<std::vector<WeaponBonusConditionType>> m_armorDamageScalarEntries;
	//std::vector<std::vector<WeaponBonusConditionType>> m_sightRangeScalarEntries;
	//std::vector<std::vector<WeaponBonusConditionType>> m_movementSpeedScalarEntries;

	BattlePlanBonusBehaviorModuleData();

	static void buildFieldParse(MultiIniFieldParse& p);

private:
	static void parseBPWeaponBonus(INI* ini, void* instance, void* store, const void* userData);
	static void parseBPWeaponSetFlag(INI* ini, void* instance, void* store, const void* userData);
	static void parseBPArmorSetFlag(INI* ini, void* instance, void* store, const void* userData);
	static void parseBPArmorDamageScalar(INI* ini, void* instance, void* store, const void* userData);
	static void parseBPSightRangeScalar(INI* ini, void* instance, void* store, const void* userData);
	//static void parseBPMovementSpeedScalar(INI* ini, void* instance, void* store, const void* userData);
	static void parseBPStatusToSet(INI* ini, void* instance, void* store, const void* userData);
	static void parseBPStatusToClear(INI* ini, void* instance, void* store, const void* userData);
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class BattlePlanBonusBehaviorInterface
{
public:
	virtual void applyBonus(const BattlePlanBonuses* bonus, bool checkIsValid = TRUE) = 0;
	//virtual void removeBonus(const BattlePlanBonuses* bonus) = 0;
	virtual Bool shouldParalyze() const = 0;
	virtual Bool isOverrideGlobalBonus() const = 0;
	virtual Bool isConflicting() const = 0;
	virtual Bool isAnyEffectApplied() const = 0;
	virtual void removeActiveEffects() = 0;
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class BattlePlanBonusBehavior : public BehaviorModule, public UpgradeMux, public BattlePlanBonusBehaviorInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(BattlePlanBonusBehavior, "BattlePlanBonusBehavior")
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA(BattlePlanBonusBehavior, BattlePlanBonusBehaviorModuleData)

private:
	//UnsignedInt m_triggerFrame;
	//// UnsignedInt m_shotsLeft;
	//// TODO: Which weaponslot
	//Bool m_triggerCompleted;

public:

	BattlePlanBonusBehavior(Thing* thing, const ModuleData* moduleData);
	// virtual destructor prototype provided by memory pool declaration

	// module methods
	static Int getInterfaceMask() { return BehaviorModule::getInterfaceMask() | MODULEINTERFACE_UPGRADE; }

	// BehaviorModule
	virtual UpgradeModuleInterface* getUpgrade() { return this; }
	virtual BattlePlanBonusBehaviorInterface* getBattlePlanBonusBehaviorInterface() { return this; }

protected:
	// BattlePlan stuff

	void applyBonus(const BattlePlanBonuses*, bool checkIsValid = TRUE);
	//void removeBonus(const BattlePlanBonuses*);
	Bool shouldParalyze() const;
	Bool isOverrideGlobalBonus() const;

	Bool isConflicting() const;
	Bool isAnyEffectApplied() const;
	void removeActiveEffects();

	// Upgrade stuff

	virtual void upgradeImplementation();

	virtual void getUpgradeActivationMasks(UpgradeMaskType& activation, UpgradeMaskType& conflicting) const
	{
		getBattlePlanBonusBehaviorModuleData()->m_upgradeMuxData.getUpgradeActivationMasks(activation, conflicting);
	}

	virtual void performUpgradeFX()
	{
		getBattlePlanBonusBehaviorModuleData()->m_upgradeMuxData.performUpgradeFX(getObject());
	}

	virtual void processUpgradeRemoval()
	{
		// I can't take it any more.  Let the record show that I think the UpgradeMux multiple inheritence is CRAP.
		getBattlePlanBonusBehaviorModuleData()->m_upgradeMuxData.muxDataProcessUpgradeRemoval(getObject());
	}

	virtual Bool requiresAllActivationUpgrades() const
	{
		return getBattlePlanBonusBehaviorModuleData()->m_upgradeMuxData.m_requiresAllTriggers;
	}

	inline Bool isUpgradeActive() const { return isAlreadyUpgraded(); }

	virtual Bool isSubObjectsUpgrade() { return false; }

private:
	Bool isEffectValid() const;
	void addBonusForType(BattlePlanStatus plan);
	void removeBonusForType(BattlePlanStatus plan);

	Bool m_effectApplied[BATTLE_PLAN_COUNT];
};

#endif // __BattlePlanBonusBehavior_H_

