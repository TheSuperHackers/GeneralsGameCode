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

// FILE: NeutronBlastBehavior.h /////////////////////////////////////////////////////////////////////////
// Author: Daniel Teh, July 2003
// Desc:   Create a neutron blast behavior that wipes out infantry, no matter where they hide
//------------------------------------------

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/DieModule.h"
#include "GameLogic/Module/UpdateModule.h"

//-------------------------------------------------------------------------------------------------
class NeutronBlastBehaviorModuleData : public UpdateModuleData
{
public:
	Real m_blastRadius;
	Bool m_isAffectAirborne;
	Bool m_affectAllies;

	NeutronBlastBehaviorModuleData()
	{
		m_blastRadius = 10.0f;
		m_isAffectAirborne = TRUE;
		m_affectAllies = TRUE;
	}

	static void buildFieldParse( MultiIniFieldParse& p )
	{
		UpdateModuleData::buildFieldParse( p );

		static const FieldParse dataFieldParse[] =
		{
			{ "BlastRadius",		INI::parseReal, NULL, offsetof( NeutronBlastBehaviorModuleData, m_blastRadius ) },
			{ "AffectAirborne", INI::parseBool, NULL, offsetof( NeutronBlastBehaviorModuleData, m_isAffectAirborne ) },
			{ "AffectAllies",		INI::parseBool, NULL, offsetof( NeutronBlastBehaviorModuleData, m_affectAllies ) },
			{ 0, 0, 0, 0 }
		};

		p.add(dataFieldParse);
  }

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class NeutronBlastBehavior : public UpdateModule,
														 public DieModuleInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( NeutronBlastBehavior, "NeutronBlastBehavior" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( NeutronBlastBehavior, NeutronBlastBehaviorModuleData )

public:

	NeutronBlastBehavior( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	static Int getInterfaceMask() { return UpdateModule::getInterfaceMask() | MODULEINTERFACE_DIE; }
	virtual DieModuleInterface* getDie() { return this; }


	virtual UpdateSleepTime update();
	virtual void onDie( const DamageInfo *damageInfo );


private:

	void neutronBlastToObject( Object *obj );
};
