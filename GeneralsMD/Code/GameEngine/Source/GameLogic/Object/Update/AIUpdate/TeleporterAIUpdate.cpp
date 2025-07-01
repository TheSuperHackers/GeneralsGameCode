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
#include "GameLogic/AIGuard.h"
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
	m_sourceFX = NULL;
	m_targetFX = NULL;
}

//-------------------------------------------------------------------------------------------------
/*static*/ void TeleporterAIUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	AIUpdateModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "MinDistanceForTeleport", INI::parseReal, NULL, offsetof(TeleporterAIUpdateModuleData, m_minDistance) },
		{ "DisabledDurationPerDistance", INI::parseDurationReal, NULL, offsetof(TeleporterAIUpdateModuleData, m_disabledDuration) },
		{ "TeleportStartFX", INI::parseFXList, NULL, offsetof(TeleporterAIUpdateModuleData, m_sourceFX) },
		{ "TeleportEndFX", INI::parseFXList, NULL, offsetof(TeleporterAIUpdateModuleData, m_targetFX) },
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
	//m_inAttackPos = FALSE;
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

UpdateSleepTime TeleporterAIUpdate::doTeleport(Coord3D targetPos, Real angle, Real dist)
{
	const TeleporterAIUpdateModuleData* d = getTeleporterAIUpdateModuleData();
	Object* obj = getObject();

	FXList::doFXObj(d->m_sourceFX, getObject());

	//TODO: Handle line of sight?!

	obj->setPosition(&targetPos);
	obj->setOrientation(angle);

	FXList::doFXObj(d->m_targetFX, getObject());

	destroyPath();
	//friend_endingMove();
	TheAI->pathfinder()->updateGoal(obj, &targetPos, TheTerrainLogic->getLayerForDestination(&targetPos));
	// setLocomotorGoalPositionExplicit(targetPos);
	setLocomotorGoalOrientation(angle);

	UnsignedInt disabledFrames = REAL_TO_UNSIGNEDINT(dist * d->m_disabledDuration);

	obj->setDisabledUntil(DISABLED_TELEPORT, TheGameLogic->getFrame() + disabledFrames);

	return UPDATE_SLEEP(disabledFrames);

}

//-------------------------------------------------------------------------------------------------
Bool TeleporterAIUpdate::isLocationValid(Object* obj, const Coord3D* targetPos, Object* victim, const Coord3D* victimPos, Weapon* weap)
{
	bool viewBlocked = TheAI->pathfinder()->isAttackViewBlockedByObstacle(obj, *targetPos, victim, *victimPos);
	bool inRange = weap->isSourceObjectWithGoalPositionWithinAttackRange(obj, targetPos, victim, victimPos);
	PathfindLayerEnum destinationLayer = TheTerrainLogic->getLayerForDestination(targetPos);
	bool posValid = TheAI->pathfinder()->validMovementPosition(getObject()->getCrusherLevel() > 0, destinationLayer, getLocomotorSet(), targetPos);

	return !viewBlocked && inRange && posValid;
}


Bool TeleporterAIUpdate::findAttackLocation(Object* victim, const Coord3D* victimPos, Coord3D* targetPos, Real* targetAngle)
{
	Object* obj = getObject();
	Weapon* weap = obj->getCurrentWeapon();
	if (!weap)
		return false;

	Coord3D newPos;
	newPos.x = targetPos->x;
	newPos.y = targetPos->y;
	newPos.z = targetPos->z;

	// Check if the current location is valid.
	// This needs to be rechecked after the disabled timer.
	if (isLocationValid(obj, targetPos, victim, victimPos, weap)) {

		// After verifying the initial location
		//if (!m_inAttackPos) { // Only adjust before a teleport
			if (!TheAI->pathfinder()->adjustDestination(obj, getLocomotorSet(), &newPos)) {
				DEBUG_LOG((">>> TPAI - findAttackLocation: AdjustDestination failed!\n"));
			}
			else {
				DEBUG_LOG((">>> TPAI - findAttackLocation: AdjustDestination: %f, %f, %f\n", newPos.x, newPos.y, newPos.z));
			}

			if (isLocationValid(obj, &newPos, victim, victimPos, weap)) {
				DEBUG_LOG((">>> TPAI - findAttackLocation: done with initial pos\n"));
				*targetPos = newPos;
				return true;
			}
		//}
		//else {
		//	DEBUG_LOG((">>> TPAI - findAttackLocation: done with initial pos\n"));
		//	*targetPos = newPos;
		//	return true;
		//}
	}

	newPos.x = targetPos->x;
	newPos.y = targetPos->y;
	newPos.z = targetPos->z;

	//bool viewBlocked = TheAI->pathfinder()->isAttackViewBlockedByObstacle(obj, *targetPos, victim, *victimPos);
	//bool inRange = weap->isSourceObjectWithGoalPositionWithinAttackRange(obj, targetPos, victim, victimPos);
	//PathfindLayerEnum destinationLayer = TheTerrainLogic->getLayerForDestination(targetPos);
	//bool posValid = TheAI->pathfinder()->validMovementPosition(getObject()->getCrusherLevel() > 0, destinationLayer, getLocomotorSet(), targetPos);

	Real RANGE_MARGIN = 10.0f;

	// If the unit's current distance is lower than the attack range, we try to keep this distance


	Real maxRange = weap->getAttackRange(obj) - RANGE_MARGIN;
	Real range = maxRange - weap->getTemplate()->getMinimumAttackRange();


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
	/*const FXList* debug_fx1 = TheFXListStore->findFXList("FX_DEBUG_MARKER_GREEN");
	const FXList* debug_fx2 = TheFXListStore->findFXList("FX_DEBUG_MARKER_RED");*/


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

			//viewBlocked = TheAI->pathfinder()->isAttackViewBlockedByObstacle(obj, newPos, victim, *victimPos);
			//inRange = weap->isSourceObjectWithGoalPositionWithinAttackRange(obj, &newPos, victim, victimPos);
			//destinationLayer = TheTerrainLogic->getLayerForDestination(&newPos);
			//posValid = TheAI->pathfinder()->validMovementPosition(getObject()->getCrusherLevel() > 0, destinationLayer, getLocomotorSet(), &newPos);

			DEBUG_LOG((">>> TPAI - findAttackLocation: candidate Pos: %f, %f, %f\n", newPos.x, newPos.y, newPos.z));

			// TheAI->pathfinder()->adjustTargetDestination(obj, victim, victimPos, weap, &newPos);
			if (!TheAI->pathfinder()->adjustDestination(obj, getLocomotorSet(), &newPos)) {
				DEBUG_LOG((">>> TPAI - findAttackLocation: AdjustDestination failed!\n"));
			}
			else {
				DEBUG_LOG((">>> TPAI - findAttackLocation: AdjustDestination: %f, %f, %f\n", newPos.x, newPos.y, newPos.z));
			}

			/*if (sign == 1)
				FXList::doFXPos(debug_fx1, &newPos);
			else
				FXList::doFXPos(debug_fx2, &newPos);*/

			if (isLocationValid(obj, &newPos, victim, victimPos, weap)) {
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

	// TODO: Check states
	// - (generic) Moving
	// - Attacking
	// - Guard
	// -- GuardAttack
	// -- Move to Object
	// - Enter

	//Path* path = getPath();

	// Get TargetPos

	if (goalObj != NULL) {
		targetPos = *goalObj->getPosition();
		//goalPos = targetPos;  //This should be the same anyways
		DEBUG_LOG((">>> TPAI - doLoc: goalOBJPos (0) = %f, %f, %f\n", targetPos.x, targetPos.y, targetPos.z));
		distSq = ThePartitionManager->getDistanceSquared(obj, &targetPos, FROM_BOUNDINGSPHERE_2D, &dir);
	}
	else if (goalPos != NULL && !(goalPos->x == 0 && goalPos->y == 0 && goalPos->z == 0)) {
		targetPos = *goalPos;
		DEBUG_LOG((">>> TPAI - doLoc: goalPOS (0) = %f, %f, %f\n", targetPos.x, targetPos.y, targetPos.z));
		distSq = ThePartitionManager->getDistanceSquared(obj, &targetPos, FROM_CENTER_2D, &dir);
	}
	//else if (getGuardLocation() != NULL && !(getGuardLocation()->x == 0 && getGuardLocation()->y == 0 && getGuardLocation()->z == 0)) {  // getStateMachine()->isInGuardIdleState()
	//	targetPos = *getGuardLocation();
	//	TheAI->pathfinder()->adjustDestination(obj, getLocomotorSet(), &targetPos);
	//	distSq = ThePartitionManager->getDistanceSquared(obj, &targetPos, FROM_CENTER_2D, &dir);
	//	DEBUG_LOG((">>> TPAI - doLoc: goalPos GUARD (0) = %f, %f, %f\n", targetPos.x, targetPos.y, targetPos.z));
	//	if (getStateMachine()->isInGuardIdleState()) {
	//		requiredRange = 25.0f; // Allow extra range to give some room for large groups guarding
	//	}
	else if (getStateMachine()->getCurrentStateID() == AI_GUARD) {
		if (isAttacking()) {
			AIGuardMachine* guardMachine = getStateMachine()->getGuardMachine();
			if (guardMachine != NULL) {
				ObjectID nemID = guardMachine->getNemesisID();
				if (nemID != INVALID_ID) {
					Object* nemesis = TheGameLogic->findObjectByID(nemID);
					if (nemesis != NULL) {
						goalObj = nemesis;
						goalPos = goalObj->getPosition();
						targetPos = *goalPos;
						
						DEBUG_LOG((">>> TPAI - doLoc: goalPos GUARD NEMESIS (0) = %f, %f, %f\n", targetPos.x, targetPos.y, targetPos.z));
						distSq = ThePartitionManager->getDistanceSquared(obj, &targetPos, FROM_BOUNDINGSPHERE_2D, &dir);
					}
				}
			}
		}
		else if (getGuardLocation() != NULL && !(getGuardLocation()->x == 0 && getGuardLocation()->y == 0 && getGuardLocation()->z == 0)) {  // getStateMachine()->isInGuardIdleState()
			targetPos = *getGuardLocation();
			TheAI->pathfinder()->adjustDestination(obj, getLocomotorSet(), &targetPos);
			distSq = ThePartitionManager->getDistanceSquared(obj, &targetPos, FROM_CENTER_2D, &dir);
			DEBUG_LOG((">>> TPAI - doLoc: goalPos GUARD (0) = %f, %f, %f\n", targetPos.x, targetPos.y, targetPos.z));
			if (getStateMachine()->isInGuardIdleState()) {
				requiredRange = 25.0f; // Allow extra range to give some room for large groups guarding
			}
		}
	}
	//else if (isAttackPath() && (path != NULL)) {
	//	targetPos = *path->getFirstNode()->getPosition();
	//	DEBUG_LOG((">>> TPAI - doLoc: PATH pos (0) = %f, %f, %f\n", targetPos.x, targetPos.y, targetPos.z));
	//	distSq = ThePartitionManager->getDistanceSquared(obj, &targetPos, FROM_BOUNDINGSPHERE_2D, &dir);
	else {
		DEBUG_LOG((">>> TPAI - doLoc: GOAL POS AND OBJ ARE NULL??\n"));
		return UPDATE_SLEEP_FOREVER;
	}

	Real targetAngle = atan2(dir.y, dir.x);
	dir.normalize();

	Real dist = sqrt(distSq);

	Real RANGE_MARGIN = 5.0f;   // We calculate distance this much shorter than weapon range
	Real TELEPORT_DIST_MARGIN = 5.0f;  // We teleport this much closer than needed


	DEBUG_LOG((">>> TPAI - doLoc: LocomotorGoalType = %d, AI STATE = %s (%d)\n", getLocomotorGoalType(), getStateMachine()->getCurrentStateName(), getStateMachine()->getCurrentStateID()));

	// We are within min range
	if (dist <= d->m_minDistance || dist <= requiredRange) {
		return AIUpdateInterface::doLocomotor();
	}

	// TODO: IF object is already in attacking position and can fire, do not adjust any positions?!
	// But still check if position is valid!

	if (isAttacking()) {
		// requiredRange = obj->getLargestWeaponRange();
		Weapon* weap = obj->getCurrentWeapon();
		if (!weap)
			return AIUpdateInterface::doLocomotor();

		// Check if current position is valid for attack
		if (isLocationValid(obj, obj->getPosition(), goalObj, goalPos, weap)) {
			return AIUpdateInterface::doLocomotor();
		}

		requiredRange = weap->getAttackRange(obj) - RANGE_MARGIN;

		//Adjust target to required distance
		if (requiredRange > 0) {
			dir.scale(min(dist, requiredRange - TELEPORT_DIST_MARGIN));
			targetPos.sub(&dir);
			targetPos.z = TheTerrainLogic->getGroundHeight(targetPos.x, targetPos.y);
		}

		// Find proper attack position for adjusted target
		if (!findAttackLocation(goalObj, goalPos, &targetPos, &targetAngle)) {
			DEBUG_LOG((">>> TPAI - doLoc: isAttacking. FAILED TO FIND VALID LOCATION!\n"));

			// This might happen if we try to attack e.g. a boat in water
			// TODO: Should we move as close as we can?

			return AIUpdateInterface::doLocomotor();
		}
		//DEBUG_LOG((">>> TPAI - doLoc: findAttackLocation targetPos (AFTER) = %f, %f, %f\n", targetPos.x, targetPos.y, targetPos.z));

		//recompute distance and angle
		distSq = ThePartitionManager->getDistanceSquared(obj, &targetPos, FROM_CENTER_2D, &dir);
		//targetAngle = atan2(dir.y, dir.x);
		dist = sqrt(distSq);

		DEBUG_LOG((">>> TPAI - doLoc: isAttacking, dist = %f, reqRange = %f\n", dist, requiredRange));
		//m_inAttackPos = TRUE;
	}
	/*else {
		m_inAttackPos = FALSE;
	}*/
	


	DEBUG_LOG((">>> TPAI - doLoc: teleport with dist = %f\n", dist));
	doTeleport(targetPos, targetAngle, dist);

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

   //xfer->xferBool(&m_inAttackPos);

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void TeleporterAIUpdate::loadPostProcess( void )
{
 // extend base class
	AIUpdateInterface::loadPostProcess();
}  // end loadPostProcess
