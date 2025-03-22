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
// FILE: LobbyUtils.cpp
// Author: Matthew D. Campbell, Sept 2002
// Description: GameSpy lobby utils
///////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/GameEngine.h"
#include "Common/MultiplayerSettings.h"
#include "Common/PlayerTemplate.h"
#include "Common/Version.h"
#include "GameClient/AnimateWindowManager.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/Image.h"
#include "GameClient/Shell.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetComboBox.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GadgetTextEntry.h"
#include "GameClient/GameText.h"
#include "GameClient/MapUtil.h"
#include "GameClient/MessageBox.h"
#include "GameClient/Mouse.h"
#include "GameNetwork/GameSpyOverlay.h"

#include "GameClient/LanguageFilter.h"
#include "GameNetwork/GameSpy/BuddyDefs.h"
#include "GameNetwork/GameSpy/LadderDefs.h"
#include "GameNetwork/GameSpy/LobbyUtils.h"
#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/GameSpy/PeerThread.h"
#include "GameNetwork/GameSpy/PersistentStorageDefs.h"
#include "GameNetwork/GameSpy/GSConfig.h"

#include "Common/STLTypedefs.h"

#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
static enum {
	COLUMN_NAME = 0,
	COLUMN_MAP,
	COLUMN_LADDER,
	COLUMN_NUMPLAYERS,
	COLUMN_PASSWORD,
	COLUMN_OBSERVER,
	COLUMN_PING,
};

static NameKeyType buttonSortAlphaID = NAMEKEY_INVALID;
static NameKeyType buttonSortPingID = NAMEKEY_INVALID;
static NameKeyType buttonSortBuddiesID = NAMEKEY_INVALID;
static NameKeyType windowSortAlphaID = NAMEKEY_INVALID;
static NameKeyType windowSortPingID = NAMEKEY_INVALID;
static NameKeyType windowSortBuddiesID = NAMEKEY_INVALID;

static GameWindow *buttonSortAlpha = NULL;
static GameWindow *buttonSortPing = NULL;
static GameWindow *buttonSortBuddies = NULL;
static GameWindow *windowSortAlpha = NULL;
static GameWindow *windowSortPing = NULL;
static GameWindow *windowSortBuddies = NULL;

static GameSortType theGameSortType = GAMESORT_ALPHA_ASCENDING;
static Bool sortBuddies = TRUE;
static void showSortIcons(void)
{
	if (windowSortAlpha && windowSortPing)
	{
		switch(theGameSortType)
		{
			case GAMESORT_ALPHA_ASCENDING:
				windowSortAlpha->winHide(FALSE);
				windowSortAlpha->winEnable(TRUE);
				windowSortPing->winHide(TRUE);
				break;
			case GAMESORT_ALPHA_DESCENDING:
				windowSortAlpha->winHide(FALSE);
				windowSortAlpha->winEnable(FALSE);
				windowSortPing->winHide(TRUE);
				break;
			case GAMESORT_PING_ASCENDING:
				windowSortPing->winHide(FALSE);
				windowSortPing->winEnable(TRUE);
				windowSortAlpha->winHide(TRUE);
				break;
			case GAMESORT_PING_DESCENDING:
				windowSortPing->winHide(FALSE);
				windowSortPing->winEnable(FALSE);
				windowSortAlpha->winHide(TRUE);
				break;
		}
	}

	if (sortBuddies)
	{
		if (windowSortBuddies)
		{
			windowSortBuddies->winHide(FALSE);
		}
	}
	else
	{
		if (windowSortBuddies)
		{
			windowSortBuddies->winHide(TRUE);
		}
	}
}
void setSortMode( GameSortType sortType ) { theGameSortType = sortType; showSortIcons(); RefreshGameListBoxes(); }
void sortByBuddies( Bool doSort ) { sortBuddies = doSort; showSortIcons(); RefreshGameListBoxes(); }

Bool HandleSortButton( NameKeyType sortButton )
{
	if (sortButton == buttonSortBuddiesID)
	{
		sortByBuddies( !sortBuddies );
		return TRUE;
	}
	else if (sortButton == buttonSortAlphaID)
	{
		if (theGameSortType == GAMESORT_ALPHA_ASCENDING)
		{
			setSortMode(GAMESORT_ALPHA_DESCENDING);
		}
		else
		{
			setSortMode(GAMESORT_ALPHA_ASCENDING);
		}
		return TRUE;
	}
	else if (sortButton == buttonSortPingID)
	{
		if (theGameSortType == GAMESORT_PING_ASCENDING)
		{
			setSortMode(GAMESORT_PING_DESCENDING);
		}
		else
		{
			setSortMode(GAMESORT_PING_ASCENDING);
		}
		return TRUE;
	}
	return FALSE;
}

// window ids ------------------------------------------------------------------------------
static NameKeyType parentID = NAMEKEY_INVALID;
//static NameKeyType parentGameListSmallID = NAMEKEY_INVALID;
static NameKeyType parentGameListLargeID = NAMEKEY_INVALID;
static NameKeyType listboxLobbyGamesSmallID = NAMEKEY_INVALID;
static NameKeyType listboxLobbyGamesLargeID = NAMEKEY_INVALID;
//static NameKeyType listboxLobbyGameInfoID = NAMEKEY_INVALID;

// Window Pointers ------------------------------------------------------------------------
static GameWindow *parent = NULL;
//static GameWindow *parentGameListSmall = NULL;
static GameWindow *parentGameListLarge = NULL;
       //GameWindow *listboxLobbyGamesSmall = NULL;
       GameWindow *listboxLobbyGamesLarge = NULL;
       //GameWindow *listboxLobbyGameInfo = NULL;

static const Image *pingImages[3] = { NULL, NULL, NULL };

static void gameTooltip(GameWindow *window,
													WinInstanceData *instData,
													UnsignedInt mouse)
{
	Int x, y, row, col;
	x = LOLONGTOSHORT(mouse);
	y = HILONGTOSHORT(mouse);

	GadgetListBoxGetEntryBasedOnXY(window, x, y, row, col);

	if (row == -1 || col == -1)
	{
		TheMouse->setCursorTooltip( UnicodeString::TheEmptyString);//TheGameText->fetch("TOOLTIP:GamesBeingFormed") );
		return;
	}

	Int gameID = (Int)GadgetListBoxGetItemData(window, row, 0);
	GameSpyStagingRoom *room = TheGameSpyInfo->findStagingRoomByID(gameID);
	if (!room)
	{
		TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:UnknownGame") );
		return;
	}

	if (col == COLUMN_PING)
	{
#ifdef DEBUG_LOGGING
		UnicodeString s;
		s.format(L"Ping is %d ms (cutoffs are %d ms and %d ms\n%hs local pings\n%hs remote pings",
			room->getPingAsInt(), TheGameSpyConfig->getPingCutoffGood(), TheGameSpyConfig->getPingCutoffBad(),
			TheGameSpyInfo->getPingString().str(), room->getPingString().str()
		);
		TheMouse->setCursorTooltip( s, 10, NULL, 2.0f ); // the text and width are the only params used.  the others are the default values.
#else
		TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:PingInfo"), 10, NULL, 2.0f ); // the text and width are the only params used.  the others are the default values.
#endif
		return;
	}
	if (col == COLUMN_NUMPLAYERS)
	{
		TheMouse->setCursorTooltip( TheGameText->fetch("TOOLTIP:NumberOfPlayers"), 10, NULL, 2.0f ); // the text and width are the only params used.  the others are the default values.
		return;
	}
	if (col == COLUMN_PASSWORD)
	{
		if (room->getHasPassword())
		{
			UnicodeString checkTooltip =TheGameText->fetch("TOOTIP:Password");
			if(!checkTooltip.compare(L"Password required to joing game"))
				checkTooltip.set(L"Password required to join game");
			TheMouse->setCursorTooltip( checkTooltip, 10, NULL, 2.0f ); // the text and width are the only params used.  the others are the default values.
		}
		else
			TheMouse->setCursorTooltip( UnicodeString::TheEmptyString );
		return;
	}

	UnicodeString tooltip;

	UnicodeString mapName;
	const MapMetaData *md = TheMapCache->findMap(room->getMap());
	if (md)
	{
		mapName = md->m_displayName;
	}
	else
	{
		const char *start = room->getMap().reverseFind('\\');
		if (start)
		{
			++start;
		}
		else
		{
			start = room->getMap().str();
		}
		mapName.translate( start );
	}
	UnicodeString tmp;
	tooltip.format(TheGameText->fetch("TOOLTIP:GameInfoGameName"), room->getGameName().str());
	if (room->getLadderPort() != 0)
	{
		const LadderInfo *linfo = TheLadderList->findLadder(room->getLadderIP(), room->getLadderPort());
		if (linfo)
		{
			tmp.format(TheGameText->fetch("TOOLTIP:GameInfoLadderName"), linfo->name.str());
			tooltip.concat(tmp);
		}
	}
	if (room->getExeCRC() != TheGlobalData->m_exeCRC || room->getIniCRC() != TheGlobalData->m_iniCRC)
	{
		tmp.format(TheGameText->fetch("TOOLTIP:InvalidGameVersion"), mapName.str());
		tooltip.concat(tmp);
	}
	tmp.format(TheGameText->fetch("TOOLTIP:GameInfoMap"), mapName.str());
	tooltip.concat(tmp);

	AsciiString aPlayer;
	UnicodeString player;
	Int numPlayers = 0;
	for (Int i=0; i<MAX_SLOTS; ++i)
	{
		GameSpyGameSlot *slot = room->getGameSpySlot(i);
		if (i == 0 && (!slot || !slot->isHuman()))
		{
			DEBUG_CRASH(("About to tooltip a non-hosted game!\n"));
		}
		if (slot && slot->isHuman())
		{
			tmp.format(TheGameText->fetch("TOOLTIP:GameInfoPlayer"), slot->getName().str(), slot->getWins(), slot->getLosses());
			tooltip.concat(tmp);
			++numPlayers;
		}
		else if (slot && slot->isAI())
		{
			++numPlayers;
			switch(slot->getState())
			{
			case SLOT_EASY_AI:
				tooltip.concat(L'\n');
				tooltip.concat(TheGameText->fetch("GUI:EasyAI"));
				break;
			case SLOT_MED_AI:
				tooltip.concat(L'\n');
				tooltip.concat(TheGameText->fetch("GUI:MediumAI"));
				break;
			case SLOT_BRUTAL_AI:
				tooltip.concat(L'\n');
				tooltip.concat(TheGameText->fetch("GUI:HardAI"));
				break;
			}
		}
	}
	DEBUG_ASSERTCRASH(numPlayers, ("Tooltipping a 0-player game!\n"));

	TheMouse->setCursorTooltip( tooltip, 10, NULL, 2.0f ); // the text and width are the only params used.  the others are the default values.
}

static Bool isSmall = TRUE;

GameWindow *GetGameListBox( void )
{
	return listboxLobbyGamesLarge;
}

GameWindow *GetGameInfoListBox( void )
{
	return NULL;
}

NameKeyType GetGameListBoxID( void )
{
	return listboxLobbyGamesLargeID;
}

NameKeyType GetGameInfoListBoxID( void )
{
	return NAMEKEY_INVALID;
}

void GrabWindowInfo( void )
{
	isSmall = TRUE;
	parentID = NAMEKEY( "WOLCustomLobby.wnd:WOLLobbyMenuParent" );
	parent = TheWindowManager->winGetWindowFromId(NULL, parentID);

	pingImages[0] = TheMappedImageCollection->findImageByName("Ping03");
	pingImages[1] = TheMappedImageCollection->findImageByName("Ping02");
	pingImages[2] = TheMappedImageCollection->findImageByName("Ping01");
	DEBUG_ASSERTCRASH(pingImages[0], ("Can't find ping image!"));
	DEBUG_ASSERTCRASH(pingImages[1], ("Can't find ping image!"));
	DEBUG_ASSERTCRASH(pingImages[2], ("Can't find ping image!"));

//	parentGameListSmallID = NAMEKEY( "WOLCustomLobby.wnd:ParentGameListSmall" );
//	parentGameListSmall = TheWindowManager->winGetWindowFromId(NULL, parentGameListSmallID);

	parentGameListLargeID = NAMEKEY( "WOLCustomLobby.wnd:ParentGameListLarge" );
	parentGameListLarge = TheWindowManager->winGetWindowFromId(NULL, parentGameListLargeID);

	listboxLobbyGamesSmallID = NAMEKEY( "WOLCustomLobby.wnd:ListboxGames" );
//	listboxLobbyGamesSmall = TheWindowManager->winGetWindowFromId(NULL, listboxLobbyGamesSmallID);
//	listboxLobbyGamesSmall->winSetTooltipFunc(gameTooltip);

	listboxLobbyGamesLargeID = NAMEKEY( "WOLCustomLobby.wnd:ListboxGamesLarge" );
	listboxLobbyGamesLarge = TheWindowManager->winGetWindowFromId(NULL, listboxLobbyGamesLargeID);
	listboxLobbyGamesLarge->winSetTooltipFunc(gameTooltip);
//
//	listboxLobbyGameInfoID = NAMEKEY( "WOLCustomLobby.wnd:ListboxGameInfo" );
//	listboxLobbyGameInfo = TheWindowManager->winGetWindowFromId(NULL, listboxLobbyGameInfoID);

	buttonSortAlphaID = NAMEKEY("WOLCustomLobby.wnd:ButtonSortAlpha");
	buttonSortPingID = NAMEKEY("WOLCustomLobby.wnd:ButtonSortPing");
	buttonSortBuddiesID = NAMEKEY("WOLCustomLobby.wnd:ButtonSortBuddies");
	windowSortAlphaID = NAMEKEY("WOLCustomLobby.wnd:WindowSortAlpha");
	windowSortPingID = NAMEKEY("WOLCustomLobby.wnd:WindowSortPing");
	windowSortBuddiesID = NAMEKEY("WOLCustomLobby.wnd:WindowSortBuddies");

	buttonSortAlpha = TheWindowManager->winGetWindowFromId(parent, buttonSortAlphaID);
	buttonSortPing = TheWindowManager->winGetWindowFromId(parent, buttonSortPingID);
	buttonSortBuddies = TheWindowManager->winGetWindowFromId(parent, buttonSortBuddiesID);
	windowSortAlpha = TheWindowManager->winGetWindowFromId(parent, windowSortAlphaID);
	windowSortPing = TheWindowManager->winGetWindowFromId(parent, windowSortPingID);
	windowSortBuddies = TheWindowManager->winGetWindowFromId(parent, windowSortBuddiesID);

	showSortIcons();
}

void ReleaseWindowInfo( void )
{
	isSmall = TRUE;
	parent = NULL;
//	parentGameListSmall = NULL;
	parentGameListLarge = NULL;
//	listboxLobbyGamesSmall = NULL;
	listboxLobbyGamesLarge = NULL;
//	listboxLobbyGameInfo = NULL;

	buttonSortAlpha = NULL;
	buttonSortPing = NULL;
	buttonSortBuddies = NULL;
	windowSortAlpha = NULL;
	windowSortPing = NULL;
	windowSortBuddies = NULL;
}

typedef std::set<GameSpyStagingRoom *> BuddyGameSet;
static BuddyGameSet *theBuddyGames = NULL;
static void populateBuddyGames(void)
{
	BuddyInfoMap *m = TheGameSpyInfo->getBuddyMap();
	theBuddyGames = NEW BuddyGameSet;
	if (!m)
	{
		return;
	}
	for (BuddyInfoMap::const_iterator bit = m->begin(); bit != m->end(); ++bit)
	{
		BuddyInfo info = bit->second;
		if (info.m_status == GP_STAGING)
		{
			StagingRoomMap *srm = TheGameSpyInfo->getStagingRoomList();
			for (StagingRoomMap::iterator srmIt = srm->begin(); srmIt != srm->end(); ++srmIt)
			{
				GameSpyStagingRoom *game = srmIt->second;
				game->cleanUpSlotPointers();
				const GameSpyGameSlot *slot = game->getGameSpySlot(0);
				if (slot && slot->getName() == info.m_locationString)
				{
					theBuddyGames->insert(game);
					break;
				}
			}
		}
	}
}

static void clearBuddyGames(void)
{
	if (theBuddyGames)
		delete theBuddyGames;
	theBuddyGames = NULL;
}

struct GameSortStruct
{
	bool operator()(GameSpyStagingRoom *g1, GameSpyStagingRoom *g2) const
	{
		// sort CRC mismatches to the bottom
		Bool g1Good = (g1->getExeCRC() != TheGlobalData->m_exeCRC || g1->getIniCRC() != TheGlobalData->m_iniCRC);
		Bool g2Good = (g1->getExeCRC() != TheGlobalData->m_exeCRC || g1->getIniCRC() != TheGlobalData->m_iniCRC);
		if ( g1Good ^ g2Good )
		{
			return g1Good;
		}

		// sort games with private ladders to the bottom
		Bool g1UnknownLadder = (g1->getLadderPort() && TheLadderList->findLadder(g1->getLadderIP(), g1->getLadderPort()) == NULL);
		Bool g2UnknownLadder = (g2->getLadderPort() && TheLadderList->findLadder(g2->getLadderIP(), g2->getLadderPort()) == NULL);
		if ( g1UnknownLadder ^ g2UnknownLadder )
		{
			return g2UnknownLadder;
		}

		// sort full games to the bottom
		Bool g1Full = (g1->getNumNonObserverPlayers() == g1->getMaxPlayers() || g1->getNumPlayers() == MAX_SLOTS);
		Bool g2Full = (g2->getNumNonObserverPlayers() == g2->getMaxPlayers() || g2->getNumPlayers() == MAX_SLOTS);
		if ( g1Full ^ g2Full )
		{
			return g2Full;
		}

		if (sortBuddies)
		{
			Bool g1HasBuddies = (theBuddyGames->find(g1) != theBuddyGames->end());
			Bool g2HasBuddies = (theBuddyGames->find(g2) != theBuddyGames->end());
			if ( g1HasBuddies ^ g2HasBuddies )
			{
				return g1HasBuddies;
			}
		}

		switch(theGameSortType)
		{
		case GAMESORT_ALPHA_ASCENDING:
			return wcsicmp(g1->getGameName().str(), g2->getGameName().str()) < 0;
			break;
		case GAMESORT_ALPHA_DESCENDING:
			return wcsicmp(g1->getGameName().str(),g2->getGameName().str()) > 0;
			break;
		case GAMESORT_PING_ASCENDING:
			return g1->getPingAsInt() < g2->getPingAsInt();
			break;
		case GAMESORT_PING_DESCENDING:
			return g1->getPingAsInt() > g2->getPingAsInt();
			break;
		}
		return false;
	}
};

static Int insertGame( GameWindow *win, GameSpyStagingRoom *game, Bool showMap )
{
	game->cleanUpSlotPointers();
	Color gameColor = GameSpyColor[GSCOLOR_GAME];
	if (game->getNumNonObserverPlayers() == game->getMaxPlayers() || game->getNumPlayers() == MAX_SLOTS)
	{
		gameColor = GameSpyColor[GSCOLOR_GAME_FULL];
	}
	if (game->getExeCRC() != TheGlobalData->m_exeCRC || game->getIniCRC() != TheGlobalData->m_iniCRC)
	{
		gameColor = GameSpyColor[GSCOLOR_GAME_CRCMISMATCH];
	}
	UnicodeString gameName = game->getGameName();
	
	if(TheGameSpyInfo->getDisallowAsianText())
	{
		const WideChar *buff = gameName.str();
		Int length =  gameName.getLength();	
		for(Int i = 0; i < length; ++i)
		{
			if(buff[i] >= 256)
				return -1;
		}
	}
	else if(TheGameSpyInfo->getDisallowNonAsianText())
	{
		const WideChar *buff = gameName.str();
		Int length =  gameName.getLength();	
		Bool hasUnicode = FALSE;
		for(Int i = 0; i < length; ++i)
		{
			if(buff[i] >= 256)
			{
				hasUnicode = TRUE;
				break;
			}
		}
		if(!hasUnicode)
			return -1;
	}



	Int index = GadgetListBoxAddEntryText(win, game->getGameName(), gameColor, -1, COLUMN_NAME);
	GadgetListBoxSetItemData(win, (void *)game->getID(), index);

	UnicodeString s;

	if (showMap)
	{
		UnicodeString mapName;
		const MapMetaData *md = TheMapCache->findMap(game->getMap());
		if (md)
		{
			mapName = md->m_displayName;
		}
		else
		{
			const char *start = game->getMap().reverseFind('\\');
			if (start)
			{
				++start;
			}
			else
			{
				start = game->getMap().str();
			}
			mapName.translate( start );
		}
		GadgetListBoxAddEntryText(win, mapName, gameColor, index, COLUMN_MAP);

		const LadderInfo * li = TheLadderList->findLadder(game->getLadderIP(), game->getLadderPort());
		if (li)
		{
			GadgetListBoxAddEntryText(win, li->name, gameColor, index, COLUMN_LADDER);
		}
		else if (game->getLadderPort())
		{
			GadgetListBoxAddEntryText(win, TheGameText->fetch("GUI:UnknownLadder"), gameColor, index, COLUMN_LADDER);
		}
		else
		{
			GadgetListBoxAddEntryText(win, TheGameText->fetch("GUI:NoLadder"), gameColor, index, COLUMN_LADDER);
		}
	}
	else
	{
		GadgetListBoxAddEntryText(win, UnicodeString(L" "), gameColor, index, COLUMN_MAP);
		GadgetListBoxAddEntryText(win, UnicodeString(L" "), gameColor, index, COLUMN_LADDER);
	}

	s.format(L"%d/%d", game->getReportedNumPlayers(), game->getReportedMaxPlayers());
	GadgetListBoxAddEntryText(win, s, gameColor, index, COLUMN_NUMPLAYERS);

	if (game->getHasPassword())
	{
		const Image *img = TheMappedImageCollection->findImageByName("Password");
		Int width = 10, height = 10;
		if (img)
		{
			width = img->getImageWidth();
			height = img->getImageHeight();
		}
		GadgetListBoxAddEntryImage(win, img, index, COLUMN_PASSWORD, width, height);
	}
	else
	{
		GadgetListBoxAddEntryText(win, UnicodeString(L" "), gameColor, index, COLUMN_PASSWORD);
	}

	if (game->getAllowObservers())
	{
		const Image *img = TheMappedImageCollection->findImageByName("Observer");
		GadgetListBoxAddEntryImage(win, img, index, COLUMN_OBSERVER);
	}
	else
	{
		GadgetListBoxAddEntryText(win, UnicodeString(L" "), gameColor, index, COLUMN_OBSERVER);
	}

	s.format(L"%d", game->getPingAsInt());
	GadgetListBoxAddEntryText(win, s, gameColor, index, COLUMN_PING);
	Int ping = game->getPingAsInt();
	Int width = 10, height = 10;
	if (pingImages[0])
	{
		width = pingImages[0]->getImageWidth();
		height = pingImages[0]->getImageHeight();
	}
	// CLH picking an arbitrary number for our ping display
	if (ping < TheGameSpyConfig->getPingCutoffGood())
	{
		GadgetListBoxAddEntryImage(win, pingImages[0], index, COLUMN_PING, width, height);
	}
	else if (ping < TheGameSpyConfig->getPingCutoffBad())
	{
		GadgetListBoxAddEntryImage(win, pingImages[1], index, COLUMN_PING, width, height);
	}
	else
	{
		GadgetListBoxAddEntryImage(win, pingImages[2], index, COLUMN_PING, width, height);
	}

	return index;
}

void RefreshGameListBox( GameWindow *win, Bool showMap )
{
	if (!win)
		return;

	// save off selection
	Int selectedIndex = -1;
	Int indexToSelect = -1;
	Int selectedID = 0;
	GadgetListBoxGetSelected(win, &selectedIndex);
	if (selectedIndex != -1 )
	{
		selectedID = (Int)GadgetListBoxGetItemData(win, selectedIndex);
	}
	int prevPos = GadgetListBoxGetTopVisibleEntry( win );

	// empty listbox
	GadgetListBoxReset(win);

	// sort our games
	typedef std::multiset<GameSpyStagingRoom *, GameSortStruct> SortedGameList;
	SortedGameList sgl;
	StagingRoomMap *srm = TheGameSpyInfo->getStagingRoomList();
	populateBuddyGames();
	for (StagingRoomMap::iterator srmIt = srm->begin(); srmIt != srm->end(); ++srmIt)
	{
		sgl.insert(srmIt->second);
	}

	// populate listbox
	for (SortedGameList::iterator sglIt = sgl.begin(); sglIt != sgl.end(); ++sglIt)
	{
		GameSpyStagingRoom *game = *sglIt;
		if (game)
		{
			Int index = insertGame(win, game, showMap);
			if (game->getID() == selectedID)
			{
				indexToSelect = index;
			}
		}
	}

	clearBuddyGames();

	// restore selection
	GadgetListBoxSetSelected(win, indexToSelect); // even for -1, so we can disable the 'Join Game' button
//	if(prevPos > 10)
		GadgetListBoxSetTopVisibleEntry( win, prevPos  );//+ 1

	if (indexToSelect < 0 && selectedID)
	{
		TheWindowManager->winSetLoneWindow(NULL);
	}
}

void RefreshGameInfoListBox( GameWindow *mainWin, GameWindow *win )
{
//	if (!mainWin || !win)
//		return;
//
//	GadgetListBoxReset(win);
//
//	Int selected = -1;
//	GadgetListBoxGetSelected(mainWin, &selected);
//	if (selected < 0)
//	{
//		return;
//	}
//
//	Int selectedID = (Int)GadgetListBoxGetItemData(mainWin, selected);
//	if (selectedID < 0)
//	{
//		return;
//	}
//
//	StagingRoomMap *srm = TheGameSpyInfo->getStagingRoomList();
//	StagingRoomMap::iterator srmIt = srm->find(selectedID);
//	if (srmIt != srm->end())
//	{
//		GameSpyStagingRoom *theRoom = srmIt->second;
//		theRoom->cleanUpSlotPointers();
//
//		// game name
////		GadgetListBoxAddEntryText(listboxLobbyGameInfo, theRoom->getGameName(), GameSpyColor[GSCOLOR_DEFAULT], -1);
//
//		const LadderInfo * li = TheLadderList->findLadder(theRoom->getLadderIP(), theRoom->getLadderPort());
//		if (li)
//		{
//			UnicodeString tmp;
//			tmp.format(TheGameText->fetch("TOOLTIP:LadderName"), li->name.str());
//			GadgetListBoxAddEntryText(listboxLobbyGameInfo, tmp, GameSpyColor[GSCOLOR_DEFAULT], -1);
//		}
//		else if (theRoom->getLadderPort())
//		{
//			GadgetListBoxAddEntryText(listboxLobbyGameInfo, TheGameText->fetch("TOOLTIP:UnknownLadder"), GameSpyColor[GSCOLOR_DEFAULT], -1);
//		}
//		else
//		{
//			GadgetListBoxAddEntryText(listboxLobbyGameInfo, TheGameText->fetch("TOOLTIP:NoLadder"), GameSpyColor[GSCOLOR_DEFAULT], -1);
//		}
//
//		if (theRoom->getExeCRC() != TheGlobalData->m_exeCRC || theRoom->getIniCRC() != TheGlobalData->m_iniCRC)
//		{
//			GadgetListBoxAddEntryText(listboxLobbyGameInfo, TheGameText->fetch("TOOLTIP:InvalidGameVersionSingleLine"), GameSpyColor[GSCOLOR_DEFAULT], -1);
//		}
//
//		// map name
//		UnicodeString mapName;
//		const MapMetaData *md = TheMapCache->findMap(theRoom->getMap());
//		if (md)
//		{
//			mapName = md->m_displayName;
//		}
//		else
//		{
//			const char *start = theRoom->getMap().reverseFind('\\');
//			if (start)
//			{
//				++start;
//			}
//			else
//			{
//				start = theRoom->getMap().str();
//			}
//			mapName.translate( start );
//		}
//
//		GadgetListBoxAddEntryText(listboxLobbyGameInfo, mapName, GameSpyColor[GSCOLOR_DEFAULT], -1);
//
//		// player list (rank, win/loss, side)
//		for (Int i=0; i<MAX_SLOTS; ++i)
//		{
//			const GameSpyGameSlot *slot = theRoom->getGameSpySlot(i);
//			if (slot && slot->isHuman())
//			{
//				UnicodeString theName, theRating, thePlayerTemplate;
//				Int colorIdx = slot->getColor();
//				theName = slot->getName();
//				theRating.format(L" (%d-%d)", slot->getWins(), slot->getLosses());
//				const PlayerTemplate * pt = ThePlayerTemplateStore->getNthPlayerTemplate(slot->getPlayerTemplate());
//				if (pt)
//				{
//					thePlayerTemplate = pt->getDisplayName();
//				}
//				else
//				{
//					thePlayerTemplate = TheGameText->fetch("GUI:Random");
//				}
//
//				UnicodeString theText;
//				theText.format(L"%ls - %ls - %ls", theName.str(), thePlayerTemplate.str(), theRating.str());
//
//				Int theColor = GameSpyColor[GSCOLOR_DEFAULT];
//				const MultiplayerColorDefinition *mcd = TheMultiplayerSettings->getColor(colorIdx);
//				if (mcd)
//				{
//					theColor = mcd->getColor();
//				}
//
//				GadgetListBoxAddEntryText(listboxLobbyGameInfo, theText, theColor, -1);
//			}
//		}
//	}

}

void RefreshGameListBoxes( void )
{
	GameWindow *main = GetGameListBox();
	GameWindow *info = GetGameInfoListBox();

	RefreshGameListBox( main, (info == NULL) );

	if (info)
	{
		RefreshGameInfoListBox( main, info );
	}
}

void ToggleGameListType( void )
{
	isSmall = !isSmall;
	if(isSmall)
	{
		parentGameListLarge->winHide(TRUE);
//		parentGameListSmall->winHide(FALSE);
	}
	else
	{
		parentGameListLarge->winHide(FALSE);
//		parentGameListSmall->winHide(TRUE);
	}

	RefreshGameListBoxes();
}

