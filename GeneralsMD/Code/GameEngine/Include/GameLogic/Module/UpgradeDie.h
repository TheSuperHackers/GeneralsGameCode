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

// FILE: UpgradeDie.h /////////////////////////////////////////////////////////////////////////////
// Author: Kris Morness, August 2002
// Desc:   Free's producer's upgrade (assuming this object was created via an upgrade).
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __UPGRADEDIE_H
#define __UPGRADEDIE_H

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/INI.h"
#include "GameLogic/Module/DieModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;

//-------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class UpgradeDieModuleData : public DieModuleData
{
public:
	AsciiString m_upgradeName;

	UpgradeDieModuleData(){}

	static void buildFieldParse(MultiIniFieldParse& p)
	{

		DieModuleData::buildFieldParse(p);

		static const FieldParse dataFieldParse[] =
		{
			{ "UpgradeToRemove",					INI::parseAsciiString,		NULL, offsetof( UpgradeDieModuleData, m_upgradeName ) },
			{ 0, 0, 0, 0 }
		};

    p.add(dataFieldParse);
	}
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class UpgradeDie : public DieModule
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( UpgradeDie, "UpgradeDie" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( UpgradeDie, UpgradeDieModuleData )

public:

	UpgradeDie( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual void onDie( const DamageInfo *damageInfo );

};

#endif // __UPGRADEDIE_H

