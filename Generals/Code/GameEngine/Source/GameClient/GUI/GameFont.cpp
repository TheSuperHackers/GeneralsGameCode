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

// FILE: GameFont.cpp /////////////////////////////////////////////////////////////////////////////
// Created:    Colin Day, June 2001
// Desc:       Access to our representation for fonts
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "PreRTS.h"	// This must go first in EVERY cpp file int the GameEngine

#include "GameClient/GameFont.h"

// PUBLIC DATA ////////////////////////////////////////////////////////////////////////////////////
FontLibrary *TheFontLibrary = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
/** Link a font to the font list */
//-------------------------------------------------------------------------------------------------
void FontLibrary::linkFont( GameFont *font )
{

	// sanity
	if( font == NULL )
		return;

	// link it
	font->next = m_fontList;
	m_fontList = font;

	// increment linked count
	m_count++;

}  // end linkFont

//-------------------------------------------------------------------------------------------------
/** Unlink a font from the font list */
//-------------------------------------------------------------------------------------------------
void FontLibrary::unlinkFont( GameFont *font )
{
	GameFont *other = NULL;

	// sanity
	if( font == NULL )
		return;

	// sanity check and make sure this font is actually in this library
	for( other = m_fontList; other; other = other->next )
		if( other == font )
			break;
	if( other == NULL )
	{

		DEBUG_CRASH(( "Font '%s' not found in library", font->nameString.str() ));
		return;

	}  // end if

	// scan for the font pointing to the one we're going to unlink
	for( other = m_fontList; other; other = other->next )
		if( other->next == font )
			break;

	//
	// if nothing was fount this was at the head of the list, otherwise
	// remove from chain
	//
	if( other == NULL )
		m_fontList = font->next;
	else
		other->next = font->next;

	// clean up this font we just unlinked just to be cool!
	font->next = NULL;

	// we now have one less font on the list
	m_count--;

}  // end unlinkFont

//-------------------------------------------------------------------------------------------------
/** Delete all font data, DO NOT throw an exception ... the destructor uses this */
//-------------------------------------------------------------------------------------------------
void FontLibrary::deleteAllFonts( void )
{
	GameFont *font;

	// release all the fonts
	while( m_fontList )
	{

		// get temp pointer to this font
		font = m_fontList;

		// remove font fron the list, this will change m_fontList
		unlinkFont( font );

		// release font data
		releaseFontData( font );

		// delete the font list element
		deleteInstance(font);

	}  // end while

}  // deleteAllFonts

///////////////////////////////////////////////////////////////////////////////////////////////////
// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
FontLibrary::FontLibrary( void )
{

	m_fontList = NULL;
	m_count = 0;

}  // end FontLibrary

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
FontLibrary::~FontLibrary( void )
{

	// delete all font data
	deleteAllFonts();

}  // end ~FontLibrary

//-------------------------------------------------------------------------------------------------
/** Initialize what we need to in the font library */
//-------------------------------------------------------------------------------------------------
void FontLibrary::init( void )
{

}  // end init

//-------------------------------------------------------------------------------------------------
/** Reset the fonts for this font library */
//-------------------------------------------------------------------------------------------------
void FontLibrary::reset( void )
{

	// delete all font data
	deleteAllFonts();

}  // end reset

//-------------------------------------------------------------------------------------------------
/** Get a font from our list, if we don't have that font loaded we will
	* attempt to load it */
//-------------------------------------------------------------------------------------------------
GameFont *FontLibrary::getFont( AsciiString name, Int pointSize, Bool bold )
{
	GameFont *font;

	// search for font in list
	for( font = m_fontList; font; font = font->next )
	{

		if( font->pointSize == pointSize &&
				font->bold == bold &&
				font->nameString == name
			)
			return font;  // found

	}  // end for font

	// font not found, allocate a new font element
	font = newInstance(GameFont);
	if( font == NULL )
	{

		DEBUG_CRASH(( "getFont: Unable to allocate new font list element" ));
		return NULL;

	}  // end if

	// copy font data over to new element
	font->nameString = name;
	font->pointSize = pointSize;
	font->bold = bold;
	font->fontData = NULL;

	//DEBUG_LOG(("Font: Loading font '%s' %d point", font->nameString.str(), font->pointSize));
	// load the device specific data pointer
	if( loadFontData( font ) == FALSE )
	{

		DEBUG_CRASH(( "getFont: Unable to load font data pointer '%s'", name.str() ));
		deleteInstance(font);
		return NULL;

	}  // end if

	// tie font into list
	linkFont( font );

	// all is done and loaded
	return font;

}  // end getFont
