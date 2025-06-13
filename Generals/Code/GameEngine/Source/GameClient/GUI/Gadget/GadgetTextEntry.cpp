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

// FILE: TextEntry.cpp ////////////////////////////////////////////////////////
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
// File name: TextEntry.cpp
//
// Created:   Colin Day, June 2001
//
// Desc:      Text entry GUI gadget
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
#include "GameClient/IMEManager.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////
#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif
///////////////////////////////////////////////////////////////////////////////
// PRIVATE DATA ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
static Byte drawCnt = 0;
// static TbIME *ourIME = NULL;  ///< @todo need this for IME kanji support
static GameWindow *curWindow = NULL;  /**< so we can keep track of the input
																					 window when using IME */

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// GadgetTextEntryInput =======================================================
/** Handle input for text entry field */
//=============================================================================
WindowMsgHandledType GadgetTextEntryInput( GameWindow *window, UnsignedInt msg,
													 WindowMsgData mData1, WindowMsgData mData2 )
{
	EntryData *e = (EntryData *)window->winGetUserData();
	WinInstanceData *instData = window->winGetInstanceData();

	if ( TheIMEManager && TheIMEManager->isAttachedTo( window) && TheIMEManager->isComposing())
	{
		// ignore input while IME has focus
		return MSG_HANDLED;
	}

	switch( msg ) 
	{
		// ------------------------------------------------------------------------
		case GWM_IME_CHAR:
		{
			WideChar ch = (WideChar) mData1;

			// --------------------------------------------------------------------
			if ( ch == VK_RETURN )
			{
				// Done with this edit
			 		TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
			 																				GEM_EDIT_DONE,
			 																				(WindowMsgData)window, 
			 																				0 );
				return MSG_HANDLED;
			};

			if( ch )
			{
				// Constrain keys based on rules for entry box.
				if( e->numericalOnly )
				{
					if( TheWindowManager->winIsDigit( ch ) == 0 )
						return MSG_HANDLED;
				}
				if( e->alphaNumericalOnly )
				{
					if( TheWindowManager->winIsAlNum( ch ) == 0 )
						return MSG_HANDLED;
				}
				if ( e->aSCIIOnly )
				{
					if ( TheWindowManager->winIsAscii( ch ) == 0 )
					{
						return MSG_HANDLED;
					}
				}

				if( e->charPos < e->maxTextLen-1 )
				{
						e->text->appendChar( ch );
						e->sText->appendChar( L'*' );
						e->charPos++;
						TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																						GEM_UPDATE_TEXT,
																						(WindowMsgData)window, 
																						0 );
				}
			}
			break;
		}
		// ------------------------------------------------------------------------
		case GWM_CHAR:
			if ( BitIsSet( mData2, KEY_STATE_DOWN ) && BitIsSet( mData2, KEY_STATE_ALT | KEY_STATE_CONTROL ) )
			{
				return MSG_IGNORED; // text extries shouldn't care about CTRL+* or ALT+*
			}

			switch( mData1 )
			{
				/*
				// --------------------------------------------------------------------
				case KEY_KPENTER:
				case KEY_ENTER:
					// Done with this edit
					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
					{
						if( e->receivedUnichar == FALSE )
						{
							TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																									GEM_EDIT_DONE,
																									(WindowMsgData)window, 
																									0 );
						}
					}

					break;
				 */
				// --------------------------------------------------------------------
				// Don't process these keys
				case KEY_ESC:
				case KEY_PGUP:
				case KEY_PGDN:
				case KEY_HOME:
				case KEY_END:
				case KEY_F1:
				case KEY_F2:
				case KEY_F3:
				case KEY_F4:
				case KEY_F5:
				case KEY_F6:
				case KEY_F7:
				case KEY_F8:
				case KEY_F9:
				case KEY_F10:
				case KEY_F11:
				case KEY_F12:
				case KEY_CAPS:
				case KEY_DEL:
					return MSG_IGNORED;

				// --------------------------------------------------------------------
				case KEY_DOWN:
				case KEY_RIGHT:
				case KEY_TAB:

					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
					{
						GameWindow *parent;
						parent = window->winGetParent();
						if(parent && !BitIsSet(parent->winGetStyle(), GWS_COMBO_BOX))
							parent = NULL;
						if(parent)
							TheWindowManager->winNextTab(parent);
						else
							TheWindowManager->winNextTab(window);
					}
					
					break;

				// --------------------------------------------------------------------
				case KEY_UP:
				case KEY_LEFT:

					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
					{
						GameWindow *parent;
						parent = window->winGetParent();
						if(parent && !BitIsSet(parent->winGetStyle(), GWS_COMBO_BOX))
							parent = NULL;
						if(parent)
							TheWindowManager->winPrevTab(parent);
						else
							TheWindowManager->winPrevTab(window);
					}
					break;

				// --------------------------------------------------------------------
				case KEY_BACKSPACE:

					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
					{
						// if conCharPos != 0 this will fall through to next case.
						// it should be noted that conCharPos can only != 0 in Jap & Kor
						if( e->conCharPos == 0 )
						{
							if( e->charPos > 0 )
							{

								e->text->removeLastChar();
								e->sText->removeLastChar();
								e->charPos--;
								TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																								GEM_UPDATE_TEXT,
																								(WindowMsgData)window, 
																								0 );
							}  // end if
						}
					}
					break;

			}  // end switch( mData1 )

			break;

		// ------------------------------------------------------------------------
		case GWM_LEFT_DOWN:
			BitSet( instData->m_state, WIN_STATE_HILITED );
			TheWindowManager->winSetFocus( window );
			break;

		// ------------------------------------------------------------------------
		case GWM_MOUSE_ENTERING:

			if (BitIsSet( instData->getStyle(), GWS_MOUSE_TRACK ) )
			{

				BitSet( instData->m_state, WIN_STATE_HILITED );
				TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																						GBM_MOUSE_ENTERING,
																						(WindowMsgData)window, 0 );
				//TheWindowManager->winSetFocus( window );
			}

			break;

		// ------------------------------------------------------------------------
		case GWM_MOUSE_LEAVING:

			if( BitIsSet( instData->getStyle(), GWS_MOUSE_TRACK ) )
			{

				BitClear( instData->m_state, WIN_STATE_HILITED );
				TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																						GBM_MOUSE_LEAVING,
																						(WindowMsgData)window, 0 );
			}
			break;

		// ------------------------------------------------------------------------
		case GWM_LEFT_DRAG:

			if( BitIsSet( instData->getStyle(), GWS_MOUSE_TRACK ) )
				TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																						GGM_LEFT_DRAG,
																						(WindowMsgData)window, 0 );
			break;

		// ------------------------------------------------------------------------
		default:
			return MSG_IGNORED;

	}  // end switch( msg )

	return MSG_HANDLED;

}  // end GadgetTextEntryInput

// GadgetTextEntrySystem ======================================================
/** Handle system messages for entry field */
//=============================================================================
WindowMsgHandledType GadgetTextEntrySystem( GameWindow *window, UnsignedInt msg,
														WindowMsgData mData1, WindowMsgData mData2 )
{
	EntryData *e = (EntryData *)window->winGetUserData();
	WinInstanceData *instData = window->winGetInstanceData();

	switch( msg )
	{

		// ------------------------------------------------------------------------
		case GEM_GET_TEXT:
			*(UnicodeString*)mData2 = e->text->getText();
			break;

		// ------------------------------------------------------------------------
		case GEM_SET_TEXT:
		{

			const UnicodeString* ustr = (const UnicodeString*)mData1;
			e->text->setText( *ustr );
			e->charPos = ustr->getLength();
			e->constructText->setText( UnicodeString::TheEmptyString );
			e->conCharPos = 0;

			// set our secret text string to be filled with '*' the same length
			e->sText->setText( UnicodeString::TheEmptyString );
			Int len = ustr->getLength();
			for( Int i = 0; i < len; i++ )
				e->sText->appendChar( L'*' );

			break;

		}  // end set text

		// ------------------------------------------------------------------------
		case GWM_CREATE:

			break;

		// ------------------------------------------------------------------------
		case GWM_DESTROY:

			// delete the edit display string
			TheDisplayStringManager->freeDisplayString( e->text );
			TheDisplayStringManager->freeDisplayString( e->sText );
			TheDisplayStringManager->freeDisplayString( e->constructText );

			// delete construct list
			if( e->constructList )
				TheWindowManager->winDestroy( e->constructList );

			// free all edit data
			delete( (EntryData *)window->winGetUserData() );
			window->winSetUserData(NULL);
			e = NULL;
			break;

		// ------------------------------------------------------------------------
		case GWM_INPUT_FOCUS:
			if( mData1 == FALSE )
			{
				// If we're losing focus
				/// @todo need to enable this for IME support
				// ourIME->UnActivate();
				curWindow = NULL;
				BitClear( instData->m_state, WIN_STATE_SELECTED );
				BitClear( instData->m_state, WIN_STATE_HILITED );

				if( e->constructList )
					e->constructList->winHide( TRUE );
				e->constructText->setText( UnicodeString::TheEmptyString );
				e->conCharPos = 0;
				if(TheIMEManager && TheIMEManager->isAttachedTo(window))
					TheIMEManager->attach(NULL);
				//TheIMEManager->detatch();
			}
			else
			{
				curWindow = window;
				/// @todo need to enable this for IME support
				if (TheIMEManager)
					TheIMEManager->attach( window );
				// ourIME->Activate( (void *)ApplicationHWnd );
				BitSet( instData->m_state, WIN_STATE_SELECTED );
				BitSet( instData->m_state, WIN_STATE_HILITED );
			}

			TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																					GGM_FOCUS_CHANGE,
																					mData1, 
																					window->winGetWindowId() );

			*(Bool*)mData2 = TRUE;
			break;

		default:
			return MSG_IGNORED;

	}  // end switch( msg )

	return MSG_HANDLED;

}  // end GadgetTextEntrySystem

/** @todo we might want to do something like this if we use IME for language
	* support in this product */
/*
// used to create interface to IME
BoolCode InitializeEntryGadget( void )
{
	ourIME = NEW TbIME;

	ourIME->Composition_SetMaxLength( 11 );

	return TRUE;
}

// used to destroy interface to IME
BoolCode ShutdownEntryGadget( void )
{
	delete ourIME;
	ourIME = NULL;

	return TRUE;
}

void InformEntry( WideChar c )
{
	Int i, listCount = 0;
	EntryData *e;

	if( ourIME == NULL || curWindow == NULL )
		return;

	e = (EntryData *)curWindow->winGetUserData();

	if( (	(OurLanguage == LANGUAGE_ID_JAPANESE)			||
				(OurLanguage == LANGUAGE_ID_KOREAN)	)			&&
			(	(e->aSCIIOnly == FALSE )					&&
				(e->alphaNumericalOnly == FALSE )	&&
				(e->numericalOnly == FALSE ) ) )
	{
		e->receivedUnichar = TRUE;

		// we must eat the following keys
		switch( c )
		{
			case L'\a':
			case L'\b':
			case L'\f':
			case L'\t':
			case L'\v':
				return;

			// we must completely ignore the return key
			case L'\r':
			case L'\n':
				e->receivedUnichar = FALSE;
				return;
		}

		if( e->charPos < e->maxTextLen-1 )
		{
			e->text[ e->charPos++ ] = c;
			e->text[ e->charPos ] = 0;
		}

		// always update the construction buffer after a key has come through here.
		TheWindowManager->winStrcpy( e->constructText, (WideChar *)ourIME->Composition_Get() );
		e->conCharPos = NoxStrlen( e->constructText );

		// we might need to update our listbox
		listCount = ourIME->CandidateList_GetSize();

		if( TRUE )	//listCount == 0)
		{
			// if no entries just hide it and leave
			if( e->constructList )
				e->constructList->winHide( TRUE );
		}
		else
		{
			Int maxWidth = 0;
			ListboxData list = NULL;
			ICoord2D constructSize, sliderSize;

			WinHide( e->constructList, FALSE );
			list = (ListBoxData)e->constructList->winGetUserData();

			e->constructList->winGetSize( &constructSize.x, &constructSize.y );
			list->slider->winGetSize( &sliderSize.x, &sliderSize.y );

			TheWindowManager->winSendSystemMsg( e->constructList, GLM_DEL_ALL, 0, 0 );

			for( i=0; i<listCount; i++ )
			{
				Int tempWidth;

				WideChar *text = (WideChar *)ourIME->CandidateList_GetItem( i );

				TheWindowManager->winGetTextSize( e->constructList->instData.font,
																					text, NULL, &tempWidth, 0 );

				if( tempWidth > maxWidth )
					maxWidth = tempWidth;

				UnicodeString tmp(text);
				TheWindowManager->winSendSystemMsg( e->constructList, GLM_ADD_ENTRY,
																						(WindowMsgData)&tmp, -1 );
			}

			e->constructList->winSetSize( maxWidth + sliderSize.y,
																		constructSize.y );
		}
	}
}
*/

// GadgetTextEntrySetFont =====================================================
/** Set the font for a text entry control, we need to set the window
	* text font, the tooltip font, and the edit text display strings for
	* the text data itself and the secret text */
//=============================================================================
void GadgetTextEntrySetFont( GameWindow *g, GameFont *font )
{
	EntryData *entryData = (EntryData *)g->winGetUserData();
	DisplayString *dString;

	// set the font for the display strings all windows have
	dString = g->winGetInstanceData()->getTextDisplayString();
	if( dString )
		dString->setFont( font );
	dString = g->winGetInstanceData()->getTooltipDisplayString();
	if( dString )
		dString->setFont( font );

	// text entry specific
	if( entryData )
	{

		dString = entryData->text;
		if( dString )
			dString->setFont( font );
		dString = entryData->sText;
		if( dString )
			dString->setFont( font );

	}  // end if

}  // end GadgetTextEntrySetFont

// GadgetTextEntryGetText =======================================================
/** Get the text for a Text entry */
//=============================================================================
UnicodeString GadgetTextEntryGetText( GameWindow *textentry )
{

	// sanity
	if( textentry == NULL )
		return UnicodeString::TheEmptyString;

	// verify that this is a list box
	if( BitIsSet( textentry->winGetStyle(), GWS_ENTRY_FIELD ) == FALSE )
		return UnicodeString::TheEmptyString;

	UnicodeString result;
	TheWindowManager->winSendSystemMsg( textentry, GEM_GET_TEXT, 0, (WindowMsgData)&result );
	return result;

}  // end GadgetListBoxGetText
