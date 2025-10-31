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
#include "GameClient/ParticleSys.h"


//enum ParticleSystemID CPP_11(: Int);


struct BuffEffectTracker
{
	const BuffTemplate* m_template;
	UnsignedInt m_frameCreated;
	UnsignedInt m_frameToRemove;
	UnsignedInt m_numStacks;
	ObjectID m_sourceID;
	Bool m_isActive;
	std::vector<ParticleSystemID> m_particleSystemIDs;

	BuffEffectTracker() {
		m_template = NULL;
		m_frameCreated = 0;
		m_frameToRemove = 0;
		m_numStacks = 0;
		m_sourceID = INVALID_ID;
		m_isActive = FALSE;
		m_particleSystemIDs.clear();
	}

	void addParticleSystem(ParticleSystemID id) {
		m_particleSystemIDs.push_back(id);
	}

	void clearParticleSystems() {
		DEBUG_LOG(("BuffEffectTracker::clearParticleSystems - count = %d", m_particleSystemIDs.size()));
		for (ParticleSystemID id : m_particleSystemIDs) {
			DEBUG_LOG(("BuffEffectTracker::clearParticleSystems - try to remove system %d", id));
			ParticleSystem* particleSystem = TheParticleSystemManager->findParticleSystem(id);
			if (particleSystem) {
				particleSystem->stop();
				particleSystem->destroy();
			}
			else {
				DEBUG_LOG(("BuffEffectTracker::clearParticleSystems - sys not found!"));
			}
		}
		m_particleSystemIDs.clear();
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

	void clearAllBuffs();
};


#endif  // end __BuffEffectHelper_H_
