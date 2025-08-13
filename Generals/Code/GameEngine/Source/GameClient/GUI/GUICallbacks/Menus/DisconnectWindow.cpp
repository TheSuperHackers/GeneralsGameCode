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

// FILE: DisconnectControls.cpp ///////////////////////////////////////////////////////////////////////
// Author: Bryan Cleveland - March 2001
// Desc: GUI menu for network disconnects
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameClient/GameWindow.h"
#include "GameClient/GameText.h"
#include "GameClient/Gadget.h"
#include "GameClient/GadgetTextEntry.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GameClient.h"
#include "GameClient/DisconnectMenu.h"
#include "GameClient/GameWindowManager.h"
#include "Common/NameKeyGenerator.h"
#include "GameNetwork/GameInfo.h"

// Private Data -----------------------------
static WindowLayout *disconnectMenuLayout;

static NameKeyType textEntryID = NAMEKEY_INVALID;
static NameKeyType textDisplayID = NAMEKEY_INVALID;

static GameWindow *textEntryWindow = NULL;
static GameWindow *textDisplayWindow = NULL;

static NameKeyType buttonQuitID = NAMEKEY_INVALID;
static GameWindow *buttonQuitWindow = NULL;

static NameKeyType buttonVotePlayer1ID = NAMEKEY_INVALID;
static NameKeyType buttonVotePlayer2ID = NAMEKEY_INVALID;
static NameKeyType buttonVotePlayer3ID = NAMEKEY_INVALID;
static NameKeyType buttonVotePlayer4ID = NAMEKEY_INVALID;
static NameKeyType buttonVotePlayer5ID = NAMEKEY_INVALID;
static NameKeyType buttonVotePlayer6ID = NAMEKEY_INVALID;
static NameKeyType buttonVotePlayer7ID = NAMEKEY_INVALID;

static GameWindow *buttonVotePlayer1Window = NULL;
static GameWindow *buttonVotePlayer2Window = NULL;
static GameWindow *buttonVotePlayer3Window = NULL;
static GameWindow *buttonVotePlayer4Window = NULL;
static GameWindow *buttonVotePlayer5Window = NULL;
static GameWindow *buttonVotePlayer6Window = NULL;
static GameWindow *buttonVotePlayer7Window = NULL;

static void InitDisconnectWindow( void ) {
	textEntryID = TheNameKeyGenerator->nameToKey( AsciiString("DisconnectScreen.wnd:TextEntry"));
	textDisplayID = TheNameKeyGenerator->nameToKey( AsciiString("DisconnectScreen.wnd:ListboxTextDisplay"));

	textEntryWindow = TheWindowManager->winGetWindowFromId(NULL, textEntryID);
	textDisplayWindow = TheWindowManager->winGetWindowFromId(NULL, textDisplayID);

	if (textEntryWindow != NULL) {
		GadgetTextEntrySetText(textEntryWindow, UnicodeString::TheEmptyString);
		TheWindowManager->winSetFocus(textEntryWindow);
	}

	buttonQuitID = TheNameKeyGenerator->nameToKey( AsciiString("DisconnectScreen.wnd:ButtonQuitGame"));
	buttonQuitWindow = TheWindowManager->winGetWindowFromId(NULL, buttonQuitID);

	buttonVotePlayer1ID = TheNameKeyGenerator->nameToKey( AsciiString("DisconnectScreen.wnd:ButtonKickPlayer1"));
	buttonVotePlayer2ID = TheNameKeyGenerator->nameToKey( AsciiString("DisconnectScreen.wnd:ButtonKickPlayer2"));
	buttonVotePlayer3ID = TheNameKeyGenerator->nameToKey( AsciiString("DisconnectScreen.wnd:ButtonKickPlayer3"));
	buttonVotePlayer4ID = TheNameKeyGenerator->nameToKey( AsciiString("DisconnectScreen.wnd:ButtonKickPlayer4"));
	buttonVotePlayer5ID = TheNameKeyGenerator->nameToKey( AsciiString("DisconnectScreen.wnd:ButtonKickPlayer5"));
	buttonVotePlayer6ID = TheNameKeyGenerator->nameToKey( AsciiString("DisconnectScreen.wnd:ButtonKickPlayer6"));
	buttonVotePlayer7ID = TheNameKeyGenerator->nameToKey( AsciiString("DisconnectScreen.wnd:ButtonKickPlayer7"));

	buttonVotePlayer1Window = TheWindowManager->winGetWindowFromId(NULL, buttonVotePlayer1ID);
	buttonVotePlayer2Window = TheWindowManager->winGetWindowFromId(NULL, buttonVotePlayer2ID);
	buttonVotePlayer3Window = TheWindowManager->winGetWindowFromId(NULL, buttonVotePlayer3ID);
	buttonVotePlayer4Window = TheWindowManager->winGetWindowFromId(NULL, buttonVotePlayer4ID);
	buttonVotePlayer5Window = TheWindowManager->winGetWindowFromId(NULL, buttonVotePlayer5ID);
	buttonVotePlayer6Window = TheWindowManager->winGetWindowFromId(NULL, buttonVotePlayer6ID);
	buttonVotePlayer7Window = TheWindowManager->winGetWindowFromId(NULL, buttonVotePlayer7ID);
}

//------------------------------------------------------
/** Show the Disconnect Screen */
//------------------------------------------------------
void ShowDisconnectWindow( void )
{

	// load the quit menu from the layout file if needed
	if( disconnectMenuLayout == NULL )
	{

		// load layout from disk
		disconnectMenuLayout = TheWindowManager->winCreateLayout( "Menus/DisconnectScreen.wnd" );

		// init it
		InitDisconnectWindow();

		// show it
		disconnectMenuLayout->hide( FALSE );

	}  // end if
	else
	{

		disconnectMenuLayout->hide( FALSE );

	}  // end else

	// Disallow voting for 2-player games.  Cheating punk.
	if ( TheGameInfo && TheGameInfo->getNumPlayers() < 3 )
	{
		buttonVotePlayer1Window->winEnable(FALSE);
		buttonVotePlayer2Window->winEnable(FALSE);
		buttonVotePlayer3Window->winEnable(FALSE);
		buttonVotePlayer4Window->winEnable(FALSE);
		buttonVotePlayer5Window->winEnable(FALSE);
		buttonVotePlayer6Window->winEnable(FALSE);
		buttonVotePlayer7Window->winEnable(FALSE);
	}
	else
	{
		buttonVotePlayer1Window->winEnable(TRUE);
		buttonVotePlayer2Window->winEnable(TRUE);
		buttonVotePlayer3Window->winEnable(TRUE);
		buttonVotePlayer4Window->winEnable(TRUE);
		buttonVotePlayer5Window->winEnable(TRUE);
		buttonVotePlayer6Window->winEnable(TRUE);
		buttonVotePlayer7Window->winEnable(TRUE);
	}
	buttonQuitWindow->winEnable(TRUE);
	disconnectMenuLayout->bringForward();

	GadgetListBoxReset(textDisplayWindow);
	GadgetListBoxAddEntryText(textDisplayWindow, TheGameText->fetch("GUI:InternetDisconnectionMenuBody1"),
		GameMakeColor(255,255,255,255), -1);

}  // end ToggleQuitMenu

//------------------------------------------------------
/** Hide the Disconnect Screen */
//------------------------------------------------------
void HideDisconnectWindow( void )
{

	// load the quit menu from the layout file if needed
	if( disconnectMenuLayout == NULL )
	{

		// load layout from disk
		disconnectMenuLayout = TheWindowManager->winCreateLayout( "Menus/DisconnectScreen.wnd" );

		// init it
		InitDisconnectWindow();

		// show it
		disconnectMenuLayout->hide( TRUE );

	}  // end if
	else
	{

		disconnectMenuLayout->hide( TRUE );

	}  // end else

}  // end ToggleQuitMenu

//-------------------------------------------------------------------------------------------------
/** Input callback for the control bar parent */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType DisconnectControlInput( GameWindow *window, UnsignedInt msg,
																						WindowMsgData mData1, WindowMsgData mData2 )
{

	return MSG_IGNORED;

} // end DisconnectControlInput

//-------------------------------------------------------------------------------------------------
/** System callback for the control bar parent */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType DisconnectControlSystem( GameWindow *window, UnsignedInt msg,
																			 WindowMsgData mData1, WindowMsgData mData2 )
{
	switch( msg )
	{

		//---------------------------------------------------------------------------------------------
		case GBM_SELECTED:
		{

			GameWindow *control = (GameWindow *) mData1;
			Int controlID = control->winGetWindowId();

			if (controlID == buttonQuitID) {
				TheDisconnectMenu->quitGame();
				buttonQuitWindow->winEnable(FALSE);
			} else if (controlID == buttonVotePlayer1ID) {
				TheDisconnectMenu->voteForPlayer(0);
				buttonVotePlayer1Window->winEnable(FALSE);
			} else if (controlID == buttonVotePlayer2ID) {
				TheDisconnectMenu->voteForPlayer(1);
				buttonVotePlayer2Window->winEnable(FALSE);
			} else if (controlID == buttonVotePlayer3ID) {
				TheDisconnectMenu->voteForPlayer(2);
				buttonVotePlayer3Window->winEnable(FALSE);
			} else if (controlID == buttonVotePlayer4ID) {
				TheDisconnectMenu->voteForPlayer(3);
				buttonVotePlayer4Window->winEnable(FALSE);
			} else if (controlID == buttonVotePlayer5ID) {
				TheDisconnectMenu->voteForPlayer(4);
				buttonVotePlayer5Window->winEnable(FALSE);
			} else if (controlID == buttonVotePlayer6ID) {
				TheDisconnectMenu->voteForPlayer(5);
				buttonVotePlayer6Window->winEnable(FALSE);
			} else if (controlID == buttonVotePlayer7ID) {
				TheDisconnectMenu->voteForPlayer(6);
				buttonVotePlayer7Window->winEnable(FALSE);
			}

			break;

		}  // end button selected

		case GEM_EDIT_DONE:
		{
//			DEBUG_LOG(("DisconnectControlSystem - got GEM_EDIT_DONE."));
			GameWindow *control = (GameWindow *)mData1;
			Int controlID = control->winGetWindowId();

			// Take the user's input and echo it into the chat window as well as
			// send it to the other clients on the lan
			if ( controlID == textEntryID )
			{
				UnicodeString txtInput;

//				DEBUG_LOG(("DisconnectControlSystem - GEM_EDIT_DONE was from the text entry control."));

				// read the user's input
				txtInput.set(GadgetTextEntryGetText( textEntryWindow ));
				// Clear the text entry line
				GadgetTextEntrySetText(textEntryWindow, UnicodeString::TheEmptyString);
				// Clean up the text (remove leading/trailing chars, etc)
				txtInput.trim();
				// Echo the user's input to the chat window
				if (!txtInput.isEmpty()) {
//					DEBUG_LOG(("DisconnectControlSystem - sending string %ls", txtInput.str()));
					TheDisconnectMenu->sendChat(txtInput);
				}

			}// if ( controlID == textEntryChatID )
			break;
		}

		//---------------------------------------------------------------------------------------------
		default:
			return MSG_IGNORED;

	}  // end switch( msg )

	return MSG_HANDLED;

}  // end ControlBarSystem

