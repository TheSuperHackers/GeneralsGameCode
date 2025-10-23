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

// FILE: GSConfig.h ///////////////////////////////////////////////////////////
// Author: Matthew D. Campbell, Sept 2002
// Description: GameSpy online config
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common/AsciiString.h"
#include "Common/STLTypedefs.h"

class GameSpyConfigInterface
{
public:
	virtual ~GameSpyConfigInterface() {}

	// Pings
	virtual std::list<AsciiString> getPingServers(void) = 0;
	virtual Int getNumPingRepetitions(void) = 0;
	virtual Int getPingTimeoutInMs(void) = 0;
	virtual Int getPingCutoffGood( void ) = 0;
	virtual Int getPingCutoffBad( void ) = 0; //Bryan sez, Maybe

	// QM
	virtual std::list<AsciiString> getQMMaps(void) = 0;
	virtual Int getQMBotID(void) = 0;
	virtual Int getQMChannel(void) = 0;
	virtual void setQMChannel(Int channel) = 0;

	// Player Info
	virtual Int getPointsForRank(Int rank) = 0;
	virtual Bool isPlayerVIP(Int id) = 0;

	// mangler Info
	virtual Bool getManglerLocation(Int index, AsciiString& host, UnsignedShort& port) = 0;

	// Ladder / Any other external parsing
	virtual AsciiString getLeftoverConfig(void) = 0;

	// Custom match
	virtual Bool restrictGamesToLobby() = 0;
	static GameSpyConfigInterface* create(AsciiString config);
};

extern GameSpyConfigInterface *TheGameSpyConfig;
