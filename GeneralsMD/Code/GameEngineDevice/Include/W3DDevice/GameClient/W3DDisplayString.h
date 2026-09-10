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

// FILE: W3DDisplayString.h ///////////////////////////////////////////////////
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
// File name:  W3DDisplayString.h
//
// Created:    Colin Day, July 2001
//
// Desc:       Display string W3D implementation, display strings hold
//						 double byte characters and all the data we need to render
//						 those strings to the screen.
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/GameMemory.h"
#include "GameClient/DisplayString.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////
class W3DDisplayStringManager;
namespace Engine::UI::WND
{
class FontFace;
class DrawList;
struct StaticTextVisual;
}

// TYPE DEFINES ///////////////////////////////////////////////////////////////

// W3DDisplayString -----------------------------------------------------------
/** */
//-----------------------------------------------------------------------------
class W3DDisplayString : public DisplayString
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( W3DDisplayString, "W3DDisplayString" )

public:

	friend W3DDisplayStringManager;

	W3DDisplayString();
	// ~W3DDisplayString();  // destructor defined by memory pool

	virtual void notifyTextChanged() override;							///< called when text contents change
	virtual void draw( Int x, Int y, Color color, Color dropColor ) override;  ///< render text
	virtual void draw( Int x, Int y, Color color, Color dropColor, Int xDrop, Int yDrop ) override;  ///< render text with the drop shadow being at the offsets passed in
	virtual void getSize( Int *width, Int *height ) override;		///< get render size
	virtual Int	getWidth( Int charPos = -1) override;
	virtual void setWordWrap( Int wordWrap ) override;						///< set the word wrap width
	virtual void setWordWrapCentered( Bool isCentered ) override; ///< If this is set to true, the text on a new line is centered
	virtual void setFont( GameFont *font ) override;							///< set a font for display
	virtual void setUseHotkey( Bool useHotkey, Color hotKeyColor = 0xffffffff ) override;
	virtual void setClipRegion( IRegion2D *region ) override;		///< clip text in this region
	bool appendDrawData( Engine::UI::WND::DrawList &drawList,
		Int x, Int y, Color color, Color dropColor, Int xDrop = 1, Int yDrop = 1,
		const IRegion2D *clipRegion = nullptr );
	bool appendStaticTextDrawData( Engine::UI::WND::DrawList &drawList,
		const Engine::UI::WND::StaticTextVisual &visual,
		Color color, Color dropColor );
	const WideChar *getTextData() const noexcept { return m_textString.str(); }

protected:

	void usingResources( UnsignedInt frame );  /**< call this whenever display
																						 resources are in use */
	void computeExtents();  ///< compupte text width and height

	Bool m_textChanged;  ///< when contents of string change this is TRUE
	Bool m_fontChanged;  ///< when font has changed this is TRUE
	Engine::UI::WND::FontFace *m_hotKeyFont;
	UnicodeString m_hotkey;		///< holds the current hotkey marker.
	Bool m_useHotKey;
	Color m_hotKeyColor;
	ICoord2D m_size;				///< (width,height) size of rendered text
	IRegion2D m_clipRegion; ///< the clipping region for text
	Int m_wordWrap;
	Bool m_wordWrapCentered;
	Bool m_hasClipRegion;
	UnsignedInt m_lastResourceFrame;  ///< last frame resources were used on

};

///////////////////////////////////////////////////////////////////////////////
// INLINING ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
inline void W3DDisplayString::usingResources( UnsignedInt frame ) { m_lastResourceFrame = frame; }

// EXTERNALS //////////////////////////////////////////////////////////////////
