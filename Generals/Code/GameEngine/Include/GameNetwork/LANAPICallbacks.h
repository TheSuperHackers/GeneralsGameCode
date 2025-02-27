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
// FILE: LANAPICallbacks.h
// Author: Chris Huybregts, October 2001
// Description: LAN API Callbacks header
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "Common/NameKeyGenerator.h"
#include "GameClient/Shell.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GadgetTextEntry.h"
#include "GameNetwork/LANAPI.h"


// LAN API Singleton ----------------------------------------------------------------------
extern LANAPI *TheLAN;

//external declarations of the Gadgets the callbacks can use
// LanLobby
extern NameKeyType listboxChatWindowID;
extern GameWindow *listboxChatWindow;
extern GameWindow *listboxPlayers;
extern NameKeyType listboxGamesID;
extern GameWindow *listboxGames;
//LanGame Options screen
extern NameKeyType listboxChatWindowLanGameID;
extern GameWindow *listboxChatWindowLanGame;
extern WindowLayout *mapSelectLayout;
// ScoreScreen
extern NameKeyType listboxChatWindowScoreScreenID;
extern GameWindow *listboxChatWindowScoreScreen;


//Colors used for the chat dialogs
extern const Color playerColor;
extern const Color gameColor;
extern const Color gameInProgressColor;
extern const Color chatNormalColor;
extern const Color chatActionColor;
extern const Color chatLocalNormalColor;
extern const Color chatLocalActionColor;
extern const Color chatSystemColor;
extern const Color chatSystemColor;
extern const Color acceptTrueColor;
extern const Color acceptFalseColor;


void lanUpdateSlotList( void );
void updateGameOptions( void );
void setLANPlayerTooltip(LANPlayer* player);

//Enum is used for the utility function so other windows do not need
//to know about controls on LanGameOptions window.
enum PostToLanGameType CPP_11(: Int){ SEND_GAME_OPTS = 0,
												MAP_BACK,
												POST_TO_LAN_GAME_TYPE_COUNT };
//the utility function mentioned above
void PostToLanGameOptions(PostToLanGameType post);
