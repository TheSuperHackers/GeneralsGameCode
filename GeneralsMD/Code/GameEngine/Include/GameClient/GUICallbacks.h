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

// FILE: GUICallbacks.h ///////////////////////////////////////////////////////////////////////////
// Created:    Colin Day, June 2001
// Desc:       GUI Callbacks header
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "GameClient/GameWindow.h"

// EXTERNALS //////////////////////////////////////////////////////////////////////////////////////

//=================================================================================================
//																				Shell Menus																						 //
//=================================================================================================

// Main Menu --------------------------------------------------------------------------------------
extern void MainMenuInit( WindowLayout *layout, void *userData );
extern void MainMenuUpdate( WindowLayout *layout, void *userData );
extern void MainMenuShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType MainMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType MainMenuInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Single Player Menu -----------------------------------------------------------------------------
extern void SinglePlayerMenuInit( WindowLayout *layout, void *userData );
extern void SinglePlayerMenuUpdate( WindowLayout *layout, void *userData );
extern void SinglePlayerMenuShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType SinglePlayerMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType SinglePlayerMenuInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Options Menu -----------------------------------------------------------------------------------
extern void OptionsMenuInit( WindowLayout *layout, void *userData );
extern void OptionsMenuUpdate( WindowLayout *layout, void *userData );
extern void OptionsMenuShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType OptionsMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType OptionsMenuInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Map Select Menu --------------------------------------------------------------------------------
extern void MapSelectMenuInit( WindowLayout *layout, void *userData );
extern void MapSelectMenuUpdate( WindowLayout *layout, void *userData );
extern void MapSelectMenuShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType MapSelectMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType MapSelectMenuInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Replay Menu --------------------------------------------------------------------------------
extern void ReplayMenuInit( WindowLayout *layout, void *userData );
extern void ReplayMenuUpdate( WindowLayout *layout, void *userData );
extern void ReplayMenuShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType ReplayMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType ReplayMenuInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Credits Menu --------------------------------------------------------------------------------
extern void CreditsMenuInit( WindowLayout *layout, void *userData );
extern void CreditsMenuUpdate( WindowLayout *layout, void *userData );
extern void CreditsMenuShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType CreditsMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType CreditsMenuInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );


// Score Screen --------------------------------------------------------------------------------
extern void ScoreScreenInit( WindowLayout *layout, void *userData );
extern void ScoreScreenUpdate( WindowLayout *layout, void *userData );
extern void ScoreScreenShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType ScoreScreenSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType ScoreScreenInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Save/Load Menu ----------------------------------------------------------------------------------
extern void SaveLoadMenuInit( WindowLayout *layout, void *userData );
extern void SaveLoadMenuFullScreenInit( WindowLayout *layout, void *userData );
extern void SaveLoadMenuUpdate( WindowLayout *layout, void *userData );
extern void SaveLoadMenuShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType SaveLoadMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType SaveLoadMenuInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Popup Communicator ------------------------------------------------------------------------------
extern void PopupCommunicatorInit( WindowLayout *layout, void *userData );
extern void PopupCommunicatorUpdate( WindowLayout *layout, void *userData );
extern void PopupCommunicatorShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType PopupCommunicatorSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType PopupCommunicatorInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// SkirmishGameOptionsMenu
extern void SkirmishGameOptionsMenuInit( WindowLayout *layout, void *userData );
extern void SkirmishGameOptionsMenuUpdate( WindowLayout *layout, void *userData );
extern void SkirmishGameOptionsMenuShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType SkirmishGameOptionsMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType SkirmishGameOptionsMenuInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// SkirmishMapSelectMenu
extern void SkirmishMapSelectMenuInit( WindowLayout *layout, void *userData );
extern void SkirmishMapSelectMenuUpdate( WindowLayout *layout, void *userData );
extern void SkirmishMapSelectMenuShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType SkirmishMapSelectMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType SkirmishMapSelectMenuInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Generals' Challenge Mode Menu
extern void ChallengeMenuInit( WindowLayout *layout, void *userData );
extern void ChallengeMenuUpdate( WindowLayout *layout, void *userData );
extern void ChallengeMenuShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType ChallengeMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType ChallengeMenuInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Keyboard Options Menu ---------------------------------------------------------------------------
extern void KeyboardOptionsMenuInit( WindowLayout *layout, void *userData );
extern void KeyboardOptionsMenuUpdate( WindowLayout *layout, void *userData );
extern void KeyboardOptionsMenuShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType KeyboardOptionsMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType KeyboardOptionsMenuInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Lan Lobby Menu ----------------------------------------------------------------------------------
extern void LanLobbyMenuInit( WindowLayout *layout, void *userData );
extern void LanLobbyMenuUpdate( WindowLayout *layout, void *userData );
extern void LanLobbyMenuShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType LanLobbyMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType LanLobbyMenuInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Lan Game Options Menu ---------------------------------------------------------------------------
extern void LanGameOptionsMenuInit( WindowLayout *layout, void *userData );
extern void LanGameOptionsMenuUpdate( WindowLayout *layout, void *userData );
extern void LanGameOptionsMenuShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType LanGameOptionsMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType LanGameOptionsMenuInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Lan Map Select Menu -----------------------------------------------------------------------------
extern void LanMapSelectMenuInit( WindowLayout *layout, void *userData );
extern void LanMapSelectMenuUpdate( WindowLayout *layout, void *userData );
extern void LanMapSelectMenuShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType LanMapSelectMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType LanMapSelectMenuInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Lan Map Select Menu -----------------------------------------------------------------------------
extern void GameInfoWindowInit( WindowLayout *layout, void *userData );
extern WindowMsgHandledType GameInfoWindowSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// (Patch) Download Menu ----------------------------------------------------------------------------------
extern void DownloadMenuInit( WindowLayout *layout, void *userData );
extern void DownloadMenuUpdate( WindowLayout *layout, void *userData );
extern void DownloadMenuShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType DownloadMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType DownloadMenuInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Popup host Game Internet -----------------------------------------------------------------------------------
extern void DifficultySelectInit( WindowLayout *layout, void *userData );
extern WindowMsgHandledType DifficultySelectSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType DifficultySelectInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );


//=================================================================================================
//																				WOL UI																								 //
//=================================================================================================
//  WOL Ladder Screen ---------------------------------------------------------------------------------
extern void WOLLadderScreenInit( WindowLayout *layout, void *userData );
extern void WOLLadderScreenUpdate( WindowLayout *layout, void *userData );
extern void WOLLadderScreenShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType WOLLadderScreenSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType WOLLadderScreenInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

//  WOL Login Menu ---------------------------------------------------------------------------------
extern void WOLLoginMenuInit( WindowLayout *layout, void *userData );
extern void WOLLoginMenuUpdate( WindowLayout *layout, void *userData );
extern void WOLLoginMenuShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType WOLLoginMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType WOLLoginMenuInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

//  WOL Locale Select Popup ---------------------------------------------------------------------------------
extern void WOLLocaleSelectInit( WindowLayout *layout, void *userData );
extern void WOLLocaleSelectUpdate( WindowLayout *layout, void *userData );
extern void WOLLocaleSelectShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType WOLLocaleSelectSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType WOLLocaleSelectInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// WOL Message Window ------------------------------------------------------------------------------
extern void WOLMessageWindowInit( WindowLayout *layout, void *userData );
extern void WOLMessageWindowUpdate( WindowLayout *layout, void *userData );
extern void WOLMessageWindowShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType WOLMessageWindowSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType WOLMessageWindowInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// WOL Quick Match Menu ----------------------------------------------------------------------------
extern void WOLQuickMatchMenuInit( WindowLayout *layout, void *userData );
extern void WOLQuickMatchMenuUpdate( WindowLayout *layout, void *userData );
extern void WOLQuickMatchMenuShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType WOLQuickMatchMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType WOLQuickMatchMenuInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// WOL Welcome Menu --------------------------------------------------------------------------------
extern void WOLWelcomeMenuInit( WindowLayout *layout, void *userData );
extern void WOLWelcomeMenuUpdate( WindowLayout *layout, void *userData );
extern void WOLWelcomeMenuShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType WOLWelcomeMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType WOLWelcomeMenuInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// WOL Welcome Menu --------------------------------------------------------------------------------
extern void WOLStatusMenuInit( WindowLayout *layout, void *userData );
extern void WOLStatusMenuUpdate( WindowLayout *layout, void *userData );
extern void WOLStatusMenuShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType WOLStatusMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType WOLStatusMenuInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// WOL Quickmatch Score Screen --------------------------------------------------------------------------------
extern void WOLQMScoreScreenInit( WindowLayout *layout, void *userData );
extern void WOLQMScoreScreenUpdate( WindowLayout *layout, void *userData );
extern void WOLQMScoreScreenShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType WOLQMScoreScreenSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType WOLQMScoreScreenInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

//  WOL Lobby Menu ---------------------------------------------------------------------------------
extern void WOLLobbyMenuInit( WindowLayout *layout, void *userData );
extern void WOLLobbyMenuUpdate( WindowLayout *layout, void *userData );
extern void WOLLobbyMenuShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType WOLLobbyMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType WOLLobbyMenuInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

//  WOL Game Setup Menu ---------------------------------------------------------------------------------
extern void WOLGameSetupMenuInit( WindowLayout *layout, void *userData );
extern void WOLGameSetupMenuUpdate( WindowLayout *layout, void *userData );
extern void WOLGameSetupMenuShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType WOLGameSetupMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType WOLGameSetupMenuInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// WOL Custom Score Screen --------------------------------------------------------------------------------
extern void WOLCustomScoreScreenInit( WindowLayout *layout, void *userData );
extern void WOLCustomScoreScreenUpdate( WindowLayout *layout, void *userData );
extern void WOLCustomScoreScreenShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType WOLCustomScoreScreenSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType WOLCustomScoreScreenInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

//  WOL Map Select Overlay ---------------------------------------------------------------------------------
extern void WOLMapSelectMenuInit( WindowLayout *layout, void *userData );
extern void WOLMapSelectMenuUpdate( WindowLayout *layout, void *userData );
extern void WOLMapSelectMenuShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType WOLMapSelectMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType WOLMapSelectMenuInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

//  WOL Buddy Overlay ---------------------------------------------------------------------------------
extern void WOLBuddyOverlayInit( WindowLayout *layout, void *userData );
extern void WOLBuddyOverlayUpdate( WindowLayout *layout, void *userData );
extern void WOLBuddyOverlayShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType WOLBuddyOverlaySystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType WOLBuddyOverlayInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// WOL Buddy Overlay Right Click menu callbacks --------------------------------------------------------------
extern void WOLBuddyOverlayRCMenuInit( WindowLayout *layout, void *userData );
extern WindowMsgHandledType WOLBuddyOverlayRCMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

//  GameSpy Player Info Overlay ---------------------------------------------------------------------------------
extern void GameSpyPlayerInfoOverlayInit( WindowLayout *layout, void *userData );
extern void GameSpyPlayerInfoOverlayUpdate( WindowLayout *layout, void *userData );
extern void GameSpyPlayerInfoOverlayShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType GameSpyPlayerInfoOverlaySystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType GameSpyPlayerInfoOverlayInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Popup host Game Internet -----------------------------------------------------------------------------------
extern void PopupHostGameInit( WindowLayout *layout, void *userData );
extern void PopupHostGameUpdate( WindowLayout * layout, void *userData);
extern WindowMsgHandledType PopupHostGameSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType PopupHostGameInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Popup InGame Message Box -----------------------------------------------------------------------------------
extern void InGamePopupMessageInit( WindowLayout *layout, void *userData );
extern WindowMsgHandledType InGamePopupMessageSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType InGamePopupMessageInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Popup join Game Internet -----------------------------------------------------------------------------------
extern void PopupJoinGameInit( WindowLayout *layout, void *userData );
extern WindowMsgHandledType PopupJoinGameSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType PopupJoinGameInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

//  Network Direct ConnectWindow ---------------------------------------------------------------------------------
extern void NetworkDirectConnectInit( WindowLayout *layout, void *userData );
extern void NetworkDirectConnectUpdate( WindowLayout *layout, void *userData );
extern void NetworkDirectConnectShutdown( WindowLayout *layout, void *userData );
extern WindowMsgHandledType NetworkDirectConnectSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType NetworkDirectConnectInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

//=================================================================================================
//																				IME UI																								 //
//=================================================================================================

// IME Candidate Window
extern WindowMsgHandledType IMECandidateWindowSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType IMECandidateWindowInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern void IMECandidateMainDraw( GameWindow *window, WinInstanceData *instData );
extern void IMECandidateTextAreaDraw( GameWindow *window, WinInstanceData *instData );


//=================================================================================================
//																				In Game UI																						 //
//=================================================================================================

// Control Bar ------------------------------------------------------------------------------------
extern WindowMsgHandledType ControlBarSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType ControlBarObserverSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType ControlBarInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType LeftHUDInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern void ToggleControlBar( Bool immediate = TRUE );
extern void HideControlBar( Bool immediate = TRUE );
extern void ShowControlBar( Bool immediate = TRUE );


// Replay Controls --------------------------------------------------------------------------------
extern WindowMsgHandledType ReplayControlSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType ReplayControlInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );


// InGame Chat Controls --------------------------------------------------------------------------------
extern WindowMsgHandledType InGameChatSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType InGameChatInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
enum InGameChatType CPP_11(: Int)
{
	INGAME_CHAT_ALLIES,
	INGAME_CHAT_EVERYONE,
	INGAME_CHAT_PLAYERS
};
extern void ToggleInGameChat( Bool immediate = TRUE );
extern void HideInGameChat( Bool immediate = TRUE );
extern void ShowInGameChat( Bool immediate = TRUE );
void ResetInGameChat( void );
void SetInGameChatType( InGameChatType chatType );
Bool IsInGameChatActive();


// Diplomacy Controls --------------------------------------------------------------------------------
WindowMsgHandledType DiplomacySystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
WindowMsgHandledType DiplomacyInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
void ToggleDiplomacy( Bool immediate = TRUE );
void HideDiplomacy( Bool immediate = TRUE );
void ResetDiplomacy( void );

// Generals Exp Points  --------------------------------------------------------------------------------
WindowMsgHandledType GeneralsExpPointsSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
WindowMsgHandledType GeneralsExpPointsInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );


// IdleWorker Controls --------------------------------------------------------------------------------
WindowMsgHandledType IdleWorkerSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Disconnect Menu --------------------------------------------------------------------------------
extern void ShowDisconnectWindow( void );
extern void HideDisconnectWindow( void );
extern WindowMsgHandledType DisconnectControlSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType DisconnectControlInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Establish Connections Window -------------------------------------------------------------------
extern void ShowEstablishConnectionsWindow( void );
extern void HideEstablishConnectionsWindow( void );
extern WindowMsgHandledType EstablishConnectionsControlSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType EstablishConnectionsControlInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// The in game quit menu --------------------------------------------------------------------------
extern void destroyQuitMenu();
extern void ToggleQuitMenu( void );
extern void HideQuitMenu( void );
extern WindowMsgHandledType QuitMenuSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Message of the Day -----------------------------------------------------------------------------
extern WindowMsgHandledType MOTDSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );

// Message Box --------------------------------------------------------------------------------
extern WindowMsgHandledType MessageBoxSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
extern WindowMsgHandledType QuitMessageBoxSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 );
