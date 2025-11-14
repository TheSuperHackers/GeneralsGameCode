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

// FILE: CheckBox.cpp /////////////////////////////////////////////////////////
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
// File name: CheckBox.cpp
//
// Created:   Colin Day, June 2001
//
// Desc:      Checkbox GUI control
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/Language.h"
#include "GameClient/Gadget.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/Keyboard.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// GadgetCheckBoxInput ========================================================
/** Handle input for check box */
//=============================================================================
WindowMsgHandledType GadgetCheckBoxInput( GameWindow *window, UnsignedInt msg,
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
				TheWindowManager->winSendSystemMsg( window->winGetOwner(),
																						GBM_MOUSE_ENTERING,
																						(WindowMsgData)window,
																						mData1 );
				//TheWindowManager->winSetFocus( window );

			}
			break;

		}

		// ------------------------------------------------------------------------
		case GWM_MOUSE_LEAVING:
		{

			if( BitIsSet( instData->getStyle(), GWS_MOUSE_TRACK ) )
			{

				BitClear( instData->m_state, WIN_STATE_HILITED );
				TheWindowManager->winSendSystemMsg( window->winGetOwner(),
																						GBM_MOUSE_LEAVING,
																						(WindowMsgData)window,
																						mData1 );
			}
			break;

		}

		// ------------------------------------------------------------------------
		case GWM_LEFT_DRAG:
		{

			TheWindowManager->winSendSystemMsg( window->winGetOwner(), GGM_LEFT_DRAG,
																					(WindowMsgData)window, mData1 );
			break;

		}

		// ------------------------------------------------------------------------
		case GWM_LEFT_DOWN:
		{

			break;

		}

		// ------------------------------------------------------------------------
		case GWM_LEFT_UP:
		{

			if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) == FALSE )
			{
				// this up click was not meant for this button
				return MSG_IGNORED;
			}

			// Toggle the check state
			instData->m_state ^= WIN_STATE_SELECTED;

			TheWindowManager->winSendSystemMsg( window->winGetOwner(), GBM_SELECTED,
																					(WindowMsgData)window, mData1 );


			break;

		}

		// ------------------------------------------------------------------------
		case GWM_RIGHT_DOWN:
		{

			break;
		}

		//-------------------------------------------------------------------------
		case GWM_RIGHT_UP:
		{
			// Need to be specially marked to care about right mouse events
			if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) )
			{
				TheWindowManager->winSendSystemMsg( instData->getOwner(), GBM_SELECTED_RIGHT,
																						(WindowMsgData)window, mData1 );

				BitClear( instData->m_state, WIN_STATE_SELECTED );
			}
			else
			{
				// this up click was not meant for this button
				return MSG_IGNORED;
			}
			break;
		}

		// ------------------------------------------------------------------------
		case GWM_CHAR:
		{

			switch( mData1 )
			{

				// --------------------------------------------------------------------
				case KEY_ENTER:
				case KEY_SPACE:
				{

					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
					{
						// Toggle the check state
						instData->m_state ^= WIN_STATE_SELECTED;

						TheWindowManager->winSendSystemMsg( window->winGetOwner(),
																								GBM_SELECTED,
																								(WindowMsgData)window,
																								0 );
					}
					break;

				}

				// --------------------------------------------------------------------
				case KEY_DOWN:
				case KEY_RIGHT:
				case KEY_TAB:
				{

					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
						TheWindowManager->winNextTab(window);
					break;

				}

				// --------------------------------------------------------------------
				case KEY_UP:
				case KEY_LEFT:
				{

					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
						TheWindowManager->winPrevTab(window);
					break;

				}

				// --------------------------------------------------------------------
				default:
				{

					return MSG_IGNORED;

				}

			}

			break;

		}

		// ------------------------------------------------------------------------
		default:
		{

			return MSG_IGNORED;

		}

	}

	return MSG_HANDLED;

}

// GadgetCheckBoxSystem =======================================================
/** Handle system messages for check box */
//=============================================================================
WindowMsgHandledType GadgetCheckBoxSystem( GameWindow *window, UnsignedInt msg,
													 WindowMsgData mData1, WindowMsgData mData2 )
{
	WinInstanceData *instData = window->winGetInstanceData();

	switch( msg )
	{
		// ------------------------------------------------------------------------
		case GGM_SET_LABEL:
		{
			window->winSetText( *(UnicodeString*)mData1 );
			break;
		}

		// ------------------------------------------------------------------------
		case GWM_CREATE:
			break;

		// ------------------------------------------------------------------------
		case GWM_DESTROY:
			break;

		// ------------------------------------------------------------------------
		case GWM_INPUT_FOCUS:

			if( mData1 == FALSE )
				BitClear( instData->m_state, WIN_STATE_HILITED );
			else
				BitSet( instData->m_state, WIN_STATE_HILITED );
			TheWindowManager->winSendSystemMsg( window->winGetOwner(),
																					GGM_FOCUS_CHANGE,
																					mData1,
																					window->winGetWindowId() );
			if( mData1 == FALSE )
				*(Bool*)mData2 = FALSE;
			else
				*(Bool*)mData2 = TRUE;

			break;

		default:
			return MSG_IGNORED;

	}

	return MSG_HANDLED;

}

// GadgetCheckBoxSetText ======================================================
/** Set the text for the control */
//=============================================================================
void GadgetCheckBoxSetText( GameWindow *g, UnicodeString text )
{

	// sanity
	if( g == NULL )
		return;

	TheWindowManager->winSendSystemMsg( g, GGM_SET_LABEL, (WindowMsgData)&text, 0 );

}

// GadgetCheckBoxSetChecked ============================================
//=============================================================================
/** Set the check state for the check box */
//=============================================================================
void GadgetCheckBoxSetChecked( GameWindow *g, Bool isChecked)
{
	WinInstanceData *instData = g->winGetInstanceData();
	if (isChecked)
	{
		BitSet(instData->m_state,  WIN_STATE_SELECTED);
	}
	else
	{
		BitClear(instData->m_state,  WIN_STATE_SELECTED);
	}

	TheWindowManager->winSendSystemMsg( g->winGetOwner(), GBM_SELECTED,
																					(WindowMsgData)g, 0 );

}

// GadgetCheckBoxToggle ============================================
//=============================================================================
/** Toggle the check state for the check box */
//=============================================================================
void GadgetCheckBoxToggle( GameWindow *g)
{
	WinInstanceData *instData = g->winGetInstanceData();
	Bool isChecked = BitIsSet(instData->m_state, WIN_STATE_SELECTED);
	if (isChecked)
	{
		BitClear(instData->m_state,  WIN_STATE_SELECTED);
	}
	else
	{
		BitSet(instData->m_state,  WIN_STATE_SELECTED);
	}

	TheWindowManager->winSendSystemMsg( g->winGetOwner(), GBM_SELECTED,
																					(WindowMsgData)g, 0 );

}

// GadgetCheckBoxIsChecked ======================================================
/** Check the check state */
//=============================================================================
Bool GadgetCheckBoxIsChecked( GameWindow *g )
{
	WinInstanceData *instData = g->winGetInstanceData();
	return (BitIsSet(instData->m_state, WIN_STATE_SELECTED));
}
