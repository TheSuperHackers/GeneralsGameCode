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

// FILE: LadderDefs.h //////////////////////////////////////////////////////
// Generals ladder definitions
// Author: Matthew D. Campbell, August 2002

#pragma once

#include "Common/UnicodeString.h"
#include "Common/AsciiString.h"
#include "Common/STLTypedefs.h"

class GameWindow;

class LadderInfo
{
public:
	LadderInfo();
	UnicodeString name;
	UnicodeString description;
	UnicodeString location;
	Int playersPerTeam;
	Int minWins;
	Int maxWins;
	Bool randomMaps;
	Bool randomFactions;
	Bool validQM;
	Bool validCustom;
	std::list<AsciiString> validMaps;
	std::list<AsciiString> validFactions;
	AsciiString cryptedPassword;
	AsciiString address;
	UnsignedShort port;
	AsciiString homepageURL;
	Bool submitReplay; // with game results
	Int index;
};

typedef std::list<LadderInfo *> LadderInfoList;

class LadderList
{
public:
	LadderList();
	~LadderList();

	const LadderInfo* findLadder( const AsciiString& addr, UnsignedShort port );
	const LadderInfo* findLadderByIndex( Int index );	// doesn't look in local ladders
	const LadderInfoList* getLocalLadders( void );
	const LadderInfoList* getSpecialLadders( void );
	const LadderInfoList* getStandardLadders( void );

private:
	void loadLocalLadders( void );
	void checkLadder( AsciiString fname, Int index );
	LadderInfoList m_localLadders;
	LadderInfoList m_specialLadders;
	LadderInfoList m_standardLadders;
};

extern LadderList *TheLadderList;
