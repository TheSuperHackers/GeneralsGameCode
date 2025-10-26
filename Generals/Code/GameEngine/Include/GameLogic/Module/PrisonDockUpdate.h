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

// FILE: PrisonDockUpdate.h ///////////////////////////////////////////////////////////////////////
// Author: Colin Day, August 2002
// Desc:   Dock update for prison structures
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/GameMemory.h"
#include "GameLogic/Module/DockUpdate.h"

#ifdef ALLOW_SURRENDER

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class PrisonDockUpdate : public DockUpdate
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( PrisonDockUpdate, "PrisonDockUpdate" )
	MAKE_STANDARD_MODULE_MACRO( PrisonDockUpdate )

public:

	PrisonDockUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by MemoryPoolObject base class
	virtual DockUpdateInterface* getDockUpdateInterface() { return this; }

	virtual Bool action( Object *docker, Object *drone = NULL );	///< for me this means do some Prison

protected:

};

#endif
