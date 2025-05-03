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

// FILE: ScatterShotUpdate.cpp ////////////////////////////////////////////////////////////////////
// Author: Andi W, April 2025
// Desc:   Projectile Scatter/Airburst module
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#define DEFINE_DEATH_NAMES

#include "Common/Xfer.h"
#include "Common/PlayerList.h"
#include "Common/Player.h"
#include "GameLogic/Object.h"
#include "GameLogic/Damage.h"
#include "GameLogic/Module/ScatterShotUpdate.h"
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/Module/StealthUpdate.h"
#include "GameLogic/WeaponStatus.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/WeaponSet.h"
#include "GameLogic/TerrainLogic.h"
#include "GameClient/FXList.h"

//-------------------------------------------------------------------------------------------------
ScatterShotUpdateModuleData::ScatterShotUpdateModuleData()
{
	m_weaponTemplate = NULL;
	m_numShots = 0;
	m_targetSearchRadius = 0;
	m_targetMinRadius = 0;
	m_maxShotsPerTarget = 0;
	m_preferSimilarTargets = false;
	m_preferNearestTargets = false;
	m_attackGroundWhenNoTargets = true;
	m_noTargetsScatterRadius = 0;

	m_triggerDistanceToTarget = 0;
	//m_triggerDistanceFromSource = 0;
	m_triggerDistancePercent = 0;
	m_triggerLifetime = 0;
	m_triggerOnImpact = false;
	m_triggerInstantly = false;
	m_stayAliveAfterTrigger = false;

	m_triggerDeathType = DEATH_NORMAL;
	m_scatterFX = NULL;
}


//-------------------------------------------------------------------------------------------------
/*static*/ void ScatterShotUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	UpdateModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "Weapon",	INI::parseWeaponTemplate, NULL, offsetof(ScatterShotUpdateModuleData, m_weaponTemplate) },
		{ "NumShots", INI::parseUnsignedInt, NULL, offsetof(ScatterShotUpdateModuleData, m_numShots) },
		{ "TargetSearchRadius", INI::parseReal, NULL, offsetof(ScatterShotUpdateModuleData, m_targetSearchRadius) },
		{ "TargetMinRadius", INI::parseReal, NULL, offsetof(ScatterShotUpdateModuleData, m_targetMinRadius) },
		{ "MaxShotsPerTarget", INI::parseUnsignedInt, NULL, offsetof(ScatterShotUpdateModuleData, m_maxShotsPerTarget) },
		{ "PreferSimilarTargets", INI::parseBool, NULL, offsetof(ScatterShotUpdateModuleData, m_preferSimilarTargets) },
		{ "PreferNearestTargets", INI::parseBool, NULL, offsetof(ScatterShotUpdateModuleData, m_preferNearestTargets) },
		{ "NoTargetsScatterRadius", INI::parseReal, NULL, offsetof(ScatterShotUpdateModuleData, m_noTargetsScatterRadius) },
		{ "AttackGroundWhenNoTargets", INI::parseBool, NULL, offsetof(ScatterShotUpdateModuleData, m_attackGroundWhenNoTargets) },

		{ "TriggerDistanceToTarget", INI::parseReal, NULL, offsetof(ScatterShotUpdateModuleData, m_triggerDistanceToTarget) },
		//{ "TriggerDistanceFromSource", INI::parseReal, NULL, offsetof(ScatterShotUpdateModuleData, m_triggerDistanceFromSource) },
		{ "TriggerDistancePercent", INI::parsePercentToReal, NULL, offsetof(ScatterShotUpdateModuleData, m_triggerDistancePercent) },
		{ "TriggerLifetime", INI::parseDurationUnsignedInt, NULL, offsetof(ScatterShotUpdateModuleData, m_triggerLifetime) },
		{ "TriggerOnImpact", INI::parseBool, NULL, offsetof(ScatterShotUpdateModuleData, m_triggerOnImpact) },
		{ "TriggerInstantly", INI::parseBool, NULL, offsetof(ScatterShotUpdateModuleData, m_triggerInstantly) },
		{ "StayAliveAfterTrigger", INI::parseBool, NULL, offsetof(ScatterShotUpdateModuleData, m_stayAliveAfterTrigger) },

		{ "TriggeredDeathType", INI::parseIndexList, TheDeathNames, offsetof(ScatterShotUpdateModuleData, m_triggerDeathType) },
		{ "ScatterFX", INI::parseFXList, NULL, offsetof(ScatterShotUpdateModuleData, m_scatterFX) },

		//{ "InitialDelay", INI::parseDurationUnsignedInt, NULL, offsetof(ScatterShotUpdateModuleData, m_initialDelayFrames) },
		//{ "ExclusiveWeaponDelay", INI::parseDurationUnsignedInt, NULL, offsetof(ScatterShotUpdateModuleData, m_exclusiveWeaponDelay) },
		{ 0, 0, 0, 0 }
	};
	p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ScatterShotUpdate::ScatterShotUpdate(Thing* thing, const ModuleData* moduleData) :
	UpdateModule(thing, moduleData),
	m_weapon(NULL)
{
	const ScatterShotUpdateModuleData* data = getScatterShotUpdateModuleData();

	const WeaponTemplate* tmpl = data->m_weaponTemplate;
	if (tmpl)
	{
		m_weapon = TheWeaponStore->allocateNewWeapon(tmpl, PRIMARY_WEAPON);
		m_weapon->loadAmmoNow(getObject());
	}

	m_goalObj = NULL;
	//m_goalPos.clear();

	m_totalTargetDistance = 0;

	m_hasTriggered = false;
	m_isInitialized = false;

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ScatterShotUpdate::~ScatterShotUpdate(void)
{
	if (m_weapon)
		m_weapon->deleteInstance();
	m_goalObj = NULL;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime ScatterShotUpdate::update(void)
{
	// DEBUG_LOG((">>> SSU Update 1\n"));
	const ScatterShotUpdateModuleData* data = getScatterShotUpdateModuleData();

	if (!m_isInitialized) {
		ProjectileUpdateInterface* pui = NULL;
		for (BehaviorModule** u = getObject()->getBehaviorModules(); *u; ++u)
		{
			if ((pui = (*u)->getProjectileUpdateInterface()) != NULL)
				break;
		}
		if (pui) {
			if (pui->projectileGetLauncherID() != 0 && pui->projectileIsArmed()) {
				const Coord3D* pos = pui->getTargetPosition();
				m_goalPos.x = pos->x;
				m_goalPos.y = pos->y;
				m_goalPos.z = pos->z;

				m_goalObj = pui->getTargetObject();

				//m_weapon->setBonusRefObjID(pui->projectileGetLauncherID());

				// We propagate the Weapon bonus from the launcher to the projectile
				Object* launcher = NULL;
				ObjectID launcherID = pui->projectileGetLauncherID();
				if (launcherID != INVALID_ID) {
					launcher = TheGameLogic->findObjectByID(launcherID);
				}
				if (launcher) {
					// Copy weapon bonus flags from the launcher to this projectile
					WeaponBonusConditionFlags bonusFlags = launcher->getWeaponBonusCondition();
					getObject()->setWeaponBonusConditionFlags(bonusFlags);
				}
				if (data->m_triggerDistancePercent > 0) {
					m_totalTargetDistance = getTargetDistance();
				}
			}
			else {
				// We are not launched yet.
				return UPDATE_SLEEP_NONE;
			}
		} // We are not a projectile at all. We scatter in place
		else {
			const Coord3D* pos = getObject()->getPosition();
			m_goalPos.x = pos->x;
			m_goalPos.y = pos->y;
			m_goalPos.z = pos->z;
		}

		if (data->m_triggerLifetime > 0) {
			m_initialDelayFrame = TheGameLogic->getFrame() + data->m_triggerLifetime;
		}

		m_isInitialized = true;
	}

	if (m_hasTriggered) {
		// DEBUG_LOG((">>> SSU Update 2 - hasTriggered\n"));
		return UPDATE_SLEEP_FOREVER;
	}

	// Instant Trigger
	if (data->m_triggerInstantly) {
		triggerScatterShot();
		return UPDATE_SLEEP_NONE;
	}

	// Lifetime Trigger
	if (data->m_triggerLifetime > 0 && TheGameLogic->getFrame() >= m_initialDelayFrame) {
		/*DEBUG_LOG((">>> SSU Update - lifetimeTrigger: m_triggerLifetime = %d, now = %d, m_initialDelayFrame = %d.\n",
			data->m_triggerLifetime, TheGameLogic->getFrame(), m_initialDelayFrame));*/
		triggerScatterShot();
		return UPDATE_SLEEP_NONE;
	}

	// Distance to target Trigger
	if (data->m_triggerDistancePercent > 0 || data->m_triggerDistanceToTarget > 0) {

		Real targetDistance = getTargetDistance();

		if (data->m_triggerDistanceToTarget > 0 && targetDistance < data->m_triggerDistanceToTarget) {
			triggerScatterShot();

			// DEBUG_LOG((">>> SSU Update - after triggerScatterShot\n"));

			return UPDATE_SLEEP_NONE;
		}

		if (data->m_triggerDistancePercent > 0 &&
			m_totalTargetDistance > 0 &&
			(targetDistance / m_totalTargetDistance) > data->m_triggerDistancePercent) {
			triggerScatterShot();
			return UPDATE_SLEEP_NONE;
		}

	}

	// Note: Disabled; I don't think this is a use case at all
	// Distance from source Trigger
	//if (data->m_triggerDistanceFromSource > 0) {
	//}

	return UPDATE_SLEEP_NONE;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

void ScatterShotUpdate::triggerScatterShot(void)
{
	// DEBUG_LOG((">>> SSU - triggerScatterShot 1\n"));
	m_hasTriggered = TRUE;

	const ScatterShotUpdateModuleData* data = getScatterShotUpdateModuleData();

	const Coord3D* pos;
	if (m_goalObj != NULL) {
		pos = m_goalObj->getPosition();
	}
	else {
		pos = &m_goalPos;
	}

	FXList::doFXObj(data->m_scatterFX, getObject());

	// Just check this when looping
	// 
	//if (!(m_weapon->getTemplate()->getAntiMask() & WEAPON_ANTI_GROUND)) {
	//	PartitionFilterRejectByObjectStatus	filterStatus(MAKE_OBJECT_STATUS_MASK(OBJECT_STATUS_AIRBORNE_TARGET), OBJECT_STATUS_MASK_NONE);
	//	filters[numFilters++] = &filterStatus;
	//}
	//if (!(m_weapon->getTemplate()->getAntiMask() & WEAPON_ANTI_AIRBORNE_VEHICLE)) {
	//	else {
	//		PartitionFilterRejectByObjectStatus	filterStatus(MAKE_OBJECT_STATUS_MASK(OBJECT_STATUS_AIRBORNE_TARGET), OBJECT_STATUS_MASK_NONE);
	//	}
	//}

	//Prefer similar targets

	int shotsLeft = data->m_numShots;
	int shotsLeftPerTarget = data->m_maxShotsPerTarget;

	// DEBUG_LOG((">>> SSU - triggerScatterShot 2\n"));

	//WeaponSet weaponSet(m_weapon);

	// DEBUG_LOG((">>> SSU - triggerScatterShot 3\n"));

	// We always try to fire at the Original weapon's target first
	bool mainTargetShot = FALSE;
	if (shotsLeftPerTarget > 0 && shotsLeft > 0 && data->m_targetMinRadius <= 0) {
		if ((m_goalObj) && isValidTarget(m_goalObj)) {
			// DEBUG_LOG((">>> SSU - fireWeapon at target Obj\n"));
			m_weapon->fireWeapon(getObject(), m_goalObj);
			// Bool status = m_weapon->fireWeapon(getObject(), m_goalObj);
			// DEBUG_LOG((">>> SSU - fireWeapon success = %s\n", status ? "true" : "false"));
			//m_weapon->loadAmmoNow(getObject());
			//weaponSet.reloadAllAmmo(getObject(), true);
			shotsLeft--;
			mainTargetShot = TRUE;
		}
	}

	// If we have shots left (and shots per target), look for targets in range
	if (shotsLeftPerTarget > 0 && shotsLeft > 0 && data->m_targetSearchRadius > 0) {
		IterOrderType orderType = ITER_FASTEST;
		if (data->m_preferNearestTargets) orderType = ITER_SORTED_NEAR_TO_FAR;

		PartitionFilterRelationship relationship(getObject(), PartitionFilterRelationship::ALLOW_ENEMIES);
		PartitionFilterSameMapStatus filterMapStatus(getObject());
		PartitionFilterAlive filterAlive;

		PartitionFilter* filters[8];
		Int numFilters = 0;
		filters[numFilters++] = &relationship;
		filters[numFilters++] = &filterAlive;
		filters[numFilters++] = &filterMapStatus;
		// if preferSimilar is set, we either get ONLY AIR, or ONLY GROUND targets
		if (data->m_preferSimilarTargets && m_goalObj != NULL) {
			if (m_goalObj->isAirborneTarget()) {
				PartitionFilterAcceptByObjectStatus	filterStatus(MAKE_OBJECT_STATUS_MASK(OBJECT_STATUS_AIRBORNE_TARGET), OBJECT_STATUS_MASK_NONE);
				filters[numFilters++] = &filterStatus;
			}
			else {
				PartitionFilterRejectByObjectStatus	filterStatus(MAKE_OBJECT_STATUS_MASK(OBJECT_STATUS_AIRBORNE_TARGET), OBJECT_STATUS_MASK_NONE);
				filters[numFilters++] = &filterStatus;
			}
		}
		filters[numFilters] = NULL;

		ObjectIterator* iter = ThePartitionManager->iterateObjectsInRange(
			pos, data->m_targetSearchRadius,
			FROM_CENTER_3D, filters, orderType);   //2D or 3D search radius??

		// Iterate over targets
		while (shotsLeftPerTarget > 0 && shotsLeft > 0) {

			for (Object* obj = iter->first(); obj; obj = iter->next())
			{
				if (shotsLeft == 0) break;

				if (obj == m_goalObj && mainTargetShot) continue;

				if (data->m_targetMinRadius > 0 && sqrt(ThePartitionManager->getDistanceSquared(getObject(), obj->getPosition(), FROM_CENTER_3D)) <= data->m_targetMinRadius) continue;

				if (isValidTarget(obj)) {
					// DEBUG_LOG((">>> SSU - fireWeapon at target Obj\n"));
					m_weapon->fireWeapon(getObject(), obj);
					// Bool status = m_weapon->fireWeapon(getObject(), obj);
					// DEBUG_LOG((">>> SSU - fireWeapon success = %s\n", status ? "true" : "false"));
					//m_weapon->loadAmmoNow(getObject());
					//weaponSet.reloadAllAmmo(getObject(), true);
					shotsLeft--;
				}
			}
			mainTargetShot = FALSE; // We only care in the first run
			shotsLeftPerTarget--;
		}
	}

	// DEBUG_LOG((">>> SSU - triggerScatterShot 4\n"));

	// Each leftover shot is fired randomly to the ground
	while (shotsLeft > 0) {
		Coord3D targetPos;
		targetPos.x = pos->x;
		targetPos.y = pos->y;

		Real scatterRadius = GameLogicRandomValueReal(data->m_targetMinRadius, data->m_noTargetsScatterRadius);
		Real scatterAngleRadian = GameLogicRandomValueReal(0, 2 * PI);

		Coord3D firingOffset;
		firingOffset.zero();
		firingOffset.x = scatterRadius * Cos(scatterAngleRadian);
		firingOffset.y = scatterRadius * Sin(scatterAngleRadian);

		targetPos.x += firingOffset.x;
		targetPos.y += firingOffset.y;
		targetPos.z = TheTerrainLogic->getGroundHeight(targetPos.x, targetPos.y);

		const Coord3D* constTargetPos = &targetPos;

		//DEBUG_LOG((">>> SSU - fireWeapon at position\n"));
		m_weapon->fireWeapon(getObject(), constTargetPos);
		//Bool status = m_weapon->fireWeapon(getObject(), constTargetPos);
		//DEBUG_LOG((">>> SSU - fireWeapon success = %s\n", status ? "true" : "false"));
		//m_weapon->loadAmmoNow(getObject());
		//weaponSet.reloadAllAmmo(getObject(), true);
		shotsLeft--;
	}

	// DEBUG_LOG((">>> SSU - triggerScatterShot 5\n"));

	// kill the object after scattering
	if (!data->m_stayAliveAfterTrigger) {
		getObject()->kill(DAMAGE_UNRESISTABLE, data->m_triggerDeathType);
	}

	//if (m_weapon) {
	//	m_weapon->deleteInstance();
	//	m_weapon = NULL;
	//}


	// DEBUG_LOG((">>> SSU - triggerScatterShot 6\n"));

	//m_weapon->forceFireWeapon(getObject(), getObject()->getPosition());

}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Real ScatterShotUpdate::getTargetDistance(void) const {
	Coord3D goal;
	Coord3D pos = *getObject()->getPosition();
	if (m_goalObj != NULL) {
		goal = *m_goalObj->getPosition();
	}
	else {
		goal = m_goalPos;
	}
	Coord3D delta;
	delta.x = pos.x - goal.x;
	delta.y = pos.y - goal.y;
	delta.z = pos.z - goal.z;
	return delta.length();
}

//-------------------------------------------------------------------------------------------------
// Copied from WeaponSet.cpp
//-------------------------------------------------------------------------------------------------
static Int getVictimAntiMask(const Object* victim)
{
	if (victim->isKindOf(KINDOF_SMALL_MISSILE))
	{
		//All missiles are also projectiles!
		return WEAPON_ANTI_SMALL_MISSILE;
	}
	else if (victim->isKindOf(KINDOF_BALLISTIC_MISSILE))
	{
		return WEAPON_ANTI_BALLISTIC_MISSILE;
	}
	else if (victim->isKindOf(KINDOF_PROJECTILE))
	{
		return WEAPON_ANTI_PROJECTILE;
	}
	else if (victim->isKindOf(KINDOF_MINE) || victim->isKindOf(KINDOF_DEMOTRAP))
	{
		return WEAPON_ANTI_MINE | WEAPON_ANTI_GROUND;
	}
	else if (victim->isAirborneTarget())
	{
		if (victim->isKindOf(KINDOF_VEHICLE))
		{
			return WEAPON_ANTI_AIRBORNE_VEHICLE;
		}
		else if (victim->isKindOf(KINDOF_INFANTRY))
		{
			return WEAPON_ANTI_AIRBORNE_INFANTRY;
		}
		else if (victim->isKindOf(KINDOF_PARACHUTE))
		{
			return WEAPON_ANTI_PARACHUTE;
		}
		else if (!victim->isKindOf(KINDOF_UNATTACKABLE))
		{
			DEBUG_CRASH(("Object %s is being targetted as airborne, but is not infantry, nor vehicle. Is this legit? -- tell Kris", victim->getTemplate()->getName().str()));
		}
		return 0;
	}
	else
	{
		return WEAPON_ANTI_GROUND;
	}
}
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

Bool ScatterShotUpdate::isValidTarget(const Object* victim) const
{
	const Object* source = getObject();
	// basic sanity checks.
	if (!source ||
		!victim ||
		source->isEffectivelyDead() ||
		victim->isEffectivelyDead() ||
		source->isDestroyed() ||
		victim->isDestroyed() ||
		victim == source)
		return FALSE;

	if (victim->testStatus(OBJECT_STATUS_MASKED))
		return FALSE;

	if (victim->isKindOf(KINDOF_UNATTACKABLE))
		return FALSE;

	// this object is not currently auto-acquireable
	if (victim->testStatus(OBJECT_STATUS_NO_ATTACK_FROM_AI))
		return FALSE;

	Bool allowStealthToPreventAttacks = TRUE;
	// If an object is stealthed and hasn't been detected yet, then it is not a valid target to fire 
	// on.
	if (allowStealthToPreventAttacks &&
		victim->testStatus(OBJECT_STATUS_STEALTHED) &&
		!victim->testStatus(OBJECT_STATUS_DETECTED))
	{
		if (!victim->isKindOf(KINDOF_DISGUISER))
		{
			return FALSE;;
		}
		else
		{
			//Exception case -- don't return false if we are a bomb truck disguised as an enemy vehicle.
			StealthUpdate* update = victim->getStealth();
			if (update && update->isDisguised())
			{
				Player* ourPlayer = source->getControllingPlayer();
				Player* otherPlayer = ThePlayerList->getNthPlayer(update->getDisguisedPlayerIndex());
				if (ourPlayer && otherPlayer)
				{
					if (ourPlayer->getRelationship(otherPlayer->getDefaultTeam()) != ENEMIES)
					{
						//Our stealthed & undetected object is disguised as a unit not perceived to be our enemy.
						return FALSE;;
					}
				}
			}
		}
	}

	// if the victim is contained within an enclosing container, it cannot be attacked directly
	const Object* victimsContainer = victim->getContainedBy();
	if (victimsContainer != NULL && victimsContainer->getContain()->isEnclosingContainerFor(victim) == TRUE)
	{
		return FALSE;
	}

	if (!(m_weapon->getAntiMask() & getVictimAntiMask(victim))) {
		return FALSE;
	}

	return TRUE;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void ScatterShotUpdate::crc(Xfer* xfer)
{
	// extend base class
	UpdateModule::crc(xfer);

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
	// ------------------------------------------------------------------------------------------------
void ScatterShotUpdate::xfer(Xfer* xfer)
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion(&version, currentVersion);

	// extend base class
	UpdateModule::xfer(xfer);

	// weapon
	xfer->xferSnapshot(m_weapon);

	// lifetime
	xfer->xferUnsignedInt(&m_initialDelayFrame);

	// has triggered
	xfer->xferBool(&m_hasTriggered);

	//is initialized
	xfer->xferBool(&m_isInitialized);

	// target pos
	xfer->xferCoord3D(&m_goalPos);

	// target distance
	xfer->xferReal(&m_totalTargetDistance);

	// target obj
	if (xfer->getXferMode() == XFER_SAVE)
	{
		ObjectID targetID = m_goalObj->getID();
		xfer->xferObjectID(&targetID);
	}
	else {
		ObjectID targetID;
		xfer->xferObjectID(&targetID);
		m_goalObj = TheGameLogic->findObjectByID(targetID);
	}

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void ScatterShotUpdate::loadPostProcess(void)
{

	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
