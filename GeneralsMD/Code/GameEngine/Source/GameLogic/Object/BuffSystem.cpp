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

#include "Common/GlobalData.h"

#include "GameLogic/GameLogic.h"
#include "GameLogic/BuffSystem.h"
#include "GameLogic/Module/BodyModule.h"


///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC DATA ////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

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

	virtual void apply(Object* targetObj, const Object* sourceObj) const
	{
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
	}

	virtual void remove(Object* targetObj) const
	{
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

	}

	static void parseValueModifier(INI* ini, void* instance, void* /*store*/, const void* /*userData*/)
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
//-------------------------------------------------------------------------------------------------
static const FieldParse TheBuffTemplateFieldParse[] =
{
	{ "ValueModifier",			ValueModifierBuffEffectNugget::parseValueModifier, 0, 0},
	//{ "CreateDebris",			GenericObjectCreationNugget::parseDebris, 0, 0},
	//{ "ApplyRandomForce",	ApplyRandomForceNugget::parse, 0, 0},
	//{ "DeliverPayload",		DeliverPayloadNugget::parse, 0, 0},
	//{ "FireWeapon",				FireWeaponNugget::parse, 0, 0},
	//{ "Attack",						AttackNugget::parse, 0, 0},
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

void BuffTemplate::applyEffects(Object* targetObj, Object* sourceObj) const
{
	for (BuffEffectNugget* buffEffectNugget : m_nuggets) {
		buffEffectNugget->apply(targetObj, sourceObj);
	}
}
//-------------------------------------------------------------------------------------------------


void BuffTemplate::removeEffects(Object* targetObj) const
{
	for (BuffEffectNugget* buffEffectNugget : m_nuggets) {
		buffEffectNugget->remove(targetObj);
	}
}

//-------------------------------------------------------------------------------------------------

UnsignedInt BuffTemplate::getNextTickFrame(UnsignedInt startFrame, UnsignedInt endFrame) const
{
	// TODO: If we have continuos buff effects (e.g. DOT), we need to consider this here.
	return endFrame;
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
	ini->initFromINI(buffTmp, TheBuffTemplateFieldParse);

	TheBuffTemplateStore->m_buffTemplates[key] = buffTmp;

	DEBUG_LOG((">>> ADDED BUFF TEMPLATE '%s'", buffTmp->getName().str()));
}

//-------------------------------------------------------------------------------------------------
/*static*/ void INI::parseBuffTemplateDefinition(INI* ini)
{
	BuffTemplateStore::parseBuffTemplateDefinition(ini);
}
