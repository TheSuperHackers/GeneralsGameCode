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

// FILE: FiringTracker.h //////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, February 2002
// Desc:   Keeps track of shots fired and people targeted for weapons that want a history of such a thing
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef FIRING_TRACKER_H
#define FIRING_TRACKER_H

#include "Common/GameType.h"
#include "Common/GameMemory.h"
#include "Common/AudioEventRTS.h"
#include "GameLogic/Module/UpdateModule.h"

class Object;
class Weapon;

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class FiringTrackerModuleData : public ModuleData
{

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class FiringTracker : public UpdateModule
{
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA( FiringTracker, FiringTrackerModuleData )
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(FiringTracker, "FiringTrackerPool" )
public:
	FiringTracker(Thing *thing, const ModuleData *modData);
	void shotFired(const Weapon* weaponFired, ObjectID victimID );			///< Owner just fired this weapon at this Object
	ObjectID getLastShotVictim() const { return m_victimID; }						///< get the last victim ID that was shot at
	Int getNumConsecutiveShotsAtVictim( const Object *victim ) const;

	/// this is never disabled, since we want disabled things to continue to slowly "spin down"... (srj)
	virtual DisabledMaskType getDisabledTypesToProcess() const { return DISABLEDMASK_ALL; }

	virtual UpdateSleepTime update();	///< See if spin down is needed because we haven't shot in a while

protected:

	/*
		The firingtracker needs to have its update run after all "normal"
		user update modules, so it redefines this. Please don't redefine this
		for other modules without very careful deliberation. (srj)
	*/
	virtual SleepyUpdatePhase getUpdatePhase() const
	{
		return PHASE_FINAL;
	}

private:

	void speedUp();		///< I've qualified for an increase in my Object flag status
	void coolDown();	///< I need to slow down because it has been too long since I fired.
	UpdateSleepTime calcTimeToSleep();

private:
	Int							m_consecutiveShots;					///< How many times I have shot at the same thing
	ObjectID				m_victimID;									///< The thing I have shot so many times
	UnsignedInt			m_frameToStartCooldown;			///< This is the frame I should cool down at, and is pushed back every time a shot is fired
	UnsignedInt			m_frameToForceReload;				///< Even more than AutoReload, this means it will pre-emptively reload instead of event triggering a delay after the last shot

	UnsignedInt			m_frameToStopLoopingSound;	///< if sound is looping, frame to stop looping it (or zero if not looping)
	AudioHandle			m_audioHandle;
};

#endif

