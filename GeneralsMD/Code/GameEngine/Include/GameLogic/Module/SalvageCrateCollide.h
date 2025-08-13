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

// FILE: SalvageCrateCollide.h /////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, March 2002
// Desc:   The Savlage system can give a Weaponset bonus, a level, or money.  Salvagers create them
//					by killing marked units, and only WeaponSalvagers can get the WeaponSet bonus
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef SALVAGE_CRATE_COLLIDE_H_
#define SALVAGE_CRATE_COLLIDE_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/Module.h"
#include "Common/STLTypedefs.h"
#include "GameLogic/Module/CrateCollide.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;

//-------------------------------------------------------------------------------------------------
class SalvageCrateCollideModuleData : public CrateCollideModuleData
{
public:
	Real m_weaponChance;	///< Chance to get a weapon upgrade, if possible
	Real m_levelChance;		///< Chance to get a level, if weaponChance fails
	Real m_moneyChance;		///< Chance to get money, if weaponChance fails
	Int m_minimumMoney;		///< How much, if we get money
	Int m_maximumMoney;		///< How much, if we get money

	SalvageCrateCollideModuleData()
	{
		m_weaponChance = 1.0f;
		m_levelChance = .25f;
		m_moneyChance = .75f;
		m_minimumMoney = 25;
		m_maximumMoney = 75;
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    CrateCollideModuleData::buildFieldParse(p);

		static const FieldParse dataFieldParse[] =
		{
			{ "WeaponChance",	INI::parsePercentToReal,	NULL, offsetof( SalvageCrateCollideModuleData, m_weaponChance ) },
			{ "LevelChance",	INI::parsePercentToReal,	NULL, offsetof( SalvageCrateCollideModuleData, m_levelChance ) },
			{ "MoneyChance",	INI::parsePercentToReal,	NULL, offsetof( SalvageCrateCollideModuleData, m_moneyChance ) },
			{ "MinMoney",			INI::parseInt,						NULL, offsetof( SalvageCrateCollideModuleData, m_minimumMoney ) },
			{ "MaxMoney",			INI::parseInt,						NULL, offsetof( SalvageCrateCollideModuleData, m_maximumMoney ) },
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);

	}
};

//-------------------------------------------------------------------------------------------------
class SalvageCrateCollide : public CrateCollide
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( SalvageCrateCollide, "SalvageCrateCollide" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( SalvageCrateCollide, SalvageCrateCollideModuleData );

public:

	SalvageCrateCollide( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual Bool isSalvageCrateCollide() const { return true; }

protected:

	/// This allows specific vetoes to certain types of crates and their data
	virtual Bool isValidToExecute( const Object *other ) const;

	/// This is the game logic execution function that all real CrateCollides will implement
	virtual Bool executeCrateBehavior( Object *other );

private:
	Bool eligibleForWeaponSet( Object *other );
	Bool eligibleForArmorSet( Object *other );
	Bool eligibleForLevel( Object *other );
	Bool testWeaponChance();
	Bool testLevelChance();

	void doWeaponSet( Object *other );
	void doArmorSet( Object *other );
	void doLevelGain( Object *other );
	void doMoney( Object *other );

};

#endif
