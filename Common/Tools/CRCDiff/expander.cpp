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

// ---------------------------------------------------------------------------
// File: expander.cpp
// Author: Matthew D. Campbell
// Creation Date: 9/13/2002
// Description: Key/value pair template expansion class
// ---------------------------------------------------------------------------

#include "debug.h"
#include "expander.h"

Expander::Expander( const std::string& leftMarker, const std::string& rightMarker ) :
	m_left(leftMarker), m_right(rightMarker)
{
}

void Expander::addExpansion( const std::string& key, const std::string val )
{
	m_expansions[key] = val;
}

void Expander::clear( void )
{
	m_expansions.clear();
}

void Expander::expand( const std::string& input,
		std::string& output,
		bool stripUnknown )
{
	output = "";
	unsigned int pos = input.find(m_left);
	unsigned int lastpos = input.npos;
	while (pos != input.npos)
	{
		// first, tack on the non-expansion part we just skipped over
		if (pos > 0)
		{
			if (lastpos == input.npos)
			{
				// first time
				output.append(input.substr(0, pos));
				//DEBUG_LOG(("First time, output='%s'\n", output.c_str()));
			}
			else
			{
				// done this before
				std::string sub = input.substr(lastpos, pos-lastpos);
				//DEBUG_LOG(("*** lastpos = %d, pos=%d, sub='%s'\n", lastpos, pos, sub.c_str()));
				output.append(sub);
				//DEBUG_LOG(("output='%s'\n", output.c_str()));
			}
		}
		else
		{
			//DEBUG_LOG(("pos == 0\n"));
		}

		// pos is the first position of a possible expansion
		//DEBUG_LOG(("Looking for endpos via '%s' in '%s'\n", m_right.c_str(), input.substr(pos+m_left.length()).c_str()));
		unsigned int endpos = input.find(m_right, pos+m_left.length());
		//DEBUG_LOG(("substr is %d-%d of '%s'\n", pos, endpos, input.c_str()));
		if (endpos != input.npos)
		{
			// found a complete token - expand it
			std::string sub = input.substr(pos+m_left.length(), endpos-pos-m_left.length());
			//DEBUG_LOG(("found token: '%s'\n", sub.c_str()));

			ExpansionMap::iterator it = m_expansions.find(sub);
			if (it == m_expansions.end())
			{
				// unknown key
				if (stripUnknown)
				{
					//output.append("<<CENSORED>>");
				}
				else
				{
					output.append(input.substr(pos, endpos-pos+m_right.length()));
				}
			}
			else
			{
				std::string toExpand = it->second;
				std::string expanded = "";
				//DEBUG_LOG(("###### expanding '%s'\n", toExpand.c_str()));
				expand(toExpand, expanded, stripUnknown);
				//DEBUG_LOG(("###### expanded '%s'\n", expanded.c_str()));
				output.append(expanded);
			}
		}

		lastpos = endpos+m_right.length();

		pos = input.find(m_left, pos+m_left.length());
	}

	if (lastpos != input.npos)
	{
		// tack on last bit
		output.append(input.substr(lastpos));
	}
	else if (!output.length() && pos == input.npos && lastpos == input.npos)
	{
		output = input;
	}
}


