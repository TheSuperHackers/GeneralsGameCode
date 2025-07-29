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

// FILE: UpgradeSpecialPower.cpp /////////////////////////////////////////////////////////////////
// Author: Andreas W, July 25
// Desc:   Special Power will grant an upgrade to the object
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/Xfer.h"
#include "Common/Player.h"
#include "Common/Upgrade.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/UpgradeSpecialPower.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UpgradeSpecialPowerModuleData::UpgradeSpecialPowerModuleData(void)
{
	m_upgradeName = "";
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void UpgradeSpecialPowerModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	SpecialPowerModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "UpgradeToGrant", INI::parseAsciiString,	NULL,   offsetof(UpgradeSpecialPowerModuleData, m_upgradeName) },
		{ 0, 0, 0, 0 }
	};
	p.add(dataFieldParse);

}  // end buildFieldParse

///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UpgradeSpecialPower::UpgradeSpecialPower(Thing* thing, const ModuleData* moduleData)
	: SpecialPowerModule(thing, moduleData)
{

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UpgradeSpecialPower::~UpgradeSpecialPower(void)
{

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void UpgradeSpecialPower::grantUpgrade(Object* object) {

	// get module data
	const UpgradeSpecialPowerModuleData* modData = getUpgradeSpecialPowerModuleData();

	const UpgradeTemplate* upgradeTemplate = TheUpgradeCenter->findUpgrade(modData->m_upgradeName);
	if (!upgradeTemplate)
	{
		DEBUG_ASSERTCRASH(0, ("UpgradeSpecialPower for %s can't find upgrade template %s.", getObject()->getName(), modData->m_upgradeName));
		return;
	}

	Player* player = object->getControllingPlayer();
	if (upgradeTemplate->getUpgradeType() == UPGRADE_TYPE_PLAYER)
	{
		// get the player
		player->addUpgrade(upgradeTemplate, UPGRADE_STATUS_COMPLETE);
	}
	else
	{
		object->giveUpgrade(upgradeTemplate);
	}

	player->getAcademyStats()->recordUpgrade(upgradeTemplate, TRUE);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void UpgradeSpecialPower::doSpecialPower(UnsignedInt commandOptions)
{
	if (getObject()->isDisabled())
		return;

	// call the base class action cause we are *EXTENDING* functionality
	SpecialPowerModule::doSpecialPower(commandOptions);

	// Grant the upgrade
	grantUpgrade(getObject());
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void UpgradeSpecialPower::doSpecialPowerAtObject(Object* obj, UnsignedInt commandOptions)
{
	if (getObject()->isDisabled())
		return;

	// call the base class action cause we are *EXTENDING* functionality
	SpecialPowerModule::doSpecialPowerAtObject(obj, commandOptions);

	// Grant the upgrade
	grantUpgrade(obj);
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void UpgradeSpecialPower::crc(Xfer* xfer)
{

	// extend base class
	SpecialPowerModule::crc(xfer);

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
	// ------------------------------------------------------------------------------------------------
void UpgradeSpecialPower::xfer(Xfer* xfer)
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion(&version, currentVersion);

	// extend base class
	SpecialPowerModule::xfer(xfer);

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void UpgradeSpecialPower::loadPostProcess(void)
{

	// extend base class
	SpecialPowerModule::loadPostProcess();

}  // end loadPostProcess
