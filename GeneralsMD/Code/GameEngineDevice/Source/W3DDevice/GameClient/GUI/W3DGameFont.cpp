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

// FILE: W3DGameFont.cpp //////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//                                                                          
//                       Westwood Studios Pacific.                          
//                                                                          
//                       Confidential Information                           
//                Copyright (C) 2001 - All Rights Reserved                  
//                                                                          
//-----------------------------------------------------------------------------
//
// Project:    RTS3
//
// File name:  W3DGameFont.cpp
//
// Created:    Colin Day, June 2001
//
// Desc:       W3D implementation for managing font definitions
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <stdlib.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/Debug.h"
#include "W3DDevice/GameClient/W3DGameFont.h"
#include "WW3D2/ww3d.h"
#include "WW3D2/assetmgr.h"
#include "WW3D2/render2dsentence.h"
#include "GameClient/GlobalLanguage.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

// PRIVATE PROTOTYPES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS //////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// W3DFontLibrary::loadFontData ===============================================
/** Load a font */
//=============================================================================
Bool W3DFontLibrary::loadFontData( GameFont *font )
{
	FontCharsClass *fontChar;

	// sanity
	if( font == NULL )
		return FALSE;

	if ((UnsignedInt)font->pointSize > 100)	//sanity check the size - anything over 100 is probably wrong. -MW
		fontChar = NULL;
	else
	{	// get the font data from the asset manager
		fontChar = WW3DAssetManager::
									Get_Instance()->Get_FontChars( font->nameString.str(), font->pointSize,
																								 font->bold ? true : false );
	}

	if( fontChar == NULL )
	{

		DEBUG_LOG(( "W3D load font: unable to find font '%s' from asset manager\n",
						 font->nameString.str() ));
		DEBUG_ASSERTCRASH(fontChar, ("Missing or Corrupted Font.  Pleas see log for details"));
		return FALSE;

	}  // end if

	// assign font data
	font->fontData = fontChar;
	font->height = fontChar->Get_Char_Height();

	FontCharsClass *unicodeFontChar = NULL;

	// load unicode of same point size
	if(TheGlobalLanguageData)
		unicodeFontChar = WW3DAssetManager::
									Get_Instance()->Get_FontChars( TheGlobalLanguageData->m_unicodeFontName.str(), font->pointSize,
																								 font->bold ? true : false );
	else
		unicodeFontChar = WW3DAssetManager::
									Get_Instance()->Get_FontChars( "Arial Unicode MS", font->pointSize,
																								 font->bold ? true : false );

	if ( unicodeFontChar )
	{
		fontChar->AlternateUnicodeFont = unicodeFontChar;
	}

	// all done and loaded
	return TRUE;

}  // end loadFont

// W3DFontLibrary::releaseFontData ============================================
/** Release font data */
//=============================================================================
void W3DFontLibrary::releaseFontData( GameFont *font )
{

	// presently we don't need to do anything because fonts are handled in
	// the W3D asset manager which is all taken for of us
	if (font && font->fontData)
	{
		if(((FontCharsClass *)(font->fontData))->AlternateUnicodeFont)
			((FontCharsClass *)(font->fontData))->AlternateUnicodeFont->Release_Ref();
		((FontCharsClass *)(font->fontData))->Release_Ref();

		font->fontData = NULL;
	}

}  // end releaseFontData

// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////

