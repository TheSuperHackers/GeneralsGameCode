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
// File: KVPair.cpp
// Author: Matthew D. Campbell
// Creation Date: 9/4/2002
// Description: Key/Value Pair class
// ---------------------------------------------------------------------------

#include "misc.h"
#include "KVPair.h"
#include "debug.h"
#include <stdio.h>
#include <string.h>

std::string intToString(int val)
{
	std::string s;
	bool neg = (val < 0);
	if (val < 0)
	{
		val = -val;
	}
	if (val == 0)
		return "0";

	char buf[2];
	buf[1] = 0;
	while (val)
	{
		buf[0] = '0' + val%10;
		val /= 10;
		s.insert(0, buf);
	}
	if (neg)
		s.insert(0, "-");
	return s;
}

static std::string trim(std::string s, const std::string& delim)
{
	unsigned int i;
	i = s.find_first_not_of(delim);
	if (i != s.npos)
	{
		s = s.substr(i);
	}

	i = s.find_last_not_of(delim);
	if (i>=0 && i<s.npos)
	{
		s = s.substr(0, i+1);
	}

	return s;
}

static KeyValueMap parseIntoKVPairs( std::string s, const std::string& delim )
{
	KeyValueMap m;
	bool done = false;
	std::string kv, k, v;
	while (!done)
	{
		int comma;
		comma = s.find_first_of(delim, 0);
		if (comma < 1)
		{
			done = true;
			kv = s;
		}
		else
		{
			kv = s.substr(0, comma);
			s = s.substr(comma+1);
		}
		s = trim(s, delim);
		kv = trim(kv, delim);

		if (!kv.empty())
		{
			int equals = kv.find_first_of('=', 0);
			if (equals < 1)
			{
				done = true;
			}
			else
			{
				k = trim(trim(kv.substr(0, equals), delim), " \t");
				v = trim(trim(kv.substr(equals+1), delim), " \t");

				if (!k.empty() && !v.empty())
				{
					m[k] = v;
				}
			}
		}
	}
	return m;
}

KVPairClass::KVPairClass( void )
{}

KVPairClass::KVPairClass( const std::string& in, const std::string& delim )
{
	set( in, delim );
}

void KVPairClass::set( const std::string& in, const std::string& delim )
{
	m_map = parseIntoKVPairs( in, delim );
}

void KVPairClass::readFromFile( const std::string& in, const std::string& delim )
{
	m_map.clear();
	FILE *fp = fopen(in.c_str(), "rb");
	if (fp)
	{
		std::string s;
		fseek(fp, 0, SEEK_END);
		int len = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		char *buf = new char[len + 1];
		memset(buf, 0, len+1);
		fread(buf, 1, len, fp);
		fclose(fp);

		s.append(buf);
		delete[] buf;
		set(s, delim);
	}
}

std::string KVPairClass::getStringVal( const std::string& key ) const
{
	KeyValueMap::const_iterator it = m_map.find(key);
	if (it == m_map.end())
	{
		return "";
	}
	return it->second;
}

bool KVPairClass::getString( const std::string& key, std::string& val ) const
{
	std::string tmp = getStringVal(key);
	if (tmp.empty())
	{
		return false;
	}

	val = tmp;
	return true;
}

bool KVPairClass::getInt( const std::string& key, int& val ) const
{
	std::string tmp = getStringVal(key);
	if (tmp.empty())
	{
		return false;
	}

	val = atoi(tmp.c_str());
	return true;
}

bool KVPairClass::getUnsignedInt( const std::string& key, unsigned int& val ) const
{
	std::string tmp = getStringVal(key);
	if (tmp.empty())
	{
		return false;
	}

	val = atoi(tmp.c_str());
	return true;
}

