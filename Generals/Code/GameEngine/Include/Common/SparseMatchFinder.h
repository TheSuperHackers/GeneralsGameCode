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

// FILE: SparseMatchFinder.h /////////////////////////////////////////////////////////////////////////
// Author: Steven Johnson, March 2002
// Desc:
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __SparseMatchFinder_H_
#define __SparseMatchFinder_H_

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "Common/BitFlags.h"
#include "Common/STLTypedefs.h"

#if defined(RTS_DEBUG)
	#define SPARSEMATCH_DEBUG
#else
	#undef SPARSEMATCH_DEBUG
#endif

typedef UnsignedInt SparseMatchFinderFlags;
enum SparseMatchFinderFlags_ CPP_11(: SparseMatchFinderFlags)
{
	SparseMatchFinderFlags_NoCopy = 1<<0,
};

//-------------------------------------------------------------------------------------------------
template<class MATCHABLE, class BITSET, SparseMatchFinderFlags FLAGS = 0>
class SparseMatchFinder
{
private:

	//-------------------------------------------------------------------------------------------------
	// TYPEDEFS
	//-------------------------------------------------------------------------------------------------

	struct HashMapHelper
	{
		size_t operator()(const BITSET& p) const
		{
			/// @todo srj -- provide a better hash function for BITSET
			size_t result = 0;

			for (int i = 0; i < p.size(); ++i)
				if (p.test(i))
					result += (i + 1);

			return result;
		}

		Bool operator()(const BITSET& a, const BITSET& b) const
		{
			return (a == b);
		}
	};

	struct MapHelper
	{
		bool operator()(const BITSET& a, const BITSET& b) const
		{
			int i;
			if (a.size() < b.size()) {
				return true;
			}
			for (i = 0; i < a.size(); ++i) {
				bool aVal = a.test(i);
				bool bVal = b.test(i);
				if (aVal && bVal) continue;
				if (!aVal && !bVal) continue;
				if (!aVal) return true;
				return false;
			}
			return false; // all bits match.
		}
	};

	//-------------------------------------------------------------------------------------------------
	typedef std::hash_map< BITSET, const MATCHABLE*, HashMapHelper, HashMapHelper > MatchMap;

	//-------------------------------------------------------------------------------------------------
	// MEMBER VARS
	//-------------------------------------------------------------------------------------------------

	mutable MatchMap m_bestMatches;

	//-------------------------------------------------------------------------------------------------
	// METHODS
	//-------------------------------------------------------------------------------------------------

	//-------------------------------------------------------------------------------------------------
	inline static Int countConditionIntersection(const BITSET& a, const BITSET& b)
	{
		return a.countIntersection(b);
	}

	//-------------------------------------------------------------------------------------------------
	inline static Int countConditionInverseIntersection(const BITSET& a, const BITSET& b)
	{
		return a.countInverseIntersection(b);
	}

	//-------------------------------------------------------------------------------------------------
	const MATCHABLE* findBestInfoSlow(const std::vector<MATCHABLE>& v, const BITSET& bits) const
	{
		const MATCHABLE* result = NULL;
		Int bestYesMatch = 0;							// want to maximize this
		Int bestYesExtraneousBits = 999;	// want to minimize this

	#ifdef SPARSEMATCH_DEBUG
		Int numDupMatches = 0;
		AsciiString curBestMatchStr, dupMatchStr;
	#endif

		for (typename std::vector<MATCHABLE>::const_iterator it = v.begin(); it != v.end(); ++it)
		{
			for (Int i = it->getConditionsYesCount()-1; i >= 0; --i)
			{
				const BITSET& yesFlags = it->getNthConditionsYes(i);

				// the best match has the most "yes" matches and the smallest number of "no" matches.
				// if there are ties, then prefer the model with the smaller number of irrelevant 'yes' bits.
				// (example of why tiebreaker is necessary: if we want to match FRONTCRUSHED,
				// it would tie with both FRONTCRUSHED and FRONTCRUSHED|BACKCRUSHED, since they
				// both match a single YES bit.)
				Int yesMatch = countConditionIntersection(bits, yesFlags);
				Int yesExtraneousBits = countConditionInverseIntersection(bits, yesFlags);

	#ifdef SPARSEMATCH_DEBUG
				if (yesMatch == bestYesMatch &&
						yesExtraneousBits == bestYesExtraneousBits)
				{
					++numDupMatches;
					dupMatchStr = it->getDescription();
				}
	#endif

				if ((yesMatch > bestYesMatch) ||
						(yesMatch >= bestYesMatch && yesExtraneousBits < bestYesExtraneousBits))
				{
					result = &(*it);
					bestYesMatch = yesMatch;
					bestYesExtraneousBits = yesExtraneousBits;
	#ifdef SPARSEMATCH_DEBUG
					numDupMatches = 0;
					curBestMatchStr = it->getDescription();
	#endif
				}
			}	// end for i

		}	// end for it

#ifdef SPARSEMATCH_DEBUG
		if (numDupMatches > 0)
		{
			AsciiString curConditionStr;
			bits.buildDescription(&curConditionStr);
			DEBUG_CRASH(("ambiguous model match in findBestInfoSlow \n\nbetween \n(%s)\n<and>\n(%s)\n\n(%d extra matches found)\n\ncurrent bits are (\n%s)",
					curBestMatchStr.str(),
					dupMatchStr.str(),
					numDupMatches,
					curConditionStr.str()));
		}
#endif

		return result;
	}

public:


	//-------------------------------------------------------------------------------------------------
	SparseMatchFinder() {}
	SparseMatchFinder(const SparseMatchFinder& other)
	{
		*this = other;
	}

	//-------------------------------------------------------------------------------------------------
	SparseMatchFinder& operator=(const SparseMatchFinder& other)
	{
		if constexpr ((FLAGS & SparseMatchFinderFlags_NoCopy) == 0)
		{
			if (this != &other)
			{
				m_bestMatches = other.m_bestMatches;
			}
		}
		return *this;
	}

	//-------------------------------------------------------------------------------------------------
	void clear()
	{
		m_bestMatches.clear();
	}

	//-------------------------------------------------------------------------------------------------
	const MATCHABLE* findBestInfo(const std::vector<MATCHABLE>& v, const BITSET& bits) const
	{
		typename MatchMap::const_iterator it = m_bestMatches.find(bits);

		const MATCHABLE *first = NULL;
		if (it != m_bestMatches.end())
		{
			first = (*it).second;
		}
		if (first != NULL) {
			return first;
		}

		const MATCHABLE* info = findBestInfoSlow(v, bits);

		DEBUG_ASSERTCRASH(info != NULL, ("no suitable match for criteria was found!"));
		if (info != NULL) {
			m_bestMatches[bits] = info;
		}

		return info;
	}

};

#endif // __SparseMatchFinder_H_

