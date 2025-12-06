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

// FILE: W3DStaticText.cpp ////////////////////////////////////////////////////
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
// File name: W3DStaticText.cpp
//
// Created:   Colin Day, June 2001
//
// Desc:      W3D implementation of the static text GUI control
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdlib.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/GlobalData.h"
#include "GameClient/GameWindowGlobal.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GadgetStaticText.h"
#include "W3DDevice/GameClient/W3DGameWindow.h"
#include "W3DDevice/GameClient/W3DGadget.h"
#include "W3DDevice/GameClient/W3DDisplay.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////
//enum { DRAW_BUF_LEN = 2048 };
//static WideChar drawBuf[ DRAW_BUF_LEN ];

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// drawStaticTextText =========================================================
/** Draw the text for a static text window */
//=============================================================================
static void drawStaticTextText( GameWindow *window, WinInstanceData *instData,
																Color textColor, Color textDropColor )
{
	TextData *tData = (TextData *)window->winGetUserData();
	Int textWidth, textHeight, wordWrap;
	DisplayString *text = tData->text;
	ICoord2D origin, size, textPos;
	IRegion2D clipRegion;
	// sanity
	if( text == NULL || text->getTextLength() == 0 )
		return;

	// get window position and size
	window->winGetScreenPosition( &origin.x, &origin.y );
	window->winGetSize( &size.x, &size.y );

	// Set the text Wrap width
	wordWrap = size.x - 10;
	//if(wordWrap == 89)
	//	wordWrap = 95;
	text->setWordWrap(wordWrap);
	if( BitIsSet(window->winGetStatus(), WIN_STATUS_WRAP_CENTERED)		)
		text->setWordWrapCentered(TRUE);
	else
		text->setWordWrapCentered(FALSE);
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_HOTKEY_TEXT ) && TheGlobalData)
		text->setUseHotkey(TRUE, TheGlobalData->m_hotKeyTextColor);
	else
		text->setUseHotkey(FALSE, 0);


	// how much space will this text take up
	text->getSize( &textWidth, &textHeight );

	//Init the clip region
	clipRegion.lo.x = origin.x ;
	clipRegion.lo.y = origin.y ;
	clipRegion.hi.x = origin.x + size.x ;
	clipRegion.hi.y = origin.y + size.y;

	// horizontal centering?
	if( tData->centered )
	{
		textPos.x = origin.x + (size.x / 2) - (textWidth / 2);
	}
	else
	{
		textPos.x = origin.x + tData->leftMargin;
	}

	// vertical centering?
	if ( tData->centeredVertically )
	{
		textPos.y = origin.y + (size.y / 2) - (textHeight / 2);
	}
	else
	{
		textPos.y = origin.y + tData->topMargin;
	}

	// draw the text
	text->setClipRegion(&clipRegion);
	text->draw( textPos.x, textPos.y, textColor, textDropColor );

}

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// W3DGadgetStaticTextDraw ====================================================
/** Draw colored text field using standard graphics */
//=============================================================================
void W3DGadgetStaticTextDraw( GameWindow *window, WinInstanceData *instData )
{
	TextData *tData = (TextData *)window->winGetUserData();
	Color backColor, backBorder, textColor, textOutlineColor;
	ICoord2D size, origin, start, end;

	// get window position and size
	window->winGetScreenPosition( &origin.x, &origin.y );
	window->winGetSize( &size.x, &size.y );

	// get the colors we will use
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) == FALSE )
	{

		backColor					= GadgetStaticTextGetDisabledColor( window );
		backBorder				= GadgetStaticTextGetDisabledBorderColor( window );
		textColor					= window->winGetDisabledTextColor();
		textOutlineColor	= window->winGetDisabledTextBorderColor();

	}
	else
	{

		backColor					= GadgetStaticTextGetEnabledColor( window );
		backBorder				= GadgetStaticTextGetEnabledBorderColor( window );
		textColor					= window->winGetEnabledTextColor();
		textOutlineColor	= window->winGetEnabledTextBorderColor();

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

	// draw the back fill area
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
  if( tData->text && (textColor != WIN_COLOR_UNDEFINED) )
		drawStaticTextText( window, instData, textColor, textOutlineColor );



}

// W3DGadgetStaticTextImageDraw ===============================================
/** Draw colored text field with user supplied images */
//=============================================================================
void W3DGadgetStaticTextImageDraw( GameWindow *window, WinInstanceData *instData )
{
	TextData *tData = (TextData *)window->winGetUserData();
	Color textColor, textOutlineColor;
	ICoord2D size, origin, start, end;
	const Image *image;

	// get window position and size
	window->winGetScreenPosition( &origin.x, &origin.y );
	window->winGetSize( &size.x, &size.y );

	// get the colors we will use
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) == FALSE )
	{

		image							= GadgetStaticTextGetDisabledImage( window );
		textColor					= window->winGetDisabledTextColor();
		textOutlineColor	= window->winGetDisabledTextBorderColor();

	}
	else
	{

		image							= GadgetStaticTextGetEnabledImage( window );
		textColor					= window->winGetEnabledTextColor();
		textOutlineColor	= window->winGetEnabledTextBorderColor();

	}

	// draw the back image
	if( image )
	{

		start.x = origin.x + instData->m_imageOffset.x;
		start.y = origin.y + instData->m_imageOffset.y;
		end.x = start.x + size.x;
		end.y = start.y + size.y;
		TheWindowManager->winDrawImage( image, start.x, start.y, end.x, end.y );

	}

	// draw the text
  if( tData->text && (textColor != WIN_COLOR_UNDEFINED) )
		drawStaticTextText( window, instData, textColor, textOutlineColor );



}

