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
// File name:  W3DDevice/GameClient/W3DDebugDisplay.h
//
// Created:    11/13/01 TR
//
//----------------------------------------------------------------------------

#pragma once

//----------------------------------------------------------------------------
//           Includes
//----------------------------------------------------------------------------

#include "GameClient/DebugDisplay.h"


//----------------------------------------------------------------------------
//           Forward References
//----------------------------------------------------------------------------

class GameFont;
class DisplayString;

//----------------------------------------------------------------------------
//           Type Defines
//----------------------------------------------------------------------------


//===============================
// W3DDebugDisplay
//===============================

class W3DDebugDisplay : public DebugDisplay
{

	public:

		W3DDebugDisplay();
		virtual ~W3DDebugDisplay();

		void init( void );																						///< Initialized the display
		void setFont( GameFont *font );																///< Set the font to render with
		void setFontWidth( Int width ) { m_fontWidth = width; };			///< Set the font width
		void setFontHeight( Int height ) { m_fontHeight = height; };		///< Set the font height

	protected:

		GameFont *m_font;			///< Font to render text with
		Int m_fontWidth;
		Int m_fontHeight;
		DisplayString *m_displayString;

		virtual void drawText( Int x, Int y, Char *text );			///< Render null terminated string at current cursor position

};


//----------------------------------------------------------------------------
//           Inlining
//----------------------------------------------------------------------------
