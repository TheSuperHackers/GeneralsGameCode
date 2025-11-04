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

// FILE: GeneralsExpPoints.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//	created:	Oct 2002
//
//	Filename: 	GeneralsExpPoints.cpp
//
//	author:		Chris Huybregts
//
//	purpose:	File used to populate/update/show/hide the generals exp screen
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "GameClient/ControlBar.h"
#include "GameClient/GUICallbacks.h"
#include "GameClient/GameWindow.h"
#include "GameClient/Gadget.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/ControlBar.h"
#include "GameClient/InGameUI.h"
//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
WindowMsgHandledType GeneralsExpPointsInput( GameWindow *window, UnsignedInt msg,
																			WindowMsgData mData1, WindowMsgData mData2 )
{

	switch( msg )
	{

		// --------------------------------------------------------------------------------------------
		case GWM_MOUSE_ENTERING:
			//Get rid of any building placement mode!
			if( TheInGameUI )
			{
				TheInGameUI->placeBuildAvailable( NULL, NULL );
			}
			break;

		case GWM_CHAR:
		{
			UnsignedByte key = mData1;
//			UnsignedByte state = mData2;

			switch( key )
			{

				// ----------------------------------------------------------------------------------------
				case KEY_ESC:
				{
					TheControlBar->hidePurchaseScience();
					return MSG_HANDLED;
					//return MSG_IGNORED;
				}

			}

			return MSG_HANDLED;

		}

	}

	return MSG_HANDLED;

}



//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
WindowMsgHandledType GeneralsExpPointsSystem( GameWindow *window, UnsignedInt msg,
																			 WindowMsgData mData1, WindowMsgData mData2 )
{

	switch( msg )
	{
		//---------------------------------------------------------------------------------------------
		case GGM_FOCUS_CHANGE:
		{
			//			Bool focus = (Bool) mData1;
			//if (focus)
				//TheWindowManager->winSetGrabWindow( chatTextEntry );
			break;
		}

		//---------------------------------------------------------------------------------------------
		case GWM_INPUT_FOCUS:
		{
			// if we're given the opportunity to take the keyboard focus we must say we don't want it
			if( mData1 == TRUE )
				*(Bool *)mData2 = FALSE;

			return MSG_HANDLED;
		}

		//---------------------------------------------------------------------------------------------
		case GBM_SELECTED:
		{
			GameWindow *control = (GameWindow *)mData1;
			NameKeyType controlID = (NameKeyType)control->winGetWindowId();
			static NameKeyType buttonExitID = NAMEKEY( "GeneralsExpPoints.wnd:ButtonExit" );
			if (controlID == buttonExitID)
			{
				TheControlBar->hidePurchaseScience();
			}
			else
				TheControlBar->processContextSensitiveButtonClick( control, (GadgetGameMessage)msg );
			break;

		}

		//---------------------------------------------------------------------------------------------
		default:
			return MSG_IGNORED;

	}

	return MSG_HANDLED;

}

//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

