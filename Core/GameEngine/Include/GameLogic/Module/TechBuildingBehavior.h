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

// FILE: TechBuildingBehavior.h ///////////////////////////////////////////////////////////////////
// Author: Colin Day, October 2002
// Desc:   Tech building basic behavior
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/DieModule.h"
#include "GameLogic/Module/UpdateModule.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class TechBuildingBehaviorModuleData : public UpdateModuleData
{

public:

	TechBuildingBehaviorModuleData( void );

	static void buildFieldParse( MultiIniFieldParse &p );

	const FXList*		m_pulseFX;										///< FXList to play when bldg is owned is updated
	UnsignedInt			m_pulseFXRate;								///< how frequently to play it

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class TechBuildingBehavior : public UpdateModule,
														 public DieModuleInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( TechBuildingBehavior, "TechBuildingBehavior" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( TechBuildingBehavior, TechBuildingBehaviorModuleData )

public:

	TechBuildingBehavior( Thing *thing, const ModuleData *modData );
	// virtual destructor prototype provided by memory pool object

	// module methods
	virtual void onCapture( Player *oldOwner, Player *newOwner );
	static Int getInterfaceMask() { return UpdateModule::getInterfaceMask() | (MODULEINTERFACE_DIE); }

	// update methods
	virtual UpdateSleepTime update( void );

	// die methods
	virtual DieModuleInterface *getDie( void ) { return this; }
	virtual void onDie( const DamageInfo *damageInfo );

protected:

};
