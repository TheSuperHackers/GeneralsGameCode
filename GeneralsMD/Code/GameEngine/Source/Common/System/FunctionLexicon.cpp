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

// FILE: FunctionLexicon.cpp //////////////////////////////////////////////////////////////////////
// Created:    Colin Day, September 2001
// Desc:       Collection of function pointers to help us in managing
//						 and assign callbacks
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/FunctionLexicon.h"
#include "GameClient/GameWindow.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GUICallbacks.h"
#include "GameClient/Gadget.h"

// Popup Ladder Select --------------------------------------------------------------------------
extern void PopupLadderSelectInit( WindowLayout *layout, void *userData );
extern WindowMsgHandledType PopupLadderSelectSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType PopupLadderSelectInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

extern WindowMsgHandledType PopupBuddyNotificationSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// WOL Buddy Overlay Right Click menu callbacks --------------------------------------------------------------
extern void RCGameDetailsMenuInit( WindowLayout *layout, void *userData );
extern WindowMsgHandledType RCGameDetailsMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Beacon control bar callback --------------------------------------------------------------
extern WindowMsgHandledType BeaconWindowInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Popup Replay Save Menu ----------------------------------------------------------------------------------
extern void PopupReplayInit( WindowLayout *layout, void *userData );
extern void PopupReplayUpdate( WindowLayout *layout, void *userData );
extern void PopupReplayShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType PopupReplaySystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType PopupReplayInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Extended MessageBox ----------------------------------------------------------------------------------
extern WindowMsgHandledType ExtendedMessageBoxSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// game window draw table -----------------------------------------------------------------------
static FunctionLexicon::TableEntry gameWinDrawTable[] =
{
	{ NAMEKEY_INVALID, "IMECandidateMainDraw",      (void*)IMECandidateMainDraw },
	{ NAMEKEY_INVALID, "IMECandidateTextAreaDraw",  (void*)IMECandidateTextAreaDraw },
	{ NAMEKEY_INVALID, nullptr,                     nullptr }
};

// game window system table -----------------------------------------------------------------------
static FunctionLexicon::TableEntry gameWinSystemTable[] =
{


	{ NAMEKEY_INVALID, "PassSelectedButtonsToParentSystem",  (void*)PassSelectedButtonsToParentSystem },
	{ NAMEKEY_INVALID, "PassMessagesToParentSystem",         (void*)PassMessagesToParentSystem },

	{ NAMEKEY_INVALID, "GameWinDefaultSystem",               (void*)GameWinDefaultSystem },
	{ NAMEKEY_INVALID, "GadgetPushButtonSystem",             (void*)GadgetPushButtonSystem },
	{ NAMEKEY_INVALID, "GadgetCheckBoxSystem",               (void*)GadgetCheckBoxSystem },
	{ NAMEKEY_INVALID, "GadgetRadioButtonSystem",            (void*)GadgetRadioButtonSystem },
	{ NAMEKEY_INVALID, "GadgetTabControlSystem",             (void*)GadgetTabControlSystem },
	{ NAMEKEY_INVALID, "GadgetListBoxSystem",                (void*)GadgetListBoxSystem },
	{ NAMEKEY_INVALID, "GadgetComboBoxSystem",               (void*)GadgetComboBoxSystem },
	{ NAMEKEY_INVALID, "GadgetHorizontalSliderSystem",       (void*)GadgetHorizontalSliderSystem },
	{ NAMEKEY_INVALID, "GadgetVerticalSliderSystem",         (void*)GadgetVerticalSliderSystem },
	{ NAMEKEY_INVALID, "GadgetProgressBarSystem",            (void*)GadgetProgressBarSystem },
	{ NAMEKEY_INVALID, "GadgetStaticTextSystem",             (void*)GadgetStaticTextSystem },
	{ NAMEKEY_INVALID, "GadgetTextEntrySystem",              (void*)GadgetTextEntrySystem },
	{ NAMEKEY_INVALID, "MessageBoxSystem",                   (void*)MessageBoxSystem },
	{ NAMEKEY_INVALID, "QuitMessageBoxSystem",               (void*)QuitMessageBoxSystem },

	{ NAMEKEY_INVALID, "ExtendedMessageBoxSystem",           (void*)ExtendedMessageBoxSystem },

	{ NAMEKEY_INVALID, "MOTDSystem",                         (void*)MOTDSystem },
	{ NAMEKEY_INVALID, "MainMenuSystem",                     (void*)MainMenuSystem },
	{ NAMEKEY_INVALID, "OptionsMenuSystem",                  (void*)OptionsMenuSystem },
	{ NAMEKEY_INVALID, "SinglePlayerMenuSystem",             (void*)SinglePlayerMenuSystem },
	{ NAMEKEY_INVALID, "QuitMenuSystem",                     (void*)QuitMenuSystem },
	{ NAMEKEY_INVALID, "MapSelectMenuSystem",                (void*)MapSelectMenuSystem },
	{ NAMEKEY_INVALID, "ReplayMenuSystem",                   (void*)ReplayMenuSystem },
	{ NAMEKEY_INVALID, "CreditsMenuSystem",                  (void*)CreditsMenuSystem },
	{ NAMEKEY_INVALID, "LanLobbyMenuSystem",                 (void*)LanLobbyMenuSystem },
	{ NAMEKEY_INVALID, "LanGameOptionsMenuSystem",           (void*)LanGameOptionsMenuSystem },
	{ NAMEKEY_INVALID, "LanMapSelectMenuSystem",             (void*)LanMapSelectMenuSystem },
	{ NAMEKEY_INVALID, "SkirmishGameOptionsMenuSystem",      (void*)SkirmishGameOptionsMenuSystem },
	{ NAMEKEY_INVALID, "SkirmishMapSelectMenuSystem",        (void*)SkirmishMapSelectMenuSystem },
	{ NAMEKEY_INVALID, "ChallengeMenuSystem",                (void*)ChallengeMenuSystem },
	{ NAMEKEY_INVALID, "SaveLoadMenuSystem",                 (void*)SaveLoadMenuSystem },
	{ NAMEKEY_INVALID, "PopupCommunicatorSystem",            (void*)PopupCommunicatorSystem },
	{ NAMEKEY_INVALID, "PopupBuddyNotificationSystem",       (void*)PopupBuddyNotificationSystem },
	{ NAMEKEY_INVALID, "PopupReplaySystem",                  (void*)PopupReplaySystem },
	{ NAMEKEY_INVALID, "KeyboardOptionsMenuSystem",          (void*)KeyboardOptionsMenuSystem },
	{ NAMEKEY_INVALID, "WOLLadderScreenSystem",              (void*)WOLLadderScreenSystem },
	{ NAMEKEY_INVALID, "WOLLoginMenuSystem",                 (void*)WOLLoginMenuSystem },
	{ NAMEKEY_INVALID, "WOLLocaleSelectSystem",              (void*)WOLLocaleSelectSystem },
	{ NAMEKEY_INVALID, "WOLLobbyMenuSystem",                 (void*)WOLLobbyMenuSystem },
	{ NAMEKEY_INVALID, "WOLGameSetupMenuSystem",             (void*)WOLGameSetupMenuSystem },
	{ NAMEKEY_INVALID, "WOLMapSelectMenuSystem",             (void*)WOLMapSelectMenuSystem },
	{ NAMEKEY_INVALID, "WOLBuddyOverlaySystem",              (void*)WOLBuddyOverlaySystem },
	{ NAMEKEY_INVALID, "WOLBuddyOverlayRCMenuSystem",        (void*)WOLBuddyOverlayRCMenuSystem },
	{ NAMEKEY_INVALID, "RCGameDetailsMenuSystem",            (void*)RCGameDetailsMenuSystem },
	{ NAMEKEY_INVALID, "GameSpyPlayerInfoOverlaySystem",     (void*)GameSpyPlayerInfoOverlaySystem },
	{ NAMEKEY_INVALID, "WOLMessageWindowSystem",             (void*)WOLMessageWindowSystem },
	{ NAMEKEY_INVALID, "WOLQuickMatchMenuSystem",            (void*)WOLQuickMatchMenuSystem },
	{ NAMEKEY_INVALID, "WOLWelcomeMenuSystem",               (void*)WOLWelcomeMenuSystem },
	{ NAMEKEY_INVALID, "WOLStatusMenuSystem",                (void*)WOLStatusMenuSystem },
	{ NAMEKEY_INVALID, "WOLQMScoreScreenSystem",             (void*)WOLQMScoreScreenSystem },
	{ NAMEKEY_INVALID, "WOLCustomScoreScreenSystem",         (void*)WOLCustomScoreScreenSystem },
	{ NAMEKEY_INVALID, "NetworkDirectConnectSystem",         (void*)NetworkDirectConnectSystem },
	{ NAMEKEY_INVALID, "PopupHostGameSystem",                (void*)PopupHostGameSystem },
	{ NAMEKEY_INVALID, "PopupJoinGameSystem",                (void*)PopupJoinGameSystem },
	{ NAMEKEY_INVALID, "PopupLadderSelectSystem",            (void*)PopupLadderSelectSystem },
	{ NAMEKEY_INVALID, "InGamePopupMessageSystem",           (void*)InGamePopupMessageSystem },
	{ NAMEKEY_INVALID, "ControlBarSystem",                   (void*)ControlBarSystem },
	{ NAMEKEY_INVALID, "ControlBarObserverSystem",           (void*)ControlBarObserverSystem },
	{ NAMEKEY_INVALID, "IMECandidateWindowSystem",           (void*)IMECandidateWindowSystem },
	{ NAMEKEY_INVALID, "ReplayControlSystem",                (void*)ReplayControlSystem },
	{ NAMEKEY_INVALID, "InGameChatSystem",                   (void*)InGameChatSystem },
	{ NAMEKEY_INVALID, "DisconnectControlSystem",            (void*)DisconnectControlSystem },
	{ NAMEKEY_INVALID, "DiplomacySystem",                    (void*)DiplomacySystem },
	{ NAMEKEY_INVALID, "GeneralsExpPointsSystem",            (void*)GeneralsExpPointsSystem },
	{ NAMEKEY_INVALID, "DifficultySelectSystem",             (void*)DifficultySelectSystem },

	{ NAMEKEY_INVALID, "IdleWorkerSystem",                   (void*)IdleWorkerSystem },
	{ NAMEKEY_INVALID, "EstablishConnectionsControlSystem",  (void*)EstablishConnectionsControlSystem },
	{ NAMEKEY_INVALID, "GameInfoWindowSystem",               (void*)GameInfoWindowSystem },
	{ NAMEKEY_INVALID, "ScoreScreenSystem",                  (void*)ScoreScreenSystem },
	{ NAMEKEY_INVALID, "DownloadMenuSystem",                 (void*)DownloadMenuSystem },

	{ NAMEKEY_INVALID, nullptr,                              nullptr }

};

// game window input table ------------------------------------------------------------------------
static FunctionLexicon::TableEntry gameWinInputTable[] =
{

	{ NAMEKEY_INVALID, "GameWinDefaultInput",               (void*)GameWinDefaultInput },
	{ NAMEKEY_INVALID, "GameWinBlockInput",                 (void*)GameWinBlockInput },
	{ NAMEKEY_INVALID, "GadgetPushButtonInput",             (void*)GadgetPushButtonInput },
	{ NAMEKEY_INVALID, "GadgetCheckBoxInput",               (void*)GadgetCheckBoxInput },
	{ NAMEKEY_INVALID, "GadgetRadioButtonInput",            (void*)GadgetRadioButtonInput },
	{ NAMEKEY_INVALID, "GadgetTabControlInput",             (void*)GadgetTabControlInput },
	{ NAMEKEY_INVALID, "GadgetListBoxInput",                (void*)GadgetListBoxInput },
	{ NAMEKEY_INVALID, "GadgetListBoxMultiInput",           (void*)GadgetListBoxMultiInput },
	{ NAMEKEY_INVALID, "GadgetComboBoxInput",               (void*)GadgetComboBoxInput },
	{ NAMEKEY_INVALID, "GadgetHorizontalSliderInput",       (void*)GadgetHorizontalSliderInput },
	{ NAMEKEY_INVALID, "GadgetVerticalSliderInput",         (void*)GadgetVerticalSliderInput },
	{ NAMEKEY_INVALID, "GadgetStaticTextInput",             (void*)GadgetStaticTextInput },
	{ NAMEKEY_INVALID, "GadgetTextEntryInput",              (void*)GadgetTextEntryInput },

	{ NAMEKEY_INVALID, "MainMenuInput",                     (void*)MainMenuInput },
	{ NAMEKEY_INVALID, "MapSelectMenuInput",                (void*)MapSelectMenuInput },
	{ NAMEKEY_INVALID, "OptionsMenuInput",                  (void*)OptionsMenuInput },
	{ NAMEKEY_INVALID, "SinglePlayerMenuInput",             (void*)SinglePlayerMenuInput },
	{ NAMEKEY_INVALID, "LanLobbyMenuInput",                 (void*)LanLobbyMenuInput },
	{ NAMEKEY_INVALID, "ReplayMenuInput",                   (void*)ReplayMenuInput },
	{ NAMEKEY_INVALID, "CreditsMenuInput",                  (void*)CreditsMenuInput },
	{ NAMEKEY_INVALID, "KeyboardOptionsMenuInput",          (void*)KeyboardOptionsMenuInput },
	{ NAMEKEY_INVALID, "PopupCommunicatorInput",            (void*)PopupCommunicatorInput },
	{ NAMEKEY_INVALID, "LanGameOptionsMenuInput",           (void*)LanGameOptionsMenuInput },
	{ NAMEKEY_INVALID, "LanMapSelectMenuInput",             (void*)LanMapSelectMenuInput },
	{ NAMEKEY_INVALID, "SkirmishGameOptionsMenuInput",      (void*)SkirmishGameOptionsMenuInput },
	{ NAMEKEY_INVALID, "SkirmishMapSelectMenuInput",        (void*)SkirmishMapSelectMenuInput },
	{ NAMEKEY_INVALID, "ChallengeMenuInput",                (void*)ChallengeMenuInput },
	{ NAMEKEY_INVALID, "WOLLadderScreenInput",              (void*)WOLLadderScreenInput },
	{ NAMEKEY_INVALID, "WOLLoginMenuInput",                 (void*)WOLLoginMenuInput },
	{ NAMEKEY_INVALID, "WOLLocaleSelectInput",              (void*)WOLLocaleSelectInput },
	{ NAMEKEY_INVALID, "WOLLobbyMenuInput",                 (void*)WOLLobbyMenuInput },
	{ NAMEKEY_INVALID, "WOLGameSetupMenuInput",             (void*)WOLGameSetupMenuInput },
	{ NAMEKEY_INVALID, "WOLMapSelectMenuInput",             (void*)WOLMapSelectMenuInput },
	{ NAMEKEY_INVALID, "WOLBuddyOverlayInput",              (void*)WOLBuddyOverlayInput },
	{ NAMEKEY_INVALID, "GameSpyPlayerInfoOverlayInput",     (void*)GameSpyPlayerInfoOverlayInput },
	{ NAMEKEY_INVALID, "WOLMessageWindowInput",             (void*)WOLMessageWindowInput },
	{ NAMEKEY_INVALID, "WOLQuickMatchMenuInput",            (void*)WOLQuickMatchMenuInput },
	{ NAMEKEY_INVALID, "WOLWelcomeMenuInput",               (void*)WOLWelcomeMenuInput },
	{ NAMEKEY_INVALID, "WOLStatusMenuInput",                (void*)WOLStatusMenuInput },
	{ NAMEKEY_INVALID, "WOLQMScoreScreenInput",             (void*)WOLQMScoreScreenInput },
	{ NAMEKEY_INVALID, "WOLCustomScoreScreenInput",         (void*)WOLCustomScoreScreenInput },
	{ NAMEKEY_INVALID, "NetworkDirectConnectInput",         (void*)NetworkDirectConnectInput },
	{ NAMEKEY_INVALID, "PopupHostGameInput",                (void*)PopupHostGameInput },
	{ NAMEKEY_INVALID, "PopupJoinGameInput",                (void*)PopupJoinGameInput },
	{ NAMEKEY_INVALID, "PopupLadderSelectInput",            (void*)PopupLadderSelectInput },
	{ NAMEKEY_INVALID, "InGamePopupMessageInput",           (void*)InGamePopupMessageInput },
	{ NAMEKEY_INVALID, "ControlBarInput",                   (void*)ControlBarInput },
	{ NAMEKEY_INVALID, "ReplayControlInput",                (void*)ReplayControlInput },
	{ NAMEKEY_INVALID, "InGameChatInput",                   (void*)InGameChatInput },
	{ NAMEKEY_INVALID, "DisconnectControlInput",            (void*)DisconnectControlInput },
	{ NAMEKEY_INVALID, "DiplomacyInput",                    (void*)DiplomacyInput },
	{ NAMEKEY_INVALID, "EstablishConnectionsControlInput",  (void*)EstablishConnectionsControlInput },
	{ NAMEKEY_INVALID, "LeftHUDInput",                      (void*)LeftHUDInput },
	{ NAMEKEY_INVALID, "ScoreScreenInput",                  (void*)ScoreScreenInput },
	{ NAMEKEY_INVALID, "SaveLoadMenuInput",                 (void*)SaveLoadMenuInput },
	{ NAMEKEY_INVALID, "BeaconWindowInput",                 (void*)BeaconWindowInput },
	{ NAMEKEY_INVALID, "DifficultySelectInput",             (void*)DifficultySelectInput },
	{ NAMEKEY_INVALID, "PopupReplayInput",                  (void*)PopupReplayInput },
	{ NAMEKEY_INVALID, "GeneralsExpPointsInput",            (void*)GeneralsExpPointsInput },

	{ NAMEKEY_INVALID, "DownloadMenuInput",                 (void*)DownloadMenuInput },

	{ NAMEKEY_INVALID, "IMECandidateWindowInput",           (void*)IMECandidateWindowInput },
	{ NAMEKEY_INVALID, nullptr,                             nullptr }

};

// game window tooltip table ----------------------------------------------------------------------
static FunctionLexicon::TableEntry gameWinTooltipTable[] =
{


	{ NAMEKEY_INVALID, "GameWinDefaultTooltip",  (void*)GameWinDefaultTooltip },

	{ NAMEKEY_INVALID, nullptr,                  nullptr }

};

// window layout init table -----------------------------------------------------------------------
static FunctionLexicon::TableEntry winLayoutInitTable[] =
{

	{ NAMEKEY_INVALID, "MainMenuInit",                  (void*)MainMenuInit },
	{ NAMEKEY_INVALID, "OptionsMenuInit",               (void*)OptionsMenuInit },
	{ NAMEKEY_INVALID, "SaveLoadMenuInit",              (void*)SaveLoadMenuInit },
	{ NAMEKEY_INVALID, "SaveLoadMenuFullScreenInit",    (void*)SaveLoadMenuFullScreenInit },

	{ NAMEKEY_INVALID, "PopupCommunicatorInit",         (void*)PopupCommunicatorInit },
	{ NAMEKEY_INVALID, "KeyboardOptionsMenuInit",       (void*)KeyboardOptionsMenuInit },
	{ NAMEKEY_INVALID, "SinglePlayerMenuInit",          (void*)SinglePlayerMenuInit },
	{ NAMEKEY_INVALID, "MapSelectMenuInit",             (void*)MapSelectMenuInit },
	{ NAMEKEY_INVALID, "LanLobbyMenuInit",              (void*)LanLobbyMenuInit },
	{ NAMEKEY_INVALID, "ReplayMenuInit",                (void*)ReplayMenuInit },
	{ NAMEKEY_INVALID, "CreditsMenuInit",               (void*)CreditsMenuInit },
	{ NAMEKEY_INVALID, "LanGameOptionsMenuInit",        (void*)LanGameOptionsMenuInit },
	{ NAMEKEY_INVALID, "LanMapSelectMenuInit",          (void*)LanMapSelectMenuInit },
	{ NAMEKEY_INVALID, "SkirmishGameOptionsMenuInit",   (void*)SkirmishGameOptionsMenuInit },
	{ NAMEKEY_INVALID, "SkirmishMapSelectMenuInit",     (void*)SkirmishMapSelectMenuInit },
	{ NAMEKEY_INVALID, "ChallengeMenuInit",             (void*)ChallengeMenuInit },
	{ NAMEKEY_INVALID, "WOLLadderScreenInit",           (void*)WOLLadderScreenInit },
	{ NAMEKEY_INVALID, "WOLLoginMenuInit",              (void*)WOLLoginMenuInit },
	{ NAMEKEY_INVALID, "WOLLocaleSelectInit",           (void*)WOLLocaleSelectInit },
	{ NAMEKEY_INVALID, "WOLLobbyMenuInit",              (void*)WOLLobbyMenuInit },
	{ NAMEKEY_INVALID, "WOLGameSetupMenuInit",          (void*)WOLGameSetupMenuInit },
	{ NAMEKEY_INVALID, "WOLMapSelectMenuInit",          (void*)WOLMapSelectMenuInit },
	{ NAMEKEY_INVALID, "WOLBuddyOverlayInit",           (void*)WOLBuddyOverlayInit },
	{ NAMEKEY_INVALID, "WOLBuddyOverlayRCMenuInit",     (void*)WOLBuddyOverlayRCMenuInit },
	{ NAMEKEY_INVALID, "RCGameDetailsMenuInit",         (void*)RCGameDetailsMenuInit },
	{ NAMEKEY_INVALID, "GameSpyPlayerInfoOverlayInit",  (void*)GameSpyPlayerInfoOverlayInit },
	{ NAMEKEY_INVALID, "WOLMessageWindowInit",          (void*)WOLMessageWindowInit },
	{ NAMEKEY_INVALID, "WOLQuickMatchMenuInit",         (void*)WOLQuickMatchMenuInit },
	{ NAMEKEY_INVALID, "WOLWelcomeMenuInit",            (void*)WOLWelcomeMenuInit },
	{ NAMEKEY_INVALID, "WOLStatusMenuInit",             (void*)WOLStatusMenuInit },
	{ NAMEKEY_INVALID, "WOLQMScoreScreenInit",          (void*)WOLQMScoreScreenInit },
	{ NAMEKEY_INVALID, "WOLCustomScoreScreenInit",      (void*)WOLCustomScoreScreenInit },
	{ NAMEKEY_INVALID, "NetworkDirectConnectInit",      (void*)NetworkDirectConnectInit },
	{ NAMEKEY_INVALID, "PopupHostGameInit",             (void*)PopupHostGameInit },
	{ NAMEKEY_INVALID, "PopupJoinGameInit",             (void*)PopupJoinGameInit },
	{ NAMEKEY_INVALID, "PopupLadderSelectInit",         (void*)PopupLadderSelectInit },
	{ NAMEKEY_INVALID, "InGamePopupMessageInit",        (void*)InGamePopupMessageInit },
	{ NAMEKEY_INVALID, "GameInfoWindowInit",            (void*)GameInfoWindowInit },
	{ NAMEKEY_INVALID, "ScoreScreenInit",               (void*)ScoreScreenInit },
	{ NAMEKEY_INVALID, "DownloadMenuInit",              (void*)DownloadMenuInit },
	{ NAMEKEY_INVALID, "DifficultySelectInit",          (void*)DifficultySelectInit },
	{ NAMEKEY_INVALID, "PopupReplayInit",               (void*)PopupReplayInit },

	{ NAMEKEY_INVALID, nullptr,                         nullptr }

};

// window layout update table ---------------------------------------------------------------------
static FunctionLexicon::TableEntry winLayoutUpdateTable[] =
{

	{ NAMEKEY_INVALID, "MainMenuUpdate",                  (void*)MainMenuUpdate },
	{ NAMEKEY_INVALID, "OptionsMenuUpdate",               (void*)OptionsMenuUpdate },
	{ NAMEKEY_INVALID, "SinglePlayerMenuUpdate",          (void*)SinglePlayerMenuUpdate },
	{ NAMEKEY_INVALID, "MapSelectMenuUpdate",             (void*)MapSelectMenuUpdate },
	{ NAMEKEY_INVALID, "LanLobbyMenuUpdate",              (void*)LanLobbyMenuUpdate },
	{ NAMEKEY_INVALID, "ReplayMenuUpdate",                (void*)ReplayMenuUpdate },
	{ NAMEKEY_INVALID, "SaveLoadMenuUpdate",              (void*)SaveLoadMenuUpdate },

	{ NAMEKEY_INVALID, "CreditsMenuUpdate",               (void*)CreditsMenuUpdate },
	{ NAMEKEY_INVALID, "LanGameOptionsMenuUpdate",        (void*)LanGameOptionsMenuUpdate },
	{ NAMEKEY_INVALID, "LanMapSelectMenuUpdate",          (void*)LanMapSelectMenuUpdate },
	{ NAMEKEY_INVALID, "SkirmishGameOptionsMenuUpdate",   (void*)SkirmishGameOptionsMenuUpdate },
	{ NAMEKEY_INVALID, "SkirmishMapSelectMenuUpdate",     (void*)SkirmishMapSelectMenuUpdate },
	{ NAMEKEY_INVALID, "ChallengeMenuUpdate",             (void*)ChallengeMenuUpdate },
	{ NAMEKEY_INVALID, "WOLLadderScreenUpdate",           (void*)WOLLadderScreenUpdate },
	{ NAMEKEY_INVALID, "WOLLoginMenuUpdate",              (void*)WOLLoginMenuUpdate },
	{ NAMEKEY_INVALID, "WOLLocaleSelectUpdate",           (void*)WOLLocaleSelectUpdate },
	{ NAMEKEY_INVALID, "WOLLobbyMenuUpdate",              (void*)WOLLobbyMenuUpdate },
	{ NAMEKEY_INVALID, "WOLGameSetupMenuUpdate",          (void*)WOLGameSetupMenuUpdate },
	{ NAMEKEY_INVALID, "PopupHostGameUpdate",             (void*)PopupHostGameUpdate },
	{ NAMEKEY_INVALID, "WOLMapSelectMenuUpdate",          (void*)WOLMapSelectMenuUpdate },
	{ NAMEKEY_INVALID, "WOLBuddyOverlayUpdate",           (void*)WOLBuddyOverlayUpdate },
	{ NAMEKEY_INVALID, "GameSpyPlayerInfoOverlayUpdate",  (void*)GameSpyPlayerInfoOverlayUpdate },
	{ NAMEKEY_INVALID, "WOLMessageWindowUpdate",          (void*)WOLMessageWindowUpdate },
	{ NAMEKEY_INVALID, "WOLQuickMatchMenuUpdate",         (void*)WOLQuickMatchMenuUpdate },
	{ NAMEKEY_INVALID, "WOLWelcomeMenuUpdate",            (void*)WOLWelcomeMenuUpdate },
	{ NAMEKEY_INVALID, "WOLStatusMenuUpdate",             (void*)WOLStatusMenuUpdate },
	{ NAMEKEY_INVALID, "WOLQMScoreScreenUpdate",          (void*)WOLQMScoreScreenUpdate },
	{ NAMEKEY_INVALID, "WOLCustomScoreScreenUpdate",      (void*)WOLCustomScoreScreenUpdate },
	{ NAMEKEY_INVALID, "NetworkDirectConnectUpdate",      (void*)NetworkDirectConnectUpdate },
	{ NAMEKEY_INVALID, "ScoreScreenUpdate",               (void*)ScoreScreenUpdate },
	{ NAMEKEY_INVALID, "DownloadMenuUpdate",              (void*)DownloadMenuUpdate },
	{ NAMEKEY_INVALID, "PopupReplayUpdate",               (void*)PopupReplayUpdate },
	{ NAMEKEY_INVALID, nullptr,                           nullptr }

};

// window layout shutdown table -------------------------------------------------------------------
static FunctionLexicon::TableEntry winLayoutShutdownTable[] =
{

	{ NAMEKEY_INVALID, "MainMenuShutdown",                  (void*)MainMenuShutdown },
	{ NAMEKEY_INVALID, "OptionsMenuShutdown",               (void*)OptionsMenuShutdown },
	{ NAMEKEY_INVALID, "SaveLoadMenuShutdown",              (void*)SaveLoadMenuShutdown },
	{ NAMEKEY_INVALID, "PopupCommunicatorShutdown",         (void*)PopupCommunicatorShutdown },
	{ NAMEKEY_INVALID, "KeyboardOptionsMenuShutdown",       (void*)KeyboardOptionsMenuShutdown },
	{ NAMEKEY_INVALID, "SinglePlayerMenuShutdown",          (void*)SinglePlayerMenuShutdown },
	{ NAMEKEY_INVALID, "MapSelectMenuShutdown",             (void*)MapSelectMenuShutdown },
	{ NAMEKEY_INVALID, "LanLobbyMenuShutdown",              (void*)LanLobbyMenuShutdown },
	{ NAMEKEY_INVALID, "ReplayMenuShutdown",                (void*)ReplayMenuShutdown },
	{ NAMEKEY_INVALID, "CreditsMenuShutdown",               (void*)CreditsMenuShutdown },
	{ NAMEKEY_INVALID, "LanGameOptionsMenuShutdown",        (void*)LanGameOptionsMenuShutdown },
	{ NAMEKEY_INVALID, "LanMapSelectMenuShutdown",          (void*)LanMapSelectMenuShutdown },
	{ NAMEKEY_INVALID, "SkirmishGameOptionsMenuShutdown",   (void*)SkirmishGameOptionsMenuShutdown },
	{ NAMEKEY_INVALID, "SkirmishMapSelectMenuShutdown",     (void*)SkirmishMapSelectMenuShutdown },
	{ NAMEKEY_INVALID, "ChallengeMenuShutdown",             (void*)ChallengeMenuShutdown },
	{ NAMEKEY_INVALID, "WOLLadderScreenShutdown",           (void*)WOLLadderScreenShutdown },
	{ NAMEKEY_INVALID, "WOLLoginMenuShutdown",              (void*)WOLLoginMenuShutdown },
	{ NAMEKEY_INVALID, "WOLLocaleSelectShutdown",           (void*)WOLLocaleSelectShutdown },
	{ NAMEKEY_INVALID, "WOLLobbyMenuShutdown",              (void*)WOLLobbyMenuShutdown },
	{ NAMEKEY_INVALID, "WOLGameSetupMenuShutdown",          (void*)WOLGameSetupMenuShutdown },
	{ NAMEKEY_INVALID, "WOLMapSelectMenuShutdown",          (void*)WOLMapSelectMenuShutdown },
	{ NAMEKEY_INVALID, "WOLBuddyOverlayShutdown",           (void*)WOLBuddyOverlayShutdown },
	{ NAMEKEY_INVALID, "GameSpyPlayerInfoOverlayShutdown",  (void*)GameSpyPlayerInfoOverlayShutdown },
	{ NAMEKEY_INVALID, "WOLMessageWindowShutdown",          (void*)WOLMessageWindowShutdown },
	{ NAMEKEY_INVALID, "WOLQuickMatchMenuShutdown",         (void*)WOLQuickMatchMenuShutdown },
	{ NAMEKEY_INVALID, "WOLWelcomeMenuShutdown",            (void*)WOLWelcomeMenuShutdown },
	{ NAMEKEY_INVALID, "WOLStatusMenuShutdown",             (void*)WOLStatusMenuShutdown },
	{ NAMEKEY_INVALID, "WOLQMScoreScreenShutdown",          (void*)WOLQMScoreScreenShutdown },
	{ NAMEKEY_INVALID, "WOLCustomScoreScreenShutdown",      (void*)WOLCustomScoreScreenShutdown },
	{ NAMEKEY_INVALID, "NetworkDirectConnectShutdown",      (void*)NetworkDirectConnectShutdown },
	{ NAMEKEY_INVALID, "ScoreScreenShutdown",               (void*)ScoreScreenShutdown },
	{ NAMEKEY_INVALID, "DownloadMenuShutdown",              (void*)DownloadMenuShutdown },
	{ NAMEKEY_INVALID, "PopupReplayShutdown",               (void*)PopupReplayShutdown },
	{ NAMEKEY_INVALID, nullptr,                             nullptr }

};

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC DATA
///////////////////////////////////////////////////////////////////////////////////////////////////
FunctionLexicon *TheFunctionLexicon = nullptr;  ///< the function dictionary

//-------------------------------------------------------------------------------------------------
/** Since we have a convenient table to organize our callbacks anyway,
	* we'll just use this same storage space to load in any run time
	* components we might want to add to the table, such as generating
	* a key based off the name supplied in the table for faster access */
//-------------------------------------------------------------------------------------------------
void FunctionLexicon::loadTable( TableEntry *table,
																 TableIndex tableIndex )
{

	// sanity
	if( table == nullptr )
		return;

	// loop through all entries
	TableEntry *entry = table;
	while( entry->name )
	{

		// assign key from name key based on name provided in table
		entry->key = TheNameKeyGenerator->nameToKey( entry->name );

		// next table entry please
		entry++;

	}

	// assign table to the index specified
	m_tables[ tableIndex ] = table;

}

//-------------------------------------------------------------------------------------------------
/** Search the provided table for a function matching the key */
//-------------------------------------------------------------------------------------------------
void *FunctionLexicon::keyToFunc( NameKeyType key, TableEntry *table )
{

	// sanity
	if( key == NAMEKEY_INVALID )
		return nullptr;

	// search table for key
	TableEntry *entry = table;
	while( entry && entry->key != NAMEKEY_INVALID )
	{

		if( entry->key == key )
			return entry->func;
		entry++;

	}

	return nullptr;  // not found

}

//-------------------------------------------------------------------------------------------------
/** Search tables for the function given this key, if the index parameter
	* is TABLE_ANY, then ALL tables will be searched.  Otherwise index refers
	* to only a single table index to be searched */
//-------------------------------------------------------------------------------------------------
void *FunctionLexicon::findFunction( NameKeyType key, TableIndex index )
{
	void *func = nullptr;

	// sanity
	if( key == NAMEKEY_INVALID )
		return nullptr;

	// search ALL tables for function if the index parameter allows if
	if( index == TABLE_ANY )
	{

		Int i;
		for( i = 0; i < MAX_FUNCTION_TABLES; i++ )
		{

			func = keyToFunc( key, m_tables[ i ] );
			if( func )
				break;  // exit for i

		}

	}
	else
	{

		// do NOT search all tables, just the one specified by the parameter
		func = keyToFunc( key, m_tables[ index ] );

	}

	// return function, if found
	return func;

}

#ifdef NOT_IN_USE
//-------------------------------------------------------------------------------------------------
/** Search for the function in the specified table */
//-------------------------------------------------------------------------------------------------
const char *FunctionLexicon::funcToName( void *func, TableEntry *table )
{

	// sanity
	if( func == nullptr )
		return nullptr;

	// search the table
	TableEntry *entry = table;
	while( entry && entry->key != NAMEKEY_INVALID )
	{

		// is this it
		if( entry->func == func )
			return entry->name;

		// not it, check next
		entry++;

	}

	return nullptr;  // not found

}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
FunctionLexicon::FunctionLexicon( void )
{
	Int i;

	// empty the tables
	for( i = 0; i < MAX_FUNCTION_TABLES; i++ )
		m_tables[ i ] = nullptr;

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
FunctionLexicon::~FunctionLexicon( void )
{

}

//-------------------------------------------------------------------------------------------------
/** Initialize our dictionary of function pointers and symbols */
//-------------------------------------------------------------------------------------------------
void FunctionLexicon::init( void )
{

	// if you change this method, check the reset() and make sure it's OK

	// initialize the name key identifiers for the lookup table
	loadTable( gameWinDrawTable,						TABLE_GAME_WIN_DRAW );
	loadTable( gameWinSystemTable,					TABLE_GAME_WIN_SYSTEM );
	loadTable( gameWinInputTable,						TABLE_GAME_WIN_INPUT );
	loadTable( gameWinTooltipTable,					TABLE_GAME_WIN_TOOLTIP );

	loadTable( winLayoutInitTable,					TABLE_WIN_LAYOUT_INIT );
	loadTable( winLayoutUpdateTable,				TABLE_WIN_LAYOUT_UPDATE );
	loadTable( winLayoutShutdownTable,			TABLE_WIN_LAYOUT_SHUTDOWN );

	validate();

}

//-------------------------------------------------------------------------------------------------
/** reset */
//-------------------------------------------------------------------------------------------------
void FunctionLexicon::reset( void )
{

	//
	// make sure the ordering of what happens here with respect to derived classes resets is
	// all OK since we're cheating and using the init() method
	//

	// nothing dynamically loaded, just reinit the tables
	init();

}

//-------------------------------------------------------------------------------------------------
/** Update */
//-------------------------------------------------------------------------------------------------
void FunctionLexicon::update( void )
{

}

/*
// !NOTE! We can not have this function, see the header for
// more information as to why
//
//-------------------------------------------------------------------------------------------------
// translate a function pointer to its symbolic name
//-------------------------------------------------------------------------------------------------
char *FunctionLexicon::functionToName( void *func )
{

	// sanity
	if( func == nullptr )
		return nullptr;

	// search ALL the tables
	Int i;
	char *name = nullptr;
	for( i = 0; i < MAX_FUNCTION_TABLES; i++ )
	{

		name = funcToName( func, m_tables[ i ] );
		if( name )
			return name;

	}

	return nullptr;  // not found

}
*/

//-------------------------------------------------------------------------------------------------
/** Scan the tables and make sure that each function address is unique.
	* We want to do this to prevent accidental entries of two identical
	* functions and because the compiler will optimize identical functions
	* to the same address (typically in empty functions with no body and the
	* same parameters) which we MUST keep separate for when we add code to
	* them */
//-------------------------------------------------------------------------------------------------
Bool FunctionLexicon::validate( void )
{
	Bool valid = TRUE;
	Int i, j;
	TableEntry *sourceEntry, *lookAtEntry;

	// scan all tables
	for( i = 0; i < MAX_FUNCTION_TABLES; i++ )
	{

		// scan through this table
		sourceEntry = m_tables[ i ];
		while( sourceEntry && sourceEntry->key != NAMEKEY_INVALID )
		{

			//
			// scan all tables looking for the function in sourceEntry, do not bother
			// of source entry is nullptr (a valid entry in the table, but not a function)
			//
			if( sourceEntry->func )
			{

				// scan all tables
				for( j = 0; j < MAX_FUNCTION_TABLES; j++ )
				{

					// scan all entries in this table
					lookAtEntry = m_tables[ j ];
					while( lookAtEntry && lookAtEntry->key != NAMEKEY_INVALID )
					{

						//
						// check for match, do not match the entry source with itself
						//
						if( sourceEntry != lookAtEntry )
							if( sourceEntry->func == lookAtEntry->func )
							{

								DEBUG_LOG(( "WARNING! Function lexicon entries match same address! '%s' and '%s'",
														sourceEntry->name, lookAtEntry->name ));
								valid = FALSE;

							}

						// next entry in this target table
						lookAtEntry++;

					}

				}

			}

			// next source entry
			sourceEntry++;

		}

	}

	// return the valid state of our tables
	return valid;

}

//============================================================================
// FunctionLexicon::gameWinDrawFunc
//============================================================================

GameWinDrawFunc FunctionLexicon::gameWinDrawFunc( NameKeyType key, TableIndex index )
{
	if ( index == TABLE_ANY )
	{
		// first search the device depended table then the device independent table
		GameWinDrawFunc func;

		func = (GameWinDrawFunc)findFunction( key, TABLE_GAME_WIN_DEVICEDRAW );
		if ( func == nullptr )
		{
			func = (GameWinDrawFunc)findFunction( key, TABLE_GAME_WIN_DRAW );
		}
		return func;
	}
	// search the specified table
	return (GameWinDrawFunc)findFunction( key, index );
}

WindowLayoutInitFunc FunctionLexicon::winLayoutInitFunc( NameKeyType key, TableIndex index )
{
	if ( index == TABLE_ANY )
	{
		// first search the device depended table then the device independent table
		WindowLayoutInitFunc func;

		func = (WindowLayoutInitFunc)findFunction( key, TABLE_WIN_LAYOUT_DEVICEINIT );
		if ( func == nullptr )
		{
			func = (WindowLayoutInitFunc)findFunction( key, TABLE_WIN_LAYOUT_INIT );
		}
		return func;
	}
	// search the specified table
	return (WindowLayoutInitFunc)findFunction( key, index );
}
