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

// FILE: LocomotorSetUpgrade.h /////////////////////////////////////////////////////////////////////////////
// Author: Steven Johnson, Aug 2002
// Desc:
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpgradeModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;
enum LocomotorSetType CPP_11(: Int);

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class LocomotorSetUpgradeModuleData : public UpgradeModuleData
{
public:

	LocomotorSetUpgradeModuleData(void);

	static void buildFieldParse(MultiIniFieldParse& p);
	static void parseLocomotorType(INI* ini, void* instance, void* store, const void* /*userData*/);

	Bool m_setUpgraded;   ///< Enable or Disable upgraded locomotor
	Bool m_useLocomotorType;  ///< Use explicit locomotor type
	LocomotorSetType m_LocomotorType;  ///< explicit lomotor type
	//Bool m_needsParkedAircraft;   ///< Aircraft attempting this upgrade needs to be stationary in hangar
};
//-------------------------------------------------------------------------------------------------
class LocomotorSetUpgrade : public UpgradeModule
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(LocomotorSetUpgrade, "LocomotorSetUpgrade")
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( LocomotorSetUpgrade, LocomotorSetUpgradeModuleData);

public:

	LocomotorSetUpgrade( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype defined by MemoryPoolObject

protected:
	virtual void upgradeImplementation( ); ///< Here's the actual work of Upgrading
	virtual Bool isSubObjectsUpgrade() { return false; }

};
