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

// FILE: ShieldBody.cpp ////////////////////////////////////////////////////////////////////////
// Author: Colin Day, November 2001
// Desc:	 Structure bodies are active bodies specifically for structures that are built
//				 and/or interactable (is that a world) with the player.
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/Xfer.h"
#include "GameLogic/Object.h"
#include "GameLogic/Damage.h"
#include "GameLogic/Module/ShieldBody.h"
#include "GameLogic/Module/EnergyShieldBehavior.h"
#include "GameLogic/ArmorSet.h"
#include "GameLogic/GameLogic.h"
#include "Common/Player.h"

// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ShieldBodyModuleData::ShieldBodyModuleData()
{
	m_damageTypesToPassThrough = DAMAGE_TYPE_FLAGS_NONE;
	m_shieldArmorSetFlag = ARMORSET_NONE;

	// Init default pass through types
	m_defaultDamageTypesToPassThrough = DAMAGE_TYPE_FLAGS_NONE;
	m_defaultDamageTypesToPassThrough = setDamageTypeFlag(m_defaultDamageTypesToPassThrough, DAMAGE_STATUS);
	m_defaultDamageTypesToPassThrough = setDamageTypeFlag(m_defaultDamageTypesToPassThrough, DAMAGE_DEPLOY);
	m_defaultDamageTypesToPassThrough = setDamageTypeFlag(m_defaultDamageTypesToPassThrough, DAMAGE_UNRESISTABLE);
	m_defaultDamageTypesToPassThrough = setDamageTypeFlag(m_defaultDamageTypesToPassThrough, DAMAGE_HEALING);
	m_defaultDamageTypesToPassThrough = setDamageTypeFlag(m_defaultDamageTypesToPassThrough, DAMAGE_PENALTY);
	m_defaultDamageTypesToPassThrough = setDamageTypeFlag(m_defaultDamageTypesToPassThrough, DAMAGE_DISARM);
	m_defaultDamageTypesToPassThrough = setDamageTypeFlag(m_defaultDamageTypesToPassThrough, DAMAGE_HAZARD_CLEANUP);
	m_defaultDamageTypesToPassThrough = setDamageTypeFlag(m_defaultDamageTypesToPassThrough, DAMAGE_TOPPLING);
	m_defaultDamageTypesToPassThrough = setDamageTypeFlag(m_defaultDamageTypesToPassThrough, DAMAGE_SUBDUAL_UNRESISTABLE);
	m_defaultDamageTypesToPassThrough = setDamageTypeFlag(m_defaultDamageTypesToPassThrough, DAMAGE_CHRONO_UNRESISTABLE);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ShieldBodyModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	ActiveBodyModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		{ "StartsActive",						INI::parseBool,						NULL,		offsetof(ShieldBodyModuleData, m_startsActive) },

		{ "ShieldMaxHealth",						INI::parseReal,						NULL,		offsetof(ShieldBodyModuleData, m_shieldMaxHealth) },
		{ "ShieldMaxHealthPercent",				parseShieldHealthPercent,						NULL,	  0}, //	offsetof(ShieldBodyModuleData, m_shieldMaxHealthPercent) },

		{ "ShieldArmorSetFlag",				INI::parseIndexList,	ArmorSetFlags::getBitNames(),	  offsetof(ShieldBodyModuleData, m_shieldArmorSetFlag) },

		{ "ShieldPassThroughDamageTypes",   INI::parseDamageTypeFlags, NULL, offsetof(ShieldBodyModuleData, m_damageTypesToPassThrough) },
		{ "DefaultShieldPassThroughDamageTypes",   INI::parseDamageTypeFlags, NULL, offsetof(ShieldBodyModuleData, m_defaultDamageTypesToPassThrough) },
		{ 0, 0, 0, 0 }
	};
	p.add(dataFieldParse);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ShieldBodyModuleData::parseShieldHealthPercent(INI* ini, void* instance, void* store, const void* /*userData*/)
{
	ShieldBodyModuleData* self = (ShieldBodyModuleData*)instance;
	Real healthPercent = INI::scanPercentToReal(ini->getNextToken());
	self->m_shieldMaxHealth = self->m_maxHealth * healthPercent;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ShieldBody::ShieldBody( Thing *thing, const ModuleData* moduleData ) : ActiveBody( thing, moduleData )
{
	 const ShieldBodyModuleData* data = getShieldBodyModuleData();

	 if (data->m_startsActive) {
		 m_active = true;
		 m_currentShieldHealth = data->m_shieldMaxHealth;
		 enableShieldEffects();
	 }
	 

}  // end ShieldBody

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ShieldBody::~ShieldBody( void )
{

}  // end ~ShieldBody

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Real ShieldBody::getShieldPercent()
{
		const ShieldBodyModuleData* data = getShieldBodyModuleData();
		return m_currentShieldHealth / data->m_shieldMaxHealth;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool ShieldBody::rechargeShieldHealth(Real amount)
{
	const ShieldBodyModuleData* data = getShieldBodyModuleData();

	// state before recharge
	Bool shieldWasUp = m_currentShieldHealth > 0;

	m_currentShieldHealth = MIN(data->m_shieldMaxHealth, m_currentShieldHealth + amount);

	//DEBUG_LOG((">>> ShieldBody::rechargeShieldHealth -  m_currentShieldHealth = %f", m_currentShieldHealth));

	if (!shieldWasUp && m_currentShieldHealth > 0) {
		enableShieldEffects();
	}

	return m_currentShieldHealth >= data->m_shieldMaxHealth;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ShieldBody::findShieldBehaviorModule() {

	EnergyShieldBehaviorInterface* esbi = NULL;

	for (BehaviorModule** u = getObject()->getBehaviorModules(); *u; ++u)
	{
		if ((esbi = (*u)->getEnergyShieldBehaviorInterface()) != NULL) {
			m_shieldBehaviorModule = esbi;
			return;
		}
	}
	
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ShieldBody::enableShieldEffects() {
	const ShieldBodyModuleData* data = getShieldBodyModuleData();
	if (data->m_shieldArmorSetFlag != ARMORSET_NONE) {
		getObject()->setArmorSetFlag(data->m_shieldArmorSetFlag);
		validateArmorAndDamageFX();
	}
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ShieldBody::disableShieldEffects() {
	const ShieldBodyModuleData* data = getShieldBodyModuleData();
	if (data->m_shieldArmorSetFlag != ARMORSET_NONE) {
		getObject()->clearArmorSetFlag(data->m_shieldArmorSetFlag);
		validateArmorAndDamageFX();
	}
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ShieldBody::doDamageFX(const DamageInfo* damageInfo)
{
	// If our damage is absorbed by the shield, we play the damageFX early to have the proper numbers
	// To avoid playing the damageFX again later on, we check if our shield is still up
	// and damage is 0, which means we got a full absorb

	if (m_currentShieldHealth > 0 && damageInfo->out.m_actualDamageDealt == 0)
		return;

	ActiveBody::doDamageFX(damageInfo);

}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ShieldBody::onDisabledEdge(Bool nowDisabled)
{
	if (!isActive()) {  // If the shield isn't enabled, no need to do anything
		return;
	}

	if (nowDisabled) {
		m_currentShieldHealth = 0;
		disableShieldEffects();

		if (m_shieldBehaviorModule == NULL) {
			findShieldBehaviorModule();
		}

		if (!m_shieldBehaviorModule) {
			return;
		}

		m_shieldBehaviorModule->applyDamage(getShieldMaxHealth());
	}
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ShieldBody::attemptDamage(DamageInfo* damageInfo)
{
	// Shield is not enabled, just pass through
	if (!isActive()) {
		ActiveBody::attemptDamage(damageInfo);
		return;
	}

	validateArmorAndDamageFX();

	// We need to do all the early return checks from ActiveBody here as well
	// For anything that should prevent even hitting the shield

	// sanity
	if (damageInfo == NULL)
		return;

	if (isIndestructible())
		return;

	// initialize these, just in case we bail out early
	damageInfo->out.m_actualDamageDealt = 0.0f;
	damageInfo->out.m_actualDamageClipped = 0.0f;

	// we cannot damage again objects that are already dead
	Object* obj = getObject();
	if (obj->isEffectivelyDead())
		return;

	// Units that get disabled by Chrono damage cannot take damage:
	if (obj->isDisabledByType(DISABLED_CHRONO) &&
		!(damageInfo->in.m_damageType == DAMAGE_CHRONO_GUN || damageInfo->in.m_damageType == DAMAGE_CHRONO_UNRESISTABLE))
		return;

	// --
	// BEGIN SHIELD CALCULATIONS
	// ---

	const ShieldBodyModuleData* data = getShieldBodyModuleData();

	// Pass through full damage
	if (getDamageTypeFlag(data->m_damageTypesToPassThrough, damageInfo->in.m_damageType) ||
		getDamageTypeFlag(data->m_defaultDamageTypesToPassThrough, damageInfo->in.m_damageType))
	{
		// We need to switch our armorset for this one
		if (data->m_shieldArmorSetFlag != ARMORSET_NONE) {
			getObject()->clearArmorSetFlag(data->m_shieldArmorSetFlag);
		}

		ActiveBody::attemptDamage(damageInfo);

		// And switch back
		if (data->m_shieldArmorSetFlag != ARMORSET_NONE) {
			getObject()->setArmorSetFlag(data->m_shieldArmorSetFlag);
			//validateArmorAndDamageFX();
		}
		return;
	}

	// Calculate Shield damage:
	// TODO: If we have other ways to use the shield, we could add some conditions
	if (m_shieldBehaviorModule == NULL) {
		findShieldBehaviorModule();
	}

	if (!m_shieldBehaviorModule) {
		DEBUG_LOG(("ShieldBody::attemptDamage - Warning, no EnergyShieldBehaviorModule found."));
		ActiveBody::attemptDamage(damageInfo);
		return;
	}

	//Shield is already down, just pass through, but stop recovery
	if (m_currentShieldHealth <= 0) {
		m_shieldBehaviorModule->applyDamage(0.0f);
		ActiveBody::attemptDamage(damageInfo);
		return;
	}

	Real rawDamage = damageInfo->in.m_amount;
	Real damageToShield = getCurrentArmor().adjustDamage(damageInfo->in.m_damageType, damageInfo->in.m_amount);

	//Note: we apply the damage scalar only to the damage to the actual shield.
	// It's later applied again for overkill damage
	if (damageInfo->in.m_damageType != DAMAGE_UNRESISTABLE)
	{
		damageToShield *= m_damageScalar;
	}

	Real damageAmountToPass = 0.0;  // The damage (done to HP) we pass to the base function

	Bool shieldStillUp = TRUE;
	if (damageToShield > 0) {
		Real remainingShieldHealth = m_currentShieldHealth - damageToShield;

		shieldStillUp = remainingShieldHealth > 0;

		m_currentShieldHealth = MAX(0, remainingShieldHealth);

		// Apply  Damage to shield and Stop shield recharge - Notify shield behavior
		m_shieldBehaviorModule->applyDamage(MAX(0, damageToShield));
		//DEBUG_LOG(("ShieldBody::attemptDamage - m_currentShieldHealth = %f.", m_currentShieldHealth));

		  //If the shield is at 0, we still need to disable it
		if (!shieldStillUp) {

			// Disable Shield Armor
			disableShieldEffects();

			Real overkillDamage = abs(remainingShieldHealth);

			// the shield overkill damage was already mitigated by shield armor
			// so we want it's relative amount of the raw damage (i.e. we invert the armor)
			Real relativeRawDamage = (overkillDamage / damageToShield) * rawDamage;

			damageAmountToPass = relativeRawDamage;
		}
	}

	if (shieldStillUp) {
		// Only play the damageFX on a full absorb
		damageInfo->out.m_actualDamageDealt = damageToShield;  //only used for damageFX
		doDamageFX(damageInfo);
		damageInfo->out.m_actualDamageDealt = 0;
	}


	damageInfo->in.m_amount = damageAmountToPass;

	// extend
	ActiveBody::attemptDamage(damageInfo);
	//DEBUG_LOG(("ShieldBody::attemptDamage - getHealth() = %f.", getHealth()));

	if (shieldStillUp) {
		// We passed on 0 damage to ActiveBody.
		// This means we need to do a couple of things that were skipped

		if (m_lastDamageTimestamp != TheGameLogic->getFrame() && m_lastDamageTimestamp != TheGameLogic->getFrame() - 1) {
			m_lastDamageInfo = *damageInfo;
			m_lastDamageCleared = false;
			m_lastDamageTimestamp = TheGameLogic->getFrame();
		}
		else {
			// Multiple damages applied in one/next frame.  We prefer the one that tells who the attacker is.
			Object* srcObj1 = TheGameLogic->findObjectByID(m_lastDamageInfo.in.m_sourceID);
			Object* srcObj2 = TheGameLogic->findObjectByID(damageInfo->in.m_sourceID);
			if (srcObj2) {
				if (srcObj1) {
					if (srcObj2->isKindOf(KINDOF_VEHICLE) || srcObj2->isKindOf(KINDOF_INFANTRY) ||
						srcObj2->isFactionStructure()) {
						m_lastDamageInfo = *damageInfo;
						m_lastDamageCleared = false;
						m_lastDamageTimestamp = TheGameLogic->getFrame();
					}
				}
				else {
					m_lastDamageInfo = *damageInfo;
					m_lastDamageCleared = false;
					m_lastDamageTimestamp = TheGameLogic->getFrame();
				}

			}
			else {
				// no change.
			}
		}

		// Notify the player that they have been attacked by this player
		if (m_lastDamageInfo.in.m_sourceID != INVALID_ID)
		{
			Object* srcObj = TheGameLogic->findObjectByID(m_lastDamageInfo.in.m_sourceID);
			if (srcObj)
			{
				Player* srcPlayer = srcObj->getControllingPlayer();
				obj->getControllingPlayer()->setAttackedBy(srcPlayer->getPlayerIndex());
			}
		}

	}

}
// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void ShieldBody::crc( Xfer *xfer )
{

	// extend base class
	ActiveBody::crc( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void ShieldBody::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// base class
	ActiveBody::xfer( xfer );

	// current shield health
	xfer->xferReal(&m_currentShieldHealth);
	//xfer->xferUnsignedInt(&m_healingStepCountdown);

	// shield activation
	xfer->xferBool(&m_active);

	// constructor object id
	//xfer->xferObjectID( &m_constructorObjectID );

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void ShieldBody::loadPostProcess( void )
{

	// extend base class
	ActiveBody::loadPostProcess();

}  // end loadPostProcess
