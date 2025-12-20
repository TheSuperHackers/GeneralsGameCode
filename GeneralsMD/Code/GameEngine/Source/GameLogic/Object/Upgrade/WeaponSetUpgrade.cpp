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
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Xfer.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/WeaponSetUpgrade.h"
#include "GameLogic/Module/JetAIUpdate.h"


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
WeaponSetUpgradeModuleData::WeaponSetUpgradeModuleData(void)
{
	m_weaponSetFlag = WEAPONSET_PLAYER_UPGRADE;
	// m_weaponSetFlagsToClear = WEAPONSET_COUNT;  // = undefined;
	m_needsParkedAircraft = FALSE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void WeaponSetUpgradeModuleData::buildFieldParse(MultiIniFieldParse& p)
{

	UpgradeModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "WeaponSetFlag", INI::parseIndexListOrNone, WeaponSetFlags::getBitNames(),offsetof(WeaponSetUpgradeModuleData, m_weaponSetFlag) },
		{ "WeaponSetFlagsToClear", WeaponSetFlags::parseFromINI, NULL, offsetof(WeaponSetUpgradeModuleData, m_weaponSetFlagsToClear) },
		{ "NeedsParkedAircraft", INI::parseBool, NULL, offsetof(WeaponSetUpgradeModuleData, m_needsParkedAircraft) },
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
Bool WeaponSetUpgrade::wouldUpgrade(UpgradeMaskType keyMask) const
{
	if (UpgradeMux::wouldUpgrade(keyMask)) {

		// Check additional conditions
		const WeaponSetUpgradeModuleData* data = getWeaponSetUpgradeModuleData();

		if (data->m_needsParkedAircraft) {
			const AIUpdateInterface* ai = getObject()->getAI();
			if (ai) {
				const JetAIUpdate* jetAI = ai->getJetAIUpdate();
				if ((jetAI) && jetAI->isParkedInHangar()){
					return TRUE;
				}
			}
		}
		else {
			return TRUE;
		}
	}

	//We can't upgrade!
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void WeaponSetUpgrade::upgradeImplementation( )
{
	// Very simple; just need to flag the Object as having the player upgrade, and the WeaponSet chooser
	// will do the work of picking the right one from ini.  This comment is as long as the code. Update: not anymore ;)
	const WeaponSetUpgradeModuleData* data = getWeaponSetUpgradeModuleData();

	Object *obj = getObject();
	if (data->m_weaponSetFlag > WEAPONSET_NONE) {
		obj->setWeaponSetFlag(data->m_weaponSetFlag);
	}

	/*DEBUG_LOG((">>> WSU: m_weaponSetFlagsToClear = %d\n",
		data->m_weaponSetFlag));*/

	if (data->m_weaponSetFlagsToClear.any()) {
		// We loop over each weaponset type and see if we have it set.
		// Andi: Not sure if this is cleaner solution than storing an array of flags.
		for (int i = 0; i < WEAPONSET_COUNT; i++) {
			WeaponSetType type = (WeaponSetType)i;
			if (data->m_weaponSetFlagsToClear.test(type)) {
				obj->clearWeaponSetFlag(type);
			}
		}
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void WeaponSetUpgrade::crc( Xfer *xfer )
{

	// extend base class
	UpgradeModule::crc( xfer );

}

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

}

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void WeaponSetUpgrade::loadPostProcess( void )
{

	// extend base class
	UpgradeModule::loadPostProcess();

}
