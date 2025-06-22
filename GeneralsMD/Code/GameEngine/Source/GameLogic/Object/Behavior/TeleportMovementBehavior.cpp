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

// FILE: TeleportMovementBehavior.cpp /////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood, July 2002
// Desc:   Behavior that reacts to poison Damage by continuously damaging us further in an Update
///////////////////////////////////////////////////////////////////////////////////////////////////


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/Xfer.h"
#include "Common/DisabledTypes.h"
#include "GameClient/Drawable.h"
#include "GameLogic/Module/TeleportMovementBehavior.h"
#include "GameLogic/AI.h"
#include "GameLogic/AIPathfind.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Damage.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/TerrainLogic.h"


//-------------------------------------------------------------------------------------------------
TeleportMovementBehaviorModuleData::TeleportMovementBehaviorModuleData()
{

}

//-------------------------------------------------------------------------------------------------
/*static*/ void TeleportMovementBehaviorModuleData::buildFieldParse(MultiIniFieldParse& p)
{

	static const FieldParse dataFieldParse[] =
	{
		{ "MinDistanceForTeleport", INI::parseReal, NULL, offsetof(TeleportMovementBehaviorModuleData, m_minDistance) },
		{ "DisabledDurationPerDistance", INI::parseDurationReal, NULL, offsetof(TeleportMovementBehaviorModuleData, m_disabledDuration) },
		{ 0, 0, 0, 0 }
	};

	UpdateModuleData::buildFieldParse(p);
	p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
TeleportMovementBehavior::TeleportMovementBehavior(Thing* thing, const ModuleData* moduleData) : UpdateModule(thing, moduleData)
{

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
TeleportMovementBehavior::~TeleportMovementBehavior(void)
{
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

void TeleportMovementBehavior::doTeleport(Coord3D targetPos, Real angle, Real dist)
{
	const TeleportMovementBehaviorModuleData* d = getTeleportMovementBehaviorModuleData();
	Object* obj = getObject();

	obj->setPosition(&targetPos);
	obj->setOrientation(angle);


	UnsignedInt disabledFrame = TheGameLogic->getFrame() + REAL_TO_UNSIGNEDINT(dist * d->m_disabledDuration);

	obj->setDisabledUntil(DISABLED_PARALYZED, disabledFrame);

	//If we have a path, clear it?

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
UpdateSleepTime TeleportMovementBehavior::update()
{
	const TeleportMovementBehaviorModuleData* d = getTeleportMovementBehaviorModuleData();

	Object* obj = getObject();

	AIUpdateInterface* ai = obj->getAI();
	if (!ai)
		return UPDATE_SLEEP_FOREVER;

	if (ai->isMoving()) {
		Object* goalObj = ai->getGoalObject();
		const Coord3D* goalPos = ai->getGoalPosition();

		Real requiredRange = 0;

		Coord3D targetPos;

		// Get TargetPos
		if (goalObj != NULL) {
			targetPos = *goalObj->getPosition();
		}
		else if(goalPos != NULL) {
			targetPos = *goalPos;
		}

		Coord3D dir;
		Real distSq = ThePartitionManager->getDistanceSquared(obj, &targetPos, FROM_CENTER_2D, &dir);
		Real targetAngle = atan2(dir.y, dir.x);
		dir.normalize();

		Real dist = sqrt(distSq);

		if (ai->isAttacking()) {
			requiredRange = obj->getLargestWeaponRange();
		}

		// We are in range already
		if (dist <= requiredRange || dist <= d->m_minDistance) {
			return UPDATE_SLEEP_NONE;
		}

		//Adjust target to required distance
		if (requiredRange > 0) {
			dir.scale(requiredRange);
			targetPos.sub(&dir);
			targetPos.z = TheTerrainLogic->getGroundHeight(targetPos.x, targetPos.y);
		}

		doTeleport(targetPos, targetAngle, dist);

	}

	return UPDATE_SLEEP_NONE;

}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void TeleportMovementBehavior::crc(Xfer* xfer)
{

	// extend base class
	UpdateModule::crc(xfer);

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
	// ------------------------------------------------------------------------------------------------
void TeleportMovementBehavior::xfer(Xfer* xfer)
{

	// version 
	const XferVersion currentVersion = 2;
	XferVersion version = currentVersion;
	xfer->xferVersion(&version, currentVersion);

	// extend base class
	UpdateModule::xfer(xfer);

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void TeleportMovementBehavior::loadPostProcess(void)
{

	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
