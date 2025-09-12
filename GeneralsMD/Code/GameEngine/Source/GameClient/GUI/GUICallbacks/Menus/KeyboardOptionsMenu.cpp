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

// FILE: KeyboardOptionsMenu.cpp /////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Electronic Arts Pacific.
//
//                       Confidential Information
//                Copyright (C) 2002 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:   Command & Conquer: Generals
//
// File name: KeyboardOptionsMenu.cpp
//
// Created:   Chris Brue, July 2002
//
// Desc:      the Keyboard options window control
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "Common/GameAudio.h"
#include "Common/GameEngine.h"
#include "Common/UserPreferences.h"

#include "GameClient/WindowLayout.h"
#include "GameClient/Gadget.h"
#include "GameClient/GadgetCheckBox.h"
#include "GameClient/GadgetComboBox.h"
#include "GameClient/GadgetListBox.h"
#include "GameClient/GadgetSlider.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/IMEManager.h"
#include "GameClient/Shell.h"
#include "GameClient/KeyDefs.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/Mouse.h"
#include "GameClient/GameText.h"
#include "GameClient/MetaEvent.h"

#include "GameNetwork/FirewallHelper.h"
#include "GameNetwork/IPEnumeration.h"

// PRIVATE DATA ///////////////////////////////////////////////////////////////////////////////////
WindowMsgHandledType KeyboardTextEntryInput( GameWindow *window, UnsignedInt msg,
													 WindowMsgData mData1, WindowMsgData mData2 );

static NameKeyType buttonBackID = NAMEKEY_INVALID;
static GameWindow *buttonBack = NULL;

static NameKeyType parentKeyboardOptionsMenuID = NAMEKEY_INVALID;
static GameWindow *parentKeyboardOptionsMenu = NULL;

static NameKeyType comboBoxCategoryListID = NAMEKEY_INVALID;
static GameWindow *comboBoxCategoryList = NULL;

static NameKeyType listBoxCommandListID = NAMEKEY_INVALID;
static GameWindow *listBoxCommandList   = NULL;

static NameKeyType staticTextDescriptionID = NAMEKEY_INVALID;
static GameWindow *staticTextDescription   = NULL;

static NameKeyType staticTextCurrentHotkeyID = NAMEKEY_INVALID;
static GameWindow *staticTextCurrentHotkey     = NULL;

static NameKeyType buttonResetAllID = NAMEKEY_INVALID;
static GameWindow *buttonResetAll   = NULL;

static NameKeyType textEntryAssignHotkeyID = NAMEKEY_INVALID;
static GameWindow *textEntryAssignHotkey   = NULL;

static NameKeyType buttonAssignID = NAMEKEY_INVALID;
static GameWindow *buttonAssign = NULL;

//use Bools to test if modifiers are used

Bool shiftDown = false;
Bool altDown = false;
Bool ctrlDown = false;

// shows whether or not a correctly formatted hotkey assignment is in the text area
Bool absolute = false;

// initialize these, they will be used a lot
UnicodeString alt;
UnicodeString ctrl;
UnicodeString shift;



void populateCategoryBox()
{
	Int i;
	Int index;
	Color color =  GameMakeColor(255,255,255,255);
	AsciiString temp;
	UnicodeString str;
	GadgetComboBoxReset(comboBoxCategoryList);
	for ( i = 0; i < CATEGORY_NUM_CATEGORIES; ++i)
	{
		temp.format("GUI:%s", CategoryListName[i]);
		str = TheGameText->fetch( temp );
		index = GadgetComboBoxAddEntry(comboBoxCategoryList, str, color);
	}

	GadgetComboBoxSetSelectedPos(comboBoxCategoryList, 0);
}

// keeps track of whether or not each text modifier is being currently displayed in the text entry field
void setKeyDown( UnicodeString mod, Bool b )
{
	if( mod == TheGameText->fetch( "KEYBOARD:Shift+" ) )
		shiftDown = b;
	else if( mod == TheGameText->fetch( "KEYBOARD:Ctrl+" ) )
		ctrlDown = b;
	else
		altDown = b;
}

// initialized the command list box
void fillCommandListBox( MappableKeyCategories cat )
{
	if(!listBoxCommandList)
		return;

	GadgetListBoxReset(listBoxCommandList);
	Color color =  GameMakeColor(255,255,255,255);

	for(const MetaMapRec *rec = TheMetaMap->getFirstMetaMapRec(); rec; rec = rec->m_next)
	{
		if(rec->m_category == cat)
			GadgetListBoxAddEntryText(listBoxCommandList, rec->m_displayName, color, -1, -1 );

	}
}

void doKeyUp(EntryData *e, UnicodeString mod )
{
	char c = e->text->getText().getCharAt( e->text->getTextLength() - 1);
	// if there are modifiers, check which ones exist and act accordingly
	if( c == '+' )
	{
		// if all of the mods are down, make string out of other two
		if( altDown && ctrlDown && shiftDown )
		{
			if( mod == shift )
			{
				UnicodeString temp = alt;
				temp.concat( ctrl );
				e->text->setText( temp );
				e->charPos = e->text->getTextLength();
				setKeyDown( mod, false );
			}
			else if( mod == alt )
			{
				UnicodeString temp = ctrl;
				temp.concat( shift );
				e->text->setText( temp );
				e->charPos = e->text->getTextLength();
				setKeyDown( mod, false );
			}
			else if( mod == ctrl )
			{
				UnicodeString temp = alt;
				temp.concat( shift );
				e->text->setText( temp );
				e->charPos = e->text->getTextLength();
				setKeyDown( mod, false );
			}
		}
		// if alt and ctrl are both down
		else if( altDown && ctrlDown )
		{
			if( mod == alt )
			{
				e->text->setText( ctrl );
				e->charPos = e->text->getTextLength();
				setKeyDown( mod, false );
			}
			else if( mod == ctrl )
			{
				e->text->setText( ctrl );
				e->charPos = e->text->getTextLength();
				setKeyDown( mod, false );
			}
		}
		// if alt and shift are both down
		else if( altDown && shiftDown )
		{
			if( mod == alt )
			{
				e->text->setText( shift );
				e->charPos = e->text->getTextLength();
				setKeyDown( mod, false );
			}
			else if( mod == shift )
			{
				e->text->setText( alt );
				e->charPos = e->text->getTextLength();
				setKeyDown( mod, false );
			}
		}
		// if ctrl and shift are both down
		else if( ctrlDown && shiftDown )
		{
			if( mod == ctrl )
			{
				e->text->setText( shift );
				e->charPos = e->text->getTextLength();
				setKeyDown( mod, false );
			}
			else if( mod == shift )
			{
				e->text->setText( ctrl );
				e->charPos = e->text->getTextLength();
				setKeyDown( mod, false );
			}
		}
		// else only one mod, just clear everything
		else
		{
			e->text->setText( UnicodeString::TheEmptyString );
			e->sText->setText( UnicodeString::TheEmptyString );
			e->charPos = e->text->getTextLength();
			setKeyDown( mod, false );
		}
	}
	else
	{
		// this absolute thang will/might need more than one test
		absolute = true;
	}
}

// preforms the correct action when a modifier key is pressed down
void doKeyDown(EntryData *e, UnicodeString mod )
{
	// simple cases if there are no mods present
	//sanity check
	if( e->text->getTextLength() <= 1 )
	{
		// reset text
		e->text->setText( mod );
		e->sText->setText( mod );
		e->charPos = e->text->getTextLength();
		setKeyDown( mod, true );
	}

	else //if( e->text->getTextLength() )
	{
		char c = e->text->getText().getCharAt( e->text->getTextLength() - 1);
		if( c != '+' && absolute)
		{
				e->text->setText( mod );
				e->sText->setText( mod );
				e->charPos = e->text->getTextLength();
				// try reseting all mods first
				setKeyDown( shift, false );
				setKeyDown( alt, false );
				setKeyDown( ctrl, false );

				setKeyDown( mod, true );
				absolute = false;

		}
		//else only allow modifiers are present
		else
		{
			if( mod == shift && shiftDown )
			{
			}
			else if( mod == ctrl && ctrlDown )
			{
			}
			else if( mod == alt && altDown )
			{
			}
			else
			{
				//figure out the cases for which mod goes first

				// puts shift at the end of the mods
				if( altDown && ctrlDown)
				{
					UnicodeString temp = alt;
					temp.concat( ctrl );
					temp.concat( mod );
					e->text->setText(temp);
					e->charPos = e->text->getTextLength();
					setKeyDown( mod, true );
				}
				// if alt and shift are down, puts ctrl in the middle
				else if( altDown && shiftDown )
				{
					UnicodeString temp = alt;
					temp.concat( ctrl );
					temp.concat( shift );
					e->text->setText( temp );
					e->charPos = e->text->getTextLength();
					setKeyDown( mod, true );
				}
				// puts either shift or ctrl after alt
				else if( altDown )
				{
					UnicodeString temp = alt;
					temp.concat( mod );
					e->text->setText(temp);
					e->charPos = e->text->getTextLength();
					setKeyDown( mod, true );
				}
				// puts alt infront of these two
				else if( ctrlDown && shiftDown )
				{
					UnicodeString temp = alt;
					temp.concat( ctrl );
					temp.concat( shift );
					e->text->setText( temp );
					e->charPos = e->text->getTextLength();
					setKeyDown( mod, true );
				}
				// if only ctrl+ is currently being displayed
				else if( ctrlDown )
				{
					// if it's alt, put it in front
					if( mod == alt )
					{
						UnicodeString temp = mod;
						temp.concat( ctrl );
						e->text->setText( temp );
						e->charPos = e->text->getTextLength();
						setKeyDown( mod, true );
					}
					//else put shift after ctrl
					else
					{
						UnicodeString temp = ctrl;
						temp.concat( mod );
						e->text->setText( temp );
						e->charPos = e->text->getTextLength();
						setKeyDown( mod, true );
					}
				}
				// else put alt or ctrl in front of shift
				else if( shiftDown )
				{
					UnicodeString temp = mod;
					temp.concat( shift );
					e->text->setText( temp );
					e->charPos = e->text->getTextLength();
					setKeyDown( mod, true );
				}

			}

		}
	}
}


//-------------------------------------------------------------------------------------------------
/** Initialize the options menu */
//-------------------------------------------------------------------------------------------------
void KeyboardOptionsMenuInit( WindowLayout *layout, void *userData )
{

	//set keyboard focus to main parent
	parentKeyboardOptionsMenuID = TheNameKeyGenerator->nameToKey("KeyboardOptionsMenu.wnd:ParentKeyboardOptionsMenu");
	parentKeyboardOptionsMenu = TheWindowManager->winGetWindowFromId( NULL, parentKeyboardOptionsMenuID );

	// get ids for our children controls
	buttonBackID = TheNameKeyGenerator->nameToKey( AsciiString("KeyboardOptionsMenu.wnd:ButtonBack") );
	buttonBack = TheWindowManager->winGetWindowFromId( parentKeyboardOptionsMenu, buttonBackID );

	comboBoxCategoryListID = TheNameKeyGenerator->nameToKey( "KeyboardOptionsMenu.wnd:ComboBoxCategoryList" );
	comboBoxCategoryList   = TheWindowManager->winGetWindowFromId( /*parentKeyboardOptionsMenu*/NULL, comboBoxCategoryListID );

	listBoxCommandListID   = TheNameKeyGenerator->nameToKey( "KeyboardOptionsMenu.wnd:ListBoxCommandList" );
	listBoxCommandList     = TheWindowManager->winGetWindowFromId( NULL, listBoxCommandListID );

	staticTextDescriptionID = TheNameKeyGenerator->nameToKey( "KeyboardOptionsMenu.wnd:StaticTextDescription" );
	staticTextDescription   = TheWindowManager->winGetWindowFromId( NULL, staticTextDescriptionID );

	staticTextCurrentHotkeyID = TheNameKeyGenerator->nameToKey( "KeyboardOptionsMenu.wnd:StaticTextCurrentHotkey" );
	staticTextCurrentHotkey   = TheWindowManager->winGetWindowFromId( NULL, staticTextCurrentHotkeyID );

	buttonResetAllID        = TheNameKeyGenerator->nameToKey( "KeyboardOptionsMenu.wnd:ButtonResetAll" );
	buttonResetAll          = TheWindowManager->winGetWindowFromId( NULL, buttonResetAllID );

	textEntryAssignHotkeyID = TheNameKeyGenerator->nameToKey( "KeyboardOptionsMenu.wnd:TextEntryAssignHotkey" );
	textEntryAssignHotkey   = TheWindowManager->winGetWindowFromId( NULL, textEntryAssignHotkeyID );

	buttonAssignID          = TheNameKeyGenerator->nameToKey( "KeyboardOptionsMenu.wnd:ButtonAssign" );
	buttonAssign            = TheWindowManager->winGetWindowFromId( NULL, buttonAssignID );



	//special text entry box that needs its own function
	textEntryAssignHotkey->winSetInputFunc( KeyboardTextEntryInput );

	// populate category combo box
	populateCategoryBox();

	// populate command list
	fillCommandListBox(CATEGORY_CONTROL);

	//disable textEntry until specific command is chosen
	textEntryAssignHotkey->winEnable( false );

	//clear textEntry field
	EntryData *e = (EntryData *)textEntryAssignHotkey->winGetUserData();
	e->text->setText( UnicodeString::TheEmptyString );
	e->charPos = e->text->getTextLength();

	// set up these strings because they will be called a lot
	alt   = TheGameText->fetch( "KEYBOARD:Alt+" );
	ctrl = TheGameText->fetch( "KEYBOARD:Ctrl+" );
	shift = TheGameText->fetch( "KEYBOARD:Shift+" );

	// show menu
	layout->hide( FALSE );

	// set keyboard focus to main parent
	TheWindowManager->winSetFocus( parentKeyboardOptionsMenu );
}

//-------------------------------------------------------------------------------------------------
/** options menu shutdown method */
//-------------------------------------------------------------------------------------------------
void KeyboardOptionsMenuShutdown( WindowLayout *layout, void *userData )
{
		// hide menu
	layout->hide( TRUE );

	// our shutdown is complete
	TheShell->shutdownComplete( layout );
}

//-------------------------------------------------------------------------------------------------
/** options menu update method */
//-------------------------------------------------------------------------------------------------
void KeyboardOptionsMenuUpdate( WindowLayout *layout, void *userData )
{

}

//-------------------------------------------------------------------------------------------------
/** Options menu input callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType KeyboardOptionsMenuInput( GameWindow *window, UnsignedInt msg,
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
						AsciiString buttonName( "KeyboardOptionsMenu.wnd:ButtonBack" );
						NameKeyType buttonID = TheNameKeyGenerator->nameToKey( buttonName );
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
/** options menu window system callback */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType KeyboardOptionsMenuSystem( GameWindow *window, UnsignedInt msg,
																				WindowMsgData mData1, WindowMsgData mData2 )
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

		// --------------------------------------------------------------------------------------------
		case GWM_INPUT_FOCUS:
		{

			// if we're givin the opportunity to take the keyboard focus we must say we want it
			if( mData1 == TRUE )
				*(Bool *)mData2 = TRUE;

			return MSG_HANDLED;

		}

		//---------------------------------------------------------------------------------------------
		case GCM_SELECTED:
		{
			GameWindow *control = (GameWindow *)mData1;
			Int controlID = control->winGetWindowId();

      if(controlID == comboBoxCategoryListID )
      {
        Int selected;
        GadgetComboBoxGetSelectedPos(comboBoxCategoryList, &selected);

				LookupListRec rec;
				rec = CategoryListName[selected];
				MappableKeyCategories cat = (MappableKeyCategories)(rec.value);
				fillCommandListBox( cat );

				//reset current hotkey description
				GadgetStaticTextSetText( staticTextDescription, TheGameText->fetch( "GUI:NULL" ) );

				//reset current hotkey text
				GadgetStaticTextSetText( staticTextCurrentHotkey, TheGameText->fetch( "GUI:NULL" ) );

				//clear textEntry field
				EntryData *e = (EntryData *)textEntryAssignHotkey->winGetUserData();
				e->text->setText( UnicodeString::TheEmptyString );
				e->charPos = e->text->getTextLength();

				//disable textEntry until specific command is chosen
				textEntryAssignHotkey->winEnable( false );

      }
			break;

		}

		// ---------------------------------------------------------------------------------------------
		case GLM_SELECTED:
		{
			GameWindow *control = (GameWindow *)mData1;
			Int controlID = control->winGetWindowId();

			if( controlID == listBoxCommandListID )
			{
				Int selected;
				GadgetListBoxGetSelected( listBoxCommandList,  &selected );
				UnicodeString str;
				str = GadgetListBoxGetText( listBoxCommandList, selected/*, Int column = 0*/ );
				for(const MetaMapRec *rec = TheMetaMap->getFirstMetaMapRec(); rec; rec = rec->m_next)
				{
					if(rec->m_displayName == str)
					{
						//set text in description window
						GadgetStaticTextSetText( staticTextDescription, rec->m_description );
						//set text in current hotkey text
						MappableKeyType type = rec->m_key;
						//enable text entry for assigning different hotkey
						textEntryAssignHotkey->winEnable( true );

						for (const LookupListRec* keyName = KeyNames; keyName->name; keyName++)
						{
							if( keyName->value == type )
							{
								const char *cptr = keyName->name;
								AsciiString aStr;
								aStr.format( cptr );
								UnicodeString uStr;
								uStr.translate( aStr );

								GadgetStaticTextSetText( staticTextCurrentHotkey, uStr );
								break;
							}
						}

						break;
					}
				}

			}

			break;

		}

		// ---------------------------------------------------------------------------------------------
		case GBM_SELECTED:
		{
			GameWindow *control = (GameWindow *)mData1;
			Int controlID = control->winGetWindowId();

			if( controlID == buttonBackID )
			{

				// go back one screen
				TheShell->pop();

			}
			else if( controlID == buttonAssignID )
			{
				// check grammar in text field
			}
			else if( controlID == buttonResetAllID )
			{
				// populate category combo box
				populateCategoryBox();

				// populate command list
				fillCommandListBox(CATEGORY_CONTROL);

				//reset current hotkey text
				GadgetStaticTextSetText( staticTextCurrentHotkey, TheGameText->fetch( "GUI:NULL" ) );

				//clear textEntry field
				EntryData *e = (EntryData *)textEntryAssignHotkey->winGetUserData();
				e->text->setText( UnicodeString::TheEmptyString );
				e->charPos = e->text->getTextLength();

				//set all mods to false
				setKeyDown(alt, false );
				setKeyDown(ctrl, false );
				setKeyDown(shift, false );

				//disable text entry
				textEntryAssignHotkey->winEnable( false );

			}

			break;

		}

		default:
			return MSG_IGNORED;

	}

	return MSG_HANDLED;

}

// KeyboardTextEntryInput =======================================================
/** Handle input for text entry field */
//=============================================================================
WindowMsgHandledType KeyboardTextEntryInput( GameWindow *window, UnsignedInt msg,
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
				else if( e->alphaNumericalOnly )
				{
					if( TheWindowManager->winIsAlNum( ch ) == 0 )
						return MSG_HANDLED;
				}
				else if ( e->aSCIIOnly )
				{
					if ( TheWindowManager->winIsAscii( ch ) == 0 )
					{
						return MSG_HANDLED;
					}
				}

				if( e->text->getTextLength() <= 1 )
				{
					e->text->setText( UnicodeString::TheEmptyString );
					e->text->appendChar( ch );
					e->charPos = e->text->getTextLength();
					TheWindowManager->winSendSystemMsg( window->winGetOwner(),
																					GEM_UPDATE_TEXT,
																					(WindowMsgData)window,
																					0 );
					return MSG_HANDLED;
				}
				//else check is modifiers are persent
				else
				{
					char c = e->text->getText().getCharAt(e->text->getTextLength() - 1 );
					if(c == '+' )
					{
						e->text->appendChar( ch );
						e->charPos++;
						TheWindowManager->winSendSystemMsg( window->winGetOwner(),
																						GEM_UPDATE_TEXT,
																						(WindowMsgData)window,
																						0 );
						return MSG_HANDLED;
					}
					// if not, reset textEntry
					else
					{
						//if any of the modifiers are down, just replace letter
						if( ( shiftDown | ctrlDown | altDown ) && ( !absolute ) )
						{
							char test = e->text->getText().getCharAt(e->text->getTextLength() - 1);
							// only replace letter if not the same as last char of string (removes flickering)
							if( test != ch )
							{
								e->text->removeLastChar();
								e->text->appendChar( ch );
								TheWindowManager->winSendSystemMsg( window->winGetOwner(),
																								GEM_UPDATE_TEXT,
																								(WindowMsgData)window,
																								0 );
							}
						}
						//else reset textEntry
						else
						{
							e->text->setText( UnicodeString::TheEmptyString );
							e->text->appendChar( ch );
							e->charPos = 1;
							TheWindowManager->winSendSystemMsg( window->winGetOwner(),
																							GEM_UPDATE_TEXT,
																							(WindowMsgData)window,
																							0 );
						}
						return MSG_HANDLED;
					}
				}


			}
			break;
		}
		// ------------------------------------------------------------------------
		case GWM_CHAR:

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

				// -------------------------------------------------------------------------------------------
				// modifier cases

				case KEY_LCTRL:
				{
					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
					{
						UnicodeString mod = ctrl;
						doKeyDown( e, mod );
						TheWindowManager->winSendSystemMsg( window->winGetOwner(),
																GEM_UPDATE_TEXT,
																(WindowMsgData)window,
																0 );

						return MSG_HANDLED;
					}
					if( BitIsSet(mData2, KEY_STATE_UP ) )
					{
							UnicodeString mod = ctrl;
							doKeyUp( e, mod );
							TheWindowManager->winSendSystemMsg( window->winGetOwner(),
																						GEM_UPDATE_TEXT,
																						(WindowMsgData)window,
																						0 );

							return MSG_HANDLED;
					}
					break;
				}

				case KEY_RSHIFT:
				case KEY_LSHIFT:
				{
					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
					{
						UnicodeString mod = shift;
						doKeyDown( e, mod );
						TheWindowManager->winSendSystemMsg( window->winGetOwner(),
																GEM_UPDATE_TEXT,
																(WindowMsgData)window,
																0 );

						return MSG_HANDLED;

					}
					if( BitIsSet( mData2, KEY_STATE_UP ) )
					{
						UnicodeString mod = shift;
						doKeyUp(e, mod );

						TheWindowManager->winSendSystemMsg( window->winGetOwner(),
																						GEM_UPDATE_TEXT,
																						(WindowMsgData)window,
																						0 );


						return MSG_HANDLED;
					}
					break;
				}

				case KEY_LALT:
				{
					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
					{
						UnicodeString mod = alt;
						doKeyDown( e, mod );

						TheWindowManager->winSendSystemMsg( window->winGetOwner(),
																GEM_UPDATE_TEXT,
																(WindowMsgData)window,
																0 );

						return MSG_HANDLED;

					}
					if( BitIsSet(mData2, KEY_STATE_UP ) )
					{
						UnicodeString mod = alt;
						doKeyUp( e, mod );
						TheWindowManager->winSendSystemMsg( window->winGetOwner(),
																GEM_UPDATE_TEXT,
																(WindowMsgData)window,
																0 );

						return MSG_HANDLED;
					}
					break;
				}

				// -------------------------------------------------------------------------------------------


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
					return MSG_IGNORED;

				// --------------------------------------------------------------------
				case KEY_DOWN:
				case KEY_RIGHT:
				case KEY_TAB:

					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
						window->winNextTab();
					break;

				// --------------------------------------------------------------------
				case KEY_UP:
				case KEY_LEFT:

					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
						window->winPrevTab();
					break;

				// --------------------------------------------------------------------
				case KEY_BACKSPACE:
				{
					e->text->setText( UnicodeString::TheEmptyString );
					e->charPos = e->text->getTextLength();
					TheWindowManager->winSendSystemMsg( window->winGetOwner(),
																					GEM_UPDATE_TEXT,
																					(WindowMsgData)window,
																					0 );
					setKeyDown(shift, false );
					setKeyDown(ctrl, false );
					setKeyDown(alt, false );
					return MSG_HANDLED;

					break;
				}
				case KEY_DEL:
				{

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
							}
						}
					}
					break;
				}

				// ----------------------------------------------------------------------------------------
				// doing research to see if this will fix the keyboard stuff
				/*default:
				{
					char ch = mData1;
					if( ch && ( BitIsSet( mData2, KEY_STATE_DOWN ) ) )
					{
						// Constrain keys based on rules for entry box.
						if( e->numericalOnly )
						{
							if( TheWindowManager->winIsDigit( ch ) == 0 )
								return MSG_HANDLED;
						}
						else if( e->alphaNumericalOnly )
						{
							if( TheWindowManager->winIsAlNum( ch ) == 0 )
								return MSG_HANDLED;
						}
						else if ( e->aSCIIOnly )
						{
							if ( TheWindowManager->winIsAscii( ch ) == 0 )
							{
								return MSG_HANDLED;
							}
						}

						if( e->text->getTextLength() <= 1 )
						{
							e->text->setText( UnicodeString::TheEmptyString );
							e->text->appendChar( ch );
							e->charPos = e->text->getTextLength();
							TheWindowManager->winSendSystemMsg( window->winGetOwner(),
																							GEM_UPDATE_TEXT,
																							(WindowMsgData)window,
																							0 );
							return MSG_HANDLED;
						}
						//else check is modifiers are persent
						else
						{
							char c = e->text->getText().getCharAt(e->text->getTextLength() - 1 );
							if(c == '+' )
							{
								e->text->appendChar( ch );
								e->charPos++;
								TheWindowManager->winSendSystemMsg( window->winGetOwner(),
																								GEM_UPDATE_TEXT,
																								(WindowMsgData)window,
																								0 );
								return MSG_HANDLED;
							}
							// if not, reset textEntry
							else
							{
								//if any of the modifiers are down, just replace letter
								if( ( shiftDown | ctrlDown | altDown ) && ( !absolute ) )
								{
									char test = e->text->getText().getCharAt(e->text->getTextLength() - 1);
									// only replace letter if not the same as last char of string (removes flickering)
									if( test != ch )
									{
										e->text->removeLastChar();
										e->text->appendChar( ch );
										TheWindowManager->winSendSystemMsg( window->winGetOwner(),
																										GEM_UPDATE_TEXT,
																										(WindowMsgData)window,
																										0 );
									}
								}
								//else reset textEntry
								else
								{
									e->text->setText( UnicodeString::TheEmptyString );
									e->text->appendChar( ch );
									e->charPos = 1;
									TheWindowManager->winSendSystemMsg( window->winGetOwner(),
																									GEM_UPDATE_TEXT,
																									(WindowMsgData)window,
																									0 );
								}
								return MSG_HANDLED;
							}
						}


					}
				}*/


			}

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
				TheWindowManager->winSetFocus( window );
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

	}

	return MSG_HANDLED;

}


