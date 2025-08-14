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

// FILE: FireOCLAfterWeaponCooldownUpdate.h ////////////////////////////////////////////////////////////////////////
// Author: Kris Morness, September 2002
// Desc:   This system tracks the objects status with regards to firing, and whenever the object stops
//         firing, and all the conditions are met, then it'll create the specified OCL.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __FIRE_OCL_AFTER_WEAPON_COOLDOWN_UPDATE_H
#define __FIRE_OCL_AFTER_WEAPON_COOLDOWN_UPDATE_H

class UpgradeMuxData;

#include "GameLogic/Module/UpdateModule.h"
#include "GameLogic/Module/UpgradeModule.h"

//-------------------------------------------------------------------------------------------------
class FireOCLAfterWeaponCooldownUpdateModuleData : public UpdateModuleData
{
public:
	UpgradeMuxData			m_upgradeMuxData;
	ObjectCreationList	*m_ocl;
	WeaponSlotType			m_weaponSlot;
	UnsignedInt					m_minShotsRequired;
	UnsignedInt					m_oclLifetimePerSecond;
	UnsignedInt					m_oclMaxFrames;

	FireOCLAfterWeaponCooldownUpdateModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

//-------------------------------------------------------------------------------------------------
class FireOCLAfterWeaponCooldownUpdate : public UpdateModule, public UpgradeMux
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( FireOCLAfterWeaponCooldownUpdate, "FireOCLAfterWeaponCooldownUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( FireOCLAfterWeaponCooldownUpdate, FireOCLAfterWeaponCooldownUpdateModuleData )

public:

	FireOCLAfterWeaponCooldownUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	// update methods
	virtual UpdateSleepTime update();							///< called once per frame

protected:
	virtual void upgradeImplementation()
	{
		// nothing!
	}

	virtual void getUpgradeActivationMasks(UpgradeMaskType& activation, UpgradeMaskType& conflicting) const
	{
		getFireOCLAfterWeaponCooldownUpdateModuleData()->m_upgradeMuxData.getUpgradeActivationMasks(activation, conflicting);
	}

	virtual void performUpgradeFX()
	{
		getFireOCLAfterWeaponCooldownUpdateModuleData()->m_upgradeMuxData.performUpgradeFX(getObject());
	}

	virtual Bool requiresAllActivationUpgrades() const
	{
		return getFireOCLAfterWeaponCooldownUpdateModuleData()->m_upgradeMuxData.m_requiresAllTriggers;
	}

	virtual Bool isSubObjectsUpgrade() { return false; }

	void resetStats();
	void fireOCL();

private:

	Bool				m_valid;
	UnsignedInt m_consecutiveShots;
	UnsignedInt m_startFrame;

};

#endif // __FIRE_OCL_AFTER_WEAPON_COOLDOWN_UPDATE_H

