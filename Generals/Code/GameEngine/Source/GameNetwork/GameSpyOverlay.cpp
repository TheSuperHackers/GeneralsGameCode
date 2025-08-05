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

// FILE: wolscreens.cpp //////////////////////////////////////////////////////
// Westwood Online screen setup/teardown
// Author: Matthew D. Campbell, November 2001

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#include "Common/AudioEventRTS.h"

#include "GameClient/GadgetListBox.h"
#include "GameClient/GameText.h"
#include "GameClient/MessageBox.h"
#include "GameClient/ShellHooks.h"
//#include "GameNetwork/GameSpy.h"
//#include "GameNetwork/GameSpyGP.h"

#include "GameNetwork/GameSpyOverlay.h"
//#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/GameSpy/BuddyThread.h"

void deleteNotificationBox( void );
static void raiseOverlays( void );

// Message boxes -------------------------------------
static GameWindow *messageBoxWindow = NULL;
static GameWinMsgBoxFunc okFunc = NULL;
static GameWinMsgBoxFunc cancelFunc = NULL;
static Bool reOpenPlayerInfoFlag = FALSE;
/**
	* messageBoxOK is called when a message box is destroyed
	* by way of an OK button, so we can clear our pointers to it.
	*/
static void messageBoxOK( void )
{
	DEBUG_ASSERTCRASH(messageBoxWindow, ("Message box window went away without being there in the first place!"));
	messageBoxWindow = NULL;
	if (okFunc)
	{
		okFunc();
		okFunc = NULL;
	}
}

/**
	* messageBoxCancel is called when a message box is destroyed
	* by way of a Cancel button, so we can clear our pointers to it.
	*/
static void messageBoxCancel( void )
{
	DEBUG_ASSERTCRASH(messageBoxWindow, ("Message box window went away without being there in the first place!"));
	messageBoxWindow = NULL;
	if (cancelFunc)
	{
		cancelFunc();
		cancelFunc = NULL;
	}
}

/**
	* clearGSMessageBoxes removes the current message box if
	* one is present.  This is usually done when putting up a
	* second messageBox.
	*/
void ClearGSMessageBoxes( void )
{
	if (messageBoxWindow)
	{
		TheWindowManager->winDestroy(messageBoxWindow);
		messageBoxWindow = NULL;
	}

	if (okFunc)
	{
		okFunc = NULL;
	}

	if (cancelFunc)
	{
		cancelFunc = NULL;
	}
}

/**
	* GSMessageBoxOk puts up an OK dialog box and saves the
	* pointers to it and its callbacks.
	*/
void GSMessageBoxOk(UnicodeString title, UnicodeString message, GameWinMsgBoxFunc newOkFunc)
{
	ClearGSMessageBoxes();
	messageBoxWindow = MessageBoxOk(title, message, messageBoxOK);
	okFunc = newOkFunc;
}

/**
	* GSMessageBoxOkCancel puts up an OK/Cancel dialog box and saves the
	* pointers to it and its callbacks.
	*/
void GSMessageBoxOkCancel(UnicodeString title, UnicodeString message, GameWinMsgBoxFunc newOkFunc, GameWinMsgBoxFunc newCancelFunc)
{
	ClearGSMessageBoxes();
	messageBoxWindow = MessageBoxOkCancel(title, message, messageBoxOK, messageBoxCancel);
	okFunc = newOkFunc;
	cancelFunc = newCancelFunc;
}

/**
	* GSMessageBoxYesNo puts up a Yes/No dialog box and saves the
	* pointers to it and its callbacks.
	*/
void GSMessageBoxYesNo(UnicodeString title, UnicodeString message, GameWinMsgBoxFunc newYesFunc, GameWinMsgBoxFunc newNoFunc)
{
	ClearGSMessageBoxes();
	messageBoxWindow = MessageBoxYesNo(title, message, messageBoxOK, messageBoxCancel);
	okFunc = newYesFunc;
	cancelFunc = newNoFunc;
}

/**
	* If the screen transitions underneath the dialog box, we
	* need to raise it to keep it visible.
	*/
void RaiseGSMessageBox( void )
{
	raiseOverlays();

	if (!messageBoxWindow)
		return;

	messageBoxWindow->winBringToTop();
}

// Overlay screens -------------------------------------

/**
	* gsOverlays holds a list of the .wnd files used in GS overlays.
	* The entries *MUST* be in the same order as the GSOverlayType enum.
	*/
static const char * gsOverlays[GSOVERLAY_MAX] =
{
	"Menus/PopupPlayerInfo.wnd",	// Player info (right-click)
	"Menus/WOLMapSelectMenu.wnd",	// Map select
	"Menus/WOLBuddyOverlay.wnd",	// Buddy list
	"Menus/WOLPageOverlay.wnd",		// Find/page
	"Menus/PopupHostGame.wnd",		// Hosting options (game name, password, etc)
	"Menus/PopupJoinGame.wnd",		// Joining options (password, etc)
	"Menus/PopupLadderSelect.wnd",// LadderSelect
	"Menus/PopupLocaleSelect.wnd",// Prompt for user's locale
	"Menus/OptionsMenu.wnd",			// popup options
};

static WindowLayout *overlayLayouts[GSOVERLAY_MAX] =
{
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
};

static void buddyTryReconnect( void )
{
	BuddyRequest req;
	req.buddyRequestType = BuddyRequest::BUDDYREQUEST_RELOGIN;
	TheGameSpyBuddyMessageQueue->addRequest( req );
}

void GameSpyOpenOverlay( GSOverlayType overlay )
{
	if (overlay == GSOVERLAY_BUDDY)
	{
		if (!TheGameSpyBuddyMessageQueue->isConnected())
		{
			// not connected - is it because we were disconnected?
			if (TheGameSpyBuddyMessageQueue->getLocalProfileID())
			{
				// used to be connected
				GSMessageBoxYesNo(TheGameText->fetch("GUI:GPErrorTitle"), TheGameText->fetch("GUI:GPDisconnected"), buddyTryReconnect, NULL);
			}
			else
			{
				// no profile
				GSMessageBoxOk(TheGameText->fetch("GUI:GPErrorTitle"), TheGameText->fetch("GUI:GPNoProfile"), NULL);
			}
			return;
		}
		AudioEventRTS buttonClick("GUICommunicatorOpen");

		if( TheAudio )
		{
			TheAudio->addAudioEvent( &buttonClick );
		}  // end if
	}
	if (overlayLayouts[overlay])
	{
		overlayLayouts[overlay]->hide( FALSE );
		overlayLayouts[overlay]->bringForward();
	}
	else
	{
		overlayLayouts[overlay] = TheWindowManager->winCreateLayout( AsciiString( gsOverlays[overlay] ) );
		overlayLayouts[overlay]->runInit();
		overlayLayouts[overlay]->hide( FALSE );
		overlayLayouts[overlay]->bringForward();
	}
}

void GameSpyCloseOverlay( GSOverlayType overlay )
{
	switch(overlay)
	{
		case GSOVERLAY_PLAYERINFO:
			DEBUG_LOG(("Closing overlay GSOVERLAY_PLAYERINFO"));
			break;
		case GSOVERLAY_MAPSELECT:
			DEBUG_LOG(("Closing overlay GSOVERLAY_MAPSELECT"));
			break;
		case GSOVERLAY_BUDDY:
			DEBUG_LOG(("Closing overlay GSOVERLAY_BUDDY"));
			break;
		case GSOVERLAY_PAGE:
			DEBUG_LOG(("Closing overlay GSOVERLAY_PAGE"));
			break;
		case GSOVERLAY_GAMEOPTIONS:
			DEBUG_LOG(("Closing overlay GSOVERLAY_GAMEOPTIONS"));
			break;
		case GSOVERLAY_GAMEPASSWORD:
			DEBUG_LOG(("Closing overlay GSOVERLAY_GAMEPASSWORD"));
			break;
		case GSOVERLAY_LADDERSELECT:
			DEBUG_LOG(("Closing overlay GSOVERLAY_LADDERSELECT"));
			break;
		case GSOVERLAY_OPTIONS:
			DEBUG_LOG(("Closing overlay GSOVERLAY_OPTIONS"));
			if( overlayLayouts[overlay] )
			{
				SignalUIInteraction(SHELL_SCRIPT_HOOK_OPTIONS_CLOSED);
			}
			break;
	}
	if( overlayLayouts[overlay] )
	{
		overlayLayouts[overlay]->runShutdown();
		overlayLayouts[overlay]->destroyWindows();
		deleteInstance(overlayLayouts[overlay]);
		overlayLayouts[overlay] = NULL;
	}
}

Bool GameSpyIsOverlayOpen( GSOverlayType overlay )
{
	return (overlayLayouts[overlay] != NULL);
}

void GameSpyToggleOverlay( GSOverlayType overlay )
{
	if (GameSpyIsOverlayOpen(overlay))
		GameSpyCloseOverlay(overlay);
	else
		GameSpyOpenOverlay(overlay);
}

void raiseOverlays( void )
{
	for (int i=0; i<GSOVERLAY_MAX; ++i)
	{
		if (overlayLayouts[(GSOverlayType)i])
		{
			overlayLayouts[(GSOverlayType)i]->bringForward();
		}
	}
}

void GameSpyCloseAllOverlays( void )
{
	for (int i=0; i<GSOVERLAY_MAX; ++i)
	{
		GameSpyCloseOverlay((GSOverlayType)i);
	}

	// if we're shutting down the rest, chances are we don't want this popping up.
	deleteNotificationBox();
}

void GameSpyUpdateOverlays( void )
{
	for (int i=0; i<GSOVERLAY_MAX; ++i)
	{
		if (overlayLayouts[(GSOverlayType)i])
		{
			overlayLayouts[(GSOverlayType)i]->runUpdate();
		}
	}
}

void ReOpenPlayerInfo( void )
{
	reOpenPlayerInfoFlag = TRUE;
}
void CheckReOpenPlayerInfo(void )
{
	if(!reOpenPlayerInfoFlag)
		return;

	GameSpyOpenOverlay(GSOVERLAY_PLAYERINFO);
	reOpenPlayerInfoFlag = FALSE;

}
