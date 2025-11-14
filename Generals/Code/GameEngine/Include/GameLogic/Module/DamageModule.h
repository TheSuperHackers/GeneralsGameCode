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

// FILE: DamageModule.h /////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, September 2001
// Desc:
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common/Module.h"
#include "GameLogic/Damage.h"
#include "GameLogic/Module/BehaviorModule.h"

enum BodyDamageType CPP_11(: Int);

//-------------------------------------------------------------------------------------------------
/** OBJECT DAMAGE MODULE base class */
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class DamageModuleInterface
{

public:

	virtual void onDamage( DamageInfo *damageInfo ) = 0;	///< damage callback
	virtual void onHealing( DamageInfo *damageInfo ) = 0;	///< healing callback
	virtual void onBodyDamageStateChange( const DamageInfo* damageInfo,
																				BodyDamageType oldState,
																				BodyDamageType newState) = 0;  ///< state change callback

};

//-------------------------------------------------------------------------------------------------
class DamageModuleData : public BehaviorModuleData
{
public:
//	DamageTypeFlags m_damageTypes;

	DamageModuleData()
//		: m_damageTypes(DAMAGE_TYPE_FLAGS_ALL)
	{
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    BehaviorModuleData::buildFieldParse(p);

		static const FieldParse dataFieldParse[] =
		{
//			{ "DamageTypes", INI::parseDamageTypeFlags, NULL, offsetof( DamageModuleData, m_damageTypes ) },
			{ 0, 0, 0, 0 }
		};

    p.add(dataFieldParse);
	}
};

//-------------------------------------------------------------------------------------------------
class DamageModule : public BehaviorModule, public DamageModuleInterface
{

	MEMORY_POOL_GLUE_ABC( DamageModule )
	MAKE_STANDARD_MODULE_MACRO_ABC( DamageModule )
	MAKE_STANDARD_MODULE_DATA_MACRO_ABC( DamageModule, DamageModuleData )

public:

	DamageModule( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype defined by MemoryPoolObject

	// module methods
	static Int getInterfaceMask() { return MODULEINTERFACE_DAMAGE; }

	// BehaviorModule
	virtual DamageModuleInterface* getDamage() { return this; }

	// damage module callbacks
	virtual void onDamage( DamageInfo *damageInfo ) = 0;	///< damage callback
	virtual void onHealing( DamageInfo *damageInfo ) = 0;	///< healing callback
	virtual void onBodyDamageStateChange( const DamageInfo* damageInfo,
																				BodyDamageType oldState,
																				BodyDamageType newState) = 0;  ///< state change callback

protected:

};
inline DamageModule::DamageModule( Thing *thing, const ModuleData* moduleData ) : BehaviorModule( thing, moduleData ) { }
inline DamageModule::~DamageModule() { }
//-------------------------------------------------------------------------------------------------
