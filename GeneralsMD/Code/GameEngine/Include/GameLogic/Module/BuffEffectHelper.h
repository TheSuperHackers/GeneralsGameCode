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

// FILE: BuffEffectHelper.h ////////////////////////////////////////////////////////////////////////
// Author: Andi W, Oct 25
// Desc:   Buff Effect Helper - Tracks active buffs
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __BuffEffectHelper_H_
#define __BuffEffectHelper_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameLogic/Module/ObjectHelper.h"
#include "GameLogic/BuffSystem.h"


struct BuffEffectTracker
{
	const BuffTemplate* m_template;
	UnsignedInt m_frameCreated;
	UnsignedInt m_frameToRemove;
	UnsignedInt m_numStacks;
	ObjectID m_sourceID;
	Bool m_isActive;

	BuffEffectTracker() {
		m_template = NULL;
		m_frameCreated = 0;
		m_frameToRemove = 0;
		m_numStacks = 0;
		m_sourceID = INVALID_ID;
		m_isActive = FALSE;
	}
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class BuffEffectHelperModuleData : public ModuleData
{

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class BuffEffectHelper : public ObjectHelper
{

	  MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA(BuffEffectHelper, BuffEffectHelperModuleData)
		MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(BuffEffectHelper, "BuffEffectHelper")

public:

	BuffEffectHelper(Thing* thing, const ModuleData* modData);
	// virtual destructor prototype provided by memory pool object

	virtual DisabledMaskType getDisabledTypesToProcess() const { return DISABLEDMASK_ALL; }
	virtual UpdateSleepTime update();

	//void doTempWeaponBonus(WeaponBonusConditionType status, UnsignedInt duration, TintStatus tintStatus = TINT_STATUS_INVALID);

	void applyBuff(const BuffTemplate* buffTemplate, Object* sourceObj, UnsignedInt duration);

protected:
	//WeaponBonusConditionType m_currentBonus;
	//TintStatus m_currentTint;
	//UnsignedInt m_frameToRemove;
	//void clearTempWeaponBonus();

	std::vector<BuffEffectTracker> m_buffEffects;
	UnsignedInt m_nextTickFrame;

};


#endif  // end __BuffEffectHelper_H_
