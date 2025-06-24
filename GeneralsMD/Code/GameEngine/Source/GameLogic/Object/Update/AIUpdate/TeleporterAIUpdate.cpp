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

// TeleporterAIUpdate.cpp //////////
// Will give self random move commands
// Author: Graham Smallwood, April 2002

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/RandomValue.h"
#include "GameLogic/Module/TeleporterAIUpdate.h"
#include "GameLogic/Object.h"
#include "Common/Xfer.h"
#include "Common/DisabledTypes.h"
#include "GameClient/Drawable.h"
#include "GameClient/FXList.h"
#include "GameLogic/AI.h"
#include "GameLogic/AIPathfind.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Damage.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Weapon.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/TerrainLogic.h"


//-------------------------------------------------------------------------------------------------
TeleporterAIUpdateModuleData::TeleporterAIUpdateModuleData( void )
{

}

//-------------------------------------------------------------------------------------------------
/*static*/ void TeleporterAIUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	AIUpdateModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "MinDistanceForTeleport", INI::parseReal, NULL, offsetof(TeleporterAIUpdateModuleData, m_minDistance) },
		{ "DisabledDurationPerDistance", INI::parseDurationReal, NULL, offsetof(TeleporterAIUpdateModuleData, m_disabledDuration) },
		{ 0, 0, 0, 0 }
	};
	p.add(dataFieldParse);
}


//-------------------------------------------------------------------------------------------------
AIStateMachine* TeleporterAIUpdate::makeStateMachine()
{
	return newInstance(AIStateMachine)( getObject(), "TeleporterAIUpdateMachine");
}

//-------------------------------------------------------------------------------------------------
TeleporterAIUpdate::TeleporterAIUpdate( Thing *thing, const ModuleData* moduleData ) : AIUpdateInterface( thing, moduleData )
{

}

//-------------------------------------------------------------------------------------------------
TeleporterAIUpdate::~TeleporterAIUpdate( void )
{

}

//-------------------------------------------------------------------------------------------------
UpdateSleepTime TeleporterAIUpdate::update( void )
{
	//// If I'm standing still, move somewhere
	//if (isIdle())
	//{
	//	Coord3D dest = *(getObject()->getPosition());
	//	dest.x += GameLogicRandomValue( 5, 50 );
	//	dest.y += GameLogicRandomValue( 5, 50 );
 //		aiMoveToPosition( &dest, CMD_FROM_AI );
	//}

	// extend
	UpdateSleepTime ret = AIUpdateInterface::update();
	//return (mine < ret) ? mine : ret;
	/// @todo srj -- someday, make sleepy. for now, must not sleep.
	return ret; // UPDATE_SLEEP_NONE;
}  // end update


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

void TeleporterAIUpdate::doTeleport(Coord3D targetPos, Real angle, Real dist)
{
	const TeleporterAIUpdateModuleData* d = getTeleporterAIUpdateModuleData();
	Object* obj = getObject();

	// TheAI->pathfinder()->adjustTargetDestination(source, target, pos, this, &approachTargetPos);
	//TODO: Handle line of sight?!

	obj->setPosition(&targetPos);
	obj->setOrientation(angle);

	destroyPath();
	setLocomotorGoalPositionExplicit(targetPos);
	setLocomotorGoalOrientation(angle);

	UnsignedInt disabledFrame = TheGameLogic->getFrame() + REAL_TO_UNSIGNEDINT(dist * d->m_disabledDuration);

	obj->setDisabledUntil(DISABLED_TELEPORT, disabledFrame);

	//If we have a path, clear it?

}

//-------------------------------------------------------------------------------------------------

Bool TeleporterAIUpdate::findAttackLocation(Object* victim, const Coord3D* victimPos, Coord3D* targetPos, Real* targetAngle)
{
	Object* obj = getObject();
	Weapon* weap = obj->getCurrentWeapon();
	if (!weap)
		return false;

	bool viewBlocked = TheAI->pathfinder()->isAttackViewBlockedByObstacle(obj, *targetPos, victim, *victimPos);
	bool inRange = weap->isSourceObjectWithGoalPositionWithinAttackRange(obj, targetPos, victim, victimPos);

	Real RANGE_MARGIN = 10.0f;

	// If the unit's current distance is lower than the attack range, we try to keep this distance


	Real maxRange = weap->getAttackRange(obj) - RANGE_MARGIN;
	Real range = maxRange - weap->getTemplate()->getMinimumAttackRange();

	if (!viewBlocked && inRange) {
		DEBUG_LOG((">>> TPAI - findAttackLocation: done with initial pos\n"));
		return true;
	}

	// Calculate direction vector from victim to candidate position
	Coord3D dir;
	Real distSq = ThePartitionManager->getGoalDistanceSquared(obj, targetPos, victimPos, FROM_CENTER_2D, &dir);
	Real dist = sqrt(distSq);
	if (dist < maxRange) {
		maxRange = dist;
	}
	
	Coord2D direction;
	direction.x = -dir.x;
	direction.y = -dir.y;
	Real initAngle = atan2(direction.y, direction.x);  // angle from victim to target

	direction.normalize();
	//distance = sqrt(distSq);

	//if (distance > 0) {
	//	direction.x = (targetPos->x - targetPos->x) / distance;
	//	direction.y = (targetPos->y - targetPos->y) / distance;
	//}
	//else {
	//	// if we are directly at the target, but are not actually in range, something went wrong
	//	return false;
	//}

	// DEBUG:
	const FXList* debug_fx1 = TheFXListStore->findFXList("FX_DEBUG_MARKER_GREEN");
	const FXList* debug_fx2 = TheFXListStore->findFXList("FX_DEBUG_MARKER_RED");

	Coord3D newPos;
	newPos.x = targetPos->x;
	newPos.y = targetPos->y;
	newPos.z = targetPos->z;


	const Real maxAngle = deg2rad(180.0f);
	const Real step_size_angle = deg2rad(10.0f);
	const Real step_size_length = 15.0f;
	// const int max_steps = 500;

	const int max_rings = REAL_TO_INT(range / step_size_length);
	const int max_steps = REAL_TO_INT(maxAngle / step_size_angle);
	DEBUG_LOG((">>> TPAI - findAttackLocation: range = %f, max_rings = %d\n", range, max_rings));
	for (int ring = 0; ring < max_rings; ++ring) {

		Real radius = maxRange - (ring * step_size_length);

		for (int step = 0; step < max_steps; ++step) {
			int sign = (step % 2) ? 1 : -1;
			Real angle = initAngle + (step * sign * step_size_angle);

			//polar offset
			newPos.x = victimPos->x + radius * cos(angle);
			newPos.y = victimPos->y + radius * sin(angle);
			newPos.z = TheTerrainLogic->getGroundHeight(newPos.x, newPos.y);

			if (sign == 1)
				FXList::doFXPos(debug_fx1, &newPos);
			else
				FXList::doFXPos(debug_fx2, &newPos);

			viewBlocked = TheAI->pathfinder()->isAttackViewBlockedByObstacle(obj, newPos, victim, *victimPos);
			inRange = weap->isSourceObjectWithGoalPositionWithinAttackRange(obj, &newPos, victim, victimPos);

			if (!viewBlocked && inRange) {
				*targetPos = newPos;
				*targetAngle = angle + PI;
				DEBUG_LOG((">>> TPAI - findAttackLocation: done after ring=%d, step=%d\n", ring, step));

				return true;
			}
		}
	}

	DEBUG_LOG((">>> TPAI - findAttackLocation: failed to find attack position\n"));

	return false;
}

//-------------------------------------------------------------------------------------------------
UpdateSleepTime TeleporterAIUpdate::doLocomotor(void)
{
	if (!isMoving()) {
		return AIUpdateInterface::doLocomotor();
	}

	const TeleporterAIUpdateModuleData* d = getTeleporterAIUpdateModuleData();

	Object* obj = getObject();

	Object* goalObj = getGoalObject();
	const Coord3D* goalPos = getGoalPosition();

	Real requiredRange = 0;

	Coord3D targetPos;
	Coord3D dir;
	Real distSq;

	//Path* path = getPath();

	// Get TargetPos
	//if (isAttackPath() && (path != NULL)) {
	//if (path != NULL) {
	//	targetPos = *path->getFirstNode()->getPosition();
	//	DEBUG_LOG((">>> TPAI - doLoc: PATH pos (0) = %f, %f, %f\n", targetPos.x, targetPos.y, targetPos.z));
	//	distSq = ThePartitionManager->getDistanceSquared(obj, &targetPos, FROM_BOUNDINGSPHERE_2D, &dir);
	//}else
	if (goalObj != NULL) {
		targetPos = *goalObj->getPosition();
		DEBUG_LOG((">>> TPAI - doLoc: goalOBJPos (0) = %f, %f, %f\n", targetPos.x, targetPos.y, targetPos.z));
		distSq = ThePartitionManager->getDistanceSquared(obj, &targetPos, FROM_BOUNDINGSPHERE_2D, &dir);
	}
	else if (goalPos != NULL) {
		targetPos = *goalPos;
		DEBUG_LOG((">>> TPAI - doLoc: goalPOS (0) = %f, %f, %f\n", targetPos.x, targetPos.y, targetPos.z));
		distSq = ThePartitionManager->getDistanceSquared(obj, &targetPos, FROM_CENTER_2D, &dir);
	}
	else {
		return UPDATE_SLEEP_FOREVER;
	}

	Real targetAngle = atan2(dir.y, dir.x);
	dir.normalize();

	Real dist = sqrt(distSq);

	Real RANGE_MARGIN = 5.0f;   // We calculate distance this much shorter than weapon range
	Real TELEPORT_DIST_MARGIN = 5.0f;  // We teleport this much closer than needed


	DEBUG_LOG((">>> TPAI - doLoc: LocomotorGoalType = %d\n", getLocomotorGoalType()));

	// We are within min range
	if (dist <= d->m_minDistance) {
		return AIUpdateInterface::doLocomotor();
	}

	if (isAttacking()) {
		// requiredRange = obj->getLargestWeaponRange();
		Weapon* weap = obj->getCurrentWeapon();
		if (weap)
			requiredRange = weap->getAttackRange(obj) - RANGE_MARGIN;

		//Adjust target to required distance
		if (requiredRange > 0) {
			dir.scale(min(dist, requiredRange - TELEPORT_DIST_MARGIN));
			targetPos.sub(&dir);
			targetPos.z = TheTerrainLogic->getGroundHeight(targetPos.x, targetPos.y);
			
			//if (dist > requiredRange)
			//	dist -= requiredRange;
		}

		DEBUG_LOG((">>> TPAI - doLoc: findAttackLocation targetPos (BEFORE) = %f, %f, %f\n", targetPos.x, targetPos.y, targetPos.z));
		findAttackLocation(goalObj, goalPos, &targetPos, &targetAngle);
		DEBUG_LOG((">>> TPAI - doLoc: findAttackLocation targetPos (AFTER) = %f, %f, %f\n", targetPos.x, targetPos.y, targetPos.z));
		//recompute distance and angle
		distSq = ThePartitionManager->getDistanceSquared(obj, &targetPos, FROM_CENTER_2D, &dir);
		//targetAngle = atan2(dir.y, dir.x);
		dist = sqrt(distSq);

		DEBUG_LOG((">>> TPAI - doLoc: isAttacking, dist = %f, reqRange = %f\n", dist, requiredRange));
	}
	/*else {
		TheAI->pathfinder()->adjustToPossibleDestination(obj, getLocomotorSet(), &targetPos);
	}*/

	// We are in range already
	//if (dist <= requiredRange) {
	//	return AIUpdateInterface::doLocomotor();
	//}



	DEBUG_LOG((">>> TPAI - doLoc: teleport with dist = %f\n", dist));
	doTeleport(targetPos, targetAngle, dist);


	//DEBUG: Distance after Teleport
	//Coord3D targetPos_org;

	//if (goalObj != NULL) {
	//	targetPos_org = *goalObj->getPosition();
	//}
	//else if (goalPos != NULL) {
	//	targetPos_org = *goalPos;
	//}
	//distSq = ThePartitionManager->getDistanceSquared(obj, &targetPos_org, FROM_CENTER_2D);
	//dist = sqrt(distSq);

	//DEBUG_LOG((">>> TPAI - doLoc: goalPos(1) = %f, %f, %f\n", targetPos_org.x, targetPos_org.y, targetPos_org.z));
	//DEBUG_LOG((">>> TPAI - doLoc: distance after teleport = %f\n", dist));


	return AIUpdateInterface::doLocomotor();


}

//-------------------------------------------------------------------------------------------------
/**
 * See if we can do a quick path without pathfinding.
 */
Bool TeleporterAIUpdate::canComputeQuickPath(void)
{
	return true;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Bool TeleporterAIUpdate::computeQuickPath(const Coord3D* destination)
{
	return AIUpdateInterface::computeQuickPath(destination);
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void TeleporterAIUpdate::crc( Xfer *xfer )
{
	// extend base class
	AIUpdateInterface::crc(xfer);
}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void TeleporterAIUpdate::xfer( Xfer *xfer )
{
  XferVersion currentVersion = 1;
  XferVersion version = currentVersion;
  xfer->xferVersion( &version, currentVersion );
 
   // extend base class
   AIUpdateInterface::xfer(xfer);

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void TeleporterAIUpdate::loadPostProcess( void )
{
 // extend base class
	AIUpdateInterface::loadPostProcess();
}  // end loadPostProcess
