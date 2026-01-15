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
// Project:   Generals
//
// Module:    Debug
//
// File name: W3DGameDevice/GameClisnt/W3DDebugDisplay.cpp
//
// Created:   11/13/01 TR
//
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//         Includes
//----------------------------------------------------------------------------

#include "W3DDevice/GameClient/W3DDebugDisplay.h"
#include "GameClient/GameFont.h"
#include "GameClient/DisplayStringManager.h"
#include "GameClient/DisplayString.h"

//----------------------------------------------------------------------------
//         Externals
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Defines
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Types
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Data
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Public Data
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Prototypes
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Private Functions
//----------------------------------------------------------------------------



//----------------------------------------------------------------------------
//         Public Functions
//----------------------------------------------------------------------------

//============================================================================
// W3DDebugDisplay::W3DDebugDisplay
//============================================================================

W3DDebugDisplay::W3DDebugDisplay()
: m_displayString(nullptr)
{

}

//============================================================================
// W3DDebugDisplay::~W3DDebugDisplay
//============================================================================

W3DDebugDisplay::~W3DDebugDisplay()
{
	if ( m_displayString )
	{
		TheDisplayStringManager->freeDisplayString( m_displayString );
	}
}

//============================================================================
// W3DDebugDisplay::init
//============================================================================

void W3DDebugDisplay::init( void )
{
	m_displayString = TheDisplayStringManager->newDisplayString();
}

//============================================================================
// W3DDebugDisplay::drawText
//============================================================================

void W3DDebugDisplay::drawText( Int x, Int y, Char *text )
{
	if ( m_font == nullptr || m_displayString == nullptr )
	{
		return ;
	}

	::Color textColor = GameMakeColor( 255, 255, 255, 255 );
	::Color dropColor = GameMakeColor( 0, 0, 0, 255 );

	UnicodeString unicode;

	unicode.translate( AsciiString( text ) );
	m_displayString->setText( unicode );
	m_displayString->draw( x*m_fontWidth, 13 + y*m_fontHeight, textColor, dropColor );
}

//============================================================================
// W3DDebugDisplay::setFont
//============================================================================

void W3DDebugDisplay::setFont( GameFont *font )
{
	m_font = font;
	if ( m_displayString )
	{
		m_displayString->setFont( m_font );
	}
}

