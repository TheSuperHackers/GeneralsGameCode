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

// ObjectTypes.h //////////////////////////////////////////////////////////////////////////////////
// Author: John McDonald, Jr.
// September 2002
// DO NOT DISTRIBUTE

#pragma once

#ifndef __OBJECT_TYPES_H__
#define __OBJECT_TYPES_H__

#include "Common/AsciiString.h"
#include "Common/Snapshot.h"

class Player;

class ObjectTypes : public MemoryPoolObject,
										public Snapshot
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(ObjectTypes, "ObjectTypes")
private:
	typedef std::vector<AsciiString> AsciiStringVec;
	typedef AsciiStringVec::iterator AsciiStringVecIt;

private:
	// Note, there is no direct access to m_objectTypes. Please endeavor to keep it this way, and
	// add any functions you might need to this class.
	AsciiString m_listName;
	AsciiStringVec m_objectTypes;

protected:
	// snapshot methods
	virtual void crc(Xfer *xfer);
	virtual void xfer(Xfer *xfer);
	virtual void loadPostProcess();

public:
	ObjectTypes();
	ObjectTypes(const AsciiString& listName);

public:
	// Maintenance
	void addObjectType(const AsciiString &objectType);
	void removeObjectType(const AsciiString &objectType);

	// Which list is this again?
	const AsciiString& getListName() const;
	void setListName(const AsciiString& listName);

	// Equivalence testing
	Bool isInSet(const AsciiString& objectType) const;
	Bool isInSet(const ThingTemplate* objectType) const;

	// Is the set empty?
	size_t getListSize(void) const { return m_objectTypes.size(); }

	// I'd like to loop through, please.
	AsciiString getNthInList( size_t index ) const { return (index < getListSize()) ? m_objectTypes[index] : AsciiString::TheEmptyString; }

	// Prep two arrays for usage with Player::countObjectsByThingTemplate
	Int prepForPlayerCounting( std::vector<const ThingTemplate *>& templates, std::vector<Int>& counts);

	// Can the player build any of the thing templates that we have names for?
	Bool canBuildAny(Player *player);
};
EMPTY_DTOR(ObjectTypes)

#endif /* __OBJECT_TYPES_H__ */
