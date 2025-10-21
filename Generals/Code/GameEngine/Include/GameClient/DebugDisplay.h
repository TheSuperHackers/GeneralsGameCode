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

//----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//----------------------------------------------------------------------------
//
// Project:    Generals
//
// Module:     Debug
//
// File name:  GameClient/DebugDisplay.h
//
// Created:    11/13/01 TR
//
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------
//           Includes
//----------------------------------------------------------------------------

#include "Lib/BaseType.h"
#include <Utility/stdio_adapter.h>


//----------------------------------------------------------------------------
//           Forward References
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//           Type Defines
//----------------------------------------------------------------------------


//===============================
// DebugDisplayInterface
//===============================

class DebugDisplayInterface
{

	public:

		enum Color
		{
			WHITE,
			BLACK,
			YELLOW,
			RED,
			GREEN,
			BLUE,
		};

		virtual ~DebugDisplayInterface() {};

		virtual void	printf( const Char *format, ...) = 0;			///< Print formatted text at current cursor position
		virtual void	setCursorPos( Int x, Int y ) = 0;		///< Set new cursor position
		virtual Int		getCursorXPos( void ) = 0;					///< Get current X position of cursor
		virtual Int		getCursorYPos( void ) = 0;					///< Get current Y position of cursor
		virtual Int		getWidth( void ) = 0;								///< Get character width of display
		virtual Int		getHeight( void ) = 0;							///< Get character height of display
		virtual void	setTextColor( Color color ) = 0;		///< Set text color
		virtual void	setRightMargin( Int rightPos ) = 0;	///< Set right margin position
		virtual void	setLeftMargin( Int leftPos ) = 0;		///< Set left margin position
		virtual void	reset( void ) = 0;									///< Reset back to default settings

	protected:

		virtual void drawText( Int x, Int y, Char *text ) = 0;			///< Render null ternimated string at current cursor position
};


//===============================
// DebugDisplay
//===============================

class DebugDisplay : public DebugDisplayInterface
{

	public:

		DebugDisplay();
		virtual ~DebugDisplay() {};

		virtual void	printf( const Char *format, ...);			///< Print formatted text at current cursor position
		virtual void	setCursorPos( Int x, Int y );		///< Set new cursor position
		virtual Int		getCursorXPos( void );					///< Get current X position of cursor
		virtual Int		getCursorYPos( void );					///< Get current Y position of cursor
		virtual Int		getWidth( void );								///< Get character width of display
		virtual Int		getHeight( void );							///< Get character height of display
		virtual void	setTextColor( Color color );		///< set text color
		virtual void	setRightMargin( Int rightPos );	///< set right margin position
		virtual void	setLeftMargin( Int leftPos );		///< set left margin position
		virtual void	reset( void );									///< Reset back to default settings

	protected:

		Color m_textColor;	///< Color to render text in
		Int		m_xPos;				///< Current X position of cursor
		Int		m_yPos;				///< Current Y position of cursor
		Int		m_width;			///< Character width of display
		Int		m_height;			///< Character height of display
		Int		m_rightMargin;///< Right margin position
		Int		m_leftMargin;	///< Left margin position

};

// displayers

#if defined(RTS_DEBUG)
extern void AudioDebugDisplay( DebugDisplayInterface *debugDisplay, void *userData, FILE *fp );
#endif


//----------------------------------------------------------------------------
//           Inlining
//----------------------------------------------------------------------------
