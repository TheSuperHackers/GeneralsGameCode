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

// TeleporterAIUpdate.h //////////
// Will give self random move commands
// Author: Graham Smallwood, April 2002
 
#pragma once

#ifndef _TELEPORTER_AI_UPDATE_H_
#define _TELEPORTER_AI_UPDATE_H_

#include "GameLogic/Module/AIUpdate.h"

class FXList;


//-------------------------------------------------------------------------------------------------
class TeleporterAIUpdateModuleData : public AIUpdateModuleData
{
public:
	Real m_minDistance;
	Real m_disabledDuration;

	const FXList* m_sourceFX;
	const FXList* m_targetFX;
	const FXList* m_recoverEndFX;

	AudioEventRTS m_recoverSoundLoop;

	TintStatus	m_tintStatus;  ///< tint color to apply when recovering from teleport

	Real m_opacityStart;
	Real m_opacityEnd;

	TeleporterAIUpdateModuleData();

	static void buildFieldParse(MultiIniFieldParse& p);

private:

};
// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
class TeleporterAIUpdate : public AIUpdateInterface
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( TeleporterAIUpdate, "TeleporterAIUpdate" )
	MAKE_STANDARD_MODULE_MACRO_WITH_MODULE_DATA(TeleporterAIUpdate, TeleporterAIUpdateModuleData)

	/*
		IMPORTANT NOTE: if you ever add module data to this, you must have it inherit from
		AIUpdateModuleData to allow locomotors to work correctly. (see SupplyTruckAIUpdate
		for an example.)
	*/

public:

	TeleporterAIUpdate( Thing *thing, const ModuleData* moduleData );
	// virtual destructor prototype provided by memory pool declaration

	virtual UpdateSleepTime update();

	/// this is never disabled, since we want disabled things to continue recovering from teleport
	virtual DisabledMaskType getDisabledTypesToProcess() const { return DISABLEDMASK_ALL; }

protected:

	UpdateSleepTime doTeleport(Coord3D targetPos, Real angle, Real dist);

	Bool findAttackLocation(Object* victim, const Coord3D* victimPos, Coord3D* targetPos, Real* targetAngle);

	Bool isLocationValid(Object* obj, const Coord3D* targetPos, Object* victim, const Coord3D* victimPos, Weapon* weap);

	virtual UpdateSleepTime doLocomotor();

	//virtual Bool getTreatAsAircraftForLocoDistToGoal() const;

	//virtual void chooseGoodLocomotorFromCurrentSet();

	//virtual void doPathfind(PathfindServicesInterface* pathfinder);
	//virtual void requestPath(Coord3D* destination, Bool isGoalDestination);	///< Queues a request to pathfind to destination.
	//virtual void requestAttackPath(ObjectID victimID, const Coord3D* victimPos);	///< computes path to attack the current target, returns false if no path
	//virtual void requestApproachPath(Coord3D* destination);	///< computes path to attack the current target, returns false if no path
	//virtual void requestSafePath(ObjectID repulsor1);	///< computes path to attack the current target, returns false if no path

	virtual Bool canComputeQuickPath(void); ///< Returns true if we can quickly comput a path.  Usually missiles & the like that just move straight to the destination.
	virtual Bool computeQuickPath(const Coord3D* destination); ///< Computes a quick path to the destination.


	 virtual AIStateMachine* makeStateMachine();

private:
	void applyRecoverEffects(Real dist);
	void removeRecoverEffects(void);

	AudioEventRTS m_recoverSoundLoop;  ///< Audio to play during recovering
	UnsignedInt m_disabledUntil;  ///< frame we are done recovering
	UnsignedInt m_disabledStart;  ///< frame we have started recovering
	bool m_isDisabled;      ///< current recovering status
};

#endif

