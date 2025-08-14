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
// FILE: WOLQMScoreScreen.cpp
// Author: Matt Campbell, November 2001
// Description: QuickMatch score screen (different from normal screen in that it has 'QM' and 'Discon' buttons)
///////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/GameEngine.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/Shell.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GadgetTextEntry.h"
//#include "GameNetwork/WOL.h"
//#include "GameNetwork/WOLmenus.h"



// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
// window ids ------------------------------------------------------------------------------
static NameKeyType parentWOLQMScoreID = NAMEKEY_INVALID;
static NameKeyType buttonDisconnectID = NAMEKEY_INVALID;
static NameKeyType buttonQuickmatchID = NAMEKEY_INVALID;

// Window Pointers ------------------------------------------------------------------------
static GameWindow *parentWOLQMScore = NULL;
static GameWindow *buttonDisconnect = NULL;
static GameWindow *buttonQuickmatch = NULL;

//-------------------------------------------------------------------------------------------------
/** Initialize the WOL Status Menu */
//-------------------------------------------------------------------------------------------------
void WOLQMScoreScreenInit( WindowLayout *layout, void *userData )
{
	parentWOLQMScoreID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLQMScoreScreen.wnd:WOLQMScoreScreenParent" ) );
	buttonDisconnectID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLQMScoreScreen.wnd:ButtonDisconnect" ) );
	buttonQuickmatchID = TheNameKeyGenerator->nameToKey( AsciiString( "WOLQMScoreScreen.wnd:ButtonQuickMatch" ) );
	parentWOLQMScore = TheWindowManager->winGetWindowFromId( NULL, parentWOLQMScoreID );
	buttonDisconnect = TheWindowManager->winGetWindowFromId( NULL,  buttonDisconnectID);
	buttonQuickmatch = TheWindowManager->winGetWindowFromId( NULL,  buttonQuickmatchID);

	/*
	if (WOL::TheWOL->getState() == WOL::WOLAPI_FATAL_ERROR)
	{
		// We can get to the score screen even though we've been disconnected.  Just hide
		// any buttons that lead back into WOL.

		buttonQuickmatch->winHide( TRUE );
	}
	*/

	// Show Menu
	layout->hide( FALSE );

	// Set Keyboard to Main Parent
	TheWindowManager->winSetFocus( parentWOLQMScore );

	//progressLayout = TheShell->top();

} // WOLQMScoreScreenInit

//-------------------------------------------------------------------------------------------------
/** WOL Status Menu shutdown method */
//-------------------------------------------------------------------------------------------------
void WOLQMScoreScreenShutdown( WindowLayout *layout, void *userData )
{

	// hide menu
	layout->hide( TRUE );

	// our shutdown is complete
	TheShell->shutdownComplete( layout );

	//progressLayout = NULL;

}  // WOLQMScoreScreenShutdown


//-------------------------------------------------------------------------------------------------
/** WOL Status Menu update method */
//-------------------------------------------------------------------------------------------------
void WOLQMScoreScreenUpdate( WindowLayout * layout, void *userData)
{
	/*
	if (WOL::TheWOL)
		WOL::TheWOL->update();
	*/
}// WOLQMScoreScreenUpdate

//-------------------------------------------------------------------------------------------------
/** WOL Status Menu input callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType WOLQMScoreScreenInput( GameWindow *window, UnsignedInt msg,
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

					}  // end if

					// don't let key fall through anywhere else
					return MSG_HANDLED;

				}  // end escape

			}  // end switch( key )

		}  // end char

	}  // end switch( msg )

	return MSG_IGNORED;
}// WOLQMScoreScreenInput

//-------------------------------------------------------------------------------------------------
/** WOL Status Menu window system callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType WOLQMScoreScreenSystem( GameWindow *window, UnsignedInt msg,
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
				// if we're given the opportunity to take the keyboard focus we must say we want it
				if( mData1 == TRUE )
					*(Bool *)mData2 = TRUE;

				return MSG_HANDLED;
			}//case GWM_INPUT_FOCUS:

		case GBM_SELECTED:
			{
				/*
				GameWindow *control = (GameWindow *)mData1;
				Int controlID = control->winGetWindowId();

				if ( controlID == buttonDisconnectID )
				{
					//TheShell->pop();
					if (WOL::TheWOL->setState( WOL::WOLAPI_FATAL_ERROR ))
					{
						WOL::TheWOL->addCommand( WOL::WOLCOMMAND_RESET );  // don't display an error, log out, or anything
					}

				} //if ( controlID == buttonDisconnect )
				else if ( controlID == buttonQuickmatchID )
				{
					//TheShell->pop();
					if (WOL::TheWOL->getState() != WOL::WOLAPI_FATAL_ERROR)
					{
						if (WOL::TheWOL->setState( WOL::WOLAPI_TOURNAMENT ))
						{
							WOL::TheWOL->setScreen( WOL::WOLAPI_MENU_QUICKMATCH );
							WOL::TheWOL->addCommand( WOL::WOLCOMMAND_FIND_MATCH_CHANNEL );
						}
					}

				} //if ( controlID == buttonDisconnect )
				*/
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
}// WOLQMScoreScreenSystem
