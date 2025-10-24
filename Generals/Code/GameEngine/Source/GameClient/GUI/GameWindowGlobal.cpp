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

// FILE: GameWindowGlobal.cpp /////////////////////////////////////////////////
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
// File name: GameWindowGlobal.cpp
//
// Created:   Colin Day, June 2001
//
// Desc:      These are some global functions that every game using this
//						window system must implement for their current technology
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/Image.h"
#include "GameClient/Display.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/GameFont.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// GameWindowManager::winDrawImage ============================================
/** draw image, coord are in screen and should be kepth within
	* that box specified */
//=============================================================================
void GameWindowManager::winDrawImage( const Image *image, Int startX, Int startY,
																			Int endX, Int endY, Color color )
{

	TheDisplay->drawImage( image, startX, startY, endX, endY, color );

}

// GameWindowManager::winFillRect =============================================
/** draw filled rect, coords are absolute screen coords */
//=============================================================================
void GameWindowManager::winFillRect( Color color, Real width,
																		 Int startX, Int startY,
																		 Int endX, Int endY )
{

	TheDisplay->drawFillRect( startX, startY,
														endX - startX, endY - startY,
														color );

}

// GameWindowManager::winOpenRect =============================================
/** draw rect outline, coords are absolute screen coords */
//=============================================================================
void GameWindowManager::winOpenRect( Color color, Real width,
																		 Int startX, Int startY,
																		 Int endX, Int endY )
{

	TheDisplay->drawOpenRect( startX, startY,
														endX - startX, endY - startY,
														width, color );

}

// GameWindowManager::winDrawLine =============================================
/** draw line, coords are absolute screen coords */
//=============================================================================
void GameWindowManager::winDrawLine( Color color, Real width,
																		 Int startX, Int startY,
																		 Int endX, Int endY )
{

	TheDisplay->drawLine( startX, startY, endX, endY, width, color );

}

// GameWindowManager::winFindImage ============================================
/** Given an image name, return an image loc to that image information.
	* You may reorganize the image loc structure however you like to suit
	* your individual project */
//=============================================================================
const Image *GameWindowManager::winFindImage( const char *name )
{

	assert( TheMappedImageCollection );
	if( TheMappedImageCollection )
		return TheMappedImageCollection->findImageByName( AsciiString( name ) );

	return NULL;

}

// GameWindowManager::winMakeColor ============================================
/** Given RGBA, make a color, you can change color representation for your
	* individual project needs */
//=============================================================================
Color GameWindowManager::winMakeColor( UnsignedByte red,
																			 UnsignedByte green,
																			 UnsignedByte blue,
																			 UnsignedByte alpha )
{

	return GameMakeColor( red, green, blue, alpha );

}

// GameWindowManager::winFormatText ===========================================
/** draw text to the screen */
//=============================================================================
void GameWindowManager::winFormatText( GameFont *font, UnicodeString text, Color color,
																			 Int x, Int y, Int width, Int height )
{

	/// @todo make all display string rendering go through here!

}

// GameWindowManager::winGetTextSize ==========================================
/** get the extent size of text */
//=============================================================================
void GameWindowManager::winGetTextSize( GameFont *font, UnicodeString text,
																				Int *width, Int *height, Int maxWidth )
{

	/// @todo make display string size stuff go through here

	if( width )
		*width = 0;

	if( height )
		*height = 0;

}

// GameWindowManager::winFontHeight ===========================================
/** Return the font height in pixels */
//=============================================================================
Int GameWindowManager::winFontHeight( GameFont *font )
{

	return font->height;

}

// GameWindowManager::winIsDigit ==============================================
/** You implementation of whether or not character is a digit */
//=============================================================================
Int GameWindowManager::winIsDigit( Int c )
{

	return iswdigit( c );

}

// GameWindowManager::winIsAscii ==============================================
/** You implementation of whether or not character is ascii */
//=============================================================================
Int GameWindowManager::winIsAscii( Int c )
{

	return iswascii( c );

}

// GameWindowManager::winIsAlNum ==============================================
/** Your implementation of whether or not character is alpha numeric */
//=============================================================================
Int GameWindowManager::winIsAlNum( Int c )
{

	return iswalnum( c );

}

// GameWindowManager::winFindFont =============================================
/** Get a font */
//=============================================================================
GameFont *GameWindowManager::winFindFont( AsciiString fontName,
																					Int pointSize,
																					Bool bold )
{

	assert( TheFontLibrary );
	if( TheFontLibrary )
		return TheFontLibrary->getFont( fontName, pointSize, bold );

	return NULL;

}

