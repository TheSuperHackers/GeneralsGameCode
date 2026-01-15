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

// FILE: StealthDetectorUpdate.h //////////////////////////////////////////////////////////////////////////
// Author: Steven Johnson, May 2002
// Desc:	 An update that checks for a status bit to stealth the owning object
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpdateModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;

//-------------------------------------------------------------------------------------------------
class StealthDetectorUpdateModuleData : public UpdateModuleData
{
public:
	UnsignedInt											m_updateRate;
	Real														m_detectionRange;
	Bool														m_initiallyDisabled;
	AudioEventRTS										m_pingSound;
	AudioEventRTS										m_loudPingSound;
	const ParticleSystemTemplate*   m_IRBeaconParticleSysTmpl;
	const ParticleSystemTemplate*   m_IRParticleSysTmpl;
	const ParticleSystemTemplate*   m_IRBrightParticleSysTmpl;
	const ParticleSystemTemplate*   m_IRGridParticleSysTmpl;
	AsciiString											m_IRParticleSysBone;
	KindOfMaskType									m_extraDetectKindof;			///< units must match any kindof bits set here, in order to be detected
	KindOfMaskType									m_extraDetectKindofNot;		///< units must NOT match any kindof bits set here, in order to be detected
	Bool														m_canDetectWhileGarrisoned;
	Bool														m_canDetectWhileTransported;

	StealthDetectorUpdateModuleData()
	{
		m_updateRate = 1;
		m_detectionRange = 0.0f;
		m_initiallyDisabled = false;
		m_IRBeaconParticleSysTmpl = nullptr;
		m_IRParticleSysTmpl = nullptr;
		m_IRBrightParticleSysTmpl = nullptr;
		m_IRGridParticleSysTmpl = nullptr;
		m_extraDetectKindof.clear();
		m_extraDetectKindofNot.clear();
		m_canDetectWhileGarrisoned = false;
		m_canDetectWhileTransported = false;
	}

	static void buildFieldParse(MultiIniFieldParse& p);

};

//-------------------------------------------------------------------------------------------------
class StealthDetectorUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( StealthDetectorUpdate, "StealthDetectorUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( StealthDetectorUpdate, StealthDetectorUpdateModuleData );

public:

	StealthDetectorUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	Bool isSDEnabled() const { return m_enabled; }
	void setSDEnabled( Bool enabled );
	virtual UpdateSleepTime update();
	virtual DisabledMaskType getDisabledTypesToProcess() const { return MAKE_DISABLED_MASK( DISABLED_HELD ); }

private:
	Bool m_enabled;

};
