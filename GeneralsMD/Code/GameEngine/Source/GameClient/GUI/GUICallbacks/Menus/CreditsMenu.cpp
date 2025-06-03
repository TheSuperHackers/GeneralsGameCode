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

// FILE: CreditsMenu.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Electronic Arts Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2002 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
//	created:	Dec 2002
//
//	Filename: 	CreditsMenu.cpp
//
//	author:		Chris Huybregts
//	
//	purpose:	The credits screen...yay
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine
#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma message("************************************** WARNING, optimization disabled for debugging purposes")
#endif


//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "Common/GameAudio.h"
#include "Common/AudioEventRTS.h"
#include "Common/AudioHandleSpecialValues.h"

#include "GameClient/Credits.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/Shell.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/GameWindowManager.h"

//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
static NameKeyType parentMainMenuID = NAMEKEY_INVALID;

// window pointers --------------------------------------------------------------------------------
static GameWindow *parentMainMenu = NULL;

//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/** Initialize the single player menu */
//-------------------------------------------------------------------------------------------------
void CreditsMenuInit( WindowLayout *layout, void *userData )
{
	TheShell->showShellMap(FALSE);
	if(TheCredits)
		delete TheCredits;
	TheCredits = new CreditsManager;
	TheCredits->load();
	TheCredits->init();
	
	parentMainMenuID = TheNameKeyGenerator->nameToKey( AsciiString("CreditsMenu.wnd:ParentCreditsWindow") );
	parentMainMenu = TheWindowManager->winGetWindowFromId( NULL, parentMainMenuID );


	// show menu
	layout->hide( FALSE );

	// set keyboard focus to main parent
	TheWindowManager->winSetFocus( parentMainMenu );



	TheAudio->removeAudioEvent( AHSV_StopTheMusicFade );
	AudioEventRTS event( AsciiString( "Credits" ) );
	event.setShouldFade( TRUE );
	TheAudio->addAudioEvent( &event );


}  // end CreditsMenuInit

//-------------------------------------------------------------------------------------------------
/** single player menu shutdown method */
//-------------------------------------------------------------------------------------------------
void CreditsMenuShutdown( WindowLayout *layout, void *userData )
{
	TheCredits->reset();
	delete TheCredits;
	TheCredits = NULL;
	TheShell->showShellMap(TRUE);

	// hide menu
	layout->hide( TRUE );

	// our shutdown is complete
	TheShell->shutdownComplete( layout );

	TheAudio->removeAudioEvent( AHSV_StopTheMusicFade );

}  // end CreditsMenuShutdown

//-------------------------------------------------------------------------------------------------
/** single player menu update method */
//-------------------------------------------------------------------------------------------------
void CreditsMenuUpdate( WindowLayout *layout, void *userData )
{

	if(TheCredits)
	{
		TheWindowManager->winSetFocus( parentMainMenu );
		TheCredits->update();
		if(TheCredits->isFinished())
			TheShell->pop();
	}
	else
		TheShell->pop();

}  // end CreditsMenuUpdate

//-------------------------------------------------------------------------------------------------
/** Replay menu input callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType CreditsMenuInput( GameWindow *window, UnsignedInt msg,
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

						TheShell->pop();

					}  // end if

					// don't let key fall through anywhere else
					return MSG_HANDLED;

				}  // end escape

			}  // end switch( key )

		}  // end char

	}  // end switch( msg )

	return MSG_IGNORED;

}  // end CreditsMenuInput

//-------------------------------------------------------------------------------------------------
/** single player menu window system callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType CreditsMenuSystem( GameWindow *window, UnsignedInt msg, 
														 WindowMsgData mData1, WindowMsgData mData2 )
{
	
	switch( msg ) 
	{

		// --------------------------------------------------------------------------------------------
		case GWM_CREATE:
		{

			
			break;

		}  // end create

		//---------------------------------------------------------------------------------------------
		case GWM_DESTROY:
		{

			break;

		}  // end case

		// --------------------------------------------------------------------------------------------
		case GWM_INPUT_FOCUS:
		{

			// if we're givin the opportunity to take the keyboard focus we must say we want it
			if( mData1 == TRUE )
				*(Bool *)mData2 = TRUE;

			return MSG_HANDLED;

		}  // end input
		//---------------------------------------------------------------------------------------------
		case GBM_SELECTED:
		{
			
			break;
		}  // end selected

		default:
			return MSG_IGNORED;
	}  // end switch

	return MSG_HANDLED;
}  // end CreditsMenuSystem

//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

