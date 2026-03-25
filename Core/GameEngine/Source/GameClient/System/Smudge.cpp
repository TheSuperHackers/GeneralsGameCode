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


DLListClass<Smudge> SmudgeSet::m_freeSmudgeList;	///<list of unused smudges for use by SmudgeSets.

SmudgeManager::SmudgeManager()
{
	m_smudgeCountLastFrame=0;
	m_hardwareSupportStatus = SMUDGE_SUPPORT_UNKNOWN;
}

SmudgeManager::~SmudgeManager()
{
	reset();	//release all smudge sets and smudges to free pool.

	SmudgeSet* head;

	//free memory used by smudge sets
	while ((head = m_freeSmudgeSetList.Head ()) != nullptr) {
		m_freeSmudgeSetList.Remove_Head ();
		delete head;
	}

	Smudge* head2;
	//free memory used by smudges
	while ((head2 = SmudgeSet::m_freeSmudgeList.Head ()) != nullptr) {
		m_freeSmudgeSetList.Remove_Head ();
		delete head2;
	}
}

void SmudgeManager::init()
{

}

void SmudgeManager::reset()
{
	SmudgeSet* head;

	//Return all smudgeSets back to free pool.
	while ((head = m_usedSmudgeSetList.Head ()) != nullptr) {
		m_usedSmudgeSetList.Remove_Head ();
		head->reset();	//free all smudges.
		m_freeSmudgeSetList.Add_Tail(head);
	}
}

void SmudgeManager::resetDraw()
{
	SmudgeSet* smudgeSet = m_usedSmudgeSetList.Head();
	for (; smudgeSet; smudgeSet = smudgeSet->Succ()) {
		smudgeSet->resetDraw();
	}
}

SmudgeSet *SmudgeManager::addSmudgeSet()
{
	SmudgeSet* set=m_freeSmudgeSetList.Head();
	if (set) {
		set->Remove();	//remove from free list
		m_usedSmudgeSetList.Add_Tail(set);	//add to used list.
		return set;
	}
	set=W3DNEW SmudgeSet();
	m_usedSmudgeSetList.Add_Tail(set);	//add to used list.
	return set;
}

void SmudgeManager::removeSmudgeSet(SmudgeSet *&smudgeSet)
{
	smudgeSet->Remove();	//remove from used list
	m_freeSmudgeSetList.Add_Head(smudgeSet);	//add to free list.
	smudgeSet = nullptr;
}

Smudge *SmudgeManager::findSmudge(Smudge::Identifier identifier)
{
	SmudgeSet *smudgeSet = m_usedSmudgeSetList.Head();
	for (; smudgeSet; smudgeSet = smudgeSet->Succ()) {
		if (Smudge *smudge = smudgeSet->findSmudge(identifier)) {
			return smudge;
		}
	}
	return nullptr;
}


SmudgeSet::SmudgeSet()
{
	m_usedSmudgeCount=0;
}

SmudgeSet::~SmudgeSet()
{
	reset();
}

void SmudgeSet::reset()
{
	Smudge* head;

	while ((head = m_usedSmudgeList.Head ()) != nullptr) {
		m_usedSmudgeList.Remove_Head ();
		m_freeSmudgeList.Add_Head(head);	//add to free list
	}

	m_usedSmudgeMap.clear();
	m_usedSmudgeCount = 0;
}

void SmudgeSet::resetDraw()
{
	Smudge* smudge = m_usedSmudgeList.Head();
	for (; smudge; smudge = smudge->Succ()) {
		smudge->m_draw = false;
	}
}

Smudge *SmudgeSet::addSmudgeToSet(Smudge::Identifier identifier)
{
	DEBUG_ASSERTCRASH(m_usedSmudgeMap.find(identifier) == m_usedSmudgeMap.end(),
		("SmudgeSet::addSmudgeToSet: identifier already present"));

	Smudge* smudge = m_freeSmudgeList.Head();
	if (smudge) {
		smudge->Remove();	//remove from free list
	} else {
		smudge = W3DNEW Smudge();
	}
	smudge->m_identifier = identifier;
	m_usedSmudgeList.Add_Tail(smudge);	//add to used list.
	m_usedSmudgeMap[identifier] = smudge;
	m_usedSmudgeCount++;
	return smudge;
}

void SmudgeSet::removeSmudgeFromSet(Smudge *&smudge)
{
	m_usedSmudgeMap.erase(smudge->m_identifier);
	smudge->Remove();	//remove from used list.
	m_freeSmudgeList.Add_Head(smudge);	//add to free list
	smudge = nullptr;
	m_usedSmudgeCount--;
}

Smudge *SmudgeSet::findSmudge(Smudge::Identifier identifier)
{
	SmudgeIdToPtrMap::const_iterator it = m_usedSmudgeMap.find(identifier);
	if (it != m_usedSmudgeMap.end())
		return it->second;
	return nullptr;
}
