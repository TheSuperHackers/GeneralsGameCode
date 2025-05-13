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

// FILE: GamespyOverlay.h //////////////////////////////////////////////////////
// Generals GameSpy overlay screens
// Author: Matthew D. Campbell, March 2002

#pragma once

#ifndef __GAMESPYOVERLAY_H__
#define __GAMESPYOVERLAY_H__

#include "Common/NameKeyGenerator.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/Shell.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/GameWindowManager.h"

void ClearGSMessageBoxes( void );	///< Tear down any GS message boxes (e.g. in case we have a new one to put up)
void GSMessageBoxOk(UnicodeString titleString,UnicodeString bodyString, GameWinMsgBoxFunc okFunc = NULL);	///< Display a Message box with Ok button and track it
void GSMessageBoxOkCancel(UnicodeString title, UnicodeString message, GameWinMsgBoxFunc okFunc, GameWinMsgBoxFunc cancelFunc);	///< Display a Message box with Ok/Cancel buttons and track it
void GSMessageBoxYesNo(UnicodeString title, UnicodeString message, GameWinMsgBoxFunc yesFunc, GameWinMsgBoxFunc noFunc);	///< Display a Message box with Yes/No buttons and track it
void RaiseGSMessageBox( void );		///< Bring GS message box to the foreground (if we transition screens while a message box is up)

enum GSOverlayType CPP_11(: Int)
{
	GSOVERLAY_PLAYERINFO,
	GSOVERLAY_MAPSELECT,
	GSOVERLAY_BUDDY,
	GSOVERLAY_PAGE,
	GSOVERLAY_GAMEOPTIONS,
	GSOVERLAY_GAMEPASSWORD,
	GSOVERLAY_LADDERSELECT,
	GSOVERLAY_LOCALESELECT,
	GSOVERLAY_OPTIONS,
	GSOVERLAY_MAX
};

void GameSpyOpenOverlay( GSOverlayType );
void GameSpyCloseOverlay( GSOverlayType );
void GameSpyCloseAllOverlays( void );
Bool GameSpyIsOverlayOpen( GSOverlayType );
void GameSpyToggleOverlay( GSOverlayType );
void GameSpyUpdateOverlays( void );
void ReOpenPlayerInfo( void );
void CheckReOpenPlayerInfo(void );
#endif // __GAMESPYOVERLAY_H__
