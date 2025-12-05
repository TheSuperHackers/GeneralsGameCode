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

///////////////////////////////////////////////////////////////////////////////////////
// FILE: WOLCustomScoreScreen.cpp
// Author: Matt Campbell, December 2001
// Description: Custom match score screen
///////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Lib/BaseType.h"
#include "Common/GameEngine.h"
#include "Common/NameKeyGenerator.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/Shell.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GadgetTextEntry.h"
#include "Common/GlobalData.h"
//#include "GameNetwork/WOL.h"
//#include "GameNetwork/WOLmenus.h"



// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
// window ids ------------------------------------------------------------------------------
static NameKeyType parentWOLCustomScoreID = NAMEKEY_INVALID;
static NameKeyType buttonDisconnectID = NAMEKEY_INVALID;
static NameKeyType buttonLobbyID = NAMEKEY_INVALID;

// Window Pointers ------------------------------------------------------------------------
static GameWindow *parentWOLCustomScore = NULL;
static GameWindow *buttonDisconnect = NULL;
static GameWindow *buttonLobby = NULL;

//-------------------------------------------------------------------------------------------------
/** Initialize the WOL Status Menu */
//-------------------------------------------------------------------------------------------------
void WOLCustomScoreScreenInit( WindowLayout *layout, void *userData )
{
	parentWOLCustomScoreID = TheNameKeyGenerator->nameToKey( "WOLCustomScoreScreen.wnd:WOLCustomScoreScreenParent" );
	buttonDisconnectID = TheNameKeyGenerator->nameToKey( "WOLCustomScoreScreen.wnd:ButtonDisconnect" );
	buttonLobbyID = TheNameKeyGenerator->nameToKey( "WOLCustomScoreScreen.wnd:ButtonLobby" );
	parentWOLCustomScore = TheWindowManager->winGetWindowFromId( NULL, parentWOLCustomScoreID );
	buttonDisconnect = TheWindowManager->winGetWindowFromId( NULL,  buttonDisconnectID);
	buttonLobby = TheWindowManager->winGetWindowFromId( NULL,  buttonLobbyID);

	/*
	if (WOL::TheWOL->getState() == WOL::WOLAPI_FATAL_ERROR)
	{
		// We can get to the score screen even though we've been disconnected.  Just hide
		// any buttons that lead back into WOL.
		buttonLobby->winHide( TRUE );
	}
	*/

	// Show Menu
	layout->hide( FALSE );

	// Set Keyboard to Main Parent
	TheWindowManager->winSetFocus( parentWOLCustomScore );
}

//-------------------------------------------------------------------------------------------------
/** WOL Status Menu shutdown method */
//-------------------------------------------------------------------------------------------------
void WOLCustomScoreScreenShutdown( WindowLayout *layout, void *userData )
{
	// hide menu
	layout->hide( TRUE );

	// our shutdown is complete
	TheShell->shutdownComplete( layout );
}


//-------------------------------------------------------------------------------------------------
/** WOL Status Menu update method */
//-------------------------------------------------------------------------------------------------
void WOLCustomScoreScreenUpdate( WindowLayout * layout, void *userData)
{
	/*
	if (WOL::TheWOL)
		WOL::TheWOL->update();
	*/
}

//-------------------------------------------------------------------------------------------------
/** WOL Status Menu input callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType WOLCustomScoreScreenInput( GameWindow *window, UnsignedInt msg,
																			 WindowMsgData mData1, WindowMsgData mData2 )
{
	switch( msg )
	{

		// --------------------------------------------------------------------------------------------
		case GWM_CHAR:
		{
			UnsignedByte key = mData1;
			UnsignedByte state = mData2;

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
																							(WindowMsgData)buttonDisconnect, buttonDisconnectID );

					}

					// don't let key fall through anywhere else
					return MSG_HANDLED;

				}

			}

		}

	}

	return MSG_IGNORED;
}

//-------------------------------------------------------------------------------------------------
/** WOL Status Menu window system callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType WOLCustomScoreScreenSystem( GameWindow *window, UnsignedInt msg,
														 WindowMsgData mData1, WindowMsgData mData2 )
{
	UnicodeString txtInput;

	switch( msg )
	{

		case GWM_CREATE:
			{

				break;
			}

		case GWM_DESTROY:
			{
				break;
			}

		case GWM_INPUT_FOCUS:
			{
				// if we're given the opportunity to take the keyboard focus we must say we want it
				if( mData1 == TRUE )
					*(Bool *)mData2 = TRUE;

				return MSG_HANDLED;
			}

		case GBM_SELECTED:
			{
				/*
				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();

				if ( controlID == buttonDisconnectID )
				{
					if (WOL::TheWOL->setState( WOL::WOLAPI_FATAL_ERROR ))
					{
						WOL::TheWOL->addCommand( WOL::WOLCOMMAND_RESET );  // don't display an error, log out, or anything
					}

				}
				else if ( controlID == buttonLobbyID )
				{
					if (WOL::TheWOL->getState() != WOL::WOLAPI_FATAL_ERROR)
					{
						WOL::TheWOL->setScreen(WOL::WOLAPI_MENU_CUSTOMLOBBY);
						WOL::TheWOL->setGameMode(WOL::WOLTYPE_CUSTOM);
						WOL::TheWOL->setState( WOL::WOLAPI_LOBBY );
						WOL::TheWOL->addCommand( WOL::WOLCOMMAND_REFRESH_CHANNELS );
					}
					else
					{
					}
				}
				*/
				break;
			}

		case GEM_EDIT_DONE:
			{
				break;
			}
		default:
			return MSG_IGNORED;

	}

	return MSG_HANDLED;
}
