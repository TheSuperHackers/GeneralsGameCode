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
// FILE: WOLMessageWindow.cpp
// Author: Chris Huybregts, November 2001
// Description: Lan Lobby Menu
///////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/GameEngine.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/Shell.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GadgetTextEntry.h"
#include "GameNetwork/IPEnumeration.h"
//#include "GameNetwork/WOL.h"



// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
// window ids ------------------------------------------------------------------------------
static NameKeyType parentWOLMessageWindowID = NAMEKEY_INVALID;
static NameKeyType buttonCancelID = NAMEKEY_INVALID;

// Window Pointers ------------------------------------------------------------------------
static GameWindow *parentWOLMessageWindow = NULL;
static GameWindow *buttonCancel = NULL;


//-------------------------------------------------------------------------------------------------
/** Initialize the WOLMessage Window */
//-------------------------------------------------------------------------------------------------
void WOLMessageWindowInit( WindowLayout *layout, void *userData )
{
	parentWOLMessageWindowID = TheNameKeyGenerator->nameToKey( "WOLMessageWindow.wnd:WOLMessageWindowParent" );
	buttonCancelID = TheNameKeyGenerator->nameToKey( "WOLMessageWindow.wnd:ButtonCancel" );
	parentWOLMessageWindow = TheWindowManager->winGetWindowFromId( NULL, parentWOLMessageWindowID );
	buttonCancel = TheWindowManager->winGetWindowFromId( NULL,  buttonCancelID);


	// Show Menu
	layout->hide( FALSE );

	// Set Keyboard to Main Parent
	TheWindowManager->winSetFocus( parentWOLMessageWindow );

}

//-------------------------------------------------------------------------------------------------
/** WOLMessage Window shutdown method */
//-------------------------------------------------------------------------------------------------
void WOLMessageWindowShutdown( WindowLayout *layout, void *userData )
{

	// hide menu
	layout->hide( TRUE );

	// our shutdown is complete
	TheShell->shutdownComplete( layout );
}


//-------------------------------------------------------------------------------------------------
/** WOLMessage Window update method */
//-------------------------------------------------------------------------------------------------
void WOLMessageWindowUpdate( WindowLayout * layout, void *userData)
{
	/*
	if (WOL::TheWOL)
		WOL::TheWOL->update();
	*/

}

//-------------------------------------------------------------------------------------------------
/** WOLMessage Window input callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType WOLMessageWindowInput( GameWindow *window, UnsignedInt msg,
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
																							(WindowMsgData)buttonCancel, buttonCancelID );

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
/** WOLMessage Window window system callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType WOLMessageWindowSystem( GameWindow *window, UnsignedInt msg,
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
				// if we're givin the opportunity to take the keyboard focus we must say we want it
				if( mData1 == TRUE )
					*(Bool *)mData2 = TRUE;

				return MSG_HANDLED;
			}

		case GBM_SELECTED:
			{
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
