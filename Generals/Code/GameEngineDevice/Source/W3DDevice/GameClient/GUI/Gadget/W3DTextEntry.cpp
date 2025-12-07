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

// FILE: W3DTextEntry.cpp /////////////////////////////////////////////////////
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
// File name: W3DTextEntry.cpp
//
// Created:   Colin Day, June 2001
//
// Desc:      W3D implementation for the text entry gadget
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdlib.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/GameWindowGlobal.h"
#include "GameClient/GadgetTextEntry.h"
#include "GameClient/IMEManager.h"
#include "W3DDevice/GameClient/W3DGadget.h"
#include "W3DDevice/GameClient/W3DDisplay.h"


// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// drawTextEntryText ==========================================================
//=============================================================================
static void drawTextEntryText( GameWindow *window, WinInstanceData *instData,
															 Color textColor, Color textDropColor,
															 Color compositeColor, Color compositeDropColor,
															 Int x, Int y, Int width, Int fontHeight )
{
	static Byte drawCnt = 0;
	EntryData *e = (EntryData *)window->winGetUserData();
//	Int charPos = e->charPos;
	Int cursorPos;

//	WideChar buffer[ ENTRY_TEXT_LEN + 1 ];
//	WideChar *bufptr = buffer;
//	Color constructColor = TheWindowManager->winMakeColor( 192, 0, 192, 255 );
	DisplayString *text = e->text;
	IRegion2D clipRegion;
	ICoord2D origin, size;
	Int compositeCursorPos = 0;

	// Check to see if the IME manager is composing text
	e->constructText->setText(UnicodeString::TheEmptyString);
	if ( TheIMEManager && TheIMEManager->isAttachedTo( window) && TheIMEManager->isComposing())
	{
		// The user is composing a string.
		// Show the composition in the text gadget.
		UnicodeString composition;

		TheIMEManager->getCompositionString( composition );

		if ( e->secretText )
		{
			e->sText->setText( UnicodeString::TheEmptyString );
			Int len = composition.getLength() + e->text->getTextLength();
			for ( int i = 0; i < len; i++ )
			{
				e->sText->appendChar( '*' );
			}
		}
		else
		{
			e->constructText->setText( composition );
			compositeCursorPos = TheIMEManager->getCompositionCursorPosition();
		}

	}


	// get out of here if no text color to show up
	if( textColor == WIN_COLOR_UNDEFINED )
		return;

	// if our text is "secret" we will print only '*' characters
	if( e->secretText )
		text = e->sText;

	// make sure our font is the same as our parents
	if( text->getFont() != window->winGetFont() )
		text->setFont( window->winGetFont() );
	if( e->constructText->getFont() != window->winGetFont() )
		e->constructText->setFont( window->winGetFont() );

	// get the size of our text, and construct text
	Int textWidth = text->getWidth();

	if (!e->drawTextFromStart)
	{
		// clip the text to the edit window size
		window->winGetScreenPosition( &origin.x, &origin.y );
		window->winGetSize( &size.x, &size.y );
		clipRegion.lo.x = x;
		clipRegion.hi.x = x + width ;
		clipRegion.lo.y = y;
		clipRegion.hi.y = y + fontHeight;
		text->setClipRegion( &clipRegion );
		e->constructText->setClipRegion( &clipRegion );

		// set construct window position if needed
		//if( e->constructList && e->constructText->getTextLength() )
		//	e->constructList->winSetPosition( (x + textWidth1), (y + fontHeight) );

		x+= 2;
		// draw the text
		if(textWidth < width)
		{
			text->draw( x, y, textColor, textDropColor );
			cursorPos = textWidth + x;
		}
		else
		{
			Int div = textWidth / (width / 2) - 1;
			text->draw(x - (div * (width/2)), y, textColor, textDropColor);
			cursorPos = textWidth - (div * (width/2)) + x;
		}

		//cursorPos = x + textWidth;
	}
	else
	{
		window->winGetScreenPosition( &origin.x, &origin.y );
		window->winGetSize( &size.x, &size.y );
		clipRegion.lo.x = origin.x;
		clipRegion.hi.x = origin.x + size.x;
		clipRegion.lo.y = origin.y;
		clipRegion.hi.y = origin.y + size.y;
		text->setClipRegion( &clipRegion );
		e->constructText->setClipRegion( &clipRegion );

		// set construct window position if needed
		//if( e->constructList && e->constructText->getTextLength() )
		//	e->constructList->winSetPosition( (x + textWidth1), (y + fontHeight) );

		x+= 5;
		// draw the text
		text->draw( x, y, textColor, textDropColor );
		cursorPos = textWidth + x;
	}

	if (e->constructText->getTextLength() > 0 )
	{
		e->constructText->draw( x + textWidth, y, compositeColor, compositeDropColor );
		cursorPos += e->constructText->getWidth( compositeCursorPos );
	}

	// draw blinking cursor
	GameWindow *parent;
	parent = window->winGetParent();
	if(parent && !BitIsSet(parent->winGetStyle(), GWS_COMBO_BOX))
		parent = nullptr;

	if( (window == TheWindowManager->winGetFocus() || (parent && parent == TheWindowManager->winGetFocus())) && ((drawCnt++ >> 3) & 0x1) )
		TheWindowManager->winFillRect( textColor, WIN_DRAW_LINE_WIDTH,
																	 cursorPos, origin.y + 3,
																	 cursorPos + 2, origin.y + size.y - 3 );
	window->winSetCursorPosition( cursorPos + 2 - origin.x, 0 );

}

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// W3DGadgetTextEntryDraw =====================================================
/** Draw colored entry field using standard graphics */
//=============================================================================
void W3DGadgetTextEntryDraw( GameWindow *window, WinInstanceData *instData )
{
	EntryData *e = (EntryData *)window->winGetUserData();
	ICoord2D origin, size, start, end;
	Color backBorder, backColor, textColor, textBorder,
			compositeColor, compositeBorder;

	// cancel unichar flag
	e->receivedUnichar = FALSE;

	// get size and position of window
	window->winGetScreenPosition( &origin.x, &origin.y );
	window->winGetSize( &size.x, &size.y );

	// get the right colors
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) == FALSE )
	{

		compositeColor	= window->winGetDisabledTextColor();
		compositeBorder	= window->winGetDisabledTextBorderColor();
		textColor		= window->winGetDisabledTextColor();
		textBorder	= window->winGetDisabledTextBorderColor();
		backColor		= GadgetTextEntryGetDisabledColor( window );
		backBorder	= GadgetTextEntryGetDisabledBorderColor( window );

	}
	else if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
	{

		compositeColor	= window->winGetIMECompositeTextColor();
		compositeBorder	= window->winGetIMECompositeBorderColor();
		textColor		= window->winGetHiliteTextColor();
		textBorder	= window->winGetHiliteTextBorderColor();
		backColor		= GadgetTextEntryGetHiliteColor( window );
		backBorder	= GadgetTextEntryGetHiliteBorderColor( window );

	}
	else
	{

		compositeColor	= window->winGetIMECompositeTextColor();
		compositeBorder	= window->winGetIMECompositeBorderColor();
		textColor		= window->winGetEnabledTextColor();
		textBorder	= window->winGetEnabledTextBorderColor();
		backColor		= GadgetTextEntryGetEnabledColor( window );
		backBorder	= GadgetTextEntryGetEnabledBorderColor( window );

	}

	// draw the back border
	if( backBorder != WIN_COLOR_UNDEFINED )
	{

		start.x = origin.x;
		start.y = origin.y;
		end.x = start.x + size.x;
		end.y = start.y + size.y;
		TheWindowManager->winOpenRect( backBorder, WIN_DRAW_LINE_WIDTH,
																	 start.x, start.y, end.x, end.y );

	}

	// draw the filled back
	if( backColor != WIN_COLOR_UNDEFINED )
	{

		start.x = origin.x + 1;
		start.y = origin.y + 1;
		end.x = start.x + size.x - 2;
		end.y = start.y + size.y - 2;
		TheWindowManager->winFillRect( backColor, WIN_DRAW_LINE_WIDTH,
																	 start.x, start.y, end.x, end.y );

	}

	// draw the text
	Int fontHeight = TheWindowManager->winFontHeight( instData->getFont() );
	Int startOffset = 5;
	Int width;

	width = size.x - (2 * startOffset);
	start.x = origin.x + startOffset;  // offset a little bit into the entry
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_ONE_LINE ) )
		start.y = size.y / 2 - fontHeight / 2;
	else
		start.y = origin.y + startOffset;  // offset a little bit into the entry

	// draw the edit text
	drawTextEntryText( window, instData, textColor, textBorder, compositeColor, compositeBorder,
										 start.x, start.y, width, fontHeight );



}

// W3DGadgetTextEntryImageDraw ================================================
/** Draw horizontal slider with user supplied images */
//=============================================================================
void W3DGadgetTextEntryImageDraw( GameWindow *window, WinInstanceData *instData )
{
	EntryData *e = (EntryData *)window->winGetUserData();
	ICoord2D origin, size, start, end;
	Color textColor, textBorder;
	Color compositeColor, compositeBorder;
	const Image *leftImage, *rightImage, *centerImage, *smallCenterImage;
	Int xOffset, yOffset;
	Int i;

	// cancel unichar flag
	e->receivedUnichar = FALSE;

	// get size and position of window
	window->winGetScreenPosition( &origin.x, &origin.y );
	window->winGetSize( &size.x, &size.y );

	// get image offset
	xOffset = instData->m_imageOffset.x;
	yOffset = instData->m_imageOffset.y;

	// get the right colors
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) == FALSE )
	{

		textColor					= window->winGetDisabledTextColor();
		textBorder				= window->winGetDisabledTextBorderColor();
		compositeColor		= window->winGetDisabledTextColor();
		compositeBorder		= window->winGetDisabledTextBorderColor();
		leftImage					= GadgetTextEntryGetDisabledImageLeft( window );
		rightImage				= GadgetTextEntryGetDisabledImageRight( window );
		centerImage				= GadgetTextEntryGetDisabledImageCenter( window );
		smallCenterImage	= GadgetTextEntryGetDisabledImageSmallCenter( window );

	}
	else if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
	{

		textColor					= window->winGetHiliteTextColor();
		textBorder				= window->winGetHiliteTextBorderColor();
		compositeColor		= window->winGetIMECompositeTextColor();
		compositeBorder		= window->winGetIMECompositeBorderColor();
		leftImage					= GadgetTextEntryGetHiliteImageLeft( window );
		rightImage				= GadgetTextEntryGetHiliteImageRight( window );
		centerImage				= GadgetTextEntryGetHiliteImageCenter( window );
		smallCenterImage	= GadgetTextEntryGetHiliteImageSmallCenter( window );

	}
	else
	{

		textColor					= window->winGetEnabledTextColor();
		textBorder				= window->winGetEnabledTextBorderColor();
		compositeColor		= window->winGetIMECompositeTextColor();
		compositeBorder		= window->winGetIMECompositeBorderColor();
		leftImage					= GadgetTextEntryGetEnabledImageLeft( window );
		rightImage				= GadgetTextEntryGetEnabledImageRight( window );
		centerImage				= GadgetTextEntryGetEnabledImageCenter( window );
		smallCenterImage	= GadgetTextEntryGetEnabledImageSmallCenter( window );

	}

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
	rightStart.y = origin.y + yOffset;

	// draw the center repeating bar
	Int centerWidth, pieces;

	// get width we have to draw our repeating center in
	centerWidth = rightStart.x - leftEnd.x;

	// how many whole repeating pieces will fit in that width
	pieces = centerWidth / centerImage->getImageWidth();

	// draw the pieces
	start.x = leftEnd.x;
	start.y = origin.y + yOffset;
	end.y = start.y + size.y;
	for( i = 0; i < pieces; i++ )
	{

		end.x = start.x + centerImage->getImageWidth();
		TheWindowManager->winDrawImage( centerImage,
																		start.x, start.y,
																		end.x, end.y );
		start.x += centerImage->getImageWidth();

	}

	//
	// how many small repeating pieces will fit in the gap from where the
	// center repeating bar stopped and the right image, draw them
	// and overlapping underneath where the right end will go
	//
	centerWidth = rightStart.x - start.x;
	pieces = centerWidth / smallCenterImage->getImageWidth() + 1;
	end.y = start.y + size.y;
	for( i = 0; i < pieces; i++ )
	{

		end.x = start.x + smallCenterImage->getImageWidth();
		TheWindowManager->winDrawImage( smallCenterImage,
																		start.x, start.y,
																		end.x, end.y );
		start.x += smallCenterImage->getImageWidth();

	}

	// draw left end
	start.x = origin.x + xOffset;
	start.y = origin.y + yOffset;
	end = leftEnd;
	TheWindowManager->winDrawImage(leftImage, start.x, start.y, end.x, end.y);

	// draw right end
	start = rightStart;
	end.x = start.x + rightSize.x;
	end.y = start.y + size.y;
	TheWindowManager->winDrawImage(rightImage, start.x, start.y, end.x, end.y);

	// draw the text
	Int fontHeight = TheWindowManager->winFontHeight( instData->getFont() );
	Int startOffset = 5;
	Int width;

	width = size.x - (2 * startOffset);
	start.x = origin.x + startOffset;  // offset a little bit into the entry
		if( BitIsSet( window->winGetStatus(), WIN_STATUS_ONE_LINE ) )
		start.y = size.y / 2 - fontHeight / 2;
	else
		start.y = origin.y + startOffset;  // offset a little bit into the entry

	// draw the edit text
	drawTextEntryText( window, instData, textColor, textBorder, compositeColor, compositeBorder,
										 start.x, start.y, width, fontHeight );



}
