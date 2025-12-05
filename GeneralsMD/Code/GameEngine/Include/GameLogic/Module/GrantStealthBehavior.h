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

// FILE: GrantStealthBehavior.h /////////////////////////////////////////////////////////////////////////
// Author: Mark Lorenzen, June 2003
// Desc:   Update that grants permanent stealth to those whose stealth is permanently grantable
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
class GrantStealthBehaviorModuleData : public UpdateModuleData
{
public:
	Bool									m_initiallyActive;
	Bool									m_singleBurst;
	Int										m_healingAmount;
	Real									m_startRadius;
	Real									m_finalRadius;
	Real									m_radiusGrowRate;
	KindOfMaskType				m_kindOf;	//Only these types can heal -- defaults to everything.
	const ParticleSystemTemplate*				m_radiusParticleSystemTmpl;					//Optional particle system meant to apply to entire effect for entire duration.

	GrantStealthBehaviorModuleData()
	{
		m_finalRadius = 200.0f;
		m_startRadius = 0.0f;
    m_radiusGrowRate = 10.0f;
		m_radiusParticleSystemTmpl = nullptr;
		SET_ALL_KINDOFMASK_BITS( m_kindOf );
	}

	static void buildFieldParse( MultiIniFieldParse& p )
	{
		UpdateModuleData::buildFieldParse( p );

		static const FieldParse dataFieldParse[] =
		{
			{ "StartRadius",						         INI::parseReal,									 nullptr, offsetof( GrantStealthBehaviorModuleData, m_startRadius ) },
			{ "FinalRadius",						         INI::parseReal,									 nullptr, offsetof( GrantStealthBehaviorModuleData, m_finalRadius ) },
			{ "RadiusGrowRate",						       INI::parseReal,									 nullptr, offsetof( GrantStealthBehaviorModuleData, m_radiusGrowRate ) },
			{ "KindOf",						    KindOfMaskType::parseFromINI,					       nullptr, offsetof( GrantStealthBehaviorModuleData, m_kindOf ) },
			{ "RadiusParticleSystemName",				 INI::parseParticleSystemTemplate, nullptr, offsetof( GrantStealthBehaviorModuleData, m_radiusParticleSystemTmpl ) },
			{ 0, 0, 0, 0 }
		};

		p.add(dataFieldParse);

  }

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class GrantStealthBehavior : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( GrantStealthBehavior, "GrantStealthBehavior" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( GrantStealthBehavior, GrantStealthBehaviorModuleData )

public:

	GrantStealthBehavior( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual UpdateSleepTime update();


private:

	void grantStealthToObject( Object *obj );
	ParticleSystemID m_radiusParticleSystemID;
  Real m_currentScanRadius;
};
