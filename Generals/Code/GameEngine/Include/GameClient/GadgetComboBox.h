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

// FILE: GadgetComboBox.h //////////////////////////////////////////////////////
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
// File name:  GadgetComboBox.h
//
// Created:    Chris Huybregts, November 2001
//
// Desc:       Helpful interface for ComboBoxes
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/Gadget.h"
#include "GameClient/GameWindow.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////

// TYPE DEFINES ///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// INLINING ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

extern void GadgetComboBoxSetFont( GameWindow *comboBox, GameFont *font );
extern UnicodeString GadgetComboBoxGetText( GameWindow *comboBox );
extern void GadgetComboBoxSetText( GameWindow *comboBox, UnicodeString text );
extern Int  GadgetComboBoxAddEntry( GameWindow *comboBox, UnicodeString text, Color color );
extern void GadgetComboBoxReset( GameWindow *comboBox );

extern void GadgetComboBoxSetSelectedPos( GameWindow *comboBox, Int selectedIndex, Bool dontHide = FALSE );
extern void GadgetComboBoxGetSelectedPos( GameWindow *comboBox, Int *selectedIndex );
extern void GadgetComboBoxSetItemData( GameWindow *comboBox, Int index, void *data );
extern void *GadgetComboBoxGetItemData( GameWindow *comboBox, Int index );
extern Int  GadgetComboBoxGetLength( GameWindow *comboBox );

extern void GadgetComboBoxHideList( GameWindow *comboBox );
// Functions that set the ComboBoxData Paramaters
extern void GadgetComboBoxSetAsciiOnly(GameWindow *comboBox, Bool isAsciiOnly  );
extern void GadgetComboBoxSetLettersAndNumbersOnly(GameWindow *comboBox, Bool isLettersAndNumbersOnly  );
extern void GadgetComboBoxSetMaxChars( GameWindow *comboBox, Int maxChars );
extern void GadgetComboBoxSetMaxDisplay( GameWindow *comboBox, Int maxDisplay );
extern void GadgetComboBoxSetIsEditable(GameWindow *comboBox, Bool isEditable  );

//setup all the Font Colors
extern void GadgetComboBoxSetEnabledTextColors( GameWindow *comboBox,Color color, Color borderColor );
extern void GadgetComboBoxSetDisabledTextColors( GameWindow *comboBox,Color color, Color borderColor );
extern void GadgetComboBoxSetHiliteTextColors( GameWindow *comboBox,Color color, Color borderColor );
extern void GadgetComboBoxSetIMECompositeTextColors( GameWindow *comboBox, Color color, Color borderColor );

//
// you can use this to set the colors for the list box all at once, note that
// it will also automatically change the colors for any attached slider
// and those slider buttons and thumb as well as the drop down button and edit box.
//
extern void GadgetComboBoxSetColors( GameWindow *comboBox,
																	  Color enabledColor,
																	  Color enabledBorderColor,
																	  Color enabledSelectedItemColor,
																	  Color enabledSelectedItemBorderColor,
																	  Color disabledColor,
																	  Color disabledBorderColor,
																	  Color disabledSelectedItemColor,
																	  Color disabledSelectedItemBorderColor,
																	  Color hiliteColor,
																	  Color hiliteBorderColor,
																	  Color hiliteSelectedItemColor,
																	  Color hiliteSelectedItemBorderColor );

inline void GadgetComboBoxSetEnabledImage( GameWindow *g, const Image *image )													{ g->winSetEnabledImage( 0, image ); }
inline void GadgetComboBoxSetEnabledColor( GameWindow *g, Color color )													{ g->winSetEnabledColor( 0, color ); }
inline void GadgetComboBoxSetEnabledBorderColor( GameWindow *g, Color color )										{ g->winSetEnabledBorderColor( 0, color ); }
inline void GadgetComboBoxSetEnabledSelectedItemImageLeft( GameWindow *g, const Image *image )					{ g->winSetEnabledImage( 1, image ); }
inline void GadgetComboBoxSetEnabledSelectedItemImageRight( GameWindow *g, const Image *image )				{ g->winSetEnabledImage( 2, image ); }
inline void GadgetComboBoxSetEnabledSelectedItemImageCenter( GameWindow *g, const Image *image )				{ g->winSetEnabledImage( 3, image ); }
inline void GadgetComboBoxSetEnabledSelectedItemImageSmallCenter( GameWindow *g, const Image *image )	{ g->winSetEnabledImage( 4, image ); }
inline void GadgetComboBoxSetEnabledSelectedItemColor( GameWindow *g, Color color )							{ g->winSetEnabledColor( 1, color ); }
inline void GadgetComboBoxSetEnabledSelectedItemBorderColor( GameWindow *g, Color color )				{ g->winSetEnabledBorderColor( 1, color ); }
inline const Image *GadgetComboBoxGetEnabledImage( GameWindow *g )												{ return g->winGetEnabledImage( 0 ); }
inline Color  GadgetComboBoxGetEnabledColor( GameWindow *g )												{ return g->winGetEnabledColor( 0 ); }
inline Color  GadgetComboBoxGetEnabledBorderColor( GameWindow *g )									{ return g->winGetEnabledBorderColor( 0 ); }
inline const Image *GadgetComboBoxGetEnabledSelectedItemImageLeft( GameWindow *g )				{ return g->winGetEnabledImage( 1 ); }
inline const Image *GadgetComboBoxGetEnabledSelectedItemImageRight( GameWindow *g )			{ return g->winGetEnabledImage( 2 ); }
inline const Image *GadgetComboBoxGetEnabledSelectedItemImageCenter( GameWindow *g )			{ return g->winGetEnabledImage( 3 ); }
inline const Image *GadgetComboBoxGetEnabledSelectedItemImageSmallCenter( GameWindow *g ){ return g->winGetEnabledImage( 4 ); }
inline Color	GadgetComboBoxGetEnabledSelectedItemColor( GameWindow *g )						{ return g->winGetEnabledColor( 1 ); }
inline Color	GadgetComboBoxGetEnabledSelectedItemBorderColor( GameWindow *g )			{ return g->winGetEnabledBorderColor( 1 ); }

inline void GadgetComboBoxSetDisabledImage( GameWindow *g, const Image *image )												{ g->winSetDisabledImage( 0, image ); }
inline void GadgetComboBoxSetDisabledColor( GameWindow *g, Color color )													{ g->winSetDisabledColor( 0, color ); }
inline void GadgetComboBoxSetDisabledBorderColor( GameWindow *g, Color color )										{ g->winSetDisabledBorderColor( 0, color ); }
inline void GadgetComboBoxSetDisabledSelectedItemImageLeft( GameWindow *g, const Image *image )				{ g->winSetDisabledImage( 1, image ); }
inline void GadgetComboBoxSetDisabledSelectedItemImageRight( GameWindow *g, const Image *image )				{ g->winSetDisabledImage( 2, image ); }
inline void GadgetComboBoxSetDisabledSelectedItemImageCenter( GameWindow *g, const Image *image )			{ g->winSetDisabledImage( 3, image ); }
inline void GadgetComboBoxSetDisabledSelectedItemImageSmallCenter( GameWindow *g, const Image *image )	{ g->winSetDisabledImage( 4, image ); }
inline void GadgetComboBoxSetDisabledSelectedItemColor( GameWindow *g, Color color )							{ g->winSetDisabledColor( 1, color ); }
inline void GadgetComboBoxSetDisabledSelectedItemBorderColor( GameWindow *g, Color color )				{ g->winSetDisabledBorderColor( 1, color ); }
inline const Image *GadgetComboBoxGetDisabledImage( GameWindow *g )												{ return g->winGetDisabledImage( 0 ); }
inline Color	GadgetComboBoxGetDisabledColor( GameWindow *g )												{ return g->winGetDisabledColor( 0 ); }
inline Color	GadgetComboBoxGetDisabledBorderColor( GameWindow *g )									{ return g->winGetDisabledBorderColor( 0 ); }
inline const Image *GadgetComboBoxGetDisabledSelectedItemImageLeft( GameWindow *g )				{ return g->winGetDisabledImage( 1 ); }
inline const Image *GadgetComboBoxGetDisabledSelectedItemImageRight( GameWindow *g )				{ return g->winGetDisabledImage( 2 ); }
inline const Image *GadgetComboBoxGetDisabledSelectedItemImageCenter( GameWindow *g )			{ return g->winGetDisabledImage( 3 ); }
inline const Image *GadgetComboBoxGetDisabledSelectedItemImageSmallCenter( GameWindow *g	)	{ return g->winGetDisabledImage( 4 ); }
inline Color	GadgetComboBoxGetDisabledSelectedItemColor( GameWindow *g )						{ return g->winGetDisabledColor( 1 ); }
inline Color	GadgetComboBoxGetDisabledSelectedItemBorderColor( GameWindow *g )			{ return g->winGetDisabledBorderColor( 1 ); }

inline void GadgetComboBoxSetHiliteImage( GameWindow *g, const Image *image )												{ g->winSetHiliteImage( 0, image ); }
inline void GadgetComboBoxSetHiliteColor( GameWindow *g, Color color )													{ g->winSetHiliteColor( 0, color ); }
inline void GadgetComboBoxSetHiliteBorderColor( GameWindow *g, Color color )										{ g->winSetHiliteBorderColor( 0, color ); }
inline void GadgetComboBoxSetHiliteSelectedItemImageLeft( GameWindow *g, const Image *image )				{ g->winSetHiliteImage( 1, image ); }
inline void GadgetComboBoxSetHiliteSelectedItemImageRight( GameWindow *g, const Image *image )				{ g->winSetHiliteImage( 2, image ); }
inline void GadgetComboBoxSetHiliteSelectedItemImageCenter( GameWindow *g, const Image *image )			{ g->winSetHiliteImage( 3, image ); }
inline void GadgetComboBoxSetHiliteSelectedItemImageSmallCenter( GameWindow *g, const Image *image )	{ g->winSetHiliteImage( 4, image ); }
inline void GadgetComboBoxSetHiliteSelectedItemColor( GameWindow *g, Color color )							{ g->winSetHiliteColor( 1, color ); }
inline void GadgetComboBoxSetHiliteSelectedItemBorderColor( GameWindow *g, Color color )				{ g->winSetHiliteBorderColor( 1, color ); }
inline const Image *GadgetComboBoxGetHiliteImage( GameWindow *g )												{ return g->winGetHiliteImage( 0 ); }
inline Color	GadgetComboBoxGetHiliteColor( GameWindow *g )												{ return g->winGetHiliteColor( 0 ); }
inline Color	GadgetComboBoxGetHiliteBorderColor( GameWindow *g )									{ return g->winGetHiliteBorderColor( 0 ); }
inline const Image *GadgetComboBoxGetHiliteSelectedItemImageLeft( GameWindow *g )				{ return g->winGetHiliteImage( 1 ); }
inline const Image *GadgetComboBoxGetHiliteSelectedItemImageRight( GameWindow *g )				{ return g->winGetHiliteImage( 2 ); }
inline const Image *GadgetComboBoxGetHiliteSelectedItemImageCenter( GameWindow *g )			{ return g->winGetHiliteImage( 3 ); }
inline const Image *GadgetComboBoxGetHiliteSelectedItemImageSmallCenter( GameWindow *g )	{ return g->winGetHiliteImage( 4 ); }
inline Color	GadgetComboBoxGetHiliteSelectedItemColor( GameWindow *g )						{ return g->winGetHiliteColor( 1 ); }
inline Color	GadgetComboBoxGetHiliteSelectedItemBorderColor( GameWindow *g )			{ return g->winGetHiliteBorderColor( 1 ); }

inline GameWindow *GadgetComboBoxGetDropDownButton( GameWindow *g )
{
	ComboBoxData *comboBoxData = (ComboBoxData *)g->winGetUserData();

	if( comboBoxData && comboBoxData->dropDownButton )
		return comboBoxData->dropDownButton;
	return nullptr;
}
inline GameWindow *GadgetComboBoxGetListBox( GameWindow *g )
{
	ComboBoxData *comboBoxData = (ComboBoxData *)g->winGetUserData();

	if( comboBoxData && comboBoxData->listBox)
		return comboBoxData->listBox;
	return nullptr;

}

inline GameWindow *GadgetComboBoxGetEditBox( GameWindow *g )
{
	ComboBoxData *comboBoxData = (ComboBoxData *)g->winGetUserData();

	if( comboBoxData && comboBoxData->editBox)
		return comboBoxData->editBox;
	return nullptr;

}

// EXTERNALS //////////////////////////////////////////////////////////////////
