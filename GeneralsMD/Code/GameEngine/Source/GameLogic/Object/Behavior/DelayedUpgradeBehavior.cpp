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

// FILE: DelayedUpgradeBehavior.cpp ///////////////////////////////////////////////////////////////////////
// Author:
// Desc:  
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine


#include "Common/INI.h"
#include "Common/Xfer.h"
#include "Common/Player.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/DelayedUpgradeBehavior.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Object.h"
#include "GameLogic/Weapon.h"


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
DelayedUpgradeBehavior::DelayedUpgradeBehavior(Thing* thing, const ModuleData* moduleData) : UpdateModule(thing, moduleData)
{
	m_triggerCompleted = FALSE;
	m_triggerFrame = 0;
	//m_shotsLeft = 0;

	if (getDelayedUpgradeBehaviorModuleData()->m_initiallyActive)
	{
		giveSelfUpgrade();
	}
	else {
		setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
DelayedUpgradeBehavior::~DelayedUpgradeBehavior(void)
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void DelayedUpgradeBehavior::upgradeImplementation(void)
{
	// DEBUG_LOG(("DelayedUpgradeBehavior::upgradeImplementation() 1\n"));

	const DelayedUpgradeBehaviorModuleData* d = getDelayedUpgradeBehaviorModuleData();

	UnsignedInt delay = d->m_triggerDelay;
	// Trigger after time:
	if (delay > 0) {
		m_triggerFrame = TheGameLogic->getFrame() + delay;
	}

	//if (d->m_triggerNumShots > 0) {
	//	m_shotsLeft = d->m_triggerNumShots;
	//	setWakeFrame(getObject(), UPDATE_SLEEP_NONE);
	//	return;
	//}

	if (delay > 0) {

		DEBUG_LOG(("DelayedUpgradeBehavior::upgradeImplementation(): trigger_frame = %d", m_triggerFrame));

		setWakeFrame(getObject(), UPDATE_SLEEP(d->m_triggerDelay));
		return;
	}

	// DEBUG_LOG(("DelayedUpgradeBehavior::upgradeImplementation(): We have no trigger!!!"));

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime DelayedUpgradeBehavior::update(void)
{
	if (m_triggerCompleted) {
		DEBUG_LOG(("DelayedUpgradeBehavior::Update(): Already triggered. We should not be awake!!!"));
		return UPDATE_SLEEP_FOREVER;
	}

	if (!isUpgradeActive()) {
		DEBUG_LOG(("DelayedUpgradeBehavior::Update(): Upgrade not applied. We should not be awake!!!"));
		return UPDATE_SLEEP_FOREVER;
	}

	const DelayedUpgradeBehaviorModuleData* d = getDelayedUpgradeBehaviorModuleData();

	if (d->m_triggerDelay > 0) {
		UnsignedInt now = TheGameLogic->getFrame();
		if (now >= m_triggerFrame) {
			// DEBUG_LOG(("DelayedUpgradeBehavior::update(): Trigger Frame reached."));
			triggerUpgrade();
			return UPDATE_SLEEP_FOREVER;
		}
	}

	//if (d->m_triggerNumShots > 0) {

	//	//checkShots();
	//	if (m_shotsLeft >= 0) {
	//		triggerUpgrade();
	//		return UPDATE_SLEEP_FOREVER;
	//	}
	//}

	return UPDATE_SLEEP_NONE;
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void DelayedUpgradeBehavior::triggerUpgrade(void)
{

	const DelayedUpgradeBehaviorModuleData* d = getDelayedUpgradeBehaviorModuleData();

	m_triggerCompleted = TRUE;

	Player* player = getObject()->getControllingPlayer();

	if (!d->m_upgradesToTrigger.empty()) {

		std::vector<AsciiString>::const_iterator it;
		for (it = d->m_upgradesToTrigger.begin(); it != d->m_upgradesToTrigger.end(); it++)
		{
			const UpgradeTemplate* upgradeTemplate = TheUpgradeCenter->findUpgrade(*it);

			if (!upgradeTemplate)
			{
				DEBUG_CRASH(("DelayedUpgradeBehavior for %s can't find upgrade template %s.", getObject()->getName(), it->str()));
				throw INI_INVALID_DATA;
			}

			if (upgradeTemplate->getUpgradeType() == UPGRADE_TYPE_PLAYER)
			{
				player->addUpgrade(upgradeTemplate, UPGRADE_STATUS_COMPLETE);
			}
			else
			{
				getObject()->giveUpgrade(upgradeTemplate);
			}

			player->getAcademyStats()->recordUpgrade(upgradeTemplate, TRUE);
		}
	}


	if (!d->m_upgradesToRemove.empty()) {

		std::vector<AsciiString>::const_iterator it;
		for (it = d->m_upgradesToRemove.begin(); it != d->m_upgradesToRemove.end(); it++)
		{
			const UpgradeTemplate* upgradeTemplate = TheUpgradeCenter->findUpgrade(*it);
			if (!upgradeTemplate)
			{
				DEBUG_CRASH(("DelayedUpgradeBehavior for '%s' cannot find upgrade to remove '%s'", getObject()->getName(), it->str()));
				throw INI_INVALID_DATA;
			}

			if (upgradeTemplate->getUpgradeType() == UPGRADE_TYPE_PLAYER)
			{
				player->removeUpgrade(upgradeTemplate);
			}
			else
			{
				getObject()->removeUpgrade(upgradeTemplate);
			}
		}
	}

	// DEBUG_LOG(("DelayedUpgradeBehavior::triggerUpgrade() Done."));


}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool DelayedUpgradeBehavior::resetUpgrade(UpgradeMaskType keyMask)
{
	// DEBUG_LOG(("DelayedUpgradeBehavior::resetUpgrade()."));
	if (UpgradeMux::resetUpgrade(keyMask)) {
		m_triggerCompleted = FALSE;
		m_triggerFrame = 0;
		// m_shotsLeft = 0;
		return TRUE;
	}
	else {
		return FALSE;
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void DelayedUpgradeBehavior::crc(Xfer* xfer)
{

	// extend base class
	BehaviorModule::crc(xfer);

	// extend upgrade mux
	UpgradeMux::upgradeMuxCRC(xfer);

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
	// ------------------------------------------------------------------------------------------------
void DelayedUpgradeBehavior::xfer(Xfer* xfer)
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion(&version, currentVersion);

	// extend base class
	BehaviorModule::xfer(xfer);

	// extend upgrade mux
	UpgradeMux::upgradeMuxXfer(xfer);

	// trigger frame
	xfer->xferUnsignedInt(&m_triggerFrame);

	// trigger completed
	xfer->xferBool(&m_triggerCompleted);

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void DelayedUpgradeBehavior::loadPostProcess(void)
{

	// extend base class
	BehaviorModule::loadPostProcess();

	// extend upgrade mux
	UpgradeMux::upgradeMuxLoadPostProcess();

}  // end loadPostProcess
