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

///////////////////////////////////////////////////////////////////////////////////////
// FILE: LadderPreferences.h
// Author: Matthew D. Campbell, August 2002
// Description: Saving/Loading of ladder preferences
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __LADDERPREFERENCES_H__
#define __LADDERPREFERENCES_H__

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "Common/UserPreferences.h"

//-----------------------------------------------------------------------------
// LadderPreferences base class
//-----------------------------------------------------------------------------

class LadderPref
{
public:
	UnicodeString name;
	AsciiString address;
	UnsignedShort port;
	time_t lastPlayDate;

	bool operator== (const LadderPref& other) const
	{
		return ( address==other.address && port==other.port );
	}
};

typedef std::map<time_t, LadderPref> LadderPrefMap;

//-----------------------------------------------------------------------------
// LadderPreferences base class
//-----------------------------------------------------------------------------
class LadderPreferences : public UserPreferences
{
public:
	LadderPreferences();
	virtual ~LadderPreferences();

	Bool loadProfile( Int profileID );
	virtual bool write( void );

	const LadderPrefMap& getRecentLadders( void );
	void addRecentLadder( LadderPref ladder );

private:
	LadderPrefMap m_ladders;
};

#endif // __LADDERPREFERENCES_H__
