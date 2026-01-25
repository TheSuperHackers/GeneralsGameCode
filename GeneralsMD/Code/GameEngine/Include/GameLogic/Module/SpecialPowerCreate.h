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

// FILE: SpecialPowerCreate.h /////////////////////////////////////////////////////////////////////////////
// Author: Matthew D. Campbell, May 2002
// Desc:   When a building is created, tell special powers to start counting down
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/CreateModule.h"

class Thing;

//-------------------------------------------------------------------------------------------------
/** SpecialPowerCreate */
//-------------------------------------------------------------------------------------------------
class SpecialPowerCreate : public CreateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( SpecialPowerCreate, "SpecialPowerCreate" )
	MAKE_STANDARD_MODULE_MACRO( SpecialPowerCreate )

public:

	SpecialPowerCreate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual void onCreate( void );
	virtual void onBuildComplete();	///< This is called when you are a finished game object

protected:

};
