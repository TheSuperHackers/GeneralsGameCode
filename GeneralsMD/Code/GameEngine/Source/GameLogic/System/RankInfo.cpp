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

// FILE: RankInfo.cpp /////////////////////////////////////////////////////////
// Created:   Steven Johnson, Sep 2002
// Desc:      
//-----------------------------------------------------------------------------

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/INI.h"
#include "Common/Player.h"
#include "GameLogic/RankInfo.h"

RankInfoStore* TheRankInfoStore = NULL;

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

//-----------------------------------------------------------------------------
RankInfo::~RankInfo()
{
}


//-----------------------------------------------------------------------------
RankInfoStore::~RankInfoStore()
{
	Int level;
	for (level =0; level < getRankLevelCount(); level++)
	{
		RankInfo* ri = m_rankInfos[level];
		if (ri)
		{
			MemoryPoolObject::deleteInstance(ri);
		}
	}
	m_rankInfos.clear();
}


//-----------------------------------------------------------------------------
void RankInfoStore::init()
{
	DEBUG_ASSERTCRASH(m_rankInfos.empty(), ("Hmm"));
	m_rankInfos.clear();
}

//-----------------------------------------------------------------------------
void RankInfoStore::reset()
{
	// nope.
	//m_rankInfos.clear();

	for (RankInfoVec::iterator it = m_rankInfos.begin(); it != m_rankInfos.end(); /*++it*/)
	{
		RankInfo* ri = *it;
		if (ri)
		{
			Overridable* temp = ri->deleteOverrides();
			if (!temp)
			{
				DEBUG_CRASH(("hmm, should not be possible for RankInfo"));
				it = m_rankInfos.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
}

//-----------------------------------------------------------------------------
Int RankInfoStore::getRankLevelCount() const
{ 
	return m_rankInfos.size(); 
}

//-----------------------------------------------------------------------------
// note that level is 1...n, NOT 0...n-1
const RankInfo* RankInfoStore::getRankInfo(Int level) const 
{ 
	if (level >= 1 && level <= getRankLevelCount())
	{
		const RankInfo* ri = m_rankInfos[level-1];
		if (ri)
		{
			return (const RankInfo*)ri->getFinalOverride();
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
void RankInfoStore::friend_parseRankDefinition( INI* ini )
{
	if (TheRankInfoStore)
	{
		Int rank = INI::scanInt(ini->getNextToken());

		static const FieldParse myFieldParse[] = 
		{
			{ "RankName", INI::parseAndTranslateLabel, NULL, offsetof( RankInfo, m_rankName ) },
			{ "SkillPointsNeeded", INI::parseInt, NULL, offsetof( RankInfo, m_skillPointsNeeded ) },
			{ "SciencesGranted", INI::parseScienceVector, NULL, offsetof( RankInfo, m_sciencesGranted ) },
			{ "SciencePurchasePointsGranted", INI::parseUnsignedInt, NULL, offsetof( RankInfo, m_sciencePurchasePointsGranted ) },
			{ 0, 0, 0, 0 }
		};

		if (ini->getLoadType() == INI_LOAD_CREATE_OVERRIDES) 
		{
			// we aren't allowed to add ranks in overrides, only to override existing ones.
			if (rank < 1 || rank > TheRankInfoStore->m_rankInfos.size())
			{
				DEBUG_CRASH(("Rank not found in map.ini"));
				throw INI_INVALID_DATA;
			}
			
			RankInfo* info = TheRankInfoStore->m_rankInfos[rank-1];
			if (!info)
			{
				DEBUG_CRASH(("Rank not found in map.ini"));
				throw INI_INVALID_DATA;
			}

			RankInfo* newInfo = newInstance(RankInfo);
			
			// copy data from final override to 'newInfo' as a set of initial default values
			info = (RankInfo*)(info->friend_getFinalOverride());

			*newInfo = *info;
			info->setNextOverride(newInfo);
			newInfo->markAsOverride();	// must do AFTER the copy

			ini->initFromINI(newInfo, myFieldParse);
			//TheRankInfoStore->m_rankInfos.push_back(newInfo);	// NO, BAD, WRONG -- don't add in this case.

		} 
		else
		{
			if (rank != TheRankInfoStore->m_rankInfos.size() + 1)
			{
				DEBUG_CRASH(("Ranks must increase monotonically"));
				throw INI_INVALID_DATA;
			}
			RankInfo* info = newInstance(RankInfo);
			ini->initFromINI(info, myFieldParse);
			TheRankInfoStore->m_rankInfos.push_back(info);
		}
	}
}

//-----------------------------------------------------------------------------
void INI::parseRankDefinition( INI* ini )
{
	RankInfoStore::friend_parseRankDefinition(ini);
}

