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
#include "GameClient/FXList.h"

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
EnergyShieldBehaviorModuleData::EnergyShieldBehaviorModuleData()
{
	m_barColor.red = 255;
	m_barColor.green = 255;
	m_barColor.blue = 255;
	m_barColor.alpha = 255;

	m_barBGColor.red = 255;
	m_barBGColor.green = 255;
	m_barBGColor.blue = 255;
	m_barBGColor.alpha = 255;

	m_shieldSubObjName.clear();
	m_shieldConditionFlag = MODELCONDITION_INVALID;
	m_shieldHitConditionFlag = MODELCONDITION_INVALID;
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
		{ "ShieldRechargeAmountPercent",	INI::parsePercentToReal,	 NULL,		offsetof(EnergyShieldBehaviorModuleData, m_shieldRechargeAmountPercent) },

		{ "ShieldHealthBarColor",	INI::parseRGBAColorInt,	 NULL,		offsetof(EnergyShieldBehaviorModuleData, m_barColor) },
		{ "ShieldHealthBarBackgroundColor",	INI::parseRGBAColorInt,	 NULL,		offsetof(EnergyShieldBehaviorModuleData, m_barBGColor) },
		{ "ShowHealthBarBackgroundWhenEmpty",	INI::parseBool,	 NULL,		offsetof(EnergyShieldBehaviorModuleData, m_showBarWhenEmpty) },
		{ "ShowHealthBarWhenUnselected",	INI::parseBool,	 NULL,		offsetof(EnergyShieldBehaviorModuleData, m_showBarWhenUnselected) },

		 { "ShieldSubObjectName",	INI::parseAsciiString,	 NULL,		offsetof(EnergyShieldBehaviorModuleData, m_shieldSubObjName) },
		 { "ShieldHitSubObjectName",	INI::parseAsciiString,	 NULL,		offsetof(EnergyShieldBehaviorModuleData, m_shieldHitSubObjName) },
		//{ "ShowShieldSubObjectWhenHitDuration",	INI::parseDurationUnsignedInt,	 NULL,		offsetof(EnergyShieldBehaviorModuleData, m_shieldSubObjHitDuration) },
		//{ "AlwaysShowShieldSubObject",	INI::parseBool,	 NULL,		offsetof(EnergyShieldBehaviorModuleData, m_alwaysShowShieldSubObj) },

		 { "ShieldModelCondition",	INI::parseIndexList,		ModelConditionFlags::getBitNames(),		offsetof(EnergyShieldBehaviorModuleData, m_shieldConditionFlag) },
		 { "ShieldHitModelCondition",	INI::parseIndexList,		ModelConditionFlags::getBitNames(),		offsetof(EnergyShieldBehaviorModuleData, m_shieldHitConditionFlag) },
		 { "ShieldHitConditionDuration",	INI::parseDurationUnsignedInt,	 NULL,		offsetof(EnergyShieldBehaviorModuleData, m_showShieldWhenHitDuration) },
		 //{ "AlwaysShowShield",	INI::parseBool,	 NULL,		offsetof(EnergyShieldBehaviorModuleData, m_alwaysShowShield) },

		{ "ShieldUpFX",	INI::parseFXList,	 NULL,		offsetof(EnergyShieldBehaviorModuleData, m_fxShieldUp) },
		{ "ShieldDownFX",	INI::parseFXList,	 NULL,		offsetof(EnergyShieldBehaviorModuleData, m_fxShieldDown) },

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
	DEBUG_ASSERTCRASH((body != nullptr), ("EnergyShieldBehavior requires ShieldBody!"));

	m_body = body;

	const EnergyShieldBehaviorModuleData *d = getEnergyShieldBehaviorModuleData();

	if (d->m_initiallyActive)
	{
		giveSelfUpgrade();
	}

	setWakeFrame(getObject(), UPDATE_SLEEP_FOREVER);  // We wake when we get attacked
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
	DEBUG_LOG((">>> EnergyShieldBehavior::applyDamage -  amount = %f", amount));

	const EnergyShieldBehaviorModuleData* data = getEnergyShieldBehaviorModuleData();
	m_healingStepCountdown = data->m_shieldRechargeDelay;
	setWakeFrame(getObject(), UPDATE_SLEEP_NONE);

	// amount = 0 means the shield was already down

	if (amount > 0)
	{
		// Show shield subobject
		if (data->m_shieldSubObjName.isNotEmpty() || data->m_shieldConditionFlag != MODELCONDITION_INVALID) {

			if (getShieldPercent() <= 0) { // Shield is down, hide it

				showShield(false); // shield
				showShield(false, true);  // shield hit

				FXList::doFXObj(data->m_fxShieldDown, getObject());

				m_shieldHitCountdown = 0;
			}
			else if (data->m_showShieldWhenHitDuration > 0) { // Shield took damage, show it
				if (m_shieldHitCountdown <= 0) {  // If it's not already up, show it.
					showShield(true, true);
					m_shieldHitCountdown = data->m_showShieldWhenHitDuration;
				}
			}
		}
	}
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void EnergyShieldBehavior::showShield(bool show, bool isHit)
{
	Drawable* draw = getObject()->getDrawable();
	if (!draw)
		return;

	const EnergyShieldBehaviorModuleData* data = getEnergyShieldBehaviorModuleData();

	AsciiString subObjName;
	ModelConditionFlagType conditionFlag;

	if (isHit) {
		subObjName = data->m_shieldHitSubObjName;
		conditionFlag = data->m_shieldHitConditionFlag;
	}
	else {
		subObjName = data->m_shieldSubObjName;
		conditionFlag = data->m_shieldConditionFlag;
	}

	if (show) {
		if (conditionFlag != MODELCONDITION_INVALID) {
			draw->setModelConditionState(conditionFlag);
		}

		if (subObjName.isNotEmpty()) {
			draw->showSubObject(subObjName, true);
			draw->updateSubObjects();
		}
	}
	else {
		if (conditionFlag != MODELCONDITION_INVALID) {
			draw->clearModelConditionState(conditionFlag);
		}

		if (subObjName.isNotEmpty()) {
			draw->showSubObject(subObjName, false);
			draw->updateSubObjects();
		}
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Real EnergyShieldBehavior::getShieldPercent() const
{
	if (isActive() && m_body != NULL)
	{
		return m_body->getShieldPercent();
	}
	return 0.0;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
bool EnergyShieldBehavior::shouldShowHealthBar(bool selected) const
{
	if (!isActive() || m_body == NULL)
		return FALSE;

	const EnergyShieldBehaviorModuleData* data = getEnergyShieldBehaviorModuleData();

	if (!selected && !getEnergyShieldBehaviorModuleData()->m_showBarWhenUnselected)
		return FALSE;

	if (!data->m_showBarWhenEmpty && m_body->getShieldCurrentHealth() <= 0.0f)
		return FALSE;

	return TRUE;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool EnergyShieldBehavior::alwaysShowShield() const
{
	return getEnergyShieldBehaviorModuleData()->m_shieldConditionFlag != MODELCONDITION_INVALID ||
		getEnergyShieldBehaviorModuleData()->m_shieldSubObjName.isNotEmpty();
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void EnergyShieldBehavior::upgradeImplementation()
{
	if (m_body) {
		m_body->setActive(true);
		m_body->rechargeShieldHealth(m_body->getShieldMaxHealth());

		// if (alwaysShowShield()) {
		showShield(true, false);
		//}

	}
	//setWakeFrame(getObject(), UPDATE_SLEEP_NONE);
}


//-------------------------------------------------------------------------------------------------
/** The update callback. */
//-------------------------------------------------------------------------------------------------
UpdateSleepTime EnergyShieldBehavior::update( void )
{
		DEBUG_LOG((">>> EnergyShieldBehavior::update1 -  m_healingStepCountdown = %d", m_healingStepCountdown));
	
		//UnsignedInt now = TheGameLogic->getFrame();
		//UnsignedInt damageFrame = getLastDamageTimestamp();
		//const ShieldBodyModuleData* data = getShieldBodyModuleData();
		//if (now < damageFrame + data->m_shieldRechargeDelay) {
		//	return UPDATE_SLEEP_NONE;
		//}

		const EnergyShieldBehaviorModuleData* data = getEnergyShieldBehaviorModuleData();

		if (m_shieldHitCountdown > 0 && --m_shieldHitCountdown == 0) {
			//if (!data->m_alwaysShowShield)
			showShield(false, true);
		}

		if (m_healingStepCountdown > 0 && --m_healingStepCountdown == 0) {

			if (m_body) {
				Real rechargeAmount;
				if (data->m_shieldRechargeAmountPercent) {
					rechargeAmount = data->m_shieldRechargeAmountPercent * m_body->getShieldMaxHealth();
				}
				else {
					rechargeAmount = data->m_shieldRechargeAmount;
				}
				DEBUG_LOG((">>> EnergyShieldBehavior::update2 -  rechargeAmount = %f", rechargeAmount));

				if (m_body->getShieldCurrentHealth() <= 0)
					FXList::doFXObj(data->m_fxShieldUp, getObject());

				Bool full = m_body->rechargeShieldHealth(rechargeAmount);
				showShield(true, false);

				if (!full)
					m_healingStepCountdown = data->m_shieldRechargeRate;

				//Bool full = m_body->rechargeShieldHealth(rechargeAmount);
				//if (full)
				//	return UPDATE_SLEEP_FOREVER;
				//else
				//	return UPDATE_SLEEP_NONE;
			}
		}

		if (m_healingStepCountdown == 0 && m_shieldHitCountdown == 0)
			return UPDATE_SLEEP_FOREVER;
		else
			return UPDATE_SLEEP_NONE;
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

	//hit frames
	xfer->xferUnsignedInt(&m_shieldHitCountdown);

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
