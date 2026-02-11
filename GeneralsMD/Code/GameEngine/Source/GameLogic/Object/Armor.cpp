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

// FILE: ArmorTemplate.cpp ///////////////////////////////////////////////////////////////////////////////
// Author: Steven Johnson, November 2001
// Desc:   ArmorTemplate descriptions
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine


#include "Common/INI.h"
#include "Common/ThingFactory.h"
#include "GameLogic/Armor.h"
#include "GameLogic/Damage.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC DATA ////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
ArmorStore* TheArmorStore = nullptr;					///< the ArmorTemplate store definition

///////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
ArmorTemplate::ArmorTemplate()
{
	clear();
}

//-------------------------------------------------------------------------------------------------
ArmorTemplate::~ArmorTemplate()
{

}

//-------------------------------------------------------------------------------------------------
void ArmorTemplate::clear()
{
	for (int i = 0; i < DAMAGE_NUM_TYPES; i++)
	{
		m_damageCoefficient[i] = 1.0f;
	}
}

//-------------------------------------------------------------------------------------------------
Real ArmorTemplate::adjustDamage(DamageType t, Real damage) const
{
	if (t == DAMAGE_UNRESISTABLE)
		return damage;
	if (t == DAMAGE_SUBDUAL_UNRESISTABLE)
		return damage;

	damage *= m_damageCoefficient[t];

	if (damage < 0.0f)
		damage = 0.0f;

	return damage;
}

//-------------------------------------------------------------------------------------------Static
/*static*/ void ArmorTemplate::parseArmorCoefficients( INI* ini, void *instance, void* /* store */, const void* userData )
{
	ArmorTemplate* self = (ArmorTemplate*) instance;

	const char* damageName = ini->getNextToken();
	Real pct = INI::scanPercentToReal(ini->getNextToken());

	if (stricmp(damageName, "Default") == 0)
	{
		for (Int i = 0; i < DAMAGE_NUM_TYPES; i++)
		{
			self->m_damageCoefficient[i] = pct;
		}
		return;
	}

	DamageType dt = (DamageType)DamageTypeFlags::getSingleBitFromName(damageName);
	self->m_damageCoefficient[dt] = pct;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
ArmorStore::ArmorStore()
{
	m_armorTemplates.clear();
}

//-------------------------------------------------------------------------------------------------
ArmorStore::~ArmorStore()
{
	for (ArmorTemplateMap::iterator it = m_armorTemplates.begin(); it != m_armorTemplates.end(); ++it)
	{
		deleteInstance(it->second);
	}

	m_armorTemplates.clear();
}

//-------------------------------------------------------------------------------------------------
const ArmorTemplate* ArmorStore::findArmorTemplate(NameKeyType namekey) const
{
	ArmorTemplateMap::const_iterator it = m_armorTemplates.find(namekey);
	if (it == m_armorTemplates.end())
	{
		return nullptr;
	}
	else
	{
		return (*it).second;
	}
}

//-------------------------------------------------------------------------------------------------
const ArmorTemplate* ArmorStore::findArmorTemplate(const AsciiString& name) const
{
	return findArmorTemplate(TheNameKeyGenerator->nameToKey(name));
}

//-------------------------------------------------------------------------------------------------
const ArmorTemplate* ArmorStore::findArmorTemplate(const char* name) const
{
	return findArmorTemplate(TheNameKeyGenerator->nameToKey(name));
}

//-------------------------------------------------------------------------------------------------
/*static */ void ArmorStore::parseArmorDefinition(INI *ini)
{
	static const FieldParse myFieldParse[] =
	{
		{ "Armor", ArmorTemplate::parseArmorCoefficients, nullptr, 0 }
	};

	const char *name = ini->getNextToken();
	const NameKeyType key = TheNameKeyGenerator->nameToKey(name);

	// TheSuperHackers @bugfix Caball009 04/01/2025 Avoid mismatches by creating overrides instead of overwriting the default data.
	// The code resembles the code of the ThingFactory.
	ArmorTemplate *armorTmpl = TheArmorStore->m_armorTemplates[key];
	if (!armorTmpl)
	{
		// no item is present, create a new one
		armorTmpl = newInstance(ArmorTemplate);
		armorTmpl->clear();

		if (ini->getLoadType() == INI_LOAD_CREATE_OVERRIDES)
		{
			// This ArmorTemplate is actually an override, so we will mark it as such so that it properly
			// gets deleted on ::reset().
			armorTmpl->markAsOverride();
		}

		TheArmorStore->m_armorTemplates[key] = armorTmpl;
	}
	else if (ini->getLoadType() != INI_LOAD_CREATE_OVERRIDES)
	{
		DEBUG_CRASH(( "[LINE: %d in '%s'] Duplicate armor definition %s found!", ini->getLineNum(), ini->getFilename().str(), name ));
	}
	else
	{
		armorTmpl = TheArmorStore->newOverride(armorTmpl, name);
	}

	ini->initFromINI(armorTmpl, myFieldParse);
}

//-------------------------------------------------------------------------------------------------
ArmorTemplate* ArmorStore::newOverride(ArmorTemplate *armorTemplate, const char *name)
{
	// sanity
	DEBUG_ASSERTCRASH( armorTemplate, ("newOverride(): NULL 'parent' armor template") );

	// sanity just for debuging, the armor must be in the master list to do overrides
	DEBUG_ASSERTCRASH( findArmorTemplate( name ) != NULL,
										 ("newOverride(): Armor template '%s' not in master list", name) );

	// find final override of the 'parent' template
	ArmorTemplate *child = static_cast<ArmorTemplate*>(armorTemplate->friend_getFinalOverride());

	// allocate new template
	ArmorTemplate *newTemplate = newInstance(ArmorTemplate);

	// copy data from final override to 'newTemplate' as a set of initial default values
	*newTemplate = *child;

	newTemplate->markAsOverride();
	child->setNextOverride(newTemplate);

	// return the newly created override for us to set values with etc
	return newTemplate;
}

//-------------------------------------------------------------------------------------------------
void ArmorStore::reset()
{
	ArmorTemplateMap::iterator it = m_armorTemplates.begin();

	while (it != m_armorTemplates.end())
	{
		ArmorTemplateMap::iterator next = it;
		++next;

		const Overridable *stillValid = it->second->deleteOverrides();

		if (stillValid == NULL)
		{
			// Also needs to be removed from the Hash map.
			m_armorTemplates.erase(it);
		}

		it = next;
	}
}

//-------------------------------------------------------------------------------------------------
/*static*/ void INI::parseArmorDefinition(INI *ini)
{
	ArmorStore::parseArmorDefinition(ini);
}

