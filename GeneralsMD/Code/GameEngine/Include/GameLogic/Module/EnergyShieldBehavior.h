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

// FILE: EnergyShieldBehavior.h /////////////////////////////////////////////////////////////////////////
// Author: Colin Day, December 2001
// Desc:   Update that heals itself
//------------------------------------------
// Modified by Kris Morness, September 2002
// Kris: Added the ability to add effects, radius healing, and restricting the type of objects
//       subjected to the heal (or repair).
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __EnergyShieldBehavior_H_
#define __EnergyShieldBehavior_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameClient/ParticleSys.h"
#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/UpgradeModule.h"
#include "GameLogic/Module/UpdateModule.h"
#include "GameLogic/Module/DamageModule.h"
#include "Common/BitFlagsIO.h"


//-------------------------------------------------------------------------------------------------
class EnergyShieldBehaviorModuleData : public UpdateModuleData
{
public:
	UpgradeMuxData				m_upgradeMuxData;
	Bool									m_initiallyActive;
	//Real m_shieldMaxHealth;  ///< MaxHealth of the shield
	//Real m_shieldMaxHealthPercent;  ///< MaxHealth as percentage of activeBody MaxHealth (takes priority)

	UnsignedInt m_shieldRechargeDelay;  ///< frames of no damage taken until shield recharges
	UnsignedInt m_shieldRechargeRate;   ///< every this often, we heal the shield ...
	Real m_shieldRechargeAmount;					///< by this much.
	Real m_shieldRechargeAmountPercent;		///< Same as above but percentage of shieldMaxHealth (takes priority)

	//DamageTypeFlags m_damageTypesToPassThrough;

	EnergyShieldBehaviorModuleData();

	static void buildFieldParse(MultiIniFieldParse& p);

	//static void parseShieldHealthPercent(INI* ini, void* instance, void* store, const void* userData);
	//static void parseShieldRechargeAmountPercent(INI* ini, void* instance, void* store, const void* userData);

};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class EnergyShieldBehaviorInterface
{
public:
	virtual void applyDamage(Real amount) = 0;
	virtual bool isActive() const = 0;
	virtual Bool getShieldPercent(Real& percentage) const = 0;
};
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class EnergyShieldBehavior : public UpdateModule,
												 public UpgradeMux,
												 public EnergyShieldBehaviorInterface
												 //public DamageModuleInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( EnergyShieldBehavior, "EnergyShieldBehavior" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( EnergyShieldBehavior, EnergyShieldBehaviorModuleData )

public:

	EnergyShieldBehavior( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	// module methods
	static Int getInterfaceMask() { return UpdateModule::getInterfaceMask() | MODULEINTERFACE_UPGRADE; }  // | MODULEINTERFACE_DAMAGE; 

	// BehaviorModule
	virtual UpgradeModuleInterface* getUpgrade() { return this; }
	virtual EnergyShieldBehaviorInterface* getEnergyShieldBehaviorInterface() { return this; }
	//virtual DamageModuleInterface* getDamage() { return this; }

	// DamageModuleInterface
	//virtual void onDamage( DamageInfo *damageInfo );
	//virtual void onHealing( DamageInfo *damageInfo ) { }
	//virtual void onBodyDamageStateChange(const DamageInfo* damageInfo, BodyDamageType oldState, BodyDamageType newState) { }

	// UpdateModuleInterface
	virtual UpdateSleepTime update();
	virtual DisabledMaskType getDisabledTypesToProcess() const { return DISABLEDMASK_ALL; }

	//EnergyShieldBehaviorInterface
	virtual void applyDamage(Real amount);
	virtual bool isActive() const { return isUpgradeActive(); }
	virtual Bool getShieldPercent(Real& percentage) const;

protected:

	virtual void upgradeImplementation()
	{
		setWakeFrame(getObject(), UPDATE_SLEEP_NONE);
	}

	virtual void getUpgradeActivationMasks(UpgradeMaskType& activation, UpgradeMaskType& conflicting) const
	{
		getEnergyShieldBehaviorModuleData()->m_upgradeMuxData.getUpgradeActivationMasks(activation, conflicting);
	}

	virtual void performUpgradeFX()
	{
		getEnergyShieldBehaviorModuleData()->m_upgradeMuxData.performUpgradeFX(getObject());
	}

	virtual void processUpgradeRemoval()
	{
		// I can't take it any more.  Let the record show that I think the UpgradeMux multiple inheritence is CRAP.
		getEnergyShieldBehaviorModuleData()->m_upgradeMuxData.muxDataProcessUpgradeRemoval(getObject());
	}

	virtual Bool requiresAllActivationUpgrades() const
	{
		return getEnergyShieldBehaviorModuleData()->m_upgradeMuxData.m_requiresAllTriggers;
	}

	inline Bool isUpgradeActive() const { return isAlreadyUpgraded(); }

	virtual Bool isSubObjectsUpgrade() { return false; }


private:

	ShieldBody*           m_body;
	//Real									m_currentShieldHealth;
	UnsignedInt           m_healingStepCountdown;

};

#endif // __EnergyShieldBehavior_H_

