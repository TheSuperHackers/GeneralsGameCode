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

#ifndef __W3DDISPLAYSTRING_H_
#define __W3DDISPLAYSTRING_H_

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Common/GameMemory.h"
#include "GameClient/DisplayString.h"
#include "WW3D2/render2dsentence.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////
class W3DDisplayStringManager;

// TYPE DEFINES ///////////////////////////////////////////////////////////////

// W3DDisplayString -----------------------------------------------------------
/** */
//-----------------------------------------------------------------------------
class W3DDisplayString : public DisplayString
{

	MEMORY_POOL_GLUE_WITH_USERLOOKUP_CREATE( W3DDisplayString, "W3DDisplayString" )

public:

	friend W3DDisplayStringManager;

	W3DDisplayString( void );
	// ~W3DDisplayString( void );  // destructor defined by memory pool

	void notifyTextChanged( void );							///< called when text contents change
	void draw( Int x, Int y, Color color, Color dropColor );  ///< render text
	void draw( Int x, Int y, Color color, Color dropColor, Int xDrop, Int yDrop );  ///< render text with the drop shadow being at the offsets passed in
	void getSize( Int *width, Int *height );		///< get render size
	Int	getWidth( Int charPos = -1);
	void setWordWrap( Int wordWrap );						///< set the word wrap width
	void setWordWrapCentered( Bool isCentered ); ///< If this is set to true, the text on a new line is centered
	void setFont( GameFont *font );							///< set a font for display
	void setUseHotkey( Bool useHotkey, Color hotKeyColor = 0xffffffff );
	void setClipRegion( IRegion2D *region );		///< clip text in this region

protected:

	void checkForChangedTextData( void );  /**< called when we need to update our
																				 render sentence and update extents */
	void usingResources( UnsignedInt frame );  /**< call this whenever display
																						 resources are in use */
	void computeExtents( void );  ///< compupte text width and height

	Render2DSentenceClass m_textRenderer;  ///< for drawing text
	Render2DSentenceClass m_textRendererHotKey;  ///< for drawing text
	Bool m_textChanged;  ///< when contents of string change this is TRUE
	Bool m_fontChanged;  ///< when font has chagned this is TRUE
	UnicodeString m_hotkey;		///< holds the current hotkey marker.
	Bool m_useHotKey;
	ICoord2D m_hotKeyPos;
	Color m_hotKeyColor;
	ICoord2D m_textPos;  ///< current text pos set in text renderer
	Color m_currTextColor,  ///< current color used in text renderer
				m_currDropColor;  ///< current color used for shadow in text
	ICoord2D m_size;				///< (width,height) size of rendered text
	IRegion2D m_clipRegion; ///< the clipping region for text
	UnsignedInt m_lastResourceFrame;  ///< last frame resources were used on

};

///////////////////////////////////////////////////////////////////////////////
// INLINING ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
inline void W3DDisplayString::usingResources( UnsignedInt frame ) { m_lastResourceFrame = frame; }

// EXTERNALS //////////////////////////////////////////////////////////////////

#endif // __W3DDISPLAYSTRING_H_

