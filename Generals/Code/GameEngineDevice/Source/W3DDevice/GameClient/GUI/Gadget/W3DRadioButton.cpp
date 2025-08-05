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

// FILE: W3DRadioButton.cpp ///////////////////////////////////////////////////
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
// File name: W3DRadioButton.cpp
//
// Created:   Colin Day, June 2001
//
// Desc:      W3D methods needed to implement the RadioButton UI control
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdlib.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/GameWindowGlobal.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetRadioButton.h"
#include "W3DDevice/GameClient/W3DGadget.h"
#include "W3DDevice/GameClient/W3DDisplay.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

// drawRadioButtonText ========================================================
/** Draw the text for a RadioButton */
//=============================================================================
static void drawRadioButtonText( GameWindow *window, WinInstanceData *instData )
{
	ICoord2D origin, size, textPos;
	Int width, height;
	Color textColor, dropColor;
	DisplayString *text = instData->getTextDisplayString();

	// sanity
	if( text == NULL || text->getTextLength() == 0 )
		return;

	// get window position and size
	window->winGetScreenPosition( &origin.x, &origin.y );
	window->winGetSize( &size.x, &size.y );

	// get the right text color
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) == FALSE )
	{
		textColor = window->winGetDisabledTextColor();
		dropColor = window->winGetDisabledTextBorderColor();
	}  // end if, disabled
	else if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
	{
		textColor = window->winGetHiliteTextColor();
		dropColor = window->winGetHiliteTextBorderColor();
	}  // end else if, hilited
	else
	{
		textColor = window->winGetEnabledTextColor();
		dropColor = window->winGetEnabledTextBorderColor();
	}  // end enabled only

	// set our font to that of our parent if not the same
	if( text->getFont() != window->winGetFont() )
		text->setFont( window->winGetFont() );

	// get text size
	text->getSize( &width, &height );

	// set the location for our text
	textPos.x = origin.x + (size.x / 2) - (width / 2);
	textPos.y = origin.y + (size.y / 2) - (height / 2);

	// draw it
	text->draw( textPos.x, textPos.y, textColor, dropColor );

}  // end drawRadioButtonText

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// W3DGadgetRadioButtonDraw ===================================================
/** Draw colored check box using standard graphics */
//=============================================================================
void W3DGadgetRadioButtonDraw( GameWindow *window, WinInstanceData *instData )
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
	// get the colors we should be using to draw, see GadgetRadioButton.h
	// draw appropriate state, see GadgetRadioButton.h for info
	//
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) == FALSE )
	{

		// disabled background
		backColor			= GadgetRadioGetDisabledColor( window );
		backBorder		= GadgetRadioGetDisabledBorderColor( window );

		// check box
		if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) )
		{
				boxColor		= GadgetRadioGetDisabledCheckedBoxColor( window );
				boxBorder		= GadgetRadioGetDisabledCheckedBoxBorderColor( window );
		}
		else
		{
				boxColor		= GadgetRadioGetDisabledUncheckedBoxColor( window );
				boxBorder		= GadgetRadioGetDisabledUncheckedBoxBorderColor( window );
		}

	}  // end if
	else if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
	{

		// hilited background
		backColor			= GadgetRadioGetHiliteColor( window );
		backBorder		= GadgetRadioGetHiliteBorderColor( window );

		// check box
		if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) )
		{
			boxColor		= GadgetRadioGetHiliteCheckedBoxColor( window );
			boxBorder		= GadgetRadioGetHiliteCheckedBoxBorderColor( window );
		}
		else
		{
			boxColor		= GadgetRadioGetHiliteUncheckedBoxColor( window );
			boxBorder		= GadgetRadioGetHiliteUncheckedBoxBorderColor( window );
		}

	}  // end else if
	else
	{

		// enabled background
		backColor			= GadgetRadioGetEnabledColor( window );
		backBorder		= GadgetRadioGetEnabledBorderColor( window );

		// check box
		if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) )
		{
			boxColor		= GadgetRadioGetEnabledCheckedBoxColor( window );
			boxBorder		= GadgetRadioGetEnabledCheckedBoxBorderColor( window );
		}
		else
		{
			boxColor		= GadgetRadioGetEnabledUncheckedBoxColor( window );
			boxBorder		= GadgetRadioGetEnabledUncheckedBoxBorderColor( window );
		}

	}  // end else

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
	start.x = origin.x + size.y;
	start.y = origin.y;
	end.x = start.x;
	end.y = start.y + size.y;
	TheWindowManager->winDrawLine( backBorder, WIN_DRAW_LINE_WIDTH,
																 start.x, start.y, end.x, end.y );

	// draw box for button
	start.x = origin.x + 1;
	start.y = origin.y + 1;
	end.x	= origin.x + size.y -1;
	end.y = origin.y + size.y -1;
	TheWindowManager->winFillRect( boxColor, WIN_DRAW_LINE_WIDTH,
																 start.x, start.y, end.x, end.y );

	// draw box border
	start.x = origin.x + size.x - size.y;
	start.y = origin.y;
	end.x = start.x;
	end.y = start.y + size.y;
	TheWindowManager->winDrawLine( backBorder, WIN_DRAW_LINE_WIDTH,
																 start.x, start.y, end.x, end.y );

	// draw box for button
	start.x = origin.x + size.x - size.y;
	start.y = origin.y + 1;
	end.x	= origin.x + size.x -1;
	end.y = origin.y + size.y -1;
	TheWindowManager->winFillRect( boxColor, WIN_DRAW_LINE_WIDTH,
																 start.x, start.y, end.x, end.y );
	// draw the button text
	if( instData->getTextLength() )
		drawRadioButtonText( window, instData );



}  // end W3DGadgetRadioButtonDraw


void W3DGadgetRadioButtonImageDraw( GameWindow *window,
																	WinInstanceData *instData )
{
	const Image *leftImage, *rightImage, *centerImage;
	ICoord2D origin, size, start, end;
	Int xOffset, yOffset;
	Int i;

	// get screen position and size
	window->winGetScreenPosition( &origin.x, &origin.y );
	window->winGetSize( &size.x, &size.y );

	IRegion2D clipLeft;

	// get image offset
	xOffset = instData->m_imageOffset.x;
	yOffset = instData->m_imageOffset.y;

	if( BitIsSet( instData->getState(), WIN_STATE_SELECTED ) )
	{
		//backgroundImage	= GadgetRadioGetEnabledCheckedBoxImage( window );
		leftImage					= GadgetRadioGetSelectedImage( window );
		centerImage				= GadgetRadioGetSelectedUncheckedBoxImage( window );
		rightImage				= GadgetRadioGetSelectedCheckedBoxImage( window );

	}
	else if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) == FALSE )
	{
		// disabled background
		leftImage					= GadgetRadioGetDisabledImage( window );
		centerImage				= GadgetRadioGetDisabledUncheckedBoxImage( window );
		rightImage				= GadgetRadioGetDisabledCheckedBoxImage( window );

	}  // end if
	else if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
	{
		// hilited background
		leftImage					= GadgetRadioGetHiliteImage( window );
		centerImage				= GadgetRadioGetHiliteUncheckedBoxImage( window );
		rightImage				= GadgetRadioGetHiliteCheckedBoxImage( window );

	}  // end else if
	else
	{
		// enabled background
		leftImage					= GadgetRadioGetEnabledImage( window );
		centerImage				= GadgetRadioGetEnabledUncheckedBoxImage( window );
		rightImage				= GadgetRadioGetEnabledCheckedBoxImage( window );

	}  // end else

	// sanity, we need to have these images to make it look right
	if( leftImage == NULL || centerImage == NULL ||
			rightImage == NULL )
		return;

	// get image sizes for the ends
	ICoord2D leftSize, rightSize;
	leftSize.x = leftImage->getImageWidth();
	leftSize.y = leftImage->getImageHeight();
	rightSize.x = rightImage->getImageWidth();
	rightSize.y = rightImage->getImageHeight();

	// get two key points used in the end drawing
	ICoord2D leftEnd, rightStart;
	leftEnd.x = origin.x + leftSize.x + xOffset;
	leftEnd.y = origin.y + size.y + yOffset;
	rightStart.x = origin.x + size.x - rightSize.x + xOffset;
	rightStart.y = origin.y  + size.y + yOffset;



	// draw the center repeating bar
	Int centerWidth, pieces;

	// get width we have to draw our repeating center in
	centerWidth = rightStart.x - leftEnd.x;

	// how many whole repeating pieces will fit in that width
	pieces = centerWidth / centerImage->getImageWidth();
	pieces++;
	// draw the pieces
	start.x = leftEnd.x;
	start.y = origin.y + yOffset;
	end.y =origin.y + size.y + yOffset;

	clipLeft.lo.x = leftEnd.x;
	clipLeft.lo.y = origin.y;
	clipLeft.hi.y = leftEnd.y;
	clipLeft.hi.x = rightStart.x	;


	TheDisplay->setClipRegion(&clipLeft);

	for( i = 0; i < pieces; i++ )
	{
		end.x = start.x + centerImage->getImageWidth();
		TheWindowManager->winDrawImage( centerImage,
																		start.x, start.y,
																		end.x, end.y );
		start.x += centerImage->getImageWidth();
	}  // end for i

	TheDisplay->enableClipping(FALSE);
	// draw left end
	start.x = origin.x + xOffset;
	start.y = origin.y + yOffset;
	end = leftEnd;
	TheWindowManager->winDrawImage(leftImage, start.x, start.y, end.x, end.y);
	// draw right end
	start.x = rightStart.x;
	start.y = origin.y + yOffset;
	end.x = origin.x + size.x;
	end.y = leftEnd.y;
	TheWindowManager->winDrawImage(rightImage, start.x, start.y, end.x, end.y);

	// draw the text
	if( instData->getTextLength() )
		drawRadioButtonText( window, instData );


}  // end W3DGadgetHorizontalSliderImageDraw

