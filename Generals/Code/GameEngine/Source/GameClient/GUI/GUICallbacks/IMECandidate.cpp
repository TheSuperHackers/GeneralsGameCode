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

// FILE: IMECandidate.cpp /////////////////////////////////////////////////////////////////////////
// Author: TR November 2001
// Desc: IME Candidate window callbacks
///////////////////////////////////////////////////////////////////////////////////////////////////

// INCLUDES ///////////////////////////////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameClient/GameWindow.h"
#include "GameClient/Gadget.h"
#include "GameClient/IMEManager.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/DisplayString.h"
#include "GameClient/DisplayStringManager.h"


Int	IMECandidateWindowLineSpacing = 2;

static DisplayString *Dstring = nullptr;

//-------------------------------------------------------------------------------------------------
/** Input procedure for the candidate window */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType IMECandidateWindowInput( GameWindow *window, UnsignedInt msg,
																			WindowMsgData mData1, WindowMsgData mData2 )
{

	return MSG_HANDLED;

}

//-------------------------------------------------------------------------------------------------
/** System callback for the IME Candidate window */
//-------------------------------------------------------------------------------------------------
WindowMsgHandledType IMECandidateWindowSystem( GameWindow *window, UnsignedInt msg,
																			 WindowMsgData mData1, WindowMsgData mData2 )
{
	switch( msg )
	{
		case GWM_CREATE:
			if ( Dstring == nullptr )
			{
				Dstring = TheDisplayStringManager->newDisplayString();
			}
		break;

		case GWM_DESTROY:

			if ( Dstring != nullptr )
			{
				TheDisplayStringManager->freeDisplayString( Dstring );
				Dstring = nullptr;
			}

		break;
		//---------------------------------------------------------------------------------------------
		default:
			return MSG_IGNORED;

	}

	return MSG_HANDLED;

}

// IMECandidateDraw ================================================================
/** Draw function for the IME candidate window */
//=============================================================================
void IMECandidateTextAreaDraw( GameWindow *window, WinInstanceData *instData )
{
	// set up for rendering
	ICoord2D origin, size, start, end;
	Color		textColor,
					textBorder,
					textSelectColor,
					textSelectBorder;
	IRegion2D textRegion;
	Color black = GameMakeColor( 0, 0, 0, 255);

	// get window position and size
	window->winGetScreenPosition( &origin.x, &origin.y );
	window->winGetSize( &size.x, &size.y );

	// get a nice region from the positions
	textRegion.lo.x = origin.x;
	textRegion.lo.y = origin.y;
	textRegion.hi.x = origin.x + size.x;
	textRegion.hi.y = origin.y + size.y;

	// get the right colors for drawing
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) == FALSE )
	{

		textSelectColor		= window->winGetDisabledTextColor();
		textSelectBorder	= window->winGetDisabledTextBorderColor();
		textColor		= window->winGetDisabledTextColor();
		textBorder	= window->winGetDisabledTextBorderColor();

	}
	else if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
	{

		textColor		= window->winGetEnabledTextColor();
		textBorder	= window->winGetEnabledTextBorderColor();
		textSelectColor		= window->winGetHiliteTextColor();
		textSelectBorder	= window->winGetHiliteTextBorderColor();

	}
	else
	{

		textSelectColor		= window->winGetHiliteTextColor();
		textSelectBorder	= window->winGetHiliteTextBorderColor();
		textColor		= window->winGetEnabledTextColor();
		textBorder	= window->winGetEnabledTextBorderColor();

	}


	{
		Real borderWidth = 1.0f;

		start.x = origin.x;
		start.y = origin.y;
		end.x = start.x + size.x;
		end.y = start.y + size.y;
		TheWindowManager->winOpenRect( black, borderWidth,
																	 start.x, start.y, end.x, end.y );
	}

	if ( Dstring == nullptr )
	{
		return;
	}

	IMEManagerInterface *ime = (IMEManagerInterface*)window->winGetUserData();

	if ( ime == nullptr )
	{
		return;
	}

	GameFont *font = window->winGetFont();

	// set the font
	Dstring->setFont( font );

	// calculate line height
	Int fontHeight = font ? font->height : 0;
	Int height = fontHeight + IMECandidateWindowLineSpacing;

	// set the clip region
	Dstring->setClipRegion( &textRegion );

	Int first = ime->getCandidatePageStart();
	Int total = ime->getCandidateCount();
	Int pageSize = ime->getCandidatePageSize();
	Int selected = ime->getSelectedCandidateIndex();

	Int count = pageSize;

	if ( count + first > total )
	{
		count = total - first;
	}

	selected = selected - first;
	UnicodeString number;

	// calculate the widest number text
	Int width;
	Dstring->setText(L"00:");
	width = Dstring->getWidth();

	// calc y start pos
	Int y = origin.y;
	Int leftEdge = origin.x + 10 + width;

	for ( Int i = 0; i < count; i++, y+= height )
	{
		const UnicodeString *candidate = ime->getCandidate( first + i );
		Int tcolor, bcolor;

		if ( i == selected )
		{
			tcolor = textSelectColor;
			bcolor = textSelectBorder;
		}
		else
		{
			tcolor = textColor;
			bcolor = textBorder;
		}

		// draw number tab first
		number.format( L"%d:", i + ime->getIndexBase());
		Dstring->setText( number );
		width = Dstring->getWidth();
		Dstring->draw( leftEdge - width, y,	tcolor, black);

		// draw candidate
		Dstring->setText( *candidate );
		Dstring->draw( leftEdge, y, tcolor, black );
	}
}

// IMECandidateDraw ================================================================
/** Draw function for the IME candidate window */
//=============================================================================
void IMECandidateMainDraw( GameWindow *window, WinInstanceData *instData )
{
	// set up for rendering
	ICoord2D origin, size, start, end;
	Color		backColor,
					backBorder;
	Real borderWidth = 1.0f;

	// get window position and size
	window->winGetScreenPosition( &origin.x, &origin.y );
	window->winGetSize( &size.x, &size.y );

	// get the right colors for drawing
	if( BitIsSet( window->winGetStatus(), WIN_STATUS_ENABLED ) == FALSE )
	{

		backColor		= window->winGetDisabledColor( 0 );
		backBorder	= window->winGetDisabledBorderColor( 0 );

	}
	else if( BitIsSet( instData->getState(), WIN_STATE_HILITED ) )
	{

		backColor		= window->winGetHiliteColor( 0 );
		backBorder	= window->winGetHiliteBorderColor( 0 );

	}
	else
	{

		backColor		= window->winGetEnabledColor( 0 );
		backBorder	= window->winGetEnabledBorderColor( 0 );

	}

	// draw the back border
	if( backBorder != WIN_COLOR_UNDEFINED )
	{

		start.x = origin.x;
		start.y = origin.y;
		end.x = start.x + size.x;
		end.y = start.y + size.y;
		TheWindowManager->winOpenRect( backBorder, borderWidth,
																	 start.x, start.y, end.x, end.y );
	}

	// draw the filled back
	if( backColor != WIN_COLOR_UNDEFINED )
	{

		start.x = origin.x + 1;
		start.y = origin.y + 1;
		end.x = start.x + size.x - 2;
		end.y = start.y + size.y - 2;
		TheWindowManager->winFillRect( backColor, 0, start.x, start.y, end.x, end.y );

	}
}

