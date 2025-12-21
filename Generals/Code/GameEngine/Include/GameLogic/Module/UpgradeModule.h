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

// FILE: UpgradeModule.h /////////////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, March 2002
// Desc:	 A Module that responds to PlayerUpgrade Bitsetting by executing code once and only once
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common/Module.h"
#include "Common/STLTypedefs.h"
#include "Common/Upgrade.h"

#include "GameClient/Drawable.h"
#include "GameClient/FXList.h"

#include "GameLogic/Module/BehaviorModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Player;

//-------------------------------------------------------------------------------------------------
/** OBJECT DIE MODULE base class */
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class UpgradeModuleInterface
{
public:

 	virtual Bool isAlreadyUpgraded() const = 0;
	virtual Bool attemptUpgrade( UpgradeMaskType keyMask ) = 0;
	virtual Bool wouldUpgrade( UpgradeMaskType keyMask ) const = 0;
	virtual Bool resetUpgrade( UpgradeMaskType keyMask ) = 0;
	virtual Bool isSubObjectsUpgrade() = 0;
	virtual void forceRefreshUpgrade() = 0;
	virtual Bool testUpgradeConditions( UpgradeMaskType keyMask ) const = 0;

};

//-------------------------------------------------------------------------------------------------
class UpgradeMuxData	// does NOT inherit from ModuleData.
{
public:
	mutable std::vector<AsciiString>	m_activationUpgradeNames;
	mutable std::vector<AsciiString>	m_conflictingUpgradeNames;
	mutable const FXList*							m_fxListUpgrade;
	mutable UpgradeMaskType						m_activationMask;				///< Activation only supports a single name currently
	mutable UpgradeMaskType						m_conflictingMask;			///< Conflicts support multiple listings, and they are an OR
	mutable Bool											m_requiresAllTriggers;

	UpgradeMuxData()
	{
		m_fxListUpgrade = nullptr;
		m_activationMask.clear();
		m_conflictingMask.clear();
		m_requiresAllTriggers = false;
	}

	static const FieldParse* getFieldParse()
	{
		static const FieldParse dataFieldParse[] =
		{
			{ "TriggeredBy",		INI::parseAsciiStringVector, nullptr, offsetof( UpgradeMuxData, m_activationUpgradeNames ) },
			{ "ConflictsWith",	INI::parseAsciiStringVector, nullptr, offsetof( UpgradeMuxData, m_conflictingUpgradeNames ) },
			{ "FXListUpgrade",	INI::parseFXList, nullptr, offsetof( UpgradeMuxData, m_fxListUpgrade ) },
			{ "RequiresAllTriggers", INI::parseBool, nullptr, offsetof( UpgradeMuxData, m_requiresAllTriggers ) },
			{ 0, 0, 0, 0 }
		};
		return dataFieldParse;
	}
	Bool requiresAllActivationUpgrades() const;
	void getUpgradeActivationMasks(UpgradeMaskType& activation, UpgradeMaskType& conflicting) const;	///< The first time someone looks at my mask, I'll figure it out.
	void performUpgradeFX(Object* obj) const;
};

//-------------------------------------------------------------------------------------------------
// implements some handy mix-in guts, but doesn't descend from ObjectModule... useful for Behaviors
// that want to implement Upgrades
class UpgradeMux : public UpgradeModuleInterface
{
public:

	UpgradeMux();

	virtual Bool isAlreadyUpgraded() const ;
	// ***DANGER! DANGER! Don't use this, unless you are forcing an already made upgrade to refresh!!
	virtual void forceRefreshUpgrade();
	virtual Bool attemptUpgrade( UpgradeMaskType keyMask );
	virtual Bool wouldUpgrade( UpgradeMaskType keyMask ) const;
	virtual Bool resetUpgrade( UpgradeMaskType keyMask );
	virtual Bool testUpgradeConditions( UpgradeMaskType keyMask ) const;

protected:

	void setUpgradeExecuted(Bool e) { m_upgradeExecuted = e; }
	virtual void upgradeImplementation( ) = 0; ///< Here's the actual work of Upgrading
	virtual void getUpgradeActivationMasks(UpgradeMaskType& activation, UpgradeMaskType& conflicting) const = 0; ///< Here's the actual work of Upgrading
	virtual void performUpgradeFX() = 0;	///< perform the associated fx list
	virtual Bool requiresAllActivationUpgrades() const = 0;
	virtual Bool isSubObjectsUpgrade() = 0;

	void giveSelfUpgrade()
	{
		// If I have an activation condition, and I haven't activated, and this key matches my condition.
		performUpgradeFX();
		upgradeImplementation();
		setUpgradeExecuted(true);
	}

	//
	// this is not a snapshot class itself, but it is a base class used in conjunction with
	// multiple inheritance for modules, so those modules need to be able to poke inside
	// this class and xfer the data here during a snapshot process
	//
	virtual void upgradeMuxCRC( Xfer *xfer );
	virtual void upgradeMuxXfer( Xfer *xfer);
	virtual void upgradeMuxLoadPostProcess( void );

private:
	Bool m_upgradeExecuted;				///< Upgrade only executes once

};

//-------------------------------------------------------------------------------------------------
struct UpgradeModuleData : public BehaviorModuleData
{
public:
	UpgradeMuxData				m_upgradeMuxData;

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    ModuleData::buildFieldParse(p);
		p.add(UpgradeMuxData::getFieldParse(), offsetof( UpgradeModuleData, m_upgradeMuxData ));
	}
};

//-------------------------------------------------------------------------------------------------
class UpgradeModule : public BehaviorModule, public UpgradeMux
{

	MEMORY_POOL_GLUE_ABC( UpgradeModule )
	MAKE_STANDARD_MODULE_MACRO_ABC( UpgradeModule )
	MAKE_STANDARD_MODULE_DATA_MACRO_ABC(UpgradeModule, UpgradeModuleData)

public:

	UpgradeModule( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype defined by MemoryPoolObject

	// module methods
	static Int getInterfaceMask() { return MODULEINTERFACE_UPGRADE; }

	// BehaviorModule
	virtual UpgradeModuleInterface* getUpgrade() { return this; }

protected:

	virtual Bool requiresAllActivationUpgrades() const
	{
		return getUpgradeModuleData()->m_upgradeMuxData.m_requiresAllTriggers;
	}

	virtual void getUpgradeActivationMasks(UpgradeMaskType& activation, UpgradeMaskType& conflicting) const
	{
		getUpgradeModuleData()->m_upgradeMuxData.getUpgradeActivationMasks(activation, conflicting);
	}

	virtual void performUpgradeFX()
	{
		getUpgradeModuleData()->m_upgradeMuxData.performUpgradeFX(getObject());
	}

};
inline UpgradeModule::UpgradeModule( Thing *thing, const ModuleData* moduleData ) : BehaviorModule( thing, moduleData ) { }
inline UpgradeModule::~UpgradeModule() { }
