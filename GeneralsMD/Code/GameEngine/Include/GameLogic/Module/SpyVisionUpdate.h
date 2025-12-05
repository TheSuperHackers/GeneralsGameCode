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

// FILE: SpyVisionUpdate.h /////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, September 2002
// Desc:   Special Power will spy on the vision of all enemy players.
//				Putting a SpecialPower in a behavior takes a big huge amount of code duplication and
//				has no precedent.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpdateModule.h"
#include "GameLogic/Module/UpgradeModule.h"

class Player;

//-------------------------------------------------------------------------------------------------
class SpyVisionUpdateModuleData : public UpdateModuleData
{
public:
	UpgradeMuxData	m_upgradeMuxData;

	Bool						m_needsUpgrade;
	Bool						m_selfPowered;
	UnsignedInt			m_selfPoweredDuration;
	UnsignedInt			m_selfPoweredInterval;
	KindOfMaskType	m_spyOnKindof;

	SpyVisionUpdateModuleData()
	{
		m_needsUpgrade = FALSE;
		m_selfPowered = FALSE;
		m_selfPoweredDuration = 0;
		m_selfPoweredInterval = 0;
		m_spyOnKindof = KINDOFMASK_NONE;
		m_spyOnKindof.flip();
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
		static const FieldParse dataFieldParse[] =
		{
			{ "NeedsUpgrade",					INI::parseBool,									nullptr, offsetof( SpyVisionUpdateModuleData, m_needsUpgrade ) },
			{ "SelfPowered",					INI::parseBool,									nullptr, offsetof( SpyVisionUpdateModuleData, m_selfPowered ) },
			{ "SelfPoweredDuration",	INI::parseDurationUnsignedInt,	nullptr, offsetof( SpyVisionUpdateModuleData, m_selfPoweredDuration ) },
			{ "SelfPoweredInterval",	INI::parseDurationUnsignedInt,	nullptr, offsetof( SpyVisionUpdateModuleData, m_selfPoweredInterval ) },
			{ "SpyOnKindof",					KindOfMaskType::parseFromINI,		nullptr, offsetof( SpyVisionUpdateModuleData, m_spyOnKindof ) },
			{ 0, 0, 0, 0 }
		};

		UpdateModuleData::buildFieldParse(p);
		p.add(dataFieldParse);
		p.add(UpgradeMuxData::getFieldParse(), offsetof( SpyVisionUpdateModuleData, m_upgradeMuxData ));
	}
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class SpyVisionUpdate : public UpdateModule, public UpgradeMux
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( SpyVisionUpdate, "SpyVisionUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( SpyVisionUpdate, SpyVisionUpdateModuleData )

public:

	SpyVisionUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual SpyVisionUpdate* getSpyVisionUpdate() { return this; }

	// module methods
	static Int getInterfaceMask() { return UpdateModule::getInterfaceMask() | MODULEINTERFACE_UPGRADE; }
	virtual void onDelete( void );
	virtual void onCapture( Player *oldOwner, Player *newOwner );
	virtual void onDisabledEdge( Bool nowDisabled );

	// BehaviorModule
	virtual UpgradeModuleInterface* getUpgrade() { return this; }

	//Update module
	virtual UpdateSleepTime update( void );

	void activateSpyVision( UnsignedInt duration );

	void setDisabledUntilFrame( UnsignedInt frame );
	UnsignedInt getDisabledUntilFrame() const { return m_disabledUntilFrame; }

protected:

	// UpgradeMux functions.  Mux standing, of course, for Majorly Ugly Xhitcode
	virtual void upgradeImplementation();
	virtual void getUpgradeActivationMasks(UpgradeMaskType& activation, UpgradeMaskType& conflicting) const
	{
		getSpyVisionUpdateModuleData()->m_upgradeMuxData.getUpgradeActivationMasks(activation, conflicting);
	}
	virtual void performUpgradeFX()
	{
		getSpyVisionUpdateModuleData()->m_upgradeMuxData.performUpgradeFX(getObject());
	}
	virtual void processUpgradeRemoval()
	{
		// I can't take it any more.  Let the record show that I think the UpgradeMux multiple inheritence is CRAP.
		getSpyVisionUpdateModuleData()->m_upgradeMuxData.muxDataProcessUpgradeRemoval(getObject());
	}

	virtual Bool requiresAllActivationUpgrades() const
	{
		return getSpyVisionUpdateModuleData()->m_upgradeMuxData.m_requiresAllTriggers;
	}
	Bool isUpgradeActive() const { return isAlreadyUpgraded(); }
	virtual Bool isSubObjectsUpgrade() { return false; }

private:

	void doActivationWork( Player *playerToSetFor, Bool setting );

	UnsignedInt m_deactivateFrame;
	UnsignedInt m_disabledUntilFrame; //sabotaged, emp'd, etc.
	Bool m_currentlyActive;
	Bool m_resetTimersNextUpdate;
};
