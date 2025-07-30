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

// FILE: DelayedUpgradeBehavior.h /////////////////////////////////////////////////////////////////////////
// Author: Andi W, July 2025
// Desc:   Update that will trigger an upgrade after some time
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __DelayedUpgradeBehavior_H_
#define __DelayedUpgradeBehavior_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////

#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/UpdateModule.h"
#include "GameLogic/Module/UpgradeModule.h"


//-------------------------------------------------------------------------------------------------
class DelayedUpgradeBehaviorModuleData : public UpdateModuleData
{
public:
	UpgradeMuxData				m_upgradeMuxData;

	Bool						m_initiallyActive;

	// AsciiString				    m_upgradeToTrigger;
	std::vector<AsciiString>	m_upgradesToTrigger;
	std::vector<AsciiString>	m_upgradesToRemove;

	UnsignedInt                 m_triggerDelay;
	//UnsignedInt                 m_triggerNumShots;

	DelayedUpgradeBehaviorModuleData()
	{
		m_upgradesToTrigger.clear();
		m_upgradesToRemove.clear();
		m_initiallyActive = false;
		m_triggerDelay = 0;
		//m_triggerNumShots = 0;
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
		static const FieldParse dataFieldParse[] =
		{
			{ "StartsActive",	INI::parseBool, NULL, offsetof(DelayedUpgradeBehaviorModuleData, m_initiallyActive) },
			{ "UpgradesToTrigger", INI::parseAsciiStringVector,	NULL, offsetof(DelayedUpgradeBehaviorModuleData, m_upgradesToTrigger) },
			{ "UpgradesToRemove", INI::parseAsciiStringVector,	NULL, offsetof(DelayedUpgradeBehaviorModuleData, m_upgradesToRemove) },
			{ "TriggerAfterTime",	INI::parseDurationUnsignedInt, NULL, offsetof(DelayedUpgradeBehaviorModuleData, m_triggerDelay) },
			//{ "TriggerAfterShotsFired",	INI::parseUnsignedInt, NULL, offsetof(DelayedUpgradeBehaviorModuleData, m_triggerNumShots) },
			{ 0, 0, 0, 0 }
		};

		UpdateModuleData::buildFieldParse(p);
		p.add(dataFieldParse);
		p.add(UpgradeMuxData::getFieldParse(), offsetof(DelayedUpgradeBehaviorModuleData, m_upgradeMuxData));
	}
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class DelayedUpgradeBehavior : public UpdateModule, public UpgradeMux
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(DelayedUpgradeBehavior, "DelayedUpgradeBehavior")
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA(DelayedUpgradeBehavior, DelayedUpgradeBehaviorModuleData)

private:
	UnsignedInt m_triggerFrame;
	// UnsignedInt m_shotsLeft;
	// TODO: Which weaponslot
	Bool m_triggerCompleted;

public:

	DelayedUpgradeBehavior(Thing* thing, const ModuleData* moduleData);
	// virtual destructor prototype provided by memory pool declaration

	// module methods
	static Int getInterfaceMask() { return UpdateModule::getInterfaceMask() | MODULEINTERFACE_UPGRADE; }

	// BehaviorModule
	virtual UpgradeModuleInterface* getUpgrade() { return this; }

	// UpdateModule
	virtual UpdateSleepTime update();

	// This should be active while disabled
	virtual DisabledMaskType getDisabledTypesToProcess() const { return DISABLEDMASK_ALL; }

protected:

	void triggerUpgrade();

	virtual Bool resetUpgrade(UpgradeMaskType keyMask);  // When this upgrade is removed, we reset our triggers.

	virtual void upgradeImplementation();

	virtual void getUpgradeActivationMasks(UpgradeMaskType& activation, UpgradeMaskType& conflicting) const
	{
		getDelayedUpgradeBehaviorModuleData()->m_upgradeMuxData.getUpgradeActivationMasks(activation, conflicting);
	}

	virtual void performUpgradeFX()
	{
		getDelayedUpgradeBehaviorModuleData()->m_upgradeMuxData.performUpgradeFX(getObject());
	}

	virtual void processUpgradeRemoval()
	{
		// I can't take it any more.  Let the record show that I think the UpgradeMux multiple inheritence is CRAP.
		getDelayedUpgradeBehaviorModuleData()->m_upgradeMuxData.muxDataProcessUpgradeRemoval(getObject());
	}

	virtual Bool requiresAllActivationUpgrades() const
	{
		return getDelayedUpgradeBehaviorModuleData()->m_upgradeMuxData.m_requiresAllTriggers;
	}

	inline Bool isUpgradeActive() const { return isAlreadyUpgraded(); }

	virtual Bool isSubObjectsUpgrade() { return false; }

};

#endif // __DelayedUpgradeBehavior_H_

