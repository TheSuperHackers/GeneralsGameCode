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

// FILE: InGamePopupMessage.cpp /////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
//	created:	Jul 2002
//
//	Filename: 	InGamePopupMessage.cpp
//
//	author:		Chris Huybregts
//
//	purpose:	Init, input, and system for the in game message popup
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// USER INCLUDES //////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/GlobalData.h"
#include "Common/NameKeyGenerator.h"
#include "Common/version.h"
#include "Common/MessageStream.h"
#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/InGameUI.h"
#include "GameClient/DisplayStringManager.h"

//-----------------------------------------------------------------------------
// DEFINES ////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

static NameKeyType parentID = NAMEKEY_INVALID;
static NameKeyType staticTextMessageID = NAMEKEY_INVALID;
static NameKeyType buttonOkID = NAMEKEY_INVALID;


static GameWindow *parent = NULL;
static GameWindow *staticTextMessage = NULL;
static GameWindow *buttonOk = NULL;


static Bool pause = FALSE;
//-----------------------------------------------------------------------------
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------
/** Initialize the InGamePopupMessageInit menu */
//-------------------------------------------------------------------------------------------------
void InGamePopupMessageInit( WindowLayout *layout, void *userData )
{

	parentID = TheNameKeyGenerator->nameToKey(AsciiString("InGamePopupMessage.wnd:InGamePopupMessageParent"));
	parent = TheWindowManager->winGetWindowFromId(NULL, parentID);

	staticTextMessageID = TheNameKeyGenerator->nameToKey(AsciiString("InGamePopupMessage.wnd:StaticTextMessage"));
	staticTextMessage = TheWindowManager->winGetWindowFromId(parent, staticTextMessageID);
	buttonOkID = TheNameKeyGenerator->nameToKey(AsciiString("InGamePopupMessage.wnd:ButtonOk"));
	buttonOk = TheWindowManager->winGetWindowFromId(parent, buttonOkID);

	PopupMessageData *pMData = TheInGameUI->getPopupMessageData();

	if(!pMData)
	{
		DEBUG_ASSERTCRASH(pMData, ("We're in InGamePopupMessage without a pointer to pMData") );
		///< @todo: add a call to the close this bitch method when I implement it CLH
		return;
	}

	DisplayString *tempString = TheDisplayStringManager->newDisplayString();
	tempString->setText(pMData->message);
	tempString->setFont(staticTextMessage->winGetFont());
	tempString->setWordWrap(pMData->width - 14);
	Int width, height;
	tempString->getSize(&width, &height);
	TheDisplayStringManager->freeDisplayString(tempString);

	GadgetStaticTextSetText(staticTextMessage, pMData->message);
	// set the positions/sizes
	Int widthOk, heightOk;
	buttonOk->winGetSize(&widthOk, &heightOk);
	parent->winSetPosition( pMData->x, pMData->y);
	parent->winSetSize( pMData->width, height + 7 + 2 + 2 + heightOk + 2 );
	staticTextMessage->winSetPosition(  2,  2);
	staticTextMessage->winSetSize( pMData->width - 4, height + 7);
	buttonOk->winSetPosition(pMData->width - widthOk - 2, height + 7 + 2 + 2);
	staticTextMessage->winSetEnabledTextColors(pMData->textColor, 0);
	pause = pMData->pause;
	if(pMData->pause)
		TheWindowManager->winSetModal( parent );

	TheWindowManager->winSetFocus( parent );

	parent->winHide(FALSE);
	parent->winBringToTop();
}

//-------------------------------------------------------------------------------------------------
/** InGamePopupMessageInput callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType InGamePopupMessageInput( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 )
{

		switch( msg )
		{

			// --------------------------------------------------------------------------------------------
			case GWM_CHAR:
			{
				UnsignedByte key = mData1;
				UnsignedByte state = mData2;
	//			if (buttonPushed)
	//				break;

				switch( key )
				{

					// ----------------------------------------------------------------------------------------
					case KEY_ENTER:
					case KEY_ESC:
					{

						//
						// send a simulated selected event to the parent window of the
						// back/exit button
						//
						if( BitIsSet( state, KEY_STATE_UP ) )
						{
							TheWindowManager->winSendSystemMsg( window, GBM_SELECTED,
																								(WindowMsgData)buttonOk, buttonOkID );

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
/** InGamePopupMessageSystem callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType InGamePopupMessageSystem( GameWindow *window, UnsignedInt msg, WindowMsgData mData1, WindowMsgData mData2 )
{

  switch( msg )
	{

		// --------------------------------------------------------------------------------------------
		case GWM_CREATE:
		{

			break;

		}
    //---------------------------------------------------------------------------------------------
		case GWM_DESTROY:
		{

			break;

		}

    //----------------------------------------------------------------------------------------------
    case GWM_INPUT_FOCUS:
		{

			// if we're givin the opportunity to take the keyboard focus we must say we want it
			if( mData1 == TRUE )
				*(Bool *)mData2 = TRUE;

			break;

		}
    //---------------------------------------------------------------------------------------------
		case GBM_SELECTED:
		{
			GameWindow *control = (GameWindow *)mData1;
			Int controlID = control->winGetWindowId();

      if( controlID == buttonOkID )
			{
				if(!pause)
					TheMessageStream->appendMessage( GameMessage::MSG_CLEAR_INGAME_POPUP_MESSAGE );
				else
					TheInGameUI->clearPopupMessageData();
			}
			break;
		}
		default:
			return MSG_IGNORED;

	}


	return MSG_HANDLED;

}

//-----------------------------------------------------------------------------
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
