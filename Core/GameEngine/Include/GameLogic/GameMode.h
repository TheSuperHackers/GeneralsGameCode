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

#pragma once

// What kind of game we're in.
enum GameMode CPP_11(: Int)
{
	GAME_SINGLE_PLAYER,
	GAME_LAN,
	GAME_SKIRMISH,
	GAME_REPLAY,
	GAME_SHELL,
	GAME_INTERNET,
	GAME_NONE
};

inline const char* toString(GameMode mode)
{
	switch (mode)
	{
		case GAME_SINGLE_PLAYER:
			return "GAME_SINGLE_PLAYER";
		case GAME_LAN:
			return "GAME_LAN";
		case GAME_SKIRMISH:
			return "GAME_SKIRMISH";
		case GAME_REPLAY:
			return "GAME_REPLAY";
		case GAME_SHELL:
			return "GAME_SHELL";
		case GAME_INTERNET:
			return "GAME_INTERNET";
		case GAME_NONE:
			return "GAME_NONE";
		default:
			return "GAME_UNKNOWN";
	}
}

namespace rts
{

// Includes Shell Game
inline Bool isGame(GameMode mode)
{
	return mode != GAME_NONE;
}

inline Bool isSinglePlayerGame(GameMode mode)
{
	return mode == GAME_SINGLE_PLAYER;
}

inline Bool isMultiplayerGame(GameMode mode)
{
	return mode == GAME_LAN || mode == GAME_INTERNET;
}

inline Bool isLanGame(GameMode mode)
{
	return mode == GAME_LAN;
}

inline Bool isInternetGame(GameMode mode)
{
	return mode == GAME_INTERNET;
}

inline Bool isShellGame(GameMode mode)
{
	return mode == GAME_SHELL;
}

inline Bool isSkirmishGame(GameMode mode)
{
	return mode == GAME_SKIRMISH;
}

inline Bool isReplayGame(GameMode mode)
{
	return mode == GAME_REPLAY;
}

inline Bool isInteractiveGame(GameMode mode)
{
	return mode != GAME_NONE && mode != GAME_SHELL;
}

} // namespace rts
