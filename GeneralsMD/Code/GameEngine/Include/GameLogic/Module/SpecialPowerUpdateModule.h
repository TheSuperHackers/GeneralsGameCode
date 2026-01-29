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

// FILE: SpecialPowerUpdateModule.h /////////////////////////////////////////////////////////////////////////////////
// Author: Kris Morness, July 2003
// Desc: Originally lived in UpdateModule.h
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common/Module.h"
#include "Common/GameType.h"

//-------------------------------------------------------------------------------------------------
class SpecialPowerUpdateInterface
{
public:
	virtual Bool doesSpecialPowerUpdatePassScienceTest() const = 0;
	virtual ScienceType getExtraRequiredScience() const = 0; //Does this object have more than one special power module with the same spTemplate?

	virtual Bool initiateIntentToDoSpecialPower(const SpecialPowerTemplate *specialPowerTemplate, const Object *targetObj, const Coord3D *targetPos, const Waypoint *way, UnsignedInt commandOptions ) = 0;
	virtual Bool isSpecialAbility() const = 0;
	virtual Bool isSpecialPower() const = 0;
	virtual Bool isActive() const = 0;
	virtual CommandOption getCommandOption() const = 0;
	virtual Bool doesSpecialPowerHaveOverridableDestinationActive() const = 0; //Is it active now?
	virtual Bool doesSpecialPowerHaveOverridableDestination() const = 0;	//Does it have it, even if it's not active?
	virtual void setSpecialPowerOverridableDestination( const Coord3D *loc ) = 0;
	virtual Bool isPowerCurrentlyInUse( const CommandButton *command = nullptr ) const = 0;
};

//-------------------------------------------------------------------------------------------------
class SpecialPowerUpdateModule : public UpdateModule, public SpecialPowerUpdateInterface
{

	MEMORY_POOL_GLUE_ABC( SpecialPowerUpdateModule )
	MAKE_STANDARD_MODULE_MACRO_ABC( SpecialPowerUpdateModule )

public:

	SpecialPowerUpdateModule( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype defined by MemoryPoolObject

	//SpecialPowerUpdateInterface virtual implementations
	virtual Bool doesSpecialPowerUpdatePassScienceTest() const;
	virtual ScienceType getExtraRequiredScience() const { return SCIENCE_INVALID; } //Does this object have more than one special power module with the same spTemplate?

	//SpecialPowerUpdateInterface PURE virtual implementations
	virtual Bool initiateIntentToDoSpecialPower(const SpecialPowerTemplate *specialPowerTemplate, const Object *targetObj, const Coord3D *targetPos, const Waypoint *way, UnsignedInt commandOptions ) = 0;
	virtual Bool isSpecialAbility() const = 0;
	virtual Bool isSpecialPower() const = 0;
	virtual Bool isActive() const = 0;
	virtual CommandOption getCommandOption() const = 0;
	virtual Bool doesSpecialPowerHaveOverridableDestinationActive() const = 0; //Is it active now?
	virtual Bool doesSpecialPowerHaveOverridableDestination() const = 0;	//Does it have it, even if it's not active?
	virtual void setSpecialPowerOverridableDestination( const Coord3D *loc ) = 0;
	virtual Bool isPowerCurrentlyInUse( const CommandButton *command = nullptr ) const = 0;

};
