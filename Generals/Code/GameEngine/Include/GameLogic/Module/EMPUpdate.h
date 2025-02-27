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

// FILE: EMPUpdate.h ///////////////////////////////////////////////////////////////////////
// Author: Mark Lorenzen Sept. 2002
// Desc:   Update that makes the electromagnetic pulse field grow, fade and disable junk
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpdateModule.h"

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
	Real m_sparksPerCubicFoot; //<just like it sounds

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
		m_disableFXParticleSystem = NULL;
		m_sparksPerCubicFoot = 0.001f;

	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    UpdateModuleData::buildFieldParse(p);
		static const FieldParse dataFieldParse[] =
		{
			{ "Lifetime",	INI::parseDurationUnsignedInt,		NULL, offsetof( EMPUpdateModuleData, m_lifeFrames ) },
			{ "StartFadeTime",	INI::parseDurationUnsignedInt,		NULL, offsetof( EMPUpdateModuleData, m_startFadeFrame ) },
			{ "StartScale",	INI::parseReal,										NULL, offsetof( EMPUpdateModuleData, m_startScale ) },
			{ "DisabledDuration",	INI::parseDurationUnsignedInt,	NULL, offsetof( EMPUpdateModuleData, m_disabledDuration ) },
			//{ "SpinRateMax",	INI::parseReal,										NULL, offsetof( EMPUpdateModuleData, m_spinRateMax ) },
			{ "TargetScaleMax",	INI::parseReal,										NULL, offsetof( EMPUpdateModuleData, m_targetScaleMax ) },
			{ "TargetScaleMin",	INI::parseReal,										NULL, offsetof( EMPUpdateModuleData, m_targetScaleMin ) },
			{ "StartColor",	INI::parseRGBColor,			NULL, offsetof( EMPUpdateModuleData, m_startColor ) },
			{ "EndColor",	INI::parseRGBColor,				NULL, offsetof( EMPUpdateModuleData, m_endColor ) },
			{ "DisableFXParticleSystem",		INI::parseParticleSystemTemplate, NULL, offsetof( EMPUpdateModuleData, m_disableFXParticleSystem ) },
			{ "SparksPerCubicFoot",		INI::parseReal, NULL, offsetof( EMPUpdateModuleData, m_sparksPerCubicFoot ) },


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
