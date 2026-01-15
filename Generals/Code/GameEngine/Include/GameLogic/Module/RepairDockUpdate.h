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

// FILE: RepairDockUpdate.h ///////////////////////////////////////////////////////////////////////
// Author: Colin Day, June 2002
// Desc:   The action of docking with a structure for repairs
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/GameMemory.h"
#include "GameLogic/Module/SupplyCenterDockUpdate.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class RepairDockUpdateModuleData : public DockUpdateModuleData
{

public:

	RepairDockUpdateModuleData( void );

	static void buildFieldParse(MultiIniFieldParse& p);

	Real m_framesForFullHeal;			///< time (in frames) something becomes fully repaired

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class RepairDockUpdate : public DockUpdate
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( RepairDockUpdate, "RepairDockUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( RepairDockUpdate, RepairDockUpdateModuleData )

public:

	RepairDockUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by MemoryPoolObject base class

	virtual DockUpdateInterface* getDockUpdateInterface() { return this; }

	virtual Bool action( Object *docker, Object *drone = nullptr );	///< for me this means do some repair

	virtual Bool isRallyPointAfterDockType(){return TRUE;} ///< A minority of docks want to give you a final command to their rally point

protected:

  ObjectID m_lastRepair;			///< object we were repairing last
	Real m_healthToAddPerFrame;	///< health to add per frame to current docked object

};
