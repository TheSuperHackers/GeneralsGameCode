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

// FILE: LayoutScheme.h ///////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//
//                       Westwood Studios Pacific.
//
//                       Confidential Information
//                Copyright (C) 2001 - All Rights Reserved
//
//-----------------------------------------------------------------------------
//
// Project:    GUIEdit
//
// File name:  LayoutScheme.h
//
// Created:    Colin Day, August 2001
//
// Desc:       Layout scheme editing and loading
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////
#include <windows.h>

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Lib/BaseType.h"
#include "Properties.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// TYPE DEFINES ///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// LayoutScheme ---------------------------------------------------------------
/** The layout scheme provides a place for default look of newly
	* created controls and windows */
//-----------------------------------------------------------------------------
class LayoutScheme
{

public:

	LayoutScheme();
	~LayoutScheme();

	void init( void );

	void openDialog( void );  ///< open the scheme info and editing dialog

	char *getSchemeFilename( void );  ///< get the scheme filename
	void setSchemeFilename( char *filename );  ///< set the scheme filename

	Bool saveScheme( char *filename );  ///< save the current scheme to file
	Bool loadScheme( char *filename );  ///< load the scheme file

	ImageAndColorInfo *getImageAndColor( StateIdentifier id );
	void storeImageAndColor( StateIdentifier id, const Image *image,
													 Color color, Color borderColor );

	void setFont( GameFont *font );
	GameFont *getFont( void );

	Color getEnabledTextColor( void );
	Color getEnabledTextBorderColor( void );
	Color getDisabledTextColor( void );
	Color getDisabledTextBorderColor( void );
	Color getHiliteTextColor( void );
	Color getHiliteTextBorderColor( void );
	void setEnabledTextColor( Color c );
	void setEnabledTextBorderColor( Color c );
	void setDisabledTextColor( Color c );
	void setDisabledTextBorderColor( Color c );
	void setHiliteTextColor( Color c );
	void setHiliteTextBorderColor( Color c );

	/** apply the image and color info stored in the state identifer tables
	used for "property editing" to all appropriate windows currently
	loaded in the editor */
	void applyPropertyTablesToWindow( GameWindow *root );

protected:

	ImageAndColorInfo *findEntry( StateIdentifier id );

	char m_schemeFilename[ _MAX_PATH ];  ///< filename

	ImageAndColorInfo m_imageAndColorTable[ NUM_STATE_IDENTIFIERS ];  // the color and image info
	TextDrawData m_enabledText;  ///< default text colors
	TextDrawData m_disabledText;  ///< default text colors
	TextDrawData m_hiliteText;  ///< default text colors

	GameFont *m_font;  ///< default font

};

///////////////////////////////////////////////////////////////////////////////
// INLINING ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
inline char *LayoutScheme::getSchemeFilename( void ) { return m_schemeFilename; }
inline void LayoutScheme::setSchemeFilename( char *filename ) { strcpy( m_schemeFilename, filename ); }
inline Color LayoutScheme::getEnabledTextColor( void ) { return m_enabledText.color; }
inline Color LayoutScheme::getEnabledTextBorderColor( void ) { return m_enabledText.borderColor; }
inline Color LayoutScheme::getDisabledTextColor( void ) { return m_disabledText.color; }
inline Color LayoutScheme::getDisabledTextBorderColor( void ) { return m_disabledText.borderColor; }
inline Color LayoutScheme::getHiliteTextColor( void ) { return m_hiliteText.color; }
inline Color LayoutScheme::getHiliteTextBorderColor( void ) { return m_hiliteText.borderColor; }
inline void LayoutScheme::setEnabledTextColor( Color c ) { m_enabledText.color = c; }
inline void LayoutScheme::setEnabledTextBorderColor( Color c ) { m_enabledText.borderColor = c; }
inline void LayoutScheme::setDisabledTextColor( Color c ) { m_disabledText.color = c; }
inline void LayoutScheme::setDisabledTextBorderColor( Color c ) { m_disabledText.borderColor = c; }
inline void LayoutScheme::setHiliteTextColor( Color c ) { m_hiliteText.color = c; }
inline void LayoutScheme::setHiliteTextBorderColor( Color c ) { m_hiliteText.borderColor = c; }
inline void LayoutScheme::setFont( GameFont *font ) { m_font = font; }
inline GameFont *LayoutScheme::getFont( void ) { return m_font; }

///////////////////////////////////////////////////////////////////////////////
// EXTERNALS //////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
extern LayoutScheme *TheDefaultScheme;
