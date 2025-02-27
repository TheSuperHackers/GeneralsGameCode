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
// File: KVPair.h
// Author: Matthew D. Campbell
// Creation Date: 9/4/2002
// Description: Key/Value Pair class
// ---------------------------------------------------------------------------

#pragma once

#include <map>
#include <string>

typedef std::map<std::string, std::string> KeyValueMap;

class KVPairClass
{
public:
	KVPairClass( void );
	KVPairClass( const std::string& in, const std::string& delim );
	void set( const std::string& in, const std::string& delim );
	void readFromFile( const std::string& in, const std::string& delim );

	std::string getStringVal( const std::string& key ) const;

	bool getString( const std::string& key, std::string& val ) const;
	bool getInt( const std::string& key, int& val ) const;
	bool getUnsignedInt( const std::string& key, unsigned int& val ) const;

protected:
	KeyValueMap m_map;
};
