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

// FILE: W3DSupplyDraw.h ////////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, September 2002
// Desc: Draw module reacts to SupplyStatus setting by hiding an equal number of the specified bone array.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef _W3D_SUPPLY_DRAW_H_
#define _W3D_SUPPLY_DRAW_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "W3DDevice/GameClient/Module/W3DModelDraw.h"

//-------------------------------------------------------------------------------------------------
class W3DSupplyDrawModuleData : public W3DModelDrawModuleData
{
public:
	AsciiString m_supplyBonePrefix;

	W3DSupplyDrawModuleData();
	~W3DSupplyDrawModuleData();
	static void buildFieldParse(MultiIniFieldParse& p);
};

//-------------------------------------------------------------------------------------------------
class W3DSupplyDraw : public W3DModelDraw
{

 	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( W3DSupplyDraw, "W3DSupplyDraw" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( W3DSupplyDraw, W3DSupplyDrawModuleData )

public:

	W3DSupplyDraw( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual void updateDrawModuleSupplyStatus( Int maxSupply, Int currentSupply ); ///< This will do visual feedback on Supplies carried
	virtual void reactToGeometryChange() { }

protected:
	Int m_totalBones;
	Int m_lastNumberShown;
};

#endif // _W3D_TRUCK_DRAW_H_

