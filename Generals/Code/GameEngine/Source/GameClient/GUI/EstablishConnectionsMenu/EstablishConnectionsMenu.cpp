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

//// EstablishConnectionsMenu.cpp /////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameClient/GUICallbacks.h"
#include "GameClient/EstablishConnectionsMenu.h"
#include "Common/NameKeyGenerator.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/GameText.h"

EstablishConnectionsMenu *TheEstablishConnectionsMenu = NULL;

const char *const EstablishConnectionsMenu::m_playerReadyControlNames[] = {
	"EstablishConnectionsScreen.wnd:ButtonAccept1",
	"EstablishConnectionsScreen.wnd:ButtonAccept2",
	"EstablishConnectionsScreen.wnd:ButtonAccept3",
	"EstablishConnectionsScreen.wnd:ButtonAccept4",
	"EstablishConnectionsScreen.wnd:ButtonAccept5",
	"EstablishConnectionsScreen.wnd:ButtonAccept6",
	"EstablishConnectionsScreen.wnd:ButtonAccept7",
	NULL};

const char *const EstablishConnectionsMenu::m_playerNameControlNames[] = {
	"EstablishConnectionsScreen.wnd:StaticPlayer1Name",
	"EstablishConnectionsScreen.wnd:StaticPlayer2Name",
	"EstablishConnectionsScreen.wnd:StaticPlayer3Name",
	"EstablishConnectionsScreen.wnd:StaticPlayer4Name",
	"EstablishConnectionsScreen.wnd:StaticPlayer5Name",
	"EstablishConnectionsScreen.wnd:StaticPlayer6Name",
	"EstablishConnectionsScreen.wnd:StaticPlayer7Name",
	NULL
};

const char *const EstablishConnectionsMenu::m_playerStatusControlNames[] = {
	"EstablishConnectionsScreen.wnd:StaticPlayer1Status",
	"EstablishConnectionsScreen.wnd:StaticPlayer2Status",
	"EstablishConnectionsScreen.wnd:StaticPlayer3Status",
	"EstablishConnectionsScreen.wnd:StaticPlayer4Status",
	"EstablishConnectionsScreen.wnd:StaticPlayer5Status",
	"EstablishConnectionsScreen.wnd:StaticPlayer6Status",
	"EstablishConnectionsScreen.wnd:StaticPlayer7Status",
	NULL
};

/**
 Constructor
 */
EstablishConnectionsMenu::EstablishConnectionsMenu() {
}

/**
 Destructor
 */
EstablishConnectionsMenu::~EstablishConnectionsMenu() {
}

/**
 Initialize the menu
 */
void EstablishConnectionsMenu::initMenu() {
	ShowEstablishConnectionsWindow();
}

/**
 Close down the menu
 */
void EstablishConnectionsMenu::endMenu() {
	HideEstablishConnectionsWindow();
}

/**
 Abort the game gracefully...ok, as gracefully as possible considering
 the game was supposed to be started and now for some reason we have to
 stop it.  Its really sad that this game isn't going to be played
 considering how difficult it is to even get a game going in the first
 place, especially one with more than two players.
 */
void EstablishConnectionsMenu::abortGame() {
}

// the slot number passed in is the index we are to use for the menu.
void EstablishConnectionsMenu::setPlayerName(Int slot, UnicodeString name) {
	NameKeyType controlID = TheNameKeyGenerator->nameToKey(m_playerNameControlNames[slot]);
	GameWindow *control = TheWindowManager->winGetWindowFromId(NULL, controlID);

	if (control == NULL) {
		DEBUG_ASSERTCRASH(control != NULL, ("player name control for slot %d is NULL", slot));
		return;
	}
	GadgetStaticTextSetText(control, name);
}

void EstablishConnectionsMenu::setPlayerStatus(Int slot, NATConnectionState state) {
	NameKeyType controlID = TheNameKeyGenerator->nameToKey(m_playerStatusControlNames[slot]);
	GameWindow *control = TheWindowManager->winGetWindowFromId(NULL, controlID);

	if (control == NULL) {
		DEBUG_ASSERTCRASH(control != NULL, ("player status control for slot %d is NULL", slot));
		return;
	}
//	if (state == NATCONNECTIONSTATE_NETGEARDELAY) {
//		GadgetStaticTextSetText(control, TheGameText->fetch("GUI:NetgearDelay"));
	if (state == NATCONNECTIONSTATE_WAITINGFORMANGLERRESPONSE) {
		GadgetStaticTextSetText(control, TheGameText->fetch("GUI:WaitingForManglerResponse"));
	} else if (state == NATCONNECTIONSTATE_WAITINGFORMANGLEDPORT) {
		GadgetStaticTextSetText(control, TheGameText->fetch("GUI:WaitingForMangledPort"));
	} else if (state == NATCONNECTIONSTATE_WAITINGFORRESPONSE) {
		GadgetStaticTextSetText(control, TheGameText->fetch("GUI:WaitingForResponse"));
	} else if (state == NATCONNECTIONSTATE_DONE) {
		GadgetStaticTextSetText(control, TheGameText->fetch("GUI:ConnectionDone"));
	} else if (state == NATCONNECTIONSTATE_FAILED) {
		GadgetStaticTextSetText(control, TheGameText->fetch("GUI:ConnectionFailed"));
	} else if (state == NATCONNECTIONSTATE_WAITINGTOBEGIN) {
		GadgetStaticTextSetText(control, TheGameText->fetch("GUI:WaitingToBeginConnection"));
	} else {
		GadgetStaticTextSetText(control, TheGameText->fetch("GUI:UnknownConnectionState"));
	}
}
