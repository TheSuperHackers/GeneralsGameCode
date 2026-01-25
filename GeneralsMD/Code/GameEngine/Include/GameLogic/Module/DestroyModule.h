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

// FILE: DestroyModule.h /////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, September 2001
// Desc:
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common/Module.h"
#include "GameLogic/Module/BehaviorModule.h"

//-------------------------------------------------------------------------------------------------
/** OBJECT DESTROY MODULE base class */
//-------------------------------------------------------------------------------------------------
class DestroyModuleInterface
{
public:
	virtual void onDestroy() = 0;
};

//-------------------------------------------------------------------------------------------------
class DestroyModule : public BehaviorModule, public DestroyModuleInterface
{

	MEMORY_POOL_GLUE_ABC( DestroyModule )
	MAKE_STANDARD_MODULE_MACRO_ABC( DestroyModule )

public:

	DestroyModule( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype defined by MemoryPoolObject

	static Int getInterfaceMask() { return MODULEINTERFACE_DESTROY; }

	// BehaviorModule
	virtual DestroyModuleInterface* getDestroy() { return this; }

	virtual void onDestroy() = 0;

protected:

};
