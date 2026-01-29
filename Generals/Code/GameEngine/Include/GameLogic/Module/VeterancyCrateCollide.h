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

// FILE: VeterancyCrateCollide.h /////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, March 2002
// Desc:   A crate that gives a level of experience to all within n distance
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/Module.h"
#include "GameLogic/Module/CrateCollide.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;

//-------------------------------------------------------------------------------------------------
class VeterancyCrateCollideModuleData : public CrateCollideModuleData
{
public:
	UnsignedInt m_rangeOfEffect;
	Bool m_addsOwnerVeterancy;
	Bool m_isPilot;

	VeterancyCrateCollideModuleData()
	{
		m_rangeOfEffect = 0;
		m_addsOwnerVeterancy = false;
		m_isPilot = false;
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    CrateCollideModuleData::buildFieldParse(p);

		static const FieldParse dataFieldParse[] =
		{
			{ "EffectRange",	INI::parseUnsignedInt,	nullptr, offsetof( VeterancyCrateCollideModuleData, m_rangeOfEffect ) },
			{ "AddsOwnerVeterancy",	INI::parseBool,	nullptr, offsetof( VeterancyCrateCollideModuleData, m_addsOwnerVeterancy ) },
			{ "IsPilot", INI::parseBool, nullptr, offsetof( VeterancyCrateCollideModuleData, m_isPilot ) },
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);

	}
};

//-------------------------------------------------------------------------------------------------
class VeterancyCrateCollide : public CrateCollide
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( VeterancyCrateCollide, "VeterancyCrateCollide" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( VeterancyCrateCollide, VeterancyCrateCollideModuleData );

public:

	VeterancyCrateCollide( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

protected:

	/// This allows specific vetoes to certain types of crates and their data
	virtual Bool isValidToExecute( const Object *other ) const;

	/// This is the game logic execution function that all real CrateCollides will implement
	virtual Bool executeCrateBehavior( Object *other );

	Int getLevelsToGain() const;

};
