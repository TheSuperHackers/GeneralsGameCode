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

// FILE: EnergyShieldBehavior.cpp ///////////////////////////////////////////////////////////////////////
// Author:
// Desc:
///////////////////////////////////////////////////////////////////////////////////////////////////


// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/Thing.h"
#include "Common/ThingTemplate.h"
#include "Common/INI.h"
#include "Common/Player.h"
#include "Common/Xfer.h"
#include "GameClient/ParticleSys.h"
#include "GameClient/Anim2D.h"
#include "GameClient/InGameUI.h"
#include "GameLogic/Module/EnergyShieldBehavior.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/ShieldBody.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
EnergyShieldBehaviorModuleData::EnergyShieldBehaviorModuleData()
{
		//m_damageTypesToPassThrough = DAMAGE_TYPE_FLAGS_NONE;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

/*static*/ void EnergyShieldBehaviorModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	static const FieldParse dataFieldParse[] =
	{
		{ "StartsActive",	INI::parseBool, NULL, offsetof(EnergyShieldBehaviorModuleData, m_initiallyActive) },
		//{ "ShieldMaxHealth",						INI::parseReal,						NULL,		offsetof(EnergyShieldBehaviorModuleData, m_shieldMaxHealth) },
		//{ "ShieldMaxHealthPercent",				parseShieldHealthPercent,						NULL,	  0}, //	offsetof(EnergyShieldBehaviorModuleData, m_shieldMaxHealthPercent) },

		{ "ShieldRechargeDelay",		INI::parseDurationUnsignedInt,	NULL,		offsetof(EnergyShieldBehaviorModuleData, m_shieldRechargeDelay) },
		{ "ShieldRechargeRate",		INI::parseDurationUnsignedInt,	NULL,		offsetof(EnergyShieldBehaviorModuleData, m_shieldRechargeRate) },
		{ "ShieldRechargeAmount",	INI::parseReal,									NULL,		offsetof(EnergyShieldBehaviorModuleData, m_shieldRechargeAmount) },
		{ "ShieldRechargeAmountPercent",	INI::parseReal,	 NULL,		offsetof(EnergyShieldBehaviorModuleData, m_shieldRechargeAmountPercent) },

		//{ "DamageTypesToPassThroughShield",   INI::parseDamageTypeFlags, NULL, offsetof(EnergyShieldBehaviorModuleData, m_damageTypesToPassThrough) },
		{ 0, 0, 0, 0 }
	};

	UpdateModuleData::buildFieldParse(p);
	p.add(dataFieldParse);
	p.add(UpgradeMuxData::getFieldParse(), offsetof(EnergyShieldBehaviorModuleData, m_upgradeMuxData));
}

////-------------------------------------------------------------------------------------------------
////-------------------------------------------------------------------------------------------------
//void EnergyShieldBehaviorModuleData::parseShieldHealthPercent(INI* ini, void* instance, void* store, const void* /*userData*/)
//{
//	EnergyShieldBehaviorModuleData* self = (EnergyShieldBehaviorModuleData*)instance;
//	Real healthPercent = INI::scanPercentToReal(ini->getNextToken());
//	self->m_shieldMaxHealth = self->m_maxHealth * healthPercent;
//}
////-------------------------------------------------------------------------------------------------
//void EnergyShieldBehaviorModuleData::parseShieldRechargeAmountPercent(INI* ini, void* instance, void* store, const void* /*userData*/)
//{
//	EnergyShieldBehaviorModuleData* self = (EnergyShieldBehaviorModuleData*)instance;
//	Real amountPercent = INI::scanPercentToReal(ini->getNextToken());
//	self->m_shieldMaxHealth = self->m_shieldMaxHealth * amountPercent;
//}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
EnergyShieldBehavior::EnergyShieldBehavior( Thing *thing, const ModuleData* moduleData ) : UpdateModule( thing, moduleData )
{

	ShieldBody* body = dynamic_cast<ShieldBody*>(getObject()->getBodyModule());
	DEBUG_ASSERTCRASH((body != nullptr), ("EnergyShieldBehavior requires ShieldBody!"))

	m_body = body;

	const EnergyShieldBehaviorModuleData *d = getEnergyShieldBehaviorModuleData();

	if (d->m_initiallyActive)
	{
		giveSelfUpgrade();
	}
	else
	{
		setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
EnergyShieldBehavior::~EnergyShieldBehavior( void )
{

}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void EnergyShieldBehavior::applyDamage(Real amount)
{
	if (amount > 0)
	{
		const EnergyShieldBehaviorModuleData* data = getEnergyShieldBehaviorModuleData();
		m_healingStepCountdown = data->m_shieldRechargeDelay;
		setWakeFrame(getObject(), UPDATE_SLEEP_NONE);
	}
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
// This should perhaps be returned from the body instead
Bool EnergyShieldBehavior::getShieldPercent(Real &progress) const
{
	if (isActive() && m_body != NULL)
	{
		//return (m_body->getShieldMaxHealth() / m_body->getShieldCurrentHealth());
		return m_body->getShieldPercent(progress);
	}
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
/** The update callback. */
//-------------------------------------------------------------------------------------------------
UpdateSleepTime EnergyShieldBehavior::update( void )
{
		DEBUG_LOG((">>> EnergyShieldBehavior::update -  m_healingStepCountdown = %d", m_healingStepCountdown));
	
		//UnsignedInt now = TheGameLogic->getFrame();
		//UnsignedInt damageFrame = getLastDamageTimestamp();
		//const ShieldBodyModuleData* data = getShieldBodyModuleData();
		//if (now < damageFrame + data->m_shieldRechargeDelay) {
		//	return UPDATE_SLEEP_NONE;
		//}
	
		m_healingStepCountdown--;
		if (m_healingStepCountdown > 0)
			return UPDATE_SLEEP_NONE;
	
		const EnergyShieldBehaviorModuleData* data = getEnergyShieldBehaviorModuleData();
		m_healingStepCountdown = data->m_shieldRechargeRate;

		if (m_body) {
			Real rechargeAmount;
			if (data->m_shieldRechargeAmountPercent) {
				rechargeAmount = data->m_shieldRechargeAmountPercent * m_body->getShieldMaxHealth();
			}else {
				rechargeAmount = data->m_shieldRechargeAmount;
			}

			Bool full = m_body->rechargeShieldHealth(rechargeAmount);
			if (full)
				return UPDATE_SLEEP_FOREVER;
			else
				return UPDATE_SLEEP_NONE;
		}
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void EnergyShieldBehavior::crc( Xfer *xfer )
{

	// extend base class
	UpdateModule::crc( xfer );

	// extend base class
	UpgradeMux::upgradeMuxCRC( xfer );

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
// ------------------------------------------------------------------------------------------------
void EnergyShieldBehavior::xfer( Xfer *xfer )
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion( &version, currentVersion );

	// extend base class
	UpdateModule::xfer( xfer );

	// extend base class
	UpgradeMux::upgradeMuxXfer( xfer );

	// current shield health
	//xfer->xferReal(&m_currentShieldHealth);
	xfer->xferUnsignedInt(&m_healingStepCountdown);

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void EnergyShieldBehavior::loadPostProcess( void )
{

	// extend base class
	UpdateModule::loadPostProcess();

	// extend base class
	UpgradeMux::upgradeMuxLoadPostProcess();

}  // end loadPostProcess
