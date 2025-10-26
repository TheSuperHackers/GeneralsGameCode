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
//  (c) 2003 Electronic Arts Inc.																				      //
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: CountermeasuresBehavior.h /////////////////////////////////////////////////////////////////////////
// Author: Kris Morness, April 2003
// Desc: Handles countermeasure firing when under missile threat, and responsible
//       for diverting missiles to the flares.
//------------------------------------------

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
class CountermeasuresBehaviorModuleData : public UpdateModuleData
{
public:
	UpgradeMuxData				m_upgradeMuxData;
	AsciiString						m_flareTemplateName;
	AsciiString						m_flareBoneBaseName;
  Real									m_evasionRate;
  UnsignedInt						m_volleySize;
  Real									m_volleyArcAngle;
	Real									m_volleyVelocityFactor;
  UnsignedInt						m_framesBetweenVolleys;
	UnsignedInt						m_numberOfVolleys;
  UnsignedInt						m_reloadFrames;
	UnsignedInt						m_missileDecoyFrames;
	UnsignedInt						m_countermeasureReactionFrames;
	Bool									m_mustReloadAtAirfield;

	CountermeasuresBehaviorModuleData()
	{
    m_volleySize            = 0;
		m_volleyArcAngle				= 0.0f;
    m_framesBetweenVolleys  = 0;
		m_numberOfVolleys       = 0;
    m_reloadFrames          = 0;
    m_evasionRate           = 0.0f;
		m_mustReloadAtAirfield	= FALSE;
		m_missileDecoyFrames		= 0;
		m_volleyVelocityFactor  = 1.0f;
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
		static const FieldParse dataFieldParse[] =
		{
			{ "FlareTemplateName",			INI::parseAsciiString,					NULL, offsetof( CountermeasuresBehaviorModuleData, m_flareTemplateName ) },
			{ "FlareBoneBaseName",			INI::parseAsciiString,					NULL, offsetof( CountermeasuresBehaviorModuleData, m_flareBoneBaseName ) },
			{ "VolleySize",							INI::parseUnsignedInt,					NULL, offsetof( CountermeasuresBehaviorModuleData, m_volleySize ) },
			{ "VolleyArcAngle",					INI::parseAngleReal,						NULL, offsetof( CountermeasuresBehaviorModuleData, m_volleyArcAngle ) },
			{ "VolleyVelocityFactor",		INI::parseReal,						NULL, offsetof( CountermeasuresBehaviorModuleData, m_volleyVelocityFactor ) },
			{ "DelayBetweenVolleys",		INI::parseDurationUnsignedInt,  NULL, offsetof( CountermeasuresBehaviorModuleData, m_framesBetweenVolleys ) },
			{ "NumberOfVolleys",				INI::parseUnsignedInt,					NULL, offsetof( CountermeasuresBehaviorModuleData, m_numberOfVolleys ) },
			{ "ReloadTime",							INI::parseDurationUnsignedInt,  NULL, offsetof( CountermeasuresBehaviorModuleData, m_reloadFrames ) },
			{ "EvasionRate",						INI::parsePercentToReal,				NULL, offsetof( CountermeasuresBehaviorModuleData, m_evasionRate ) },
			{ "MustReloadAtAirfield",		INI::parseBool,									NULL, offsetof( CountermeasuresBehaviorModuleData, m_mustReloadAtAirfield ) },
			{ "MissileDecoyDelay",			INI::parseDurationUnsignedInt,	NULL, offsetof( CountermeasuresBehaviorModuleData, m_missileDecoyFrames ) },
			{ "ReactionLaunchLatency",	INI::parseDurationUnsignedInt,	NULL, offsetof( CountermeasuresBehaviorModuleData, m_countermeasureReactionFrames ) },
			{ 0, 0, 0, 0 }
		};

		UpdateModuleData::buildFieldParse(p);
		p.add(dataFieldParse);
		p.add(UpgradeMuxData::getFieldParse(), offsetof( CountermeasuresBehaviorModuleData, m_upgradeMuxData ));
	}

};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class CountermeasuresBehaviorInterface
{
public:
	virtual void reportMissileForCountermeasures( Object *missile ) = 0;
	virtual ObjectID calculateCountermeasureToDivertTo( const Object& victim ) = 0;
	virtual void reloadCountermeasures() = 0;
	virtual Bool isActive() const = 0;
};


typedef std::vector<ObjectID> CountermeasuresVec;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class CountermeasuresBehavior : public UpdateModule, public UpgradeMux, public CountermeasuresBehaviorInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( CountermeasuresBehavior, "CountermeasuresBehavior" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( CountermeasuresBehavior, CountermeasuresBehaviorModuleData )

public:

	CountermeasuresBehavior( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	// module methods
	static Int getInterfaceMask() { return UpdateModule::getInterfaceMask() | MODULEINTERFACE_UPGRADE; }

	// BehaviorModule
	virtual UpgradeModuleInterface* getUpgrade() { return this; }
	virtual CountermeasuresBehaviorInterface* getCountermeasuresBehaviorInterface() { return this; }
	virtual const CountermeasuresBehaviorInterface* getCountermeasuresBehaviorInterface() const { return this; }

	// UpdateModuleInterface
	virtual UpdateSleepTime update();
	virtual DisabledMaskType getDisabledTypesToProcess() const { return MAKE_DISABLED_MASK( DISABLED_HELD ); }

	// CountermeasuresBehaviorInterface
	virtual void reportMissileForCountermeasures( Object *missile );
	virtual ObjectID calculateCountermeasureToDivertTo( const Object& victim );
	virtual void reloadCountermeasures();
	virtual Bool isActive() const;


protected:

	virtual void upgradeImplementation()
	{
		setWakeFrame(getObject(), UPDATE_SLEEP_NONE);
	}

	virtual void getUpgradeActivationMasks(UpgradeMaskType& activation, UpgradeMaskType& conflicting) const
	{
		getCountermeasuresBehaviorModuleData()->m_upgradeMuxData.getUpgradeActivationMasks(activation, conflicting);
	}

	virtual void performUpgradeFX()
	{
		getCountermeasuresBehaviorModuleData()->m_upgradeMuxData.performUpgradeFX(getObject());
	}

	virtual void processUpgradeRemoval()
	{
		// I can't take it any more.  Let the record show that I think the UpgradeMux multiple inheritence is CRAP.
		getCountermeasuresBehaviorModuleData()->m_upgradeMuxData.muxDataProcessUpgradeRemoval(getObject());
	}

	virtual Bool requiresAllActivationUpgrades() const
	{
		return getCountermeasuresBehaviorModuleData()->m_upgradeMuxData.m_requiresAllTriggers;
	}

	inline Bool isUpgradeActive() const { return isAlreadyUpgraded(); }

	virtual Bool isSubObjectsUpgrade() { return false; }

	void launchVolley();

private:
	CountermeasuresVec m_counterMeasures;		//vector of countermeasures in the world.
	UnsignedInt m_availableCountermeasures;	//number of countermeasures that can be launched to divert missiles.
	UnsignedInt m_activeCountermeasures;		//number of countermeasures currently able to divert missiles.
	UnsignedInt m_divertedMissiles;					//number of missiles that have been diverted to countermeasures.
	UnsignedInt m_incomingMissiles;					//grand total of all missiles that were ever fired at me.
	UnsignedInt m_reactionFrame;						//The frame countermeasures will be launched after initial hostile act.
	UnsignedInt m_nextVolleyFrame;					//Frame the next volley is fired.
	UnsignedInt m_reloadFrame;							//The frame countermeasures will be ready to use again.
};
