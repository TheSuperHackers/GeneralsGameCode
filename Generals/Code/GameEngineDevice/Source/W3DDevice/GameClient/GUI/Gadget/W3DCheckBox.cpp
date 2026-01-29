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

// FILE: W3DCheckBox.cpp //////////////////////////////////////////////////////
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
// File name: W3DCheckBox.cpp
//
// Created:   Colin Day, June 2001
//
// Desc:      W3D methods needed to implement the checkbox UI control
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdlib.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/GadgetCheckBox.h"
#include "GameClient/GameWindowGlobal.h"
#include "GameClient/GameWindowManager.h"
#include "W3DDevice/GameClient/W3DGadget.h"
#include "W3DDevice/GameClient/W3DDisplay.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

// drawCheckBoxText ===========================================================
/** Draw the text for a checkbox */
//=============================================================================
static void drawCheckBoxText( GameWindow *window, WinInstanceData *instData )
{
	ICoord2D origin, size, textPos;
	Int width, height;
	Color textColor, dropColor;
	DisplayString *text = instData->getTextDisplayString();

	// sanity
	if( text == nullptr || text->getTextLength() == 0 )
		return;

	// get window position and size
	window->winGetScreenPosition( &origin.x, &origin.y );
	window->winGetSize( &size.x, &size.y );

	// get the right text color
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) == FALSE )
	{
		textColor = window->winGetDisabledTextColor();
		dropColor = window->winGetDisabledTextBorderColor();
	}
	else if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
	{
		textColor = window->winGetHiliteTextColor();
		dropColor = window->winGetHiliteTextBorderColor();
	}
	else
	{
		textColor = window->winGetEnabledTextColor();
		dropColor = window->winGetEnabledTextBorderColor();
	}

	// set our font to that of our parent if not the same
	if( text->getFont() != window->winGetFont() )
		text->setFont( window->winGetFont() );

	// get text size
	text->getSize( &width, &height );

	// where to draw
	textPos.x = origin.x + size.y;//(size.x / 2) - (width / 2);
	textPos.y = origin.y + (size.y / 2) - (height / 2);

	// draw it
	text->draw( textPos.x, textPos.y, textColor, dropColor );

}

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// W3DGadgetCheckBoxDraw ======================================================
/** Draw colored check box using standard graphics */
//=============================================================================
void W3DGadgetCheckBoxDraw( GameWindow *window, WinInstanceData *instData )
{
	Int checkOffsetFromLeft;
	Color backColor,
				backBorder,
				boxColor,
				boxBorder;
	ICoord2D origin, size, start, end;

	// get window position and size
	window->winGetScreenPosition( &origin.x, &origin.y );
	window->winGetSize( &size.x, &size.y );

	// compute start of check offset
	checkOffsetFromLeft = size.x / 16;

	//
	// get the colors we should be using to draw, see GadgetCheckBox.h
	// draw appropriate state, see GadgetCheckBox.h for info
	//
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) == FALSE )
	{

		// disabled background
		backColor			= GadgetCheckBoxGetDisabledColor( window );
		backBorder		= GadgetCheckBoxGetDisabledBorderColor( window );

		// check box
		if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) )
		{
				boxColor		= GadgetCheckBoxGetDisabledCheckedBoxColor( window );
				boxBorder		= GadgetCheckBoxGetDisabledCheckedBoxBorderColor( window );
		}
		else
		{
				boxColor		= GadgetCheckBoxGetDisabledUncheckedBoxColor( window );
				boxBorder		= GadgetCheckBoxGetDisabledUncheckedBoxBorderColor( window );
		}

	}
	else if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
	{

		// hilited background
		backColor			= GadgetCheckBoxGetHiliteColor( window );
		backBorder		= GadgetCheckBoxGetHiliteBorderColor( window );

		// check box
		if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) )
		{
			boxColor		= GadgetCheckBoxGetHiliteCheckedBoxColor( window );
			boxBorder		= GadgetCheckBoxGetHiliteCheckedBoxBorderColor( window );
		}
		else
		{
			boxColor		= GadgetCheckBoxGetHiliteUncheckedBoxColor( window );
			boxBorder		= GadgetCheckBoxGetHiliteUncheckedBoxBorderColor( window );
		}

	}
	else
	{

		// enabled background
		backColor			= GadgetCheckBoxGetEnabledColor( window );
		backBorder		= GadgetCheckBoxGetEnabledBorderColor( window );

		// check box
		if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) )
		{
			boxColor		= GadgetCheckBoxGetEnabledCheckedBoxColor( window );
			boxBorder		= GadgetCheckBoxGetEnabledCheckedBoxBorderColor( window );
		}
		else
		{
			boxColor		= GadgetCheckBoxGetEnabledUncheckedBoxColor( window );
			boxBorder		= GadgetCheckBoxGetEnabledUncheckedBoxBorderColor( window );
		}

	}

	// draw background border
	start.x = origin.x;
	start.y = origin.y;
	end.x = start.x + size.x;
	end.y = start.y + size.y;
	TheWindowManager->winOpenRect( backBorder, WIN_DRAW_LINE_WIDTH,
																 start.x, start.y, end.x, end.y );

	// draw the background
	start.x++;
	start.y++;
	end.x--;
	end.y--;
	TheWindowManager->winFillRect( backColor, WIN_DRAW_LINE_WIDTH,
																 start.x, start.y, end.x, end.y );

	// draw box border
	start.x = origin.x + checkOffsetFromLeft;
	start.y = origin.y + (size.y / 3);
	end.x = start.x + (size.y / 3);
	end.y = start.y + (size.y / 3);
	TheWindowManager->winOpenRect( boxBorder, WIN_DRAW_LINE_WIDTH,
																 start.x, start.y, end.x, end.y );

	// draw "x" for button
	if( boxColor != WIN_COLOR_UNDEFINED )
	{

		TheWindowManager->winDrawLine( boxColor, WIN_DRAW_LINE_WIDTH,
																	 start.x, start.y, end.x, end.y );
		TheWindowManager->winDrawLine( boxColor, WIN_DRAW_LINE_WIDTH,
																	 start.x, end.y, end.x, start.y );

	}

	// draw the button text
	if( instData->getTextLength() )
		drawCheckBoxText( window, instData );

}

// W3DGadgetCheckBoxImageDraw =================================================
/** Draw check box with user supplied images */
//=============================================================================
void W3DGadgetCheckBoxImageDraw( GameWindow *window, WinInstanceData *instData )
{
	Int checkOffsetFromLeft;
	const Image  *boxImage = nullptr;//*backgroundImage = nullptr,
	ICoord2D origin, start, end, size;

	// get window position and size
	window->winGetScreenPosition( &origin.x, &origin.y );
	window->winGetSize( &size.x, &size.y );

	// compute screen coords
	start.x = origin.x + instData->m_imageOffset.x;
	start.y = origin.y + instData->m_imageOffset.y;
	end.x = start.x + size.x;
	end.y = start.y + size.y;

	// compute start of check offset
	checkOffsetFromLeft =  0;

	//
	// get the colors we should be using to draw, see GadgetCheckBoxButton.h
	// draw appropriate state, see GadgetCheckBoxButton.h for info
	//
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) == FALSE )
	{

		// disabled background
//		backgroundImage	= GadgetCheckBoxGetDisabledImage( window );

		// check box
		if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) )
				boxImage		= GadgetCheckBoxGetDisabledCheckedBoxImage( window );
		else
				boxImage		= GadgetCheckBoxGetDisabledUncheckedBoxImage( window );

	}
	else if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
	{

		// hilited background
//		backgroundImage	= GadgetCheckBoxGetHiliteImage( window );

		// check box
		if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) )
			boxImage		= GadgetCheckBoxGetHiliteCheckedBoxImage( window );
		else
			boxImage		= GadgetCheckBoxGetHiliteUncheckedBoxImage( window );

	}
	else
	{

		// enabled background
//		backgroundImage	= GadgetCheckBoxGetEnabledImage( window );

		// check box
		if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) )
			boxImage		= GadgetCheckBoxGetEnabledCheckedBoxImage( window );
		else
			boxImage		= GadgetCheckBoxGetEnabledUncheckedBoxImage( window );

	}

	// draw background image
//	if( backgroundImage )
//		TheWindowManager->winDrawImage( backgroundImage, start.x, start.y,
//																		end.x, end.y );

	// draw the box image
	if( boxImage )
	{

		start.x = origin.x + instData->m_imageOffset.x + checkOffsetFromLeft;
		start.y = origin.y +  3;
		end.x = start.x + (size.y - 6);
		end.y = start.y + (size.y - 6);
		TheWindowManager->winDrawImage( boxImage, start.x, start.y,
																	  end.x, end.y );

	}

	// draw the text
	if( instData->getTextLength() )
		drawCheckBoxText( window, instData );

}
