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

// FILE: Science.cpp /////////////////////////////////////////////////////////
// Created:   Steven Johnson, October 2001
// Desc:      @todo
//-----------------------------------------------------------------------------

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/INI.h"
#include "Common/Player.h"
#include "Common/Science.h"

ScienceStore* TheScienceStore = NULL;


//-----------------------------------------------------------------------------
void ScienceStore::init()
{
	DEBUG_ASSERTCRASH(m_sciences.empty(), ("Hmm"));
	m_sciences.clear();
}

//-----------------------------------------------------------------------------
ScienceStore::~ScienceStore()
{
	// nope.
	//m_sciences.clear();

	// go through all sciences and delete any overrides
	for (ScienceInfoVec::iterator it = m_sciences.begin(); it != m_sciences.end(); /*++it*/)
	{
		ScienceInfo* si = *it;
		++it;
		deleteInstance(si);
	}
}

//-----------------------------------------------------------------------------
void ScienceStore::reset()
{
	// nope.
	//m_sciences.clear();

	// go through all sciences and delete any overrides
	for (ScienceInfoVec::iterator it = m_sciences.begin(); it != m_sciences.end(); /*++it*/)
	{
		ScienceInfo* si = *it;
		Overridable* temp = si->deleteOverrides();
		if (!temp)
		{
			it = m_sciences.erase(it);
		}
		else
		{
			++it;
		}
	}
}

//-----------------------------------------------------------------------------
ScienceType ScienceStore::getScienceFromInternalName(const AsciiString& name) const
{
	if (name.isEmpty())
		return SCIENCE_INVALID;
	NameKeyType nkt = TheNameKeyGenerator->nameToKey(name);
	ScienceType st = (ScienceType)nkt;
	return st;
}

//-----------------------------------------------------------------------------
AsciiString ScienceStore::getInternalNameForScience(ScienceType science) const
{
	if (science == SCIENCE_INVALID)
		return AsciiString::TheEmptyString;
	NameKeyType nk = (NameKeyType)(science);
	return TheNameKeyGenerator->keyToName(nk);
}

//-----------------------------------------------------------------------------
// return a vector of all the currently-known science names
// NOTE: this is really only for use by WorldBuilder! Please
// do not use it in RTS!
std::vector<AsciiString> ScienceStore::friend_getScienceNames() const
{
	std::vector<AsciiString> v;
	for (ScienceInfoVec::const_iterator it = m_sciences.begin(); it != m_sciences.end(); ++it)
	{
		const ScienceInfo* si = (const ScienceInfo*)(*it)->getFinalOverride();
		NameKeyType nk = (NameKeyType)(si->m_science);
		v.push_back(TheNameKeyGenerator->keyToName(nk));
	}
	return v;
}

//-----------------------------------------------------------------------------
void ScienceInfo::addRootSciences(ScienceVec& v) const
{
	if (m_prereqSciences.empty())
	{
		// we're a root. add ourselves.
		if (std::find(v.begin(), v.end(), m_science) == v.end())
			v.push_back(m_science);
	}
	else
	{
		// we're not a root. add the roots of all our prereqs.
		for (ScienceVec::const_iterator it = m_prereqSciences.begin(); it != m_prereqSciences.end(); ++it)
		{
			const ScienceInfo* si = TheScienceStore->findScienceInfo(*it);
			if (si)
				si->addRootSciences(v);
		}
	}
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
const ScienceInfo* ScienceStore::findScienceInfo(ScienceType st) const
{
	for (ScienceInfoVec::const_iterator it = m_sciences.begin(); it != m_sciences.end(); ++it)
	{
		const ScienceInfo* si = (const ScienceInfo*)(*it)->getFinalOverride();
		if (si->m_science == st)
		{
			return si;
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
/*static*/ void ScienceStore::friend_parseScienceDefinition( INI* ini )
{
	const char* c = ini->getNextToken();
	NameKeyType nkt = NAMEKEY(c);
	ScienceType st = (ScienceType)nkt;

	if (TheScienceStore)
	{

		static const FieldParse myFieldParse[] =
		{
			{ "PrerequisiteSciences", INI::parseScienceVector, NULL, offsetof( ScienceInfo, m_prereqSciences ) },
			{ "SciencePurchasePointCost", INI::parseInt, NULL, offsetof( ScienceInfo, m_sciencePurchasePointCost ) },
			{ "IsGrantable", INI::parseBool, NULL, offsetof( ScienceInfo, m_grantable ) },
			{ "DisplayName", INI::parseAndTranslateLabel, NULL, offsetof( ScienceInfo, m_name) },
			{ "Description", INI::parseAndTranslateLabel, NULL, offsetof( ScienceInfo, m_description) },
			{ 0, 0, 0, 0 }
		};

		ScienceInfo* info = NULL;

		// see if the science already exists. (can't use findScienceInfo() since it is const and should remain so.)
		for (ScienceInfoVec::iterator it = TheScienceStore->m_sciences.begin(); it != TheScienceStore->m_sciences.end(); ++it)
		{
			// note that we don't use getFinalOverride here. this is correct and as-desired.
			if ((*it)->m_science == st)
			{
				info = *it;
				break;
			}
		}

		if (ini->getLoadType() == INI_LOAD_CREATE_OVERRIDES)
		{
			ScienceInfo* newInfo = newInstance(ScienceInfo);

			if (info == NULL)
			{
				// only add if it's not overriding an existing one.
				info = newInfo;
				info->markAsOverride();	// yep, so we will get cleared on reset()
				TheScienceStore->m_sciences.push_back(info);
			}
			else
			{
				// copy data from final override to 'newInfo' as a set of initial default values
				info = (ScienceInfo*)(info->friend_getFinalOverride());

				*newInfo = *info;
				info->setNextOverride(newInfo);
				newInfo->markAsOverride();	// must do AFTER the copy

				// use the newly created override for us to set values with etc
				info = newInfo;
				//TheScienceStore->m_sciences.push_back(info);	// NO, BAD, WRONG -- don't add in this case.
			}
		}
		else
		{
			if (info != NULL)
			{
				DEBUG_CRASH(("duplicate science %s!",c));
				throw INI_INVALID_DATA;
			}
			info = newInstance(ScienceInfo);
			TheScienceStore->m_sciences.push_back(info);
		}

		ini->initFromINI(info, myFieldParse);
		info->m_science = st;
		info->addRootSciences(info->m_rootSciences);
	}
}

//-----------------------------------------------------------------------------
Int ScienceStore::getSciencePurchaseCost(ScienceType st) const
{
	const ScienceInfo* si = findScienceInfo(st);
	if (si)
	{
		return si->m_sciencePurchasePointCost;
	}
	else
	{
		return 0;
	}
}

//-----------------------------------------------------------------------------
Bool ScienceStore::isScienceGrantable(ScienceType st) const
{
	const ScienceInfo* si = findScienceInfo(st);
	if (si)
	{
		return si->m_grantable;
	}
	else
	{
		return false;
	}
}

//-----------------------------------------------------------------------------
Bool ScienceStore::getNameAndDescription(ScienceType st, UnicodeString& name, UnicodeString& description) const
{
	const ScienceInfo* si = findScienceInfo(st);
	if (si)
	{
		name = si->m_name;
		description = si->m_description;
		return true;
	}
	else
	{
		return false;
	}
}

//-----------------------------------------------------------------------------
Bool ScienceStore::playerHasPrereqsForScience(const Player* player, ScienceType st) const
{
	const ScienceInfo* si = findScienceInfo(st);
	if (si)
	{
		for (ScienceVec::const_iterator it2 = si->m_prereqSciences.begin(); it2 != si->m_prereqSciences.end(); ++it2)
		{
			if (!player->hasScience(*it2))
			{
				return false;
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}

//-----------------------------------------------------------------------------
Bool ScienceStore::playerHasRootPrereqsForScience(const Player* player, ScienceType st) const
{
	const ScienceInfo* si = findScienceInfo(st);
	if (si)
	{
		for (ScienceVec::const_iterator it2 = si->m_rootSciences.begin(); it2 != si->m_rootSciences.end(); ++it2)
		{
			if (!player->hasScience(*it2))
			{
				return false;
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}

//-----------------------------------------------------------------------------
/** return a list of the sciences the given player can purchase now, and a list he might be able to purchase in the future,
	but currently lacks prereqs or points for. (either might be an empty list) */
void ScienceStore::getPurchasableSciences(const Player* player, ScienceVec& purchasable, ScienceVec& potentiallyPurchasable) const
{
	purchasable.clear();
	potentiallyPurchasable.clear();
	for (ScienceInfoVec::const_iterator it = m_sciences.begin(); it != m_sciences.end(); ++it)
	{
		const ScienceInfo* si = (const ScienceInfo*)(*it)->getFinalOverride();

		if (si->m_sciencePurchasePointCost == 0)
		{
			// 0 means "cannot be purchased"
			continue;
		}

		if (player->hasScience(si->m_science))
		{
			continue;
		}

		if (playerHasPrereqsForScience(player, si->m_science))
		{
			purchasable.push_back(si->m_science);
		}
		else if (playerHasRootPrereqsForScience(player, si->m_science))
		{
			potentiallyPurchasable.push_back(si->m_science);
		}
	}
}

//-----------------------------------------------------------------------------
// this is intended ONLY for use by INI::scanScience.
// Don't use it anywhere else. In particular, never, ever, ever
// call this with a hardcoded science name. (srj)
ScienceType ScienceStore::friend_lookupScience(const char* scienceName) const
{
	NameKeyType nkt = NAMEKEY(scienceName);
	ScienceType st = (ScienceType)nkt;
	if (!isValidScience(st))
	{
		DEBUG_CRASH(("Science name %s not known! (Did you define it in Science.ini?)",scienceName));
		throw INI_INVALID_DATA;
	}
	return st;
}

//-----------------------------------------------------------------------------
Bool ScienceStore::isValidScience(ScienceType st) const
{
	const ScienceInfo* si = findScienceInfo(st);
	return si != NULL;
}

//-----------------------------------------------------------------------------
void INI::parseScienceDefinition( INI* ini )
{
	ScienceStore::friend_parseScienceDefinition(ini);
}

