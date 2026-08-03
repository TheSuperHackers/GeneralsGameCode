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

// Smudge.cpp ////////////////////////////////////////////////////////////////////////////////
// Smudge System implementation
// Author: Mark Wilczynski, June 2003
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file in the Game
#include "GameClient/Smudge.h"


SmudgeDeque SmudgeSet::m_freeSmudgeList;	///<list of unused smudges for use by SmudgeSets.

SmudgeManager::SmudgeManager()
{
	m_smudgeCountLastFrame=0;
	m_hardwareSupportStatus = SMUDGE_SUPPORT_UNKNOWN;
}

SmudgeManager::~SmudgeManager()
{
	reset();	//release all smudge sets and smudges to free pool.

	//free memory used by smudge sets
	while (!m_freeSmudgeSetList.empty()) {
		SmudgeSet* smudgeSet = m_freeSmudgeSetList.front();
		m_freeSmudgeSetList.pop_front();
		delete smudgeSet;
	}

	//free memory used by smudges
	while (!SmudgeSet::m_freeSmudgeList.empty()) {
		Smudge* smudge = SmudgeSet::m_freeSmudgeList.front();
		SmudgeSet::m_freeSmudgeList.pop_front();
		delete smudge;
	}
}

void SmudgeManager::init()
{

}

void SmudgeManager::reset()
{
	//Return all smudgeSets back to free pool.
	while (!m_usedSmudgeSetList.empty()) {
		SmudgeSet* smudgeSet = m_usedSmudgeSetList.front();
		m_usedSmudgeSetList.pop_front();
		smudgeSet->reset();	//free all smudges.
		m_freeSmudgeSetList.push_back(smudgeSet);
	}
}

void SmudgeManager::resetDraw()
{
	for (SmudgeSetDeque::iterator it = m_usedSmudgeSetList.begin(); it != m_usedSmudgeSetList.end(); ++it) {
		SmudgeSet* smudgeSet = *it;
		smudgeSet->resetDraw();
	}
}

SmudgeSet *SmudgeManager::addSmudgeSet()
{
	if (!m_freeSmudgeSetList.empty()) {
		SmudgeSet* smudgeSet = m_freeSmudgeSetList.front();
		m_freeSmudgeSetList.pop_front();	//remove from free list
		m_usedSmudgeSetList.push_back(smudgeSet);	//add to used list.
		return smudgeSet;
	}

	SmudgeSet* smudgeSet = W3DNEW SmudgeSet();
	m_usedSmudgeSetList.push_back(smudgeSet);	//add to used list.
	return smudgeSet;
}

Smudge *SmudgeManager::findSmudge(Smudge::Identifier identifier)
{
	for (SmudgeSetDeque::iterator it = m_usedSmudgeSetList.begin(); it != m_usedSmudgeSetList.end(); ++it) {
		SmudgeSet* smudgeSet = *it;
		if (Smudge* smudge = smudgeSet->findSmudge(identifier)) {
			return smudge;
		}
	}
	return nullptr;
}


SmudgeSet::SmudgeSet()
{
}

SmudgeSet::~SmudgeSet()
{
	reset();
}

void SmudgeSet::reset()
{
	while (!m_usedSmudgeList.empty()) {
		Smudge* smudge = m_usedSmudgeList.front();
		m_usedSmudgeList.pop_front();
		m_freeSmudgeList.push_front(smudge);	// add to free list
	}

	m_usedSmudgeMap.clear();
}

void SmudgeSet::resetDraw()
{
	for (SmudgeDeque::iterator it = m_usedSmudgeList.begin(); it != m_usedSmudgeList.end(); ++it) {
		Smudge* smudge = *it;
		smudge->m_draw = false;
	}
}

Smudge *SmudgeSet::addSmudgeToSet(Smudge::Identifier identifier)
{
	DEBUG_ASSERTCRASH(m_usedSmudgeMap.find(identifier) == m_usedSmudgeMap.end(),
		("SmudgeSet::addSmudgeToSet: identifier already present"));

	Smudge* smudge;
	if (!m_freeSmudgeList.empty()) {
		smudge = m_freeSmudgeList.front();
		m_freeSmudgeList.pop_front();	//remove from free list
	} else {
		smudge = W3DNEW Smudge();
	}
	smudge->m_identifier = identifier;
	m_usedSmudgeList.push_back(smudge);	//add to used list.
	m_usedSmudgeMap[identifier] = smudge;
	return smudge;
}

Smudge *SmudgeSet::findSmudge(Smudge::Identifier identifier)
{
	SmudgeIdToPtrMap::const_iterator it = m_usedSmudgeMap.find(identifier);
	if (it != m_usedSmudgeMap.end())
		return it->second;
	return nullptr;
}
