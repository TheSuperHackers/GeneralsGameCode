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

// FILE: SquishCollide.h ////////////////////////////////////////////////////////////////////////////
// Author: Steven Johnson, Jan 2002
// Desc:   Topple collide module
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/CollideModule.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////////////////////////
class Thing;

//-------------------------------------------------------------------------------------------------
/** The tank collide module */
//-------------------------------------------------------------------------------------------------
class SquishCollide : public CollideModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( SquishCollide, "SquishCollide" )
	MAKE_STANDARD_MODULE_MACRO( SquishCollide );

public:

	SquishCollide( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	/// This collide method gets called when collision occur
	virtual void onCollide( Object *other, const Coord3D *loc, const Coord3D *normal );

protected:

};
