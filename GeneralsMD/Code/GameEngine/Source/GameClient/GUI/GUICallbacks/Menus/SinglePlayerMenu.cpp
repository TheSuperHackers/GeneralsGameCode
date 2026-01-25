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

// FILE: SinglePlayerMenu.cpp /////////////////////////////////////////////////////////////////////
// Author: Colin Day, October 2001
// Description: Single Player Menus
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/GameEngine.h"
#include "GameClient/AnimateWindowManager.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/Shell.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/GameWindowManager.h"

static Bool isShuttingDown = false;
static Bool buttonPushed = false;
//-------------------------------------------------------------------------------------------------
/** This is called when a shutdown is complete for this menu */
//-------------------------------------------------------------------------------------------------
static void shutdownComplete( WindowLayout *layout )
{

	isShuttingDown = false;

	// hide the layout
	layout->hide( TRUE );

	// our shutdown is complete
	TheShell->shutdownComplete( layout );

}

//-------------------------------------------------------------------------------------------------
/** Initialize the single player menu */
//-------------------------------------------------------------------------------------------------
void SinglePlayerMenuInit( WindowLayout *layout, void *userData )
{
	TheShell->showShellMap(TRUE);
	buttonPushed = false;
	isShuttingDown = false;

	// show menu
	layout->hide( FALSE );

	// set keyboard focus to main parent
	NameKeyType parentID = TheNameKeyGenerator->nameToKey( "SinglePlayerMenu.wnd:SinglePlayerMenuParent" );
	GameWindow *parent = TheWindowManager->winGetWindowFromId( nullptr, parentID );
	TheWindowManager->winSetFocus( parent );

	NameKeyType buttonNewID = TheNameKeyGenerator->nameToKey( "SinglePlayerMenu.wnd:ButtonNew" );
	GameWindow *buttonNew = TheWindowManager->winGetWindowFromId( nullptr, buttonNewID );
	TheShell->registerWithAnimateManager(buttonNew, WIN_ANIMATION_SLIDE_LEFT, TRUE,1);

	NameKeyType buttonLoadID = TheNameKeyGenerator->nameToKey( "SinglePlayerMenu.wnd:ButtonLoad" );
	GameWindow *buttonLoad = TheWindowManager->winGetWindowFromId( nullptr, buttonLoadID );
	TheShell->registerWithAnimateManager(buttonLoad, WIN_ANIMATION_SLIDE_LEFT, TRUE,200);

	NameKeyType buttonBackID = TheNameKeyGenerator->nameToKey( "SinglePlayerMenu.wnd:ButtonBack" );
	GameWindow *buttonBack = TheWindowManager->winGetWindowFromId( nullptr, buttonBackID );
	TheShell->registerWithAnimateManager(buttonBack, WIN_ANIMATION_SLIDE_RIGHT, TRUE,1);


	//TheShell->registerWithAnimateManager(parent, WIN_ANIMATION_SLIDE_TOP, TRUE);

}

//-------------------------------------------------------------------------------------------------
/** single player menu shutdown method */
//-------------------------------------------------------------------------------------------------
void SinglePlayerMenuShutdown( WindowLayout *layout, void *userData )
{
	isShuttingDown = true;

	// if we are shutting down for an immediate pop, skip the animations
	Bool popImmediate = *(Bool *)userData;
	if( popImmediate )
	{

		shutdownComplete( layout );
		return;

	}

	TheShell->reverseAnimatewindow();

}

//-------------------------------------------------------------------------------------------------
/** single player menu update method */
//-------------------------------------------------------------------------------------------------
void SinglePlayerMenuUpdate( WindowLayout *layout, void *userData )
{

	// We'll only be successful if we've requested to
	if(isShuttingDown && TheShell->isAnimFinished())
		shutdownComplete(layout);

}

//-------------------------------------------------------------------------------------------------
/** SinglePlayer menu input callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType SinglePlayerMenuInput( GameWindow *window, UnsignedInt msg,
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
						NameKeyType buttonID = TheNameKeyGenerator->nameToKey( "SinglePlayerMenu.wnd:ButtonBack" );
						GameWindow *button = TheWindowManager->winGetWindowFromId( window, buttonID );

						TheWindowManager->winSendSystemMsg( window, GBM_SELECTED,
																								(WindowMsgData)button, buttonID );

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
/** single player menu window system callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType SinglePlayerMenuSystem( GameWindow *window, UnsignedInt msg,
														 WindowMsgData mData1, WindowMsgData mData2 )
{
	static NameKeyType buttonNew = NAMEKEY_INVALID;
	static NameKeyType buttonLoad = NAMEKEY_INVALID;
	static NameKeyType buttonBack = NAMEKEY_INVALID;

	switch( msg )
	{

		// --------------------------------------------------------------------------------------------
		case GWM_CREATE:
		{

			// get ids for our children controls
			buttonNew = TheNameKeyGenerator->nameToKey( "SinglePlayerMenu.wnd:ButtonNew" );
			buttonLoad = TheNameKeyGenerator->nameToKey( "SinglePlayerMenu.wnd:ButtonLoad" );
			buttonBack = TheNameKeyGenerator->nameToKey( "SinglePlayerMenu.wnd:ButtonBack" );

			break;

		}

		//---------------------------------------------------------------------------------------------
		case GWM_DESTROY:
		{

			break;

		}

		// --------------------------------------------------------------------------------------------
		case GWM_INPUT_FOCUS:
		{

			// if we're givin the opportunity to take the keyboard focus we must say we want it
			if( mData1 == TRUE )
				*(Bool *)mData2 = TRUE;

			return MSG_HANDLED;

		}

		//---------------------------------------------------------------------------------------------
		case GBM_SELECTED:
		{
			GameWindow *control = (GameWindow *)mData1;
			Int controlID = control->winGetWindowId();
			if (buttonPushed)
				break;

			if( controlID == buttonNew )
			{

				// open up the map select menu
				TheShell->push( "Menus/MapSelectMenu.wnd" );
				buttonPushed = true;

			}
			else if( controlID == buttonLoad )
			{

			}
			else if( controlID == buttonBack )
			{

				// thou art directed to return to thy known solar system immediately!
				TheShell->pop();
				buttonPushed = true;

			}

			break;

		}

		default:
			return MSG_IGNORED;
	}

	return MSG_HANDLED;
}

