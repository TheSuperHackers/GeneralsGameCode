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

// FILE: ArmorDamageScalarUpdate.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Electronic Arts Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2002-2003 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
//	created:	March 2025
//
//	Filename: 	ArmorDamageScalarUpdate.cpp
//
//	author:		Andi W
//	
//	purpose:	Apply a temporary multiplier to damage taken to units in an area
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameLogic/Module/ArmorDamageScalarUpdate.h"

// #define DEFINE_WEAPONBONUSCONDITION_NAMES

#include "Common/ThingTemplate.h"
#include "Common/DamageFX.h"
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Weapon.h"
#include "GameClient/Drawable.h"
#include "GameClient/ParticleSys.h"

//-----------------------------------------------------------------------------
ArmorDamageScalarUpdateModuleData::ArmorDamageScalarUpdateModuleData()
{
	m_allowAffectKindOf.clear();
	m_forbiddenAffectKindOf.clear();
	m_targetsMask = 0;
	m_isAffectAirborne = true;
	m_bonusDuration = 0;
	m_initialDelay = 0;
	m_bonusRange = 0;
	m_armorDamageScalar = 1.0f;
	m_damageFx = NULL;
	m_effectParticleSystem = NULL;
	m_scaleParticleCount = false;
	// m_sparksPerCubicFoot = 0.001f;
}

//-----------------------------------------------------------------------------
void ArmorDamageScalarUpdateModuleData::buildFieldParse(MultiIniFieldParse& p)
{
  UpdateModuleData::buildFieldParse(p);
	static const FieldParse dataFieldParse[] = 
	{
		{ "AllowedAffectKindOf",		KindOfMaskType::parseFromINI,		NULL, offsetof(ArmorDamageScalarUpdateModuleData, m_allowAffectKindOf) },
		{ "ForbiddenAffectKindOf",	KindOfMaskType::parseFromINI,		NULL, offsetof(ArmorDamageScalarUpdateModuleData, m_forbiddenAffectKindOf ) },
		{ "AffectsTargets", INI::parseBitString32,	TheWeaponAffectsMaskNames, offsetof(ArmorDamageScalarUpdateModuleData, m_targetsMask) },
		{ "AffectAirborne", INI::parseBool, NULL, offsetof(ArmorDamageScalarUpdateModuleData, m_isAffectAirborne) },
		{ "BonusDuration",					INI::parseDurationUnsignedInt,	NULL, offsetof(ArmorDamageScalarUpdateModuleData, m_bonusDuration ) },
		{ "BonusRange",							INI::parseReal,									NULL, offsetof(ArmorDamageScalarUpdateModuleData, m_bonusRange ) },
		{ "InitialDelay",				INI::parseDurationUnsignedInt,	NULL, offsetof(ArmorDamageScalarUpdateModuleData, m_initialDelay) },
		{ "ArmorDamageScalar",			INI::parseReal,  NULL, offsetof(ArmorDamageScalarUpdateModuleData, m_armorDamageScalar) },
		{ "OverrideDamageFX",			INI::parseDamageFX,	NULL, offsetof(ArmorDamageScalarUpdateModuleData, m_damageFx) },
		{ "EffectParticleSystem",		INI::parseParticleSystemTemplate, NULL, offsetof(ArmorDamageScalarUpdateModuleData, m_effectParticleSystem) },
		{ "ScaleParticleSystem",			INI::parseBool, NULL, offsetof(ArmorDamageScalarUpdateModuleData, m_scaleParticleCount) },
		//{ "ParticlesPerCubicFoot",		INI::parseReal, NULL, offsetof(ArmorDamageScalarUpdateModuleData, m_sparksPerCubicFoot) },
		{ 0, 0, 0, 0 }
	};
  p.add(dataFieldParse);
}

//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
ArmorDamageScalarUpdate::ArmorDamageScalarUpdate( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{
	m_effectApplied = FALSE;

	m_affectedObjects.clear();
	m_dieFrame = 0;

	const ArmorDamageScalarUpdateModuleData* d = getArmorDamageScalarUpdateModuleData();
	if (d->m_initialDelay > 0) {
		setWakeFrame(getObject(), UPDATE_SLEEP(d->m_initialDelay));
	}
	else {
		setWakeFrame(getObject(), UPDATE_SLEEP_NONE);
	}
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ArmorDamageScalarUpdate::~ArmorDamageScalarUpdate( void )
{
	removeEffect();
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
UpdateSleepTime ArmorDamageScalarUpdate::update(void)
{
	if (!m_effectApplied) {
		applyEffect();
		const ArmorDamageScalarUpdateModuleData* d = getArmorDamageScalarUpdateModuleData();
		Int delay = d->m_bonusDuration;
		m_dieFrame = TheGameLogic->getFrame() + delay;
		return UPDATE_SLEEP(delay);
	}
	else {
		removeEffect();
		// We get deleted after the effect is over
		TheGameLogic->destroyObject(getObject());
		return UPDATE_SLEEP_FOREVER;
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

void ArmorDamageScalarUpdate::applyEffect(void) {

	// DEBUG_LOG(("ADSU: Apply Effect 1\n"));

	if (m_effectApplied) return;

	m_effectApplied = TRUE;
	const ArmorDamageScalarUpdateModuleData * data = getArmorDamageScalarUpdateModuleData();
	Object *me = getObject();

	Int targetFlags = 0;
	if (data->m_targetsMask & WEAPON_AFFECTS_ALLIES) targetFlags |= PartitionFilterRelationship::ALLOW_ALLIES;
	if (data->m_targetsMask & WEAPON_AFFECTS_ENEMIES) targetFlags |= PartitionFilterRelationship::ALLOW_ENEMIES;
	if (data->m_targetsMask & WEAPON_AFFECTS_NEUTRALS) targetFlags |= PartitionFilterRelationship::ALLOW_NEUTRAL;

	PartitionFilterRelationship relationship( me, targetFlags);
	PartitionFilterSameMapStatus filterMapStatus(me);
	PartitionFilterAlive filterAlive;

	// Leaving this here commented out to show that I need to reach valid contents of invalid transports.
	// So these checks are on an individual basis, not in the Partition query
    //	PartitionFilterAcceptByKindOf filterKindof(data->m_requiredAffectKindOf,data->m_forbiddenAffectKindOf);
	PartitionFilter *filters[] = { &relationship, &filterAlive, &filterMapStatus, NULL };

	// scan objects in our region
	ObjectIterator *iter = ThePartitionManager->iterateObjectsInRange( me->getPosition(), 
																	   data->m_bonusRange, 
																	   FROM_CENTER_2D, 
																	   filters );
	MemoryPoolObjectHolder hold( iter );
	
	for( Object *currentObj = iter->first(); currentObj != NULL; currentObj = iter->next() )
	{
		if (data->m_isAffectAirborne || (!currentObj->isKindOf(KINDOF_AIRCRAFT) && !currentObj->isAirborneTarget())) {

			if (currentObj->isAnyKindOf(data->m_allowAffectKindOf) && !currentObj->isAnyKindOf(data->m_forbiddenAffectKindOf))
			{
				applyEffectToObject(currentObj);
				m_affectedObjects.push_back(currentObj->getID());
			}

			// TODO: get contained objects and apply effect
			//if( currentObj->getContain() )
			//{
			//	currentObj->getContain()->iterateContained(containIteratingDoArmorDamageScalar, &armorDamageScalarData, FALSE);
			//}

		}
	}

	// DEBUG_LOG(("ADSU: Apply Effect Done. m_affectedObjects.size = %d\n", m_affectedObjects.size()));
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void ArmorDamageScalarUpdate::removeEffect(void) {

	// DEBUG_LOG((">>> ADSU: Remove Effect 1\n"));

	if (!m_effectApplied) return;

	m_effectApplied = FALSE;
	// const ArmorDamageScalarUpdateModuleData* data = getArmorDamageScalarUpdateModuleData();
	// Object* me = getObject();

	for (ObjectIDVectorIterator it = m_affectedObjects.begin(); it != m_affectedObjects.end(); ) {
		Object* obj = TheGameLogic->findObjectByID(*it);
		if (obj) {
			if (!obj->isEffectivelyDead()) {
				removeEffectFromObject(obj);
			}
		}

		it = m_affectedObjects.erase(it);
	}

	// DEBUG_LOG((">>> ADSU: Remove Effect Done.\n"));
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------

void ArmorDamageScalarUpdate::applyEffectToObject(Object *obj) {
	const ArmorDamageScalarUpdateModuleData* data = getArmorDamageScalarUpdateModuleData();
	BodyModuleInterface* body = obj->getBodyModule();
	body->applyDamageScalar(data->m_armorDamageScalar);

	//DEBUG_LOG((">>>ADSU: apply scalar '%f' to obj '%s' - new scalar = '%f' \n",
	//	data->m_armorDamageScalar, obj->getTemplate()->getName().str(), body->getDamageScalar()));

	// Apply Particle System
	Drawable* drw = obj->getDrawable();
	if (drw)
	{
		const ParticleSystemTemplate* tmp = data->m_effectParticleSystem;
		if (tmp)
		{
			ParticleSystem* sys = TheParticleSystemManager->createParticleSystem(tmp);

			if (sys)
			{
				sys->attachToObject(obj);
				sys->setSystemLifetime(data->m_bonusDuration);

				if (data->m_scaleParticleCount) {
					// Scale particle count based on size
					Real x = obj->getGeometryInfo().getMajorRadius();
					Real y = obj->getGeometryInfo().getMinorRadius();
					Real z = obj->getGeometryInfo().getMaxHeightAbovePosition() * 0.5;
					sys->setEmissionBoxHalfSize(x, y, z);
					Real size = x * y;
					sys->setBurstCountMultiplier(MAX(1.0, sqrt(size * 0.02f))); // these are somewhat tweaked right now
					sys->setBurstDelayMultiplier(MIN(5.0, sqrt(500.0f / size)));

					//DEBUG_LOG((">>>ADSU: attach particles to %s: with size (%f, %f, %f) countMultiplier = %f, delayMultiplier = %f \n",
					//	obj->getTemplate()->getName().str(), x, y, z, sys->getBurstCountMultiplier(), sys->getBurstDelayMultiplier()));
				}
				
				DEBUG_LOG((">>>ADSU: spawned particle system with lifetime '%d'.\n",
					data->m_bonusDuration));
			}
		}
	}

	// Apply DamageFX

	if (data->m_damageFx) {
		body->overrideDamageFX(data->m_damageFx);
	}

}
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void ArmorDamageScalarUpdate::removeEffectFromObject(Object* obj) {
	const ArmorDamageScalarUpdateModuleData* data = getArmorDamageScalarUpdateModuleData();
	BodyModuleInterface* body = obj->getBodyModule();
	Real scalar = 1.0f / __max(data->m_armorDamageScalar, 0.01f);
	body->applyDamageScalar(scalar);

	//DEBUG_LOG((">>>ADSU: apply (=remove) scalar '%f' to obj '%s' - new scalar = '%f' \n",
	//	scalar, obj->getTemplate()->getName().str(), body->getDamageScalar()));

	if (data->m_damageFx) {
		body->overrideDamageFX(NULL);
	}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void ArmorDamageScalarUpdate::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void ArmorDamageScalarUpdate::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// affected objects
	Int vectorSize = m_affectedObjects.size();
	xfer->xferInt(&vectorSize);
	m_affectedObjects.resize(vectorSize);
	for (Int vectorIndex = 0; vectorIndex < vectorSize; ++vectorIndex)
	{
		xfer->xferObjectID(&m_affectedObjects[vectorIndex]);
	}

	// die frame
	xfer->xferUnsignedInt(&m_dieFrame);

	// effect applied
	xfer->xferBool(&m_effectApplied);


}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void ArmorDamageScalarUpdate::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

}  // end loadPostProcess
