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

// ---------------------------------------------------------------------------
// File: expander.h
// Author: Matthew D. Campbell
// Creation Date: 9/13/2002
// Description: Key/value pair template expansion class
// ---------------------------------------------------------------------------

#ifndef __EXPANDER_H__
#define __EXPANDER_H__

#include <map>
#include <Utility/hash_map_adapter.h>
#include <string>

typedef std::map<std::string, std::string> ExpansionMap;

class Expander
{
	public:
		Expander( const std::string& leftMarker, const std::string& rightMarker );

		void addExpansion( const std::string& key, const std::string val );
		void clear( void );

		void expand( const std::string& input,
				std::string& output,
				bool stripUnknown = false );

	protected:
		ExpansionMap m_expansions;
		std::string m_left;
		std::string m_right;
};

#endif // __EXPANDER_H__

