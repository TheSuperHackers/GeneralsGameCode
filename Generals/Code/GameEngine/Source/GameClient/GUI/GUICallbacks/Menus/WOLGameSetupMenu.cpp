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
// FILE: WOLGameSetupMenu.cpp
// Author: Matt Campbell, December 2001
// Description: WOL Game Options Menu
///////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/GameEngine.h"
#include "Common/GameState.h"
#include "GameClient/GameText.h"
#include "Common/MultiplayerSettings.h"
#include "Common/PlayerTemplate.h"
#include "Common/CustomMatchPreferences.h"
#include "GameClient/AnimateWindowManager.h"
#include "GameClient/InGameUI.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Mouse.h"
#include "GameClient/Gadget.h"
#include "GameClient/Shell.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetComboBox.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GadgetTextEntry.h"
#include "GameClient/GadgetPushButton.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/MapUtil.h"
#include "GameClient/EstablishConnectionsMenu.h"
#include "GameClient/GameWindowTransitions.h"

#include "GameNetwork/GameSpy/BuddyDefs.h"
#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/GameSpy/PeerThread.h"
#include "GameNetwork/GameSpy/PersistentStorageDefs.h"
#include "GameNetwork/GameSpy/PersistentStorageThread.h"
#include "GameNetwork/GameSpyOverlay.h"
#include "GameNetwork/NAT.h"
#include "GameNetwork/GUIUtil.h"
#include "GameNetwork/GameSpy/GSConfig.h"

void WOLDisplaySlotList( void );


extern std::list<PeerResponse> TheLobbyQueuedUTMs;
extern void MapSelectorTooltip(GameWindow *window, WinInstanceData *instData,	UnsignedInt mouse);


#if defined(RTS_DEBUG)
extern Bool g_debugSlots;
void slotListDebugLog(const char *fmt, ...)
{
	static char buf[1024];
	va_list va;
	va_start( va, fmt );
	vsnprintf(buf, 1024, fmt, va );
	va_end( va );

	DEBUG_LOG(("%s", buf));
	if (g_debugSlots)
	{
		UnicodeString msg;
		msg.translate(buf);
		TheGameSpyInfo->addText(msg, GameSpyColor[GSCOLOR_DEFAULT], NULL);
	}
}
#define SLOTLIST_DEBUG_LOG(x) slotListDebugLog x
#else
#define SLOTLIST_DEBUG_LOG(x) DEBUG_LOG(x)
#endif

void SendStatsToOtherPlayers(const GameInfo *game)
{
	PeerRequest req;
	req.peerRequestType = PeerRequest::PEERREQUEST_UTMPLAYER;
	req.UTM.isStagingRoom = TRUE;
	req.id = "STATS/";
	AsciiString fullStr;
	PSPlayerStats fullStats = TheGameSpyPSMessageQueue->findPlayerStatsByID(TheGameSpyInfo->getLocalProfileID());
	PSPlayerStats subStats;
	subStats.id = fullStats.id;
	subStats.wins = fullStats.wins;
	subStats.losses = fullStats.losses;
	subStats.discons = fullStats.discons;
	subStats.desyncs = fullStats.desyncs;
	subStats.games = fullStats.games;
	subStats.locale = fullStats.locale;
	subStats.gamesAsRandom = fullStats.gamesAsRandom;
	GetAdditionalDisconnectsFromUserFile(&subStats);
	fullStr.format("%d %s", TheGameSpyInfo->getLocalProfileID(), TheGameSpyPSMessageQueue->formatPlayerKVPairs( subStats ).c_str());
	req.options = fullStr.str();

	Int localIndex = game->getLocalSlotNum();
	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		const GameSlot *slot = game->getConstSlot(i);
		if (slot->isHuman() && i != localIndex)
		{
			AsciiString hostName;
			hostName.translate(slot->getName());
			req.nick = hostName.str();
			DEBUG_LOG(("SendStatsToOtherPlayers() - sending to '%s', data of\n\t'%s'", hostName.str(), req.options.c_str()));
			TheGameSpyPeerMessageQueue->addRequest(req);
		}
	}
}

// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
static Bool isShuttingDown = false;
static Bool buttonPushed = false;
static const char *nextScreen = NULL;
static Bool raiseMessageBoxes = false;
static Bool launchGameNext = FALSE;

// window ids ------------------------------------------------------------------------------
static NameKeyType parentWOLGameSetupID = NAMEKEY_INVALID;

static NameKeyType comboBoxPlayerID[MAX_SLOTS] = { NAMEKEY_INVALID,NAMEKEY_INVALID,
																											NAMEKEY_INVALID,NAMEKEY_INVALID,
																											NAMEKEY_INVALID,NAMEKEY_INVALID,
																											NAMEKEY_INVALID,NAMEKEY_INVALID };

static NameKeyType staticTextPlayerID[MAX_SLOTS] = { NAMEKEY_INVALID,NAMEKEY_INVALID,
																											NAMEKEY_INVALID,NAMEKEY_INVALID,
																											NAMEKEY_INVALID,NAMEKEY_INVALID,
																											NAMEKEY_INVALID,NAMEKEY_INVALID };

static NameKeyType buttonAcceptID[MAX_SLOTS] = { NAMEKEY_INVALID,NAMEKEY_INVALID,
																									NAMEKEY_INVALID,NAMEKEY_INVALID,
																									NAMEKEY_INVALID,NAMEKEY_INVALID,
																									NAMEKEY_INVALID,NAMEKEY_INVALID };

static NameKeyType comboBoxColorID[MAX_SLOTS] = { NAMEKEY_INVALID,NAMEKEY_INVALID,
																										NAMEKEY_INVALID,NAMEKEY_INVALID,
																										NAMEKEY_INVALID,NAMEKEY_INVALID,
																										NAMEKEY_INVALID,NAMEKEY_INVALID };

static NameKeyType comboBoxPlayerTemplateID[MAX_SLOTS] = { NAMEKEY_INVALID,NAMEKEY_INVALID,
																										NAMEKEY_INVALID,NAMEKEY_INVALID,
																										NAMEKEY_INVALID,NAMEKEY_INVALID,
																										NAMEKEY_INVALID,NAMEKEY_INVALID };

static NameKeyType comboBoxTeamID[MAX_SLOTS] = { NAMEKEY_INVALID,NAMEKEY_INVALID,
																										NAMEKEY_INVALID,NAMEKEY_INVALID,
																										NAMEKEY_INVALID,NAMEKEY_INVALID,
																										NAMEKEY_INVALID,NAMEKEY_INVALID };
//static NameKeyType buttonStartPositionID[MAX_SLOTS] = { NAMEKEY_INVALID,NAMEKEY_INVALID,
//																										NAMEKEY_INVALID,NAMEKEY_INVALID,
//																										NAMEKEY_INVALID,NAMEKEY_INVALID,
//																										NAMEKEY_INVALID,NAMEKEY_INVALID };

static NameKeyType buttonMapStartPositionID[MAX_SLOTS] = { NAMEKEY_INVALID,NAMEKEY_INVALID,
																										NAMEKEY_INVALID,NAMEKEY_INVALID,
																										NAMEKEY_INVALID,NAMEKEY_INVALID,
																										NAMEKEY_INVALID,NAMEKEY_INVALID };
static NameKeyType genericPingWindowID[MAX_SLOTS] = { NAMEKEY_INVALID,NAMEKEY_INVALID,
																										NAMEKEY_INVALID,NAMEKEY_INVALID,
																										NAMEKEY_INVALID,NAMEKEY_INVALID,
																										NAMEKEY_INVALID,NAMEKEY_INVALID };

static NameKeyType textEntryChatID = NAMEKEY_INVALID;
static NameKeyType textEntryMapDisplayID = NAMEKEY_INVALID;
static NameKeyType buttonBackID = NAMEKEY_INVALID;
static NameKeyType buttonStartID = NAMEKEY_INVALID;
static NameKeyType buttonEmoteID = NAMEKEY_INVALID;
static NameKeyType buttonSelectMapID = NAMEKEY_INVALID;
static NameKeyType windowMapID = NAMEKEY_INVALID;

static NameKeyType windowMapSelectMapID = NAMEKEY_INVALID;
// Window Pointers ------------------------------------------------------------------------
static GameWindow *parentWOLGameSetup = NULL;
static GameWindow *buttonBack = NULL;
static GameWindow *buttonStart = NULL;
static GameWindow *buttonSelectMap = NULL;
static GameWindow *buttonEmote = NULL;
static GameWindow *textEntryChat = NULL;
static GameWindow *textEntryMapDisplay = NULL;
static GameWindow *windowMap = NULL;

static GameWindow *comboBoxPlayer[MAX_SLOTS] = {NULL,NULL,NULL,NULL,
																									 NULL,NULL,NULL,NULL };
static GameWindow *staticTextPlayer[MAX_SLOTS] = {NULL,NULL,NULL,NULL,
																									 NULL,NULL,NULL,NULL };
static GameWindow *buttonAccept[MAX_SLOTS] = {NULL,NULL,NULL,NULL,
																								NULL,NULL,NULL,NULL };

static GameWindow *comboBoxColor[MAX_SLOTS] = {NULL,NULL,NULL,NULL,
																								NULL,NULL,NULL,NULL };

static GameWindow *comboBoxPlayerTemplate[MAX_SLOTS] = {NULL,NULL,NULL,NULL,
																								NULL,NULL,NULL,NULL };

static GameWindow *comboBoxTeam[MAX_SLOTS] = {NULL,NULL,NULL,NULL,
																								NULL,NULL,NULL,NULL };

//static GameWindow *buttonStartPosition[MAX_SLOTS] = {NULL,NULL,NULL,NULL,
//																								NULL,NULL,NULL,NULL };
//
static GameWindow *buttonMapStartPosition[MAX_SLOTS] = {NULL,NULL,NULL,NULL,
																								NULL,NULL,NULL,NULL };

static GameWindow *genericPingWindow[MAX_SLOTS] = {NULL,NULL,NULL,NULL,
																								NULL,NULL,NULL,NULL };

static const Image *pingImages[3] = { NULL, NULL, NULL };

WindowLayout *WOLMapSelectLayout = NULL;

void PopBackToLobby( void )
{
	// delete TheNAT, its no good for us anymore.
	delete TheNAT;
	TheNAT = NULL;

	if (TheGameSpyInfo) // this can be blown away by a disconnect on the map transfer screen
	{
		TheGameSpyInfo->getCurrentStagingRoom()->reset();
		TheGameSpyInfo->leaveStagingRoom();
		//TheGameSpyInfo->joinBestGroupRoom();
	}

	DEBUG_LOG(("PopBackToLobby() - parentWOLGameSetup is %X", parentWOLGameSetup));
	if (parentWOLGameSetup)
	{
		nextScreen = "Menus/WOLCustomLobby.wnd";
		TheShell->pop();
	}
}

void updateMapStartSpots( GameInfo *myGame, GameWindow *buttonMapStartPositions[], Bool onLoadScreen = FALSE );
void positionStartSpots( GameInfo *myGame, GameWindow *buttonMapStartPositions[], GameWindow *mapWindow);
void positionStartSpots(AsciiString mapName, GameWindow *buttonMapStartPositions[], GameWindow *mapWindow);
void WOLPositionStartSpots( void )
{
	GameWindow *win = windowMap;

	if (WOLMapSelectLayout != NULL) {
		win = TheWindowManager->winGetWindowFromId(NULL, windowMapSelectMapID);

		// get the controls.
		NameKeyType listboxMapID = TheNameKeyGenerator->nameToKey( AsciiString("WOLMapSelectMenu.wnd:ListboxMap") );
		GameWindow *listboxMap = TheWindowManager->winGetWindowFromId( NULL, listboxMapID );

		if (listboxMap != NULL) {
			Int selected;
			UnicodeString map;

			// get the selected index
			GadgetListBoxGetSelected( listboxMap, &selected );

			if( selected != -1 )
			{

				// get text of the map to load
				map = GadgetListBoxGetText( listboxMap, selected, 0 );


				// set the map name in the global data map name
				AsciiString asciiMap;
				const char *mapFname = (const char *)GadgetListBoxGetItemData( listboxMap, selected );
				DEBUG_ASSERTCRASH(mapFname, ("No map item data"));
				if (mapFname) {
					asciiMap = mapFname;
				} else {
					asciiMap.translate( map );
				}

				positionStartSpots(asciiMap, buttonMapStartPosition, win);
			}
		}

	} else {
		DEBUG_ASSERTCRASH(win != NULL, ("no map preview window"));
		positionStartSpots( TheGameSpyInfo->getCurrentStagingRoom(), buttonMapStartPosition, win);
	}
}
static void savePlayerInfo( void )
{
	if (TheGameSpyGame)
	{
		Int slotNum = TheGameSpyGame->getLocalSlotNum();
		if (slotNum >= 0)
		{
			GameSpyGameSlot *slot = TheGameSpyGame->getGameSpySlot(slotNum);
			if (slot)
			{
				// save off some prefs
				CustomMatchPreferences pref;
				pref.setPreferredColor(slot->getColor());
				pref.setPreferredFaction(slot->getPlayerTemplate());
				if (TheGameSpyGame->amIHost())
				{
					pref.setPreferredMap(TheGameSpyGame->getMap());
				}
				pref.write();
			}
		}
	}
}

// Tooltips -------------------------------------------------------------------------------

static void playerTooltip(GameWindow *window,
													WinInstanceData *instData,
													UnsignedInt mouse)
{
	Int slotIdx = -1;
	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		if (window == comboBoxPlayer[i] || window == staticTextPlayer[i])
		{
			slotIdx = i;
			break;
		}
	}
	if (slotIdx < 0)
	{
		TheMouse->setCursorTooltip( UnicodeString::TheEmptyString, -1, NULL, 1.5f );
		return;
	}

	GameSpyStagingRoom *game = TheGameSpyInfo->getCurrentStagingRoom();
	if (!game)
	{
		TheMouse->setCursorTooltip( UnicodeString::TheEmptyString, -1, NULL, 1.5f );
		return;
	}

	GameSpyGameSlot *slot = game->getGameSpySlot(slotIdx);
	if (!slot || !slot->isHuman())
	{
		TheMouse->setCursorTooltip( UnicodeString::TheEmptyString, -1, NULL, 1.5f );
		return;
	}

	// for tooltip, we want:
	// * player name
	// * ping
	// * locale
	// * win/loss history
	// * discons/desyncs as one var
	// * favorite army
	// in that order.  got it?  good.

	UnicodeString uName = slot->getName();

	AsciiString aName;
	aName.translate(uName);
	PlayerInfoMap::iterator pmIt = TheGameSpyInfo->getPlayerInfoMap()->find(aName);
	if (pmIt == TheGameSpyInfo->getPlayerInfoMap()->end())
	{
		TheMouse->setCursorTooltip( uName, -1, NULL, 1.5f );
		return;
	}
	Int profileID = pmIt->second.m_profileID;

	PSPlayerStats stats = TheGameSpyPSMessageQueue->findPlayerStatsByID(profileID);
	if (stats.id == 0)
	{
		TheMouse->setCursorTooltip( uName, -1, NULL, 1.5f );
		return;
	}

	Bool isLocalPlayer = slot == game->getGameSpySlot(game->getLocalSlotNum());

	AsciiString localeIdentifier;
	localeIdentifier.format("WOL:Locale%2.2d", stats.locale);
	UnicodeString	playerInfo;
	Int totalWins = 0, totalLosses = 0, totalDiscons = 0;
	PerGeneralMap::iterator it;

	for (it = stats.wins.begin(); it != stats.wins.end(); ++it)
	{
		totalWins += it->second;
	}
	for (it = stats.losses.begin(); it != stats.losses.end(); ++it)
	{
		totalLosses += it->second;
	}
	for (it = stats.discons.begin(); it != stats.discons.end(); ++it)
	{
		totalDiscons += it->second;
	}
	for (it = stats.desyncs.begin(); it != stats.desyncs.end(); ++it)
	{
		totalDiscons += it->second;
	}
	UnicodeString favoriteSide;
	Int numGames = 0;
	Int favorite = 0;
	for(it = stats.games.begin(); it != stats.games.end(); ++it)
	{
		if(it->second >= numGames)
		{
			numGames = it->second;
			favorite = it->first;
		}
	}
	if(numGames == 0)
		favoriteSide = TheGameText->fetch("GUI:None");
	else if( stats.gamesAsRandom >= numGames )
		favoriteSide = TheGameText->fetch("GUI:Random");
	else
	{
		const PlayerTemplate *fac = ThePlayerTemplateStore->getNthPlayerTemplate(favorite);
		if (fac)
		{
			AsciiString side;
			side.format("SIDE:%s", fac->getSide().str());

			favoriteSide = TheGameText->fetch(side);
		}
	}

	playerInfo.format(TheGameText->fetch("TOOLTIP:StagingPlayerInfo"),
		TheGameText->fetch(localeIdentifier).str(),
		slot->getPingAsInt(),
		totalWins, totalLosses, totalDiscons,
		favoriteSide.str());

	UnicodeString tooltip = UnicodeString::TheEmptyString;
	if (isLocalPlayer)
	{
		tooltip.format(TheGameText->fetch("TOOLTIP:LocalPlayer"), uName.str());
	}
	else
	{
		// not us
		if (TheGameSpyInfo->getBuddyMap()->find(profileID) != TheGameSpyInfo->getBuddyMap()->end())
		{
			// buddy
			tooltip.format(TheGameText->fetch("TOOLTIP:BuddyPlayer"), uName.str());
		}
		else
		{
			if (profileID)
			{
				// non-buddy profiled player
				tooltip.format(TheGameText->fetch("TOOLTIP:ProfiledPlayer"), uName.str());
			}
			else
			{
				// non-profiled player
				tooltip.format(TheGameText->fetch("TOOLTIP:GenericPlayer"), uName.str());
			}
		}
	}

	tooltip.concat(playerInfo);

	TheMouse->setCursorTooltip( tooltip, -1, NULL, 1.5f ); // the text and width are the only params used.  the others are the default values.
}

void gameAcceptTooltip(GameWindow *window, WinInstanceData *instData, UnsignedInt mouse)
{
	Int x, y;
	x = LOLONGTOSHORT(mouse);
	y = HILONGTOSHORT(mouse);

	Int winPosX, winPosY, winWidth, winHeight;

	window->winGetScreenPosition(&winPosX, &winPosY);

	window->winGetSize(&winWidth, &winHeight);

	if ((x > winPosX && x < (winPosX + winWidth)) && (y > winPosY && y < (winPosY + winHeight)))
	{
		TheMouse->setCursorTooltip(TheGameText->fetch("TOOLTIP:GameAcceptance"), -1, NULL);
	}
}

void pingTooltip(GameWindow *window, WinInstanceData *instData, UnsignedInt mouse)
{
	Int x, y;
	x = LOLONGTOSHORT(mouse);
	y = HILONGTOSHORT(mouse);


	Int winPosX, winPosY, winWidth, winHeight;

	window->winGetScreenPosition(&winPosX, &winPosY);

	window->winGetSize(&winWidth, &winHeight);

	if ((x > winPosX && x < (winPosX + winWidth)) && (y > winPosY && y < (winPosY + winHeight)))
	{
		TheMouse->setCursorTooltip(TheGameText->fetch("TOOLTIP:ConnectionSpeed"), -1, NULL);
	}
}

//external declarations of the Gadgets the callbacks can use
GameWindow *listboxGameSetupChat = NULL;
NameKeyType listboxGameSetupChatID = NAMEKEY_INVALID;

static void handleColorSelection(int index)
{
	GameWindow *combo = comboBoxColor[index];
	Int color, selIndex;
	GadgetComboBoxGetSelectedPos(combo, &selIndex);
	color = (Int)GadgetComboBoxGetItemData(combo, selIndex);

	GameInfo *myGame = TheGameSpyInfo->getCurrentStagingRoom();

	if (myGame)
	{
		GameSlot * slot = myGame->getSlot(index);
		if (color == slot->getColor())
			return;

		if (color >= -1 && color < TheMultiplayerSettings->getNumColors())
		{
			Bool colorAvailable = TRUE;
			if(color != -1 )
			{
				for(Int i=0; i <MAX_SLOTS; i++)
				{
					GameSlot *checkSlot = myGame->getSlot(i);
					if(color == checkSlot->getColor() && slot != checkSlot)
					{
						colorAvailable = FALSE;
						break;
					}
				}
			}
			if(!colorAvailable)
				return;
		}

		slot->setColor(color);

		if (TheGameSpyInfo->amIHost())
		{
			// send around a new slotlist
			TheGameSpyInfo->setGameOptions();
			WOLDisplaySlotList();
		}
		else
		{
			// request the color from the host
			if (!slot->isPlayer(TheGameSpyInfo->getLocalName()))
				return;

			AsciiString options;
			options.format("Color=%d", color);
			AsciiString hostName;
			hostName.translate(myGame->getSlot(0)->getName());
			PeerRequest req;
			req.peerRequestType = PeerRequest::PEERREQUEST_UTMPLAYER;
			req.UTM.isStagingRoom = TRUE;
			req.id = "REQ/";
			req.nick = hostName.str();
			req.options = options.str();
			TheGameSpyPeerMessageQueue->addRequest(req);
		}
	}
}

static void handlePlayerTemplateSelection(int index)
{
	GameWindow *combo = comboBoxPlayerTemplate[index];
	Int playerTemplate, selIndex;
	GadgetComboBoxGetSelectedPos(combo, &selIndex);
	playerTemplate = (Int)GadgetComboBoxGetItemData(combo, selIndex);
	GameInfo *myGame = TheGameSpyInfo->getCurrentStagingRoom();

	if (myGame)
	{
		GameSlot * slot = myGame->getSlot(index);
		if (playerTemplate == slot->getPlayerTemplate())
			return;

		Int oldTemplate = slot->getPlayerTemplate();
		slot->setPlayerTemplate(playerTemplate);

		if (oldTemplate == PLAYERTEMPLATE_OBSERVER)
		{
			// was observer, so populate color & team with all, and enable
			GadgetComboBoxSetSelectedPos(comboBoxColor[index], 0);
			GadgetComboBoxSetSelectedPos(comboBoxTeam[index], 0);
			slot->setStartPos(-1);
		}
		else if (playerTemplate == PLAYERTEMPLATE_OBSERVER)
		{
			// is becoming observer, so populate color & team with random only, and disable
			GadgetComboBoxSetSelectedPos(comboBoxColor[index], 0);
			GadgetComboBoxSetSelectedPos(comboBoxTeam[index], 0);
			slot->setStartPos(-1);
		}


		if (TheGameSpyInfo->amIHost())
		{
			// send around a new slotlist
			myGame->resetAccepted();
			TheGameSpyInfo->setGameOptions();
			WOLDisplaySlotList();
		}
		else
		{
			// request the playerTemplate from the host
			AsciiString options;
			options.format("PlayerTemplate=%d", playerTemplate);
			AsciiString hostName;
			hostName.translate(myGame->getSlot(0)->getName());
			PeerRequest req;
			req.peerRequestType = PeerRequest::PEERREQUEST_UTMPLAYER;
			req.UTM.isStagingRoom = TRUE;
			req.id = "REQ/";
			req.nick = hostName.str();
			req.options = options.str();
			TheGameSpyPeerMessageQueue->addRequest(req);
		}
	}
}


static void handleStartPositionSelection(Int player, int startPos)
{
	GameSpyStagingRoom *myGame = TheGameSpyInfo->getCurrentStagingRoom();

	if (myGame)
	{
		GameSpyGameSlot * slot = myGame->getGameSpySlot(player);
		if (!slot)
			return;

		if (startPos == slot->getStartPos())
			return;
		Bool skip = FALSE;
		if (startPos < 0)
		{
			skip = TRUE;
		}

		if(!skip)
		{
			Bool isAvailable = TRUE;
			for(Int i = 0; i < MAX_SLOTS; ++i)
			{
				if(i != player && myGame->getSlot(i)->getStartPos() == startPos)
				{
					isAvailable = FALSE;
					break;
				}
			}
			if( !isAvailable )
				return;
		}
		slot->setStartPos(startPos);

		if (myGame->amIHost())
		{
			// send around a new slotlist
			myGame->resetAccepted();
			TheGameSpyInfo->setGameOptions();
			WOLDisplaySlotList();
		}
		else
		{
			// request the color from the host
			if (AreSlotListUpdatesEnabled())
			{
				// request the playerTemplate from the host
				AsciiString options;
				options.format("StartPos=%d", slot->getStartPos());
				AsciiString hostName;
				hostName.translate(myGame->getSlot(0)->getName());
				PeerRequest req;
				req.peerRequestType = PeerRequest::PEERREQUEST_UTMPLAYER;
				req.UTM.isStagingRoom = TRUE;
				req.id = "REQ/";
				req.nick = hostName.str();
				req.options = options.str();
				TheGameSpyPeerMessageQueue->addRequest(req);

			}
		}
	}
}



static void handleTeamSelection(int index)
{
	GameWindow *combo = comboBoxTeam[index];
	Int team, selIndex;
	GadgetComboBoxGetSelectedPos(combo, &selIndex);
	team = (Int)GadgetComboBoxGetItemData(combo, selIndex);
	GameInfo *myGame = TheGameSpyInfo->getCurrentStagingRoom();

	if (myGame)
	{
		GameSlot * slot = myGame->getSlot(index);
		if (team == slot->getTeamNumber())
			return;

		slot->setTeamNumber(team);

		if (TheGameSpyInfo->amIHost())
		{
			// send around a new slotlist
			myGame->resetAccepted();
			TheGameSpyInfo->setGameOptions();
			WOLDisplaySlotList();
		}
		else
		{
			// request the team from the host
			AsciiString options;
			options.format("Team=%d", team);
			AsciiString hostName;
			hostName.translate(myGame->getSlot(0)->getName());
			PeerRequest req;
			req.peerRequestType = PeerRequest::PEERREQUEST_UTMPLAYER;
			req.UTM.isStagingRoom = TRUE;
			req.id = "REQ/";
			req.nick = hostName.str();
			req.options = options.str();
			TheGameSpyPeerMessageQueue->addRequest(req);
		}
	}
}


static void StartPressed(void)
{
	Bool isReady = TRUE;
	Bool allHaveMap = TRUE;
	Int playerCount = 0;
	Int humanCount = 0;
	GameSpyStagingRoom *myGame = TheGameSpyInfo->getCurrentStagingRoom();
	if (!myGame)
		return;

	// see if everyone's accepted and count the number of players in the game
	UnicodeString mapDisplayName;
	const MapMetaData *mapData = TheMapCache->findMap( myGame->getMap() );
	Bool willTransfer = TRUE;
	if (mapData)
	{
		mapDisplayName.format(L"%ls", mapData->m_displayName.str());
		willTransfer = !mapData->m_isOfficial;
	}
	else
	{
		mapDisplayName.format(L"%hs", myGame->getMap().str());
		willTransfer = WouldMapTransfer(myGame->getMap());
	}
	int i = 0;
	for( ; i < MAX_SLOTS; i++ )
	{
		if ((myGame->getSlot(i)->isAccepted() == FALSE) && (myGame->getSlot(i)->isHuman() == TRUE))
		{
			isReady = FALSE;
			if (!myGame->getSlot(i)->hasMap() && !willTransfer)
			{
				UnicodeString msg;
				msg.format(TheGameText->fetch("GUI:PlayerNoMap"), myGame->getSlot(i)->getName().str(), mapDisplayName.str());
				TheGameSpyInfo->addText(msg, GameSpyColor[GSCOLOR_DEFAULT], listboxGameSetupChat);
				allHaveMap = FALSE;
			}
		}
		if(myGame->getSlot(i)->isOccupied() && myGame->getSlot(i)->getPlayerTemplate() != PLAYERTEMPLATE_OBSERVER)
		{
			if (myGame->getSlot(i)->isHuman())
				humanCount++;
			playerCount++;
		}
	}

	// Check for too many players
	const MapMetaData *md = TheMapCache->findMap( myGame->getMap() );
	if (!md || md->m_numPlayers < playerCount)
	{
		if (myGame->amIHost())
		{
			UnicodeString text;
			text.format(TheGameText->fetch("LAN:TooManyPlayers"), (md)?md->m_numPlayers:0);
			TheGameSpyInfo->addText(text, GameSpyColor[GSCOLOR_DEFAULT], listboxGameSetupChat);
		}
		return;
	}

	// Check for observer + AI players
	if (TheGlobalData->m_netMinPlayers && !humanCount)
	{
		if (myGame->amIHost())
		{
			UnicodeString text = TheGameText->fetch("GUI:NeedHumanPlayers");
			TheGameSpyInfo->addText(text, GameSpyColor[GSCOLOR_DEFAULT], listboxGameSetupChat);
		}
		return;
	}

	// Check for too few players
	if (playerCount < TheGlobalData->m_netMinPlayers)
	{
		if (myGame->amIHost())
		{
			UnicodeString text;
			text.format(TheGameText->fetch("LAN:NeedMorePlayers"),playerCount);
			TheGameSpyInfo->addText(text, GameSpyColor[GSCOLOR_DEFAULT], listboxGameSetupChat);
		}
		return;
	}

	// Check for too few teams
	int numRandom = 0;
	std::set<Int> teams;
	for (i=0; i<MAX_SLOTS; ++i)
	{
		GameSlot *slot = myGame->getSlot(i);
		if (slot && slot->isOccupied() && slot->getPlayerTemplate() != PLAYERTEMPLATE_OBSERVER)
		{
			if (slot->getTeamNumber() >= 0)
			{
				teams.insert(slot->getTeamNumber());
			}
			else
			{
				++numRandom;
			}
		}
	}
	if (numRandom + teams.size() < TheGlobalData->m_netMinPlayers)
	{
		if (myGame->amIHost())
		{
			UnicodeString text;
			text.format(TheGameText->fetch("LAN:NeedMoreTeams"));
			TheGameSpyInfo->addText(text, GameSpyColor[GSCOLOR_DEFAULT], listboxGameSetupChat);
		}
		return;
	}

	if (numRandom + teams.size() < 2)
	{
		UnicodeString text;
		text.format(TheGameText->fetch("GUI:SandboxMode"));
		TheGameSpyInfo->addText(text, GameSpyColor[GSCOLOR_DEFAULT], listboxGameSetupChat);
	}

	if(isReady)
	{
		PeerRequest req;
		req.peerRequestType = PeerRequest::PEERREQUEST_STARTGAME;
		TheGameSpyPeerMessageQueue->addRequest(req);

		SendStatsToOtherPlayers(myGame);

		// we've started, there's no going back
		// i.e. disable the back button.
		buttonBack->winEnable(FALSE);
		GameWindow *buttonBuddy = TheWindowManager->winGetWindowFromId(NULL, NAMEKEY("GameSpyGameOptionsMenu.wnd:ButtonCommunicator"));
		if (buttonBuddy)
			buttonBuddy->winEnable(FALSE);
		GameSpyCloseOverlay(GSOVERLAY_BUDDY);

		*TheGameSpyGame = *myGame;
		TheGameSpyGame->startGame(0);
	}
	else if (allHaveMap)
	{
		TheGameSpyInfo->addText(TheGameText->fetch("GUI:NotifiedStartIntent"), GameSpyColor[GSCOLOR_DEFAULT], listboxGameSetupChat);
		PeerRequest req;
		req.peerRequestType = PeerRequest::PEERREQUEST_UTMROOM;
		req.UTM.isStagingRoom = TRUE;
		req.id = "HWS/";
		req.options = "true";
		TheGameSpyPeerMessageQueue->addRequest(req);
	}

}//void StartPressed(void)

//-------------------------------------------------------------------------------------------------
/** Update options on screen */
//-------------------------------------------------------------------------------------------------
void WOLDisplayGameOptions( void )
{
	GameSpyStagingRoom *theGame = TheGameSpyInfo->getCurrentStagingRoom();
	if (!parentWOLGameSetup || !theGame)
		return;

	const GameSlot *localSlot = NULL;
	if (theGame->getLocalSlotNum() >= 0)
		localSlot = theGame->getConstSlot(theGame->getLocalSlotNum());

	const MapMetaData *md = TheMapCache->findMap(TheGameSpyInfo->getCurrentStagingRoom()->getMap());
	if (md && localSlot && localSlot->hasMap())
	{
		GadgetStaticTextSetText(textEntryMapDisplay, md->m_displayName);
	}
	else
	{
		AsciiString s = TheGameSpyInfo->getCurrentStagingRoom()->getMap();
		if (s.reverseFind('\\'))
		{
			s = s.reverseFind('\\') + 1;
		}
		UnicodeString mapDisplay;
		mapDisplay.translate(s);
		GadgetStaticTextSetText(textEntryMapDisplay, mapDisplay);
	}
	WOLPositionStartSpots();
	updateMapStartSpots(TheGameSpyInfo->getCurrentStagingRoom(), buttonMapStartPosition);
}

//  -----------------------------------------------------------------------------------------
// The Bad munkee slot list displaying function
//-------------------------------------------------------------------------------------------------
void WOLDisplaySlotList( void )
{
	if (!parentWOLGameSetup || !TheGameSpyInfo->getCurrentStagingRoom())
		return;

	GameSpyStagingRoom *game = TheGameSpyInfo->getCurrentStagingRoom();
	if (!game->isInGame())
		return;

	DEBUG_ASSERTCRASH(!game->getConstSlot(0)->isOpen(), ("Open host!"));

	UpdateSlotList( game, comboBoxPlayer, comboBoxColor,
		comboBoxPlayerTemplate, comboBoxTeam, buttonAccept, buttonStart, buttonMapStartPosition );

	WOLDisplayGameOptions();

	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		GameSpyGameSlot *slot = game->getGameSpySlot(i);
		if (slot && slot->isHuman())
		{
			if (i == game->getLocalSlotNum())
			{
				// set up my own ping...
				slot->setPingString(TheGameSpyInfo->getPingString());
			}

			if (genericPingWindow[i])
			{
				genericPingWindow[i]->winHide(FALSE);
				Int ping = slot->getPingAsInt();
				if (ping < TheGameSpyConfig->getPingCutoffGood())
				{
					genericPingWindow[i]->winSetEnabledImage(0, pingImages[0]);
				}
				else if (ping < TheGameSpyConfig->getPingCutoffBad())
				{
					genericPingWindow[i]->winSetEnabledImage(0, pingImages[1]);
				}
				else
				{
					genericPingWindow[i]->winSetEnabledImage(0, pingImages[2]);
				}
			}
		}
		else
		{
			if (genericPingWindow[i])
				genericPingWindow[i]->winHide(TRUE);
		}
	}
}

//-------------------------------------------------------------------------------------------------
/** Initialize the Gadgets Options Menu */
//-------------------------------------------------------------------------------------------------
void InitWOLGameGadgets( void )
{
	GameSpyStagingRoom *theGameInfo = TheGameSpyInfo->getCurrentStagingRoom();
	pingImages[0] = TheMappedImageCollection->findImageByName("Ping03");
	pingImages[1] = TheMappedImageCollection->findImageByName("Ping02");
	pingImages[2] = TheMappedImageCollection->findImageByName("Ping01");
	DEBUG_ASSERTCRASH(pingImages[0], ("Can't find ping image!"));
	DEBUG_ASSERTCRASH(pingImages[1], ("Can't find ping image!"));
	DEBUG_ASSERTCRASH(pingImages[2], ("Can't find ping image!"));

	//Initialize the gadget IDs
	parentWOLGameSetupID = TheNameKeyGenerator->nameToKey( AsciiString( "GameSpyGameOptionsMenu.wnd:GameSpyGameOptionsMenuParent" ) );
	buttonBackID = TheNameKeyGenerator->nameToKey( AsciiString( "GameSpyGameOptionsMenu.wnd:ButtonBack" ) );
	buttonStartID = TheNameKeyGenerator->nameToKey( AsciiString( "GameSpyGameOptionsMenu.wnd:ButtonStart" ) );
	textEntryChatID = TheNameKeyGenerator->nameToKey( AsciiString( "GameSpyGameOptionsMenu.wnd:TextEntryChat" ) );
	textEntryMapDisplayID = TheNameKeyGenerator->nameToKey( AsciiString( "GameSpyGameOptionsMenu.wnd:TextEntryMapDisplay" ) );
	listboxGameSetupChatID = TheNameKeyGenerator->nameToKey( AsciiString( "GameSpyGameOptionsMenu.wnd:ListboxChatWindowGameSpyGameSetup" ) );
	buttonEmoteID = TheNameKeyGenerator->nameToKey( AsciiString( "GameSpyGameOptionsMenu.wnd:ButtonEmote" ) );
	buttonSelectMapID = TheNameKeyGenerator->nameToKey( AsciiString( "GameSpyGameOptionsMenu.wnd:ButtonSelectMap" ) );
	windowMapID = TheNameKeyGenerator->nameToKey( AsciiString( "GameSpyGameOptionsMenu.wnd:MapWindow" ) );
	windowMapSelectMapID = TheNameKeyGenerator->nameToKey(AsciiString("WOLMapSelectMenu.wnd:WinMapPreview"));

	NameKeyType staticTextTitleID = NAMEKEY("GameSpyGameOptionsMenu.wnd:StaticTextGameName");

	// Initialize the pointers to our gadgets
	parentWOLGameSetup = TheWindowManager->winGetWindowFromId( NULL, parentWOLGameSetupID );
	buttonEmote = TheWindowManager->winGetWindowFromId( parentWOLGameSetup,buttonEmoteID  );
	buttonSelectMap = TheWindowManager->winGetWindowFromId( parentWOLGameSetup,buttonSelectMapID  );
	buttonStart = TheWindowManager->winGetWindowFromId( parentWOLGameSetup,buttonStartID  );
	buttonBack = TheWindowManager->winGetWindowFromId( parentWOLGameSetup,  buttonBackID);
	listboxGameSetupChat = TheWindowManager->winGetWindowFromId( parentWOLGameSetup, listboxGameSetupChatID );
	textEntryChat = TheWindowManager->winGetWindowFromId( parentWOLGameSetup, textEntryChatID );
	textEntryMapDisplay = TheWindowManager->winGetWindowFromId( parentWOLGameSetup, textEntryMapDisplayID );
	windowMap = TheWindowManager->winGetWindowFromId( parentWOLGameSetup,windowMapID  );
	DEBUG_ASSERTCRASH(windowMap, ("Could not find the parentWOLGameSetup.wnd:MapWindow" ));

	//Added By Sadullah Nader
	//Tooltip Function set
	windowMap->winSetTooltipFunc(MapSelectorTooltip);
	//

	GameWindow *staticTextTitle = TheWindowManager->winGetWindowFromId( parentWOLGameSetup, staticTextTitleID );
	if (staticTextTitle)
	{
		GadgetStaticTextSetText(staticTextTitle, TheGameSpyGame->getGameName());
	}

	if (!theGameInfo)
	{
		DEBUG_CRASH(("No staging room!"));
		return;
	}

	for (Int i = 0; i < MAX_SLOTS; i++)
	{
		AsciiString tmpString;
		tmpString.format("GameSpyGameOptionsMenu.wnd:ComboBoxPlayer%d", i);
		comboBoxPlayerID[i] = TheNameKeyGenerator->nameToKey( tmpString );
		comboBoxPlayer[i] = TheWindowManager->winGetWindowFromId( parentWOLGameSetup, comboBoxPlayerID[i] );
		GadgetComboBoxReset(comboBoxPlayer[i]);
		comboBoxPlayer[i]->winSetTooltipFunc(playerTooltip);

		tmpString.format("GameSpyGameOptionsMenu.wnd:StaticTextPlayer%d", i);
		staticTextPlayerID[i] = TheNameKeyGenerator->nameToKey( tmpString );
		staticTextPlayer[i] = TheWindowManager->winGetWindowFromId( parentWOLGameSetup, staticTextPlayerID[i] );
		staticTextPlayer[i]->winSetTooltipFunc(playerTooltip);
		if (TheGameSpyInfo->amIHost())
			staticTextPlayer[i]->winHide(TRUE);

		if(theGameInfo->getLocalSlotNum() != i)
		{
			GadgetComboBoxAddEntry(comboBoxPlayer[i],TheGameText->fetch("GUI:Open"),GameSpyColor[GSCOLOR_PLAYER_NORMAL]);
			GadgetComboBoxAddEntry(comboBoxPlayer[i],TheGameText->fetch("GUI:Closed"),GameSpyColor[GSCOLOR_PLAYER_NORMAL]);
			GadgetComboBoxAddEntry(comboBoxPlayer[i],TheGameText->fetch("GUI:EasyAI"),GameSpyColor[GSCOLOR_PLAYER_NORMAL]);
			GadgetComboBoxAddEntry(comboBoxPlayer[i],TheGameText->fetch("GUI:MediumAI"),GameSpyColor[GSCOLOR_PLAYER_NORMAL]);
			GadgetComboBoxAddEntry(comboBoxPlayer[i],TheGameText->fetch("GUI:HardAI"),GameSpyColor[GSCOLOR_PLAYER_NORMAL]);
			GadgetComboBoxSetSelectedPos(comboBoxPlayer[i],0);
		}

		tmpString.format("GameSpyGameOptionsMenu.wnd:ComboBoxColor%d", i);
		comboBoxColorID[i] = TheNameKeyGenerator->nameToKey( tmpString );
		comboBoxColor[i] = TheWindowManager->winGetWindowFromId( parentWOLGameSetup, comboBoxColorID[i] );
		DEBUG_ASSERTCRASH(comboBoxColor[i], ("Could not find the comboBoxColor[%d]",i ));
		PopulateColorComboBox(i, comboBoxColor, theGameInfo);
		GadgetComboBoxSetSelectedPos(comboBoxColor[i], 0);

		tmpString.format("GameSpyGameOptionsMenu.wnd:ComboBoxPlayerTemplate%d", i);
		comboBoxPlayerTemplateID[i] = TheNameKeyGenerator->nameToKey( tmpString );
		comboBoxPlayerTemplate[i] = TheWindowManager->winGetWindowFromId( parentWOLGameSetup, comboBoxPlayerTemplateID[i] );
		DEBUG_ASSERTCRASH(comboBoxPlayerTemplate[i], ("Could not find the comboBoxPlayerTemplate[%d]",i ));
		PopulatePlayerTemplateComboBox(i, comboBoxPlayerTemplate, theGameInfo, theGameInfo->getAllowObservers());

		tmpString.format("GameSpyGameOptionsMenu.wnd:ComboBoxTeam%d", i);
		comboBoxTeamID[i] = TheNameKeyGenerator->nameToKey( tmpString );
		comboBoxTeam[i] = TheWindowManager->winGetWindowFromId( parentWOLGameSetup, comboBoxTeamID[i] );
		DEBUG_ASSERTCRASH(comboBoxTeam[i], ("Could not find the comboBoxTeam[%d]",i ));
		PopulateTeamComboBox(i, comboBoxTeam, theGameInfo);

		tmpString.format("GameSpyGameOptionsMenu.wnd:ButtonAccept%d", i);
		buttonAcceptID[i] = TheNameKeyGenerator->nameToKey( tmpString );
		buttonAccept[i] = TheWindowManager->winGetWindowFromId( parentWOLGameSetup, buttonAcceptID[i] );
		DEBUG_ASSERTCRASH(buttonAccept[i], ("Could not find the buttonAccept[%d]",i ));
		buttonAccept[i]->winSetTooltipFunc(gameAcceptTooltip);

		tmpString.format("GameSpyGameOptionsMenu.wnd:GenericPing%d", i);
		genericPingWindowID[i] = TheNameKeyGenerator->nameToKey( tmpString );
		genericPingWindow[i] = TheWindowManager->winGetWindowFromId( parentWOLGameSetup, genericPingWindowID[i] );
		DEBUG_ASSERTCRASH(genericPingWindow[i], ("Could not find the genericPingWindow[%d]",i ));
		genericPingWindow[i]->winSetTooltipFunc(pingTooltip);

//		tmpString.format("GameSpyGameOptionsMenu.wnd:ButtonStartPosition%d", i);
//		buttonStartPositionID[i] = TheNameKeyGenerator->nameToKey( tmpString );
//		buttonStartPosition[i] = TheWindowManager->winGetWindowFromId( parentWOLGameSetup, buttonStartPositionID[i] );
//		DEBUG_ASSERTCRASH(buttonStartPosition[i], ("Could not find the ButtonStartPosition[%d]",i ));

		tmpString.format("GameSpyGameOptionsMenu.wnd:ButtonMapStartPosition%d", i);
		buttonMapStartPositionID[i] = TheNameKeyGenerator->nameToKey( tmpString );
		buttonMapStartPosition[i] = TheWindowManager->winGetWindowFromId( parentWOLGameSetup, buttonMapStartPositionID[i] );
		DEBUG_ASSERTCRASH(buttonMapStartPosition[i], ("Could not find the ButtonMapStartPosition[%d]",i ));

//		if (buttonStartPosition[i])
//			buttonStartPosition[i]->winHide(TRUE);

		if(i !=0 && buttonAccept[i])
			buttonAccept[i]->winHide(TRUE);
	}

	if( buttonAccept[0] )
		buttonAccept[0]->winEnable(TRUE);

	if (buttonBack != NULL)
	{
		buttonBack->winEnable(TRUE);
	}
		//GadgetButtonSetEnabledColor(buttonAccept[0], GameSpyColor[GSCOLOR_ACCEPT_TRUE]);
}

void DeinitWOLGameGadgets( void )
{
	parentWOLGameSetup = NULL;
	buttonEmote = NULL;
	buttonSelectMap = NULL;
	buttonStart = NULL;
	buttonBack = NULL;
	listboxGameSetupChat = NULL;
	textEntryChat = NULL;
	textEntryMapDisplay = NULL;
	if (windowMap)
	{
		windowMap->winSetUserData(NULL);
		windowMap = NULL;
	}
//	GameWindow *staticTextTitle = NULL;
	for (Int i = 0; i < MAX_SLOTS; i++)
	{
		comboBoxPlayer[i] = NULL;
		staticTextPlayer[i] = NULL;
		comboBoxColor[i] = NULL;
		comboBoxPlayerTemplate[i] = NULL;
		comboBoxTeam[i] = NULL;
		buttonAccept[i] = NULL;
//		buttonStartPosition[i] = NULL;
		buttonMapStartPosition[i] = NULL;
		genericPingWindow[i] = NULL;
	}
}

static Bool initDone = false;
UnsignedInt lastSlotlistTime = 0;
UnsignedInt enterTime = 0;
Bool initialAcceptEnable = FALSE;
//-------------------------------------------------------------------------------------------------
/** Initialize the Lan Game Options Menu */
//-------------------------------------------------------------------------------------------------
void WOLGameSetupMenuInit( WindowLayout *layout, void *userData )
{
	if (TheGameSpyGame && TheGameSpyGame->isGameInProgress())
	{
		TheGameSpyGame->setGameInProgress(FALSE);

		// check if we were disconnected
		Int disconReason;
		if (TheGameSpyInfo->isDisconnectedAfterGameStart(&disconReason))
		{
			AsciiString disconMunkee;
			disconMunkee.format("GUI:GSDisconReason%d", disconReason);
			UnicodeString title, body;
			title = TheGameText->fetch( "GUI:GSErrorTitle" );
			body = TheGameText->fetch( disconMunkee );
			GameSpyCloseAllOverlays();
			GSMessageBoxOk( title, body );
			TheGameSpyInfo->reset();
			DEBUG_LOG(("WOLGameSetupMenuInit() - game was in progress, and we were disconnected, so pop immediate back to main menu"));
			TheShell->popImmediate();
			return;
		}

		// If we init while the game is in progress, we are really returning to the menu
		// after the game.  So, we pop the menu and go back to the lobby.  Whee!
		DEBUG_LOG(("WOLGameSetupMenuInit() - game was in progress, so pop immediate back to lobby"));
		TheShell->popImmediate();
		if (TheGameSpyPeerMessageQueue && TheGameSpyPeerMessageQueue->isConnected())
		{
			DEBUG_LOG(("We're still connected, so pushing back on the lobby"));
			TheShell->push("Menus/WOLCustomLobby.wnd", TRUE);
		}
		return;
	}
	TheGameSpyInfo->setCurrentGroupRoom(0);

	if (TheNAT != NULL) {
		delete TheNAT;
		TheNAT = NULL;
	}

	nextScreen = NULL;
	buttonPushed = false;
	isShuttingDown = false;
	launchGameNext = FALSE;

	//initialize the gadgets
	EnableSlotListUpdates(FALSE);
	InitWOLGameGadgets();
	EnableSlotListUpdates(TRUE);
	TheGameSpyInfo->registerTextWindow(listboxGameSetupChat);

	//The dialog needs to react differently depending on whether it's the host or not.
	TheMapCache->updateCache();
	GameSpyStagingRoom *game = TheGameSpyInfo->getCurrentStagingRoom();
	GameSpyGameSlot *hostSlot = game->getGameSpySlot(0);
	hostSlot->setAccept();
	if (TheGameSpyInfo->amIHost())
	{
		OptionPreferences natPref;
		CustomMatchPreferences customPref;
		hostSlot->setColor( customPref.getPreferredColor() );
		hostSlot->setPlayerTemplate( customPref.getPreferredFaction() );
		hostSlot->setNATBehavior((FirewallHelperClass::FirewallBehaviorType)natPref.getFirewallBehavior());
		hostSlot->setPingString(TheGameSpyInfo->getPingString());
		game->setMap(customPref.getPreferredMap());

		for (Int i=1; i<MAX_SLOTS; ++i)
		{
			GameSpyGameSlot *slot = game->getGameSpySlot(i);
			slot->setState( SLOT_OPEN );
		}

		AsciiString lowerMap = customPref.getPreferredMap();
		lowerMap.toLower();
		std::map<AsciiString, MapMetaData>::iterator it = TheMapCache->find(lowerMap);
		if (it != TheMapCache->end())
		{
			hostSlot->setMapAvailability(TRUE);
			game->setMapCRC( it->second.m_CRC );
			game->setMapSize( it->second.m_filesize );

			game->adjustSlotsForMap(); // BGC- adjust the slots for the new map.
		}


		WOLDisplaySlotList();
		WOLDisplayGameOptions();

		// TheSuperHackers @tweak disable the combo box for the host's player name
		comboBoxPlayer[0]->winEnable(FALSE);
	}
	else
	{
		OptionPreferences natPref;
		CustomMatchPreferences customPref;
		AsciiString options;
		PeerRequest req;
		UnicodeString uName = hostSlot->getName();
		AsciiString aName;
		aName.translate(uName);
		req.peerRequestType = PeerRequest::PEERREQUEST_UTMPLAYER;
		req.UTM.isStagingRoom = TRUE;
		req.id = "REQ/";
		req.nick = aName.str();
		options.format("PlayerTemplate=%d", customPref.getPreferredFaction());
		req.options = options.str();
		TheGameSpyPeerMessageQueue->addRequest(req);
		options.format("Color=%d", customPref.getPreferredColor());
		req.options = options.str();
		TheGameSpyPeerMessageQueue->addRequest(req);
		options.format("NAT=%d", natPref.getFirewallBehavior());
		req.options = options.str();
		TheGameSpyPeerMessageQueue->addRequest(req);
		options.format("Ping=%s", TheGameSpyInfo->getPingString().str());
		req.options = options.str();
		TheGameSpyPeerMessageQueue->addRequest(req);

		game->setMapCRC( game->getMapCRC() );		// force a recheck
		game->setMapSize( game->getMapSize() ); // of if we have the map

		for (Int i = 0; i < MAX_SLOTS; ++i)
		{
			//I'm a client, disable the controls I can't touch.
			comboBoxPlayer[i]->winEnable(FALSE);

			comboBoxColor[i]->winEnable(FALSE);
			comboBoxPlayerTemplate[i]->winEnable(FALSE);
			comboBoxTeam[i]->winEnable(FALSE);
//			buttonStartPosition[i]->winEnable(FALSE);
			buttonMapStartPosition[i]->winEnable(FALSE);

		}
		buttonStart->winSetText(TheGameText->fetch("GUI:Accept"));
		buttonStart->winEnable( FALSE );
		buttonSelectMap->winEnable( FALSE );
		initialAcceptEnable = FALSE;
	}

	// Show the Menu
	layout->hide( FALSE );

	// Make sure the text fields are clear
	GadgetListBoxReset( listboxGameSetupChat );
	GadgetTextEntrySetText(textEntryChat, UnicodeString::TheEmptyString);

	initDone = true;
	TheGameSpyInfo->setGameOptions();
	//TheShell->registerWithAnimateManager(parentWOLGameSetup, WIN_ANIMATION_SLIDE_TOP, TRUE);
	WOLPositionStartSpots();

	lastSlotlistTime = 0;
	enterTime = timeGetTime();

	// Set Keyboard to chat entry
	TheWindowManager->winSetFocus( textEntryChat );
	raiseMessageBoxes = true;
	TheTransitionHandler->setGroup("GameSpyGameOptionsMenuFade");
}// void WOLGameSetupMenuInit( WindowLayout *layout, void *userData )

//-------------------------------------------------------------------------------------------------
/** This is called when a shutdown is complete for this menu */
//-------------------------------------------------------------------------------------------------
static void shutdownComplete( WindowLayout *layout )
{

	isShuttingDown = false;

	// hide the layout
	layout->hide( TRUE );

	// our shutdown is complete
	TheShell->shutdownComplete( layout, (nextScreen != NULL) );

	if (nextScreen != NULL)
	{
		if (!TheGameSpyPeerMessageQueue || !TheGameSpyPeerMessageQueue->isConnected())
		{
			DEBUG_LOG(("GameSetup shutdownComplete() - skipping push because we're disconnected"));
		}
		else
		{
			TheShell->push(nextScreen);
		}
	}

	/*
	if (launchGameNext)
	{
		TheGameSpyGame->launchGame();
		TheGameSpyInfo->leaveStagingRoom();
	}
	*/

	nextScreen = NULL;

}  // end if

//-------------------------------------------------------------------------------------------------
/** GameSpy Game Options menu shutdown method */
//-------------------------------------------------------------------------------------------------
void WOLGameSetupMenuShutdown( WindowLayout *layout, void *userData )
{
	TheGameSpyInfo->unregisterTextWindow(listboxGameSetupChat);

	if( WOLMapSelectLayout )
	{
		WOLMapSelectLayout->destroyWindows();
		deleteInstance(WOLMapSelectLayout);
		WOLMapSelectLayout = NULL;
	}
	parentWOLGameSetup = NULL;
	EnableSlotListUpdates(FALSE);
	DeinitWOLGameGadgets();
	if (TheEstablishConnectionsMenu != NULL)
	{
		TheEstablishConnectionsMenu->endMenu();
	}
	initDone = false;

	isShuttingDown = true;

	// if we are shutting down for an immediate pop, skip the animations
	Bool popImmediate = *(Bool *)userData;
	if( popImmediate )
	{

		shutdownComplete( layout );
		return;

	}  //end if

	TheShell->reverseAnimatewindow();

	RaiseGSMessageBox();
	TheTransitionHandler->reverse("GameSpyGameOptionsMenuFade");
}  // void WOLGameSetupMenuShutdown( WindowLayout *layout, void *userData )

static void fillPlayerInfo(const PeerResponse *resp, PlayerInfo *info)
{
	info->m_name			= resp->nick.c_str();
	info->m_profileID	= resp->player.profileID;
	info->m_flags			= resp->player.flags;
	info->m_wins			= resp->player.wins;
	info->m_losses		= resp->player.losses;
	info->m_locale		= resp->locale.c_str();
	info->m_rankPoints= resp->player.rankPoints;
	info->m_side			= resp->player.side;
	info->m_preorder	= resp->player.preorder;
}

//-------------------------------------------------------------------------------------------------
/** Lan Game Options menu update method */
//-------------------------------------------------------------------------------------------------
void WOLGameSetupMenuUpdate( WindowLayout * layout, void *userData)
{
	// We'll only be successful if we've requested to
	if(isShuttingDown && TheShell->isAnimFinished() && TheTransitionHandler->isFinished())
	{
		shutdownComplete(layout);
		return;
	}

	if (raiseMessageBoxes)
	{
		RaiseGSMessageBox();
		raiseMessageBoxes = false;
	}

	if (TheShell->isAnimFinished() && !buttonPushed && TheGameSpyPeerMessageQueue)
	{
		HandleBuddyResponses();
		HandlePersistentStorageResponses();

		if (TheGameSpyGame && TheGameSpyGame->isGameInProgress())
		{
			if (TheGameSpyInfo->isDisconnectedAfterGameStart(NULL))
			{
				return; // already been disconnected, so don't worry.
			}

			Int allowedMessages = TheGameSpyInfo->getMaxMessagesPerUpdate();
			Bool sawImportantMessage = FALSE;
			PeerResponse resp;
			while (allowedMessages-- && !sawImportantMessage && TheGameSpyPeerMessageQueue->getResponse( resp ))
			{
				switch (resp.peerResponseType)
				{
				case PeerResponse::PEERRESPONSE_DISCONNECT:
					{
						sawImportantMessage = TRUE;
						AsciiString disconMunkee;
						disconMunkee.format("GUI:GSDisconReason%d", resp.discon.reason);

						// check for scorescreen
						NameKeyType listboxChatWindowScoreScreenID = NAMEKEY("ScoreScreen.wnd:ListboxChatWindowScoreScreen");
						GameWindow *listboxChatWindowScoreScreen = TheWindowManager->winGetWindowFromId( NULL, listboxChatWindowScoreScreenID );
						if (listboxChatWindowScoreScreen)
						{
							GadgetListBoxAddEntryText(listboxChatWindowScoreScreen, TheGameText->fetch(disconMunkee),
								GameSpyColor[GSCOLOR_DEFAULT], -1);
						}
						else
						{
							// still ingame
							TheInGameUI->message(disconMunkee);
						}
						TheGameSpyInfo->markAsDisconnectedAfterGameStart(resp.discon.reason);
					}
				}
			}

			return; // if we're in game, all we care about is if we've been disconnected from the chat server
		}

		Bool isHosting = TheGameSpyInfo->amIHost(); // only while in game setup screen
		isHosting = isHosting || (TheGameSpyGame && TheGameSpyGame->isInGame() && TheGameSpyGame->amIHost()); // while in game
		if (!isHosting && !lastSlotlistTime && timeGetTime() > enterTime + 10000)
		{
			// don't do this if we're disconnected
			if (TheGameSpyPeerMessageQueue->isConnected())
			{
				// haven't seen ourselves
				buttonPushed = true;
				DEBUG_LOG(("Haven't seen ourselves in slotlist"));
				if (TheGameSpyGame)
					TheGameSpyGame->reset();
				TheGameSpyInfo->leaveStagingRoom();
				//TheGameSpyInfo->joinBestGroupRoom();
				GSMessageBoxOk(TheGameText->fetch("GUI:HostLeftTitle"), TheGameText->fetch("GUI:HostLeft"));
				nextScreen = "Menus/WOLCustomLobby.wnd";
				TheShell->pop();
			}
			return;
		}

		if (TheNAT != NULL) {
			NATStateType NATState = TheNAT->update();
			if (NATState == NATSTATE_DONE)
			{
				//launchGameNext = TRUE;
				//TheShell->pop();
				TheGameSpyGame->launchGame();
				if (TheGameSpyInfo) // this can be blown away by a disconnect on the map transfer screen
					TheGameSpyInfo->leaveStagingRoom();
				return;
			}
			else if (NATState == NATSTATE_FAILED)
			{
				// Just back out.  This cleans up some slot list problems
				buttonPushed = true;

				// delete TheNAT, its no good for us anymore.
				delete TheNAT;
				TheNAT = NULL;

				TheGameSpyInfo->getCurrentStagingRoom()->reset();
				TheGameSpyInfo->leaveStagingRoom();
				//TheGameSpyInfo->joinBestGroupRoom();
				GSMessageBoxOk(TheGameText->fetch("GUI:Error"), TheGameText->fetch("GUI:NATNegotiationFailed"));
				nextScreen = "Menus/WOLCustomLobby.wnd";
				TheShell->pop();
				return;
			}
		}

		PeerResponse resp;

		Int allowedMessages = TheGameSpyInfo->getMaxMessagesPerUpdate();
		Bool sawImportantMessage = FALSE;
		while (allowedMessages-- && !sawImportantMessage)
		{

			if (!TheLobbyQueuedUTMs.empty())
			{
				DEBUG_LOG(("Got response from queued lobby UTM list"));
				resp = TheLobbyQueuedUTMs.front();
				TheLobbyQueuedUTMs.pop_front();
			}
			else if (TheGameSpyPeerMessageQueue->getResponse( resp ))
			{
				DEBUG_LOG(("Got response from message queue"));
			}
			else
			{
				break;
			}

			switch (resp.peerResponseType)
			{
			case PeerResponse::PEERRESPONSE_FAILEDTOHOST:
				{
					// oops - we've not heard from the qr server.  bail.
					TheGameSpyInfo->addText(TheGameText->fetch("GUI:GSFailedToHost"), GameSpyColor[GSCOLOR_DEFAULT], NULL);
				}
				break;
			case PeerResponse::PEERRESPONSE_GAMESTART:
				{
					sawImportantMessage = TRUE;
					GameSpyStagingRoom *myGame = TheGameSpyInfo->getCurrentStagingRoom();
					if (!myGame || !myGame->isInGame())
						break;

					if (!TheGameSpyGame)
						break;

					SendStatsToOtherPlayers(TheGameSpyGame);

					// we've started, there's no going back
					// i.e. disable the back button.
					buttonBack->winEnable(FALSE);
					GameWindow *buttonBuddy = TheWindowManager->winGetWindowFromId(NULL, NAMEKEY("GameSpyGameOptionsMenu.wnd:ButtonCommunicator"));
					if (buttonBuddy)
						buttonBuddy->winEnable(FALSE);
					GameSpyCloseOverlay(GSOVERLAY_BUDDY);

					*TheGameSpyGame = *myGame;
					TheGameSpyGame->startGame(0);
				}
				break;
			case PeerResponse::PEERRESPONSE_PLAYERCHANGEDFLAGS:
				{
					PlayerInfo p;
					fillPlayerInfo(&resp, &p);
					TheGameSpyInfo->updatePlayerInfo(p);
					WOLDisplaySlotList();
				}
				break;
			case PeerResponse::PEERRESPONSE_PLAYERINFO:
				{
					PlayerInfo p;
					fillPlayerInfo(&resp, &p);
					TheGameSpyInfo->updatePlayerInfo(p);
					WOLDisplaySlotList();
					// send out new slotlist if I'm host
					TheGameSpyInfo->setGameOptions();
				}
				break;
			case PeerResponse::PEERRESPONSE_PLAYERJOIN:
				{
					if (resp.player.roomType != StagingRoom)
					{
						break;
					}
					sawImportantMessage = TRUE;
					PlayerInfo p;
					fillPlayerInfo(&resp, &p);
					TheGameSpyInfo->updatePlayerInfo(p);

					if (p.m_profileID)
					{
						if (TheGameSpyPSMessageQueue->findPlayerStatsByID(p.m_profileID).id == 0)
						{
							PSRequest req;
							req.requestType = PSRequest::PSREQUEST_READPLAYERSTATS;
							req.player.id = p.m_profileID;
							TheGameSpyPSMessageQueue->addRequest(req);
						}
					}

					// check if we have room for the dude
					GameInfo *game = TheGameSpyInfo->getCurrentStagingRoom();
					if (TheGameSpyInfo->amIHost() && game)
					{
						if (TheNAT)
						{
							// ditch him
							PeerRequest req;
							req.peerRequestType = PeerRequest::PEERREQUEST_UTMPLAYER;
							req.UTM.isStagingRoom = TRUE;
							req.id = "KICK/";
							req.nick = p.m_name.str();
							req.options = "GameStarted";
							TheGameSpyPeerMessageQueue->addRequest(req);
						}
						else
						{
							// look for room for him
							// See if there's room
							// First get the number of players currently in the room.
							Int numPlayers = 0;
							for (Int player = 0; player < MAX_SLOTS; ++player)
							{
								if (game->getSlot(player)->isOccupied() &&
									game->getSlot(player)->getPlayerTemplate() != PLAYERTEMPLATE_OBSERVER)
								{
									++numPlayers;
								}
							}

							// now get the number of starting spots on the map.
							Int numStartingSpots = MAX_SLOTS;
							const MapMetaData *md = TheMapCache->findMap(game->getMap());
							if (md != NULL)
							{
								numStartingSpots = md->m_numPlayers;
							}

							Int openSlotIndex = -1;
							for (Int i=0; i<MAX_SLOTS; ++i)
							{
								const GameSlot *slot = game->getConstSlot(i);
								if (slot && slot->isOpen())
								{
									openSlotIndex = i;
									break;
								}
							}

							if (openSlotIndex >= 0)
							{
								// add him
								GameSlot newSlot;
								UnicodeString uName;
								uName.translate(p.m_name);
								newSlot.setState(SLOT_PLAYER, uName);
								newSlot.setIP(ntohl(resp.player.IP));
								game->setSlot( openSlotIndex, newSlot );
								game->resetAccepted(); // BGC - need to unaccept everyone if someone joins the game.
							}
							else
							{
								// ditch him
								PeerRequest req;
								req.peerRequestType = PeerRequest::PEERREQUEST_UTMPLAYER;
								req.UTM.isStagingRoom = TRUE;
								req.id = "KICK/";
								req.nick = p.m_name.str();
								req.options = "GameFull";
								TheGameSpyPeerMessageQueue->addRequest(req);
							}

							// send out new slotlist if I'm host
							TheGameSpyInfo->setGameOptions();
						}
					}
					WOLDisplaySlotList();
				}
				break;

			case PeerResponse::PEERRESPONSE_PLAYERLEFT:
				{
					sawImportantMessage = TRUE;
					PlayerInfo p;
					fillPlayerInfo(&resp, &p);
					TheGameSpyInfo->playerLeftGroupRoom(resp.nick.c_str());

					if (TheGameSpyGame && TheGameSpyGame->isGameInProgress())
					{
						break;
					}

					if (TheNAT == NULL) // don't update slot list if we're trying to start a game
					{

						GameInfo *game = TheGameSpyInfo->getCurrentStagingRoom();
						if (game && TheGameSpyInfo->amIHost())
						{
							Int idx = game->getSlotNum(resp.nick.c_str());
							if (idx >= 0)
							{
								game->getSlot(idx)->setState(SLOT_OPEN);
								game->resetAccepted(); // BGC - need to unaccept everyone if someone leaves the game.
							}
						}

						// send out new slotlist if I'm host
						TheGameSpyInfo->setGameOptions();
						WOLDisplaySlotList();

						if (game && !TheGameSpyInfo->amIHost())
						{
							Int idx = game->getSlotNum(resp.nick.c_str());
							if (idx == 0)
							{
								// host left
								buttonPushed = true;
								TheGameSpyInfo->getCurrentStagingRoom()->reset();
								TheGameSpyInfo->leaveStagingRoom();
								//TheGameSpyInfo->joinBestGroupRoom();
								GSMessageBoxOk(TheGameText->fetch("GUI:HostLeftTitle"), TheGameText->fetch("GUI:HostLeft"));
								nextScreen = "Menus/WOLCustomLobby.wnd";
								TheShell->pop();
							}
						}

					}
				}
				break;

			case PeerResponse::PEERRESPONSE_MESSAGE:
				{
					TheGameSpyInfo->addChat(resp.nick.c_str(), resp.message.profileID,
						UnicodeString(resp.text.c_str()), !resp.message.isPrivate, resp.message.isAction, listboxGameSetupChat);
				}
				break;

			case PeerResponse::PEERRESPONSE_DISCONNECT:
				{
					sawImportantMessage = TRUE;
					UnicodeString title, body;
					AsciiString disconMunkee;
					disconMunkee.format("GUI:GSDisconReason%d", resp.discon.reason);
					title = TheGameText->fetch( "GUI:GSErrorTitle" );
					body = TheGameText->fetch( disconMunkee );
					GameSpyCloseAllOverlays();
					GSMessageBoxOk( title, body );
					TheGameSpyInfo->reset();
					TheShell->pop();
				}
				break;

			case PeerResponse::PEERRESPONSE_ROOMUTM:
				{
					sawImportantMessage = TRUE;
#if defined(RTS_DEBUG)
					if (g_debugSlots)
					{
						DEBUG_LOG(("About to process a room UTM.  Command is '%s', command options is '%s'",
							resp.command.c_str(), resp.commandOptions.c_str()));
					}
#endif
					if (!strcmp(resp.command.c_str(), "SL"))
					{
						// slotlist
						GameSpyStagingRoom *game = TheGameSpyInfo->getCurrentStagingRoom();
						Bool isValidSlotList = game && game->getSlot(0) && game->getSlot(0)->isPlayer( resp.nick.c_str() ) && !TheGameSpyInfo->amIHost();
						if (!isValidSlotList)
						{
							SLOTLIST_DEBUG_LOG(("Not a valid slotlist"));
							if (!game)
							{
								SLOTLIST_DEBUG_LOG(("No game!"));
							}
							else
							{
								if (!game->getSlot(0))
								{
									SLOTLIST_DEBUG_LOG(("No slot 0!"));
								}
								else
								{
									if (TheGameSpyInfo->amIHost())
									{
										SLOTLIST_DEBUG_LOG(("I'm the host!"));
									}
									else
									{
										SLOTLIST_DEBUG_LOG(("Not from the host!  isHuman:%d, name:'%ls', sender:'%s'",
											game->getSlot(0)->isHuman(), game->getSlot(0)->getName().str(),
											resp.nick.c_str()));
									}
								}
							}
						}
						else // isValidSlotList
						{
							Int oldLocalSlotNum = (game->isInGame()) ? game->getLocalSlotNum() : -1;
							Bool wasInGame = oldLocalSlotNum >= 0;
							AsciiString oldMap = game->getMap();
							UnsignedInt oldMapCRC, newMapCRC;
							oldMapCRC = game->getMapCRC();

							AsciiString options = resp.commandOptions.c_str();
							options.trim();
							UnsignedShort ports[MAX_SLOTS];
							UnsignedInt ips[MAX_SLOTS];
							Int i;
							for (i=0; i<MAX_SLOTS; ++i)
							{
								if (game && game->getConstSlot(i))
								{
									ips[i] = game->getConstSlot(i)->getIP();
									ports[i] = game->getConstSlot(i)->getPort();
								}
								else
								{
									ips[i] = 0;
									ports[i] = 0;
								}
							}
							Bool optionsOK = ParseAsciiStringToGameInfo(game, options.str());
							if (TheNAT)
							{
								for (i=0; i<MAX_SLOTS; ++i)
								{
									if (game && game->getSlot(i))
									{
#ifdef DEBUG_LOGGING
										UnsignedShort newPort = game->getConstSlot(i)->getPort();
										UnsignedInt newIP = game->getConstSlot(i)->getIP();
										DEBUG_ASSERTLOG(newIP == ips[i], ("IP was different for player %d (%X --> %X)",
											i, ips[i], newIP));
										DEBUG_ASSERTLOG(newPort == ports[i], ("Port was different for player %d (%d --> %d)",
											i, ports[i], newPort));
#endif
										game->getSlot(i)->setPort(ports[i]);
										game->getSlot(i)->setIP(ips[i]);
									}
								}
							}
							Int newLocalSlotNum = (game->isInGame()) ? game->getLocalSlotNum() : -1;
							Bool isInGame = newLocalSlotNum >= 0;
							if (!optionsOK)
							{
								SLOTLIST_DEBUG_LOG(("Options are bad!  bailing!"));
								break;
							}
							else
							{
								SLOTLIST_DEBUG_LOG(("Options are good, local slot is %d", newLocalSlotNum));
								if (!isInGame)
								{
									SLOTLIST_DEBUG_LOG(("Not in game; players are:"));
									for (Int i=0; i<MAX_SLOTS; ++i)
									{
										const GameSpyGameSlot *slot = game->getGameSpySlot(i);
										if (slot && slot->isHuman())
										{
											UnicodeString munkee;
											munkee.format(L"\t%d: %ls", i, slot->getName().str());
											SLOTLIST_DEBUG_LOG(("%ls", munkee.str()));
										}
									}
								}
							}
							WOLDisplaySlotList();

							// if I changed map availability, send it across
							newMapCRC = game->getMapCRC();
							if (isInGame)
							{
								lastSlotlistTime = timeGetTime();
								if ( (oldMapCRC ^ newMapCRC) || (!wasInGame && isInGame) )
								{
									// it changed.  send it
									UnicodeString hostName = TheGameSpyInfo->getCurrentStagingRoom()->getSlot(0)->getName();
									AsciiString asciiName;
									asciiName.translate(hostName);
									PeerRequest req;
									req.peerRequestType = PeerRequest::PEERREQUEST_UTMPLAYER;
									req.UTM.isStagingRoom = TRUE;
									req.id = "MAP";
									req.nick = asciiName.str();
									req.options = (game->getSlot(newLocalSlotNum)->hasMap())?"1":"0";
									TheGameSpyPeerMessageQueue->addRequest(req);
									if (!game->getSlot(newLocalSlotNum)->hasMap())
									{
										UnicodeString text;
										UnicodeString mapDisplayName;
										const MapMetaData *mapData = TheMapCache->findMap( game->getMap() );
										Bool willTransfer = TRUE;
										if (mapData)
										{
											mapDisplayName.format(L"%ls", mapData->m_displayName.str());
											willTransfer = !mapData->m_isOfficial;
										}
										else
										{
											mapDisplayName.format(L"%hs", TheGameState->getMapLeafName(game->getMap()).str());
											willTransfer = WouldMapTransfer(game->getMap());
										}
										if (willTransfer)
											text.format(TheGameText->fetch("GUI:LocalPlayerNoMapWillTransfer"), mapDisplayName.str());
										else
											text.format(TheGameText->fetch("GUI:LocalPlayerNoMap"), mapDisplayName.str());
										TheGameSpyInfo->addText(text, GameSpyColor[GSCOLOR_DEFAULT], listboxGameSetupChat);
									}
								}
								if (!initialAcceptEnable)
								{
									buttonStart->winEnable( TRUE );
									initialAcceptEnable = TRUE;
								}
							}
							else
							{
								if (lastSlotlistTime)
								{
									// can't see ourselves
									buttonPushed = true;
									DEBUG_LOG(("Can't see ourselves in slotlist %s", options.str()));
									TheGameSpyInfo->getCurrentStagingRoom()->reset();
									TheGameSpyInfo->leaveStagingRoom();
									//TheGameSpyInfo->joinBestGroupRoom();
									GSMessageBoxOk(TheGameText->fetch("GUI:GSErrorTitle"), TheGameText->fetch("GUI:GSKicked"));
									nextScreen = "Menus/WOLCustomLobby.wnd";
									TheShell->pop();
								}
							}
						}
					}
					else if (!strcmp(resp.command.c_str(), "HWS"))
					{
						// host wants to start
						GameInfo *game = TheGameSpyInfo->getCurrentStagingRoom();
						if (game && game->isInGame() && game->getSlot(0) && game->getSlot(0)->isPlayer( resp.nick.c_str() ))
						{
							Int slotNum = game->getLocalSlotNum();
							GameSlot *slot = game->getSlot(slotNum);
							if (slot && (slot->isAccepted() == false))
							{
								TheGameSpyInfo->addText(TheGameText->fetch("GUI:HostWantsToStart"), GameSpyColor[GSCOLOR_DEFAULT], listboxGameSetupChat);
							}
						}
					}
					else if (!stricmp(resp.command.c_str(), "NAT"))
					{
						if (TheNAT != NULL) {
							TheNAT->processGlobalMessage(-1, resp.commandOptions.c_str());
						}
					}
					else if (!stricmp(resp.command.c_str(), "Pings"))
					{
						if (!TheGameSpyInfo->amIHost())
						{
							AsciiString pings = resp.commandOptions.c_str();
							AsciiString token;
							for (Int i=0; i<MAX_SLOTS; ++i)
							{
								GameSpyGameSlot *slot = TheGameSpyInfo->getCurrentStagingRoom()->getGameSpySlot(i);
								if (pings.nextToken(&token, ","))
								{
									token.trim();
									slot->setPingString(token);
								}
								else
								{
									slot->setPingString("");
								}
							}
						}
					}
				}
				break;

			case PeerResponse::PEERRESPONSE_PLAYERUTM:
				{
					sawImportantMessage = TRUE;
					if (!strcmp(resp.command.c_str(), "STATS"))
					{
						PSPlayerStats stats = TheGameSpyPSMessageQueue->parsePlayerKVPairs(resp.commandOptions.c_str());
						if (stats.id && (TheGameSpyPSMessageQueue->findPlayerStatsByID(stats.id).id == 0))
							TheGameSpyPSMessageQueue->trackPlayerStats(stats);
						break;
					}
					GameSpyStagingRoom *game = TheGameSpyInfo->getCurrentStagingRoom();
					if (game)
					{
						Int slotNum = game->getSlotNum(resp.nick.c_str());
						if ((slotNum >= 0) && (slotNum < MAX_SLOTS) && (!stricmp(resp.command.c_str(), "NAT"))) {
							// this is a command for NAT negotiations, pass if off to TheNAT
							if (TheNAT != NULL) {
								TheNAT->processGlobalMessage(slotNum, resp.commandOptions.c_str());
							}
						}
						if (slotNum == 0 && !TheGameSpyInfo->amIHost())
						{
							if (!strcmp(resp.command.c_str(), "KICK"))
							{
								// oops - we've been kicked.  bail.
								buttonPushed = true;
								TheGameSpyInfo->getCurrentStagingRoom()->reset();
								TheGameSpyInfo->leaveStagingRoom();
								//TheGameSpyInfo->joinBestGroupRoom();
								UnicodeString message = TheGameText->fetch("GUI:GSKicked");
								AsciiString commandMessage = resp.commandOptions.c_str();
								commandMessage.trim();
								DEBUG_LOG(("We were kicked: reason was '%s'", resp.commandOptions.c_str()));
								if (commandMessage == "GameStarted")
								{
									message = TheGameText->fetch("GUI:GSKickedGameStarted");
								}
								else if (commandMessage == "GameFull")
								{
									message = TheGameText->fetch("GUI:GSKickedGameFull");
								}
								GSMessageBoxOk(TheGameText->fetch("GUI:GSErrorTitle"), message);
								nextScreen = "Menus/WOLCustomLobby.wnd";
								TheShell->pop();
							}
						}
						else if (slotNum > 0 && TheGameSpyInfo->amIHost())
						{
							if (!strcmp(resp.command.c_str(), "accept"))
							{
								game->getSlot(slotNum)->setAccept();
								TheGameSpyInfo->setGameOptions();
								WOLDisplaySlotList();
							}
							else if (!strcmp(resp.command.c_str(), "MAP"))
							{
								Bool hasMap = atoi(resp.commandOptions.c_str());
								game->getSlot(slotNum)->setMapAvailability(hasMap);
								if (!hasMap)
								{
									// tell the host the user doesn't have the map
									UnicodeString mapDisplayName;
									const MapMetaData *mapData = TheMapCache->findMap( game->getMap() );
									Bool willTransfer = TRUE;
									if (mapData)
									{
										mapDisplayName.format(L"%ls", mapData->m_displayName.str());
										willTransfer = !mapData->m_isOfficial;
									}
									else
									{
										mapDisplayName.format(L"%hs", game->getMap().str());
										willTransfer = WouldMapTransfer(game->getMap());
									}
									UnicodeString text;
									if (willTransfer)
										text.format(TheGameText->fetch("GUI:PlayerNoMapWillTransfer"), game->getSlot(slotNum)->getName().str(), mapDisplayName.str());
									else
										text.format(TheGameText->fetch("GUI:PlayerNoMap"), game->getSlot(slotNum)->getName().str(), mapDisplayName.str());
									TheGameSpyInfo->addText(text, GameSpyColor[GSCOLOR_DEFAULT], listboxGameSetupChat);
								}
								WOLDisplaySlotList();
							}
							else if (!strcmp(resp.command.c_str(), "REQ"))
							{
								AsciiString options = resp.commandOptions.c_str();
								options.trim();

								Bool change = false;
								Bool shouldUnaccept = false;
								AsciiString key;
								options.nextToken(&key, "=");
								Int val = atoi(options.str()+1);
								UnsignedInt uVal = atoi(options.str()+1);
								DEBUG_LOG(("GameOpt request: key=%s, val=%s from player %d", key.str(), options.str()+1, slotNum));

								GameSpyGameSlot *slot = game->getGameSpySlot(slotNum);
								if (!slot)
									break;

								if (key == "Color")
								{
									if (val >= -1 && val < TheMultiplayerSettings->getNumColors() && val != slot->getColor() && slot->getPlayerTemplate() != PLAYERTEMPLATE_OBSERVER)
									{
										Bool colorAvailable = TRUE;
										if(val != -1 )
										{
											for(Int i=0; i <MAX_SLOTS; i++)
											{
												GameSlot *checkSlot = game->getSlot(i);
												if(val == checkSlot->getColor() && slot != checkSlot)
												{
													colorAvailable = FALSE;
													break;
												}
											}
										}
										if(colorAvailable)
											slot->setColor(val);
										change = true;
									}
									else
									{
										DEBUG_LOG(("Rejecting invalid color %d", val));
									}
								}
								else if (key == "PlayerTemplate")
								{
									if (val >= PLAYERTEMPLATE_MIN && val < ThePlayerTemplateStore->getPlayerTemplateCount() && val != slot->getPlayerTemplate())
									{
										slot->setPlayerTemplate(val);
										if (val == PLAYERTEMPLATE_OBSERVER)
										{
											slot->setColor(-1);
											slot->setStartPos(-1);
											slot->setTeamNumber(-1);
										}
										change = true;
										shouldUnaccept = true;
									}
									else
									{
										DEBUG_LOG(("Rejecting invalid PlayerTemplate %d", val));
									}
								}
								else if (key == "StartPos")
								{
									if (val >= -1 && val < MAX_SLOTS && val != slot->getStartPos() && slot->getPlayerTemplate() != PLAYERTEMPLATE_OBSERVER)
									{
										Bool startPosAvailable = TRUE;
										if(val != -1)
										{
											for(Int i=0; i <MAX_SLOTS; i++)
											{
												GameSlot *checkSlot = game->getSlot(i);
												if(val == checkSlot->getStartPos() && slot != checkSlot)
												{
													startPosAvailable = FALSE;
													break;
												}
											}
										}
										if(startPosAvailable)
											slot->setStartPos(val);
										change = true;
										shouldUnaccept = true;
									}
									else
									{
										DEBUG_LOG(("Rejecting invalid startPos %d", val));
									}
								}
								else if (key == "Team")
								{
									if (val >= -1 && val < MAX_SLOTS/2 && val != slot->getTeamNumber() && slot->getPlayerTemplate() != PLAYERTEMPLATE_OBSERVER)
									{
										slot->setTeamNumber(val);
										change = true;
										shouldUnaccept = true;
									}
									else
									{
										DEBUG_LOG(("Rejecting invalid team %d", val));
									}
								}
								else if (key == "IP")
								{
									if (uVal != slot->getIP())
									{
										DEBUG_LOG(("setting IP of player %ls from 0x%08x to be 0x%08x", slot->getName().str(), slot->getIP(), uVal));
										slot->setIP(uVal);
										change = true;
										shouldUnaccept = true;
									}
									else
									{
										DEBUG_LOG(("Rejecting invalid IP %d", uVal));
									}
								}
								else if (key == "NAT")
								{
									if ((val >= FirewallHelperClass::FIREWALL_MIN) &&
											(val <= FirewallHelperClass::FIREWALL_MAX))
									{
										slot->setNATBehavior((FirewallHelperClass::FirewallBehaviorType)val);
										DEBUG_LOG(("Setting NAT behavior to %d for player %d", val, slotNum));
										change = true;
									}
									else
									{
										DEBUG_LOG(("Rejecting invalid NAT behavior %d from player %d", val, slotNum));
									}
								}
								else if (key == "Ping")
								{
									slot->setPingString(options.str()+1);
									TheGameSpyInfo->setGameOptions();
									DEBUG_LOG(("Setting ping string to %s for player %d", options.str()+1, slotNum));
								}

								if (change)
								{
									if (shouldUnaccept)
										game->resetAccepted();

									TheGameSpyInfo->setGameOptions();

									WOLDisplaySlotList();
									DEBUG_LOG(("Slot value is color=%d, PlayerTemplate=%d, startPos=%d, team=%d, IP=0x%8.8X",
										slot->getColor(), slot->getPlayerTemplate(), slot->getStartPos(), slot->getTeamNumber(), slot->getIP()));
									DEBUG_LOG(("Slot list updated to %s", GameInfoToAsciiString(game).str()));
								}
							}
						}
					}
				}
				break;

			}
		}


	}
}// void WOLGameSetupMenuUpdate( WindowLayout * layout, void *userData)

//-------------------------------------------------------------------------------------------------
/** Lan Game Options menu input callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType WOLGameSetupMenuInput( GameWindow *window, UnsignedInt msg,
																			 WindowMsgData mData1, WindowMsgData mData2 )
{
	/*
	switch( msg )
	{

		//-------------------------------------------------------------------------------------------------
		case GWM_RIGHT_UP:
		{
			if (buttonPushed)
				break;

			GameWindow *control = (GameWindow *)mData1;
			NameKeyType controlID = (NameKeyType)control->winGetWindowId();
			DEBUG_LOG(("GWM_RIGHT_UP for control %d(%s)", controlID, TheNameKeyGenerator->keyToName(controlID).str()));
			break;
		}

		// --------------------------------------------------------------------------------------------
		case GWM_CHAR:
		{
			UnsignedByte key = mData1;
			UnsignedByte state = mData2;
			if (buttonPushed)
				break;

			switch( key )
			{
				// ----------------------------------------------------------------------------------------
				case KEY_ESC:
				{
					//
					// send a simulated selected event to the parent window of the
					// back/exit button
					//
					if( BitIsSet( state, KEY_STATE_UP ) )
					{
						TheWindowManager->winSendSystemMsg( window, GBM_SELECTED,
																							(WindowMsgData)buttonBack, buttonBackID );
					}  // end if
					// don't let key fall through anywhere else
					return MSG_HANDLED;
				}  // end escape
			}  // end switch( key )
		}  // end char
	}  // end switch( msg )
	*/
	return MSG_IGNORED;
}//WindowMsgHandledType WOLGameSetupMenuInput( GameWindow *window, UnsignedInt msg,


// Slash commands -------------------------------------------------------------------------
extern "C" {
int getQR2HostingStatus(void);
}
extern int isThreadHosting;

Bool handleGameSetupSlashCommands(UnicodeString uText)
{
	AsciiString message;
	message.translate(uText);

	if (message.getCharAt(0) != '/')
	{
		return FALSE; // not a slash command
	}

	AsciiString remainder = message.str() + 1;
	AsciiString token;
	remainder.nextToken(&token);
	token.toLower();

	if (token == "host")
	{
		UnicodeString s;
		s.format(L"Hosting qr2:%d thread:%d", getQR2HostingStatus(), isThreadHosting);
		TheGameSpyInfo->addText(s, GameSpyColor[GSCOLOR_DEFAULT], NULL);
		return TRUE; // was a slash command
	}
	else if (token == "me" && uText.getLength()>4)
	{
		TheGameSpyInfo->sendChat(UnicodeString(uText.str()+4), TRUE, NULL);
		return TRUE; // was a slash command
	}
#if defined(RTS_DEBUG)
	else if (token == "slots")
	{
		g_debugSlots = !g_debugSlots;
		TheGameSpyInfo->addText(UnicodeString(L"Toggled SlotList debug"), GameSpyColor[GSCOLOR_DEFAULT], NULL);
		return TRUE; // was a slash command
	}
	else if (token == "discon")
	{
		PeerRequest req;
		req.peerRequestType = PeerRequest::PEERREQUEST_LOGOUT;
		TheGameSpyPeerMessageQueue->addRequest( req );
		return TRUE;
	}
#endif // defined(RTS_DEBUG)

	return FALSE; // not a slash command
}

static Int getNextSelectablePlayer(Int start)
{
	GameSpyStagingRoom *game = TheGameSpyInfo->getCurrentStagingRoom();
	if (!game->amIHost())
		return -1;
	for (Int j=start; j<MAX_SLOTS; ++j)
	{
		GameSpyGameSlot *slot = game->getGameSpySlot(j);
		if (slot && slot->getStartPos() == -1 &&
			( (j==game->getLocalSlotNum() && game->getConstSlot(j)->getPlayerTemplate()!=PLAYERTEMPLATE_OBSERVER)
			|| slot->isAI()))
		{
			return j;
		}
	}
	return -1;
}

static Int getFirstSelectablePlayer(const GameInfo *game)
{
	const GameSlot *slot = game->getConstSlot(game->getLocalSlotNum());
	if (!game->amIHost() || slot && slot->getPlayerTemplate() != PLAYERTEMPLATE_OBSERVER)
		return game->getLocalSlotNum();

	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		slot = game->getConstSlot(i);
		if (slot && slot->isAI())
			return i;
	}

	return game->getLocalSlotNum();
}

//-------------------------------------------------------------------------------------------------
/** WOL Game Options menu window system callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType WOLGameSetupMenuSystem( GameWindow *window, UnsignedInt msg,
														 WindowMsgData mData1, WindowMsgData mData2 )
{
	UnicodeString txtInput;
	static Int buttonCommunicatorID = NAMEKEY_INVALID;
	switch( msg )
	{
		//-------------------------------------------------------------------------------------------------
		case GWM_CREATE:
			{
				buttonCommunicatorID = NAMEKEY("GameSpyGameOptionsMenu.wnd:ButtonCommunicator");
				break;
			} // case GWM_DESTROY:
		//-------------------------------------------------------------------------------------------------
		case GWM_DESTROY:
			{
				if (windowMap)
					windowMap->winSetUserData(NULL);

				break;
			} // case GWM_DESTROY:
		//-------------------------------------------------------------------------------------------------
		case GWM_INPUT_FOCUS:
			{
				// if we're givin the opportunity to take the keyboard focus we must say we want it
				if( mData1 == TRUE )
					*(Bool *)mData2 = TRUE;

				return MSG_HANDLED;
			}//case GWM_INPUT_FOCUS:
		//-------------------------------------------------------------------------------------------------
		case GCM_SELECTED:
			{
				if (!initDone)
					break;
				if (buttonPushed)
					break;
				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();
				GameSpyStagingRoom *myGame = TheGameSpyInfo->getCurrentStagingRoom();
				for (Int i = 0; i < MAX_SLOTS; i++)
				{
					if (controlID == comboBoxColorID[i])
					{
						handleColorSelection(i);
					}
					else if (controlID == comboBoxPlayerTemplateID[i])
					{
						handlePlayerTemplateSelection(i);
					}
					else if (controlID == comboBoxTeamID[i])
					{
						handleTeamSelection(i);
					}
					else if( controlID == comboBoxPlayerID[i] && TheGameSpyInfo->amIHost() )
					{
						// We don't have anything that'll happen if we click on ourselves
						if(i == myGame->getLocalSlotNum())
						 break;
						// Get
						Int pos = -1;
						GadgetComboBoxGetSelectedPos(comboBoxPlayer[i], &pos);
						if( pos != SLOT_PLAYER && pos >= 0)
						{
							if( myGame->getSlot(i)->getState() == SLOT_PLAYER )
							{
								PeerRequest req;
								req.peerRequestType = PeerRequest::PEERREQUEST_UTMPLAYER;
								req.UTM.isStagingRoom = TRUE;
								AsciiString aName;
								aName.translate(myGame->getSlot(i)->getName());
								req.nick = aName.str();
								req.id = "KICK/";
								req.options = "true";
								TheGameSpyPeerMessageQueue->addRequest(req);

								UnicodeString name = myGame->getSlot(i)->getName();
								myGame->getSlot(i)->setState(SlotState(pos));
								myGame->resetAccepted();
								TheGameSpyInfo->setGameOptions();
								WOLDisplaySlotList();
								//TheLAN->OnPlayerLeave(name);
							}
							else if( myGame->getSlot(i)->getState() != pos )
							{
								Bool wasAI = (myGame->getSlot(i)->isAI());
								myGame->getSlot(i)->setState(SlotState(pos));
								Bool isAI = (myGame->getSlot(i)->isAI());
								myGame->resetAccepted();
								if (wasAI ^ isAI)
									PopulatePlayerTemplateComboBox(i, comboBoxPlayerTemplate, myGame, wasAI && myGame->getAllowObservers());
								TheGameSpyInfo->setGameOptions();
								WOLDisplaySlotList();
							}
						}
						break;
					}
				}
			}// case GCM_SELECTED:
		//-------------------------------------------------------------------------------------------------
		case GBM_SELECTED:
			{
				if (buttonPushed)
					break;

				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();
				static Int buttonCommunicatorID = NAMEKEY("GameSpyGameOptionsMenu.wnd:ButtonCommunicator");

				if ( controlID == buttonBackID )
				{
					savePlayerInfo();
					if( WOLMapSelectLayout )
					{
						WOLMapSelectLayout->destroyWindows();
						deleteInstance(WOLMapSelectLayout);
						WOLMapSelectLayout = NULL;
					}

					TheGameSpyInfo->getCurrentStagingRoom()->reset();
					//peerLeaveRoom(TheGameSpyChat->getPeer(), StagingRoom, NULL);
					TheGameSpyInfo->leaveStagingRoom();
					buttonPushed = true;
					nextScreen = "Menus/WOLCustomLobby.wnd";
					TheShell->pop();

				} //if ( controlID == buttonBack )
				else if ( controlID == buttonCommunicatorID )
				{
					GameSpyToggleOverlay( GSOVERLAY_BUDDY );

				}
				else if ( controlID == buttonEmoteID )
				{
					// read the user's input
					txtInput.set(GadgetTextEntryGetText( textEntryChat ));
					// Clear the text entry line
					GadgetTextEntrySetText(textEntryChat, UnicodeString::TheEmptyString);
					// Clean up the text (remove leading/trailing chars, etc)
					txtInput.trim();
					// Echo the user's input to the chat window
					if (!txtInput.isEmpty())
						TheGameSpyInfo->sendChat(txtInput, FALSE, NULL); // 'emote' button is now carriage-return
				} //if ( controlID == buttonEmote )
				else if ( controlID == buttonSelectMapID )
				{
					WOLMapSelectLayout = TheWindowManager->winCreateLayout( "Menus/WOLMapSelectMenu.wnd" );
					WOLMapSelectLayout->runInit();
					WOLMapSelectLayout->hide( FALSE );
					WOLMapSelectLayout->bringForward();
				}
				else if ( controlID == buttonStartID )
				{
					savePlayerInfo();
					if (TheGameSpyInfo->amIHost())
					{
						StartPressed();
					}
					else
					{
						//I'm the Client... send an accept message to the host.
						GameSlot *localSlot = TheGameSpyInfo->getCurrentStagingRoom()->getSlot(TheGameSpyInfo->getCurrentStagingRoom()->getLocalSlotNum());
						if (localSlot)
						{
							localSlot->setAccept();
						}
						UnicodeString hostName = TheGameSpyInfo->getCurrentStagingRoom()->getSlot(0)->getName();
						AsciiString asciiName;
						asciiName.translate(hostName);
						PeerRequest req;
						req.peerRequestType = PeerRequest::PEERREQUEST_UTMPLAYER;
						req.UTM.isStagingRoom = TRUE;
						req.id = "accept";
						req.nick = asciiName.str();
						req.options = "true";
						TheGameSpyPeerMessageQueue->addRequest(req);
						//peerSetReady( PEER, PEERTrue );
						WOLDisplaySlotList();
					}
				}
				else
				{
					for (Int i = 0; i < MAX_SLOTS; i++)
					{
						if (controlID == buttonMapStartPositionID[i])
						{
							GameSpyStagingRoom *game = TheGameSpyInfo->getCurrentStagingRoom();
							Int playerIdxInPos = -1;
							for (Int j=0; j<MAX_SLOTS; ++j)
							{
								GameSpyGameSlot *slot = game->getGameSpySlot(j);
								if (slot && slot->getStartPos() == i)
								{
									playerIdxInPos = j;
									break;
								}
							}
							if (playerIdxInPos >= 0)
							{
								GameSpyGameSlot *slot = game->getGameSpySlot(playerIdxInPos);
								if (playerIdxInPos == game->getLocalSlotNum() || (game->amIHost() && slot && slot->isAI()))
								{
									// it's one of my type.  Try to change it.
									Int nextPlayer = getNextSelectablePlayer(playerIdxInPos+1);
									handleStartPositionSelection(playerIdxInPos, -1);
									if (nextPlayer >= 0)
									{
										handleStartPositionSelection(nextPlayer, i);
									}
								}
							}
							else
							{
								// nobody in the slot - put us in
								Int nextPlayer = getNextSelectablePlayer(0);
								if (nextPlayer < 0)
									nextPlayer = getFirstSelectablePlayer(game);
								handleStartPositionSelection(nextPlayer, i);
							}
						}
					}
				}


				break;
			}// case GBM_SELECTED:
		//-------------------------------------------------------------------------------------------------
		case GBM_SELECTED_RIGHT:
   		{
   			if (buttonPushed)
   				break;

   			GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();
				for (Int i = 0; i < MAX_SLOTS; i++)
				{
					if (controlID == buttonMapStartPositionID[i])
					{
						GameSpyStagingRoom *game = TheGameSpyInfo->getCurrentStagingRoom();
						Int playerIdxInPos = -1;
						for (Int j=0; j<MAX_SLOTS; ++j)
						{
							GameSpyGameSlot *slot = game->getGameSpySlot(j);
							if (slot && slot->getStartPos() == i)
							{
								playerIdxInPos = j;
								break;
							}
						}
						if (playerIdxInPos >= 0)
						{
							GameSpyGameSlot *slot = game->getGameSpySlot(playerIdxInPos);
							if (playerIdxInPos == game->getLocalSlotNum() || (game->amIHost() && slot && slot->isAI()))
							{
								// it's one of my type.  Remove it.
								handleStartPositionSelection(playerIdxInPos, -1);
							}
						}
					}
				}
				break;
			}

		//-------------------------------------------------------------------------------------------------
		case GEM_EDIT_DONE:
			{
				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();
				// Take the user's input and echo it into the chat window as well as
				// send it to the other clients on the lan
				if ( controlID == textEntryChatID )
				{

					// read the user's input
					txtInput.set(GadgetTextEntryGetText( textEntryChat ));
					// Clear the text entry line
					GadgetTextEntrySetText(textEntryChat, UnicodeString::TheEmptyString);
					// Clean up the text (remove leading/trailing chars, etc)
					txtInput.trim();
					// Echo the user's input to the chat window
					if (!txtInput.isEmpty())
					{
						if (!handleGameSetupSlashCommands(txtInput))
						{
							TheGameSpyInfo->sendChat(txtInput, false, NULL);
						}
					}

				}// if ( controlID == textEntryChatID )
				break;
			}
		//-------------------------------------------------------------------------------------------------
		default:
			return MSG_IGNORED;
	}//Switch
	return MSG_HANDLED;
}//WindowMsgHandledType WOLGameSetupMenuSystem( GameWindow *window, UnsignedInt msg,


