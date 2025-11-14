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

// FILE: WinInstanceData.h ////////////////////////////////////////////////////
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
// File name:  WinInstanceData.h
//
// Created:    Colin Day, July 2001
//
// Desc:       The game window instance dat
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/DisplayString.h"
#include "GameClient/GameFont.h"
#include "GameClient/Image.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////
class GameWindow;
class VideoBuffer;
// TYPE DEFINES ///////////////////////////////////////////////////////////////

#define WIN_STATE_HILITED 	0x00000002	// Mouse is over window or has focus
#define WIN_STATE_SELECTED	0x00000004	// Window has been selected

enum
{
	MAX_WINDOW_NAME_LEN = 64,
	/** how many elements there are for each of the draw states for the inst
		* data of a window, note if you increase this you must update the parsing
		* tables make sure you can parse the new fields from the window scrip
		* files */
	MAX_DRAW_DATA = 9,
	MAX_TEXT_LABEL = 128  ///< max length of text label
};

// WinDrawData ----------------------------------------------------------------
//-----------------------------------------------------------------------------
struct WinDrawData
{

	const Image *image;
	Color color;
	Color borderColor;

};

// TextDrawData ---------------------------------------------------------------
//-----------------------------------------------------------------------------
struct TextDrawData
{
	Color color;  ///< the text color
	Color borderColor;  ///< outline color
};

// WinInstanceData ------------------------------------------------------------
// NOTE if you add data to this make sure you update winSetInstanceData()
// NOTE if you add data to this make sure you update winSetInstanceData()
// NOTE if you add data to this make sure you update winSetInstanceData()
//-----------------------------------------------------------------------------
class WinInstanceData
{

public:

	WinInstanceData( void );  ///< constructor automatically runs init()
	virtual ~WinInstanceData( void );

	void init( void );  ///< initialize default values if desired


	// setting text
	void setTooltipText( UnicodeString tip );  ///< set tooltip text
	void setText( UnicodeString text );  ///< set instance text text

	// a couple of nice access methods
	UnicodeString getTooltipText( void );		///< get tooltip text
	UnicodeString getText( void );					///< get instance text
	Int getTextLength( void );						///< get number of chars in instance text
	Int getTooltipTextLength( void );						///< get number of chars in tooltip text
	UnsignedInt getStyle( void );				///< return window style
	UnsignedInt getStatus( void );			///< return widnow status
	UnsignedInt getState( void );				///< return window state
	GameWindow *getOwner( void );				///< return window owner
	GameFont	 *getFont( void );				///< return window font

	DisplayString *getTextDisplayString( void );			///< return the text display string
	DisplayString *getTooltipDisplayString( void );		///< return the tooltip display string

	void setVideoBuffer( VideoBuffer * videoBuffer );	///< set the videobuffer to display a video frame

// NOTE if you add data to this make sure you update winSetInstanceData()
// NOTE if you add data to this make sure you update winSetInstanceData()
// NOTE if you add data to this make sure you update winSetInstanceData()

	/** @todo you may want to make these data members protected, they are public
	because of the legacy of porting all this code in from Nox, but they
	really should be protected and have the rest of the code use access
	functions to edit them */

  Int m_id;								            // Id of the window (used mainly for scripts)
  Int m_state;                 // Flags indicating state of window
  UnsignedInt m_style;         // Flags indicating style of window
	UnsignedInt m_status;	       // Status bits for this window (mirrored in GameWindow)
  GameWindow *m_owner;

	WinDrawData m_enabledDrawData[ MAX_DRAW_DATA ];  ///< image/color info for enabled state
	WinDrawData m_disabledDrawData[ MAX_DRAW_DATA ];  ///< image/color info for disabled state
	WinDrawData m_hiliteDrawData[ MAX_DRAW_DATA ];  ///< image/color info for hilite state

	TextDrawData m_enabledText;   ///< enabled text colors
	TextDrawData m_disabledText;  ///< disabled text colors
	TextDrawData m_hiliteText;		///< hilite text colors
	TextDrawData m_imeCompositeText;///< IME composite text colors

  ICoord2D	m_imageOffset;		 // dx, dy for blitting bkgnd images

	GameFont *m_font;								 // font which this window should use

	AsciiString m_textLabelString;  ///< text label from window file if present
	AsciiString m_decoratedNameString;  ///< window text name from GUIEdit
	AsciiString m_tooltipString;  ///< tooltip Label from window file if present

	AsciiString m_headerTemplateName;		///< name of the template we're going to base our font off of.

  Int m_tooltipDelay;           ///< desired delay before showing tooltip

	DisplayString *m_text;				 ///< generic text for any window to display
	DisplayString *m_tooltip;		 ///< tooltip for display

	//NOTE Video Buffer cannot be transfered to another window.
	VideoBuffer *m_videoBuffer;			///< Each window can be made to play a video in it.

// NOTE if you add data to this make sure you update winSetInstanceData()
// NOTE if you add data to this make sure you update winSetInstanceData()
// NOTE if you add data to this make sure you update winSetInstanceData()

protected:

};

///////////////////////////////////////////////////////////////////////////////
// INLINING ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
inline UnsignedInt WinInstanceData::getStyle( void ) { return m_style; }
inline UnsignedInt WinInstanceData::getStatus( void ) { return m_status; }
inline UnsignedInt WinInstanceData::getState( void ) { return m_state; }
inline GameWindow *WinInstanceData::getOwner( void ) { return m_owner; }
inline GameFont		*WinInstanceData::getFont( void ) { return m_font; }
inline DisplayString *WinInstanceData::getTextDisplayString( void ) { return m_text; }
inline DisplayString *WinInstanceData::getTooltipDisplayString( void ) { return m_tooltip; }
inline UnicodeString WinInstanceData::getTooltipText( void )
{
	if( m_tooltip )
		return m_tooltip->getText();
	return UnicodeString::TheEmptyString;

}
inline UnicodeString WinInstanceData::getText( void )
{
	if( m_text )
		return m_text->getText();
	return UnicodeString::TheEmptyString;
}
inline Int WinInstanceData::getTextLength( void )
{
	if( m_text )
		return m_text->getTextLength();
	return 0;
}
inline Int WinInstanceData::getTooltipTextLength( void )
{
	if( m_tooltip )
		return m_tooltip->getTextLength();
	return 0;
}

// EXTERNALS //////////////////////////////////////////////////////////////////
