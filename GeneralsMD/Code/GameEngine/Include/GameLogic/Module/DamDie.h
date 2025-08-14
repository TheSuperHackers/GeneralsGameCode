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

// FILE: DamDie.h /////////////////////////////////////////////////////////////////////////////////
// Author: Colin Day, April 2002
// Desc:   The big water dam dying
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __DAMDIE_H_
#define __DAMDIE_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/DieModule.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class DamDieModuleData : public DieModuleData
{

public:

	DamDieModuleData( void );

	static void buildFieldParse(MultiIniFieldParse& p);

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class DamDie : public DieModule
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( DamDie, "DamDie" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( DamDie, DamDieModuleData )

public:

	DamDie( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prorotype provided by MemoryPoolObject

	virtual void onDie( const DamageInfo *damageInfo );

};

#endif  // end __DAMDIE_H_
