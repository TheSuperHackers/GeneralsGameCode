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

// FILE: MessageBox.cpp /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:   RTS3
//
// File name: MessageBox.cpp
//
// Created:   Chris Huybregts, June 2001
//
// Desc:      the Message Box control
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/GameEngine.h"
#include "Common/NameKeyGenerator.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/Shell.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/MessageBox.h"


GameWindow *MessageBoxYesNo(UnicodeString titleString,UnicodeString bodyString,GameWinMsgBoxFunc yesCallback,GameWinMsgBoxFunc noCallback)  ///< convenience function for displaying a Message box with Yes and No buttons
{
	return TheWindowManager->gogoMessageBox(-1,-1,-1,-1,MSG_BOX_NO | MSG_BOX_YES , titleString, bodyString, yesCallback, noCallback, NULL, NULL);
}
GameWindow *QuitMessageBoxYesNo(UnicodeString titleString,UnicodeString bodyString,GameWinMsgBoxFunc yesCallback,GameWinMsgBoxFunc noCallback)  ///< convenience function for displaying a Message box with Yes and No buttons
{
	return TheWindowManager->gogoMessageBox(-1,-1,-1,-1,MSG_BOX_NO | MSG_BOX_YES , titleString, bodyString, yesCallback, noCallback, NULL, NULL, TRUE);
}


GameWindow *MessageBoxYesNoCancel(UnicodeString titleString,UnicodeString bodyString, GameWinMsgBoxFunc yesCallback, GameWinMsgBoxFunc noCallback, GameWinMsgBoxFunc cancelCallback)///< convenience function for displaying a Message box with Yes,No and Cancel buttons
{
	return TheWindowManager->gogoMessageBox(-1,-1,-1,-1,MSG_BOX_NO | MSG_BOX_YES | MSG_BOX_CANCEL , titleString, bodyString, yesCallback, noCallback, NULL, cancelCallback);
}


GameWindow *MessageBoxOkCancel(UnicodeString titleString,UnicodeString bodyString,GameWinMsgBoxFunc okCallback,GameWinMsgBoxFunc cancelCallback)///< convenience function for displaying a Message box with Ok and Cancel buttons
{
	return TheWindowManager->gogoMessageBox(-1,-1,-1,-1,MSG_BOX_OK | MSG_BOX_CANCEL , titleString, bodyString, NULL, NULL, okCallback, cancelCallback);
}

GameWindow *MessageBoxOk(UnicodeString titleString,UnicodeString bodyString,GameWinMsgBoxFunc okCallback)///< convenience function for displaying a Message box with Ok button
{
	return TheWindowManager->gogoMessageBox(-1,-1,-1,-1,MSG_BOX_OK, titleString, bodyString, NULL, NULL, okCallback, NULL);
}


GameWindow *MessageBoxCancel(UnicodeString titleString,UnicodeString bodyString,GameWinMsgBoxFunc cancelCallback)///< convenience function for displaying a Message box with Cancel button
{
	return TheWindowManager->gogoMessageBox(-1,-1,-1,-1, MSG_BOX_CANCEL, titleString, bodyString, NULL, NULL, NULL, cancelCallback);
}


// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////



//-------------------------------------------------------------------------------------------------
/** Message Box window system callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType MessageBoxSystem( GameWindow *window, UnsignedInt msg,
										 WindowMsgData mData1, WindowMsgData mData2 )
{


	switch( msg )
	{

		//---------------------------------------------------------------------------------------------
		case GWM_DESTROY:
		{
			delete (WindowMessageBoxData *)window->winGetUserData();
			window->winSetUserData( NULL );
			break;

		}  // end case

		// --------------------------------------------------------------------------------------------
		case GWM_INPUT_FOCUS:
		{

			// if we're givin the opportunity to take the keyboard focus we must say we want it
			if( mData1 == TRUE )
				*(Bool *)mData2 = TRUE;

			break;

		}  // end input

		//---------------------------------------------------------------------------------------------
		case GBM_SELECTED:
		{
			GameWindow *control = (GameWindow *)mData1;
			Int controlID = control->winGetWindowId();
			static NameKeyType buttonOkID = TheNameKeyGenerator->nameToKey( AsciiString( "MessageBox.wnd:ButtonOk" ) );
			static NameKeyType buttonYesID = TheNameKeyGenerator->nameToKey( AsciiString( "MessageBox.wnd:ButtonYes" ) );
			static NameKeyType buttonNoID = TheNameKeyGenerator->nameToKey( AsciiString( "MessageBox.wnd:ButtonNo" ) );
			static NameKeyType buttonCancelID = TheNameKeyGenerator->nameToKey( AsciiString( "MessageBox.wnd:ButtonCancel" ) );
			WindowMessageBoxData *MsgBoxCallbacks = (WindowMessageBoxData *)window->winGetUserData();

			if( controlID == buttonOkID )
			{
				//simple enough,if we have a callback, call it, if not, then just destroy the window
				if (MsgBoxCallbacks->okCallback)
					MsgBoxCallbacks->okCallback();

				TheWindowManager->winDestroy(window);

			}  // end if
			else if( controlID == buttonYesID )
			{
				if (MsgBoxCallbacks->yesCallback)
					MsgBoxCallbacks->yesCallback();
				TheWindowManager->winDestroy(window);
			}  // end else if
			else if( controlID == buttonNoID )
			{
				if (MsgBoxCallbacks->noCallback)
					MsgBoxCallbacks->noCallback();
				TheWindowManager->winDestroy(window);
			}  // end else if
			else if( controlID == buttonCancelID )
			{
				if (MsgBoxCallbacks->cancelCallback)
					MsgBoxCallbacks->cancelCallback();
				TheWindowManager->winDestroy(window);
			}  // end else if

			break;

		}  // end selected

		//---------------------------------------------------------------------------------------------
		default:
			return MSG_IGNORED;

	}  // end switch

	return MSG_HANDLED;

}  // end MessageBoxSystem
//-------------------------------------------------------------------------------------------------
/** Message Box window system callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType QuitMessageBoxSystem( GameWindow *window, UnsignedInt msg,
										 WindowMsgData mData1, WindowMsgData mData2 )
{


	switch( msg )
	{

		//---------------------------------------------------------------------------------------------
		case GWM_DESTROY:
		{
			delete (WindowMessageBoxData *)window->winGetUserData();
			window->winSetUserData( NULL );
			break;

		}  // end case

		// --------------------------------------------------------------------------------------------
		case GWM_INPUT_FOCUS:
		{

			// if we're givin the opportunity to take the keyboard focus we must say we want it
			if( mData1 == TRUE )
				*(Bool *)mData2 = TRUE;

			break;

		}  // end input

		//---------------------------------------------------------------------------------------------
		case GBM_SELECTED:
		{
			GameWindow *control = (GameWindow *)mData1;
			Int controlID = control->winGetWindowId();
			static NameKeyType buttonOkID = TheNameKeyGenerator->nameToKey( AsciiString( "QuitMessageBox.wnd:ButtonOk" ) );
			static NameKeyType buttonYesID = TheNameKeyGenerator->nameToKey( AsciiString( "QuitMessageBox.wnd:ButtonYes" ) );
			static NameKeyType buttonNoID = TheNameKeyGenerator->nameToKey( AsciiString( "QuitMessageBox.wnd:ButtonNo" ) );
			static NameKeyType buttonCancelID = TheNameKeyGenerator->nameToKey( AsciiString( "QuitMessageBox.wnd:ButtonCancel" ) );
			WindowMessageBoxData *MsgBoxCallbacks = (WindowMessageBoxData *)window->winGetUserData();

			if( controlID == buttonOkID )
			{
				//simple enough,if we have a callback, call it, if not, then just destroy the window
				if (MsgBoxCallbacks->okCallback)
					MsgBoxCallbacks->okCallback();

				TheWindowManager->winDestroy(window);

			}  // end if
			else if( controlID == buttonYesID )
			{
				if (MsgBoxCallbacks->yesCallback)
					MsgBoxCallbacks->yesCallback();
				TheWindowManager->winDestroy(window);
			}  // end else if
			else if( controlID == buttonNoID )
			{
				if (MsgBoxCallbacks->noCallback)
					MsgBoxCallbacks->noCallback();
				TheWindowManager->winDestroy(window);
			}  // end else if
			else if( controlID == buttonCancelID )
			{
				if (MsgBoxCallbacks->cancelCallback)
					MsgBoxCallbacks->cancelCallback();
				TheWindowManager->winDestroy(window);
			}  // end else if

			break;

		}  // end selected

		//---------------------------------------------------------------------------------------------
		default:
			return MSG_IGNORED;

	}  // end switch

	return MSG_HANDLED;

}  // end MessageBoxSystem
