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

// FILE: CashHackSpecialPower.h ///////////////////////////////////////////////////////////////
// Author: Colin Day, June 2002
// Desc:   The Spy Satellite will reveal shrouded areas of the map that the player chooses
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/SpecialPowerModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Object;
class SpecialPowerTemplate;
struct FieldParse;
enum ScienceType CPP_11(: Int);

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class CashHackSpecialPowerModuleData : public SpecialPowerModuleData
{

public:

	struct Upgrades
	{
		ScienceType									m_science;
		Int													m_amountToSteal;

		Upgrades() : m_science(SCIENCE_INVALID), m_amountToSteal(0)
		{
		}
	};
	std::vector<Upgrades> m_upgrades;
	Int m_defaultAmountToSteal;					///< the amount of money that we will steal

	CashHackSpecialPowerModuleData( void );
	static void buildFieldParse( MultiIniFieldParse& p );
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class CashHackSpecialPower : public SpecialPowerModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( CashHackSpecialPower, "CashHackSpecialPower" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( CashHackSpecialPower, CashHackSpecialPowerModuleData )

public:

	CashHackSpecialPower( Thing *thing, const ModuleData *moduleData );
	// virtual destructor provided by memory pool object

	virtual void doSpecialPowerAtObject( Object *obj, UnsignedInt commandOptions );
	virtual void doSpecialPowerAtLocation( const Coord3D *loc, Real angle, UnsignedInt commandOptions );

protected:

	Int findAmountToSteal() const;

};
