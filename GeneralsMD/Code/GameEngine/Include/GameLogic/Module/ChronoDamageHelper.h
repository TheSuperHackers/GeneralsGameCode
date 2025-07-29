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

// FILE: ChronoDamageHelper.h ////////////////////////////////////////////////////////////////////////
// Author: Andi W, July 2025
// Desc:   Object helper - Clears chrono disable status and heals chrono damage since Body modules can't have Updates
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __ChronoDamageHelper_H_
#define __ChronoDamageHelper_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/ObjectHelper.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class ChronoDamageHelperModuleData : public ModuleData
{

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class ChronoDamageHelper : public ObjectHelper
{

	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( ChronoDamageHelper, ChronoDamageHelperModuleData )
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(ChronoDamageHelper, "ChronoDamageHelper" )	

public:

	ChronoDamageHelper( Thing *thing, const ModuleData *modData );
	// virtual destructor prototype provided by memory pool object

	virtual DisabledMaskType getDisabledTypesToProcess() const { return DISABLEDMASK_ALL; }
	virtual UpdateSleepTime update();

	void notifyChronoDamage( Real amount );

protected:
	UnsignedInt m_healingStepCountdown;
};


#endif  // end __ChronoDamageHelper_H_
