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

// FILE: EMPUpdate.h ///////////////////////////////////////////////////////////////////////
// Author: Mark Lorenzen Sept. 2002
// Desc:   Update that makes the electromagnetic pulse field grow, fade and disable junk
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpdateModule.h"
#include "GameLogic/Module/DieModule.h"
#include "GameLogic/Weapon.h"


//-------------------------------------------------------------------------------------------------
class EMPUpdateModuleData : public UpdateModuleData
{
public:
	UnsignedInt m_lifeFrames;
	UnsignedInt m_startFadeFrame;
	UnsignedInt m_disabledDuration;
	Real				m_startScale; ///< how big I start drawing
	Real				m_targetScaleMin; ///< how big I change to by the time I'm done
	Real				m_targetScaleMax; ///< how big I change to by the time I'm done
	//Real				m_spinRateMax; ///< how fast may I spin?
	RGBColor		m_startColor;
	RGBColor		m_endColor;
	const ParticleSystemTemplate *m_disableFXParticleSystem;
	Real				m_sparksPerCubicFoot; //<just like it sounds
	Real				m_effectRadius;
	Int         m_rejectMask;

  KindOfMaskType m_victimKindOf;
  KindOfMaskType m_victimKindOfNot;
	Bool				m_doesNotAffectMyOwnBuildings;

	EMPUpdateModuleData()
	{
		m_lifeFrames = 1;
		m_startFadeFrame = 0;
		m_startScale = 1.0f;
		m_targetScaleMax = 1.0f;
		m_targetScaleMin = 1.0f;
		m_startColor.setFromInt(0xffffffff);
		m_endColor.setFromInt  (0x00000000);
		//m_spinRateMax = 0.0f;
		m_disabledDuration = 0;
		m_disableFXParticleSystem = nullptr;
		m_sparksPerCubicFoot = 0.001f;
		m_effectRadius = 200;
		m_rejectMask = 0;
		m_doesNotAffectMyOwnBuildings = FALSE;

    m_victimKindOf.clear();
    m_victimKindOfNot.clear();
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    UpdateModuleData::buildFieldParse(p);
		static const FieldParse dataFieldParse[] =
		{
			{ "Lifetime",	INI::parseDurationUnsignedInt,		nullptr, offsetof( EMPUpdateModuleData, m_lifeFrames ) },
			{ "StartFadeTime",	INI::parseDurationUnsignedInt,		nullptr, offsetof( EMPUpdateModuleData, m_startFadeFrame ) },
			{ "StartScale",	INI::parseReal,										nullptr, offsetof( EMPUpdateModuleData, m_startScale ) },
			{ "DisabledDuration",	INI::parseDurationUnsignedInt,	nullptr, offsetof( EMPUpdateModuleData, m_disabledDuration ) },
			//{ "SpinRateMax",	INI::parseReal,										nullptr, offsetof( EMPUpdateModuleData, m_spinRateMax ) },
			{ "TargetScaleMax",	INI::parseReal,										nullptr, offsetof( EMPUpdateModuleData, m_targetScaleMax ) },
			{ "TargetScaleMin",	INI::parseReal,										nullptr, offsetof( EMPUpdateModuleData, m_targetScaleMin ) },
			{ "StartColor",	INI::parseRGBColor,			nullptr, offsetof( EMPUpdateModuleData, m_startColor ) },
			{ "EndColor",	INI::parseRGBColor,				nullptr, offsetof( EMPUpdateModuleData, m_endColor ) },
			{ "DisableFXParticleSystem",		INI::parseParticleSystemTemplate, nullptr, offsetof( EMPUpdateModuleData, m_disableFXParticleSystem ) },
			{ "SparksPerCubicFoot",		INI::parseReal, nullptr, offsetof( EMPUpdateModuleData, m_sparksPerCubicFoot ) },
			{ "EffectRadius",	INI::parseReal,										nullptr, offsetof( EMPUpdateModuleData, m_effectRadius ) },
			{ "DoesNotAffect", INI::parseBitString32,	TheWeaponAffectsMaskNames, offsetof(EMPUpdateModuleData, m_rejectMask) },
			{ "DoesNotAffectMyOwnBuildings", INI::parseBool, nullptr, offsetof( EMPUpdateModuleData, m_doesNotAffectMyOwnBuildings ) },

      { "VictimRequiredKindOf", KindOfMaskType::parseFromINI, nullptr, offsetof( EMPUpdateModuleData, m_victimKindOf ) },
		  { "VictimForbiddenKindOf", KindOfMaskType::parseFromINI, nullptr, offsetof( EMPUpdateModuleData, m_victimKindOfNot ) },

			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);
	}
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class EMPUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( EMPUpdate, "EMPUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( EMPUpdate, EMPUpdateModuleData )

public:

	EMPUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	UnsignedInt getDieFrame() { return m_dieFrame; }

	virtual UpdateSleepTime update( void );
	void doDisableAttack( void );

protected:

	UnsignedInt m_dieFrame;			///< frame we die on
	UnsignedInt m_tintEnvFadeFrames;///< param for tint envelope
	UnsignedInt m_tintEnvPlayFrame;///< which frame to trigger the tint envelope

	Real				m_targetScale;  ///How big will I get
	//Real				m_spinRate;			///HowMuch To Spin each frame;
	Real				m_currentScale; ///< how big I am drawing this frame

	//static Bool s_lastInstanceSpunPositive;/// so that only every other instance spins positive direction


};












//-------------------------------------------------------------------------------------------------
class LeafletDropBehaviorModuleData : public UpdateModuleData
{
public:
	UnsignedInt m_delayFrames;
	UnsignedInt m_disabledDuration;
  Real m_radius;
	const ParticleSystemTemplate *m_leafletFXParticleSystem;


	LeafletDropBehaviorModuleData()
	{
		m_delayFrames = 1;
		m_disabledDuration = 0;
    m_radius = 60.0f;
    m_leafletFXParticleSystem = nullptr;
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    UpdateModuleData::buildFieldParse(p);
		static const FieldParse dataFieldParse[] =
		{
			{ "Delay",	        INI::parseDurationUnsignedInt,	nullptr, offsetof( LeafletDropBehaviorModuleData, m_delayFrames ) },
			{ "DisabledDuration",	INI::parseDurationUnsignedInt,	nullptr, offsetof( LeafletDropBehaviorModuleData, m_disabledDuration ) },
      { "AffectRadius",     INI::parseReal,                 nullptr, offsetof( LeafletDropBehaviorModuleData, m_radius ) },
      { "LeafletFXParticleSystem", INI::parseParticleSystemTemplate,  nullptr, offsetof( LeafletDropBehaviorModuleData, m_leafletFXParticleSystem ) },



      { 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);
	}
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class LeafletDropBehavior : public UpdateModule,
													public DieModuleInterface

{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( LeafletDropBehavior, "LeafletDropBehavior" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( LeafletDropBehavior, LeafletDropBehaviorModuleData )

public:

	LeafletDropBehavior( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual UpdateSleepTime update( void );
	void doDisableAttack( void );

  // BehaviorModule
	virtual DieModuleInterface* getDie() { return this; }

	// DieModuleInterface
	virtual void onDie( const DamageInfo *damageInfo );



protected:

	UnsignedInt m_startFrame;			///< frame we die on
  Bool  m_fxFired; ///< have we done our fx yet
};
