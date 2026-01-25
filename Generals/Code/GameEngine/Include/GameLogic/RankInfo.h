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

// FILE: RankInfo.h ////////////////////////////////////////////////////////////////////////////////
// Author: Steven Johnson, Sep 2002
// Desc:   RankInfo descriptions
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/Science.h"
#include "Common/UnicodeString.h"
#include "Common/STLTypedefs.h"

class Player;

//-------------------------------------------------------------------------------------------------
class RankInfo : public Overridable
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( RankInfo, "RankInfo" );
public:
	UnicodeString		m_rankName;
	Int							m_skillPointsNeeded;
	Int							m_sciencePurchasePointsGranted;
	ScienceVec			m_sciencesGranted;
};
//EMPTY_DTOR(RankInfo)

//-------------------------------------------------------------------------------------------------
class RankInfoStore : public SubsystemInterface
{
public:
	virtual ~RankInfoStore();

public:
	void init();
	void reset();
	void update() { }

	Int getRankLevelCount() const;

	// note that level is 1...n, NOT 0...n-1
	const RankInfo* getRankInfo(Int level) const;

	static void friend_parseRankDefinition(INI* ini);

private:

	typedef std::vector<RankInfo*> RankInfoVec;
	RankInfoVec m_rankInfos;
};

extern RankInfoStore* TheRankInfoStore;
