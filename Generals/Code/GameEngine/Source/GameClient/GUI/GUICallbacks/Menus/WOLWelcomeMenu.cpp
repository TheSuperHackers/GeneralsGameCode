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
// FILE: WOLWelcomeMenu.cpp
// Author: Chris Huybregts, November 2001
// Description: Lan Lobby Menu
///////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "gamespy/peer/peer.h"

#include "Common/GameEngine.h"
#include "Common/GameSpyMiscPreferences.h"
#include "Common/CustomMatchPreferences.h"
#include "Common/GlobalData.h"
#include "Common/UserPreferences.h"

#include "GameClient/AnimateWindowManager.h"
#include "GameClient/Display.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/GameText.h"
#include "GameClient/Shell.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GadgetTextEntry.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/MessageBox.h"
#include "GameClient/GameWindowTransitions.h"

#include "GameNetwork/FirewallHelper.h"
#include "GameNetwork/GameSpyOverlay.h"

#include "GameNetwork/GameSpy/BuddyDefs.h"
#include "GameNetwork/GameSpy/BuddyThread.h"
#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/GameSpy/PeerThread.h"
#include "GameNetwork/GameSpy/PersistentStorageDefs.h"
#include "GameNetwork/GameSpy/PersistentStorageThread.h"
#include "GameNetwork/GameSpy/BuddyThread.h"
#include "GameNetwork/GameSpy/ThreadUtils.h"
#include "GameNetwork/GameSpy/MainMenuUtils.h"
#include "GameNetwork/WOLBrowser/WebBrowser.h"

#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif
// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
static Bool isShuttingDown = FALSE;
static Bool buttonPushed = FALSE;
static const char *nextScreen = NULL;

// window ids ------------------------------------------------------------------------------
static NameKeyType parentWOLWelcomeID = NAMEKEY_INVALID;
static NameKeyType buttonBackID = NAMEKEY_INVALID;
static NameKeyType buttonQuickMatchID = NAMEKEY_INVALID;
static NameKeyType buttonLobbyID = NAMEKEY_INVALID;
static NameKeyType buttonBuddiesID = NAMEKEY_INVALID;
static NameKeyType buttonLadderID = NAMEKEY_INVALID;
static NameKeyType buttonMyInfoID = NAMEKEY_INVALID;

static NameKeyType listboxInfoID = NAMEKEY_INVALID;
static NameKeyType buttonOptionsID = NAMEKEY_INVALID;
// Window Pointers ------------------------------------------------------------------------
static GameWindow *parentWOLWelcome = NULL;
static GameWindow *buttonBack = NULL;
static GameWindow *buttonQuickMatch = NULL;
static GameWindow *buttonLobby = NULL;
static GameWindow *buttonBuddies = NULL;
static GameWindow *buttonLadder = NULL;
static GameWindow *buttonMyInfo = NULL;
static GameWindow *buttonbuttonOptions = NULL;
static WindowLayout *welcomeLayout = NULL;
static GameWindow *listboxInfo = NULL;

static GameWindow *staticTextServerName = NULL;
static GameWindow *staticTextLastUpdated = NULL;

static GameWindow *staticTextLadderWins = NULL;
static GameWindow *staticTextLadderLosses = NULL;
static GameWindow *staticTextLadderRank = NULL;
static GameWindow *staticTextLadderPoints = NULL;
static GameWindow *staticTextLadderDisconnects = NULL;

static GameWindow *staticTextHighscoreWins = NULL;
static GameWindow *staticTextHighscoreLosses = NULL;
static GameWindow *staticTextHighscoreRank = NULL;
static GameWindow *staticTextHighscorePoints = NULL;

static UnicodeString gServerName;
void updateServerDisplay(UnicodeString serverName)
{
	if (staticTextServerName)
	{
		GadgetStaticTextSetText(staticTextServerName, serverName);
	}
	gServerName = serverName;
}

/*
void updateLocalPlayerScores(AsciiString name, const WOL::Ladder *ladder, const WOL::Highscore *highscore)
{
	if (ladder)
	{
		AsciiString a;
		UnicodeString u;

		a.format("%d", ladder->wins);
		u.translate(a);
		GadgetStaticTextSetText(staticTextLadderWins, u);

		a.format("%d", ladder->losses);
		u.translate(a);
		GadgetStaticTextSetText(staticTextLadderLosses, u);

		a.format("%d", ladder->rank);
		u.translate(a);
		GadgetStaticTextSetText(staticTextLadderRank, u);

		a.format("%d", ladder->points);
		u.translate(a);
		GadgetStaticTextSetText(staticTextLadderPoints, u);

		a.format("%d", ladder->disconnects);
		u.translate(a);
		GadgetStaticTextSetText(staticTextLadderDisconnects, u);
	}
	if (highscore)
	{
		AsciiString a;
		UnicodeString u;

		a.format("%d", highscore->wins);
		u.translate(a);
		GadgetStaticTextSetText(staticTextHighscoreWins, u);

		a.format("%d", highscore->losses);
		u.translate(a);
		GadgetStaticTextSetText(staticTextHighscoreLosses, u);

		a.format("%d", highscore->rank);
		u.translate(a);
		GadgetStaticTextSetText(staticTextHighscoreRank, u);

		a.format("%d", highscore->points);
		u.translate(a);
		GadgetStaticTextSetText(staticTextHighscorePoints, u);
	}
}
*/


static void enableControls( Bool state )
{
	if (buttonQuickMatch)
		buttonQuickMatch->winEnable(state);
	if (buttonLobby)
		buttonLobby->winEnable(state);
}

//-------------------------------------------------------------------------------------------------
/** This is called when a shutdown is complete for this menu */
//-------------------------------------------------------------------------------------------------
static void shutdownComplete( WindowLayout *layout )
{

	isShuttingDown = FALSE;

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
/** Handle Num Players Online data */
//-------------------------------------------------------------------------------------------------

static Int lastNumPlayersOnline = 0;

static UnsignedByte grabUByte(const char *s)
{
	char tmp[5] = "0xff";
	tmp[2] = s[0];
	tmp[3] = s[1];
	UnsignedByte b = strtol(tmp, NULL, 16);
	return b;
}

static void updateNumPlayersOnline(void)
{
	GameWindow *playersOnlineWindow = TheWindowManager->winGetWindowFromId(
		NULL, NAMEKEY("WOLWelcomeMenu.wnd:StaticTextNumPlayersOnline") );

	if (playersOnlineWindow)
	{
		UnicodeString valStr;
		valStr.format(TheGameText->fetch("GUI:NumPlayersOnline"), lastNumPlayersOnline);
		GadgetStaticTextSetText(playersOnlineWindow, valStr);
	}

	if (listboxInfo && TheGameSpyInfo)
	{
		GadgetListBoxReset(listboxInfo);
		AsciiString aLine;
		UnicodeString line;
		AsciiString aMotd = TheGameSpyInfo->getMOTD();
		UnicodeString headingStr;
		headingStr.format(TheGameText->fetch("MOTD:NumPlayersHeading"), lastNumPlayersOnline);

		while (headingStr.nextToken(&line, UnicodeString(L"\n")))
		{
			if (line.getCharAt(line.getLength()-1) == '\r')
				line.removeLastChar();	// there is a trailing '\r'

			line.trim();

			if (line.isEmpty())
			{
				line = UnicodeString(L" ");
			}

			GadgetListBoxAddEntryText(listboxInfo, line, GameSpyColor[GSCOLOR_MOTD_HEADING], -1, -1);
		}
		GadgetListBoxAddEntryText(listboxInfo, UnicodeString(L" "), GameSpyColor[GSCOLOR_MOTD_HEADING], -1, -1);

		while (aMotd.nextToken(&aLine, "\n"))
		{
			if (aLine.getCharAt(aLine.getLength()-1) == '\r')
				aLine.removeLastChar();	// there is a trailing '\r'

			aLine.trim();

			if (aLine.isEmpty())
			{
				aLine = " ";
			}

			Color c = GameSpyColor[GSCOLOR_MOTD];
			if (aLine.startsWith("\\\\"))
			{
				aLine = aLine.str()+1;
			}
			else if (aLine.startsWith("\\") && aLine.getLength() > 9)
			{
				// take out the hex value from strings starting as "\ffffffffText"
				UnsignedByte a, r, g, b;
				a = grabUByte(aLine.str()+1);
				r = grabUByte(aLine.str()+3);
				g = grabUByte(aLine.str()+5);
				b = grabUByte(aLine.str()+7);
				c = GameMakeColor(r, g, b, a);
				DEBUG_LOG(("MOTD line '%s' has color %X\n", aLine.str(), c));
				aLine = aLine.str() + 9;
			}
			line = UnicodeString(MultiByteToWideCharSingleLine(aLine.str()).c_str());

			GadgetListBoxAddEntryText(listboxInfo, line, c, -1, -1);
		}
	}
}

void HandleNumPlayersOnline( Int numPlayersOnline )
{
	lastNumPlayersOnline = numPlayersOnline;
	if (lastNumPlayersOnline < 1)
		lastNumPlayersOnline = 1;
	updateNumPlayersOnline();
}

//-------------------------------------------------------------------------------------------------
/** Handle Overall Stats data */
//-------------------------------------------------------------------------------------------------

static OverallStats s_statsUSA, s_statsChina, s_statsGLA;

OverallStats::OverallStats()
{
	for (Int i=0; i<STATS_MAX; ++i)
	{
		wins[i] = losses[i] = 0;
	}
}

static UnicodeString calcPercent(const OverallStats& stats, Int n, UnicodeString sideStr)
{
	// per side percentage of total wins
	Real winPercentUSA   = s_statsUSA.wins[n]*100/INT_TO_REAL(max(1, s_statsUSA.wins[n]+s_statsUSA.losses[n])); // 0.0f - 100.0f
	Real winPercentChina = s_statsChina.wins[n]*100/INT_TO_REAL(max(1, s_statsChina.wins[n]+s_statsChina.losses[n])); // 0.0f - 100.0f
	Real winPercentGLA   = s_statsGLA.wins[n]*100/INT_TO_REAL(max(1, s_statsGLA.wins[n]+s_statsGLA.losses[n])); // 0.0f - 100.0f
	Real thisWinPercent  = stats.wins[n]*100/INT_TO_REAL(max(1, stats.wins[n]+stats.losses[n])); // 0.0f - 100.0f
	Real totalWinPercent = winPercentUSA + winPercentChina + winPercentGLA;

	Real val = thisWinPercent*100/max(1.0f,totalWinPercent);

	UnicodeString s;
	s.format(TheGameText->fetch("GUI:PerSideWinPercentage"), REAL_TO_INT(val), sideStr.str());

	/*
	Int totalDenominator = s_statsUSA.wins[n] + s_statsChina.wins[n] + s_statsGLA.wins[n];
	if (!totalDenominator)
		totalDenominator = 1;

	UnicodeString s;
	s.format(TheGameText->fetch("GUI:PerSideWinPercentage"), REAL_TO_INT(stats.wins[n]*100/totalDenominator), sideStr.str());
	*/
	return s;
}

static void updateOverallStats(void)
{
	UnicodeString usa, china, gla;
	GameWindow *win;

	usa = calcPercent(s_statsUSA, STATS_LASTWEEK, TheGameText->fetch("SIDE:America"));
	china = calcPercent(s_statsChina, STATS_LASTWEEK, TheGameText->fetch("SIDE:China"));
	gla = calcPercent(s_statsGLA, STATS_LASTWEEK, TheGameText->fetch("SIDE:GLA"));
	DEBUG_LOG(("Last Week: %ls %ls %ls\n", usa.str(), china.str(), gla.str()));
	win = TheWindowManager->winGetWindowFromId( NULL, NAMEKEY("WOLWelcomeMenu.wnd:StaticTextUSALastWeek") );
	GadgetStaticTextSetText(win, usa);
	win = TheWindowManager->winGetWindowFromId( NULL, NAMEKEY("WOLWelcomeMenu.wnd:StaticTextChinaLastWeek") );
	GadgetStaticTextSetText(win, china);
	win = TheWindowManager->winGetWindowFromId( NULL, NAMEKEY("WOLWelcomeMenu.wnd:StaticTextGLALastWeek") );
	GadgetStaticTextSetText(win, gla);

	usa = calcPercent(s_statsUSA, STATS_TODAY, TheGameText->fetch("SIDE:America"));
	china = calcPercent(s_statsChina, STATS_TODAY, TheGameText->fetch("SIDE:China"));
	gla = calcPercent(s_statsGLA, STATS_TODAY, TheGameText->fetch("SIDE:GLA"));
	DEBUG_LOG(("Today: %ls %ls %ls\n", usa.str(), china.str(), gla.str()));
	win = TheWindowManager->winGetWindowFromId( NULL, NAMEKEY("WOLWelcomeMenu.wnd:StaticTextUSAToday") );
	GadgetStaticTextSetText(win, usa);
	win = TheWindowManager->winGetWindowFromId( NULL, NAMEKEY("WOLWelcomeMenu.wnd:StaticTextChinaToday") );
	GadgetStaticTextSetText(win, china);
	win = TheWindowManager->winGetWindowFromId( NULL, NAMEKEY("WOLWelcomeMenu.wnd:StaticTextGLAToday") );
	GadgetStaticTextSetText(win, gla);
}

void HandleOverallStats( const OverallStats& USA, const OverallStats& China, const OverallStats& GLA )
{
	s_statsUSA = USA;
	s_statsChina = China;
	s_statsGLA = GLA;
	updateOverallStats();
}

//-------------------------------------------------------------------------------------------------
/** Handle player stats */
//-------------------------------------------------------------------------------------------------

void UpdateLocalPlayerStats(void)
{

	GameWindow *welcomeParent = TheWindowManager->winGetWindowFromId( NULL, NAMEKEY("WOLWelcomeMenu.wnd:WOLWelcomeMenuParent") );

	if (welcomeParent)
	{
		PopulatePlayerInfoWindows( "WOLWelcomeMenu.wnd" );
	}
	else
	{
		PopulatePlayerInfoWindows( "WOLQuickMatchMenu.wnd" );
	}
	
	return;
}

static Bool raiseMessageBoxes = FALSE;
//-------------------------------------------------------------------------------------------------
/** Initialize the WOL Welcome Menu */
//-------------------------------------------------------------------------------------------------
void WOLWelcomeMenuInit( WindowLayout *layout, void *userData )
{
	nextScreen = NULL;
	buttonPushed = FALSE;
	isShuttingDown = FALSE;

	welcomeLayout = layout;

	//TheWOL->reset();

	parentWOLWelcomeID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLWelcomeMenu.wnd:WOLWelcomeMenuParent" ) );
	buttonBackID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLWelcomeMenu.wnd:ButtonBack" ) );
	parentWOLWelcome = TheWindowManager->winGetWindowFromId( NULL, parentWOLWelcomeID );
	buttonBack = TheWindowManager->winGetWindowFromId( NULL,  buttonBackID);
	buttonOptionsID = TheNameKeyGenerator->nameToKey( "WOLWelcomeMenu.wnd:ButtonOptions" );
	buttonbuttonOptions = TheWindowManager->winGetWindowFromId( NULL,  buttonOptionsID);
	listboxInfoID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLWelcomeMenu.wnd:InfoListbox" ) );

	listboxInfo = TheWindowManager->winGetWindowFromId( NULL,  listboxInfoID);

	staticTextServerName = TheWindowManager->winGetWindowFromId( parentWOLWelcome,
		TheNameKeyGenerator->nameToKey( "WOLWelcomeMenu.wnd:StaticTextServerName" ));
	staticTextLastUpdated = TheWindowManager->winGetWindowFromId( parentWOLWelcome,
		TheNameKeyGenerator->nameToKey( "WOLWelcomeMenu.wnd:StaticTextLastUpdated" ));

	staticTextLadderWins = TheWindowManager->winGetWindowFromId( parentWOLWelcome,
		TheNameKeyGenerator->nameToKey( "WOLWelcomeMenu.wnd:StaticTextLadderWins" ));
	staticTextLadderLosses = TheWindowManager->winGetWindowFromId( parentWOLWelcome,
		TheNameKeyGenerator->nameToKey( "WOLWelcomeMenu.wnd:StaticTextLadderLosses" ));
	staticTextLadderPoints = TheWindowManager->winGetWindowFromId( parentWOLWelcome,
		TheNameKeyGenerator->nameToKey( "WOLWelcomeMenu.wnd:StaticTextLadderPoints" ));
	staticTextLadderRank = TheWindowManager->winGetWindowFromId( parentWOLWelcome,
		TheNameKeyGenerator->nameToKey( "WOLWelcomeMenu.wnd:StaticTextLadderRank" ));
	staticTextLadderDisconnects = TheWindowManager->winGetWindowFromId( parentWOLWelcome,
		TheNameKeyGenerator->nameToKey( "WOLWelcomeMenu.wnd:StaticTextDisconnects" ));

	staticTextHighscoreWins = TheWindowManager->winGetWindowFromId( parentWOLWelcome,
		TheNameKeyGenerator->nameToKey( "WOLWelcomeMenu.wnd:StaticTextHighscoreWins" ));
	staticTextHighscoreLosses = TheWindowManager->winGetWindowFromId( parentWOLWelcome,
		TheNameKeyGenerator->nameToKey( "WOLWelcomeMenu.wnd:StaticTextHighscoreLosses" ));
	staticTextHighscorePoints = TheWindowManager->winGetWindowFromId( parentWOLWelcome,
		TheNameKeyGenerator->nameToKey( "WOLWelcomeMenu.wnd:StaticTextHighscorePoints" ));
	staticTextHighscoreRank = TheWindowManager->winGetWindowFromId( parentWOLWelcome,
		TheNameKeyGenerator->nameToKey( "WOLWelcomeMenu.wnd:StaticTextHighscoreRank" ));

	if (staticTextServerName)
	{
		GadgetStaticTextSetText(staticTextServerName, gServerName);
	}

	GameWindow *staticTextTitle = TheWindowManager->winGetWindowFromId(parentWOLWelcome, NAMEKEY("WOLWelcomeMenu.wnd:StaticTextTitle"));
	if (staticTextTitle && TheGameSpyInfo)
	{
		UnicodeString title;
		title.format(TheGameText->fetch("GUI:WOLWelcome"), TheGameSpyInfo->getLocalBaseName().str());
		GadgetStaticTextSetText(staticTextTitle, title);
	}

	// Clear some defaults
	/*
	UnicodeString questionMark = UnicodeString(L"?");
	GadgetStaticTextSetText(staticTextLastUpdated, questionMark);
	GadgetStaticTextSetText(staticTextLadderWins, questionMark);
	GadgetStaticTextSetText(staticTextLadderLosses, questionMark);
	GadgetStaticTextSetText(staticTextLadderPoints, questionMark);
	GadgetStaticTextSetText(staticTextLadderRank, questionMark);
	GadgetStaticTextSetText(staticTextLadderDisconnects, questionMark);
	GadgetStaticTextSetText(staticTextHighscoreWins, questionMark);
	GadgetStaticTextSetText(staticTextHighscoreLosses, questionMark);
	GadgetStaticTextSetText(staticTextHighscorePoints, questionMark);
	GadgetStaticTextSetText(staticTextHighscoreRank, questionMark);
	*/

	//DEBUG_ASSERTCRASH(listboxInfo, ("No control found!"));

	buttonQuickMatchID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLWelcomeMenu.wnd:ButtonQuickMatch" ) );
	buttonQuickMatch = TheWindowManager->winGetWindowFromId( parentWOLWelcome, buttonQuickMatchID );

	buttonLobbyID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLWelcomeMenu.wnd:ButtonCustomMatch" ) );
	buttonLobby = TheWindowManager->winGetWindowFromId( parentWOLWelcome, buttonLobbyID );

	buttonBuddiesID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLWelcomeMenu.wnd:ButtonBuddies" ) );
	buttonBuddies = TheWindowManager->winGetWindowFromId( parentWOLWelcome, buttonBuddiesID );

	buttonMyInfoID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLWelcomeMenu.wnd:ButtonMyInfo" ) );
	buttonMyInfo = TheWindowManager->winGetWindowFromId( parentWOLWelcome, buttonMyInfoID );

	buttonLadderID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLWelcomeMenu.wnd:ButtonLadder" ) );
	buttonLadder = TheWindowManager->winGetWindowFromId( parentWOLWelcome, buttonLadderID );

	if (TheFirewallHelper == NULL) {
		TheFirewallHelper = createFirewallHelper();
	}
	if (TheFirewallHelper->detectFirewall() == TRUE) {
		// don't need to detect firewall, already been done.
		delete TheFirewallHelper;
		TheFirewallHelper = NULL;
	}
	/*

	if (TheGameSpyChat && TheGameSpyChat->isConnected())
	{
		const char *keys[3] = { "locale", "wins", "losses" };
		char valueStrings[3][20];
		char *values[3] = { valueStrings[0], valueStrings[1], valueStrings[2] };
		snprintf(values[0], 20, "%s", TheGameSpyPlayerInfo->getLocale().str());
		snprintf(values[1], 20, "%d", TheGameSpyPlayerInfo->getWins());
		snprintf(values[2], 20, "%d", TheGameSpyPlayerInfo->getLosses());
		peerSetGlobalKeys(TheGameSpyChat->getPeer(), 3, (const char **)keys, (const char **)values);
		peerSetGlobalWatchKeys(TheGameSpyChat->getPeer(), GroupRoom,   3, keys, PEERFalse);
		peerSetGlobalWatchKeys(TheGameSpyChat->getPeer(), StagingRoom, 3, keys, PEERFalse);
	}
	*/

//	// animate controls
//	TheShell->registerWithAnimateManager(buttonQuickMatch, WIN_ANIMATION_SLIDE_LEFT, TRUE, 800);
//	TheShell->registerWithAnimateManager(buttonLobby, WIN_ANIMATION_SLIDE_LEFT, TRUE, 600);
//	//TheShell->registerWithAnimateManager(NULL, WIN_ANIMATION_SLIDE_LEFT, TRUE, 400);
//	TheShell->registerWithAnimateManager(buttonBuddies, WIN_ANIMATION_SLIDE_LEFT, TRUE, 200);
//	//TheShell->registerWithAnimateManager(NULL, WIN_ANIMATION_SLIDE_LEFT, TRUE, 1);
//	TheShell->registerWithAnimateManager(buttonBack, WIN_ANIMATION_SLIDE_BOTTOM, TRUE, 1);

	// Show Menu
	layout->hide( FALSE );

	// Set Keyboard to Main Parent
	TheWindowManager->winSetFocus( parentWOLWelcome );

	enableControls( TheGameSpyInfo->gotGroupRoomList() );
	TheShell->showShellMap(TRUE);

	updateNumPlayersOnline();
	updateOverallStats();

	UpdateLocalPlayerStats();

	GameSpyMiscPreferences cPref;
	if (cPref.getLocale() < LOC_MIN || cPref.getLocale() > LOC_MAX)
	{
		GameSpyOpenOverlay(GSOVERLAY_LOCALESELECT);
	}

	raiseMessageBoxes = TRUE;
	TheTransitionHandler->setGroup("WOLWelcomeMenuFade");

} // WOLWelcomeMenuInit

//-------------------------------------------------------------------------------------------------
/** WOL Welcome Menu shutdown method */
//-------------------------------------------------------------------------------------------------
void WOLWelcomeMenuShutdown( WindowLayout *layout, void *userData )
{
	listboxInfo = NULL;

	if (TheFirewallHelper != NULL) {
		delete TheFirewallHelper;
		TheFirewallHelper = NULL;
	}

	isShuttingDown = TRUE;

	// if we are shutting down for an immediate pop, skip the animations
	Bool popImmediate = *(Bool *)userData;
	if( popImmediate )
	{

		shutdownComplete( layout );
		return;

	}  //end if

	TheShell->reverseAnimatewindow();
	TheTransitionHandler->reverse("WOLWelcomeMenuFade");


	RaiseGSMessageBox();
}  // WOLWelcomeMenuShutdown


//-------------------------------------------------------------------------------------------------
/** WOL Welcome Menu update method */
//-------------------------------------------------------------------------------------------------
void WOLWelcomeMenuUpdate( WindowLayout * layout, void *userData)
{
	// We'll only be successful if we've requested to 
	if(isShuttingDown && TheShell->isAnimFinished() && TheTransitionHandler->isFinished())
		shutdownComplete(layout);

	if (raiseMessageBoxes)
	{
		RaiseGSMessageBox();
		raiseMessageBoxes = FALSE;
	}

	if (TheFirewallHelper != NULL)
	{
		if (TheFirewallHelper->behaviorDetectionUpdate())
		{
			TheWritableGlobalData->m_firewallBehavior = TheFirewallHelper->getFirewallBehavior();

			TheFirewallHelper->writeFirewallBehavior();

			TheFirewallHelper->flagNeedToRefresh(FALSE); // 2/19/03 BGC, we're done, so we don't need to refresh the NAT anymore.
			
			// we are now done with the firewall helper
			delete TheFirewallHelper;
			TheFirewallHelper = NULL;
		}
	}

	if (TheShell->isAnimFinished() && !buttonPushed && TheGameSpyPeerMessageQueue)
	{
		HandleBuddyResponses();
		HandlePersistentStorageResponses();

		Int allowedMessages = TheGameSpyInfo->getMaxMessagesPerUpdate();
		Bool sawImportantMessage = FALSE;
		PeerResponse resp;
		while (allowedMessages-- && !sawImportantMessage && TheGameSpyPeerMessageQueue->getResponse( resp ))
		{
			switch (resp.peerResponseType)
			{
			case PeerResponse::PEERRESPONSE_GROUPROOM:
				{
					GameSpyGroupRoom room;
					room.m_groupID = resp.groupRoom.id;
					room.m_maxWaiting = resp.groupRoom.maxWaiting;
					room.m_name = resp.groupRoomName.c_str();
					room.m_translatedName = UnicodeString(L"TEST");
					room.m_numGames = resp.groupRoom.numGames;
					room.m_numPlaying = resp.groupRoom.numPlaying;
					room.m_numWaiting = resp.groupRoom.numWaiting;
					TheGameSpyInfo->addGroupRoom( room );
					if (room.m_groupID == 0)
					{
						enableControls( TRUE );
					}
				}
				break;
			case PeerResponse::PEERRESPONSE_JOINGROUPROOM:
				{
					sawImportantMessage = TRUE;
					enableControls( TRUE );
					if (resp.joinGroupRoom.ok)
					{
						//buttonPushed = TRUE;
						TheGameSpyInfo->setCurrentGroupRoom(resp.joinGroupRoom.id);
						//GSMessageBoxOk( TheGameText->fetch("GUI:GSErrorTitle"), TheGameText->fetch("GUI:GSGroupRoomJoinOK") );

						buttonPushed = TRUE;
						nextScreen = "Menus/WOLCustomLobby.wnd";
						TheShell->pop();
						//TheShell->push( "Menus/WOLCustomLobby.wnd" );
					}
					else
					{
						GSMessageBoxOk( TheGameText->fetch("GUI:GSErrorTitle"), TheGameText->fetch("GUI:GSGroupRoomJoinFail") );
					}
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
					TheShell->pop();
				}
				break;
			}
		}
	}

}// WOLWelcomeMenuUpdate

//-------------------------------------------------------------------------------------------------
/** WOL Welcome Menu input callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType WOLWelcomeMenuInput( GameWindow *window, UnsignedInt msg,
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
}// WOLWelcomeMenuInput

//-------------------------------------------------------------------------------------------------
/** WOL Welcome Menu window system callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType WOLWelcomeMenuSystem( GameWindow *window, UnsignedInt msg, 
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

		case GBM_SELECTED:
			{
				if (buttonPushed)
					break;

				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();

				if ( controlID == buttonBackID )
				{
					//DEBUG_ASSERTCRASH(TheGameSpyChat->getPeer(), ("No GameSpy Peer object!"));
					//TheGameSpyChat->disconnectFromChat();
					
					PeerRequest req;
					req.peerRequestType = PeerRequest::PEERREQUEST_LOGOUT;
					TheGameSpyPeerMessageQueue->addRequest( req );
					BuddyRequest breq;
					breq.buddyRequestType = BuddyRequest::BUDDYREQUEST_LOGOUT;
					TheGameSpyBuddyMessageQueue->addRequest( breq );

					DEBUG_LOG(("Tearing down GameSpy from WOLWelcomeMenuSystem(GBM_SELECTED)\n"));
					TearDownGameSpy();

					/*
					if (TheGameSpyChat->getPeer())
					{
						peerDisconnect(TheGameSpyChat->getPeer());
					}
					*/

					buttonPushed = TRUE;

					TheShell->pop();

					/// @todo: log out instead of disconnecting
					//TheWOL->addCommand( WOL::WOLCOMMAND_LOGOUT );
					/**
					closeAllOverlays();
					TheShell->pop();
					delete TheWOL;
					TheWOL = NULL;
					delete TheWOLGame;
					TheWOLGame = NULL;
					**/

				} //if ( controlID == buttonBack )
				else if (controlID == buttonOptionsID)
				{					
					GameSpyOpenOverlay( GSOVERLAY_OPTIONS );
				}
				else if (controlID == buttonQuickMatchID)
				{
					GameSpyMiscPreferences mPref;
					if ((TheDisplay->getWidth() != 800 || TheDisplay->getHeight() != 600) && mPref.getQuickMatchResLocked())
					{
						GSMessageBoxOk(TheGameText->fetch("GUI:GSErrorTitle"), TheGameText->fetch("GUI:QuickMatch800x600"));
					}
					else
					{
						buttonPushed = TRUE;
						nextScreen = "Menus/WOLQuickMatchMenu.wnd";
						TheShell->pop();
					}
				}// else if
				else if (controlID == buttonMyInfoID )
				{
					SetLookAtPlayer(TheGameSpyInfo->getLocalProfileID(), TheGameSpyInfo->getLocalName());
					GameSpyToggleOverlay(GSOVERLAY_PLAYERINFO);
				}
				else if (controlID == buttonLobbyID)
				{
					//TheGameSpyChat->clearGroupRoomList();
					//peerListGroupRooms(TheGameSpyChat->getPeer(), ListGroupRoomsCallback, NULL, PEERTrue);
					TheGameSpyInfo->joinBestGroupRoom();
					enableControls( FALSE );


					/*
					TheWOL->setScreen(WOL::WOLAPI_MENU_CUSTOMLOBBY);
					TheWOL->setGameMode(WOL::WOLTYPE_CUSTOM);
					TheWOL->setState( WOL::WOLAPI_LOBBY );
					TheWOL->addCommand( WOL::WOLCOMMAND_REFRESH_CHANNELS );
					*/
				}// else if
				else if (controlID == buttonBuddiesID)
				{
					GameSpyToggleOverlay( GSOVERLAY_BUDDY );
					/*
					Bool joinedRoom = FALSE;
					ClearGroupRoomList();
					peerJoinTitleRoom(TheGameSpyChat->getPeer(), JoinRoomCallback, &joinedRoom, PEERTrue);
					if (joinedRoom)
					{
						GameSpyUsingGroupRooms = FALSE;
						GameSpyCurrentGroupRoomID = 0;
						TheShell->pop();
						TheShell->push("Menus/WOLCustomLobby.wnd");
					}
					else
					{
						GameSpyCurrentGroupRoomID = 0;
						GSMessageBoxOk(UnicodeString(L"Oops"), UnicodeString(L"Unable to join title room"), NULL);
					}
					*/
				}
				else if (controlID == buttonLadderID)
				{
					TheShell->push(AsciiString("Menus/WOLLadderScreen.wnd"));
				}
				break;
			}// case GBM_SELECTED:
	
		case GEM_EDIT_DONE:
			{
				break;
			}
		default:
			return MSG_IGNORED;

	}//Switch

	return MSG_HANDLED;
}// WOLWelcomeMenuSystem
