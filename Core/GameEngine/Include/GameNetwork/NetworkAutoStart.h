/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
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

#pragma once

#if defined(RTS_DEBUG)

#include "Common/AsciiString.h"
#include "Common/UnicodeString.h"
#include "GameNetwork/LANAPI.h"

// TheSuperHackers @feature bobtista 10/08/2026 Automate network match startup
// for multi-instance testing.
class NetworkAutoStart
{
public:
	enum { MIN_EXPECTED_PLAYERS = 1 };

	enum Mode
	{
		MODE_NONE,
		MODE_DIRECT_CONNECT,
	};

	enum Role
	{
		ROLE_NONE,
		ROLE_HOST,
		ROLE_JOIN,
	};

	static Bool setMode(AsciiString mode);
	static Bool setHost(Int expectedPlayers);
	static Bool setJoin(AsciiString hostAddress);
	static Bool setLocalAddress(AsciiString localAddress);
	static Bool setPlayerName(AsciiString playerName);
	static Bool setMapName(AsciiString mapName);
	static Bool setTimeoutSeconds(Int seconds);

	static Bool hasArguments();
	static Bool isEnabled();
	static Bool shouldOpenDirectConnect();
	static void markDirectConnectOpened();

	static AsciiString getMapName();
	static UnsignedInt getLocalAddress();
	static UnicodeString getPlayerName();

	static void updateDirectConnect();
	static void updateGameOptions();
	static void onGameCreate(LANAPIInterface::ReturnType result);
	static void onGameJoin(LANAPIInterface::ReturnType result);
	static void onLocalAddressSet(Bool result);
	static void onGameStartFailure();
	static void onGameStart();

private:
	static Bool validateConfiguration();
	static Bool checkTimeout();
	static void fail(const char *message);
};

#endif
