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

///////////////////////////////////////////////////////////////////////////////////////////////////
//
// FILE: DefectorSpecialPower.h 
// Author: Mark Lorenzen, JULY 2002
// Desc:   General can click command cursor on any enemy, and it becomes his
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __DEFECTORSPECIALPOWER_H_
#define __DEFECTORSPECIALPOWER_H_

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/SpecialPowerModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Object;
class SpecialPowerTemplate;
struct FieldParse;
enum ScienceType CPP_11(: Int);




//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class DefectorSpecialPowerModuleData : public SpecialPowerModuleData
{

public:

	DefectorSpecialPowerModuleData( void );

	static void buildFieldParse( MultiIniFieldParse& p );

	Real m_fatCursorRadius;					///< the distance around the target we will reveal

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class DefectorSpecialPower : public SpecialPowerModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( DefectorSpecialPower, "DefectorSpecialPower" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( DefectorSpecialPower, DefectorSpecialPowerModuleData )

public:

	DefectorSpecialPower( Thing *thing, const ModuleData *moduleData );
	// virtual destructor prototype provided by memory pool object

	virtual void doSpecialPowerAtObject( Object *obj, UnsignedInt commandOptions );
	virtual void doSpecialPowerAtLocation( const Coord3D *loc, Real angle, UnsignedInt commandOptions );

protected:

};
#endif  // end DefectorSpecialPower

