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

// FILE: OCLSpecialPower.h ////////////////////////////////////////////////////////////////////////
// Author: Colin Day, April 2002
// Desc:   Special powers that are driven by object creation lists
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __OCLSPECIALPOWER_H_
#define __OCLSPECIALPOWER_H_

// USER INCLUDES //////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/SpecialPowerModule.h"
#include "Common/Science.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class ObjectCreationList;

enum OCLCreateLocType CPP_11(: Int)
{
	CREATE_AT_EDGE_NEAR_SOURCE,
	CREATE_AT_EDGE_NEAR_TARGET,
	CREATE_AT_LOCATION,
	USE_OWNER_OBJECT,
	CREATE_ABOVE_LOCATION,
	CREATE_AT_EDGE_FARTHEST_FROM_TARGET,
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class OCLSpecialPowerModuleData : public SpecialPowerModuleData
{

public:

	struct Upgrades
	{
		ScienceType									m_science;
		const ObjectCreationList*		m_ocl;

		Upgrades() : m_science(SCIENCE_INVALID), m_ocl(NULL)
		{
		}
	};

	std::vector<Upgrades>			m_upgradeOCL;
	const ObjectCreationList*	m_defaultOCL;
	OCLCreateLocType					m_createLoc;

	OCLSpecialPowerModuleData( void );
	static void buildFieldParse(MultiIniFieldParse& p);

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class OCLSpecialPower : public SpecialPowerModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( OCLSpecialPower, "OCLSpecialPower" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( OCLSpecialPower, OCLSpecialPowerModuleData )

public:

	OCLSpecialPower( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool object

	virtual void doSpecialPower( UnsignedInt commandOptions );
	virtual void doSpecialPowerAtObject( Object *obj, UnsignedInt commandOptions );
	virtual void doSpecialPowerAtLocation( const Coord3D *loc, UnsignedInt commandOptions );

protected:

	const ObjectCreationList* findOCL() const;
};

#endif  // end __OCLSPECIALPOWER_H_
