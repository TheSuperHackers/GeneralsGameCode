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

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameClient/DisconnectMenu.h"
#include "GameClient/GUICallbacks.h"
#include "Common/NameKeyGenerator.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GameText.h"
#include "GameNetwork/NetworkInterface.h"

const char *const DisconnectMenu::m_playerNameTextControlNames[] = {
	"DisconnectScreen.wnd:StaticPlayer1Name",
	"DisconnectScreen.wnd:StaticPlayer2Name",
	"DisconnectScreen.wnd:StaticPlayer3Name",
	"DisconnectScreen.wnd:StaticPlayer4Name",
	"DisconnectScreen.wnd:StaticPlayer5Name",
	"DisconnectScreen.wnd:StaticPlayer6Name",
	"DisconnectScreen.wnd:StaticPlayer7Name",
	NULL
};

const char *const DisconnectMenu::m_playerTimeoutTextControlNames[] = {
	"DisconnectScreen.wnd:StaticPlayer1Timeout",
	"DisconnectScreen.wnd:StaticPlayer2Timeout",
	"DisconnectScreen.wnd:StaticPlayer3Timeout",
	"DisconnectScreen.wnd:StaticPlayer4Timeout",
	"DisconnectScreen.wnd:StaticPlayer5Timeout",
	"DisconnectScreen.wnd:StaticPlayer6Timeout",
	"DisconnectScreen.wnd:StaticPlayer7Timeout",
	NULL
};

const char *const DisconnectMenu::m_playerVoteButtonControlNames[] = {
	"DisconnectScreen.wnd:ButtonKickPlayer1",
	"DisconnectScreen.wnd:ButtonKickPlayer2",
	"DisconnectScreen.wnd:ButtonKickPlayer3",
	"DisconnectScreen.wnd:ButtonKickPlayer4",
	"DisconnectScreen.wnd:ButtonKickPlayer5",
	"DisconnectScreen.wnd:ButtonKickPlayer6",
	"DisconnectScreen.wnd:ButtonKickPlayer7",
	NULL
};

const char *const DisconnectMenu::m_playerVoteCountControlNames[] = {
	"DisconnectScreen.wnd:StaticPlayer1Votes",
	"DisconnectScreen.wnd:StaticPlayer2Votes",
	"DisconnectScreen.wnd:StaticPlayer3Votes",
	"DisconnectScreen.wnd:StaticPlayer4Votes",
	"DisconnectScreen.wnd:StaticPlayer5Votes",
	"DisconnectScreen.wnd:StaticPlayer6Votes",
	"DisconnectScreen.wnd:StaticPlayer7Votes",
	NULL
};

const char *const DisconnectMenu::m_packetRouterTimeoutControlName = "DisconnectScreen.wnd:StaticPacketRouterTimeout";
const char *const DisconnectMenu::m_packetRouterTimeoutLabelControlName = "DisconnectScreen.wnd:StaticPacketRouterTimeoutLabel";
const char *const DisconnectMenu::m_textDisplayControlName = "DisconnectScreen.wnd:ListboxTextDisplay";

static const Color chatNormalColor =  GameMakeColor(255,0,0,255);

DisconnectMenu *TheDisconnectMenu = NULL;

DisconnectMenu::DisconnectMenu() {
	m_disconnectManager = NULL;
}

DisconnectMenu::~DisconnectMenu() {
}

void DisconnectMenu::init() {
	m_disconnectManager = NULL;
	HideDisconnectWindow();
	m_menuState = DISCONNECTMENUSTATETYPE_SCREENOFF;
}

void DisconnectMenu::attachDisconnectManager(DisconnectManager *disconnectManager) {
	m_disconnectManager = disconnectManager;
}

void DisconnectMenu::showScreen() {
	HideDiplomacy();
	HideInGameChat();
	HideQuitMenu();
	ShowDisconnectWindow();
	m_menuState = DISCONNECTMENUSTATETYPE_SCREENON;
}

void DisconnectMenu::hideScreen() {
	HideDisconnectWindow();
	m_menuState = DISCONNECTMENUSTATETYPE_SCREENOFF;
}

void DisconnectMenu::setPlayerName(Int playerNum, UnicodeString name) {
	NameKeyType id = TheNameKeyGenerator->nameToKey(m_playerNameTextControlNames[playerNum]);
	GameWindow *control = TheWindowManager->winGetWindowFromId(NULL, id);

	if (control != NULL) {
		if (name.getLength() > 0) {
			GadgetStaticTextSetText(control, name);
//			showPlayerControls(playerNum);
		}
	}

	id = TheNameKeyGenerator->nameToKey(m_playerTimeoutTextControlNames[playerNum]);
	control = TheWindowManager->winGetWindowFromId(NULL, id);

	if (control != NULL) {
		if (name.getLength() > 0) {
			GadgetStaticTextSetText(control, UnicodeString(L""));
		}
	}

	if (name.getLength() > 0) {
		showPlayerControls(playerNum);
	} else {
		hidePlayerControls(playerNum);
	}
}

void DisconnectMenu::setPlayerTimeoutTime(Int playerNum, time_t newTime) {
	NameKeyType id = TheNameKeyGenerator->nameToKey(m_playerTimeoutTextControlNames[playerNum]);
	GameWindow *control = TheWindowManager->winGetWindowFromId(NULL, id);

	char str[33]; // itoa uses a max of 33 bytes.
	itoa(newTime, str, 10);
	AsciiString asciiNum;
	asciiNum.set(str);
	UnicodeString uninum;
	uninum.translate(asciiNum);
	if (control != NULL) {
		GadgetStaticTextSetText(control, uninum);
	}
}

void DisconnectMenu::showPlayerControls(Int slot) {
	NameKeyType id = TheNameKeyGenerator->nameToKey(m_playerNameTextControlNames[slot]);
	GameWindow *control = TheWindowManager->winGetWindowFromId(NULL, id);
	if (control != NULL) {
		control->winHide(FALSE);
	}

	id = TheNameKeyGenerator->nameToKey(m_playerTimeoutTextControlNames[slot]);
	control = TheWindowManager->winGetWindowFromId(NULL, id);
	if (control != NULL) {
		control->winHide(FALSE);
	}

	id = TheNameKeyGenerator->nameToKey(m_playerVoteButtonControlNames[slot]);
	control = TheWindowManager->winGetWindowFromId(NULL, id);
	if (control != NULL) {
		control->winHide(FALSE);
		control->winEnable(TRUE);
	}

	id = TheNameKeyGenerator->nameToKey(m_playerVoteCountControlNames[slot]);
	control = TheWindowManager->winGetWindowFromId(NULL, id);
	if (control != NULL) {
		control->winHide(FALSE);
	}
}

void DisconnectMenu::hidePlayerControls(Int slot) {
	NameKeyType id = TheNameKeyGenerator->nameToKey(m_playerNameTextControlNames[slot]);
	GameWindow *control = TheWindowManager->winGetWindowFromId(NULL, id);
	if (control != NULL) {
		control->winHide(TRUE);
	}

	id = TheNameKeyGenerator->nameToKey(m_playerTimeoutTextControlNames[slot]);
	control = TheWindowManager->winGetWindowFromId(NULL, id);
	if (control != NULL) {
		control->winHide(TRUE);
	}

	id = TheNameKeyGenerator->nameToKey(m_playerVoteButtonControlNames[slot]);
	control = TheWindowManager->winGetWindowFromId(NULL, id);
	if (control != NULL) {
		control->winHide(TRUE);
		control->winEnable(TRUE);
	}

	id = TheNameKeyGenerator->nameToKey(m_playerVoteCountControlNames[slot]);
	control = TheWindowManager->winGetWindowFromId(NULL, id);
	if (control != NULL) {
		control->winHide(TRUE);
	}
}

void DisconnectMenu::showPacketRouterTimeout() {
	NameKeyType id = TheNameKeyGenerator->nameToKey(m_packetRouterTimeoutLabelControlName);
	GameWindow *control = TheWindowManager->winGetWindowFromId(NULL, id);

	if (control != NULL) {
		control->winHide(FALSE);
	}

	id = TheNameKeyGenerator->nameToKey(m_packetRouterTimeoutControlName);
	control = TheWindowManager->winGetWindowFromId(NULL, id);

	if (control != NULL) {
		GadgetStaticTextSetText(control, UnicodeString(L"")); // start it off with a blank string.
		control->winHide(FALSE);
	}
}

void DisconnectMenu::hidePacketRouterTimeout() {
	NameKeyType id = TheNameKeyGenerator->nameToKey(m_packetRouterTimeoutLabelControlName);
	GameWindow *control = TheWindowManager->winGetWindowFromId(NULL, id);

	if (control != NULL) {
		control->winHide(TRUE);
	}

	id = TheNameKeyGenerator->nameToKey(m_packetRouterTimeoutControlName);
	control = TheWindowManager->winGetWindowFromId(NULL, id);

	if (control != NULL) {
		control->winHide(TRUE);
	}
}

void DisconnectMenu::setPacketRouterTimeoutTime(time_t newTime) {
	NameKeyType id = TheNameKeyGenerator->nameToKey(m_packetRouterTimeoutControlName);
	GameWindow *control = TheWindowManager->winGetWindowFromId(NULL, id);

	char str[33]; // itoa uses a max of 33 bytes.
	itoa(newTime, str, 10);
	AsciiString asciiNum;
	asciiNum.set(str);
	UnicodeString uninum;
	uninum.translate(asciiNum);
	if (control != NULL) {
		GadgetStaticTextSetText(control, uninum);
	}
}

void DisconnectMenu::sendChat(UnicodeString text) {
	TheNetwork->sendDisconnectChat(text);
}

void DisconnectMenu::showChat(UnicodeString text) {
	NameKeyType displayID = TheNameKeyGenerator->nameToKey(m_textDisplayControlName);
	GameWindow *displayControl = TheWindowManager->winGetWindowFromId(NULL, displayID);

	if (displayControl != NULL) {
		GadgetListBoxAddEntryText(displayControl, text, chatNormalColor, -1, -1);
	}
}

void DisconnectMenu::quitGame() {
	TheNetwork->quitGame();
}

void DisconnectMenu::removePlayer(Int slot, UnicodeString playerName) {
	hidePlayerControls(slot);

	NameKeyType displayID = TheNameKeyGenerator->nameToKey(m_textDisplayControlName);
	GameWindow *displayControl = TheWindowManager->winGetWindowFromId(NULL, displayID);

	UnicodeString text;
//	UnicodeString name;
//	name.translate(playerName);
	text.format(TheGameText->fetch("Network:PlayerLeftGame"), playerName.str());

	if (displayControl != NULL) {
		GadgetListBoxAddEntryText(displayControl, text, chatNormalColor, -1, -1);
	}
}

void DisconnectMenu::voteForPlayer(Int slot) {
	DEBUG_LOG(("Casting vote for disconnect slot %d", slot));
	TheNetwork->voteForPlayerDisconnect(slot); // Do this next.
}

void DisconnectMenu::updateVotes(Int slot, Int votes) {
	NameKeyType id = TheNameKeyGenerator->nameToKey(m_playerVoteCountControlNames[slot]);
	GameWindow *control = TheWindowManager->winGetWindowFromId(NULL, id);

	if (control != NULL) {
		char votestr[16];
		itoa(votes, votestr, 10);
		AsciiString asciivotes;
		asciivotes.set(votestr);
		UnicodeString unistr;
		unistr.translate(asciivotes);

		GadgetStaticTextSetText(control, unistr);
	}
}
