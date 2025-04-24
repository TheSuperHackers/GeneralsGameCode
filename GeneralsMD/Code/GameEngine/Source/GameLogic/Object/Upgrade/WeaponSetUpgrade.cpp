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

// FILE: WeaponSetUpgrade.cpp /////////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, March 2002
// Desc:	 UpgradeModule that sets a weapon set bit for the Best Fit weapon set chooser to discover
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Xfer.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/WeaponSetUpgrade.h"


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
WeaponSetUpgradeModuleData::WeaponSetUpgradeModuleData(void)
{
	m_weaponSetFlagSet = WEAPONSET_PLAYER_UPGRADE;
	m_weaponSetFlagClear = WEAPONSET_COUNT;  // = undefined;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void WeaponSetUpgradeModuleData::buildFieldParse(MultiIniFieldParse& p)
{

	UpgradeModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "WeaponSetFlag", INI::parseIndexList,	WeaponSetFlags::getBitNames(),offsetof(WeaponSetUpgradeModuleData, m_weaponSetFlagSet) },
		{ "WeaponSetFlagToClear", INI::parseIndexList, WeaponSetFlags::getBitNames(), offsetof(WeaponSetUpgradeModuleData, m_weaponSetFlagClear) },
		{ 0, 0, 0, 0 }
	};

	p.add(dataFieldParse);

}  // end buildFieldParse

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
WeaponSetUpgrade::WeaponSetUpgrade( Thing *thing, const ModuleData* moduleData ) : UpgradeModule( thing, moduleData )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
WeaponSetUpgrade::~WeaponSetUpgrade( void )
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void WeaponSetUpgrade::upgradeImplementation( )
{
	// Very simple; just need to flag the Object as having the player upgrade, and the WeaponSet chooser 
	// will do the work of picking the right one from ini.  This comment is as long as the code. Update: not anymore ;)
	const WeaponSetUpgradeModuleData* data = getWeaponSetUpgradeModuleData();

	Object *obj = getObject();
	obj->setWeaponSetFlag(data->m_weaponSetFlagSet);

	if (data->m_weaponSetFlagClear < WEAPONSET_COUNT) {
		obj->clearWeaponSetFlag(data->m_weaponSetFlagClear);
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void WeaponSetUpgrade::crc( Xfer *xfer )
{

	// extend base class
	UpgradeModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void WeaponSetUpgrade::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpgradeModule::xfer( xfer );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void WeaponSetUpgrade::loadPostProcess( void )
{

	// extend base class
	UpgradeModule::loadPostProcess();

}  // end loadPostProcess
