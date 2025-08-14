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

// FILE: GadgetListBox.h //////////////////////////////////////////////////////
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
// File name:  GadgetListBox.h
//
// Created:    Colin Day, June 2001
//
// Desc:       Helpful interface for ListBoxes
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __GADGETLISTBOX_H_
#define __GADGETLISTBOX_H_

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/Gadget.h"
#include "GameClient/GameWindow.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////

// TYPE DEFINES ///////////////////////////////////////////////////////////////
typedef struct _RightClickStruct
{
	Int mouseX;
	Int mouseY;
	Int pos;
} RightClickStruct;

///////////////////////////////////////////////////////////////////////////////
// INLINING ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
extern Int GadgetListBoxGetEntryBasedOnXY( GameWindow *listbox, Int x, Int y, Int &row, Int &column);
extern void GadgetListboxCreateScrollbar( GameWindow *listbox );
extern void GadgetListBoxAddMultiSelect( GameWindow *listbox );
extern void GadgetListBoxRemoveMultiSelect( GameWindow *listbox );
extern void GadgetListBoxSetListLength( GameWindow *listbox, Int newLength );
extern Int  GadgetListBoxGetListLength( GameWindow *listbox );
extern Int  GadgetListBoxGetNumEntries( GameWindow *listbox );
extern Int  GadgetListBoxGetNumColumns( GameWindow *listbox );
extern Int  GadgetListBoxGetColumnWidth( GameWindow *listbox, Int column );

extern void GadgetListBoxSetFont( GameWindow *listbox, GameFont *font );
extern UnicodeString GadgetListBoxGetText( GameWindow *listbox, Int row, Int column = 0);
extern UnicodeString GadgetListBoxGetTextAndColor( GameWindow *listbox, Color *color, Int row, Int column = 0);
extern Int GadgetListBoxAddEntryText( GameWindow *listbox,	UnicodeString text,	Color color, Int row, Int column = -1, Bool overwrite = TRUE);
extern Int GadgetListBoxAddEntryImage( GameWindow *listbox, const Image *image,
																			Int row, Int column = -1,
																			Bool overwrite = TRUE, Color color = 0xFFFFFFFF );
extern Int GadgetListBoxAddEntryImage( GameWindow *listbox, const Image *image,
																			Int row, Int column,
																			Int hight, Int width,
																			Bool overwrite = TRUE, Color color = 0xFFFFFFFF );
extern void GadgetListBoxSetSelected( GameWindow *listbox, Int selectIndex );
extern void GadgetListBoxSetSelected( GameWindow *listbox, const Int *selectList, Int selectCount = 1 );
extern void GadgetListBoxGetSelected( GameWindow *listbox, Int *selectList );
extern void GadgetListBoxReset( GameWindow *listbox );
extern void GadgetListBoxSetItemData( GameWindow *listbox,  void *data, Int row, Int column = 0);
extern void *GadgetListBoxGetItemData( GameWindow *listbox, Int row, Int column = 0);

extern bool GadgetListBoxIsFull(GameWindow *window);

extern Int GadgetListBoxGetBottomVisibleEntry( GameWindow *window );
extern void GadgetListBoxSetBottomVisibleEntry( GameWindow *window, Int newPos );

extern Int GadgetListBoxGetTopVisibleEntry( GameWindow *window );
extern void GadgetListBoxSetTopVisibleEntry( GameWindow *window, Int newPos );

extern void GadgetListBoxSetAudioFeedback( GameWindow *listbox, Bool enable );

//
// you can use this to set the colors for the list box all at once, note that
// it will also automatically change the colors for any attached slider
// and those slider buttons and thumb
//
extern void GadgetListBoxSetColors( GameWindow *listbox,
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

inline void GadgetListBoxSetEnabledImage( GameWindow *g, const Image *image )													{ g->winSetEnabledImage( 0, image ); }
inline void GadgetListBoxSetEnabledColor( GameWindow *g, Color color )													{ g->winSetEnabledColor( 0, color ); }
inline void GadgetListBoxSetEnabledBorderColor( GameWindow *g, Color color )										{ g->winSetEnabledBorderColor( 0, color ); }
inline void GadgetListBoxSetEnabledSelectedItemImageLeft( GameWindow *g, const Image *image )					{ g->winSetEnabledImage( 1, image ); }
inline void GadgetListBoxSetEnabledSelectedItemImageRight( GameWindow *g, const Image *image )				{ g->winSetEnabledImage( 2, image ); }
inline void GadgetListBoxSetEnabledSelectedItemImageCenter( GameWindow *g, const Image *image )				{ g->winSetEnabledImage( 3, image ); }
inline void GadgetListBoxSetEnabledSelectedItemImageSmallCenter( GameWindow *g, const Image *image )	{ g->winSetEnabledImage( 4, image ); }
inline void GadgetListBoxSetEnabledSelectedItemColor( GameWindow *g, Color color )							{ g->winSetEnabledColor( 1, color ); }
inline void GadgetListBoxSetEnabledSelectedItemBorderColor( GameWindow *g, Color color )				{ g->winSetEnabledBorderColor( 1, color ); }
inline const Image *GadgetListBoxGetEnabledImage( GameWindow *g )												{ return g->winGetEnabledImage( 0 ); }
inline Color  GadgetListBoxGetEnabledColor( GameWindow *g )												{ return g->winGetEnabledColor( 0 ); }
inline Color  GadgetListBoxGetEnabledBorderColor( GameWindow *g )									{ return g->winGetEnabledBorderColor( 0 ); }
inline const Image *GadgetListBoxGetEnabledSelectedItemImageLeft( GameWindow *g )				{ return g->winGetEnabledImage( 1 ); }
inline const Image *GadgetListBoxGetEnabledSelectedItemImageRight( GameWindow *g )			{ return g->winGetEnabledImage( 2 ); }
inline const Image *GadgetListBoxGetEnabledSelectedItemImageCenter( GameWindow *g )			{ return g->winGetEnabledImage( 3 ); }
inline const Image *GadgetListBoxGetEnabledSelectedItemImageSmallCenter( GameWindow *g ){ return g->winGetEnabledImage( 4 ); }
inline Color	GadgetListBoxGetEnabledSelectedItemColor( GameWindow *g )						{ return g->winGetEnabledColor( 1 ); }
inline Color	GadgetListBoxGetEnabledSelectedItemBorderColor( GameWindow *g )			{ return g->winGetEnabledBorderColor( 1 ); }

inline void GadgetListBoxSetDisabledImage( GameWindow *g, const Image *image )												{ g->winSetDisabledImage( 0, image ); }
inline void GadgetListBoxSetDisabledColor( GameWindow *g, Color color )													{ g->winSetDisabledColor( 0, color ); }
inline void GadgetListBoxSetDisabledBorderColor( GameWindow *g, Color color )										{ g->winSetDisabledBorderColor( 0, color ); }
inline void GadgetListBoxSetDisabledSelectedItemImageLeft( GameWindow *g, const Image *image )				{ g->winSetDisabledImage( 1, image ); }
inline void GadgetListBoxSetDisabledSelectedItemImageRight( GameWindow *g, const Image *image )				{ g->winSetDisabledImage( 2, image ); }
inline void GadgetListBoxSetDisabledSelectedItemImageCenter( GameWindow *g, const Image *image )			{ g->winSetDisabledImage( 3, image ); }
inline void GadgetListBoxSetDisabledSelectedItemImageSmallCenter( GameWindow *g, const Image *image )	{ g->winSetDisabledImage( 4, image ); }
inline void GadgetListBoxSetDisabledSelectedItemColor( GameWindow *g, Color color )							{ g->winSetDisabledColor( 1, color ); }
inline void GadgetListBoxSetDisabledSelectedItemBorderColor( GameWindow *g, Color color )				{ g->winSetDisabledBorderColor( 1, color ); }
inline const Image *GadgetListBoxGetDisabledImage( GameWindow *g )												{ return g->winGetDisabledImage( 0 ); }
inline Color	GadgetListBoxGetDisabledColor( GameWindow *g )												{ return g->winGetDisabledColor( 0 ); }
inline Color	GadgetListBoxGetDisabledBorderColor( GameWindow *g )									{ return g->winGetDisabledBorderColor( 0 ); }
inline const Image *GadgetListBoxGetDisabledSelectedItemImageLeft( GameWindow *g )				{ return g->winGetDisabledImage( 1 ); }
inline const Image *GadgetListBoxGetDisabledSelectedItemImageRight( GameWindow *g )				{ return g->winGetDisabledImage( 2 ); }
inline const Image *GadgetListBoxGetDisabledSelectedItemImageCenter( GameWindow *g )			{ return g->winGetDisabledImage( 3 ); }
inline const Image *GadgetListBoxGetDisabledSelectedItemImageSmallCenter( GameWindow *g	)	{ return g->winGetDisabledImage( 4 ); }
inline Color	GadgetListBoxGetDisabledSelectedItemColor( GameWindow *g )						{ return g->winGetDisabledColor( 1 ); }
inline Color	GadgetListBoxGetDisabledSelectedItemBorderColor( GameWindow *g )			{ return g->winGetDisabledBorderColor( 1 ); }

inline void GadgetListBoxSetHiliteImage( GameWindow *g, const Image *image )												{ g->winSetHiliteImage( 0, image ); }
inline void GadgetListBoxSetHiliteColor( GameWindow *g, Color color )													{ g->winSetHiliteColor( 0, color ); }
inline void GadgetListBoxSetHiliteBorderColor( GameWindow *g, Color color )										{ g->winSetHiliteBorderColor( 0, color ); }
inline void GadgetListBoxSetHiliteSelectedItemImageLeft( GameWindow *g, const Image *image )				{ g->winSetHiliteImage( 1, image ); }
inline void GadgetListBoxSetHiliteSelectedItemImageRight( GameWindow *g, const Image *image )				{ g->winSetHiliteImage( 2, image ); }
inline void GadgetListBoxSetHiliteSelectedItemImageCenter( GameWindow *g, const Image *image )			{ g->winSetHiliteImage( 3, image ); }
inline void GadgetListBoxSetHiliteSelectedItemImageSmallCenter( GameWindow *g, const Image *image )	{ g->winSetHiliteImage( 4, image ); }
inline void GadgetListBoxSetHiliteSelectedItemColor( GameWindow *g, Color color )							{ g->winSetHiliteColor( 1, color ); }
inline void GadgetListBoxSetHiliteSelectedItemBorderColor( GameWindow *g, Color color )				{ g->winSetHiliteBorderColor( 1, color ); }
inline const Image *GadgetListBoxGetHiliteImage( GameWindow *g )												{ return g->winGetHiliteImage( 0 ); }
inline Color	GadgetListBoxGetHiliteColor( GameWindow *g )												{ return g->winGetHiliteColor( 0 ); }
inline Color	GadgetListBoxGetHiliteBorderColor( GameWindow *g )									{ return g->winGetHiliteBorderColor( 0 ); }
inline const Image *GadgetListBoxGetHiliteSelectedItemImageLeft( GameWindow *g )				{ return g->winGetHiliteImage( 1 ); }
inline const Image *GadgetListBoxGetHiliteSelectedItemImageRight( GameWindow *g )				{ return g->winGetHiliteImage( 2 ); }
inline const Image *GadgetListBoxGetHiliteSelectedItemImageCenter( GameWindow *g )			{ return g->winGetHiliteImage( 3 ); }
inline const Image *GadgetListBoxGetHiliteSelectedItemImageSmallCenter( GameWindow *g )	{ return g->winGetHiliteImage( 4 ); }
inline Color	GadgetListBoxGetHiliteSelectedItemColor( GameWindow *g )						{ return g->winGetHiliteColor( 1 ); }
inline Color	GadgetListBoxGetHiliteSelectedItemBorderColor( GameWindow *g )			{ return g->winGetHiliteBorderColor( 1 ); }

inline GameWindow *GadgetListBoxGetSlider( GameWindow *g )
{
	ListboxData *listData = (ListboxData *)g->winGetUserData();

	if( listData && listData->slider )
		return listData->slider;
	return NULL;
}
inline GameWindow *GadgetListBoxGetUpButton( GameWindow *g )
{
	ListboxData *listData = (ListboxData *)g->winGetUserData();

	if( listData && listData->upButton )
		return listData->upButton;
	return NULL;
}
inline GameWindow *GadgetListBoxGetDownButton( GameWindow *g )
{
	ListboxData *listData = (ListboxData *)g->winGetUserData();

	if( listData && listData->downButton )
		return listData->downButton;
	return NULL;

}

// EXTERNALS //////////////////////////////////////////////////////////////////

#endif // __GADGETLISTBOX_H_

