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

// FILE: GameFont.h ///////////////////////////////////////////////////////////////////////////////
// Created:    Colin Day, June 2001
// Desc:       Game representations for fonts
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __GAMEFONT_H_
#define __GAMEFONT_H_

#include "Common/SubsystemInterface.h"
#include "Lib/BaseType.h"
#include "Common/AsciiString.h"
#include "Common/GameMemory.h"

//-------------------------------------------------------------------------------------------------
/** A font for use in the device independent game */
//-------------------------------------------------------------------------------------------------
class GameFont : public MemoryPoolObject
{
	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE(GameFont, "GameFont")
public:
	GameFont*			next;  ///< for library use
	AsciiString		nameString;
	Int						pointSize;		 ///< point size of font
	Int						height;				///< pixel height of font
	void*					fontData;  ///< font data to be filled out for device specific font
	Bool					bold;				 ///< is this font bold
};
EMPTY_DTOR(GameFont)

//-------------------------------------------------------------------------------------------------
/** Interface to access fonts for the system */
//-------------------------------------------------------------------------------------------------
class FontLibrary : public SubsystemInterface
{

public:


public:

	FontLibrary( void );
	virtual ~FontLibrary( void );

	virtual void init( void );
	virtual void reset( void );
	virtual void update( void ) { }

	GameFont *getFont( AsciiString name, Int pointSize, Bool bold );  ///< get a font pointer

	GameFont *firstFont( void );  ///< return first font
	GameFont *nextFont( GameFont *font );  ///< get next font in library

	Int getCount( void );  ///< return how many fonts are loaded in this lib

protected:

	void deleteAllFonts( void );				///< delete all fonts in this library
	void linkFont( GameFont *font );		///< add to font list
	void unlinkFont( GameFont *font );  ///< remove font from list

	/// load the font data pointer based on everything else we already have set
	virtual Bool loadFontData( GameFont *font ) = 0;
	/// release the font data pointer
	virtual void releaseFontData( GameFont *font ) { };

	GameFont *m_fontList;  ///< list of fonts we have loaded
	Int m_count;  ///< number of unique fonts loaded in this lib

};

// INLINING ///////////////////////////////////////////////////////////////////////////////////////
inline Int FontLibrary::getCount( void ) { return m_count; }
inline GameFont *FontLibrary::firstFont( void ) { return m_fontList; }
inline GameFont *FontLibrary::nextFont( GameFont *font )
{
	if( font )
		return font->next;
	return NULL;
}

// EXTERNALS //////////////////////////////////////////////////////////////////////////////////////
extern FontLibrary *TheFontLibrary;  ///< font library external

#endif // __GAMEFONT_H_

