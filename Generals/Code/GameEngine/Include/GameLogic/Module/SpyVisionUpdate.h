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

// FILE: SpyVisionUpdate.h /////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, September 2002
// Desc:   Special Power will spy on the vision of all enemy players.
//				Putting a SpecialPower in a behavior takes a big huge amount of code duplication and
//				has no precedent.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/UpdateModule.h"

//-------------------------------------------------------------------------------------------------
class SpyVisionUpdateModuleData : public UpdateModuleData
{
public:

	SpyVisionUpdateModuleData()
	{
	}

	static void buildFieldParse(MultiIniFieldParse& p);
};

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
class SpyVisionUpdate : public UpdateModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( SpyVisionUpdate, "SpyVisionUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( SpyVisionUpdate, SpyVisionUpdateModuleData )

public:

	SpyVisionUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual void onDelete( void );
	virtual UpdateSleepTime update( void );

	void activateSpyVision( UnsignedInt duration );

private:

	void doActivationWork( Bool setting );

	UnsignedInt m_deactivateFrame;
};
