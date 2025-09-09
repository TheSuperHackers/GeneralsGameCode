/*
**	Command & Conquer Generals(tm)
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

// FILE: CrateSystem.cpp /////////////////////////////////////////////////////////////////////////////////
// Author: Graham Smallwood Feb 2002
// Desc:   System responsible for Crates as code objects - ini, new/delete etc
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#define DEFINE_VETERANCY_NAMES				// for TheVeterancyNames[]

#include "GameLogic/CrateSystem.h"
#include "Common/BitFlagsIO.h"

CrateSystem *TheCrateSystem = NULL;

CrateSystem::CrateSystem()
{
	m_crateTemplateVector.clear();
}

CrateSystem::~CrateSystem()
{
	Int count = m_crateTemplateVector.size();
	for( Int templateIndex = 0; templateIndex < count; templateIndex ++ )
	{
		CrateTemplate *currentTemplate = m_crateTemplateVector[templateIndex];
		deleteInstance(currentTemplate);
	}
	m_crateTemplateVector.clear();
}

void CrateSystem::init( void )
{
	reset();
}

void CrateSystem::reset( void )
{
	// clean up overrides
	std::vector<CrateTemplate *>::iterator it;
	for( it = m_crateTemplateVector.begin(); it != m_crateTemplateVector.end(); )
	{
		CrateTemplate *currentTemplate = *it;
		if( currentTemplate )
		{
			Overridable *tempCrateTemplate = currentTemplate->deleteOverrides();
			if (!tempCrateTemplate)
			{
				// base dude was an override - kill it from the vector
				it = m_crateTemplateVector.erase(it);
			}
			else
			{
				++it;
			}
		}
		else
		{
			it = m_crateTemplateVector.erase(it);
		}
	}
}

void CrateSystem::parseCrateTemplateDefinition(INI* ini)
{
	AsciiString name;

	// read the crateTemplate name
	const char* c = ini->getNextToken();
	name.set(c);

	CrateTemplate *crateTemplate = TheCrateSystem->friend_findCrateTemplate(name);
	if (crateTemplate == NULL) {
		crateTemplate = TheCrateSystem->newCrateTemplate(name);

		if (ini->getLoadType() == INI_LOAD_CREATE_OVERRIDES) {
			crateTemplate->markAsOverride();
		}
	} else if( ini->getLoadType() != INI_LOAD_CREATE_OVERRIDES ) {
			DEBUG_CRASH(( "[LINE: %d in '%s'] Duplicate crate %s found!", ini->getLineNum(), ini->getFilename().str(), name.str() ));
	} else {
		crateTemplate = TheCrateSystem->newCrateTemplateOverride(crateTemplate);
	}

	// parse the ini weapon definition
	ini->initFromINI(crateTemplate, crateTemplate->getFieldParse());
}

CrateTemplate *CrateSystem::newCrateTemplate( AsciiString name )
{
	// sanity
	if(name.isEmpty())
		return NULL;

	// allocate a new weapon
	CrateTemplate *ct = newInstance(CrateTemplate);

	// if the default template is present, get it and copy over any data to the new template
	const CrateTemplate *defaultCT = findCrateTemplate(AsciiString("DefaultCrate"));
	if(defaultCT)
	{
		*ct = *defaultCT;
	}

	ct->setName( name );
	m_crateTemplateVector.push_back(ct);

	return ct;
}

CrateTemplate *CrateSystem::newCrateTemplateOverride( CrateTemplate *crateToOverride )
{
	if (!crateToOverride) {
		return NULL;
	}

	CrateTemplate *newOverride = newInstance(CrateTemplate);
	*newOverride = *crateToOverride;

	newOverride->markAsOverride();

	crateToOverride->setNextOverride(newOverride);
	return newOverride;
}

const CrateTemplate *CrateSystem::findCrateTemplate(AsciiString name) const
{
	// search weapon list for name
	for (size_t i = 0; i < m_crateTemplateVector.size(); i++)
		if(m_crateTemplateVector[i]->getName() == name) {
			CrateTemplateOverride overridable(m_crateTemplateVector[i]);
			return overridable;
		}


	return NULL;
}

CrateTemplate *CrateSystem::friend_findCrateTemplate(AsciiString name)
{
	// search weapon list for name
	for (size_t i = 0; i < m_crateTemplateVector.size(); i++)
		if(m_crateTemplateVector[i]->getName() == name) {
			CrateTemplateOverride overridable(m_crateTemplateVector[i]);
			return const_cast<CrateTemplate*>((const CrateTemplate *)overridable);
		}
	return NULL;
}



//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
const FieldParse CrateTemplate::TheCrateTemplateFieldParseTable[] =
{
	{ "CreationChance",		INI::parseReal,													NULL,									offsetof( CrateTemplate, m_creationChance ) },
	{ "VeterancyLevel",		INI::parseIndexList,										TheVeterancyNames,		offsetof( CrateTemplate, m_veterancyLevel ) },
	{ "KilledByType",			KindOfMaskType::parseFromINI,												NULL,									offsetof( CrateTemplate, m_killedByTypeKindof) },
	{ "CrateObject",			CrateTemplate::parseCrateCreationEntry,	NULL,									NULL },
	{ "KillerScience",		INI::parseScience,											NULL,									offsetof( CrateTemplate, m_killerScience) },
	{ "OwnedByMaker",			INI::parseBool,													NULL,									offsetof( CrateTemplate, m_isOwnedByMaker) },
	{ NULL,								NULL,																		NULL,									NULL },		// keep this last!
};

CrateTemplate::CrateTemplate()
{
	m_name = "";

	m_creationChance = 0;
	CLEAR_KINDOFMASK(m_killedByTypeKindof);
	m_veterancyLevel = LEVEL_INVALID;
	m_killerScience = SCIENCE_INVALID;
	m_possibleCrates.clear();
	m_isOwnedByMaker = FALSE;
}

CrateTemplate::~CrateTemplate()
{
	m_possibleCrates.clear();
}

void CrateTemplate::parseCrateCreationEntry( INI* ini, void *instance, void *, const void*  )
{
	CrateTemplate *self = (CrateTemplate *)instance;

	const char *token = ini->getNextToken();
	AsciiString crateName = token;

	token = ini->getNextToken();
	Real crateValue;
	if (sscanf( token, "%f", &crateValue ) != 1)
		throw INI_INVALID_DATA;

	crateCreationEntry newEntry;
	newEntry.crateName = crateName;
	newEntry.crateChance = crateValue;

	self->m_possibleCrates.push_back( newEntry );
}

