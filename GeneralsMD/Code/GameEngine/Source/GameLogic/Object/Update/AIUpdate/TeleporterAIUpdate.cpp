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

#include "Common/GameAudio.h"
#include "Common/RandomValue.h"
#include "GameLogic/Module/TeleporterAIUpdate.h"
#include "GameLogic/Object.h"
#include "Common/Xfer.h"
#include "Common/DisabledTypes.h"
#include "Common/ModelState.h"
#include "GameClient/Drawable.h"
#include "GameClient/FXList.h"
#include "GameLogic/AI.h"
#include "GameLogic/AIGuard.h"
#include "GameLogic/AIGuardRetaliate.h"
#include "GameLogic/AIPathfind.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Damage.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Weapon.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/TerrainLogic.h"
#include "GameClient/TintStatus.h"


//-------------------------------------------------------------------------------------------------
TeleporterAIUpdateModuleData::TeleporterAIUpdateModuleData( void )
{
	m_sourceFX = NULL;
	m_targetFX = NULL;
	m_recoverEndFX = NULL;
	m_tintStatus = TINT_STATUS_INVALID;
	m_opacityStart = 1.0;
	m_opacityEnd = 1.0;
}

//-------------------------------------------------------------------------------------------------
/*static*/ void TeleporterAIUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	AIUpdateModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "MinDistanceForTeleport", INI::parseReal, NULL, offsetof(TeleporterAIUpdateModuleData, m_minDistance) },
		{ "MinDisabledDuration", INI::parseDurationReal, NULL, offsetof(TeleporterAIUpdateModuleData, m_minDisabledDuration) },
		{ "DisabledDurationPerDistance", INI::parseDurationReal, NULL, offsetof(TeleporterAIUpdateModuleData, m_disabledDuration) },
		{ "TeleportStartFX", INI::parseFXList, NULL, offsetof(TeleporterAIUpdateModuleData, m_sourceFX) },
		{ "TeleportTargetFX", INI::parseFXList, NULL, offsetof(TeleporterAIUpdateModuleData, m_targetFX) },
		{ "TeleportRecoverEndFX", INI::parseFXList, NULL, offsetof(TeleporterAIUpdateModuleData, m_recoverEndFX) },
		{ "TeleportRecoverSoundAmbient", INI::parseAudioEventRTS, NULL, offsetof(TeleporterAIUpdateModuleData, m_recoverSoundLoop) },
		{ "TeleportRecoverTint", TintStatusFlags::parseSingleBitFromINI, NULL, offsetof(TeleporterAIUpdateModuleData, m_tintStatus) },
		{ "TeleportRecoverOpacityStart", INI::parsePercentToReal, NULL, offsetof(TeleporterAIUpdateModuleData, m_opacityStart) },
		{ "TeleportRecoverOpacityEnd", INI::parsePercentToReal, NULL, offsetof(TeleporterAIUpdateModuleData, m_opacityEnd) },
		{ 0, 0, 0, 0 }
	};
	p.add(dataFieldParse);
}

static void createDebugFX(const Coord3D* pos, const char* name) {
	const FXList* debug_fx1 = TheFXListStore->findFXList(name);
	FXList::doFXPos(debug_fx1, pos);
}


//-------------------------------------------------------------------------------------------------
AIStateMachine* TeleporterAIUpdate::makeStateMachine()
{
	return newInstance(AIStateMachine)( getObject(), "TeleporterAIUpdateMachine");
}

//-------------------------------------------------------------------------------------------------
TeleporterAIUpdate::TeleporterAIUpdate( Thing *thing, const ModuleData* moduleData ) : AIUpdateInterface( thing, moduleData )
{
	m_disabledUntil = 0;
	m_disabledStart = 0;
	m_isDisabled = false;
}

//-------------------------------------------------------------------------------------------------
TeleporterAIUpdate::~TeleporterAIUpdate( void )
{

}

//-------------------------------------------------------------------------------------------------
UpdateSleepTime TeleporterAIUpdate::update(void)
{
	const TeleporterAIUpdateModuleData* d = getTeleporterAIUpdateModuleData();
	Object* obj = getObject();

	//UpdateSleepTime ret = UPDATE_SLEEP_FOREVER;

	UnsignedInt now = TheGameLogic->getFrame();

	if (m_isDisabled) {
		if (m_disabledUntil > now) {
			// We are currently disabled
			Real progress = __max(__min(INT_TO_REAL(now - m_disabledStart) / INT_TO_REAL(m_disabledUntil - m_disabledStart), 1.0), 0.0);

			Drawable* draw = obj->getDrawable();
			if (draw)
			{
				// - set opacity
				if (d->m_opacityStart < 1.0f || d->m_opacityEnd < 1.0f) {
					Real opacity = (1.0 - progress) * d->m_opacityStart + progress * d->m_opacityEnd;
					//DEBUG_LOG((">>> TPAI Update: progress = %f, opacity = %f.", progress, opacity));
					draw->setDrawableOpacity(opacity);
					//draw->setEffectiveOpacity(opacity);
					//draw->setSecondMaterialPassOpacity(opacity);
				}
			}
			// We actually need to stop here, because the default update would allow us to attack while disabled
			return UPDATE_SLEEP_NONE;
			//ret = UPDATE_SLEEP_NONE;
		}
		else {
			// We are done
			removeRecoverEffects();
			m_isDisabled = false;
		}
	}

	// extend
	// UpdateSleepTime ret2 = AIUpdateInterface::update();
	// return (ret < ret2) ? ret : ret2;

	return AIUpdateInterface::update();

}  // end update


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void TeleporterAIUpdate::applyRecoverEffects(Real dist)
{
	const TeleporterAIUpdateModuleData* d = getTeleporterAIUpdateModuleData();
	Object* obj = getObject();

	// - set conditionstate
	obj->setModelConditionState(MODELCONDITION_TELEPORT_RECOVER);

	// - add ambient sound
	m_recoverSoundLoop = d->m_recoverSoundLoop;
	m_recoverSoundLoop.setObjectID(obj->getID());
	m_recoverSoundLoop.setPlayingHandle(TheAudio->addAudioEvent(&m_recoverSoundLoop));
	
	Drawable* draw = obj->getDrawable();
	if (draw)
	{
		// - set color tint
		if (d->m_tintStatus > TINT_STATUS_INVALID && d->m_tintStatus < TINT_STATUS_COUNT)
		{
			draw->setTintStatus(d->m_tintStatus);
		}

		// - set opacity
		if (d->m_opacityStart < 1.0) {
			//draw->setEffectiveOpacity(1.0);
			//draw->setSecondMaterialPassOpacity(1.0);
			draw->setDrawableOpacity(d->m_opacityStart);
			DEBUG_LOG((">>> TPAI applyRecoverEffects - set opacity %f", d->m_opacityStart));
		}

	}

}

// ------------------------------------------------------------------------------------------------
void TeleporterAIUpdate::removeRecoverEffects()
{
	const TeleporterAIUpdateModuleData* d = getTeleporterAIUpdateModuleData();
	Object* obj = getObject();

	obj->clearModelConditionState(MODELCONDITION_TELEPORT_RECOVER);

	TheAudio->removeAudioEvent(m_recoverSoundLoop.getPlayingHandle());

	Drawable* draw = obj->getDrawable();
	if (draw)
	{
		// - clear color tint
		if (d->m_tintStatus > TINT_STATUS_INVALID && d->m_tintStatus < TINT_STATUS_COUNT)
		{
			draw->clearTintStatus(d->m_tintStatus);
		}

		// - set opacity
		if (d->m_opacityStart < 1.0 || d->m_opacityEnd < 1.0) {
			//draw->setEffectiveOpacity(1.0);
			//draw->setSecondMaterialPassOpacity(1.0);
			draw->setDrawableOpacity(1.0);
			DEBUG_LOG((">>> TPAI removeRecoverEffects - restore Opacity!\n"));
		}
	}

	FXList::doFXObj(d->m_recoverEndFX, getObject());
}
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

UpdateSleepTime TeleporterAIUpdate::doTeleport(Coord3D targetPos, Real angle, Real dist)
{
	const TeleporterAIUpdateModuleData* d = getTeleporterAIUpdateModuleData();
	Object* obj = getObject();

	FXList::doFXObj(d->m_sourceFX, getObject());

	obj->setPosition(&targetPos);
	obj->setOrientation(angle);

	FXList::doFXObj(d->m_targetFX, getObject());

	destroyPath();

	TheAI->pathfinder()->updateGoal(obj, &targetPos, TheTerrainLogic->getLayerForDestination(&targetPos));
	setLocomotorGoalOrientation(angle);

	UnsignedInt disabledFrames = REAL_TO_UNSIGNEDINT((dist * d->m_disabledDuration) + d->m_minDisabledDuration);

	m_disabledStart = TheGameLogic->getFrame();
	m_disabledUntil = m_disabledStart + disabledFrames;

	m_isDisabled = true;
	obj->setDisabledUntil(DISABLED_TELEPORT, m_disabledUntil);

	applyRecoverEffects(dist);

	// return UPDATE_SLEEP(disabledFrames);
	return UPDATE_SLEEP_NONE;  // We can't actually sleep since we need to adjust some things dynamically

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

//-------------------------------------------------------------------------------------------------
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
		if (!TheAI->pathfinder()->adjustDestination(obj, getLocomotorSet(), &newPos)) {
			DEBUG_LOG((">>> TPAI - findAttackLocation: AdjustDestination failed!\n"));
		}
		//else {
		//	DEBUG_LOG((">>> TPAI - findAttackLocation: AdjustDestination: %f, %f, %f\n", newPos.x, newPos.y, newPos.z));
		//}

		if (isLocationValid(obj, &newPos, victim, victimPos, weap)) {
			// DEBUG_LOG((">>> TPAI - findAttackLocation: done with initial pos\n"));
			*targetPos = newPos;
			return true;
		}
	}

	newPos.x = targetPos->x;
	newPos.y = targetPos->y;
	newPos.z = targetPos->z;


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

	const Real maxAngle = deg2rad(180.0f);
	const Real step_size_angle = deg2rad(10.0f);
	const Real step_size_length = 15.0f;
	// const int max_steps = 500;

	const int max_rings = REAL_TO_INT(range / step_size_length);
	const int max_steps = REAL_TO_INT(maxAngle / step_size_angle);
	// DEBUG_LOG((">>> TPAI - findAttackLocation: range = %f, max_rings = %d\n", range, max_rings));
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

			// DEBUG_LOG((">>> TPAI - findAttackLocation: candidate Pos: %f, %f, %f\n", newPos.x, newPos.y, newPos.z));

			// TheAI->pathfinder()->adjustTargetDestination(obj, victim, victimPos, weap, &newPos);
			if (!TheAI->pathfinder()->adjustDestination(obj, getLocomotorSet(), &newPos)) {
				DEBUG_LOG((">>> TPAI - findAttackLocation: AdjustDestination failed!\n"));
			}
			//else {
			//	DEBUG_LOG((">>> TPAI - findAttackLocation: AdjustDestination: %f, %f, %f\n", newPos.x, newPos.y, newPos.z));
			//}

			/*if (sign == 1)
				FXList::doFXPos(debug_fx1, &newPos);
			else
				FXList::doFXPos(debug_fx2, &newPos);*/

			if (isLocationValid(obj, &newPos, victim, victimPos, weap)) {
				*targetPos = newPos;
				*targetAngle = angle + PI;
				//DEBUG_LOG((">>> TPAI - findAttackLocation: done after ring=%d, step=%d\n", ring, step));

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

	//TODO: RETALIATE!

	// TODO: Check states
	// - (generic) Moving
	// - Attacking
	// - Guard
	// -- GuardAttack
	// -- Move to Object
	// - Enter

	//Path* path = getPath();

	// Get TargetPos

	bool useWeaponRange = false;  // We need this for retaliate state, where isAttacking is false

	if (goalObj != NULL) {
		targetPos = *goalObj->getPosition();
		//goalPos = targetPos;  //This should be the same anyways
		//DEBUG_LOG((">>> TPAI - doLoc: goalOBJPos (0) = %f, %f, %f\n", targetPos.x, targetPos.y, targetPos.z));
		//if (isAttacking())
			distSq = ThePartitionManager->getDistanceSquared(obj, goalObj, FROM_BOUNDINGSPHERE_2D, &dir);
		//else
		//	distSq = ThePartitionManager->getDistanceSquared(obj, &targetPos, FROM_CENTER_2D, &dir);
	}
	else if (goalPos != NULL && !(goalPos->x == 0 && goalPos->y == 0 && goalPos->z == 0)) {
		targetPos = *goalPos;
		//DEBUG_LOG((">>> TPAI - doLoc: goalPOS (0) = %f, %f, %f\n", targetPos.x, targetPos.y, targetPos.z));
		distSq = ThePartitionManager->getDistanceSquared(obj, &targetPos, FROM_CENTER_2D, &dir);
	}
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
						
						//DEBUG_LOG((">>> TPAI - doLoc: goalPos GUARD NEMESIS (0) = %f, %f, %f\n", targetPos.x, targetPos.y, targetPos.z));
						//distSq = ThePartitionManager->getDistanceSquared(obj, &targetPos, FROM_BOUNDINGSPHERE_2D, &dir);
						distSq = ThePartitionManager->getDistanceSquared(obj, goalObj, FROM_BOUNDINGSPHERE_2D, &dir);
					}
				}
			}
		}
		else if (getGuardLocation() != NULL && !(getGuardLocation()->x == 0 && getGuardLocation()->y == 0 && getGuardLocation()->z == 0)) {  // getStateMachine()->isInGuardIdleState()
			targetPos = *getGuardLocation();
			//TheAI->pathfinder()->adjustDestination(obj, getLocomotorSet(), &targetPos);
			distSq = ThePartitionManager->getDistanceSquared(obj, &targetPos, FROM_CENTER_2D, &dir);
			// DEBUG_LOG((">>> TPAI - doLoc: distSq = %f - goalPos GUARD (0) = %f, %f, %f", distSq, targetPos.x, targetPos.y, targetPos.z));
			if (getStateMachine()->isInGuardIdleState()) {
				requiredRange = 25.0f; // Allow extra range to give some room for large groups guarding
			}
			//createDebugFX(&targetPos, "FX_DEBUG_MARKER_RED");
			//createDebugFX(obj->getPosition(), "FX_DEBUG_MARKER_GREEN");
		}
		//DEBUG_LOG((">>> TPAI - doLoc: AI_GUARD - distSq = %f, targetPos = {%f, %f, %f}, isAttacking = %d, GuardIdle = %d",
		//	distSq, targetPos.x, targetPos.y, targetPos.z, isAttacking(), getStateMachine()->isInGuardIdleState()));
		//createDebugFX(&targetPos, "FX_DEBUG_MARKER_GREEN");
	}
	else {
		DEBUG_LOG((">>> TPAI - doLoc: GOAL POS AND OBJ ARE NULL??\n"));
		return UPDATE_SLEEP_FOREVER;
	}

	if (getStateMachine()->getCurrentStateID() == AI_ENTER) {
		// If we want to enter and got this close, we just move normally
		requiredRange = 15.0f;
	//} else if (getStateMachine()->getCurrentStateID() == AI_DOCK) {
	//	// Get the dock's approach position.
	//	// If we are at least X distance away, teleport, otherwise, do normal movement
	//	DockUpdateInterface* dock = goalObj->getDockUpdateInterface();
	//	if (dock != NULL) {
	//		int dockIndex;  // we don't really need this
	//		Bool reserved = dock->reserveApproachPosition(obj, &targetPos, &dockIndex);
	//		if (reserved) {
	//			// Get dist to goal obj center
	//			Real distSqObj = ThePartitionManager->getDistanceSquared(obj, goalObj, FROM_CENTER_2D, &dir);
	//			// Get dist to approach pos
	//			distSq = ThePartitionManager->getDistanceSquared(obj, &targetPos, FROM_CENTER_2D, &dir);

	//			DEBUG_LOG((">>> TPAI: DOCK distSq = %f, distSqObj = %f\n", distSq, distSqObj));

	//			// If we are close to both the approach pos and the center pos, move normally
	//			Real minDistSq = 25.0f * 25.0f;
	//			if (distSqObj < minDistSq && distSqObj < minDistSq) {
	//				return AIUpdateInterface::doLocomotor();
	//			}
	//			// otherwise teleport
	//		}
	//	}
	}else if (getStateMachine()->getCurrentStateID() == AI_GUARD_RETALIATE) {
		/*DEBUG_LOG((">>> TPAI - doLoc: AI_GUARD_RETALIATE - distSq = %f, targetPos = {%f, %f, %f}, isAttacking = %d, GuardIdle = %d",
			distSq, targetPos.x, targetPos.y, targetPos.z, isAttacking(), getStateMachine()->isInGuardIdleState()));*/
		
		AIGuardRetaliateMachine* guardRetaliateMachine = getStateMachine()->getGuardRetaliateMachine();
		if (guardRetaliateMachine != NULL) {
			ObjectID nemID = guardRetaliateMachine->getNemesisID();
			if (nemID != INVALID_ID) {
				Object* nemesis = TheGameLogic->findObjectByID(nemID);
				if (nemesis != NULL) {
					goalObj = nemesis;
					goalPos = goalObj->getPosition();
					targetPos = *goalPos;

					// DEBUG_LOG((">>> TPAI - doLoc: goalPos GUARD_RETALIATE NEMESIS (0) = %f, %f, %f\n", targetPos.x, targetPos.y, targetPos.z));
					
					useWeaponRange = true;

					distSq = ThePartitionManager->getDistanceSquared(obj, goalObj, FROM_BOUNDINGSPHERE_2D, &dir);
				}
			}
		}
	}

	DEBUG_LOG((">>> TPAI - doLoc: LocomotorGoalType = %d, AI STATE = %s (%d)\n", getLocomotorGoalType(), getStateMachine()->getCurrentStateName(), getStateMachine()->getCurrentStateID()));

	Real RANGE_MARGIN = 5.0f;   // We calculate distance this much shorter than weapon range
	Real TELEPORT_DIST_MARGIN = 5.0f;  // We teleport this much closer than needed

	// Get initial dist and dir
	// distSq = ThePartitionManager->getDistanceSquared(obj, &targetPos, FROM_CENTER_2D, &dir);
	Real dist = sqrt(distSq);
	Real targetAngle = atan2(dir.y, dir.x);
	dir.normalize();

	// We are within min range
	if (dist <= d->m_minDistance || dist <= requiredRange) {
		return AIUpdateInterface::doLocomotor();
	}

	//When we attack, we attempt to teleport into range
	if (isAttacking() || useWeaponRange) {
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
		////targetAngle = atan2(dir.y, dir.x);
		dist = sqrt(distSq);

		//DEBUG_LOG((">>> TPAI - doLoc: isAttacking, dist = %f, reqRange = %f\n", dist, requiredRange));
		//m_inAttackPos = TRUE;
	}
	//else if( /*use special power?*/) {
	//	//same as with attacks, try to get into range
	//}
	// else if (getStateMachine()->getCurrentStateID() == AI_ENTER || getStateMachine()->getCurrentStateID() == AI_ENTER) {
	else if (goalObj != NULL) {
		// We need to correct the position to the outer bounding box of the structure
		// TODO: Respect actual geometry, not just radius
		requiredRange = goalObj->getGeometryInfo().getBoundingCircleRadius();
		if (requiredRange > 0) {
			dir.scale(min(dist, requiredRange));
			targetPos.sub(&dir);
			targetPos.z = TheTerrainLogic->getGroundHeight(targetPos.x, targetPos.y);
		}
		TheAI->pathfinder()->adjustDestination(obj, getLocomotorSet(), &targetPos);

		//recompute distance and angle
		distSq = ThePartitionManager->getDistanceSquared(obj, &targetPos, FROM_CENTER_2D, &dir);

		// targetAngle = atan2(dir.y, dir.x);
		targetAngle = atan2(goalPos->y - targetPos.y, goalPos->x - targetPos.x);
		dist = sqrt(distSq);
	}
	else {
		// TODO: if this doesn't find a location, 
		TheAI->pathfinder()->adjustDestination(obj, getLocomotorSet(), &targetPos);

		//recompute distance and angle
		distSq = ThePartitionManager->getDistanceSquared(obj, &targetPos, FROM_CENTER_2D, &dir);
		targetAngle = atan2(dir.y, dir.x);
		dist = sqrt(distSq);

		// We are within min range (Not sure why, but we need to check this again here)
		if (dist <= d->m_minDistance || dist <= requiredRange) {
			return AIUpdateInterface::doLocomotor();
		}

		//DEBUG_LOG((">>> TPAI - doLoc: after AI_GUARD - distSq = %f, targetPos = {%f, %f, %f}, isAttacking = %d, GuardIdle = %d",
		//	distSq, targetPos.x, targetPos.y, targetPos.z, isAttacking(), getStateMachine()->isInGuardIdleState()));
	}

	// DEBUG_LOG((">>> TPAI - doLoc: teleport with dist = %f\n", dist));
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

   xfer->xferBool(&m_isDisabled);

   xfer->xferUnsignedInt(&m_disabledUntil);
   xfer->xferUnsignedInt(&m_disabledStart);

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void TeleporterAIUpdate::loadPostProcess( void )
{
 // extend base class
	AIUpdateInterface::loadPostProcess();
}  // end loadPostProcess
