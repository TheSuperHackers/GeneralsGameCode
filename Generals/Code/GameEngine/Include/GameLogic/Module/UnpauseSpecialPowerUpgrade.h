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

// FILE: UnpauseSpecialPowerUpgrade.h ///////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, August 2002
// Desc:	 An upgrade that starts the timer on a Special Power module, so you can have them
// dependent on upgrades on the logic side, like NEED_UPGRADE does on the client side by disabling
// the button.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpgradeModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class SpecialPowerTemplate;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class UnpauseSpecialPowerUpgradeModuleData : public UpgradeModuleData
{

public:

	UnpauseSpecialPowerUpgradeModuleData( void );

	static void buildFieldParse(MultiIniFieldParse& p);

	const SpecialPowerTemplate *m_specialPower;

};

//-------------------------------------------------------------------------------------------------
/** The OCL upgrade module */
//-------------------------------------------------------------------------------------------------
class UnpauseSpecialPowerUpgrade : public UpgradeModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( UnpauseSpecialPowerUpgrade, "UnpauseSpecialPowerUpgrade" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( UnpauseSpecialPowerUpgrade, UnpauseSpecialPowerUpgradeModuleData );

public:

	UnpauseSpecialPowerUpgrade( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype defined by MemoryPoolObject

protected:

	virtual void upgradeImplementation( void ); ///< Here's the actual work of Upgrading
	virtual Bool isSubObjectsUpgrade() { return false; }

};
