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

// FILE: WeaponSetUpgrade.h /////////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, March 2002
// Desc:	 UpgradeModule that sets a weapon set bit for the Best Fit weapon set chooser to discover
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpgradeModule.h"
#include "GameLogic/WeaponSet.h"


// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;
//enum ModelConditionFlagType CPP_11(: Int);
enum WeaponSetType CPP_11(: Int);


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class WeaponSetUpgradeModuleData : public UpgradeModuleData
{

public:

	WeaponSetUpgradeModuleData(void);

	static void buildFieldParse(MultiIniFieldParse& p);

	WeaponSetType m_weaponSetFlag;  ///< The weaponset flag to set (default = WEAPONSET_PLAYER_UPGRADE)
	WeaponSetFlags m_weaponSetFlagsToClear;  ///< The weaponset flags to clear. This is needed if we want to disable a previous upgrade.
	Bool m_needsParkedAircraft;   ///< Aircraft attempting this upgrade needs to be stationary in hangar

};
//-------------------------------------------------------------------------------------------------
class WeaponSetUpgrade : public UpgradeModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( WeaponSetUpgrade, "WeaponSetUpgrade" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA(WeaponSetUpgrade, WeaponSetUpgradeModuleData);

public:

	virtual Bool wouldUpgrade(UpgradeMaskType keyMask) const;

	WeaponSetUpgrade( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype defined by MemoryPoolObject

protected:
	virtual void upgradeImplementation( ); ///< Here's the actual work of Upgrading
	virtual Bool isSubObjectsUpgrade() { return false; }

};
