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

// FILE: TempWeaponBonusHelper.h ////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, June 2003
// Desc:   Object helper - Clears Temporary weapon bonus effects
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/ObjectHelper.h"

enum WeaponBonusConditionType CPP_11(: Int);

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class TempWeaponBonusHelperModuleData : public ModuleData
{

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class TempWeaponBonusHelper : public ObjectHelper
{

	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( TempWeaponBonusHelper, TempWeaponBonusHelperModuleData )
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(TempWeaponBonusHelper, "TempWeaponBonusHelper" )

public:

	TempWeaponBonusHelper( Thing *thing, const ModuleData *modData );
	// virtual destructor prototype provided by memory pool object

	virtual DisabledMaskType getDisabledTypesToProcess() const { return DISABLEDMASK_ALL; }
	virtual UpdateSleepTime update();

	void doTempWeaponBonus( WeaponBonusConditionType status, UnsignedInt duration );

protected:
	WeaponBonusConditionType m_currentBonus;
	UnsignedInt m_frameToRemove;
	void clearTempWeaponBonus();
};
