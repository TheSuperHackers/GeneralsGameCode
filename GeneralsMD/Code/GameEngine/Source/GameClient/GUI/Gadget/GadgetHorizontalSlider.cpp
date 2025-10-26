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

// FILE: HorizontalSlider.cpp /////////////////////////////////////////////////
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
// File name: HorizontalSlider.cpp
//
// Created:   Colin Day, June 2001
//
// Desc:      Horizontal GUI slider
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/Language.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/Gadget.h"
#include "GameClient/GadgetSlider.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// GadgetHorizontalSliderInput ================================================
/** Handle input for horizontal slider */
//=============================================================================
WindowMsgHandledType GadgetHorizontalSliderInput( GameWindow *window, UnsignedInt msg,
																	WindowMsgData mData1, WindowMsgData mData2 )
{
	SliderData *s = (SliderData *)window->winGetUserData();
	WinInstanceData *instData = window->winGetInstanceData();
	ICoord2D size, childSize, childCenter;
	window->winGetSize( &size.x, &size.y );
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
																						0 );
				//TheWindowManager->winSetFocus( window );

			}

			if(window->winGetChild() && BitIsSet(window->winGetChild()->winGetStyle(),GWS_PUSH_BUTTON) )
			{
				WinInstanceData *instDataChild = window->winGetChild()->winGetInstanceData();
				BitSet(instDataChild->m_state, WIN_STATE_HILITED);
			}

			break;

		}

		// ------------------------------------------------------------------------
		case GWM_MOUSE_LEAVING:
		{

			if( BitIsSet( instData->getStyle(), GWS_MOUSE_TRACK ))
			{

				BitClear( instData->m_state, WIN_STATE_HILITED );
				TheWindowManager->winSendSystemMsg( window->winGetOwner(),
																						GBM_MOUSE_LEAVING,
																						(WindowMsgData)window,
																						0 );
			}
			if(window->winGetChild() && BitIsSet(window->winGetChild()->winGetStyle(),GWS_PUSH_BUTTON) )
			{
				WinInstanceData *instDataChild = window->winGetChild()->winGetInstanceData();
				BitClear(instDataChild->m_state, WIN_STATE_HILITED);
			}

			break;

		}

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
			Int mousex = mData1 & 0xFFFF;
//			Int mousey = mData1 >> 16;

			GameWindow *child = window->winGetChild();
			Int pageClickSize, clickPos;

			window->winGetScreenPosition( &x, &y );

			child->winGetSize( &childSize.x, &childSize.y );
			child->winGetPosition( &childCenter.x, &childCenter.y );
			childCenter.x += childSize.x / 2;
			childCenter.y += childSize.y / 2;

			//
			// when you click on the slider, but not the button, we will jump
			// the slider position up/down by this much
			//
			pageClickSize = size.x / 5;

			clickPos = mousex - x;
			if( clickPos >= childCenter.x )
			{

				clickPos = childCenter.x + pageClickSize;
				if( clickPos > mousex - x )
					clickPos = mousex - x;

			}
			else
			{

				clickPos = childCenter.x - pageClickSize;
				if( clickPos < mousex - x )
					clickPos = mousex - x;

			}

			// keep it all valid to the window
			if( clickPos > x + size.x - childSize.x / 2 )
				clickPos = x + size.y - childSize.x / 2;
			if( clickPos < childSize.x / 2 )
				clickPos = childSize.x / 2;

			child->winSetPosition( clickPos - childSize.x / 2, HORIZONTAL_SLIDER_THUMB_POSITION);
			TheWindowManager->winSendSystemMsg( window, GGM_LEFT_DRAG, 0, mData1 );
			break;

		}

		// ------------------------------------------------------------------------
		case GWM_CHAR:
		{

			switch( mData1 )
			{

				// --------------------------------------------------------------------
				case KEY_RIGHT:
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
							child->winSetPosition( (Int)((s->position - s->minVal) * s->numTicks), HORIZONTAL_SLIDER_THUMB_POSITION );

						}

					}

					break;

				// --------------------------------------------------------------------
				case KEY_LEFT:

					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
					{

						if( s->position < s->maxVal - 1 )
						{
							GameWindow *child = window->winGetChild();

							s->position += 2;
							TheWindowManager->winSendSystemMsg( window->winGetOwner(),
																									GSM_SLIDER_TRACK,
																									(WindowMsgData)window,
																									s->position );

							// Translate to window coords
							child->winSetPosition( (Int)((s->position - s->minVal) * s->numTicks),HORIZONTAL_SLIDER_THUMB_POSITION );

						}

					}

					break;

				// --------------------------------------------------------------------
				case KEY_DOWN:
				case KEY_TAB:

					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
						window->winNextTab();
					break;

				// --------------------------------------------------------------------
				case KEY_UP:

					if( BitIsSet( mData2, KEY_STATE_DOWN ) )
						window->winPrevTab();
					break;

				// --------------------------------------------------------------------
				default:
					return MSG_IGNORED;

			}

			break;

		}

		// ------------------------------------------------------------------------
		default:
			return MSG_IGNORED;

	}

	return MSG_HANDLED;

}

// GadgetHorizontalSliderSystem ===============================================
/** Handle system messages for horizontal slider */
//=============================================================================
WindowMsgHandledType GadgetHorizontalSliderSystem( GameWindow *window, UnsignedInt msg,
																	 WindowMsgData mData1, WindowMsgData mData2 )
{
	SliderData *s = (SliderData *)window->winGetUserData();
	WinInstanceData *instData = window->winGetInstanceData();
	ICoord2D size, childSize, childCenter,childRelativePos;
	window->winGetSize( &size.x, &size.y );
	switch( msg )
	{
		// ------------------------------------------------------------------------
		case GGM_LEFT_DRAG:
		{
			Int mousex = mData2 & 0xFFFF;
//			Int mousey = mData2 >> 16;
			Int x, y, delta;
			GameWindow *child = window->winGetChild();


			window->winGetScreenPosition( &x, &y );

			child->winGetSize( &childSize.x, &childSize.y );
			child->winGetScreenPosition( &childCenter.x, &childCenter.y );
			child->winGetPosition(&childRelativePos.x, &childRelativePos.y);
			childCenter.x += childSize.x / 2;
			childCenter.y += childSize.y / 2;

			//
			// ignore drag attempts when the mouse is right or left of slider totally
			// and put the dragging thumb back at the slider pos
			//
			if( mousex > x + size.x -HORIZONTAL_SLIDER_THUMB_WIDTH/2 )
			{

				TheWindowManager->winSendSystemMsg( window, GSM_SET_SLIDER,
																						s->maxVal, 0 );
				// tell owner i moved
				TheWindowManager->winSendSystemMsg( window->winGetOwner(),
																						GSM_SLIDER_TRACK,
																						(WindowMsgData)window,
																						s->position );
				break;

			}
			else if( mousex < x + HORIZONTAL_SLIDER_THUMB_WIDTH/2)
			{

				TheWindowManager->winSendSystemMsg( window, GSM_SET_SLIDER,
																						s->minVal, 0 );
				// tell owner i moved
				TheWindowManager->winSendSystemMsg( window->winGetOwner(),
																						GSM_SLIDER_TRACK,
																						(WindowMsgData)window,
																						s->position );
				break;

			}

			if( childCenter.x < x + childSize.x / 2 )
			{
				child->winSetPosition( 0,HORIZONTAL_SLIDER_THUMB_POSITION );
				s->position = s->minVal;

			}
			else if( childCenter.x >= x + size.x - childSize.x / 2 )
			{
				child->winSetPosition( (Int)((s->maxVal - s->minVal) * s->numTicks) -HORIZONTAL_SLIDER_THUMB_WIDTH/2 , HORIZONTAL_SLIDER_THUMB_POSITION );
				s->position = s->maxVal;

			}
			else
			{
				delta = childCenter.x - x -HORIZONTAL_SLIDER_THUMB_WIDTH/2;

				// Calc slider position
				s->position = (Int)((delta) / s->numTicks)+ s->minVal ;

				/*
				s->position += s->minVal;
				*/

				if( s->position > s->maxVal )
					s->position = s->maxVal;
				if( s->position < s->minVal)
					s->position = s->minVal;

				child->winSetPosition( childRelativePos.x, HORIZONTAL_SLIDER_THUMB_POSITION );
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

			if( newPos < s->minVal || newPos > s->maxVal )
				break;

			s->position = newPos;

			// Translate to window coords
			newPos = (Int)((newPos - s->minVal) * s->numTicks);

			child->winSetPosition( newPos , HORIZONTAL_SLIDER_THUMB_POSITION );
			break;

		}

		// ------------------------------------------------------------------------
		case GSM_SET_MIN_MAX:
		{
			ICoord2D size;
			GameWindow *child = window->winGetChild();

			window->winGetSize( &size.x, &size.y );

			s->minVal = (Int)mData1;
			s->maxVal = (Int)mData2;
			s->numTicks = (Real)(size.x - HORIZONTAL_SLIDER_THUMB_WIDTH)/(Real)(s->maxVal - s->minVal);
			s->position = s->minVal;

			child->winSetPosition( 0, HORIZONTAL_SLIDER_THUMB_POSITION );
			break;

		}

		// ------------------------------------------------------------------------
		case GWM_CREATE:
			break;

		// ------------------------------------------------------------------------
		case GWM_DESTROY:
			delete ( (SliderData *)window->winGetUserData() );
			window->winSetUserData(NULL);
			s = NULL;
			break;

		// ------------------------------------------------------------------------
		case GWM_INPUT_FOCUS:
		{

			// If we're losing focus
			if( mData1 == FALSE )
				BitClear( instData->m_state, WIN_STATE_HILITED );
			else
				BitSet( instData->m_state, WIN_STATE_HILITED );

			TheWindowManager->winSendSystemMsg( window->winGetOwner(),
																					GGM_FOCUS_CHANGE,
																					mData1,
																					window->winGetWindowId() );

			*(Bool*)mData2 = TRUE;
			break;

		}

		// ------------------------------------------------------------------------
		case GGM_RESIZED:
		{
//			Int width = (Int)mData1;
			Int height = (Int)mData2;
			GameWindow *thumb = window->winGetChild();

			if( thumb )
				thumb->winSetSize( GADGET_SIZE, height );

			break;

		}

		default:
			return MSG_IGNORED;

	}

	return MSG_HANDLED;

}
