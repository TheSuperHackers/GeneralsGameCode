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

// FILE: SupplyWarehouseDockUpdate.h /////////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood Feb 2002
// Desc:   The action of this dock update is identifying who is docking and either taking Boxes away or giving them
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/INI.h"
#include "Common/GameMemory.h"
#include "GameLogic/Module/DockUpdate.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class SupplyWarehouseDockUpdateModuleData : public DockUpdateModuleData
{
public:

  SupplyWarehouseDockUpdateModuleData( void );

	static void buildFieldParse(MultiIniFieldParse& p);

	Int m_startingBoxesData;
	Bool m_deleteWhenEmpty;
};

//-------------------------------------------------------------------------------------------------
class SupplyWarehouseDockUpdate : public DockUpdate
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( SupplyWarehouseDockUpdate, "SupplyWarehouseDockUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( SupplyWarehouseDockUpdate, SupplyWarehouseDockUpdateModuleData )

public:

	virtual DockUpdateInterface* getDockUpdateInterface() { return this; }

	SupplyWarehouseDockUpdate( Thing *thing, const ModuleData* moduleData );

	virtual void setDockCrippled( Bool setting ); ///< Game Logic can set me as inoperative.  I get to decide what that means.
	virtual Bool action( Object* docker, Object *drone = nullptr );	///<For me, this means identifying who is docking and either taking Boxes away or giving them

	Int getBoxesStored() const { return m_boxesStored; }

	void setCashValue( Int cashValue );

	virtual void onObjectCreated();
protected:


	Int m_boxesStored;

};
