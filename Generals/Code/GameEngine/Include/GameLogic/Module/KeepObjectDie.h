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

// FILE: KeepObjectDie.h /////////////////////////////////////////////////////////////////////////////
// Author: Kris Morness, November 2002
// Desc:   Die module for things that want to leave rubble in the world and don't have other die
//         modules. This fixes civilian buildings that don't have garrison contains. Garrison
//         contains have a die module built in, so these buildings need something. Without it
//         they default to the destroydie module which outright removes the object.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/DieModule.h"
#include "Common/INI.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;

class KeepObjectDie : public DieModule
{

	MAKE_STANDARD_MODULE_MACRO( KeepObjectDie );
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( KeepObjectDie, "KeepObjectDie" )

public:

	KeepObjectDie( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual void onDie( const DamageInfo *damageInfo );

};
