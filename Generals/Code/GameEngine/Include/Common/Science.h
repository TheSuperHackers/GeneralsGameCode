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

// FILE: Science.h ////////////////////////////////////////////////////////////////////////////////
// Author: Steven Johnson, Colin Day November 2001
// Desc:   Science descriptions
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/Overridable.h"
#include "Common/NameKeyGenerator.h"
#include "Common/UnicodeString.h"

class Player;

//-------------------------------------------------------------------------------------------------
enum ScienceType CPP_11(: Int)
{
	SCIENCE_INVALID = -1
};

//-------------------------------------------------------------------------------------------------
typedef std::vector<ScienceType> ScienceVec;

//-------------------------------------------------------------------------------------------------
class ScienceInfo : public Overridable
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( ScienceInfo, "ScienceInfo"  )

	friend class ScienceStore;

private:
	ScienceType						m_science;
	UnicodeString					m_name;
	UnicodeString					m_description;
	ScienceVec						m_rootSciences;			// this is calced at runtime, NOT read from INI
	ScienceVec						m_prereqSciences;
	Int										m_sciencePurchasePointCost;
	Bool									m_grantable;

	ScienceInfo() :
		m_science(SCIENCE_INVALID),
		m_sciencePurchasePointCost(0),	// 0 means "cannot be purchased"
		m_grantable(true)
	{
	}

	void addRootSciences(ScienceVec& v) const;
};
EMPTY_DTOR(ScienceInfo);

//-------------------------------------------------------------------------------------------------
class ScienceStore : public SubsystemInterface
{
	friend class ScienceInfo;

public:
	virtual ~ScienceStore();

	void init();
	void reset();
	void update() { }

	Bool isValidScience(ScienceType st) const;

	Bool isScienceGrantable(ScienceType st) const;

	Bool getNameAndDescription(ScienceType st, UnicodeString& name, UnicodeString& description) const;

	Bool playerHasPrereqsForScience(const Player* player, ScienceType st) const;

	/**
		this is a subtle call, and should ALMOST NEVER be called by external code...
		this is used to determine if you have the "root" requirements for a given science,
		and thus could *potentially* obtain it if you got extra prereqs.

		Generally, you should call getPurchasableSciences() instead of this!
	*/
	Bool playerHasRootPrereqsForScience(const Player* player, ScienceType st) const;

	Int getSciencePurchaseCost(ScienceType science) const;

	ScienceType getScienceFromInternalName(const AsciiString& name) const;
	AsciiString getInternalNameForScience(ScienceType science) const;

	/** return a list of the sciences the given player can purchase now, and a list he might be able to purchase in the future,
		but currently lacks prereqs or points for. (either might be an empty list) */
	void getPurchasableSciences(const Player* player, ScienceVec& purchasable, ScienceVec& potentiallyPurchasable) const;

	// this is intended ONLY for use by INI::scanScience.
	// Don't use it anywhere else. In particular, never, ever, ever
	// call this with a hardcoded science name. (srj)
	ScienceType friend_lookupScience(const char* scienceName) const;
	static void friend_parseScienceDefinition(INI* ini);

	// return a vector of all the currently-known science names
	// NOTE: this is really only for use by WorldBuilder! Please
	// do not use it in RTS!
	std::vector<AsciiString> friend_getScienceNames() const;


private:

	const ScienceInfo* findScienceInfo(ScienceType st) const;

	typedef std::vector<ScienceInfo*> ScienceInfoVec;
	ScienceInfoVec m_sciences;
};

extern ScienceStore* TheScienceStore;
