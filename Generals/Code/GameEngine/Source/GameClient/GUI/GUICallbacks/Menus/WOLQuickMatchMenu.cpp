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
// FILE: WOLQuickMatchMenu.cpp
// Author: Chris Huybregts, November 2001
// Description: Lan Lobby Menu
///////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/GameEngine.h"
#include "Common/QuickmatchPreferences.h"
#include "Common/LadderPreferences.h"
#include "Common/MultiplayerSettings.h"
#include "Common/PlayerTemplate.h"
#include "GameClient/AnimateWindowManager.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/GameText.h"
#include "GameClient/InGameUI.h"
#include "GameClient/Shell.h"
#include "GameClient/ShellHooks.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetComboBox.h"
#include "GameClient/GadgetPushButton.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GadgetTextEntry.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/MapUtil.h"
#include "GameClient/GameWindowTransitions.h"

#include "GameLogic/GameLogic.h"

#include "GameNetwork/NAT.h"
#include "GameNetwork/GameSpyOverlay.h"
#include "GameNetwork/GameSpy/BuddyDefs.h"
#include "GameNetwork/GameSpy/GSConfig.h"
#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/GameSpy/PeerThread.h"
#include "GameNetwork/GameSpy/PersistentStorageDefs.h"
#include "GameNetwork/GameSpy/PersistentStorageThread.h"
#include "GameNetwork/RankPointValue.h"
#include "GameNetwork/GameSpy/LadderDefs.h"
#ifdef _INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

#ifdef DEBUG_LOGGING
#include "Common/MiniLog.h"
//#define PERF_TEST
static LogClass s_perfLog("QMPerf.txt");
static Bool s_inQM = FALSE;
#define PERF_LOG(x) s_perfLog.log x
#else // DEBUG_LOGGING
#define PERF_LOG(x) {}
#endif // DEBUG_LOGGING

// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
// window ids ------------------------------------------------------------------------------
static NameKeyType parentWOLQuickMatchID = NAMEKEY_INVALID;
static NameKeyType buttonBackID = NAMEKEY_INVALID;
static NameKeyType buttonStartID = NAMEKEY_INVALID;
static NameKeyType buttonStopID = NAMEKEY_INVALID;
static NameKeyType buttonWidenID = NAMEKEY_INVALID;
static NameKeyType buttonBuddiesID = NAMEKEY_INVALID;
static NameKeyType listboxQuickMatchID = NAMEKEY_INVALID;
static NameKeyType listboxMapSelectID = NAMEKEY_INVALID;
static NameKeyType buttonSelectAllMapsID = NAMEKEY_INVALID;
static NameKeyType buttonSelectNoMapsID = NAMEKEY_INVALID;
//static NameKeyType textEntryMaxDisconnectsID = NAMEKEY_INVALID;
//static NameKeyType textEntryMaxPointsID = NAMEKEY_INVALID;
//static NameKeyType textEntryMinPointsID = NAMEKEY_INVALID;
static NameKeyType textEntryWaitTimeID = NAMEKEY_INVALID;
static NameKeyType comboBoxNumPlayersID = NAMEKEY_INVALID;
static NameKeyType comboBoxMaxPingID = NAMEKEY_INVALID;
static NameKeyType comboBoxLadderID = NAMEKEY_INVALID;
static NameKeyType comboBoxMaxDisconnectsID = NAMEKEY_INVALID;
static NameKeyType staticTextNumPlayersID = NAMEKEY_INVALID;
static NameKeyType comboBoxSideID = NAMEKEY_INVALID;
static NameKeyType comboBoxColorID = NAMEKEY_INVALID;


// Window Pointers ------------------------------------------------------------------------
static GameWindow *parentWOLQuickMatch = NULL;
static GameWindow *buttonBack = NULL;
static GameWindow *buttonStart = NULL;
static GameWindow *buttonStop = NULL;
static GameWindow *buttonWiden = NULL;
GameWindow *quickmatchTextWindow = NULL;
static GameWindow *listboxMapSelect = NULL;
//static GameWindow *textEntryMaxDisconnects = NULL;
//static GameWindow *textEntryMaxPoints = NULL;
//static GameWindow *textEntryMinPoints = NULL;
static GameWindow *textEntryWaitTime = NULL;
static GameWindow *comboBoxNumPlayers = NULL;
static GameWindow *comboBoxMaxPing = NULL;
static GameWindow *comboBoxLadder = NULL;
static GameWindow *comboBoxDisabledLadder = NULL; // enable and disable this, but never use it.  it is a stand-in for comboBoxLadder for when there are no ladders
static GameWindow *comboBoxMaxDisconnects = NULL;
static GameWindow *staticTextNumPlayers = NULL;
static GameWindow *comboBoxSide = NULL;
static GameWindow *comboBoxColor = NULL;

static Bool isShuttingDown = false;
static Bool buttonPushed = false;
static const char *nextScreen = NULL;
static Bool raiseMessageBoxes = false;
static Bool isInInit = FALSE;
static const Image *selectedImage = NULL;
static const Image *unselectedImage = NULL;

static bool isPopulatingLadderBox = false;
static Int maxPingEntries = 0;
static Int maxPoints= 100;
static Int minPoints = 0;

static const LadderInfo * getLadderInfo( void );


// [SKB: Jul 01 2003 @ 7:7pm] :
// German2 now has fewer maps.  When trying to do a QM with a Retail version
// the bool string sent to the QMBot is almost always a different size.  This
// mapping is kept so that we now send a string of all maps instead of just
// the ones that are visible to the user.

#define VARIABLE_NUMBER_OF_MAPS 1

#if VARIABLE_NUMBER_OF_MAPS
typedef std::vector<Int> MapListboxIndex;
static MapListboxIndex mapListboxIndex;
#endif


static Bool isInfoShown(void)
{
	static NameKeyType parentStatsID = NAMEKEY("WOLQuickMatchMenu.wnd:ParentStats");
	GameWindow *parentStats = TheWindowManager->winGetWindowFromId( parentWOLQuickMatch, parentStatsID );
	if (parentStats)
		return !parentStats->winIsHidden();
	return FALSE;
}

static void hideInfoGadgets(Bool doIt)
{
	static NameKeyType parentStatsID = NAMEKEY("WOLQuickMatchMenu.wnd:ParentStats");
	GameWindow *parentStats = TheWindowManager->winGetWindowFromId( parentWOLQuickMatch, parentStatsID );
	if (parentStats)
	{
		parentStats->winHide(doIt);
	}
}

static void hideOptionsGadgets(Bool doIt)
{
	static NameKeyType parentOptionsID = NAMEKEY("WOLQuickMatchMenu.wnd:ParentOptions");
	GameWindow *parentOptions = TheWindowManager->winGetWindowFromId( parentWOLQuickMatch, parentOptionsID );
	if (parentOptions)
	{
		parentOptions->winHide(doIt);
		if (comboBoxSide)
			comboBoxSide->winHide(doIt);
		if (comboBoxColor)
			comboBoxColor->winHide(doIt);
		if (comboBoxNumPlayers)
			comboBoxNumPlayers->winHide(doIt);
		if (comboBoxLadder)
			comboBoxLadder->winHide(doIt);
		if (comboBoxDisabledLadder)
			comboBoxDisabledLadder->winHide(doIt);
		if (comboBoxMaxPing)
			comboBoxMaxPing->winHide(doIt);
		if (comboBoxMaxDisconnects)
			comboBoxMaxDisconnects->winHide(doIt);
	}
}

static void enableOptionsGadgets(Bool doIt)
{
#ifdef PERF_TEST
	s_inQM = !doIt;
#endif // PERF_TEST
	static NameKeyType parentOptionsID = NAMEKEY("WOLQuickMatchMenu.wnd:ParentOptions");
	GameWindow *parentOptions = TheWindowManager->winGetWindowFromId( parentWOLQuickMatch, parentOptionsID );
	const LadderInfo *li = getLadderInfo();
	if (parentOptions)
	{
		parentOptions->winEnable(doIt);
		if (comboBoxSide)
			comboBoxSide->winEnable(doIt && (!li || !li->randomFactions));
		if (comboBoxColor)
			comboBoxColor->winEnable(doIt);
		if (comboBoxNumPlayers)
			comboBoxNumPlayers->winEnable(doIt);
		if (comboBoxLadder)
			comboBoxLadder->winEnable(doIt);
		if (comboBoxDisabledLadder)
			comboBoxDisabledLadder->winEnable(FALSE);
		if (comboBoxMaxPing)
			comboBoxMaxPing->winEnable(doIt);
		if (comboBoxMaxDisconnects)
			comboBoxMaxDisconnects->winEnable(doIt);
	}
}

enum 
{
	MAX_DISCONNECTS_ANY = 0,
	MAX_DISCONNECTS_5 = 5,
	MAX_DISCONNECTS_10 = 10,
	MAX_DISCONNECTS_25 = 25,
	MAX_DISCONNECTS_50 = 50,
};
enum{ MAX_DISCONNECTS_COUNT = 5 };

static Int MAX_DISCONNECTS[MAX_DISCONNECTS_COUNT] = {MAX_DISCONNECTS_ANY, MAX_DISCONNECTS_5, 
																											MAX_DISCONNECTS_10, MAX_DISCONNECTS_25,
																											MAX_DISCONNECTS_50};
																										

void UpdateStartButton(void)
{
	if (!comboBoxLadder || !buttonStart || !listboxMapSelect)
		return;

	Int index;
	Int selected;
	GadgetComboBoxGetSelectedPos( comboBoxLadder, &selected );
	index = (Int)GadgetComboBoxGetItemData( comboBoxLadder, selected );
	const LadderInfo *li = TheLadderList->findLadderByIndex( index );
	if (li)
	{
		buttonStart->winEnable(TRUE);
		return;
	}

	Int numMaps = GadgetListBoxGetNumEntries(listboxMapSelect);
	for ( Int i=0; i<numMaps; ++i )
	{
		if ((Bool)GadgetListBoxGetItemData(listboxMapSelect, i, 0))
		{
			buttonStart->winEnable(TRUE);
			return;
		}
	}
	buttonStart->winEnable(FALSE);
}

// -----------------------------------------------------------------------------

static void populateQMColorComboBox(QuickMatchPreferences& pref)
{
	Int numColors = TheMultiplayerSettings->getNumColors();
	UnicodeString colorName;

	GadgetComboBoxReset(comboBoxColor);

	MultiplayerColorDefinition *def = TheMultiplayerSettings->getColor(PLAYERTEMPLATE_RANDOM);
	Int newIndex = GadgetComboBoxAddEntry(comboBoxColor, TheGameText->fetch("GUI:???"), def->getColor());
	GadgetComboBoxSetItemData(comboBoxColor, newIndex, (void *)-1);

	for (Int c=0; c<numColors; ++c)
	{
		def = TheMultiplayerSettings->getColor(c);
		if (!def)
			continue;

		colorName = TheGameText->fetch(def->getTooltipName().str());
		newIndex = GadgetComboBoxAddEntry(comboBoxColor, colorName, def->getColor());
		GadgetComboBoxSetItemData(comboBoxColor, newIndex, (void *)c);
	}
	GadgetComboBoxSetSelectedPos(comboBoxColor, pref.getColor());
}

// -----------------------------------------------------------------------------

static void populateQMSideComboBox(Int favSide, const LadderInfo *li = NULL)
{
	Int numPlayerTemplates = ThePlayerTemplateStore->getPlayerTemplateCount();
	UnicodeString playerTemplateName;

	GadgetComboBoxReset(comboBoxSide);

	MultiplayerColorDefinition *def = TheMultiplayerSettings->getColor(PLAYERTEMPLATE_RANDOM);
	Int newIndex = GadgetComboBoxAddEntry(comboBoxSide, TheGameText->fetch("GUI:Random"), def->getColor());
	GadgetComboBoxSetItemData(comboBoxSide, newIndex, (void *)PLAYERTEMPLATE_RANDOM);

	std::set<AsciiString> seenSides;
	
	Int entryToSelect = 0; // select Random by default

	for (Int c=0; c<numPlayerTemplates; ++c)
	{
		const PlayerTemplate *fac = ThePlayerTemplateStore->getNthPlayerTemplate(c);
		if (!fac)
			continue;

		if (fac->getStartingBuilding().isEmpty())
			continue;

		AsciiString side;
		side.format("SIDE:%s", fac->getSide().str());
		if (seenSides.find(side) != seenSides.end())
			continue;

		if (li)
		{
			if (std::find(li->validFactions.begin(), li->validFactions.end(), fac->getSide()) == li->validFactions.end())
				continue; // ladder doesn't allow it.
		}

		seenSides.insert(side);

		newIndex = GadgetComboBoxAddEntry(comboBoxSide, TheGameText->fetch(side), def->getColor());
		GadgetComboBoxSetItemData(comboBoxSide, newIndex, (void *)c);

		if (c == favSide)
			entryToSelect = newIndex;
	}
	seenSides.clear();

	GadgetComboBoxSetSelectedPos(comboBoxSide, entryToSelect);
	if (li && li->randomFactions)
		comboBoxSide->winEnable(FALSE);
	else
		comboBoxSide->winEnable(TRUE);
}

void HandleQMLadderSelection(Int ladderID)
{
	if (!parentWOLQuickMatch)
		return;

	QuickMatchPreferences pref;

	if (ladderID < 1)
	{
		pref.setLastLadder(AsciiString::TheEmptyString, 0);
		pref.write();
		return;
	}

	const LadderInfo *info = TheLadderList->findLadderByIndex(ladderID);
	if (!info)
	{
		pref.setLastLadder(AsciiString::TheEmptyString, 0);
	}
	else
	{
		pref.setLastLadder(info->address, info->port);
	}

	pref.write();
}

static inline Bool isValidLadder( const LadderInfo *lad )
{
	if (lad && lad->index > 0 && lad->validQM)
	{
		PSPlayerStats stats = TheGameSpyPSMessageQueue->findPlayerStatsByID(TheGameSpyInfo->getLocalProfileID());
		Int numWins = 0;
		PerGeneralMap::iterator it;
		for (it = stats.wins.begin(); it != stats.wins.end(); ++it)
		{
			numWins += it->second;
		}
		if (!lad->maxWins || lad->maxWins >=numWins)
		{
			if (!lad->minWins || lad->minWins<=numWins)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}

void PopulateQMLadderListBox( GameWindow *win )
{
	if (!parentWOLQuickMatch || !comboBoxLadder)
		return;

	isPopulatingLadderBox = true;

	QuickMatchPreferences pref;
	AsciiString userPrefFilename;
	Int localProfile = TheGameSpyInfo->getLocalProfileID();

	Color specialColor = GameSpyColor[GSCOLOR_MAP_SELECTED];
	Color normalColor = GameSpyColor[GSCOLOR_MAP_UNSELECTED];
	Color favoriteColor = GameSpyColor[GSCOLOR_MAP_UNSELECTED];
	Int index;
	GadgetListBoxReset( win );

	std::set<const LadderInfo *> usedLadders;

	// start with "No Ladder"
	index = GadgetListBoxAddEntryText( win, TheGameText->fetch("GUI:NoLadder"), normalColor, -1 );
	GadgetListBoxSetItemData( win, 0, index );

	// add the last ladder
	Int selectedPos = 0;
	AsciiString lastLadderAddr = pref.getLastLadderAddr();
	UnsignedShort lastLadderPort = pref.getLastLadderPort();
	const LadderInfo *info = TheLadderList->findLadder( lastLadderAddr, lastLadderPort );
	if (isValidLadder(info))
	{
		usedLadders.insert(info);
		index = GadgetListBoxAddEntryText( win, info->name, favoriteColor, -1 );
		GadgetListBoxSetItemData( win, (void *)(info->index), index );
		selectedPos = index;
	}

	// our recent ladders
	LadderPreferences ladPref;
	ladPref.loadProfile( localProfile );
	const LadderPrefMap recentLadders = ladPref.getRecentLadders();
	for (LadderPrefMap::const_iterator cit = recentLadders.begin(); cit != recentLadders.end(); ++cit)
	{
		AsciiString addr = cit->second.address;
		UnsignedShort port = cit->second.port;
		if (addr == lastLadderAddr && port == lastLadderPort)
			continue;
		const LadderInfo *info = TheLadderList->findLadder( addr, port );
		if (isValidLadder(info) && usedLadders.find(info) == usedLadders.end())
		{
			usedLadders.insert(info);
			index = GadgetListBoxAddEntryText( win, info->name, favoriteColor, -1 );
			GadgetListBoxSetItemData( win, (void *)(info->index), index );
		}
	}

	// special ladders
	const LadderInfoList *lil = TheLadderList->getSpecialLadders();
	LadderInfoList::const_iterator lit;
	for (lit = lil->begin(); lit != lil->end(); ++lit)
	{
		const LadderInfo *info = *lit;
		if (isValidLadder(info) && usedLadders.find(info) == usedLadders.end())
		{
			usedLadders.insert(info);
			index = GadgetListBoxAddEntryText( win, info->name, specialColor, -1 );
			GadgetListBoxSetItemData( win, (void *)(info->index), index );
		}
	}

	// standard ladders
	lil = TheLadderList->getStandardLadders();
	for (lit = lil->begin(); lit != lil->end(); ++lit)
	{
		const LadderInfo *info = *lit;
		if (isValidLadder(info) && usedLadders.find(info) == usedLadders.end())
		{
			usedLadders.insert(info);
			index = GadgetListBoxAddEntryText( win, info->name, normalColor, -1 );
			GadgetListBoxSetItemData( win, (void *)(info->index), index );
		}
	}

	GadgetListBoxSetSelected( win, selectedPos );
	isPopulatingLadderBox = false;
}

static const LadderInfo * getLadderInfo( void )
{
	Int index;
	Int selected;
	GadgetComboBoxGetSelectedPos( comboBoxLadder, &selected );
	index = (Int)GadgetComboBoxGetItemData( comboBoxLadder, selected );
	const LadderInfo *li = TheLadderList->findLadderByIndex( index );
	return li;
}

void PopulateQMLadderComboBox( void )
{
	if (!parentWOLQuickMatch || !comboBoxLadder)
		return;

	isPopulatingLadderBox = true;

	QuickMatchPreferences pref;
	Int localProfile = TheGameSpyInfo->getLocalProfileID();

	Color specialColor = GameSpyColor[GSCOLOR_MAP_SELECTED];
	Color normalColor = GameSpyColor[GSCOLOR_MAP_UNSELECTED];
	Int index;
	GadgetComboBoxReset( comboBoxLadder );
	index = GadgetComboBoxAddEntry( comboBoxLadder, TheGameText->fetch("GUI:NoLadder"), normalColor );
	GadgetComboBoxSetItemData( comboBoxLadder, index, 0 );

	std::set<const LadderInfo *> usedLadders;

	Int selectedPos = 0;
	AsciiString lastLadderAddr = pref.getLastLadderAddr();
	UnsignedShort lastLadderPort = pref.getLastLadderPort();
	const LadderInfo *info = TheLadderList->findLadder( lastLadderAddr, lastLadderPort );
	if (isValidLadder(info))
	{
		usedLadders.insert(info);
		index = GadgetComboBoxAddEntry( comboBoxLadder, info->name, specialColor );
		GadgetComboBoxSetItemData( comboBoxLadder, index, (void *)(info->index) );
		selectedPos = index;

		// we selected a ladder?  No game size choice for us...
		GadgetComboBoxSetSelectedPos(comboBoxNumPlayers, info->playersPerTeam-1);
		comboBoxNumPlayers->winEnable( FALSE );
	}
	else
	{
		comboBoxNumPlayers->winEnable( TRUE );
	}

	LadderPreferences ladPref;
	ladPref.loadProfile( localProfile );
	const LadderPrefMap recentLadders = ladPref.getRecentLadders();
	for (LadderPrefMap::const_iterator cit = recentLadders.begin(); cit != recentLadders.end(); ++cit)
	{
		AsciiString addr = cit->second.address;
		UnsignedShort port = cit->second.port;
		if (addr == lastLadderAddr && port == lastLadderPort)
			continue;
		const LadderInfo *info = TheLadderList->findLadder( addr, port );
		if (isValidLadder(info) && usedLadders.find(info) == usedLadders.end())
		{
			usedLadders.insert(info);
			index = GadgetComboBoxAddEntry( comboBoxLadder, info->name, normalColor );
			GadgetComboBoxSetItemData( comboBoxLadder, index, (void *)(info->index) );
		}
	}

	index = GadgetComboBoxAddEntry( comboBoxLadder, TheGameText->fetch("GUI:ChooseLadder"), normalColor );
	GadgetComboBoxSetItemData( comboBoxLadder, index, (void *)-1 );

	GadgetComboBoxSetSelectedPos( comboBoxLadder, selectedPos );
	isPopulatingLadderBox = false;

	populateQMSideComboBox(pref.getSide(), getLadderInfo()); // this will set side to random and disable if necessary
}

static void populateQuickMatchMapSelectListbox( QuickMatchPreferences& pref )
{
	std::list<AsciiString> maps = TheGameSpyConfig->getQMMaps();

	// enable/disable box based on ladder status
	Int index;
	Int selected;
	GadgetComboBoxGetSelectedPos( comboBoxLadder, &selected );
	index = (Int)GadgetComboBoxGetItemData( comboBoxLadder, selected );
	const LadderInfo *li = TheLadderList->findLadderByIndex( index );
	//listboxMapSelect->winEnable( li == NULL || li->randomMaps == FALSE );

	Int numPlayers = 0;
	if (li)
	{
		numPlayers = li->playersPerTeam*2;

		maps = li->validMaps;
	}
	else
	{
		GadgetComboBoxGetSelectedPos(comboBoxNumPlayers, &selected);
		if (selected < 0)
			selected = 0;
		numPlayers = (selected+1)*2;
	}

	#if VARIABLE_NUMBER_OF_MAPS
	mapListboxIndex.clear();
	#endif

	GadgetListBoxReset(listboxMapSelect);
	for (std::list<AsciiString>::const_iterator it = maps.begin(); it != maps.end(); ++it)
	{
		AsciiString theMap = *it;
		const MapMetaData *md = TheMapCache->findMap(theMap);
		if (md && md->m_numPlayers >= numPlayers)
		{
			UnicodeString displayName;
			displayName = md->m_displayName;
			Bool isSelected = pref.isMapSelected(theMap);
			if (li && li->randomMaps)
				isSelected = TRUE;
			Int width = 10;
			Int height = 10;
			const Image *img = (isSelected)?selectedImage:unselectedImage;
			if ( img )
			{
				width = min(GadgetListBoxGetColumnWidth(listboxMapSelect, 0), img->getImageWidth());
				height = width;
			}
			Int index = GadgetListBoxAddEntryImage(listboxMapSelect, img, -1, 0, height, width);
			GadgetListBoxAddEntryText(listboxMapSelect, displayName, GameSpyColor[(isSelected)?GSCOLOR_MAP_SELECTED:GSCOLOR_MAP_UNSELECTED], index, 1);
			GadgetListBoxSetItemData(listboxMapSelect, (void *)isSelected, index);
			GadgetListBoxSetItemData(listboxMapSelect, (void *)md, index, 1);

			#if VARIABLE_NUMBER_OF_MAPS
			mapListboxIndex.push_back(index);
			#endif
		} 
		else
		{
			#if VARIABLE_NUMBER_OF_MAPS
			// [SKB: Jul 01 2003 @ 7:9pm] :
			// Keep track of maps that are not visible right now so
			// they are added to the information sent to the QMBot.
			mapListboxIndex.push_back(-1);
			#endif
		}
	}
}

static void saveQuickMatchOptions( void )
{
	if(isInInit)
		return;
	QuickMatchPreferences pref;

	std::list<AsciiString> maps = TheGameSpyConfig->getQMMaps();

	Int index;
	Int selected;
	GadgetComboBoxGetSelectedPos( comboBoxLadder, &selected );
	index = (Int)GadgetComboBoxGetItemData( comboBoxLadder, selected );
	const LadderInfo *li = TheLadderList->findLadderByIndex( index );
	Int numPlayers = 0;

	if (li)
	{
		pref.setLastLadder( li->address, li->port );
		numPlayers = li->playersPerTeam*2;

		pref.write();
		//return; // don't save our defaults based on the tournament's defaults
	}
	else
	{
		pref.setLastLadder( AsciiString::TheEmptyString, 0 );
		GadgetComboBoxGetSelectedPos(comboBoxNumPlayers, &selected);
		if (selected < 0)
			selected = 0;
		numPlayers = (selected+1)*2;
	}

	if (!li || !li->randomMaps)  // don't save the map as selected if we couldn't choose
	{
		Int row = 0;
		Int entries = GadgetListBoxGetNumEntries(listboxMapSelect);
		while ( row < entries)
		{
			const MapMetaData *md = (const MapMetaData *)GadgetListBoxGetItemData(listboxMapSelect, row, 1);
			if(md)
				pref.setMapSelected(md->m_fileName, (Bool)GadgetListBoxGetItemData(listboxMapSelect, row));
			row++;
		}
	}

	UnicodeString u;
	AsciiString a;
//	u = GadgetTextEntryGetText(textEntryMaxDisconnects);
//	a.translate(u);
//	pref.setMaxDisconnects(atoi(a.str()));
//	u = GadgetTextEntryGetText(textEntryMaxPoints);
//	a.translate(u);
//	pref.setMaxPoints(max(100, atoi(a.str())));
//	u = GadgetTextEntryGetText(textEntryMinPoints);
//	a.translate(u);
//	pref.setMinPoints(min(100, atoi(a.str())));
	//u = GadgetTextEntryGetText(textEntryWaitTime);
	//a.translate(u);
	//pref.setWaitTime(atoi(a.str()));

	GadgetComboBoxGetSelectedPos(comboBoxNumPlayers, &selected);
	pref.setNumPlayers(selected);
	GadgetComboBoxGetSelectedPos(comboBoxMaxPing, &selected);
	pref.setMaxPing(selected);

	Int item;
	GadgetComboBoxGetSelectedPos(comboBoxSide, &selected);
	item = (Int)GadgetComboBoxGetItemData(comboBoxSide, selected);
	pref.setSide(max(0, item));
	GadgetComboBoxGetSelectedPos(comboBoxColor, &selected);
	pref.setColor(max(0, selected));

	GadgetComboBoxGetSelectedPos(comboBoxMaxDisconnects, &selected);
	pref.setMaxDisconnects(selected);


	pref.write();
}

//-------------------------------------------------------------------------------------------------
/** Initialize the WOL Quick Match Menu */
//-------------------------------------------------------------------------------------------------
void WOLQuickMatchMenuInit( WindowLayout *layout, void *userData )
{
	isInInit = TRUE;
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
			DEBUG_LOG(("WOLQuickMatchMenuInit() - game was in progress, and we were disconnected, so pop immediate back to main menu\n"));
			TheShell->popImmediate();
			return;
		}
	}

	nextScreen = NULL;
	buttonPushed = false;
	isShuttingDown = false;
	raiseMessageBoxes = true;

	if (TheNAT != NULL) {
		delete TheNAT;
		TheNAT = NULL;
	}

	parentWOLQuickMatchID = NAMEKEY( "WOLQuickMatchMenu.wnd:WOLQuickMatchMenuParent" );
	buttonBackID = NAMEKEY( "WOLQuickMatchMenu.wnd:ButtonBack" );
	buttonBuddiesID = NAMEKEY( "WOLQuickMatchMenu.wnd:ButtonBuddies" );
	buttonStartID = NAMEKEY( "WOLQuickMatchMenu.wnd:ButtonStart" );
	buttonStopID = NAMEKEY( "WOLQuickMatchMenu.wnd:ButtonStop" );
	buttonWidenID = NAMEKEY( "WOLQuickMatchMenu.wnd:ButtonWiden" );
	listboxQuickMatchID = NAMEKEY( "WOLQuickMatchMenu.wnd:ListboxQuickMatch" );
	listboxMapSelectID = NAMEKEY( "WOLQuickMatchMenu.wnd:ListBoxMapSelect" );
	buttonSelectAllMapsID = NAMEKEY( "WOLQuickMatchMenu.wnd:ButtonSelectAllMaps" );
	buttonSelectNoMapsID = NAMEKEY( "WOLQuickMatchMenu.wnd:ButtonSelectNoMaps" );
	//textEntryMaxDisconnectsID = NAMEKEY( "WOLQuickMatchMenu.wnd:TextEntryMaxDisconnects" );
	//textEntryMaxPointsID = NAMEKEY( "WOLQuickMatchMenu.wnd:TextEntryMaxPointPercent" );
	//textEntryMinPointsID = NAMEKEY( "WOLQuickMatchMenu.wnd:TextEntryMinPointPercent" );
	textEntryWaitTimeID = NAMEKEY( "WOLQuickMatchMenu.wnd:TextEntryWaitTime" );
	comboBoxMaxPingID = NAMEKEY( "WOLQuickMatchMenu.wnd:ComboBoxMaxPing" );
	comboBoxNumPlayersID = NAMEKEY( "WOLQuickMatchMenu.wnd:ComboBoxNumPlayers" );
	comboBoxLadderID = NAMEKEY( "WOLQuickMatchMenu.wnd:ComboBoxLadder" );
	comboBoxMaxDisconnectsID = NAMEKEY( "WOLQuickMatchMenu.wnd:ComboBoxMaxDisconnects" );
	staticTextNumPlayersID = NAMEKEY( "WOLQuickMatchMenu.wnd:StaticTextNumPlayers" );
	comboBoxSideID = NAMEKEY( "WOLQuickMatchMenu.wnd:ComboBoxSide" );
	comboBoxColorID = NAMEKEY( "WOLQuickMatchMenu.wnd:ComboBoxColor" );

	parentWOLQuickMatch = TheWindowManager->winGetWindowFromId( NULL, parentWOLQuickMatchID );
	buttonBack = TheWindowManager->winGetWindowFromId( parentWOLQuickMatch,  buttonBackID);
	buttonStart = TheWindowManager->winGetWindowFromId( parentWOLQuickMatch,  buttonStartID);
	buttonStop = TheWindowManager->winGetWindowFromId( parentWOLQuickMatch,  buttonStopID);
	buttonWiden = TheWindowManager->winGetWindowFromId( parentWOLQuickMatch,  buttonWidenID);
	quickmatchTextWindow = TheWindowManager->winGetWindowFromId( parentWOLQuickMatch,  listboxQuickMatchID);
	listboxMapSelect = TheWindowManager->winGetWindowFromId( parentWOLQuickMatch,  listboxMapSelectID);
	//textEntryMaxDisconnects = TheWindowManager->winGetWindowFromId( parentWOLQuickMatch, textEntryMaxDisconnectsID );
	//textEntryMaxPoints = TheWindowManager->winGetWindowFromId( parentWOLQuickMatch, textEntryMaxPointsID );
	//textEntryMinPoints = TheWindowManager->winGetWindowFromId( parentWOLQuickMatch, textEntryMinPointsID );
	textEntryWaitTime = TheWindowManager->winGetWindowFromId( parentWOLQuickMatch, textEntryWaitTimeID );
	comboBoxMaxPing = TheWindowManager->winGetWindowFromId( parentWOLQuickMatch, comboBoxMaxPingID );
	comboBoxNumPlayers = TheWindowManager->winGetWindowFromId( parentWOLQuickMatch, comboBoxNumPlayersID );
	comboBoxLadder = TheWindowManager->winGetWindowFromId( parentWOLQuickMatch, comboBoxLadderID );
	comboBoxMaxDisconnects = TheWindowManager->winGetWindowFromId( parentWOLQuickMatch, comboBoxMaxDisconnectsID );
	TheGameSpyInfo->registerTextWindow(quickmatchTextWindow);
	staticTextNumPlayers = TheWindowManager->winGetWindowFromId( parentWOLQuickMatch, staticTextNumPlayersID );
	comboBoxSide = TheWindowManager->winGetWindowFromId( parentWOLQuickMatch, comboBoxSideID );
	comboBoxColor = TheWindowManager->winGetWindowFromId( parentWOLQuickMatch, comboBoxColorID );

	if (TheLadderList->getStandardLadders()->size() == 0
		&& TheLadderList->getSpecialLadders()->size() == 0
		&& TheLadderList->getLocalLadders()->size() == 0)
	{
		// no ladders, so just disable them
		comboBoxDisabledLadder = comboBoxLadder;
		comboBoxLadder = NULL;

		isPopulatingLadderBox = TRUE;

		Color normalColor = GameSpyColor[GSCOLOR_MAP_UNSELECTED];
		Int index;
		GadgetComboBoxReset( comboBoxDisabledLadder );
		index = GadgetComboBoxAddEntry( comboBoxDisabledLadder, TheGameText->fetch("GUI:NoLadder"), normalColor );
		GadgetComboBoxSetItemData( comboBoxDisabledLadder, index, 0 );
		GadgetComboBoxSetSelectedPos( comboBoxDisabledLadder, index );

		isPopulatingLadderBox = FALSE;

		/** This code would actually *hide* the combo box, but it doesn't look as good.  Left here since someone will want to
		 ** see it at some point.  :P
		if (comboBoxLadder)
		{
			comboBoxLadder->winHide(TRUE);
			comboBoxLadder->winEnable(FALSE);
		}
		comboBoxLadder = NULL;
		GameWindow *staticTextLadder = TheWindowManager->winGetWindowFromId( parentWOLQuickMatch,
			NAMEKEY("WOLQuickMatchMenu.wnd:StaticTextLadder") );
		if (staticTextLadder)
			staticTextLadder->winHide(TRUE);
		*/
	}

	GameWindow *buttonBuddies = TheWindowManager->winGetWindowFromId(NULL, buttonBuddiesID);
	if (buttonBuddies)
		buttonBuddies->winEnable(TRUE);

	GameWindow *staticTextTitle = TheWindowManager->winGetWindowFromId( parentWOLQuickMatch,
		NAMEKEY("WOLQuickMatchMenu.wnd:StaticTextTitle") );
	if (staticTextTitle)
	{
		UnicodeString tmp;
		tmp.format(TheGameText->fetch("GUI:QuickMatchTitle"), TheGameSpyInfo->getLocalName().str());
		GadgetStaticTextSetText(staticTextTitle, tmp);
	}

	// QM is not going yet, so disable the Widen Search button
	buttonWiden->winEnable( FALSE );
	buttonStop->winHide( TRUE );
	buttonStart->winHide( FALSE );
	GadgetListBoxReset(quickmatchTextWindow);
	enableOptionsGadgets(TRUE);

	// Show Menu
	layout->hide( FALSE );

	// Set Keyboard to Main Parent
	TheWindowManager->winSetFocus( parentWOLQuickMatch );

	// fill in preferences
	selectedImage = TheMappedImageCollection->findImageByName("CustomMatch_selected");
	unselectedImage = TheMappedImageCollection->findImageByName("CustomMatch_deselected");
	QuickMatchPreferences pref;


	UnicodeString s;
//	s.format(L"%d", pref.getMaxDisconnects());
//	GadgetTextEntrySetText(textEntryMaxDisconnects, s);
//	s.format(L"%d", pref.getMaxPoints());
//	GadgetTextEntrySetText(textEntryMaxPoints, s);
//	s.format(L"%d", pref.getMinPoints());
//	GadgetTextEntrySetText(textEntryMinPoints, s);
	//s.format(L"%d", pref.getWaitTime());
	//GadgetTextEntrySetText(textEntryWaitTime, s);
	maxPoints= pref.getMaxPoints();
	minPoints = pref.getMinPoints();

	Color c = GameSpyColor[GSCOLOR_DEFAULT];
	GadgetComboBoxReset( comboBoxNumPlayers );
	Int i;
	for (i=1; i<5; ++i)
	{
		s.format(TheGameText->fetch("GUI:PlayersVersusPlayers"), i, i);
		GadgetComboBoxAddEntry( comboBoxNumPlayers, s, c );
	}
	GadgetComboBoxSetSelectedPos( comboBoxNumPlayers, max(0, pref.getNumPlayers()) );

	GadgetComboBoxReset(comboBoxMaxDisconnects);
	GadgetComboBoxAddEntry( comboBoxMaxDisconnects, TheGameText->fetch("GUI:Any"), c);
	for( i = 1; i < MAX_DISCONNECTS_COUNT; ++i )
	{
		s.format(L"%d", MAX_DISCONNECTS[i]);
		GadgetComboBoxAddEntry( comboBoxMaxDisconnects, s, c );
	}
	Int maxDisconIndex = max(0, pref.getMaxDisconnects());
	GadgetComboBoxSetSelectedPos(comboBoxMaxDisconnects, maxDisconIndex);

	GadgetComboBoxReset( comboBoxMaxPing );
	maxPingEntries = (TheGameSpyConfig->getPingTimeoutInMs() - 1) / 100;
	maxPingEntries++; // need to add the entry for the actual timeout
	for (i=1; i <maxPingEntries; ++i)
	{
		s.format(TheGameText->fetch("GUI:TimeInMilliseconds"), i*100);
		GadgetComboBoxAddEntry( comboBoxMaxPing, s, c );
	}
	GadgetComboBoxAddEntry( comboBoxMaxPing, TheGameText->fetch("GUI:ANY"), c );
	i = pref.getMaxPing();
	if( i < 0 )
		i = 0;
	if( i >= maxPingEntries )
		i = maxPingEntries - 1;
	GadgetComboBoxSetSelectedPos( comboBoxMaxPing, i );

	populateQMColorComboBox(pref);
	populateQMSideComboBox(pref.getSide(), getLadderInfo());

	PopulateQMLadderComboBox();
	TheShell->showShellMap(TRUE);
	TheGameSpyGame->reset();

	GadgetListBoxReset(listboxMapSelect);
	populateQuickMatchMapSelectListbox(pref);

	UpdateLocalPlayerStats();
	UpdateStartButton();
	TheTransitionHandler->setGroup("WOLQuickMatchMenuFade");
	isInInit= FALSE;
} // WOLQuickMatchMenuInit

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
		TheShell->push(nextScreen);
	}

	nextScreen = NULL;

}  // end if

//-------------------------------------------------------------------------------------------------
/** WOL Quick Match Menu shutdown method */
//-------------------------------------------------------------------------------------------------
void WOLQuickMatchMenuShutdown( WindowLayout *layout, void *userData )
{
	TheGameSpyInfo->unregisterTextWindow(quickmatchTextWindow);

	if (!TheGameEngine->getQuitting())
		saveQuickMatchOptions();

	parentWOLQuickMatch = NULL;
	buttonBack = NULL;
	quickmatchTextWindow = NULL;
	selectedImage = unselectedImage = NULL;

	isShuttingDown = true;

	// if we are shutting down for an immediate pop, skip the animations
	Bool popImmediate = *(Bool *)userData;
	if( popImmediate )
	{

		shutdownComplete( layout );
		return;

	}  //end if

	TheShell->reverseAnimatewindow();
	TheTransitionHandler->reverse("WOLQuickMatchMenuFade");

	RaiseGSMessageBox();
}  // WOLQuickMatchMenuShutdown


#ifdef PERF_TEST
static const char* getMessageString(Int t)
{
	switch(t)
	{
		case PeerResponse::PEERRESPONSE_LOGIN:
			return "login";
		case PeerResponse::PEERRESPONSE_DISCONNECT:
			return "disconnect";
		case PeerResponse::PEERRESPONSE_MESSAGE:
			return "message";
		case PeerResponse::PEERRESPONSE_GROUPROOM:
			return "group room";
		case PeerResponse::PEERRESPONSE_STAGINGROOM:
			return "staging room";
		case PeerResponse::PEERRESPONSE_STAGINGROOMPLAYERINFO:
			return "staging room player info";
		case PeerResponse::PEERRESPONSE_JOINGROUPROOM:
			return "group room join";
		case PeerResponse::PEERRESPONSE_CREATESTAGINGROOM:
			return "staging room create";
		case PeerResponse::PEERRESPONSE_JOINSTAGINGROOM:
			return "staging room join";
		case PeerResponse::PEERRESPONSE_PLAYERJOIN:
			return "player join";
		case PeerResponse::PEERRESPONSE_PLAYERLEFT:
			return "player part";
		case PeerResponse::PEERRESPONSE_PLAYERCHANGEDNICK:
			return "player nick";
		case PeerResponse::PEERRESPONSE_PLAYERINFO:
			return "player info";
		case PeerResponse::PEERRESPONSE_PLAYERCHANGEDFLAGS:
			return "player flags";
		case PeerResponse::PEERRESPONSE_ROOMUTM:
			return "room UTM";
		case PeerResponse::PEERRESPONSE_PLAYERUTM:
			return "player UTM";
		case PeerResponse::PEERRESPONSE_QUICKMATCHSTATUS:
			return "QM status";
		case PeerResponse::PEERRESPONSE_GAMESTART:
			return "game start";
		case PeerResponse::PEERRESPONSE_FAILEDTOHOST:
			return "host failure";
	}
	return "unknown";
}
#endif // PERF_TEST

//-------------------------------------------------------------------------------------------------
/** WOL Quick Match Menu update method */
//-------------------------------------------------------------------------------------------------
void WOLQuickMatchMenuUpdate( WindowLayout * layout, void *userData)
{
	if (TheGameLogic->isInShellGame() && TheGameLogic->getFrame() == 1)
	{
		SignalUIInteraction(SHELL_SCRIPT_HOOK_GENERALS_ONLINE_ENTERED_FROM_GAME);
	}

	// We'll only be successful if we've requested to 
	if(isShuttingDown && TheShell->isAnimFinished()&& TheTransitionHandler->isFinished())
		shutdownComplete(layout);

	if (raiseMessageBoxes)
	{
		RaiseGSMessageBox();
		raiseMessageBoxes = false;
	}
	
	/// @todo: MDC handle disconnects in-game the same way as Custom Match!

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

		if (TheNAT != NULL) {
			NATStateType NATState = TheNAT->update();
			if (NATState == NATSTATE_DONE)
			{
				TheGameSpyGame->launchGame();
				if (TheGameSpyInfo) // this can be blown away by a disconnect on the map transfer screen
					TheGameSpyInfo->leaveStagingRoom();
				return; // don't do any more processing this frame, in case the screen goes away
			}
			else if (NATState == NATSTATE_FAILED)
			{
				// delete TheNAT, its no good for us anymore.
				delete TheNAT;
				TheNAT = NULL;

				// Just back out.  This cleans up some slot list problems
				buttonPushed = true;
				GSMessageBoxOk(TheGameText->fetch("GUI:Error"), TheGameText->fetch("GUI:NATNegotiationFailed"));
				nextScreen = "Menus/WOLWelcomeMenu.wnd";
				TheShell->pop();
				return; // don't do any more processing this frame, in case the screen goes away
			}
		}

#ifdef PERF_TEST
		UnsignedInt start = timeGetTime();
		UnsignedInt end = timeGetTime();
		std::list<Int> responses;
		Int numMessages = 0;
#endif // PERF_TEST

		Int allowedMessages = TheGameSpyInfo->getMaxMessagesPerUpdate();
		Bool sawImportantMessage = FALSE;
		PeerResponse resp;
		while (allowedMessages-- && !sawImportantMessage && TheGameSpyPeerMessageQueue->getResponse( resp ))
		{
#ifdef PERF_TEST
			++numMessages;
			responses.push_back(resp.peerResponseType);
#endif // PERF_TEST
			switch (resp.peerResponseType)
			{
			case PeerResponse::PEERRESPONSE_PLAYERUTM:
				{
					if (!stricmp(resp.command.c_str(), "STATS"))
					{
						DEBUG_LOG(("Saw STATS from %s, data was '%s'\n", resp.nick.c_str(), resp.commandOptions.c_str()));
						AsciiString data = resp.commandOptions.c_str();
						AsciiString idStr;
						data.nextToken(&idStr, " ");
						Int id = atoi(idStr.str());
						DEBUG_LOG(("data: %d(%s) - '%s'\n", id, idStr.str(), data.str()));

						PSPlayerStats stats = TheGameSpyPSMessageQueue->parsePlayerKVPairs(data.str());
						PSPlayerStats oldStats = TheGameSpyPSMessageQueue->findPlayerStatsByID(id);
						stats.id = id;
						DEBUG_LOG(("Parsed ID is %d, old ID is %d\n", stats.id, oldStats.id));
						if (stats.id && (oldStats.id == 0))
							TheGameSpyPSMessageQueue->trackPlayerStats(stats);

						// now fill in the profileID in the game slot
						AsciiString nick = resp.nick.c_str();
						for (Int i=0; i<MAX_SLOTS; ++i)
						{
							GameSpyGameSlot *slot = TheGameSpyGame->getGameSpySlot(i);
							if (slot && slot->isHuman() && (slot->getLoginName().compareNoCase(nick) == 0))
							{
								slot->setProfileID(id);
								break;
							}
						}
					}
					Int slotNum = TheGameSpyGame->getSlotNum(resp.nick.c_str());
					if ((slotNum >= 0) && (slotNum < MAX_SLOTS) && (!stricmp(resp.command.c_str(), "NAT"))) {
						// this is a command for NAT negotiations, pass if off to TheNAT
						sawImportantMessage = TRUE;
						if (TheNAT != NULL) {
							TheNAT->processGlobalMessage(slotNum, resp.commandOptions.c_str());
						}
					}
					/*
					else if (key == "NAT")
					{
						if ((val >= FirewallHelperClass::FIREWALL_TYPE_SIMPLE) &&
								(val <= FirewallHelperClass::FIREWALL_TYPE_DESTINATION_PORT_DELTA))
						{
							slot->setNATBehavior((FirewallHelperClass::FirewallBehaviorType)val);
							DEBUG_LOG(("Setting NAT behavior to %d for player %d\n", val, slotNum));
							change = true;
						}
						else
						{
							DEBUG_LOG(("Rejecting invalid NAT behavior %d from player %d\n", val, slotNum));
						}
					}
					*/
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
			case PeerResponse::PEERRESPONSE_QUICKMATCHSTATUS:
				{
					sawImportantMessage = TRUE;
					switch( resp.qmStatus.status )
					{
					case QM_IDLE:
						//TheGameSpyInfo->addText(UnicodeString(L"Status: QM_IDLE"), GameSpyColor[GSCOLOR_DEFAULT], quickmatchTextWindow);
						break;
					case QM_JOININGQMCHANNEL:
						TheGameSpyInfo->addText(TheGameText->fetch("QM:JOININGQMCHANNEL"), GameSpyColor[GSCOLOR_DEFAULT], quickmatchTextWindow);
						break;
					case QM_LOOKINGFORBOT:
						TheGameSpyInfo->addText(TheGameText->fetch("QM:LOOKINGFORBOT"), GameSpyColor[GSCOLOR_DEFAULT], quickmatchTextWindow);
						break;
					case QM_SENTINFO:
						TheGameSpyInfo->addText(TheGameText->fetch("QM:SENTINFO"), GameSpyColor[GSCOLOR_DEFAULT], quickmatchTextWindow);
						break;
					case QM_WORKING:
						{
							UnicodeString s;
							s.format(TheGameText->fetch("QM:WORKING"), resp.qmStatus.poolSize);
							TheGameSpyInfo->addText(s, GameSpyColor[GSCOLOR_DEFAULT], quickmatchTextWindow);
						}
						buttonWiden->winEnable( TRUE );
						break;
					case QM_POOLSIZE:
						{
							UnicodeString s;
							s.format(TheGameText->fetch("QM:POOLSIZE"), resp.qmStatus.poolSize);
							TheGameSpyInfo->addText(s, GameSpyColor[GSCOLOR_DEFAULT], quickmatchTextWindow);
						}
						break;
					case QM_WIDENINGSEARCH:
						TheGameSpyInfo->addText(TheGameText->fetch("QM:WIDENINGSEARCH"), GameSpyColor[GSCOLOR_DEFAULT], quickmatchTextWindow);
						buttonWiden->winEnable( FALSE );
						break;
					case QM_MATCHED:
						{
							TheGameSpyInfo->addText(TheGameText->fetch("QM:MATCHED"), GameSpyColor[GSCOLOR_DEFAULT], quickmatchTextWindow);
							buttonWiden->winEnable( FALSE );

							TheGameSpyGame->enterGame();
							TheGameSpyGame->setSeed(resp.qmStatus.seed);

							TheGameSpyGame->markGameAsQM();

							const LadderInfo *info = getLadderInfo();
							if (!info)
							{
								TheGameSpyGame->setLadderIP("localhost");
								TheGameSpyGame->setLadderPort(0);
							}
							else
							{
								TheGameSpyGame->setLadderIP(info->address);
								TheGameSpyGame->setLadderPort(info->port);
							}

							Int i;
							Int numPlayers = 0;
							for (i=0; i<MAX_SLOTS; ++i)
							{
								if (!resp.stagingRoomPlayerNames[i].empty())
									++numPlayers;
							}

							std::list<AsciiString> maps = TheGameSpyConfig->getQMMaps();

							#if VARIABLE_NUMBER_OF_MAPS
							std::list<AsciiString>::const_iterator it = maps.begin();
							std::advance(it, resp.qmStatus.mapIdx);

							AsciiString theMap = *it;
							theMap.toLower();
							TheGameSpyGame->setMap(theMap);

							#else
							for (std::list<AsciiString>::const_iterator it = maps.begin(); it != maps.end(); ++it)
							{
								AsciiString theMap = *it;
								theMap.toLower();
								const MapMetaData *md = TheMapCache->findMap(theMap);

								if (md && md->m_numPlayers >= numPlayers)
								{
									TheGameSpyGame->setMap(*it);
									if (resp.qmStatus.mapIdx-- == 0)
										break;
								}
							}
							#endif

							Int numPlayersPerTeam = numPlayers/2;
							DEBUG_ASSERTCRASH(numPlayersPerTeam, ("0 players per team???"));
							if (!numPlayersPerTeam)
								numPlayersPerTeam = 1;

							for (i=0; i<MAX_SLOTS; ++i)
							{
								GameSpyGameSlot *slot = TheGameSpyGame->getGameSpySlot(i);
								if (resp.stagingRoomPlayerNames[i].empty())
								{
									slot->setState(SLOT_CLOSED);
								}
								else
								{
									AsciiString aName = resp.stagingRoomPlayerNames[i].c_str();
									UnicodeString uName;
									uName.translate(aName);
									slot->setState(SLOT_PLAYER, uName, resp.qmStatus.IP[i]);
									slot->setColor(resp.qmStatus.color[i]);
									slot->setPlayerTemplate(resp.qmStatus.side[i]);
									//slot->setProfileID(0);
									slot->setNATBehavior((FirewallHelperClass::FirewallBehaviorType)resp.qmStatus.nat[i]);
									slot->setLocale("");
									slot->setTeamNumber( i/numPlayersPerTeam );
									if (i==0)
										TheGameSpyGame->setGameName(uName);
								}
							}

							DEBUG_LOG(("Starting a QM game: options=[%s]\n", GameInfoToAsciiString(TheGameSpyGame).str()));
							SendStatsToOtherPlayers(TheGameSpyGame);
							TheGameSpyGame->startGame(0);
							GameWindow *buttonBuddies = TheWindowManager->winGetWindowFromId(NULL, buttonBuddiesID);
							if (buttonBuddies)
								buttonBuddies->winEnable(FALSE);
							GameSpyCloseOverlay(GSOVERLAY_BUDDY);
						}
						break;
					case QM_INCHANNEL:
						TheGameSpyInfo->addText(TheGameText->fetch("QM:INCHANNEL"), GameSpyColor[GSCOLOR_DEFAULT], quickmatchTextWindow);
						break;
					case QM_NEGOTIATINGFIREWALLS:
						TheGameSpyInfo->addText(TheGameText->fetch("QM:NEGOTIATINGFIREWALLS"), GameSpyColor[GSCOLOR_DEFAULT], quickmatchTextWindow);
						break;
					case QM_STARTINGGAME:
						TheGameSpyInfo->addText(TheGameText->fetch("QM:STARTINGGAME"), GameSpyColor[GSCOLOR_DEFAULT], quickmatchTextWindow);
						break;
					case QM_COULDNOTFINDBOT:
						TheGameSpyInfo->addText(TheGameText->fetch("QM:COULDNOTFINDBOT"), GameSpyColor[GSCOLOR_DEFAULT], quickmatchTextWindow);
						buttonWiden->winEnable( FALSE );
						buttonStart->winHide( FALSE );
						buttonStop->winHide( TRUE );
						enableOptionsGadgets(TRUE);
						break;
					case QM_COULDNOTFINDCHANNEL:
						TheGameSpyInfo->addText(TheGameText->fetch("QM:COULDNOTFINDCHANNEL"), GameSpyColor[GSCOLOR_DEFAULT], quickmatchTextWindow);
						buttonWiden->winEnable( FALSE );
						buttonStart->winHide( FALSE );
						buttonStop->winHide( TRUE );
						enableOptionsGadgets(TRUE);
						break;
					case QM_COULDNOTNEGOTIATEFIREWALLS:
						TheGameSpyInfo->addText(TheGameText->fetch("QM:COULDNOTNEGOTIATEFIREWALLS"), GameSpyColor[GSCOLOR_DEFAULT], quickmatchTextWindow);
						buttonWiden->winEnable( FALSE );
						buttonStart->winHide( FALSE );
						buttonStop->winHide( TRUE );
						enableOptionsGadgets(TRUE);
						break;
					case QM_STOPPED:
						TheGameSpyInfo->addText(TheGameText->fetch("QM:STOPPED"), GameSpyColor[GSCOLOR_DEFAULT], quickmatchTextWindow);
						buttonWiden->winEnable( FALSE );
						buttonStart->winHide( FALSE );
						buttonStop->winHide( TRUE );
						enableOptionsGadgets(TRUE);
						break;
					}
				}
				break;
			}
		}
#ifdef PERF_TEST
		// check performance
		end = timeGetTime();
		UnsignedInt frameTime = end-start;
		if (frameTime > 100 || responses.size() > 20)
		{
			UnicodeString munkee;
			munkee.format(L"inQM:%d %d ms, %d messages", s_inQM, frameTime, responses.size());
			TheGameSpyInfo->addText(munkee, GameSpyColor[GSCOLOR_DEFAULT], quickmatchTextWindow);
			PERF_LOG(("%ls\n", munkee.str()));

			std::list<Int>::const_iterator it;
			for (it = responses.begin(); it != responses.end(); ++it)
			{
				PERF_LOG(("  %s\n", getMessageString(*it)));
			}
		}
#endif // PERF_TEST
	}
}// WOLQuickMatchMenuUpdate

//-------------------------------------------------------------------------------------------------
/** WOL Quick Match Menu input callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType WOLQuickMatchMenuInput( GameWindow *window, UnsignedInt msg,
																			 WindowMsgData mData1, WindowMsgData mData2 )
{
	switch( msg ) 
	{

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
						if(!buttonBack->winIsHidden())
							TheWindowManager->winSendSystemMsg( window, GBM_SELECTED, 
																							(WindowMsgData)buttonBack, buttonBackID );

					}  // end if

					// don't let key fall through anywhere else
					return MSG_HANDLED;

				}  // end escape

			}  // end switch( key )

		}  // end char

	}  // end switch( msg )

	return MSG_IGNORED;
}// WOLQuickMatchMenuInput

//-------------------------------------------------------------------------------------------------
/** WOL Quick Match Menu window system callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType WOLQuickMatchMenuSystem( GameWindow *window, UnsignedInt msg, 
														 WindowMsgData mData1, WindowMsgData mData2 )
{
	UnicodeString txtInput;

	switch( msg )
	{
		
		
		case GWM_CREATE:
			{
				
				break;
			} // case GWM_DESTROY:

		case GWM_DESTROY:
			{
				break;
			} // case GWM_DESTROY:

		case GWM_INPUT_FOCUS:
			{	
				// if we're givin the opportunity to take the keyboard focus we must say we want it
				if( mData1 == TRUE )
					*(Bool *)mData2 = TRUE;

				return MSG_HANDLED;
			}//case GWM_INPUT_FOCUS:

		case GCM_SELECTED:
			{
				if (buttonPushed)
					break;
				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();
				Int pos = -1;
				GadgetComboBoxGetSelectedPos(control, &pos);

				saveQuickMatchOptions();
				if (controlID == comboBoxLadderID && !isPopulatingLadderBox)
				{
					if (pos >= 0)
					{
						QuickMatchPreferences pref;
						Int ladderID = (Int)GadgetComboBoxGetItemData(control, pos);
						if (ladderID == 0)
						{
							// no ladder selected - enable buttons
							GadgetComboBoxSetSelectedPos(comboBoxNumPlayers, max(0, pref.getNumPlayers()/2-1));
							comboBoxNumPlayers->winEnable( TRUE );
							populateQMSideComboBox(pref.getSide()); // this will set side to random and disable if necessary
						}
						else if (ladderID > 0)
						{
							// ladder selected - disable buttons
							const LadderInfo *li = TheLadderList->findLadderByIndex(ladderID);
							if (li)
								GadgetComboBoxSetSelectedPos(comboBoxNumPlayers, li->playersPerTeam-1);
							else
								GadgetComboBoxSetSelectedPos(comboBoxNumPlayers, 0);
							comboBoxNumPlayers->winEnable( FALSE );

							populateQMSideComboBox(pref.getSide(), li); // this will set side to random and disable if necessary
						}
						else
						{
							// "Choose a ladder" selected - open overlay
							PopulateQMLadderComboBox(); // this restores the non-"Choose a ladder" selection
							GameSpyOpenOverlay( GSOVERLAY_LADDERSELECT );
						}
					}
				}
				if (!isInInit)
				{
					QuickMatchPreferences pref;
					populateQuickMatchMapSelectListbox(pref);
					UpdateStartButton();
				}
				break;
			} // case GCM_SELECTED

		case GBM_SELECTED:
			{
				if (buttonPushed)
					break;
				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();
				static NameKeyType buttonOptionsID = NAMEKEY("WOLQuickMatchMenu.wnd:ButtonOptions");

				if ( controlID == buttonStopID )
				{
					PeerRequest req;
					req.peerRequestType = PeerRequest::PEERREQUEST_STOPQUICKMATCH;
					TheGameSpyPeerMessageQueue->addRequest(req);
					buttonWiden->winEnable( FALSE );
					buttonStart->winHide( FALSE );
					buttonStop->winHide( TRUE );
					enableOptionsGadgets(TRUE);
					TheGameSpyInfo->addText(TheGameText->fetch("GUI:QMAborted"), GameSpyColor[GSCOLOR_DEFAULT], quickmatchTextWindow);
				}
				else if ( controlID == buttonOptionsID )
				{
					GameWindow *win =TheWindowManager->winGetWindowFromId(parentWOLQuickMatch,buttonOptionsID);
					if (isInfoShown())
					{
						hideInfoGadgets(TRUE);
						hideOptionsGadgets(FALSE);
						GadgetButtonSetText(win, TheGameText->fetch("GUI:PlayerInfo"));
					}
					else
					{
						hideInfoGadgets(FALSE);
						hideOptionsGadgets(TRUE);
						GadgetButtonSetText(win, TheGameText->fetch("GUI:Setup"));
					}
				}
				else if ( controlID == buttonWidenID )
				{
					PeerRequest req;
					req.peerRequestType = PeerRequest::PEERREQUEST_WIDENQUICKMATCHSEARCH;
					TheGameSpyPeerMessageQueue->addRequest(req);
					buttonWiden->winEnable( FALSE );
				}
				else if ( controlID == buttonStartID )
				{
					PeerRequest req;
					req.peerRequestType = PeerRequest::PEERREQUEST_STARTQUICKMATCH;
					req.qmMaps.clear();

					#if VARIABLE_NUMBER_OF_MAPS
					for (MapListboxIndex::iterator idxIt = mapListboxIndex.begin(); idxIt != mapListboxIndex.end(); ++idxIt) {
						Int index = (*idxIt);
						if (index >= 0)
						{
							req.qmMaps.push_back(GadgetListBoxGetItemData(listboxMapSelect, index, 0));
						} 
						else
						{
							req.qmMaps.push_back(false);
						}
					}
					#else 
					Int numMaps = GadgetListBoxGetNumEntries(listboxMapSelect);
					for ( Int i=0; i<numMaps; ++i )
					{
						req.qmMaps.push_back(GadgetListBoxGetItemData(listboxMapSelect, i, 0));
					}
					#endif
					
					UnicodeString u;
					AsciiString a;
//					u = GadgetTextEntryGetText(textEntryMaxDisconnects);
//					a.translate(u);
//					req.QM.maxDiscons = atoi(a.str());
//					u = GadgetTextEntryGetText(textEntryMaxPoints);
//					a.translate(u);
					req.QM.maxPointPercentage = max(100, maxPoints);
//					u = GadgetTextEntryGetText(textEntryMinPoints);
//					a.translate(u);
					req.QM.minPointPercentage = min(100, minPoints);
					//u = GadgetTextEntryGetText(textEntryWaitTime);
					//a.translate(u);
					//req.QM.widenTime = atoi(a.str());
					req.QM.widenTime = 0;

					Int val;
					GadgetComboBoxGetSelectedPos(comboBoxMaxDisconnects, &val);
					if( val < 0)
						 val = 0;
					req.QM.maxDiscons = MAX_DISCONNECTS[val];

					GadgetComboBoxGetSelectedPos(comboBoxMaxPing, &val);
					if (val < 0)
						val = 0;
					if( val >= maxPingEntries - 1)
					{
						req.QM.maxPing = TheGameSpyConfig->getPingTimeoutInMs();	
					}
					else
						req.QM.maxPing = (val+1)*100;

					PSPlayerStats stats = TheGameSpyPSMessageQueue->findPlayerStatsByID(TheGameSpyInfo->getLocalProfileID());
					req.QM.points = CalculateRank(stats);

					Int ladderIndex, index, selected;
					GadgetComboBoxGetSelectedPos( comboBoxLadder, &selected );
					ladderIndex = (Int)GadgetComboBoxGetItemData( comboBoxLadder, selected );
					const LadderInfo *ladderInfo = NULL;
					if (ladderIndex < 0)
					{
						ladderIndex = 0;
					}
					if (ladderIndex)
					{
						ladderInfo = TheLadderList->findLadderByIndex( ladderIndex );
						if (!ladderInfo)
						{
							ladderIndex = 0; // sanity
						}
					}
					req.QM.ladderID = ladderIndex;

					req.QM.ladderPassCRC = 0;

					index = -1;
					GadgetComboBoxGetSelectedPos( comboBoxSide, &selected );
					if (selected >= 0)
						index = (Int)GadgetComboBoxGetItemData( comboBoxSide, selected );
					req.QM.side = index;
					if (ladderInfo && ladderInfo->randomFactions)
					{
						Int sideNum = GameClientRandomValue(0, ladderInfo->validFactions.size()-1);
						DEBUG_LOG(("Looking for %d out of %d random sides\n", sideNum, ladderInfo->validFactions.size()));
						AsciiStringListConstIterator cit = ladderInfo->validFactions.begin();
						while (sideNum)
						{
							++cit;
							--sideNum;
						}
						if (cit != ladderInfo->validFactions.end())
						{
							Int numPlayerTemplates = ThePlayerTemplateStore->getPlayerTemplateCount();
							AsciiString sideStr = *cit;
							DEBUG_LOG(("Chose %s as our side... finding\n", sideStr.str()));
							for (Int c=0; c<numPlayerTemplates; ++c)
							{
								const PlayerTemplate *fac = ThePlayerTemplateStore->getNthPlayerTemplate(c);
								if (fac && fac->getSide() == sideStr)
								{
									DEBUG_LOG(("Found %s in index %d\n", sideStr.str(), c));
									req.QM.side = c;
									break;
								}
							}
						}
					}

					index = -1;
					GadgetComboBoxGetSelectedPos( comboBoxColor, &selected );
					if (selected >= 0)
						index = (Int)GadgetComboBoxGetItemData( comboBoxColor, selected );
					req.QM.color = index;

					OptionPreferences natPref;
					req.QM.NAT = natPref.getFirewallBehavior();

					if (ladderIndex)
					{
						req.QM.numPlayers = (ladderInfo)?ladderInfo->playersPerTeam*2 : 2;
					}
					else
					{
						GadgetComboBoxGetSelectedPos(comboBoxNumPlayers, &val);
						if (val < 0)
							val = 0;
						req.QM.numPlayers = (val+1)*2;
					}

					Int numDiscons = 0;
					PerGeneralMap::iterator it;
					for(it =stats.discons.begin(); it != stats.discons.end(); ++it)
					{
						numDiscons += it->second;
					}
					for(it =stats.desyncs.begin(); it != stats.desyncs.end(); ++it)
					{
						numDiscons += it->second;
					}
					req.QM.discons = numDiscons;


					strncpy(req.QM.pings, TheGameSpyInfo->getPingString().str(), 17);
					req.QM.pings[16] = 0;
					
					req.QM.botID = TheGameSpyConfig->getQMBotID();
					req.QM.roomID = TheGameSpyConfig->getQMChannel();

					req.QM.exeCRC = TheGlobalData->m_exeCRC;
					req.QM.iniCRC = TheGlobalData->m_iniCRC;

					TheGameSpyPeerMessageQueue->addRequest(req);
					buttonWiden->winEnable( FALSE );
					buttonStart->winHide( TRUE );
					buttonStop->winHide( FALSE );
					enableOptionsGadgets(FALSE);

					if (ladderIndex > 0)
					{
						// save the ladder as being played upon even if we cancel out of matching early...
						LadderPreferences ladPref;
						ladPref.loadProfile( TheGameSpyInfo->getLocalProfileID() );
						LadderPref p;
						p.lastPlayDate = time(NULL);
						p.address = ladderInfo->address;
						p.port = ladderInfo->port;
						p.name = ladderInfo->name;
						ladPref.addRecentLadder( p );
						ladPref.write();
					}
				}
				else if ( controlID == buttonBuddiesID )
				{
					GameSpyToggleOverlay( GSOVERLAY_BUDDY );
				}
				else if ( controlID == buttonBackID )
				{
					buttonPushed = true;
					TheGameSpyInfo->leaveGroupRoom();
					nextScreen = "Menus/WOLWelcomeMenu.wnd";
					TheShell->pop();
				} //if ( controlID == buttonBack )
				else if ( controlID == buttonSelectAllMapsID )
				{
					Int numMaps = GadgetListBoxGetNumEntries(listboxMapSelect);
					for ( Int i=0; i<numMaps; ++i )
					{
						GadgetListBoxAddEntryImage(listboxMapSelect, selectedImage, i, 0);
						GadgetListBoxSetItemData(listboxMapSelect, (void *)1, i);
						GadgetListBoxAddEntryText(listboxMapSelect, GadgetListBoxGetText(listboxMapSelect, i, 1), GameSpyColor[GSCOLOR_MAP_SELECTED], i, 1);
					}
				} //if ( controlID == buttonSelectAllMapsID )
				else if ( controlID == buttonSelectNoMapsID )
				{
					Int numMaps = GadgetListBoxGetNumEntries(listboxMapSelect);
					for ( Int i=0; i<numMaps; ++i )
					{
						GadgetListBoxAddEntryImage(listboxMapSelect, unselectedImage, i, 0);
						GadgetListBoxSetItemData(listboxMapSelect, (void *)0, i);
						GadgetListBoxAddEntryText(listboxMapSelect, GadgetListBoxGetText(listboxMapSelect, i, 1), GameSpyColor[GSCOLOR_MAP_UNSELECTED], i, 1);
					}
				} //if ( controlID == buttonSelectNoMapsID )
				break;
			}// case GBM_SELECTED:
	
		case GLM_SELECTED:
			{
				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();
				Int selected = (Int)mData2;

				if ( controlID == listboxMapSelectID )
				{
					const LadderInfo *li = getLadderInfo();
					if (selected >= 0 && (!li || !li->randomMaps))
					{
						Bool wasSelected = (Bool)GadgetListBoxGetItemData(control, selected, 0);
						GadgetListBoxSetItemData(control, (void *)(!wasSelected), selected, 0);
						Int width = 10;
						Int height = 10;
						const Image *img = (!wasSelected)?selectedImage:unselectedImage;
						if ( img )
						{
							width = min(GadgetListBoxGetColumnWidth(control, 0), img->getImageWidth());
							height = width;
						}
						GadgetListBoxAddEntryImage(control, img, selected, 0, height, width);
						GadgetListBoxAddEntryText(control, GadgetListBoxGetText(control, selected, 1), GameSpyColor[(wasSelected)?GSCOLOR_MAP_UNSELECTED:GSCOLOR_MAP_SELECTED], selected, 1);
					}
					if (selected >= 0)
						GadgetListBoxSetSelected(control, -1);
				}
				UpdateStartButton();
				break;
			}// case GLM_SELECTED

		case GEM_EDIT_DONE:
			{
				break;
			}
		default:
			return MSG_IGNORED;

	}//Switch

	return MSG_HANDLED;
}// WOLQuickMatchMenuSystem
