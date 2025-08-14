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
// FILE: WOLLocaleSelectPopup.cpp
// Author: Matt Campbell, December 2001
// Description: WOL locale select popup
///////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameClient/GameText.h"
#include "Common/CustomMatchPreferences.h"
#include "Common/GameEngine.h"
#include "Common/GameSpyMiscPreferences.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/Shell.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetListBox.h"
#include "Common/GlobalData.h"
#include "GameNetwork/GameSpyOverlay.h"
#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/GameSpy/PeerThread.h"
#include "GameNetwork/GameSpy/PersistentStorageDefs.h"
#include "GameNetwork/GameSpy/PersistentStorageThread.h"


// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
// window ids ------------------------------------------------------------------------------
static NameKeyType parentLocaleSelectID = NAMEKEY_INVALID;
static NameKeyType buttonOkID = NAMEKEY_INVALID;
static NameKeyType listboxLocaleID = NAMEKEY_INVALID;

// Window Pointers ------------------------------------------------------------------------
static GameWindow *parentLocaleSelect = NULL;
static GameWindow *buttonOk = NULL;
static GameWindow *listboxLocale = NULL;

//-------------------------------------------------------------------------------------------------
/** Initialize the WOL Status Menu */
//-------------------------------------------------------------------------------------------------
void WOLLocaleSelectInit( WindowLayout *layout, void *userData )
{
	parentLocaleSelectID = TheNameKeyGenerator->nameToKey( AsciiString( "PopupLocaleSelect.wnd:ParentLocaleSelect" ) );
	buttonOkID = TheNameKeyGenerator->nameToKey( AsciiString( "PopupLocaleSelect.wnd:ButtonOk" ) );
	listboxLocaleID = TheNameKeyGenerator->nameToKey( AsciiString( "PopupLocaleSelect.wnd:ListBoxLocaleSelect" ) );
	parentLocaleSelect = TheWindowManager->winGetWindowFromId( NULL, parentLocaleSelectID );
	buttonOk = TheWindowManager->winGetWindowFromId( NULL,  buttonOkID);
	listboxLocale = TheWindowManager->winGetWindowFromId( NULL,  listboxLocaleID);

	for (int i=LOC_MIN; i<=LOC_MAX; ++i)
	{
		AsciiString id;
		id.format("WOL:Locale%2.2d", i);
		GadgetListBoxAddEntryText(listboxLocale, TheGameText->fetch(id.str()), GameSpyColor[GSCOLOR_DEFAULT], -1, -1);
	}
	GadgetListBoxSetSelected(listboxLocale, 0);

	// Show Menu
	layout->hide( FALSE );

	// Set Keyboard to Main Parent
	TheWindowManager->winSetFocus( parentLocaleSelect );
	TheWindowManager->winSetModal( parentLocaleSelect );
} // WOLLocaleSelectInit

//-------------------------------------------------------------------------------------------------
/** WOL Status Menu shutdown method */
//-------------------------------------------------------------------------------------------------
void WOLLocaleSelectShutdown( WindowLayout *layout, void *userData )
{

	// hide menu
	layout->hide( TRUE );

	// our shutdown is complete
	TheShell->shutdownComplete( layout );

}  // WOLLocaleSelectShutdown


//-------------------------------------------------------------------------------------------------
/** WOL Status Menu update method */
//-------------------------------------------------------------------------------------------------
void WOLLocaleSelectUpdate( WindowLayout * layout, void *userData)
{

}// WOLLocaleSelectUpdate

//-------------------------------------------------------------------------------------------------
/** WOL Status Menu input callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType WOLLocaleSelectInput( GameWindow *window, UnsignedInt msg,
																			 WindowMsgData mData1, WindowMsgData mData2 )
{
	switch( msg )
	{

		// --------------------------------------------------------------------------------------------
		case GWM_CHAR:
		{
//			UnsignedByte key = mData1;
//			UnsignedByte state = mData2;

			// ----------------------------------------------------------------------------------------
			// don't let key fall through anywhere else
			return MSG_HANDLED;
		}  // end char
	}  // end switch( msg )
	return MSG_IGNORED;
}// WOLLocaleSelectInput

//Int getRegistryNicknameOffset(AsciiString nick); /// @todo: mdc remove this once we can save ini pref files
//-------------------------------------------------------------------------------------------------
/** WOL Status Menu window system callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType WOLLocaleSelectSystem( GameWindow *window, UnsignedInt msg,
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
				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();

				if ( controlID == buttonOkID )
				{
					int selected;
					GadgetListBoxGetSelected(listboxLocale, &selected);
					if (selected < 0)
						return MSG_HANDLED; // can't select nothing!
					PSRequest psReq;
					psReq.requestType = PSRequest::PSREQUEST_UPDATEPLAYERLOCALE;
					psReq.player.locale = selected + LOC_MIN;
					psReq.email = TheGameSpyInfo->getLocalEmail().str();
					psReq.nick = TheGameSpyInfo->getLocalBaseName().str();
					psReq.password = TheGameSpyInfo->getLocalPassword().str();
					psReq.player.id = TheGameSpyInfo->getLocalProfileID();

					TheGameSpyPSMessageQueue->addRequest(psReq);
					GameSpyCloseOverlay(GSOVERLAY_LOCALESELECT);

					GameSpyMiscPreferences cPref;
					cPref.setLocale(psReq.player.locale);
					cPref.write();

					PSPlayerStats stats = TheGameSpyPSMessageQueue->findPlayerStatsByID(TheGameSpyInfo->getLocalProfileID());
					stats.locale = psReq.player.locale;
					if (stats.id == TheGameSpyInfo->getLocalProfileID())
						TheGameSpyPSMessageQueue->trackPlayerStats(stats);

					if(stats.id == 0)
					{
						stats = TheGameSpyInfo->getCachedLocalPlayerStats();
						stats.locale = psReq.player.locale;
						TheGameSpyInfo->setCachedLocalPlayerStats(stats);
					}
					else
					{
						// force an update of our shtuff
						PSResponse newResp;
						newResp.responseType = PSResponse::PSRESPONSE_PLAYERSTATS;
						newResp.player = TheGameSpyPSMessageQueue->findPlayerStatsByID(TheGameSpyInfo->getLocalProfileID());
						TheGameSpyPSMessageQueue->addResponse(newResp);
					}
					CheckReOpenPlayerInfo();
				} //if ( controlID == buttonDisconnect )
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
}// WOLLocaleSelectSystem
