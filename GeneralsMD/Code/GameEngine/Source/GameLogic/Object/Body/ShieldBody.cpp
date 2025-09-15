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

// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ShieldBodyModuleData::ShieldBodyModuleData()
{
	m_damageTypesToPassThrough = DAMAGE_TYPE_FLAGS_NONE;
	//m_maxHealth = 0;
	//m_initialHealth = 0;
	//m_subdualDamageCap = 0;
	//m_subdualDamageHealRate = 0;
	//m_subdualDamageHealAmount = 0;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ShieldBodyModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	ActiveBodyModuleData::buildFieldParse(p);
	//UpdateModuleData::buildFieldParse(p);

	static const FieldParse dataFieldParse[] =
	{
		//TODO
		{ "StartsActive",						INI::parseBool,						NULL,		offsetof(ShieldBodyModuleData, m_startsActive) },

		{ "ShieldMaxHealth",						INI::parseReal,						NULL,		offsetof(ShieldBodyModuleData, m_shieldMaxHealth) },
		{ "ShieldMaxHealthPercent",				parseShieldHealthPercent,						NULL,	  0}, //	offsetof(ShieldBodyModuleData, m_shieldMaxHealthPercent) },

		//{ "ShieldRechargeDelay",		INI::parseDurationUnsignedInt,	NULL,		offsetof(ShieldBodyModuleData, m_shieldRechargeDelay) },
		//{ "ShieldRechargeRate",		INI::parseDurationUnsignedInt,	NULL,		offsetof(ShieldBodyModuleData, m_shieldRechargeRate) },
		//{ "ShieldRechargeAmount",	INI::parseReal,									NULL,		offsetof(ShieldBodyModuleData, m_shieldRechargeAmount) },
		//{ "ShieldRechargeAmountPercent",	parseShieldRechargeAmountPercent,	 NULL,		0}, //offsetof(ShieldBodyModuleData, m_shieldRechargeAmountPercent) },

		{ "DamageTypesToPassThroughShield",   INI::parseDamageTypeFlags, NULL, offsetof(ShieldBodyModuleData, m_damageTypesToPassThrough) },
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
//void ShieldBodyModuleData::parseShieldRechargeAmountPercent(INI* ini, void* instance, void* store, const void* /*userData*/)
//{
//	ShieldBodyModuleData* self = (ShieldBodyModuleData*)instance;
//	Real amountPercent = INI::scanPercentToReal(ini->getNextToken());
//	self->m_shieldMaxHealth = self->m_shieldMaxHealth * amountPercent;
//}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ShieldBody::ShieldBody( Thing *thing, const ModuleData* moduleData ) : ActiveBody( thing, moduleData )
{
	/*const ShieldBodyModuleData* data = getShieldBodyModuleData();
	m_currentShieldHealth = data->m_shieldMaxHealth;*/

}  // end ShieldBody

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ShieldBody::~ShieldBody( void )
{

}  // end ~ShieldBody

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//UpdateSleepTime ShieldBody::update(void)
//{
//	DEBUG_LOG((">>> ShieldBody::update - m_currentShieldHealth = %f, m_healingStepCountdown = %d", m_currentShieldHealth, m_healingStepCountdown));
//
//	//UnsignedInt now = TheGameLogic->getFrame();
//	//UnsignedInt damageFrame = getLastDamageTimestamp();
//	//const ShieldBodyModuleData* data = getShieldBodyModuleData();
//	//if (now < damageFrame + data->m_shieldRechargeDelay) {
//	//	return UPDATE_SLEEP_NONE;
//	//}
//
//	m_healingStepCountdown--;
//	if (m_healingStepCountdown > 0)
//		return UPDATE_SLEEP_NONE;
//
//	const ShieldBodyModuleData* data = getShieldBodyModuleData();
//	m_healingStepCountdown = data->m_shieldRechargeRate;
//
//	m_currentShieldHealth = MIN(data->m_shieldMaxHealth, m_currentShieldHealth + data->m_shieldRechargeAmount);
//
//	//if (m_currentShieldHealth < data->m_shieldMaxHealth)
//	//	return UPDATE_SLEEP_NONE;
//	//else
//	//	return UPDATE_SLEEP_FOREVER;
//	return UPDATE_SLEEP_NONE;
//}  // end update
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool ShieldBody::getShieldPercent(Real& percentage)
{
	// TODO:
	//if (isActive()) {
		const ShieldBodyModuleData* data = getShieldBodyModuleData();
		percentage = m_currentShieldHealth / data->m_shieldMaxHealth;
		return TRUE;
	//}
		//return FALSE;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool ShieldBody::rechargeShieldHealth(Real amount)
{
	const ShieldBodyModuleData* data = getShieldBodyModuleData();
	m_currentShieldHealth = MIN(data->m_shieldMaxHealth, m_currentShieldHealth + amount);
	return m_currentShieldHealth >= data->m_shieldMaxHealth;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void ShieldBody::attemptDamage(DamageInfo* damageInfo)
{
	validateArmorAndDamageFX();

	// sanity
	if (damageInfo == NULL)
		return;

	const ShieldBodyModuleData* data = getShieldBodyModuleData();
	//Object* obj = getObject();

	// Pass through full famage
	if (getDamageTypeFlag(data->m_damageTypesToPassThrough, damageInfo->in.m_damageType))
	{
		ActiveBody::attemptDamage(damageInfo);
		return;
	}

	// Calculate Shield damage:
	// TODO: If we have other ways to use the shield, we could add some conditions
	EnergyShieldBehaviorInterface* esbi = getEnergyShieldBehaviorInterface();
	if (!esbi) {
		DEBUG_LOG(("ShieldBody::attemptDamage - Warning, no EnergyShieldBehaviorModule found."));
		ActiveBody::attemptDamage(damageInfo);
		return;
	}
	
	Real damageToShield = getCurrentArmor().adjustDamage(damageInfo->in.m_damageType, damageInfo->in.m_amount);

	Real remainingShieldHealth = m_currentShieldHealth - damageToShield;
	m_currentShieldHealth = MAX(0, remainingShieldHealth);

	// Apply  Damage to shield and Stop shield recharge - Notify shield behavior
	esbi->applyDamage(MAX(0, damageToShield));

	if (remainingShieldHealth < 0) {

		//TODO: Disable Shield Armor

		Real overkillDamage = abs(remainingShieldHealth);
		damageInfo->in.m_amount = overkillDamage;
	}
	else {
		damageInfo->in.m_amount = 0.0001;
	}

	// extend
	ActiveBody::attemptDamage(damageInfo);
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
