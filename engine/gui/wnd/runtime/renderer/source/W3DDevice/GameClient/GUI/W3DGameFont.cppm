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
#include "Precompiled/PreRTS.h"
#include <cstdint>

import Engine.UI.WND;
import Assets.Cache;
import Assets.Runtime;

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/Debug.h"
#include "W3DDevice/GameClient/W3DGameFont.h"

// DEFINES ////////////////////////////////////////////////////////////////////

// PRIVATE TYPES //////////////////////////////////////////////////////////////

// PRIVATE DATA ///////////////////////////////////////////////////////////////

// PUBLIC DATA ////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////

// W3DFontLibrary::loadFontData ===============================================
/** Load a font */
//=============================================================================
Bool W3DFontLibrary::loadFontData( GameFont *font )
{
	// sanity
	if( font == nullptr )
		return FALSE;

	Assets::AssetCache *cache = Assets::Try_Get_Asset_Cache();
	if (cache == nullptr)
		return FALSE;

	// The authored menu face uses Arial with a constrained average character
	// width, matching the original font request at 96 DPI.
	const bool condensed = font->nameString == "Generals";
	const std::uint32_t pixelHeight = static_cast<std::uint32_t>(font->pointSize) * 96u / 72u;
	const Assets::FontAssetHandle handle = cache->Request_Font(
		condensed ? "Arial" : font->nameString.str(),
		static_cast<std::uint32_t>(font->pointSize),
		font->bold != FALSE,
		condensed ? pixelHeight * 2u / 5u : 0u);
	if (!handle.Is_Valid())
		return FALSE;
	cache->Wait(handle);
	const Assets::FontAsset *font_asset = cache->Try_Get_Font(handle);
	if (font_asset == nullptr) {
		DEBUG_CRASH(( "Unable to load font asset '%s'", font->nameString.str() ));
		return FALSE;
	}

	Engine::UI::WND::FontFace *font_face = new Engine::UI::WND::FontFace;
	if (!font_face->Build(*font_asset)) {
		delete font_face;
		return FALSE;
	}

	font->fontData = font_face;
	font->height = font_face->Height();

	return TRUE;
}

// W3DFontLibrary::releaseFontData ============================================
/** Release font data */
//=============================================================================
void W3DFontLibrary::releaseFontData( GameFont *font )
{

	if (font && font->fontData)
	{
		delete static_cast<Engine::UI::WND::FontFace *>(font->fontData);
		font->fontData = nullptr;
	}

}

// PUBLIC FUNCTIONS ///////////////////////////////////////////////////////////

