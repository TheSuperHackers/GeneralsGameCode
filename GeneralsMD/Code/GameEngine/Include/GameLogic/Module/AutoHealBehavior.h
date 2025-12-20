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

// FILE: AutoHealBehavior.h /////////////////////////////////////////////////////////////////////////
// Author: Colin Day, December 2001
// Desc:   Update that heals itself
//------------------------------------------
// Modified by Kris Morness, September 2002
// Kris: Added the ability to add effects, radius healing, and restricting the type of objects
//       subjected to the heal (or repair).
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameClient/ParticleSys.h"
#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/UpgradeModule.h"
#include "GameLogic/Module/UpdateModule.h"
#include "GameLogic/Module/DamageModule.h"
#include "Common/BitFlagsIO.h"

class ParticleSystem;
class ParticleSystemTemplate;

//-------------------------------------------------------------------------------------------------
class AutoHealBehaviorModuleData : public UpdateModuleData
{
public:
	UpgradeMuxData				m_upgradeMuxData;
	Bool									m_initiallyActive;
	Bool									m_singleBurst;
	Int										m_healingAmount;
	UnsignedInt						m_healingDelay;
	UnsignedInt						m_startHealingDelay;	///< how long since our last damage till autoheal starts.
	Real									m_radius; //If non-zero, then it becomes a area effect.
	Bool									m_affectsWholePlayer; ///< I have more than a range, I try to affect everything the player owns
	Bool									m_skipSelfForHealing; ///< Don't heal myself.
	KindOfMaskType				m_kindOf;	//Only these types can heal -- defaults to everything.
	KindOfMaskType				m_forbiddenKindOf;	//Only these types can heal -- defaults to everything.
	const ParticleSystemTemplate*				m_radiusParticleSystemTmpl;					//Optional particle system meant to apply to entire effect for entire duration.
	const ParticleSystemTemplate*				m_unitHealPulseParticleSystemTmpl;	//Optional particle system applying to each object getting healed each heal pulse.

	AutoHealBehaviorModuleData()
	{
		m_initiallyActive = false;
		m_singleBurst = FALSE;
		m_healingAmount = 0;
		m_healingDelay = UINT_MAX;
		m_startHealingDelay = 0;
		m_radius = 0.0f;
		m_radiusParticleSystemTmpl = NULL;
		m_unitHealPulseParticleSystemTmpl = NULL;
		m_affectsWholePlayer = FALSE;
		m_skipSelfForHealing = FALSE;
		SET_ALL_KINDOFMASK_BITS( m_kindOf );
		m_forbiddenKindOf.clear();
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
		static const FieldParse dataFieldParse[] =
		{
			{ "StartsActive",	INI::parseBool, NULL, offsetof( AutoHealBehaviorModuleData, m_initiallyActive ) },
			{ "SingleBurst",	INI::parseBool, NULL, offsetof( AutoHealBehaviorModuleData, m_singleBurst ) },
			{ "HealingAmount",		INI::parseInt,												NULL, offsetof( AutoHealBehaviorModuleData, m_healingAmount ) },
			{ "HealingDelay",			INI::parseDurationUnsignedInt,				NULL, offsetof( AutoHealBehaviorModuleData, m_healingDelay ) },
			{ "Radius",						INI::parseReal,												NULL, offsetof( AutoHealBehaviorModuleData, m_radius ) },
			{ "KindOf",						KindOfMaskType::parseFromINI,											NULL, offsetof( AutoHealBehaviorModuleData, m_kindOf ) },
			{ "ForbiddenKindOf",	KindOfMaskType::parseFromINI,											NULL, offsetof( AutoHealBehaviorModuleData, m_forbiddenKindOf ) },
			{ "RadiusParticleSystemName",					INI::parseParticleSystemTemplate,	NULL, offsetof( AutoHealBehaviorModuleData, m_radiusParticleSystemTmpl ) },
			{ "UnitHealPulseParticleSystemName",	INI::parseParticleSystemTemplate,	NULL, offsetof( AutoHealBehaviorModuleData, m_unitHealPulseParticleSystemTmpl ) },
			{ "StartHealingDelay",			INI::parseDurationUnsignedInt,				NULL, offsetof( AutoHealBehaviorModuleData, m_startHealingDelay ) },
			{ "AffectsWholePlayer",			INI::parseBool,												NULL, offsetof( AutoHealBehaviorModuleData, m_affectsWholePlayer ) },
			{ "SkipSelfForHealing",			INI::parseBool,												NULL, offsetof( AutoHealBehaviorModuleData, m_skipSelfForHealing ) },
			{ 0, 0, 0, 0 }
		};

		UpdateModuleData::buildFieldParse(p);
		p.add(dataFieldParse);
		p.add(UpgradeMuxData::getFieldParse(), offsetof( AutoHealBehaviorModuleData, m_upgradeMuxData ));
	}

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class AutoHealBehavior : public UpdateModule,
												 public UpgradeMux,
												 public DamageModuleInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( AutoHealBehavior, "AutoHealBehavior" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( AutoHealBehavior, AutoHealBehaviorModuleData )

public:

	AutoHealBehavior( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	// module methods
	static Int getInterfaceMask() { return UpdateModule::getInterfaceMask() | MODULEINTERFACE_UPGRADE | MODULEINTERFACE_DAMAGE; }

	// BehaviorModule
	virtual UpgradeModuleInterface* getUpgrade() { return this; }
	virtual DamageModuleInterface* getDamage() { return this; }

	// DamageModuleInterface
	virtual void onDamage( DamageInfo *damageInfo );
	virtual void onHealing( DamageInfo *damageInfo ) { }
	virtual void onBodyDamageStateChange(const DamageInfo* damageInfo, BodyDamageType oldState, BodyDamageType newState) { }

	// UpdateModuleInterface
	virtual UpdateSleepTime update();
	virtual DisabledMaskType getDisabledTypesToProcess() const { return MAKE_DISABLED_MASK( DISABLED_HELD ); }

	void stopHealing();
	void undoUpgrade(); ///<pretend like we have not been activated yet, so we can be reactivated later

protected:

	virtual void upgradeImplementation()
	{
		setWakeFrame(getObject(), UPDATE_SLEEP_NONE);
	}

	virtual void getUpgradeActivationMasks(UpgradeMaskType& activation, UpgradeMaskType& conflicting) const
	{
		getAutoHealBehaviorModuleData()->m_upgradeMuxData.getUpgradeActivationMasks(activation, conflicting);
	}

	virtual void performUpgradeFX()
	{
		getAutoHealBehaviorModuleData()->m_upgradeMuxData.performUpgradeFX(getObject());
	}

	virtual void processUpgradeRemoval()
	{
		// I can't take it any more.  Let the record show that I think the UpgradeMux multiple inheritence is CRAP.
		getAutoHealBehaviorModuleData()->m_upgradeMuxData.muxDataProcessUpgradeRemoval(getObject());
	}

	virtual Bool requiresAllActivationUpgrades() const
	{
		return getAutoHealBehaviorModuleData()->m_upgradeMuxData.m_requiresAllTriggers;
	}

	Bool isUpgradeActive() const { return isAlreadyUpgraded(); }

	virtual Bool isSubObjectsUpgrade() { return false; }


private:

	void pulseHealObject( Object *obj );

	ParticleSystemID m_radiusParticleSystemID;

	UnsignedInt m_soonestHealFrame;/** I need to record this, because with multiple wake up sources,
																		I can't rely solely on my sleeping.  So this will guard onDamage's wake up.
																		I could guard the act of healing, but that would defeat the gain of being
																		a sleepy module.  I never want to run update unless I am going to heal.
																 */

	Bool m_stopped;

};
