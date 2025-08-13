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

// FILE: ConvertToCarBombCrateCollide.h /////////////////////////////////////////////////////////////////////////
// Author: Kris Morness, April 2002
// Desc:   A crate (actually a terrorist - mobile crate) that converts a car into a carbomb, activating
//         it's weapon and then activating it's AI.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef CONVERT_TO_CAR_BOMB_CRATE_COLLIDE_H_
#define CONVERT_TO_CAR_BOMB_CRATE_COLLIDE_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/Module.h"
#include "GameLogic/Module/CrateCollide.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;
class FXList;

//-------------------------------------------------------------------------------------------------
class ConvertToCarBombCrateCollideModuleData : public CrateCollideModuleData
{
public:
	UnsignedInt m_rangeOfEffect;
	const FXList *m_fxList;

	ConvertToCarBombCrateCollideModuleData()
	{
		m_rangeOfEffect = 0;
		m_fxList = NULL;
	}

	static void buildFieldParse(MultiIniFieldParse& p)
	{
    CrateCollideModuleData::buildFieldParse(p);

		static const FieldParse dataFieldParse[] =
		{
			{ "FXList",		INI::parseFXList,		NULL, offsetof( ConvertToCarBombCrateCollideModuleData, m_fxList ) },
			{ 0, 0, 0, 0 }
		};
    p.add(dataFieldParse);
	}
};

//-------------------------------------------------------------------------------------------------
class ConvertToCarBombCrateCollide : public CrateCollide
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( ConvertToCarBombCrateCollide, "ConvertToCarBombCrateCollide" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( ConvertToCarBombCrateCollide, ConvertToCarBombCrateCollideModuleData );

public:

	ConvertToCarBombCrateCollide( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

protected:

	/// This allows specific vetoes to certain types of crates and their data
	virtual Bool isValidToExecute( const Object *other ) const;

	/// This is the game logic execution function that all real CrateCollides will implement
	virtual Bool executeCrateBehavior( Object *other );
	virtual Bool isRailroad() const { return FALSE;};
	virtual Bool isCarBombCrateCollide() const { return TRUE; }
};

#endif
