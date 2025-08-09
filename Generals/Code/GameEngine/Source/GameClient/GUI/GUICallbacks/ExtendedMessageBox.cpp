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

// FILE: ExtendedMessageBox.cpp ///////////////////////////////////////////////////////////////////
// Author: Matt Campbell, January 2003
// Description: We go quiet in 1 day, gold in 15.  Poor time to rewrite message boxes, so
//              we get this file instead.  Phooey.
///////////////////////////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/GameEngine.h"
#include "Common/NameKeyGenerator.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/Shell.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/ExtendedMessageBox.h"

WindowMsgHandledType ExtendedMessageBoxSystem( GameWindow *window, UnsignedInt msg,
										 WindowMsgData mData1, WindowMsgData mData2 );

//-------------------------------------------------------------------------------------------------
/** Create an extended Modal Message Box */
//-------------------------------------------------------------------------------------------------
static GameWindow *gogoExMessageBox(Int x, Int y, Int width, Int height, UnsignedShort buttonFlags,
                        UnicodeString titleString, UnicodeString bodyString, void *userData,
                        MessageBoxFunc yesCallback,
                        MessageBoxFunc noCallback,
                        MessageBoxFunc okCallback,
                        MessageBoxFunc cancelCallback )
{
	// first check to make sure we have some buttons to display
	if(buttonFlags == 0 )
	{
		return NULL;
	}

	GameWindow *parent = TheWindowManager->winCreateFromScript( AsciiString("Menus/MessageBox.wnd") );
	TheWindowManager->winSetModal( parent );
	TheWindowManager->winSetFocus( NULL ); // make sure we lose focus from other windows even if we refuse focus ourselves
	TheWindowManager->winSetFocus( parent	 );

	// If the user wants the size to be different then the default
	float ratioX, ratioY = 1;

	if( width > 0 && height > 0 )
	{
		ICoord2D temp;
		//First grab the percent increase/decrease compaired to the default size
		parent->winGetSize( &temp.x, &temp.y);
		ratioX = (float)width / (float)temp.x;
		ratioY = (float)height / (float)temp.y;
		//Set the window's new size
		parent->winSetSize( width, height);

		//Resize/reposition all the children windows based off the ratio
		GameWindow *child;
		for( child = parent->winGetChild(); child; child = child->winGetNext() )
		{
			child->winGetSize(&temp.x, &temp.y);
			temp.x =Int(temp.x * ratioX);
			temp.y =Int(temp.y * ratioY);
			child->winSetSize(temp.x, temp.y);

			child->winGetPosition(&temp.x, &temp.y);
			temp.x =Int(temp.x * ratioX);
			temp.y =Int(temp.y * ratioY);
			child->winSetPosition(temp.x, temp.y);
		}
	}

	// If the user wants to position the message box somewhere other then default
	if( x >= 0 && y >= 0)
		parent->winSetPosition(x, y);

	// Reposition the buttons
	Int buttonX[3], buttonY[3];

	//In the layout, buttonOk will be in the first button position
	NameKeyType buttonOkID = TheNameKeyGenerator->nameToKey( AsciiString( "MessageBox.wnd:ButtonOk" ) );
	GameWindow *buttonOk = TheWindowManager->winGetWindowFromId(parent, buttonOkID);
	buttonOk->winGetPosition(&buttonX[0], &buttonY[0]);

	NameKeyType buttonYesID = TheNameKeyGenerator->nameToKey( AsciiString( "MessageBox.wnd:ButtonYes" ) );
	GameWindow *buttonYes = TheWindowManager->winGetWindowFromId(parent, buttonYesID);
	//buttonNo in the second position
	NameKeyType buttonNoID = TheNameKeyGenerator->nameToKey( AsciiString( "MessageBox.wnd:ButtonNo" ) );
	GameWindow *buttonNo = TheWindowManager->winGetWindowFromId(parent, buttonNoID);
	buttonNo->winGetPosition(&buttonX[1], &buttonY[1]);

	//and buttonCancel in the third
	NameKeyType buttonCancelID = TheNameKeyGenerator->nameToKey( AsciiString( "MessageBox.wnd:ButtonCancel" ) );
	GameWindow *buttonCancel = TheWindowManager->winGetWindowFromId(parent, buttonCancelID);
	buttonCancel->winGetPosition(&buttonX[2], &buttonY[2]);

	//we shouldn't have button OK and Yes on the same dialog
	if((buttonFlags & (MSG_BOX_OK | MSG_BOX_YES)) == (MSG_BOX_OK | MSG_BOX_YES) )
	{
		DEBUG_ASSERTCRASH(false, ("Passed in MSG_BOX_OK and MSG_BOX_YES.  Big No No."));
	}

	//Position the OK button if we have one
	if( (buttonFlags & MSG_BOX_OK) == MSG_BOX_OK)
	{
		buttonOk->winSetPosition(buttonX[0], buttonY[0]);
		buttonOk->winHide(FALSE);
	}
	else if( (buttonFlags & MSG_BOX_YES) == MSG_BOX_YES)
	{
		//Position the Yes if we have one
		buttonYes->winSetPosition(buttonX[0], buttonY[0]);
		buttonYes->winHide(FALSE);
	}

	if((buttonFlags & (MSG_BOX_NO | MSG_BOX_CANCEL)) == (MSG_BOX_NO | MSG_BOX_CANCEL) )
	{
		//If we have both the No and Cancel button, then the no should go in the middle position
		buttonNo->winSetPosition(buttonX[1], buttonY[1]);
		buttonCancel->winSetPosition(buttonX[2], buttonY[2]);
		buttonNo->winHide(FALSE);
		buttonCancel->winHide(FALSE);
	}
	else if( (buttonFlags & MSG_BOX_NO) == MSG_BOX_NO)
	{
		//if we just have the no button, then position it in the right most spot
		buttonNo->winSetPosition(buttonX[2], buttonY[2]);
		buttonNo->winHide(FALSE);
	}
	else if( (buttonFlags & MSG_BOX_CANCEL) == MSG_BOX_CANCEL)
	{
		//else if we just have the Cancel button, well, it should always go in the right spot
		buttonCancel->winSetPosition(buttonX[2], buttonY[2]);
		buttonCancel->winHide(FALSE);
	}

	// Fill the text into the text boxes
	NameKeyType staticTextTitleID = TheNameKeyGenerator->nameToKey( AsciiString( "MessageBox.wnd:StaticTextTitle" ) );
	GameWindow *staticTextTitle = TheWindowManager->winGetWindowFromId(parent, staticTextTitleID);
	GadgetStaticTextSetText(staticTextTitle,titleString);

	NameKeyType staticTextMessageID = TheNameKeyGenerator->nameToKey( AsciiString( "MessageBox.wnd:StaticTextMessage" ) );
	GameWindow *staticTextMessage = TheWindowManager->winGetWindowFromId(parent, staticTextMessageID);
	GadgetStaticTextSetText(staticTextMessage,bodyString);

	// create a structure that will pass the functions to
	WindowExMessageBoxData *MsgBoxCallbacks = NEW WindowExMessageBoxData;
	MsgBoxCallbacks->cancelCallback = cancelCallback;
	MsgBoxCallbacks->noCallback = noCallback;
	MsgBoxCallbacks->okCallback = okCallback;
	MsgBoxCallbacks->yesCallback = yesCallback;
	MsgBoxCallbacks->userData = userData;
	//pass the structure to the dialog
	parent->winSetUserData( MsgBoxCallbacks );

	parent->winSetSystemFunc(ExtendedMessageBoxSystem);

	//make sure the dialog is showing and bring it to the top
	parent->winHide(FALSE);
	parent->winBringToTop();

	return parent;
}// gogoExMessageBox

GameWindow *ExMessageBoxYesNo				(UnicodeString titleString,UnicodeString bodyString, void *userData,
																		 MessageBoxFunc yesCallback, MessageBoxFunc noCallback)
{
	return gogoExMessageBox(-1,-1,-1,-1,MSG_BOX_NO | MSG_BOX_YES , titleString, bodyString, userData, yesCallback, noCallback, NULL, NULL);
}

GameWindow *ExMessageBoxYesNoCancel	(UnicodeString titleString,UnicodeString bodyString, void *userData,
																		 MessageBoxFunc yesCallback, MessageBoxFunc noCallback, MessageBoxFunc cancelCallback)
{
	return gogoExMessageBox(-1,-1,-1,-1,MSG_BOX_NO | MSG_BOX_YES | MSG_BOX_CANCEL , titleString, bodyString, userData, yesCallback, noCallback, NULL, cancelCallback);
}

GameWindow *ExMessageBoxOkCancel		(UnicodeString titleString,UnicodeString bodyString, void *userData,
																		 MessageBoxFunc okCallback, MessageBoxFunc cancelCallback)
{
	return gogoExMessageBox(-1,-1,-1,-1,MSG_BOX_OK | MSG_BOX_CANCEL , titleString, bodyString, userData, NULL, NULL, okCallback, cancelCallback);
}

GameWindow *ExMessageBoxOk					(UnicodeString titleString,UnicodeString bodyString, void *userData,
																		 MessageBoxFunc okCallback)
{
	return gogoExMessageBox(-1,-1,-1,-1,MSG_BOX_OK, titleString, bodyString, userData, NULL, NULL, okCallback, NULL);
}

GameWindow *ExMessageBoxCancel			(UnicodeString titleString,UnicodeString bodyString, void *userData,
																		 MessageBoxFunc cancelCallback)
{
	return gogoExMessageBox(-1,-1,-1,-1, MSG_BOX_CANCEL, titleString, bodyString, userData, NULL, NULL, NULL, cancelCallback);
}



// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////



//-------------------------------------------------------------------------------------------------
/** Message Box window system callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType ExtendedMessageBoxSystem( GameWindow *window, UnsignedInt msg,
										 WindowMsgData mData1, WindowMsgData mData2 )
{


	switch( msg )
	{

		//---------------------------------------------------------------------------------------------
		case GWM_DESTROY:
		{
			delete (WindowExMessageBoxData *)window->winGetUserData();
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
			WindowExMessageBoxData *MsgBoxCallbacks = (WindowExMessageBoxData *)window->winGetUserData();

			MessageBoxReturnType ret = MB_RETURN_CLOSE;

			if( controlID == buttonOkID )
			{
				if (MsgBoxCallbacks->okCallback)
					ret = MsgBoxCallbacks->okCallback(MsgBoxCallbacks->userData);
			}  // end if
			else if( controlID == buttonYesID )
			{
				if (MsgBoxCallbacks->yesCallback)
					ret = MsgBoxCallbacks->yesCallback(MsgBoxCallbacks->userData);
			}  // end else if
			else if( controlID == buttonNoID )
			{
				if (MsgBoxCallbacks->noCallback)
					ret = MsgBoxCallbacks->noCallback(MsgBoxCallbacks->userData);
			}  // end else if
			else if( controlID == buttonCancelID )
			{
				if (MsgBoxCallbacks->cancelCallback)
					ret = MsgBoxCallbacks->cancelCallback(MsgBoxCallbacks->userData);
			}  // end else if

			if (ret == MB_RETURN_CLOSE)
				TheWindowManager->winDestroy(window);

			break;

		}  // end selected

		//---------------------------------------------------------------------------------------------
		default:
			return MSG_IGNORED;

	}  // end switch

	return MSG_HANDLED;

}  // end ExtendedMessageBoxSystem
