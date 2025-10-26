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

// FILE: FXListDie.h /////////////////////////////////////////////////////////////////////////////
// Author: Steven Johnson, Jan 2002
// Desc:   Simple die module
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/INI.h"
#include "GameLogic/Module/DieModule.h"
#include "GameLogic/Weapon.h"
#include "GameLogic/Damage.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;
class FXList;

//-------------------------------------------------------------------------------------------------
class FXListDieModuleData : public DieModuleData
{
public:
	const FXList *m_defaultDeathFX;								///< default fx to make
	Bool m_orientToObject;

	FXListDieModuleData()
	{
		m_defaultDeathFX = NULL;
		m_orientToObject = true;
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    DieModuleData::buildFieldParse(p);

		static const FieldParse dataFieldParse[] =
		{
			{ "DeathFX",							INI::parseFXList,		NULL, offsetof( FXListDieModuleData, m_defaultDeathFX ) },
			{ "OrientToObject",				INI::parseBool,		NULL, offsetof( FXListDieModuleData, m_orientToObject ) },
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);
	}
};

//-------------------------------------------------------------------------------------------------
class FXListDie : public DieModule
{

	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( FXListDie, FXListDieModuleData );
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( FXListDie, "FXListDie" )

public:

	FXListDie( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual void onDie( const DamageInfo *damageInfo );

};
