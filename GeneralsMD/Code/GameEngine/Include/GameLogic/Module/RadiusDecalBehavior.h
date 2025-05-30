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

// FILE: RadiusDecalBehavior.h /////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __RadiusDecalBehavior_H_
#define __RadiusDecalBehavior_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/UpgradeModule.h"
#include "GameLogic/Module/UpdateModule.h"
#include "GameClient/RadiusDecal.h"

//-------------------------------------------------------------------------------------------------
class RadiusDecalBehaviorModuleData : public UpdateModuleData
{
public:
	UpgradeMuxData				m_upgradeMuxData;
	Bool						m_initiallyActive;

	RadiusDecalTemplate	        m_decalTemplate;
	Real					    m_decalRadius;

	RadiusDecalBehaviorModuleData();

	static void buildFieldParse(MultiIniFieldParse& p);
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class RadiusDecalBehavior : public UpdateModule, public UpgradeMux
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( RadiusDecalBehavior, "RadiusDecalBehavior" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( RadiusDecalBehavior, RadiusDecalBehaviorModuleData )

public:

	RadiusDecalBehavior( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	// module methids
	static Int getInterfaceMask() { return UpdateModule::getInterfaceMask() | (MODULEINTERFACE_UPGRADE); }

	// BehaviorModule
	virtual UpgradeModuleInterface* getUpgrade() { return this; }

	//void createRadiusDecal( const Coord3D& pos );
	// void createRadiusDecal( const RadiusDecalTemplate& tmpl, Real radius, const Coord3D& pos );
	
	void createRadiusDecal( void );
	void killRadiusDecal( void );

	// UpdateModuleInterface
	virtual UpdateSleepTime update();

protected:


	virtual void upgradeImplementation()
	{
		createRadiusDecal();
		setWakeFrame(getObject(), UPDATE_SLEEP_NONE);
	}

	virtual void getUpgradeActivationMasks(UpgradeMaskType& activation, UpgradeMaskType& conflicting) const
	{
		getRadiusDecalBehaviorModuleData()->m_upgradeMuxData.getUpgradeActivationMasks(activation, conflicting);
	}

	virtual void performUpgradeFX()
	{
		getRadiusDecalBehaviorModuleData()->m_upgradeMuxData.performUpgradeFX(getObject());
	}

	virtual void processUpgradeRemoval()
	{
		// I can't take it any more.  Let the record show that I think the UpgradeMux multiple inheritence is CRAP.
		getRadiusDecalBehaviorModuleData()->m_upgradeMuxData.muxDataProcessUpgradeRemoval(getObject());
	}

	virtual Bool requiresAllActivationUpgrades() const
	{
		return getRadiusDecalBehaviorModuleData()->m_upgradeMuxData.m_requiresAllTriggers;
	}

	inline Bool isUpgradeActive() const { return isAlreadyUpgraded(); }

	virtual Bool isSubObjectsUpgrade() { return false; }

private:

	RadiusDecal m_radiusDecal;

	void clearDecal( void );
};

#endif // __RadiusDecalBehavior_H_

