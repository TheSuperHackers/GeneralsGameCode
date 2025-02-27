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

// FILE: VeterancyGainCreate.h //////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, August 2002
// Desc:   On creation, will set Object's veterancy level if required Science is present.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#define DEFINE_VETERANCY_NAMES
// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/GameCommon.h"
#include "Common/Science.h"
#include "GameLogic/Module/CreateModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;

class VeterancyGainCreateModuleData : public CreateModuleData
{
public:
	VeterancyLevel m_startingLevel;			///< Level to set Object at
	ScienceType m_scienceRequired;			///< The science you must have to trigger this

	VeterancyGainCreateModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class VeterancyGainCreate : public CreateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( VeterancyGainCreate, "VeterancyGainCreate" );
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( VeterancyGainCreate, VeterancyGainCreateModuleData );

public:

	VeterancyGainCreate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	/// the create method
	virtual void onCreate( void );

protected:

};
