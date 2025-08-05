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

// FILE: GadgetTextEntry.h ////////////////////////////////////////////////////
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
// File name:  GadgetTextEntry.h
//
// Created:    Colin Day, June 2001
//
// Desc:       Helpful interface for TextEntrys
//
// TextEntry IMAGE/COLOR organization
//
// note that windows that have an outlined text field will use the color
// for the outline specified with the TextBorder... functions
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __GADGETTEXTENTRY_H_
#define __GADGETTEXTENTRY_H_

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/GameWindowManager.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////
class GameWindow;

// TYPE DEFINES ///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// INLINING ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

inline void GadgetTextEntrySetText( GameWindow *g, UnicodeString text )
{
	TheWindowManager->winSendSystemMsg( g, GEM_SET_TEXT, (WindowMsgData)&text, 0 );
}
extern UnicodeString GadgetTextEntryGetText( GameWindow *textentry ); ///< Get the text from the text entry field
extern void GadgetTextEntrySetFont( GameWindow *g, GameFont *font );  ///< set font for window and edit text display strings
inline void GadgetTextEntrySetTextColor( GameWindow *g, Color color )
{
	Color back = g->winGetEnabledTextBorderColor();
	g->winSetEnabledTextColors(color,back);
	g->winSetDisabledTextColors(GameDarkenColor(color, 25),back);
}
// text colors

// enabled
inline void GadgetTextEntrySetEnabledImageLeft( GameWindow *g, const Image *image )					{ g->winSetEnabledImage( 0, image ); }
inline void GadgetTextEntrySetEnabledImageRight( GameWindow *g, const Image *image )				{ g->winSetEnabledImage( 1, image ); }
inline void GadgetTextEntrySetEnabledImageCenter( GameWindow *g, const Image *image )				{ g->winSetEnabledImage( 2, image ); }
inline void GadgetTextEntrySetEnabledImageSmallCenter( GameWindow *g, const Image *image )	{ g->winSetEnabledImage( 3, image ); }
inline void GadgetTextEntrySetEnabledColor( GameWindow *g, Color color )				{ g->winSetEnabledColor( 0, color ); }
inline void GadgetTextEntrySetEnabledBorderColor( GameWindow *g, Color color )	{ g->winSetEnabledBorderColor( 0, color ); }
inline const Image *GadgetTextEntryGetEnabledImageLeft( GameWindow *g )					{ return g->winGetEnabledImage( 0 ); }
inline const Image *GadgetTextEntryGetEnabledImageRight( GameWindow *g )				{ return g->winGetEnabledImage( 1 ); }
inline const Image *GadgetTextEntryGetEnabledImageCenter( GameWindow *g )				{ return g->winGetEnabledImage( 2 ); }
inline const Image *GadgetTextEntryGetEnabledImageSmallCenter( GameWindow *g )	{ return g->winGetEnabledImage( 3 ); }
inline Color GadgetTextEntryGetEnabledColor( GameWindow *g )							{ return g->winGetEnabledColor( 0 ); }
inline Color GadgetTextEntryGetEnabledBorderColor( GameWindow *g )				{ return g->winGetEnabledBorderColor( 0 ); }

// disabled
inline void GadgetTextEntrySetDisabledImageLeft( GameWindow *g, const Image *image )				{ g->winSetDisabledImage( 0, image ); }
inline void GadgetTextEntrySetDisabledImageRight( GameWindow *g, const Image *image )				{ g->winSetDisabledImage( 1, image ); }
inline void GadgetTextEntrySetDisabledImageCenter( GameWindow *g, const Image *image )			{ g->winSetDisabledImage( 2, image ); }
inline void GadgetTextEntrySetDisabledImageSmallCenter( GameWindow *g, const Image *image )	{ g->winSetDisabledImage( 3, image ); }
inline void GadgetTextEntrySetDisabledColor( GameWindow *g, Color color )				{ g->winSetDisabledColor( 0, color ); }
inline void GadgetTextEntrySetDisabledBorderColor( GameWindow *g, Color color )	{ g->winSetDisabledBorderColor( 0, color ); }
inline const Image *GadgetTextEntryGetDisabledImageLeft( GameWindow *g )					{ return g->winGetDisabledImage( 0 ); }
inline const Image *GadgetTextEntryGetDisabledImageRight( GameWindow *g )					{ return g->winGetDisabledImage( 1 ); }
inline const Image *GadgetTextEntryGetDisabledImageCenter( GameWindow *g )				{ return g->winGetDisabledImage( 2 ); }
inline const Image *GadgetTextEntryGetDisabledImageSmallCenter( GameWindow *g )		{ return g->winGetDisabledImage( 3 ); }
inline Color GadgetTextEntryGetDisabledColor( GameWindow *g )								{ return g->winGetDisabledColor( 0 ); }
inline Color GadgetTextEntryGetDisabledBorderColor( GameWindow *g )					{ return g->winGetDisabledBorderColor( 0 ); }

// hilite
inline void GadgetTextEntrySetHiliteImageLeft( GameWindow *g, const Image *image )					{ g->winSetHiliteImage( 0, image ); }
inline void GadgetTextEntrySetHiliteImageRight( GameWindow *g, const Image *image )					{ g->winSetHiliteImage( 1, image ); }
inline void GadgetTextEntrySetHiliteImageCenter( GameWindow *g, const Image *image )				{ g->winSetHiliteImage( 2, image ); }
inline void GadgetTextEntrySetHiliteImageSmallCenter( GameWindow *g, const Image *image )		{ g->winSetHiliteImage( 3, image ); }
inline void GadgetTextEntrySetHiliteColor( GameWindow *g, Color color )					{ g->winSetHiliteColor( 0, color ); }
inline void GadgetTextEntrySetHiliteBorderColor( GameWindow *g, Color color )		{ g->winSetHiliteBorderColor( 0, color ); }
inline const Image *GadgetTextEntryGetHiliteImageLeft( GameWindow *g )					{ return g->winGetHiliteImage( 0 ); }
inline const Image *GadgetTextEntryGetHiliteImageRight( GameWindow *g )					{ return g->winGetHiliteImage( 1 ); }
inline const Image *GadgetTextEntryGetHiliteImageCenter( GameWindow *g )				{ return g->winGetHiliteImage( 2 ); }
inline const Image *GadgetTextEntryGetHiliteImageSmallCenter( GameWindow *g )		{ return g->winGetHiliteImage( 3 ); }
inline Color GadgetTextEntryGetHiliteColor( GameWindow *g )								{ return g->winGetHiliteColor( 0 ); }
inline Color GadgetTextEntryGetHiliteBorderColor( GameWindow *g )					{ return g->winGetHiliteBorderColor( 0 ); }

// EXTERNALS //////////////////////////////////////////////////////////////////

#endif // __GADGETTEXTENTRY_H_

