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

// FILE: VerticalSlider.cpp ///////////////////////////////////////////////////
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
// File name: VerticalSlider.cpp
//
// Created:   Colin Day, June 2001
//
// Desc:      Vertical slider gui control
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

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef RTS_INTERNAL
// for occasional debugging...
//#pragma optimize("", off)
//#pragma MESSAGE("************************************** WARNING, optimization disabled for debugging purposes")
#endif

// GadgetVerticlaSliderInput ==================================================
/** Handle input for vertical slider */
//=============================================================================
WindowMsgHandledType GadgetVerticalSliderInput( GameWindow *window, UnsignedInt msg,
																WindowMsgData mData1, WindowMsgData mData2 )
{
	SliderData *s = (SliderData *)window->winGetUserData();
	WinInstanceData *instData = window->winGetInstanceData();

	switch( msg ) 
	{

		// ------------------------------------------------------------------------
		case GWM_MOUSE_ENTERING:

			if( BitIsSet( instData->getStyle(), GWS_MOUSE_TRACK ) ) 
			{

				BitSet( instData->m_state, WIN_STATE_HILITED );
				TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																						GBM_MOUSE_ENTERING,
																						(WindowMsgData)window, 
																						0 );
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
																						(WindowMsgData)window, 
																						0 );
			}
			break;

		// ------------------------------------------------------------------------
		case GWM_LEFT_DRAG:

			if( BitIsSet( instData->getStyle(), GWS_MOUSE_TRACK ) )
				TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																						GGM_LEFT_DRAG,
																						(WindowMsgData)window, 
																						mData1 );
			break;

		// ------------------------------------------------------------------------
		case GWM_LEFT_DOWN:
			break;

		// ------------------------------------------------------------------------
		case GWM_LEFT_UP:
		{
			Int x, y;
//			Int mousex = mData1 & 0xFFFF;
			Int mousey = mData1 >> 16;
			ICoord2D size, childSize, childCenter;
			GameWindow *child = window->winGetChild();
			Int pageClickSize, clickPos;

			window->winGetScreenPosition( &x, &y );
			window->winGetSize( &size.x, &size.y );
			child->winGetSize( &childSize.x, &childSize.y );
			child->winGetPosition( &childCenter.x, &childCenter.y );
			childCenter.x += childSize.x / 2;
			childCenter.y += childSize.y / 2;

			//
			// when you click on the slider, but not the button, we will jump
			// the slider position up/down by this much
			//
			pageClickSize = size.y / 5;

			clickPos = mousey - y;
			if( clickPos >= childCenter.y )
			{

				clickPos = childCenter.y + pageClickSize;
				if( clickPos > mousey - y )
					clickPos = mousey - y;

			}  // end if
			else
			{

				clickPos = childCenter.y - pageClickSize;
				if( clickPos < mousey - y )
					clickPos = mousey - y;

			}  // end else

			// keep pos valid on window
			if( clickPos > y + size.y - childSize.y / 2 )
				clickPos = y + size.y - childSize.y / 2;
			if( clickPos < childSize.y / 2 )
				clickPos = childSize.y / 2;

			child->winSetPosition( 0, clickPos - childSize.y / 2 );
			TheWindowManager->winSendSystemMsg( window, GGM_LEFT_DRAG, 0, mData1 );
			break;

		}

		// ------------------------------------------------------------------------
		case GWM_CHAR:
		{

			switch (mData1)
			{

				// --------------------------------------------------------------------
				case KEY_UP:

					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
					{

						if( s->position < s->maxVal - 1)
						{
							GameWindow *child = window->winGetChild();

							s->position += 2;
							TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																									GSM_SLIDER_TRACK,
																									(WindowMsgData)window, 
																									s->position );
							// Translate to window coords
							child->winSetPosition( 0, (Int)((s->maxVal - s->position) * s->numTicks) );

						}

					}

					break;

				// --------------------------------------------------------------------
				case KEY_DOWN:

					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
					{

						if( s->position > s->minVal + 1 ) 
						{
							GameWindow *child = window->winGetChild();

							s->position -= 2;
							TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																									GSM_SLIDER_TRACK,
																									(WindowMsgData)window, 
																									s->position );
							// Translate to window coords
							child->winSetPosition( 0, (Int)((s->maxVal - s->position) * s->numTicks) );
						}
					}
					break;

				// --------------------------------------------------------------------
				case KEY_RIGHT:
				case KEY_TAB:

					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
						window->winNextTab();
					break;

				// --------------------------------------------------------------------
				case KEY_LEFT:

					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
						window->winPrevTab();
					break;

				default:
					return MSG_IGNORED;

			}  // end switch( mData1 )

			break;

		}  // end char

		default:
			return MSG_IGNORED;

	}  // end switch( msg )

	return MSG_HANDLED;

}  // end GadgetVerticalSliderInput

// GadgetVerticalSliderSystem =================================================
/** Handle system messages for vertical slider */
//=============================================================================
WindowMsgHandledType GadgetVerticalSliderSystem( GameWindow *window, UnsignedInt msg,
																 WindowMsgData mData1, WindowMsgData mData2 )
{
	SliderData *s = (SliderData *)window->winGetUserData();
	WinInstanceData *instData = window->winGetInstanceData();

	switch( msg ) 
	{

		// ------------------------------------------------------------------------
		case GBM_SELECTED:
		{
			// tell owner I've finished moving
			TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																					GSM_SLIDER_DONE,
																					(WindowMsgData)window, 
																					s->position );
			break;

		}

		// ------------------------------------------------------------------------
		case GGM_LEFT_DRAG:
		{
//			Int mousex = mData2 & 0xFFFF;
			Int mousey = mData2 >> 16;
			Int x, y, delta;
			ICoord2D size, childSize, childCenter;
			GameWindow *child = window->winGetChild();

			window->winGetScreenPosition( &x, &y );
			window->winGetSize( &size.x, &size.y );
			child->winGetSize( &childSize.x, &childSize.y );
			child->winGetScreenPosition( &childCenter.x, &childCenter.y );
			childCenter.x += childSize.x / 2;
			childCenter.y += childSize.y / 2;

			//
			// ignore drag attempts when the mouse is below or above the slider totally
			// and put the dragging thumb back at the slider pos
			//
			if( mousey > y + size.y )
			{

				//s->position = s->minVal;
				TheWindowManager->winSendSystemMsg( window, GSM_SET_SLIDER, 
																						s->minVal, 0 );
				// tell owner i moved
				TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																						GSM_SLIDER_TRACK,
																						(WindowMsgData)window, 
																						s->position );
				break;
			
			}  // end if
			else if( mousey < y )
			{

				//s->position = s->maxVal;
				TheWindowManager->winSendSystemMsg( window, GSM_SET_SLIDER, 
																						s->maxVal, 0 );
				// tell owner i moved
				TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																						GSM_SLIDER_TRACK,
																						(WindowMsgData)window, 
																						s->position );
				break;

			}  // end else if

			if( childCenter.y <= y + childSize.y / 2 )
			{
				child->winSetPosition( 0, 0 );
				s->position = s->maxVal;
			}
			else if( childCenter.y >= y + size.y - childSize.y / 2 )
			{
				child->winSetPosition( 0, size.y - childSize.y );
				s->position = s->minVal;
			}
			else
			{
				delta = childCenter.y - y - childSize.y/2;

				// Calc slider position
				s->position = (Int)(delta / s->numTicks) ;

				/*
				s->position += s->minVal;
				*/

				if( s->position > s->maxVal )
					s->position = s->maxVal;

				// Invert slider position so that maxval is at the top
				s->position = s->maxVal - s->position;
				
			}

			// tell owner i moved
			TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																					GSM_SLIDER_TRACK,
																					(WindowMsgData)window, 
																					s->position );
			break;

		}

		// ------------------------------------------------------------------------
		case GSM_SET_SLIDER:
		{
			Int newPos = (Int)mData1;
			GameWindow *child = window->winGetChild();

			if (newPos < s->minVal || newPos > s->maxVal)
				break;

			s->position = newPos;

			// Translate to window coords
			newPos = (Int)((s->maxVal - newPos) * s->numTicks);

			child->winSetPosition( 0, newPos );

			break;

		}

		// ------------------------------------------------------------------------
		case GSM_SET_MIN_MAX:
		{
			Int newPos;
			ICoord2D size;
			GameWindow *child = window->winGetChild();

			window->winGetSize( &size.x, &size.y );

			s->minVal = (Int)mData1;
			s->maxVal = (Int)mData2;
			s->numTicks = (Real)( size.y-GADGET_SIZE)/(Real)(s->maxVal - s->minVal);
			s->position = s->minVal;

			// Translate to window coords
			newPos = (Int)((s->maxVal - s->minVal) * s->numTicks);

			child->winSetPosition( 0, newPos );
			break;

		}

		// ------------------------------------------------------------------------
		case GWM_CREATE:
			break;

		// ------------------------------------------------------------------------
		case GWM_DESTROY:
			delete( (SliderData *)window->winGetUserData() );
			window->winSetUserData(NULL);
			s = NULL;
			break;

		// ------------------------------------------------------------------------
		case GWM_INPUT_FOCUS:

			// If we're losing focus
			if( mData1 == FALSE ) 
			{
				BitClear( instData->m_state, WIN_STATE_HILITED );
			} else {
				BitSet( instData->m_state, WIN_STATE_HILITED );
			}

			TheWindowManager->winSendSystemMsg( window->winGetOwner(), 
																					GGM_FOCUS_CHANGE,
																					mData1, 
																					window->winGetWindowId() );

			*(Bool*)mData2 = TRUE;
			break;

		// ------------------------------------------------------------------------
		case GGM_RESIZED:
		{
			Int width = (Int)mData1;
//			Int height = (Int)mData2;
			GameWindow *thumb = window->winGetChild();

			if( thumb )
				thumb->winSetSize( width, GADGET_SIZE );

			break;

		}  // end resized

		default:
			return MSG_IGNORED;

	}  // end switch( msg )

	return MSG_HANDLED;

}  // end GadgetVerticalSliderSystem

