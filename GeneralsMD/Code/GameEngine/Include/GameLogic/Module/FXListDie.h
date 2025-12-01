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

// FILE: FXListDie.h /////////////////////////////////////////////////////////////////////////////
// Author: Steven Johnson, Jan 2002
// Desc:   Simple die module
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/INI.h"
#include "GameLogic/Module/DieModule.h"
#include "GameLogic/Module/UpgradeModule.h"
#include "GameLogic/Weapon.h"
#include "GameLogic/Damage.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;
class FXList;

//-------------------------------------------------------------------------------------------------
class FXListDieModuleData : public DieModuleData
{
public:
	const FXList					*m_defaultDeathFX;								///< default fx to make
	UpgradeMuxData				m_upgradeMuxData;
	Bool									m_orientToObject;
	Bool									m_initiallyActive;

	FXListDieModuleData()
	{
		m_defaultDeathFX = NULL;
		m_orientToObject = TRUE;
		m_initiallyActive = TRUE; //Patch 1.02 -- Craptacular HACK -- should default to FALSE but only ONE case sets it false out of 847!
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    DieModuleData::buildFieldParse(p);

		static const FieldParse dataFieldParse[] =
		{
			{ "StartsActive",					INI::parseBool, NULL, offsetof( FXListDieModuleData, m_initiallyActive ) },
			{ "DeathFX",							INI::parseFXList,		NULL, offsetof( FXListDieModuleData, m_defaultDeathFX ) },
			{ "OrientToObject",				INI::parseBool,		NULL, offsetof( FXListDieModuleData, m_orientToObject ) },
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);
		p.add(UpgradeMuxData::getFieldParse(), offsetof( FXListDieModuleData, m_upgradeMuxData ));
	}
};

//-------------------------------------------------------------------------------------------------
class FXListDie : public DieModule, public UpgradeMux
{

	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( FXListDie, FXListDieModuleData );
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( FXListDie, "FXListDie" )

public:

	FXListDie( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	// module methods
	static Int getInterfaceMask() { return BehaviorModule::getInterfaceMask() | (MODULEINTERFACE_UPGRADE) | (MODULEINTERFACE_DIE); }

	// BehaviorModule
	virtual UpgradeModuleInterface* getUpgrade() { return this; }
	virtual DieModuleInterface* getDie() { return this; }

	virtual void onDie( const DamageInfo *damageInfo );

protected:

	virtual void upgradeImplementation()
	{
		// nothing!
	}

	virtual void getUpgradeActivationMasks(UpgradeMaskType& activation, UpgradeMaskType& conflicting) const
	{
		getFXListDieModuleData()->m_upgradeMuxData.getUpgradeActivationMasks(activation, conflicting);
	}

	virtual void performUpgradeFX()
	{
		getFXListDieModuleData()->m_upgradeMuxData.performUpgradeFX(getObject());
	}

	virtual void processUpgradeRemoval()
	{
		// I can't take it any more.  Let the record show that I think the UpgradeMux multiple inheritence is CRAP.
		getFXListDieModuleData()->m_upgradeMuxData.muxDataProcessUpgradeRemoval(getObject());
	}

	virtual Bool requiresAllActivationUpgrades() const
	{
		return getFXListDieModuleData()->m_upgradeMuxData.m_requiresAllTriggers;
	}

	Bool isUpgradeActive() const { return isAlreadyUpgraded(); }

	virtual Bool isSubObjectsUpgrade() { return false; }


};
