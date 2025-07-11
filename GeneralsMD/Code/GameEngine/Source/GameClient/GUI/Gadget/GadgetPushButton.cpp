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

// FILE: GadgetPushButton.cpp /////////////////////////////////////////////////
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
// File name: PushButton.cpp
//
// Created:   Colin Day, June 2001
//
// Desc:      Pushbutton GUI gadget control callbacks
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/AudioEventRTS.h"
#include "Common/Language.h"
#include "Common/GameAudio.h"
#include "GameClient/Gadget.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/InGameUI.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////

static Bool buttonTriggersOnMouseDown(GameWindow *window)
{
	// Buttons with the on down status set trigger on mouse down. jba. [8/6/2003]
	Bool onDown = BitIsSet( window->winGetStatus(), WIN_STATUS_ON_MOUSE_DOWN);

	// Checkboxes always trigger on mouse down. jba [8/6/2003]
	if (BitIsSet( window->winGetStatus(), WIN_STATUS_CHECK_LIKE )) {
		onDown = true;
	}
	return onDown;
}

// GadgetPushButtonInput ======================================================
/** Handle input for push button */
//=============================================================================
WindowMsgHandledType GadgetPushButtonInput( GameWindow *window, 
																						UnsignedInt msg,
																						WindowMsgData mData1, 
																						WindowMsgData mData2 )
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
			}
			if(window->winGetParent() && BitIsSet(window->winGetParent()->winGetStyle(),GWS_HORZ_SLIDER) )
			{
				WinInstanceData *instDataParent = window->winGetParent()->winGetInstanceData();
				BitSet(instDataParent->m_state, WIN_STATE_HILITED);
			}
			break;

		}  // end mouse entering

		// ------------------------------------------------------------------------
		case GWM_MOUSE_LEAVING:
		{

			if(BitIsSet( instData->getStyle(), GWS_MOUSE_TRACK ) ) 
			{
				BitClear( instData->m_state, WIN_STATE_HILITED );
				TheWindowManager->winSendSystemMsg( instData->getOwner(), 
																						GBM_MOUSE_LEAVING,
																						(WindowMsgData)window, 
																						mData1 );
			}
		
			//
			// if this is not a check-like button, clear any selected state when the
			// move leaves the window area
			//
			if( BitIsSet( window->winGetStatus(), WIN_STATUS_CHECK_LIKE ) == FALSE )
				if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) )
					BitClear( instData->m_state, WIN_STATE_SELECTED );
			//TheWindowManager->winSetFocus( NULL );
			if(window->winGetParent() && BitIsSet(window->winGetParent()->winGetStyle(),GWS_HORZ_SLIDER) )
			{
				WinInstanceData *instDataParent = window->winGetParent()->winGetInstanceData();
				BitClear(instDataParent->m_state, WIN_STATE_HILITED);
			}
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
			PushButtonData *pData = (PushButtonData *)window->winGetUserData();
			AudioEventRTS buttonClick;
			if(pData && pData->altSound.isNotEmpty())
				buttonClick.setEventName(pData->altSound);
			else
				buttonClick.setEventName("GUIClick");

			if( TheAudio )
			{
				TheAudio->addAudioEvent( &buttonClick );
			}  // end if

			//
			// for 'check-like' buttons we have "dual state", we flip the selected status
			// in that case instead of just turning it on like normal ... also note
			// that selected messages are sent immediately
			//
			if( BitIsSet( window->winGetStatus(), WIN_STATUS_CHECK_LIKE ) )
			{
				
				if( BitIsSet( instData->m_state, WIN_STATE_SELECTED ) )
					BitClear( instData->m_state, WIN_STATE_SELECTED );
				else
					BitSet( instData->m_state, WIN_STATE_SELECTED );


			}  // end if
			else
			{
				
				// just select as normal
				BitSet( instData->m_state, WIN_STATE_SELECTED );

			}  // end else

			if (buttonTriggersOnMouseDown(window)) {
				TheWindowManager->winSendSystemMsg( instData->getOwner(), GBM_SELECTED,
																						(WindowMsgData)window, mData1 );
			}

			break;
		}  // end left down

		//-------------------------------------------------------------------------
		case GWM_LEFT_UP:
		{

			//
			// note check like selected messages aren't sent here ... they are sent
			// on the down press
			//
			if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) &&
					BitIsSet( window->winGetStatus(), WIN_STATUS_CHECK_LIKE ) == FALSE )
			{

				if (!buttonTriggersOnMouseDown(window)) {
					// If it didn't trigger on mouse down, trigger on the mouse up. jba  [8/6/2003]
					TheWindowManager->winSendSystemMsg( instData->getOwner(), GBM_SELECTED,
																							(WindowMsgData)window, mData1 );
				}

				BitClear( instData->m_state, WIN_STATE_SELECTED );

			}
			else
			{

				// this up click was not meant for this button
				return MSG_IGNORED;

			}

			break;

		}  // end left up or left click

		// ------------------------------------------------------------------------
		case GWM_RIGHT_DOWN:
		{
			PushButtonData *pData = (PushButtonData *)window->winGetUserData();
			AudioEventRTS buttonClick;
			if(pData && pData->altSound.isNotEmpty())
				buttonClick.setEventName(pData->altSound);
			else
				buttonClick.setEventName("GUIClick");
				

			if( BitIsSet( instData->getStatus(), WIN_STATUS_RIGHT_CLICK ) )
			{
				// Need to be specially marked to care about right mouse events
				if( TheAudio )
				{
					TheAudio->addAudioEvent( &buttonClick );
				}  // end if

				//
				// for 'check-like' buttons we have "dual state", we flip the selected status
				// in that case instead of just turning it on like normal ... also note
				// that selected messages are sent immediately
				//
				if( BitIsSet( window->winGetStatus(), WIN_STATUS_CHECK_LIKE ) )
				{
					
					if( BitIsSet( instData->m_state, WIN_STATE_SELECTED ) )
						BitClear( instData->m_state, WIN_STATE_SELECTED );
					else
						BitSet( instData->m_state, WIN_STATE_SELECTED );

					TheWindowManager->winSendSystemMsg( instData->getOwner(), GBM_SELECTED_RIGHT,
																							(WindowMsgData)window, mData1 );

				}  // end if
				else
				{
					
					// just select as normal
					BitSet( instData->m_state, WIN_STATE_SELECTED );

				}  // end else

			}
			else
			{
				// Else I don't care about right events
				return MSG_IGNORED;
			}
			break;
		}  // end right down

		//-------------------------------------------------------------------------
		case GWM_RIGHT_UP:
		{
			
			if( BitIsSet( instData->getStatus(), WIN_STATUS_RIGHT_CLICK ) )
			{

				//
				// note check like selected messages aren't sent here ... they are sent
				// on the down press
				//
				if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) &&
						BitIsSet( window->winGetStatus(), WIN_STATUS_CHECK_LIKE ) == FALSE )
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

			}
			else
			{
				// Else I don't care about right events
				return MSG_IGNORED;
			}

			break;

		}  // end right up or right click

		// ------------------------------------------------------------------------
		case GWM_CHAR:
		{
			switch( mData1 )
			{
				// --------------------------------------------------------------------
				case KEY_ENTER:
				case KEY_SPACE:
				{

					if( BitIsSet( mData2, KEY_STATE_UP ) )
					{

						//
						// note check like selected messages aren't sent here ... they are sent
						// on the down press
						//
						if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) &&
								BitIsSet( window->winGetStatus(), WIN_STATUS_CHECK_LIKE ) == FALSE )
						{

							TheWindowManager->winSendSystemMsg( instData->getOwner(), GBM_SELECTED,
																									(WindowMsgData)window, 0 );

							BitClear( instData->m_state, WIN_STATE_SELECTED );

						}

					} 
					else
					{

						//
						// for 'check-like' buttons we have "dual state", we flip the selected status
						// in that case instead of just turning it on like normal ... also note
						// that selected messages are sent immediately
						//
						if( BitIsSet( window->winGetStatus(), WIN_STATUS_CHECK_LIKE ) )
						{
							
							if( BitIsSet( instData->m_state, WIN_STATE_SELECTED ) )
								BitClear( instData->m_state, WIN_STATE_SELECTED );
							else
								BitSet( instData->m_state, WIN_STATE_SELECTED );

							TheWindowManager->winSendSystemMsg( instData->getOwner(), GBM_SELECTED,
																									(WindowMsgData)window, mData1 );

						}  // end if
						else
						{
							
							// just select as normal
							BitSet( instData->m_state, WIN_STATE_SELECTED );

						}  // end else


					}  // end else

					break;

				}  // end handle enter and space button

				// --------------------------------------------------------------------
				case KEY_DOWN:
				case KEY_RIGHT:
				case KEY_TAB:
				{

					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
						TheWindowManager->winNextTab(window);
					break;
				
				}  // end key down, right or tab

				// --------------------------------------------------------------------
				case KEY_UP:
				case KEY_LEFT:
				{

					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
						TheWindowManager->winPrevTab(window);
					break;

				}  // end key up or left

				// --------------------------------------------------------------------
				default:
					return MSG_IGNORED;

			}  // end switch on char

			break;

		}  // end character message

		// ------------------------------------------------------------------------
		default:
			return MSG_IGNORED;

	}  // end switch( msg )

	return MSG_HANDLED;

}  // end GadgetPushButtonInput

// GadgetPushButtonSystem =====================================================
/** Handle system messages for push button */
//=============================================================================
WindowMsgHandledType GadgetPushButtonSystem( GameWindow *window, UnsignedInt msg,
														 WindowMsgData mData1, WindowMsgData mData2 )
{
	WinInstanceData *instData = window->winGetInstanceData();

	switch( msg ) 
	{
		
		// ------------------------------------------------------------------------
		case GGM_SET_LABEL:
		{
			// set text into the win instance text data field
			window->winSetText( *(UnicodeString*)mData1 );
			break;
		}

		// ------------------------------------------------------------------------
		case GWM_CREATE:
			break;

		// ------------------------------------------------------------------------
		case GWM_DESTROY:
		{
			delete (PushButtonData *)window->winGetUserData();
			window->winSetUserData(NULL);
			break;
		}

		// ------------------------------------------------------------------------
		case GWM_INPUT_FOCUS:

			if( mData1 == FALSE )
				BitClear( instData->m_state, WIN_STATE_HILITED );
			else
				BitSet( instData->m_state, WIN_STATE_HILITED );

			TheWindowManager->winSendSystemMsg( instData->getOwner(), 
																					GGM_FOCUS_CHANGE,
																					(WindowMsgData)mData1, 
																					window->winGetWindowId() );
			if( mData1 == FALSE )
				*(Bool*)mData2 = FALSE;
			else
				*(Bool*)mData2 = TRUE;
			break;

		default:
			return MSG_IGNORED;

	}  // end switch( msg )

	return MSG_HANDLED;

}  // end GadgetPushButtonSystem

// ------------------------------------------------------------------------------------------------
/** Set the visual status of a button to make it looked checked/unchecked ... DO NOT send
	* any actual button selected messages, this is ONLY VISUAL */
// ------------------------------------------------------------------------------------------------
void GadgetCheckLikeButtonSetVisualCheck( GameWindow *g, Bool checked )
{

	// sanity
	if( g == NULL )
		return;

	// get instance data
	WinInstanceData *instData = g->winGetInstanceData();	
	if( instData == NULL )
		return;

	// sanity, must be a check like button
	if( BitIsSet( g->winGetStatus(), WIN_STATUS_CHECK_LIKE ) == FALSE )
	{

		DEBUG_CRASH(( "GadgetCheckLikeButtonSetVisualCheck: Window is not 'CHECK-LIKE'\n" ));
		return;

	}  // end if

	// set or clear the 'pushed' state
	if( instData )
	{

		if( checked == TRUE )
			BitSet( instData->m_state, WIN_STATE_SELECTED );
		else
			BitClear( instData->m_state, WIN_STATE_SELECTED );

	}  // end if

}  // end GadgetCheckLikeButtonSetVisualCheck

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
Bool GadgetCheckLikeButtonIsChecked( GameWindow *g )
{

	// sanity
	if( g == NULL )
		return FALSE;

	// get instance data
	WinInstanceData *instData = g->winGetInstanceData();
	if( instData == NULL )
		return FALSE;

	// we just hold this "check like dual state thingie" using the selected state
	return BitIsSet( instData->m_state, WIN_STATE_SELECTED );

}  // end GadgetCheckLikeButtonIsChecked

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
void GadgetButtonEnableCheckLike( GameWindow *g, Bool makeCheckLike, Bool initiallyChecked )
{

	// sanity
	if( g == NULL )
		return;

	// get inst data
	WinInstanceData *instData = g->winGetInstanceData();
	if( instData == NULL )
		return;

	// make it check like
	if( makeCheckLike )
		g->winSetStatus( WIN_STATUS_CHECK_LIKE );
	else
		g->winClearStatus( WIN_STATUS_CHECK_LIKE );

	// set the initially checked "state"
	if( initiallyChecked )
		BitSet( instData->m_state, WIN_STATE_SELECTED );
	else
		BitClear( instData->m_state, WIN_STATE_SELECTED );

}  // end GadgetButtonEnableCheckLike

// GadgetButtonSetText ========================================================
/** Set the text for a push button */
//=============================================================================
void GadgetButtonSetText( GameWindow *g, UnicodeString text )
{

	// sanity
	if( g == NULL )
		return;

	TheWindowManager->winSendSystemMsg( g, GGM_SET_LABEL, (WindowMsgData)&text, 0 );

}  // end GadgetButtonSetText

PushButtonData * getNewPushButtonData( void )
{
	PushButtonData *p = NEW PushButtonData;
	if(!p)
		return NULL;
	
	p->userData = NULL;
	p->drawBorder = FALSE;
	p->drawClock = NO_CLOCK;
	p->overlayImage = NULL;
	return p;
}

// GadgetButtonSetBorder ======================================================
/** Set to draw the special borders in the game */
//=============================================================================
void GadgetButtonSetBorder( GameWindow *g, Color color, Bool drawBorder = TRUE )
{
	if( g == NULL )
		return;

	PushButtonData *pData = (PushButtonData *)g->winGetUserData();
	if(!pData)
	{
		pData = getNewPushButtonData();
	}
	pData->drawBorder = drawBorder;
	pData->colorBorder = color;
	g->winSetUserData(pData);
}

// GadgetButtonDrawClock ======================================================
/** Set to draw a rectClock on the button */
//=============================================================================
void GadgetButtonDrawClock( GameWindow *g, Int percent, Color color )
{

	if( g == NULL )
		return;

	PushButtonData *pData = (PushButtonData *)g->winGetUserData();
	if(!pData)
	{
		pData = getNewPushButtonData();
	}
	pData->drawClock = NORMAL_CLOCK;
	pData->percentClock = percent;
	pData->colorClock = color;
	g->winSetUserData(pData);

}

// GadgetButtonDrawInverseClock ======================================================
/** Set to draw an inversed rectClock on the button */
//=============================================================================
void GadgetButtonDrawInverseClock( GameWindow *g, Int percent, Color color )
{

	if( g == NULL )
		return;

	PushButtonData *pData = (PushButtonData *)g->winGetUserData();
	if(!pData)
	{
		pData = getNewPushButtonData();
	}
	pData->drawClock = INVERSE_CLOCK;
	pData->percentClock = percent;
	pData->colorClock = color;
	g->winSetUserData(pData);

}

void GadgetButtonDrawOverlayImage( GameWindow *g, const Image *image )
{
	if( g == NULL )
		return;

	PushButtonData *pData = (PushButtonData *)g->winGetUserData();
	if(!pData)
	{
		pData = getNewPushButtonData();
	}
	pData->overlayImage = image;
	g->winSetUserData(pData);
}


// GadgetButtonSetData ======================================================
/** Sets random data that the user can contain on the button */
//=============================================================================
void GadgetButtonSetData(GameWindow *g, void *data)
{
	if( g == NULL )
		return;

	PushButtonData *pData = (PushButtonData *)g->winGetUserData();
	if(!pData)
	{
		pData = getNewPushButtonData();	
	}
	pData->userData = data;
	g->winSetUserData(pData);
}

// GadgetButtonGetData ======================================================
/** Gets the random data the user had already set on the button */
//=============================================================================
void *GadgetButtonGetData(GameWindow *g)
{
	if( g == NULL )
		return NULL;

	PushButtonData *pData = (PushButtonData *)g->winGetUserData();
	if(!pData)
	{
		return NULL;
	}
	return pData->userData;
}

void GadgetButtonSetAltSound(GameWindow *g, AsciiString altSound )
{
	if(!g)
		return;
	PushButtonData *pData = (PushButtonData *)g->winGetUserData();
	if(!pData)
	{
		return;
	}
	pData->altSound = altSound;

}

