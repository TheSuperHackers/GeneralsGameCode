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

Bool TeleporterAIUpdate::findAttackLocation(Object* victim, Coord3D* victimPos, Coord3D* targetPos)
{
	Object* obj = getObject();
	Weapon* weap = obj->getCurrentWeapon();
	if (!weap)
		return False;

	bool viewBlocked;
	bool inRange;

	Coord3D newPos = *targetPos;

	// TODO: use adjustTargetDestination, or replicate it (with included line of sight check)

	while (TRUE) {

		// Maybe we can use a simpler check here?
		viewBlocked = TheAI->pathfinder()->isAttackViewBlockedByObstacle(obj, *targetPos, victim, *victimPos);
		inRange = weap->isSourceObjectWithGoalPositionWithinAttackRange(obj, targetPos, victim, victimPos);

		if (!viewBlocked && inRange) {
			targetPos* = newPos;
			return true;
		}
	}
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
			requiredRange = weap->getAttackRange(obj) -RANGE_MARGIN;

		//Adjust target to required distance
		if (requiredRange > 0) {
			dir.scale(requiredRange - TELEPORT_DIST_MARGIN);
			targetPos.sub(&dir);
			targetPos.z = TheTerrainLogic->getGroundHeight(targetPos.x, targetPos.y);
			dist -= requiredRange;
		}

		// TODO: Compute path from the adjusted position

		DEBUG_LOG((">>> TPAI - doLoc: isAttacking, dist = %f, reqRange = %f\n", dist, requiredRange));
	}
	else {
		TheAI->pathfinder()->adjustToPossibleDestination(obj, getLocomotorSet(), &targetPos);
	}

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
