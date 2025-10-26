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

// FILE: ObjectWeaponStatusHelper.h ///////////////////////////////////////////////////////////////
// Author: Steven Johnson, Colin Day - September 202
// Desc:   Object helpder - WeaponStatus
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/ThingTemplate.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/ObjectHelper.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class ObjectWeaponStatusHelperModuleData : public ModuleData
{

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class ObjectWeaponStatusHelper : public ObjectHelper
{

	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( ObjectWeaponStatusHelper, ObjectWeaponStatusHelperModuleData )
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(ObjectWeaponStatusHelper, "ObjectWeaponStatusHelperPool" )

protected:

	/*
		The ObjectWeaponStatusHelper needs to have its update run after all "normal"
		user update modules, so it redefines this. Please don't redefine this
		for other modules without very careful deliberation. (srj)
	*/
	virtual SleepyUpdatePhase getUpdatePhase() const
	{
		return PHASE_FINAL;
	}
public:

	ObjectWeaponStatusHelper( Thing *thing, const ModuleData *modData ) :
		ObjectHelper( thing, modData )
	{
		// unlike other helpers, this one needs to start active.
		DEBUG_ASSERTCRASH(getObject()->getTemplate()->canPossiblyHaveAnyWeapon(), ("should not be instantiated if we have no weapon"));
		setWakeFrame(getObject(), UPDATE_SLEEP_NONE);
	}
	// virtual destructor prototype provided by memory pool object

	virtual UpdateSleepTime update()
	{
		getObject()->adjustModelConditionForWeaponStatus();
		// unlike other helpers, this one must run every frame.
		return UPDATE_SLEEP_NONE;
	}
};
