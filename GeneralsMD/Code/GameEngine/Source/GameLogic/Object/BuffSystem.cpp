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

// FILE: BuffSystem.cpp ///////////////////////////////////////////////////////////////////////////////
// Author: Andi W, October 25
// Desc:   Buff/Debuff effects
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameLogic/Module/BuffEffectHelper.h"

#define DEFINE_WEAPONBONUSCONDITION_NAMES

#include "Common/GlobalData.h"
#include "GameLogic/Weapon.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/BuffSystem.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameClient/TintStatus.h"
#include "GameClient/Drawable.h"
#include "GameClient/ParticleSys.h"



///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC DATA ////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// BUFF EFFECT NUGGETS AND ALL THEIR PARSE FUNCTIONS GO HERE:
// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// GAMEPLAY EFFECTS / MODIFIER NUGGETS GO HERE:
// ------------------------------------------------------------------------------------------------


// Limit multipliers to [0.01, 100.0] to avoid numerical instability
static void parseMultiplier(INI* ini, void* /*instance*/, void* store, const void* /*userData*/)
{
	Real value = INI::scanReal(ini->getNextToken());
	*(Real*)store = __min(__max(value, 0.01f), 100.0f);
}

//-------------------------------------------------------------------------------------------------
class ValueModifierBuffEffectNugget : public BuffEffectNugget
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(ValueModifierBuffEffectNugget, "ValueModifierBuffEffectNugget")
public:

	ValueModifierBuffEffectNugget()
	{
		m_armorDamageScalar = 1.0f;
		m_sightRangeScalar = 1.0f;
		m_moveSpeedScalar = 1.0f;
	}

	virtual void apply(Object* targetObj, const Object* sourceObj, BuffEffectTracker* buffTracker) const
	{
		DEBUG_LOG(("ValueModifierBuffEffectNugget::apply 0"));

		if (m_armorDamageScalar != 1.0) {
			BodyModuleInterface* body = targetObj->getBodyModule();
			if (body) {
				body->applyDamageScalar(m_armorDamageScalar);
			}
		}

		if (m_sightRangeScalar != 1.0) {
			targetObj->setVisionRange(targetObj->getVisionRange() * m_sightRangeScalar);
			targetObj->setShroudClearingRange(targetObj->getShroudClearingRange() * m_sightRangeScalar);
		}

		if (m_moveSpeedScalar != 1.0) {
			AIUpdateInterface* ai = targetObj->getAI();
			if (ai) {
				ai->applySpeedMultiplier(m_moveSpeedScalar);
			}
		}
	}

	virtual void remove(Object* targetObj, BuffEffectTracker* buffTracker) const
	{
		DEBUG_LOG(("ValueModifierBuffEffectNugget::remove 0"));

		if (m_armorDamageScalar != 1.0) {
			BodyModuleInterface* body = targetObj->getBodyModule();
			if (body) {
				body->applyDamageScalar(1.0f / m_armorDamageScalar);
			}
		}

		if (m_sightRangeScalar != 1.0) {
			Real scalar = 1.0f / m_sightRangeScalar;
			targetObj->setVisionRange(targetObj->getVisionRange() * scalar);
			targetObj->setShroudClearingRange(targetObj->getShroudClearingRange() * scalar);
		}

		if (m_moveSpeedScalar != 1.0) {
			AIUpdateInterface* ai = targetObj->getAI();
			if (ai) {
				ai->applySpeedMultiplier(1.0f / m_moveSpeedScalar);
			}
		}

	}

	static void parse(INI* ini, void* instance, void* /*store*/, const void* /*userData*/)
	{
		static const FieldParse myFieldParse[] =
		{
			{ "MovementSpeedScalar", parseMultiplier, NULL, offsetof(ValueModifierBuffEffectNugget, m_moveSpeedScalar) },
			{ "ArmorDamageScalar", parseMultiplier, NULL, offsetof(ValueModifierBuffEffectNugget, m_armorDamageScalar) },
			{ "SightRangeScalar", parseMultiplier, NULL, offsetof(ValueModifierBuffEffectNugget, m_sightRangeScalar) },
			{ 0, 0, 0, 0 }
		};

		MultiIniFieldParse p;
		p.add(myFieldParse);

		ValueModifierBuffEffectNugget* nugget = newInstance(ValueModifierBuffEffectNugget);

		ini->initFromINIMulti(nugget, p);

		((BuffTemplate*)instance)->addBuffEffectNugget(nugget);
	}

private:
	Real m_moveSpeedScalar;
	Real m_armorDamageScalar;
	Real m_sightRangeScalar;

};
EMPTY_DTOR(ValueModifierBuffEffectNugget)


//-------------------------------------------------------------------------------------------------
class FlagModifierBuffEffectNugget : public BuffEffectNugget
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(FlagModifierBuffEffectNugget, "FlagModifierBuffEffectNugget")
public:

	FlagModifierBuffEffectNugget()
	{
		m_bonusType = WEAPONBONUSCONDITION_INVALID;
		m_bonusTypeAgainst = WEAPONBONUSCONDITION_INVALID;
		m_weaponSetFlag = WEAPONSET_NONE;
		m_armorSetFlag = ARMORSET_NONE;

		m_statusToSet.clear();
		m_statusToClear.clear();
	}

	virtual void apply(Object* targetObj, const Object* sourceObj, BuffEffectTracker* buffTracker) const
	{
		DEBUG_LOG(("FlagModifierBuffEffectNugget::apply 0"));

		if (m_bonusType != WEAPONBONUSCONDITION_INVALID) {
			targetObj->setWeaponBonusCondition(m_bonusType);
		}

		if (m_bonusTypeAgainst != WEAPONBONUSCONDITION_INVALID) {
			targetObj->setWeaponBonusConditionAgainst(m_bonusTypeAgainst);
		}

		if (m_weaponSetFlag != WEAPONSET_NONE) {
			targetObj->setWeaponSetFlag(m_weaponSetFlag);
		}

		if (m_armorSetFlag != ARMORSET_NONE) {
			targetObj->setArmorSetFlag(m_armorSetFlag);
		}

		// Any check needed, or just run it anyways?
		targetObj->setStatus(m_statusToSet);
		targetObj->clearStatus(m_statusToClear);

	}

	virtual void remove(Object* targetObj, BuffEffectTracker* buffTracker) const
	{
		DEBUG_LOG(("FlagModifierBuffEffectNugget::remove 0"));

		if (m_bonusType != WEAPONBONUSCONDITION_INVALID) {
			targetObj->clearWeaponBonusCondition(m_bonusType);
		}

		if (m_bonusTypeAgainst != WEAPONBONUSCONDITION_INVALID) {
			targetObj->clearWeaponBonusConditionAgainst(m_bonusTypeAgainst);
		}

		if (m_weaponSetFlag != WEAPONSET_NONE) {
			targetObj->clearWeaponSetFlag(m_weaponSetFlag);
		}

		if (m_armorSetFlag != ARMORSET_NONE) {
			targetObj->clearArmorSetFlag(m_armorSetFlag);
		}

		// Any check needed, or just run it anyways?
		targetObj->clearStatus(m_statusToSet);
		targetObj->setStatus(m_statusToClear);

	}

	static void parse(INI* ini, void* instance, void* /*store*/, const void* /*userData*/)
	{
		static const FieldParse myFieldParse[] =
		{
			{ "WeaponBonus", INI::parseIndexList,	TheWeaponBonusNames, offsetof(FlagModifierBuffEffectNugget, m_bonusType) },
			{ "WeaponBonusAgainst", INI::parseIndexList,	TheWeaponBonusNames, offsetof(FlagModifierBuffEffectNugget, m_bonusTypeAgainst) },
			{ "WeaponSetFlag", INI::parseIndexList, WeaponSetFlags::getBitNames(), offsetof(FlagModifierBuffEffectNugget, m_weaponSetFlag) },
			{ "ArmorSetFlag", INI::parseIndexList,	ArmorSetFlags::getBitNames(), offsetof(FlagModifierBuffEffectNugget, m_weaponSetFlag) },
			{ "StatusToSet", ObjectStatusMaskType::parseFromINI,	NULL, offsetof(FlagModifierBuffEffectNugget, m_statusToSet) },
			{ "StatusToClear", ObjectStatusMaskType::parseFromINI,	NULL, offsetof(FlagModifierBuffEffectNugget, m_statusToClear) },
			{ 0, 0, 0, 0 }
		};

		MultiIniFieldParse p;
		p.add(myFieldParse);

		FlagModifierBuffEffectNugget* nugget = newInstance(FlagModifierBuffEffectNugget);

		ini->initFromINIMulti(nugget, p);

		((BuffTemplate*)instance)->addBuffEffectNugget(nugget);
	}

private:
	WeaponBonusConditionType	m_bonusType;         ///< weapon bonus granted to the object
	WeaponBonusConditionType	m_bonusTypeAgainst;  ///< weapon bonus granted when attacking the object
	WeaponSetType m_weaponSetFlag;                 ///< the weaponset flag to set
	ArmorSetType m_armorSetFlag;                   ///< the armorset flag to set

	ObjectStatusMaskType m_statusToSet;
	ObjectStatusMaskType m_statusToClear;

};
EMPTY_DTOR(FlagModifierBuffEffectNugget)




//-------------------------------------------------------------------------------------------------
// VISUAL / AUDIO EFFECT NUGGETS GO HERE:
//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
class ColorTintBuffEffectNugget : public BuffEffectNugget
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(ColorTintBuffEffectNugget, "ColorTintBuffEffectNugget")
public:

	ColorTintBuffEffectNugget()
	{
		m_tintStatus = TINT_STATUS_INVALID;
	}

	virtual void apply(Object* targetObj, const Object* sourceObj, BuffEffectTracker* buffTracker) const
	{
		Drawable* draw = targetObj->getDrawable();
		if (draw)
		{
			if (m_tintStatus > TINT_STATUS_INVALID && m_tintStatus < TINT_STATUS_COUNT) {
				draw->setTintStatus(m_tintStatus);
			}
		}
	}

	virtual void remove(Object* targetObj, BuffEffectTracker* buffTracker) const
	{
		Drawable* draw = targetObj->getDrawable();
		if (draw)
		{
			if (m_tintStatus > TINT_STATUS_INVALID && m_tintStatus < TINT_STATUS_COUNT) {
				draw->clearTintStatus(m_tintStatus);
			}
		}
	}

	static void parse(INI* ini, void* instance, void* /*store*/, const void* /*userData*/)
	{
		static const FieldParse myFieldParse[] =
		{
			{ "TintStatusType", TintStatusFlags::parseSingleBitFromINI,	NULL, offsetof(ColorTintBuffEffectNugget, m_tintStatus) },
			{ 0, 0, 0, 0 }
		};

		MultiIniFieldParse p;
		p.add(myFieldParse);

		ColorTintBuffEffectNugget* nugget = newInstance(ColorTintBuffEffectNugget);

		ini->initFromINIMulti(nugget, p);

		((BuffTemplate*)instance)->addBuffEffectNugget(nugget);
	}

private:
	TintStatus m_tintStatus;

};
EMPTY_DTOR(ColorTintBuffEffectNugget)


//-------------------------------------------------------------------------------------------------
class ParticleSystemBuffEffectNugget : public BuffEffectNugget
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(ParticleSystemBuffEffectNugget, "ParticleSystemBuffEffectNugget")
public:

	ParticleSystemBuffEffectNugget()
	{
		m_particleSysTemplate = NULL;
	}

	virtual void apply(Object* targetObj, const Object* sourceObj, BuffEffectTracker* buffTracker) const
	{
		ParticleSystem* sys = TheParticleSystemManager->createParticleSystem(m_particleSysTemplate);

		if (sys) {
			sys->attachToObject(targetObj);
			if (buffTracker) {
				DEBUG_LOG(("ParticleSystemBuffEffectNugget::apply - add system %d to buffTracker", sys->getSystemID()));
				buffTracker->addParticleSystem(sys->getSystemID());
			}
			//sys->setSystemLifetime(data->m_bonusDuration);
		}
	}

	virtual void remove(Object* targetObj, BuffEffectTracker* buffTracker) const
	{
		// ParticleSystems are cleared for the whole Buff, not per nugget.
	}

	static void parse(INI* ini, void* instance, void* /*store*/, const void* /*userData*/)
	{
		static const FieldParse myFieldParse[] =
		{
			{ "ParticleSystem", INI::parseParticleSystemTemplate,	NULL, offsetof(ParticleSystemBuffEffectNugget, m_particleSysTemplate) },
			{ 0, 0, 0, 0 }
		};

		MultiIniFieldParse p;
		p.add(myFieldParse);

		ParticleSystemBuffEffectNugget* nugget = newInstance(ParticleSystemBuffEffectNugget);

		ini->initFromINIMulti(nugget, p);

		((BuffTemplate*)instance)->addBuffEffectNugget(nugget);
	}

private:
	const ParticleSystemTemplate* m_particleSysTemplate;

};
EMPTY_DTOR(ParticleSystemBuffEffectNugget)

//-------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
// END BUFF EFFECT NUGGETS
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
const FieldParse BuffTemplate::TheBuffTemplateFieldParse[] =
{
	// Effect Nuggets:
	{ "ValueModifier",			ValueModifierBuffEffectNugget::parse, 0, 0},
	{ "FlagModifier",			FlagModifierBuffEffectNugget::parse, 0, 0},
	{ "ColorTintEffect",		ColorTintBuffEffectNugget::parse, 0, 0},
	{ "ParticleSystemEffect",		ParticleSystemBuffEffectNugget::parse, 0, 0},
	// Generic params:
	{ "MaxStacksSize",		INI::parseUnsignedInt, NULL,  offsetof(BuffTemplate, m_maxStackSize)},
	
	{ NULL, NULL, 0, 0 }  // keep this last
};

//-------------------------------------------------------------------------------------------------
void BuffTemplate::clear()
{
	// do NOT delete the nuggets -- they're owned by the Store.
	 m_nuggets.clear();
}

//-------------------------------------------------------------------------------------------------
void BuffTemplate::addBuffEffectNugget(BuffEffectNugget* nugget)
{
	m_nuggets.push_back(nugget);
	TheBuffTemplateStore->addBuffEffectNugget(nugget);
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

void BuffTemplate::applyEffects(Object* targetObj, Object* sourceObj, BuffEffectTracker* buffTracker) const
{
	for (BuffEffectNugget* buffEffectNugget : m_nuggets) {
		buffEffectNugget->apply(targetObj, sourceObj, buffTracker);
	}
}
//-------------------------------------------------------------------------------------------------


void BuffTemplate::removeEffects(Object* targetObj, BuffEffectTracker* buffTracker) const
{
	for (BuffEffectNugget* buffEffectNugget : m_nuggets) {
		buffEffectNugget->remove(targetObj, buffTracker);
	}

	buffTracker->clearParticleSystems();
}

//-------------------------------------------------------------------------------------------------

UnsignedInt BuffTemplate::getNextTickFrame(UnsignedInt startFrame, UnsignedInt endFrame) const
{
	// TODO: If we have continuos buff effects (e.g. DOT), we need to consider this here.
	return endFrame;
}

//-------------------------------------------------------------------------------------------------
Bool BuffTemplate::hasPriorityOver(AsciiString templateName) const
{
	for (AsciiString str : m_priorityTemplates) {
		if (str == templateName)
			return TRUE;
	}
	return FALSE;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------


BuffTemplateStore* TheBuffTemplateStore = NULL;					///< the TheBuffTemplate store definition
//-------------------------------------------------------------------------------------------------
BuffTemplateStore::BuffTemplateStore()
{
}

//-------------------------------------------------------------------------------------------------
BuffTemplateStore::~BuffTemplateStore()
{
	// Delete all nuggets
	for (BuffEffectNuggetVector::iterator i = m_nuggets.begin(); i != m_nuggets.end(); ++i)
	{
		if (*i)
			deleteInstance(*i);
	}
	m_nuggets.clear();

	// Delete all templates (?)
		// delete all the templates, then clear out the table.
	BuffTemplateMap::iterator it;
	for (it = m_buffTemplates.begin(); it != m_buffTemplates.end(); ++it) {
		deleteInstance(it->second);
	}

	m_buffTemplates.clear();
}

//-------------------------------------------------------------------------------------------------
const BuffTemplate* BuffTemplateStore::findBuffTemplate(const char* name) const
{
	if (stricmp(name, "None") == 0)
		return NULL;

	BuffTemplateMap::const_iterator it = m_buffTemplates.find(NAMEKEY(name));
	if (it == m_buffTemplates.end())
	{
		return NULL;
	}
	else
	{
		return (*it).second;
	}
}

//-------------------------------------------------------------------------------------------------
BuffTemplate* BuffTemplateStore::findBuffTemplate(const char* name)
{
	if (stricmp(name, "None") == 0)
		return NULL;

	BuffTemplateMap::iterator it = m_buffTemplates.find(NAMEKEY(name));
	if (it == m_buffTemplates.end())
	{
		return NULL;
	}
	else
	{
		return (*it).second;
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void BuffTemplateStore::addBuffEffectNugget(BuffEffectNugget* nugget)
{
	m_nuggets.push_back(nugget);
}

//-------------------------------------------------------------------------------------------------
//BuffTemplate* BuffTemplateStore::newBuffTemplate(AsciiString& name)
//{
//	BuffTemplate* buffTemplate = const_cast<BuffTemplate*>(TheBuffTemplateStore->findBuffTemplate(name));
//	if (buffTemplate == NULL) {
//		buffTemplate = newInstance(BuffTemplate)(name);
//
//		if (!m_templateMap.insert(std::make_pair(name, sysTemplate)).second) {
//			deleteInstance(sysTemplate);
//			sysTemplate = NULL;
//		}
//	}
//
//	return buffTemplate;
//}

//-------------------------------------------------------------------------------------------------
///*static */ void BuffTemplateStore::parseBuffTemplateDefinition(INI* ini)
//{
//	AsciiString name;
//
//	// read the BuffTemplate name
//	const char* c = ini->getNextToken();
//	name.set(c);
//
//	// Does this make sense?
//	if (!TheBuffTemplateStore) {
//		return;
//	}
//
//	NameKeyType key = TheNameKeyGenerator->nameToKey(c);
//	BuffTemplate& buffTmp = TheBuffTemplateStore->m_buffTemplates[key];
//	buffTmp.clear();
//	ini->initFromINI(&buffTmp, TheBuffTemplateFieldParse);
//	buffTmp.friend_setName(name);
//
//	DEBUG_LOG((">>> ADDED BUFF TEMPLATE '%s'", buffTmp.getName().str()));
//}

//-------------------------------------------------------------------------------------------------
/*static */ void BuffTemplateStore::parseBuffTemplateDefinition(INI* ini)
{
	AsciiString name;
	BuffTemplate* buffTmp;

	// read the BuffTemplate name
	const char* c = ini->getNextToken();
	name.set(c);

	// Does this make sense?
	if (!TheBuffTemplateStore) {
		return;
	}

	NameKeyType key = TheNameKeyGenerator->nameToKey(c);

	//buffTmp = TheBuffTemplateStore->newBuffTemplate(name);
	buffTmp = newInstance(BuffTemplate);

	buffTmp->clear();
	buffTmp->friend_setName(name);
	ini->initFromINI(buffTmp, buffTmp->getFieldParse());

	TheBuffTemplateStore->m_buffTemplates[key] = buffTmp;

	DEBUG_LOG((">>> ADDED BUFF TEMPLATE '%s'", buffTmp->getName().str()));
}

//-------------------------------------------------------------------------------------------------
/*static*/ void INI::parseBuffTemplateDefinition(INI* ini)
{
	BuffTemplateStore::parseBuffTemplateDefinition(ini);
}
