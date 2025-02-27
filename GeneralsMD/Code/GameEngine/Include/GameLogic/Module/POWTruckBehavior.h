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

// FILE: POWTruckBehavior.h ///////////////////////////////////////////////////////////////////////
// Author: Colin Day
// Desc:   POW Truck Behavior
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/OpenContain.h"

#ifdef ALLOW_SURRENDER

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class POWTruckBehaviorModuleData : public OpenContainModuleData
{

public:

	POWTruckBehaviorModuleData( void );

	static void buildFieldParse( MultiIniFieldParse &p );

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class POWTruckBehavior : public OpenContain
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( POWTruckBehavior, "POWTruckBehavior" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( POWTruckBehavior, POWTruckBehaviorModuleData )

public:

	POWTruckBehavior( Thing *thing, const ModuleData *moduleData );
	// virtual destructor prototype provided by memory pool declaration

	// collide methods
	virtual void onCollide( Object *other, const Coord3D *loc, const Coord3D *normal );

protected:

};

#endif
