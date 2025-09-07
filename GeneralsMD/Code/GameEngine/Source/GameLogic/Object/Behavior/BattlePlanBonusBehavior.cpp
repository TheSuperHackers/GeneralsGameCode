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

// FILE: BattlePlanBonusBehavior.cpp ///////////////////////////////////////////////////////////////////////
// Author:
// Desc:  
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#define DEFINE_BATTLEPLANSTATUS_NAMES
#define DEFINE_WEAPONBONUSCONDITION_NAMES

#include "Common/INI.h"
#include "Common/Xfer.h"
#include "Common/Player.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/BattlePlanBonusBehavior.h"
#include "GameLogic/Module/BattlePlanUpdate.h"
#include "GameLogic/Object.h"
#include "GameLogic/Weapon.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/AIUpdate.h"
//#include "Common/ObjectStatusTypes.h"


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
BattlePlanBonusBehaviorModuleData::BattlePlanBonusBehaviorModuleData()
{
	m_initiallyActive = false;
	m_shouldParalyze = true;
	for (UnsignedInt i = 0; i < BATTLE_PLAN_COUNT; i++) {
		m_weaponBonusEntries[i] = WEAPONBONUSCONDITION_INVALID;
		m_weaponSetFlagEntries[i] = WEAPONSET_NONE;
		m_armorSetFlagEntries[i] = ARMORSET_NONE;
		m_armorDamageScalarEntries[i] = 1.0;
		m_sightRangeScalarEntries[i] = 1.0;
		//m_movementSpeedScalarEntries[i] = 1.0;
		m_statusToSetEntries[i] = OBJECT_STATUS_NONE;
		m_statusToClearEntries[i] = OBJECT_STATUS_NONE;
	}
}
//-------------------------------------------------------------------------------------------------
void BattlePlanBonusBehaviorModuleData::parseBPWeaponBonus(INI* ini, void* instance, void* store, const void* userData)
{
	BattlePlanBonusBehaviorModuleData* self = (BattlePlanBonusBehaviorModuleData*)instance;
	BattlePlanStatus plan = (BattlePlanStatus)INI::scanIndexList(ini->getNextToken(), TheBattlePlanStatusNames);
	if ((plan) == PLANSTATUS_NONE)
		return;

	self->m_weaponBonusEntries[plan-1] = (WeaponBonusConditionType)INI::scanIndexList(ini->getNextToken(), TheWeaponBonusNames);
}
//-------------------------------------------------------------------------------------------------
void BattlePlanBonusBehaviorModuleData::parseBPWeaponSetFlag(INI* ini, void* instance, void* store, const void* userData)
{
	BattlePlanBonusBehaviorModuleData* self = (BattlePlanBonusBehaviorModuleData*)instance;
	BattlePlanStatus plan = (BattlePlanStatus)INI::scanIndexList(ini->getNextToken(), TheBattlePlanStatusNames);
	if ((plan) == PLANSTATUS_NONE)
		return;

	self->m_weaponSetFlagEntries[plan - 1] = (WeaponSetType)INI::scanIndexList(ini->getNextToken(), WeaponSetFlags::getBitNames());
}
//-------------------------------------------------------------------------------------------------
void BattlePlanBonusBehaviorModuleData::parseBPArmorSetFlag(INI* ini, void* instance, void* store, const void* userData)
{
	BattlePlanBonusBehaviorModuleData* self = (BattlePlanBonusBehaviorModuleData*)instance;
	BattlePlanStatus plan = (BattlePlanStatus)INI::scanIndexList(ini->getNextToken(), TheBattlePlanStatusNames);
	if ((plan) == PLANSTATUS_NONE)
		return;

	self->m_armorSetFlagEntries[plan - 1] = (ArmorSetType)INI::scanIndexList(ini->getNextToken(), ArmorSetFlags::getBitNames());
}
//-------------------------------------------------------------------------------------------------
void BattlePlanBonusBehaviorModuleData::parseBPArmorDamageScalar(INI* ini, void* instance, void* store, const void* userData)
{
	BattlePlanBonusBehaviorModuleData* self = (BattlePlanBonusBehaviorModuleData*)instance;
	BattlePlanStatus plan = (BattlePlanStatus)INI::scanIndexList(ini->getNextToken(), TheBattlePlanStatusNames);
	if ((plan) == PLANSTATUS_NONE)
		return;

	self->m_armorDamageScalarEntries[plan - 1] = INI::scanReal(ini->getNextToken());
}
//-------------------------------------------------------------------------------------------------
void BattlePlanBonusBehaviorModuleData::parseBPSightRangeScalar(INI* ini, void* instance, void* store, const void* userData)
{
	BattlePlanBonusBehaviorModuleData* self = (BattlePlanBonusBehaviorModuleData*)instance;
	BattlePlanStatus plan = (BattlePlanStatus)INI::scanIndexList(ini->getNextToken(), TheBattlePlanStatusNames);
	if ((plan) == PLANSTATUS_NONE)
		return;

	self->m_sightRangeScalarEntries[plan - 1] = INI::scanReal(ini->getNextToken());
}
//-------------------------------------------------------------------------------------------------
//void BattlePlanBonusBehaviorModuleData::parseBPMovementSpeedScalar(INI* ini, void* instance, void* store, const void* userData)
//{
//	BattlePlanBonusBehaviorModuleData* self = (BattlePlanBonusBehaviorModuleData*)instance;
//	BattlePlanStatus plan = (BattlePlanStatus)INI::scanIndexList(ini->getNextToken(), TheBattlePlanStatusNames);
//	if ((plan) == PLANSTATUS_NONE)
//		return;
//
//	self->m_movementSpeedScalarEntries[plan - 1] = INI::scanReal(ini->getNextToken());
//}
//-------------------------------------------------------------------------------------------------
void BattlePlanBonusBehaviorModuleData::parseBPStatusToSet(INI* ini, void* instance, void* store, const void* userData)
{
	BattlePlanBonusBehaviorModuleData* self = (BattlePlanBonusBehaviorModuleData*)instance;
	BattlePlanStatus plan = (BattlePlanStatus)INI::scanIndexList(ini->getNextToken(), TheBattlePlanStatusNames);
	if ((plan) == PLANSTATUS_NONE)
		return;

	self->m_statusToSetEntries[plan - 1] = (ObjectStatusTypes)INI::scanIndexList(ini->getNextToken(), ObjectStatusMaskType::getBitNames());
}
//-------------------------------------------------------------------------------------------------
void BattlePlanBonusBehaviorModuleData::parseBPStatusToClear(INI* ini, void* instance, void* store, const void* userData)
{
	BattlePlanBonusBehaviorModuleData* self = (BattlePlanBonusBehaviorModuleData*)instance;
	BattlePlanStatus plan = (BattlePlanStatus)INI::scanIndexList(ini->getNextToken(), TheBattlePlanStatusNames);
	if ((plan) == PLANSTATUS_NONE)
		return;

	self->m_statusToClearEntries[plan - 1] = (ObjectStatusTypes)INI::scanIndexList(ini->getNextToken(), ObjectStatusMaskType::getBitNames());
}

//-------------------------------------------------------------------------------------------------
/*static*/ void BattlePlanBonusBehaviorModuleData::buildFieldParse(MultiIniFieldParse& p)
{
	static const FieldParse dataFieldParse[] =
	{
		{ "StartsActive",	INI::parseBool, NULL, offsetof(BattlePlanBonusBehaviorModuleData, m_initiallyActive) },
		{ "OverrideGlobalBonus",	INI::parseBool, NULL, offsetof(BattlePlanBonusBehaviorModuleData, m_overrideGlobal) },
		{ "ShouldParalyzeOnPlanChange",	INI::parseBool, NULL, offsetof(BattlePlanBonusBehaviorModuleData, m_shouldParalyze) },

		{ "WeaponBonus",	parseBPWeaponBonus, NULL, 0 },
		{ "WeaponSet",	parseBPWeaponSetFlag, NULL, 0 },
		{ "ArmorSet",	parseBPArmorSetFlag, NULL, 0 },
		{ "ArmorDamageScalar",	parseBPArmorDamageScalar, NULL, 0 },
		{ "SightRangeScalar",	parseBPSightRangeScalar, NULL, 0 },
		//{ "MovementSpeedScalar",	parseBPMovementSpeedScalar, NULL, 0 },
		{ "StatusToSet",	parseBPStatusToSet, NULL, 0 },
		{ "StatusToClear",	parseBPStatusToClear, NULL, 0 },
		
		{ 0, 0, 0, 0 }
	};

	BehaviorModuleData::buildFieldParse(p);
	p.add(dataFieldParse);
	p.add(UpgradeMuxData::getFieldParse(), offsetof(BattlePlanBonusBehaviorModuleData, m_upgradeMuxData));
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
BattlePlanBonusBehavior::BattlePlanBonusBehavior(Thing* thing, const ModuleData* moduleData) : BehaviorModule(thing, moduleData)
{
	if (getBattlePlanBonusBehaviorModuleData()->m_initiallyActive)
	{
		giveSelfUpgrade();
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
BattlePlanBonusBehavior::~BattlePlanBonusBehavior(void)
{
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void BattlePlanBonusBehavior::upgradeImplementation(void)
{
	// DEBUG_LOG(("### BattlePlanBonusBehavior(%s)::upgradeImplementation 0", KEYNAME(getModuleTagNameKey()).str()));

	// Get other BattlePlanBonusBehavior and remove their active effects if they are conflicting
	Object* obj = getObject();

	for (BehaviorModule** b = obj->getBehaviorModules(); *b; ++b)
	{
		BattlePlanBonusBehaviorInterface* bpbi = (*b)->getBattlePlanBonusBehaviorInterface();
		if (bpbi && bpbi != this) {
			if (bpbi->isConflicting() && bpbi->isAnyEffectApplied()) {
				DEBUG_LOG(("### BattlePlanBonusBehavior(%s)::upgradeImplementation 1. Try to remove effects of conflicting module", KEYNAME(getModuleTagNameKey()).str()));
				bpbi->removeActiveEffects();
			}
		}
	}


	// Note: this doesn't handle the Regular bonus being applied/removed Instead of Applying our bonus, we should instead just reapply all.
	Player* player = obj->getControllingPlayer();
	const BattlePlanBonuses* bonus = player->getBattlePlanBonuses();
	if (bonus) {
		player->removeBattlePlanBonusesForObject(obj);
		player->applyBattlePlanBonusesForObject(obj);

		// We still need to apply our bonus, because at this point we do not count as Upgraded
		// Check for valid KindOf. This shouldn't be needed unless you screwed up your INI, but just to be safe
		if (player->doesObjectQualifyForBattlePlan(obj)) {
			applyBonus(bonus, FALSE);
		}
	}

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void BattlePlanBonusBehavior::applyBonus(const BattlePlanBonuses* bonus, bool checkIsValid /* = True*/)
{
	//DEBUG_LOG(("---\n"));
	//DEBUG_LOG(("### BattlePlanBonusBehavior(%s)::applyBonus", KEYNAME(getModuleTagNameKey()).str()));
	//DEBUG_LOG(("- m_bombardment = %d", bonus->m_bombardment));
	//DEBUG_LOG(("- m_holdTheLine = %d", bonus->m_holdTheLine));
	//DEBUG_LOG(("- m_searchAndDestroy = %d", bonus->m_searchAndDestroy));
	//DEBUG_LOG(("---\n"));


	//const BattlePlanBonusBehaviorModuleData* d = getBattlePlanBonusBehaviorModuleData();
	if (!checkIsValid || isEffectValid()) {

		//DEBUG_LOG(("### BattlePlanBonusBehavior(%s)::applyBonus - valid", KEYNAME(getModuleTagNameKey()).str()));

		//Object* obj = getObject();

		if (bonus->m_bombardment >= 1) {
			addBonusForType(PLANSTATUS_BOMBARDMENT);
		}
		else if (bonus->m_bombardment <= -1) {
			removeBonusForType(PLANSTATUS_BOMBARDMENT);
		}

		if (bonus->m_holdTheLine >= 1) {
			addBonusForType(PLANSTATUS_HOLDTHELINE);
		}
		else if (bonus->m_holdTheLine <= -1) {
			removeBonusForType(PLANSTATUS_HOLDTHELINE);
		}

		if (bonus->m_searchAndDestroy >= 1) {
			addBonusForType(PLANSTATUS_SEARCHANDDESTROY);
		}
		else if (bonus->m_searchAndDestroy <= -1) {
			removeBonusForType(PLANSTATUS_SEARCHANDDESTROY);
		}
	}
	/*else {
		DEBUG_LOG(("### BattlePlanBonusBehavior(%s)::applyBonus - Not Valid.", KEYNAME(getModuleTagNameKey()).str()));
	}*/
	DEBUG_LOG(("---\n"));
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void BattlePlanBonusBehavior::addBonusForType(BattlePlanStatus plan)
{
	// DEBUG_LOG(("### BattlePlanBonusBehavior(%s)::addBonusForType %d", KEYNAME(getModuleTagNameKey()).str(), plan));
	const BattlePlanBonusBehaviorModuleData* d = getBattlePlanBonusBehaviorModuleData();
	Object* obj = getObject();
	int idx = plan - 1;

	//Sanity check:
	DEBUG_ASSERTCRASH(!m_effectApplied[idx], ("BattlePlanBonusBehavior::addBonusForType -- Bonus is currently active!"));

	if (idx > BATTLE_PLAN_COUNT)
		return;
	if (d->m_weaponBonusEntries[idx] != -1) {
		obj->setWeaponBonusCondition(d->m_weaponBonusEntries[idx]);
	}
	if (d->m_weaponSetFlagEntries[idx] != -1) {
		obj->setWeaponSetFlag(d->m_weaponSetFlagEntries[idx]);
	}
	if (d->m_armorSetFlagEntries[idx] != -1) {
		BodyModuleInterface* body = obj->getBodyModule();
		if (body)
			body->setArmorSetFlag(d->m_armorSetFlagEntries[idx]);
	}
	if (d->m_armorDamageScalarEntries[idx] != 1.0) {
		BodyModuleInterface* body = obj->getBodyModule();
		if (body)
			body->applyDamageScalar(d->m_armorDamageScalarEntries[idx]);
	}
	if (d->m_sightRangeScalarEntries[idx] != 1.0) {
		obj->setVisionRange(obj->getVisionRange() * d->m_sightRangeScalarEntries[idx]);
		obj->setShroudClearingRange(obj->getShroudClearingRange() * d->m_sightRangeScalarEntries[idx]);
	}
	/*if (d->m_movementSpeedScalarEntries[idx] != 1.0) {
		AIUpdateInterface* ai = obj->getAI();
		if (ai) {
			Locomotor* loco = ai->getCurLocomotor();
			loco->applySpeedMultiplier(d->m_movementSpeedScalarEntries[idx]);
		}
	}*/
	if (d->m_statusToSetEntries[idx] != -1) {
		obj->setStatus(MAKE_OBJECT_STATUS_MASK(d->m_statusToSetEntries[idx]));
	}
	if (d->m_statusToClearEntries[idx] != -1) {
		obj->setStatus(MAKE_OBJECT_STATUS_MASK(d->m_statusToClearEntries[idx]), FALSE);
	}

	m_effectApplied[idx] = TRUE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void BattlePlanBonusBehavior::removeBonusForType(BattlePlanStatus plan)
{
	// DEBUG_LOG(("### BattlePlanBonusBehavior(%s)::removeBonusForType %d", KEYNAME(getModuleTagNameKey()).str(), plan));

	const BattlePlanBonusBehaviorModuleData* d = getBattlePlanBonusBehaviorModuleData();
	Object* obj = getObject();
	int idx = plan - 1;

	DEBUG_ASSERTCRASH(m_effectApplied[idx], ("BattlePlanBonusBehavior::removeBonusForType -- Bonus is currently NOT active!"));

	if (idx > BATTLE_PLAN_COUNT)
		return;
	if (d->m_weaponBonusEntries[idx] != -1) {
		obj->clearWeaponBonusCondition(d->m_weaponBonusEntries[idx]);
	}
	if (d->m_weaponSetFlagEntries[idx] != -1) {
		obj->clearWeaponSetFlag(d->m_weaponSetFlagEntries[idx]);
	}
	if (d->m_armorSetFlagEntries[idx] != -1) {
		BodyModuleInterface* body = obj->getBodyModule();
		if (body)
			body->clearArmorSetFlag(d->m_armorSetFlagEntries[idx]);
	}
	if (d->m_armorDamageScalarEntries[idx] != 1.0) {
		BodyModuleInterface* body = obj->getBodyModule();
		if (body)
			body->applyDamageScalar(1.0f / __max(d->m_armorDamageScalarEntries[idx], 0.01f));
	}
	if (d->m_sightRangeScalarEntries[idx] != 1.0) {
		Real sightRangeScalar = 1.0f / __max(d->m_sightRangeScalarEntries[idx], 0.01f);
		obj->setVisionRange(obj->getVisionRange() * sightRangeScalar);
		obj->setShroudClearingRange(obj->getShroudClearingRange() * sightRangeScalar);
	}
	/*if (d->m_movementSpeedScalarEntries[idx] != 1.0) {
		AIUpdateInterface* ai = obj->getAI();
		if (ai) {
			Locomotor* loco = ai->getCurLocomotor();
			loco->applySpeedMultiplier(d->m_movementSpeedScalarEntries[idx]);
		}
	}*/
	if (d->m_statusToSetEntries[idx] != -1) {
		obj->setStatus(MAKE_OBJECT_STATUS_MASK(d->m_statusToSetEntries[idx]), FALSE);
	}
	if (d->m_statusToClearEntries[idx] != -1) {
		obj->setStatus(MAKE_OBJECT_STATUS_MASK(d->m_statusToClearEntries[idx]));
	}

	m_effectApplied[idx] = FALSE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool BattlePlanBonusBehavior::shouldParalyze(void) const
{
	// DEBUG_LOG(("### BattlePlanBonusBehavior(%s)::shouldParalyze ?", KEYNAME(getModuleTagNameKey()).str()));
	if (isEffectValid()) {
		Bool shouldParalyze = getBattlePlanBonusBehaviorModuleData()->m_shouldParalyze;
		// DEBUG_LOG(("### BattlePlanBonusBehavior(%s)::shouldParalyze = %d!!!", KEYNAME(getModuleTagNameKey()).str(), shouldParalyze));
		return shouldParalyze;
	}
	return TRUE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool BattlePlanBonusBehavior::isOverrideGlobalBonus(void) const
{
	if (isEffectValid()) {
		return getBattlePlanBonusBehaviorModuleData()->m_overrideGlobal;
	}
	return FALSE;
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool BattlePlanBonusBehavior::isConflicting(void) const
{
	// Check for conflicting upgrades
	UpgradeMaskType activation, conflicting;
	getUpgradeActivationMasks(activation, conflicting);
	const Object* obj = getObject();
	if (obj->getObjectCompletedUpgradeMask().testForAny(conflicting))
	{
		return TRUE;
	}
	if (obj->getControllingPlayer() && obj->getControllingPlayer()->getCompletedUpgradeMask().testForAny(conflicting))
	{
		return TRUE;
	}
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void BattlePlanBonusBehavior::removeActiveEffects(void)
{
	// DEBUG_LOG(("### BattlePlanBonusBehavior(%s)::removeActiveEffects 0", KEYNAME(getModuleTagNameKey()).str()));

	for (int i = 0; i < BATTLE_PLAN_COUNT; i++) {
		if (m_effectApplied[i]) {
			BattlePlanStatus plan = (BattlePlanStatus)(i + 1);
			removeBonusForType(plan);
		}
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool BattlePlanBonusBehavior::isEffectValid(void) const
{
	return isUpgradeActive() && !isConflicting();
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
Bool BattlePlanBonusBehavior::isAnyEffectApplied(void) const
{
	for (int i = 0; i < BATTLE_PLAN_COUNT; i++) {
		if (m_effectApplied[i]) {
			return TRUE;
		}
	}
	return FALSE;
}

// ------------------------------------------------------------------------------------------------
/** CRC */
// ------------------------------------------------------------------------------------------------
void BattlePlanBonusBehavior::crc(Xfer* xfer)
{

	// extend base class
	BehaviorModule::crc(xfer);

	// extend upgrade mux
	UpgradeMux::upgradeMuxCRC(xfer);

}  // end crc

// ------------------------------------------------------------------------------------------------
/** Xfer method
	* Version Info:
	* 1: Initial version */
	// ------------------------------------------------------------------------------------------------
void BattlePlanBonusBehavior::xfer(Xfer* xfer)
{

	// version
	XferVersion currentVersion = 1;
	XferVersion version = currentVersion;
	xfer->xferVersion(&version, currentVersion);

	// extend base class
	BehaviorModule::xfer(xfer);

	// extend upgrade mux
	UpgradeMux::upgradeMuxXfer(xfer);

	//// trigger frame
	//xfer->xferUnsignedInt(&m_triggerFrame);

	// m_effectApplied
	//xfer->xferBool(&m_effectApplied);
	// This is now an array
	//if (xfer->getXferMode() == XFER_SAVE)
	//{
		for (int i = 0; i < BATTLE_PLAN_COUNT; i++) {
			xfer->xferBool(&m_effectApplied[i]);
		}
	//}

}  // end xfer

// ------------------------------------------------------------------------------------------------
/** Load post process */
// ------------------------------------------------------------------------------------------------
void BattlePlanBonusBehavior::loadPostProcess(void)
{

	// extend base class
	BehaviorModule::loadPostProcess();

	// extend upgrade mux
	UpgradeMux::upgradeMuxLoadPostProcess();

}  // end loadPostProcess
