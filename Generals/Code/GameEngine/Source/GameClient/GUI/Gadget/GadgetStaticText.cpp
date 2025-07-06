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

// FILE: StaticText.cpp ///////////////////////////////////////////////////////
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
// File name: StaticText.cpp
//
// Created:   Colin Day, June 2001
//
// Desc:      Static text control
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/Language.h"
#include "GameClient/DisplayStringManager.h"
#include "GameClient/GameWindow.h"
#include "GameClient/Gadget.h"
#include "GameClient/GameWindowManager.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// GadgetStaticTextInput ======================================================
/** Handle input for text field */
//=============================================================================
WindowMsgHandledType GadgetStaticTextInput( GameWindow *window, UnsignedInt msg,
											      WindowMsgData mData1, WindowMsgData mData2 )
{

  switch( msg ) 
	{
		// ------------------------------------------------------------------------
    case GWM_CHAR:
      switch (mData1)
			{
					
        case KEY_DOWN:
        case KEY_RIGHT:
        case KEY_TAB:
					// Just in case some fool sets static text as a tab stop
					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
            window->winNextTab();
          break;

        case KEY_UP:
        case KEY_LEFT:
					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
            window->winPrevTab();
          break;

				default:
					return MSG_IGNORED;
      }

			break;

		default:
			return MSG_IGNORED;

  }
	return MSG_HANDLED;
 

}  // end GadgetStaticTextInput

// GadgetStaticTextSystem =====================================================
/** Handle system messages for text field */
//=============================================================================
WindowMsgHandledType GadgetStaticTextSystem( GameWindow *window, UnsignedInt msg,
														 WindowMsgData mData1, WindowMsgData mData2 )
{
//	WinInstanceData *instData = window->winGetInstanceData();

  switch( msg )
	{
		// ------------------------------------------------------------------------
		case GGM_GET_LABEL:
		{
			TextData *tData = (TextData *)window->winGetUserData();
			if (tData && tData->text)
				*(UnicodeString*)mData2 = tData->text->getText();
			break;
		
		}  // end get label

		// ------------------------------------------------------------------------
		case GGM_SET_LABEL:
		{
			if( mData1 )
			{
				TextData *tData = (TextData *)window->winGetUserData();
				if (tData && tData->text)
					tData->text->setText( *(UnicodeString*)mData1 );
			}

      break;

		}  // end set label

		// ------------------------------------------------------------------------
		case GWM_CREATE:
      break;

		// ------------------------------------------------------------------------
    case GWM_DESTROY:
		{
			TextData *data = (TextData *)window->winGetUserData();

			// free the display string
			TheDisplayStringManager->freeDisplayString( data->text );

			// free text data
			delete (TextData *)window->winGetUserData();
			window->winSetUserData( NULL );

      break;

		}  // end destroy

		default:
			return MSG_IGNORED;

  }  // end switch( msg )

	return MSG_HANDLED;
}  // end GadgetStaticTextSystem

// GadgetStaticTextSetText ====================================================
/** Set the text for a static text control */
//=============================================================================
void GadgetStaticTextSetText( GameWindow *window, UnicodeString text )
{
	if(!window)
		return;
	TheWindowManager->winSendSystemMsg( window, GGM_SET_LABEL, (WindowMsgData)&text, 0 );

}  // end GadgetStaticTextSetText

UnicodeString GadgetStaticTextGetText( GameWindow *window )
{
	if(!window)
		return UnicodeString::TheEmptyString;
	TextData *tData = (TextData *)window->winGetUserData();
	if(!tData)
		return UnicodeString::TheEmptyString;
	return tData->text->getText();
}

// GadgetStaticTextSetFont ====================================================
/** Set the font for a text control, we need to set the window
	* text font, the tooltip font, and the static text display strings for
	* the text data itself */
//=============================================================================
void GadgetStaticTextSetFont( GameWindow *g, GameFont *font )
{
	TextData *textData = (TextData *)g->winGetUserData();
	DisplayString *dString;

	// set the font for the display strings all windows have
	dString = g->winGetInstanceData()->getTextDisplayString();
	if( dString )
		dString->setFont( font );
	dString = g->winGetInstanceData()->getTooltipDisplayString();
	if( dString )
		dString->setFont( font );

	// static text specific
	if( textData )
	{

		dString = textData->text;
		if( dString )
			dString->setFont( font );

	}  // end if

}  // end GadgetStaticTextSetFont
