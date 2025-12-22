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

// FILE: FreeFallProjectileBehavior.cpp
// Author: Andi W, June 2025
// Desc:   

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/GameAudio.h"
#include "Common/BezierSegment.h"
#include "Common/GameCommon.h"
#include "Common/GameState.h"
#include "Common/Player.h"
#include "Common/ThingTemplate.h"
#include "Common/RandomValue.h"
#include "Common/Xfer.h"
#include "GameClient/Drawable.h"
#include "GameClient/FXList.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/Module/FreeFallProjectileBehavior.h"
#include "GameLogic/Module/MissileAIUpdate.h"
#include "GameLogic/Module/PhysicsUpdate.h"
#include "GameLogic/Weapon.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
const Int DEFAULT_MAX_LIFESPAN = 10 * LOGICFRAMES_PER_SECOND;

//-----------------------------------------------------------------------------
FreeFallProjectileBehaviorModuleData::FreeFallProjectileBehaviorModuleData() :
	m_maxLifespan(DEFAULT_MAX_LIFESPAN),
	m_detonateCallsKill(FALSE),
	m_tumbleRandomly(FALSE),
	m_courseCorrectionScalar(1.0f),
	m_exitPitchRate(1.0f),
	m_applyLauncherBonus(FALSE),
	// m_inheritTransportVelocity(FALSE),
	m_useWeaponSpeed(FALSE),
	m_garrisonHitKillCount(0),
	m_garrisonHitKillFX(NULL),
	m_detonateOnGround(TRUE),
	m_detonateOnCollide(TRUE)
{
}

//-----------------------------------------------------------------------------
void FreeFallProjectileBehaviorModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	UpdateModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "MaxLifespan", INI::parseDurationUnsignedInt, NULL, offsetof(FreeFallProjectileBehaviorModuleData, m_maxLifespan) },
		{ "TumbleRandomly", INI::parseBool, NULL, offsetof(FreeFallProjectileBehaviorModuleData, m_tumbleRandomly) },
		{ "DetonateCallsKill", INI::parseBool, NULL, offsetof(FreeFallProjectileBehaviorModuleData, m_detonateCallsKill) },
		{ "CourseCorrectionScalar",	INI::parseReal,		NULL, offsetof(FreeFallProjectileBehaviorModuleData, m_courseCorrectionScalar) },
		{ "ExitPitchRate",	INI::parseAngularVelocityReal,		NULL, offsetof(FreeFallProjectileBehaviorModuleData, m_exitPitchRate) },
		{ "UseWeaponSpeed", INI::parseBool,  NULL, offsetof(FreeFallProjectileBehaviorModuleData, m_useWeaponSpeed) },
		//{ "InheritShooterVelocity", INI::parseBool,  NULL, offsetof(FreeFallProjectileBehaviorModuleData, m_inheritTransportVelocity) },
		{ "ApplyLauncherBonus", INI::parseBool,  NULL, offsetof(FreeFallProjectileBehaviorModuleData, m_applyLauncherBonus) },
		
		{ "DetonateOnGround", INI::parseBool,  NULL, offsetof(FreeFallProjectileBehaviorModuleData, m_detonateOnGround) },
		{ "DetonateOnCollide", INI::parseBool,  NULL, offsetof(FreeFallProjectileBehaviorModuleData, m_detonateOnCollide) },

		{ "GarrisonHitKillRequiredKindOf", KindOfMaskType::parseFromINI, NULL, offsetof(FreeFallProjectileBehaviorModuleData, m_garrisonHitKillKindof) },
		{ "GarrisonHitKillForbiddenKindOf", KindOfMaskType::parseFromINI, NULL, offsetof(FreeFallProjectileBehaviorModuleData, m_garrisonHitKillKindofNot) },
		{ "GarrisonHitKillCount", INI::parseUnsignedInt, NULL, offsetof(FreeFallProjectileBehaviorModuleData, m_garrisonHitKillCount) },
		{ "GarrisonHitKillFX", INI::parseFXList, NULL, offsetof(FreeFallProjectileBehaviorModuleData, m_garrisonHitKillFX) },

		{ 0, 0, 0, 0 }
	};

	p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
FreeFallProjectileBehavior::FreeFallProjectileBehavior(Thing* thing, const ModuleData* moduleData) : UpdateModule(thing, moduleData)
{
	m_launcherID = INVALID_ID;
	m_victimID = INVALID_ID;
	m_targetPos.zero();
	m_detonationWeaponTmpl = NULL;
	m_lifespanFrame = 0;
	m_extraBonusFlags = 0;

	m_hasDetonated = FALSE;
}

//-------------------------------------------------------------------------------------------------
FreeFallProjectileBehavior::~FreeFallProjectileBehavior()
{
}


//-------------------------------------------------------------------------------------------------
// Prepares the missile for launch via proper weapon-system channels.
//-------------------------------------------------------------------------------------------------
void FreeFallProjectileBehavior::projectileLaunchAtObjectOrPosition(
	const Object* victim,
	const Coord3D* victimPos,
	const Object* launcher,
	WeaponSlotType wslot,
	Int specificBarrelToUse,
	const WeaponTemplate* detWeap,
	const ParticleSystemTemplate* exhaustSysOverride
)
{
	const FreeFallProjectileBehaviorModuleData* d = getFreeFallProjectileBehaviorModuleData();

	DEBUG_ASSERTCRASH(specificBarrelToUse >= 0, ("specificBarrelToUse must now be explicit"));

	m_launcherID = launcher ? launcher->getID() : INVALID_ID;
	m_extraBonusFlags = launcher ? launcher->getWeaponBonusCondition() : 0;

	if (d->m_applyLauncherBonus && m_extraBonusFlags != 0) {
		getObject()->setWeaponBonusConditionFlags(m_extraBonusFlags);
	}

	m_victimID = victim ? victim->getID() : INVALID_ID;
	m_detonationWeaponTmpl = detWeap;
	m_lifespanFrame = TheGameLogic->getFrame() + d->m_maxLifespan;

	Object* projectile = getObject();

	Weapon::positionProjectileForLaunch(projectile, launcher, wslot, specificBarrelToUse);

	projectileFireAtObjectOrPosition(victim, victimPos, detWeap, exhaustSysOverride);
}

//-------------------------------------------------------------------------------------------------
// The actual firing of the missile once setup. Uses a Bezier curve with points parameterized in ini
//-------------------------------------------------------------------------------------------------
void FreeFallProjectileBehavior::projectileFireAtObjectOrPosition(const Object* victim, const Coord3D* victimPos, const WeaponTemplate* detWeap, const ParticleSystemTemplate* exhaustSysOverride)
{
	const FreeFallProjectileBehaviorModuleData* d = getFreeFallProjectileBehaviorModuleData();
	Object* projectile = getObject();

	// if an object, aim at the center, not the ground part
	Coord3D victimPosToUse;
	if (victim)
		victim->getGeometryInfo().getCenterPosition(*victim->getPosition(), victimPosToUse);
	else
		victimPosToUse = *victimPos;

	m_targetPos = victimPosToUse;

	PhysicsBehavior* physics = projectile->getPhysics();
	if (physics) {
		
		Real pitchRate = physics->getCenterOfMassOffset() * d->m_exitPitchRate;
	
		if (d->m_tumbleRandomly)
		{
			pitchRate += GameLogicRandomValueReal(-1.0f / PI, 1.0f / PI);
			physics->setYawRate(GameLogicRandomValueReal(-1.0f / PI, 1.0f / PI));
			physics->setRollRate(GameLogicRandomValueReal(-1.0f / PI, 1.0f / PI));
		}

		physics->setPitchRate(pitchRate);

		// Note: The weapon actually does this already
		
		//if (d->m_inheritTransportVelocity)
		//{
		//	Coord3D velocity = *owner->getPhysics()->getVelocity();
		//	physics->applyForce(&velocity);
		//}

		if (d->m_useWeaponSpeed) {
			Real weaponSpeed = detWeap ? detWeap->getWeaponSpeed() : 0.0f;
			Real minWeaponSpeed = detWeap ? detWeap->getMinWeaponSpeed() : 0.0f;

			if (detWeap && detWeap->isScaleWeaponSpeed())
			{
				// Some weapons want to scale their start speed to the range
				Real minRange = detWeap->getMinimumAttackRange();
				Real maxRange = detWeap->getUnmodifiedAttackRange();
				Real range = sqrt(ThePartitionManager->getDistanceSquared(projectile, &victimPosToUse, FROM_CENTER_2D));
				Real rangeRatio = (range - minRange) / (maxRange - minRange);
				weaponSpeed = (rangeRatio * (weaponSpeed - minWeaponSpeed)) + minWeaponSpeed;
			}

			Coord3D velocity;
			projectile->getUnitDirectionVector3D(velocity);
			velocity.scale(weaponSpeed);
			physics->applyForce(&velocity);

		}
	} // If we don't have physics, this module is kinda useless, but whatever

	projectile->setModelConditionState(MODELCONDITION_FREEFALL);
	
	AudioEventRTS fallingSound = *projectile->getTemplate()->getSoundFalling();
	fallingSound.setObjectID(projectile->getID());
	TheAudio->addAudioEvent(&fallingSound);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool FreeFallProjectileBehavior::projectileHandleCollision(Object* other)
{
	const FreeFallProjectileBehaviorModuleData* d = getFreeFallProjectileBehaviorModuleData();

	if (other != NULL)
	{
		Object* projectileLauncher = TheGameLogic->findObjectByID(projectileGetLauncherID());

		// if it's not the specific thing we were targeting, see if we should incidentally collide...
		if (!m_detonationWeaponTmpl->shouldProjectileCollideWith(projectileLauncher, getObject(), other, m_victimID))
		{
			//DEBUG_LOG(("ignoring projectile collision with %s at frame %d\n",other->getTemplate()->getName().str(),TheGameLogic->getFrame()));
			return true;
		}

		if (d->m_garrisonHitKillCount > 0)
		{
			ContainModuleInterface* contain = other->getContain();
			if (contain && contain->getContainCount() > 0 && contain->isGarrisonable() && !contain->isImmuneToClearBuildingAttacks())
			{
				Int numKilled = 0;

				// garrisonable buildings subvert the normal process here.
				const ContainedItemsList* items = contain->getContainedItemsList();
				if (items)
				{
					for (ContainedItemsList::const_iterator it = items->begin(); it != items->end() && numKilled < d->m_garrisonHitKillCount; )
					{
						Object* thingToKill = *it++;
						if (!thingToKill->isEffectivelyDead() && thingToKill->isKindOfMulti(d->m_garrisonHitKillKindof, d->m_garrisonHitKillKindofNot))
						{
							//DEBUG_LOG(("Killed a garrisoned unit (%08lx %s) via Flash-Bang!\n",thingToKill,thingToKill->getTemplate()->getName().str()));
							if (projectileLauncher)
								projectileLauncher->scoreTheKill(thingToKill);
							thingToKill->kill();
							++numKilled;
						}
					} // next contained item
				} // if items

				if (numKilled > 0)
				{
					// note, fx is played at center of building, not at grenade's location
					FXList::doFXObj(d->m_garrisonHitKillFX, other, NULL);

					// don't do the normal explosion; just destroy ourselves & return
					TheGameLogic->destroyObject(getObject());

					return true;
				}
			}	// if a garrisonable thing
		}

		if (!d->m_detonateOnCollide) {
			return true;
		}

	}

	if (!d->m_detonateOnGround) {
		return true;
	}

	// collided with something... blow'd up!
	detonate();

	// mark ourself as "no collisions" (since we might still exist in slow death mode)
	getObject()->setStatus(MAKE_OBJECT_STATUS_MASK(OBJECT_STATUS_NO_COLLISIONS));
	return true;
}

//-------------------------------------------------------------------------------------------------
void FreeFallProjectileBehavior::detonate()
{
	if (m_hasDetonated)
		return;

	Object* obj = getObject();
	if (m_detonationWeaponTmpl)
	{
		TheWeaponStore->handleProjectileDetonation(m_detonationWeaponTmpl, obj, obj->getPosition(), m_extraBonusFlags);

		if (getFreeFallProjectileBehaviorModuleData()->m_detonateCallsKill)
		{
			// don't call kill(); do it manually, so we can specify DEATH_DETONATED
			DamageInfo damageInfo;
			damageInfo.in.m_damageType = DAMAGE_UNRESISTABLE;
			damageInfo.in.m_deathType = DEATH_DETONATED;
			damageInfo.in.m_sourceID = INVALID_ID;
			damageInfo.in.m_amount = obj->getBodyModule()->getMaxHealth();
			obj->attemptDamage(&damageInfo);
		}
		else
		{
			TheGameLogic->destroyObject(obj);
		}

	}
	else
	{
		// don't call kill(); do it manually, so we can specify DEATH_DETONATED
		DamageInfo damageInfo;
		damageInfo.in.m_damageType = DAMAGE_UNRESISTABLE;
		damageInfo.in.m_deathType = DEATH_DETONATED;
		damageInfo.in.m_sourceID = INVALID_ID;
		damageInfo.in.m_amount = obj->getBodyModule()->getMaxHealth();
		obj->attemptDamage(&damageInfo);
	}

	if (obj->getDrawable())
		obj->getDrawable()->setDrawableHidden(true);

	m_hasDetonated = TRUE;

}

//-------------------------------------------------------------------------------------------------
/**
 * Simulate one frame of a missile's behavior
 */
UpdateSleepTime FreeFallProjectileBehavior::update()
{
	const FreeFallProjectileBehaviorModuleData* d = getFreeFallProjectileBehaviorModuleData();

	if (m_lifespanFrame != 0 && TheGameLogic->getFrame() >= m_lifespanFrame)
	{
		// lifetime demands detonation
		detonate();
		return UPDATE_SLEEP_NONE;
	}

	{ // SmartBombTargetingUpdate
		Object* self = getObject();
		if (!self)
			return UPDATE_SLEEP_NONE;

		if (!self->isSignificantlyAboveTerrain())
			return UPDATE_SLEEP_NONE;

		const Coord3D* currentPos = self->getPosition();

		Coord3D pos;
		pos.zero();

		Real statusCoeff = MAX(0.0f, MIN(1.0f, d->m_courseCorrectionScalar));
		Real targetCoeff = 1.0f - statusCoeff;

		pos.x = m_targetPos.x * targetCoeff + currentPos->x * statusCoeff;
		pos.y = m_targetPos.y * targetCoeff + currentPos->y * statusCoeff;
		pos.z = currentPos->z;

		self->setPosition(&pos);
	}



	return UPDATE_SLEEP_NONE;//This no longer flys with physics, so it needs to not sleep
}

// ------------------------------------------------------------------------------------------------
const Coord3D* FreeFallProjectileBehavior::getTargetPosition()
{
	return &m_targetPos;
}
// ------------------------------------------------------------------------------------------------
Object* FreeFallProjectileBehavior::getTargetObject()
{
	return TheGameLogic->findObjectByID(m_victimID);
}

bool FreeFallProjectileBehavior::projectileShouldCollideWithWater() const
{
	if (m_detonationWeaponTmpl != nullptr) {
		return m_detonationWeaponTmpl->getProjectileCollideMask() & WeaponCollideMaskType::WEAPON_COLLIDE_WATER;
	}
	return false;
}


// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void FreeFallProjectileBehavior::crc(Xfer* xfer)
{

	// extend base class
	UpdateModule::crc(xfer);

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
	// ------------------------------------------------------------------------------------------------
void FreeFallProjectileBehavior::xfer(Xfer* xfer)
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion(&version, currentVersion);

	// extend base class
	UpdateModule::xfer(xfer);

	// launcher
	xfer->xferObjectID(&m_launcherID);

	// victim ID
	xfer->xferObjectID(&m_victimID);

	// target pos
	xfer->xferCoord3D(&m_targetPos);

	// weapon template
	AsciiString weaponTemplateName = AsciiString::TheEmptyString;
	if (m_detonationWeaponTmpl)
		weaponTemplateName = m_detonationWeaponTmpl->getName();
	xfer->xferAsciiString(&weaponTemplateName);
	if (xfer->getXferMode() == XFER_LOAD)
	{

		if (weaponTemplateName == AsciiString::TheEmptyString)
			m_detonationWeaponTmpl = NULL;
		else
		{

			// find template
			m_detonationWeaponTmpl = TheWeaponStore->findWeaponTemplate(weaponTemplateName);

			// sanity
			if (m_detonationWeaponTmpl == NULL)
			{

				DEBUG_CRASH(("FreeFallProjectileBehavior::xfer - Unknown weapon template '%s'\n",
					weaponTemplateName.str()));
				throw SC_INVALID_DATA;

			}  // end if

		}  // end else

	}  // end if

	// lifespan frame
	// xfer->xferUnsignedInt(&m_lifespanFrame);

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void FreeFallProjectileBehavior::loadPostProcess(void)
{

	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
