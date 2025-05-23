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

// FILE: Special Ability.h ///////////////////////////////////////////////////////////////
// Author: Kris Morness, July 2002
// Desc:   This is the class that handles processing of any special attack from a unit. There are 
//         many different styles and rules for various attacks.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __SPECIAL_ABILITY_H_
#define __SPECIAL_ABILITY_H_

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/SpecialPowerModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Object;

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class SpecialAbilityModuleData : public SpecialPowerModuleData
{
	// nothing
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class SpecialAbility : public SpecialPowerModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( SpecialAbility, "SpecialAbility" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( SpecialAbility, SpecialAbilityModuleData )

public:

	SpecialAbility( Thing *thing, const ModuleData *moduleData );

	virtual void doSpecialPowerAtObject( Object *obj, UnsignedInt commandOptions );
	virtual void doSpecialPowerAtLocation( const Coord3D *loc, Real angle, UnsignedInt commandOptions );
	virtual void doSpecialPower( UnsignedInt commandOptions );

protected:

};

#endif  // end __SPECIAL_ABILITY_H_