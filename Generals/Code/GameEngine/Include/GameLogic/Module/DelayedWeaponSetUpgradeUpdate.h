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

// FILE: DelayedWeaponSetUpgradeUpdate.h ////////////////////////////////////////////////////////////////////////
// Author: Will act like an WeaponSet UpgradeModule, but after a delay.
// Desc:   Graham Smallwood, April 2002
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __DELAYED_WEAPON_SET_UPGRADE_UPDATE_H_
#define __DELAYED_WEAPON_SET_UPGRADE_UPDATE_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpdateModule.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class DelayedWeaponSetUpgradeUpdateModuleData: public UpdateModuleData
{

public:

	DelayedWeaponSetUpgradeUpdateModuleData( void );

	static void buildFieldParse(MultiIniFieldParse& p);

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class DelayedWeaponSetUpgradeUpdate : public UpdateModule, public DelayedUpgradeUpdateInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( DelayedWeaponSetUpgradeUpdate, "DelayedWeaponSetUpgradeUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( DelayedWeaponSetUpgradeUpdate, DelayedWeaponSetUpgradeUpdateModuleData )

public:

	DelayedWeaponSetUpgradeUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual Bool isTriggeredBy( UpgradeMaskType potentialMask );	///< If you were an upgrade, would you trigger for this?
	virtual void setDelay( UnsignedInt startingDelay );	///< Start the upgrade doing countdown

	virtual UpdateSleepTime update();

protected:

};

#endif
