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

// FILE: RadioButton.cpp //////////////////////////////////////////////////////
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
// File name: RadioButton.cpp
//
// Created:   Colin Day, June 2001
//
// Desc:      Radio button GUI control
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/Language.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/Gadget.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// doRadioUnselect ============================================================
/** Do the unselect of matching group not including exception window */
//=============================================================================
static void doRadioUnselect( GameWindow *window, Int group, Int screen,
												GameWindow *except )
{
	
	//
	// if this is a radio button we have something to consider, but we
	// will ignore the except window
	//
	if( window != except && BitIsSet( window->winGetStyle(), GWS_RADIO_BUTTON ) )
	{
		RadioButtonData *radioData = (RadioButtonData *)window->winGetUserData();

		if( radioData->group == group && radioData->screen == screen )
		{
			WinInstanceData *instData = window->winGetInstanceData();

			BitClear( instData->m_state, WIN_STATE_SELECTED );

		}  // end if
					
	}  // end if

	// recursively call on all my children
	GameWindow *child;

	for( child = window->winGetChild(); child; child = child->winGetNext() )
		doRadioUnselect( child, group, screen, except );

}  // end doRadioUnselect

// unselectOtherRadioOfGroup ==================================================
/** Go through the entire window system, including child windows and
	* unselect any radio buttons of the specified group, but not the
	* window specified */
//=============================================================================
static void unselectOtherRadioOfGroup( Int group, Int screen, 
																			 GameWindow *except )
{
	GameWindow *window = TheWindowManager->winGetWindowList();

	for( window = TheWindowManager->winGetWindowList();
			 window;
			 window = window->winGetNext() )
		doRadioUnselect( window, group, screen, except );

}  // end unselectOtherRadioOfGroup

// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////

// GadgetRadioButtonInput =====================================================
/** Handle input for radio button */
//=============================================================================
WindowMsgHandledType GadgetRadioButtonInput( GameWindow *window, UnsignedInt msg,
														 WindowMsgData mData1, WindowMsgData mData2 )
{
	WinInstanceData *instData = window->winGetInstanceData();

	switch( msg ) 
	{

		// ------------------------------------------------------------------------
		case GWM_MOUSE_ENTERING:
		{

			if( BitIsSet( instData->getStyle(), GWS_MOUSE_TRACK ) ) 
			{

				BitSet( instData->m_state, WIN_STATE_HILITED );
				TheWindowManager->winSendSystemMsg( instData->getOwner(), 
																						GBM_MOUSE_ENTERING,
																						(WindowMsgData)window, 
																						mData1 );
				//TheWindowManager->winSetFocus( window );

			}  // end if

			break;

		}  // end mouse enter

		// ------------------------------------------------------------------------
		case GWM_MOUSE_LEAVING:
		{

			if( BitIsSet( instData->getStyle(), GWS_MOUSE_TRACK ) ) 
			{

				BitClear( instData->m_state, WIN_STATE_HILITED );
				TheWindowManager->winSendSystemMsg( instData->getOwner(), 
																						GBM_MOUSE_LEAVING,
																					  (WindowMsgData)window, 
																						mData1 );
			}  // end if

			break;

		}  // end mouse leaving

		// ------------------------------------------------------------------------
		case GWM_LEFT_DRAG:
		{

			TheWindowManager->winSendSystemMsg( instData->getOwner(), GGM_LEFT_DRAG,
																					(WindowMsgData)window, mData1 );
			break;

		}  // end left drag

		// ------------------------------------------------------------------------
		case GWM_LEFT_DOWN:
		{

			break;

		}  // end down

		// ------------------------------------------------------------------------
		case GWM_LEFT_UP:
		{

			if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) == FALSE )
			{
				RadioButtonData *radioData = (RadioButtonData *)window->winGetUserData();

				TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																						GBM_SELECTED,
																						(WindowMsgData)window, 
																						mData1 );

				//
				// unselect any windows in the system (including children) that
				// are radio buttons with this same group and screen ID
				//
				if( radioData->group != 0 )
					unselectOtherRadioOfGroup(radioData->group, radioData->screen, window );

				// this button is now selected
				BitSet( instData->m_state, WIN_STATE_SELECTED );

			}  // end if, not selected
			else if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) == FALSE )
			{

				// this up click was not meant for this button
				return MSG_IGNORED;

			}  // end else if

			break;

		}  // end left up or click

		// ------------------------------------------------------------------------
		case GWM_CHAR:
		{

			switch( mData1 ) 
			{

				// --------------------------------------------------------------------
				case KEY_ENTER:
				case KEY_SPACE:
					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
					{

						if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) == FALSE )
						{
							RadioButtonData *radioData = (RadioButtonData *)window->winGetUserData();

							TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																									GBM_SELECTED,
																									(WindowMsgData)window, 
																									mData1 );

							//
							// unselect any windows in the system (including children) that
							// are radio buttons with this same group and screen ID
							//
							if( radioData->group != 0 )
								unselectOtherRadioOfGroup(radioData->group, radioData->screen, window );

							// this button is now selected
							BitSet( instData->m_state, WIN_STATE_SELECTED );

						}  // end if, not selected

					}  // end key down

					break;

				// --------------------------------------------------------------------
				case KEY_DOWN:
				case KEY_RIGHT:
				case KEY_TAB:
				{

					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
						window->winNextTab();
					break;

				}  // end down, right, or tab

				// --------------------------------------------------------------------
				case KEY_UP:
				case KEY_LEFT:
				{

					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
						window->winPrevTab();
					break;

				}  // end up, left

				// --------------------------------------------------------------------
				default:
				{

					return MSG_IGNORED;

				}  // end default

			}  // end switch( mData1 )

			break;

		}  // end char messsage

		// ------------------------------------------------------------------------
		default:
		{

			return MSG_IGNORED;

		}  // end default

	}  // end switch( msg )

	return MSG_HANDLED;

}  // end GadgetRadioButtonInput

// GadgetRadioButtonSystem ====================================================
/** Handle system messages for radio button */
//=============================================================================
WindowMsgHandledType GadgetRadioButtonSystem( GameWindow *window, UnsignedInt msg,
															WindowMsgData mData1, WindowMsgData mData2 )
{
	WinInstanceData *instData = window->winGetInstanceData();

	switch( msg ) 
	{

		// ------------------------------------------------------------------------
		case GBM_SET_SELECTION:
		{

			if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) == FALSE )
			{

				// do we want to send a selected message?
				if( (Bool)mData1 == TRUE )
				{

					TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																							GBM_SELECTED,
																							(WindowMsgData)window, 
																							0 );
				}  // end if

				//
				// unselect any windows in the system (including children) that
				// are radio buttons with this same group and screen ID
				//
				RadioButtonData *radioData = (RadioButtonData *)window->winGetUserData();
				if( radioData->group != 0 )
					unselectOtherRadioOfGroup(radioData->group, radioData->screen, window );

				// this button is now selected
				BitSet( instData->m_state, WIN_STATE_SELECTED );

			}  // end if

			break;

		}  // end set selection

		// ------------------------------------------------------------------------
		case GGM_SET_LABEL:
		{

			window->winSetText( *(UnicodeString*)mData1 );
			break;

		}  // end set label

		// ------------------------------------------------------------------------
		case GWM_CREATE:
			break;

		// ------------------------------------------------------------------------
		case GWM_DESTROY:
		{
			RadioButtonData *radioData = (RadioButtonData *)window->winGetUserData();

			// free radio button user data
			delete radioData;

			break;
		
		}  // end destroy

		// ------------------------------------------------------------------------
		case GWM_INPUT_FOCUS:
		{

			if( mData1 == FALSE )
				BitClear( instData->m_state, WIN_STATE_HILITED );

			TheWindowManager->winSendSystemMsg( window->winGetOwner(),
																					GGM_FOCUS_CHANGE,
																					mData1, 
																					window->winGetWindowId() );

			*(Bool*)mData2 = TRUE;
			break;

		}  // end focus

		default:
			return MSG_IGNORED;

	}  // end switch( msg )

	return MSG_HANDLED;

}  // end GadgetRadioButtonSystem

// GadgetRadioSetText =========================================================
/** Set the text for the control */
//=============================================================================
void GadgetRadioSetText( GameWindow *g, UnicodeString text )
{

	// sanity
	if( g == NULL )
		return;

	TheWindowManager->winSendSystemMsg( g, GGM_SET_LABEL, (WindowMsgData)&text, 0 );

}  // end GadgetRadioSetText

// GadgetRadioSetGroup ========================================================
/** Set the group number for a radio button, only one radio button of
	* a group can be selected at any given time */
//=============================================================================
void GadgetRadioSetGroup( GameWindow *g, Int group, Int screen )
{
	RadioButtonData *radioData = (RadioButtonData *)g->winGetUserData();

	radioData->group = group;
	radioData->screen = screen;

}  // end GadgetRadioSetGroup


// GadgetRadioSetText =========================================================
/** Set the text for the control */
//=============================================================================
void GadgetRadioSetSelection( GameWindow *g, Bool sendMsg )
{

	// sanity
	if( g == NULL )
		return;

	TheWindowManager->winSendSystemMsg( g, GBM_SET_SELECTION, (WindowMsgData)&sendMsg, 0 );

}  // end GadgetRadioSetText

