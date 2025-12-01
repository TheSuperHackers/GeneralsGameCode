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

// FILE: FireWeaponWhenDeadBehavior.h /////////////////////////////////////////////////////////////////////////
// Author: Colin Day, December 2001
// Desc:   Update that will count down a lifetime and destroy object when it reaches zero
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/DieModule.h"
#include "GameLogic/Module/UpgradeModule.h"


//-------------------------------------------------------------------------------------------------
class FireWeaponWhenDeadBehaviorModuleData : public BehaviorModuleData
{
public:
	UpgradeMuxData				m_upgradeMuxData;
	Bool									m_initiallyActive;
	DieMuxData						m_dieMuxData;
	const WeaponTemplate* m_deathWeapon;						///< fire this weapon when we are damaged

	FireWeaponWhenDeadBehaviorModuleData()
	{
		m_initiallyActive = false;
		m_deathWeapon = NULL;
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
		static const FieldParse dataFieldParse[] =
		{
			{ "StartsActive",	INI::parseBool, NULL, offsetof( FireWeaponWhenDeadBehaviorModuleData, m_initiallyActive ) },
			{ "DeathWeapon", INI::parseWeaponTemplate,	NULL, offsetof( FireWeaponWhenDeadBehaviorModuleData, m_deathWeapon ) },
			{ 0, 0, 0, 0 }
		};

		BehaviorModuleData::buildFieldParse(p);
		p.add(dataFieldParse);
		p.add(UpgradeMuxData::getFieldParse(), offsetof( FireWeaponWhenDeadBehaviorModuleData, m_upgradeMuxData ));
		p.add(DieMuxData::getFieldParse(), offsetof( FireWeaponWhenDeadBehaviorModuleData, m_dieMuxData ));
	}
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class FireWeaponWhenDeadBehavior : public BehaviorModule,
																	 public UpgradeMux,
																	 public DieModuleInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( FireWeaponWhenDeadBehavior, "FireWeaponWhenDeadBehavior" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( FireWeaponWhenDeadBehavior, FireWeaponWhenDeadBehaviorModuleData )

public:

	FireWeaponWhenDeadBehavior( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	// module methods
	static Int getInterfaceMask() { return BehaviorModule::getInterfaceMask() | (MODULEINTERFACE_UPGRADE) | (MODULEINTERFACE_DIE); }

	// BehaviorModule
	virtual UpgradeModuleInterface* getUpgrade() { return this; }
	virtual DieModuleInterface* getDie() { return this; }

	// DamageModuleInterface
	virtual void onDie( const DamageInfo *damageInfo );

protected:

	virtual void upgradeImplementation()
	{
		// nothing!
	}

	virtual void getUpgradeActivationMasks(UpgradeMaskType& activation, UpgradeMaskType& conflicting) const
	{
		getFireWeaponWhenDeadBehaviorModuleData()->m_upgradeMuxData.getUpgradeActivationMasks(activation, conflicting);
	}

	virtual void performUpgradeFX()
	{
		getFireWeaponWhenDeadBehaviorModuleData()->m_upgradeMuxData.performUpgradeFX(getObject());
	}

	virtual Bool requiresAllActivationUpgrades() const
	{
		return getFireWeaponWhenDeadBehaviorModuleData()->m_upgradeMuxData.m_requiresAllTriggers;
	}

	Bool isUpgradeActive() const { return isAlreadyUpgraded(); }

	virtual Bool isSubObjectsUpgrade() { return false; }

};
