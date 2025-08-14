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

// FILE: FireSpreadUpdate.h /////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, April 2002
// Desc:   Update looks for ::Aflame and explicitly ignites someone nearby if set
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __FIRE_SPREAD_UPDATE_H_
#define __FIRE_SPREAD_UPDATE_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpdateModule.h"

class ObjectCreationList;

//-------------------------------------------------------------------------------------------------
class FireSpreadUpdateModuleData : public UpdateModuleData
{
public:
	const ObjectCreationList *m_oclEmbers;
	UnsignedInt m_minSpreadTryDelayData;
	UnsignedInt m_maxSpreadTryDelayData;
	Real m_spreadTryRange;

	FireSpreadUpdateModuleData();

	static void buildFieldParse(MultiIniFieldParse& p);

private:

};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class FireSpreadUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( FireSpreadUpdate, "FireSpreadUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( FireSpreadUpdate, FireSpreadUpdateModuleData )

public:

	FireSpreadUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual UpdateSleepTime update();

	void startFireSpreading();

protected:

	UnsignedInt calcNextSpreadDelay();

};

#endif

