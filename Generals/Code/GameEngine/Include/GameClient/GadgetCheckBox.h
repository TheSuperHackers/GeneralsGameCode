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

// FILE: GadgetCheckBox.h /////////////////////////////////////////////////////
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
// File name:  GadgetCheckBox.h
//
// Created:    Colin Day, June 2001
//
// Desc:       Helpful interface for checkboxes
//
// CheckBox IMAGE/COLOR organization
// When control is enabled:
//   enabledDrawData[ 0 ] is the background image for the whole enabled control
//   enabledDrawData[ 1 ] is the enabled, unselected check box
//   enabledDrawData[ 2 ] is the enabled, selected check box
//
// When control is disabled:
//   disabledDrawData[ 0 ] is the background image for the whole disabled control
//   disabledDrawData[ 1 ] is the disabled, unselected check box
//   disabledDrawData[ 2 ] si the disabled, selected check box
//
// When control is hilited (mouse over it and enabled)
//   hiliteDrawData[ 0 ] is the background image for the whole hilited control
//   hiliteDrawData[ 1 ] is the hilited, unselected check box
//   hiliteDrawData[ 2 ] is the hilited, selected check box
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/GameWindow.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////

// TYPE DEFINES ///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// INLINING ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

extern void GadgetCheckBoxSetText( GameWindow *g, UnicodeString text );
extern Bool GadgetCheckBoxIsChecked( GameWindow *g );
extern void GadgetCheckBoxSetChecked( GameWindow *g, Bool isChecked);
extern void GadgetCheckBoxToggle( GameWindow *g);

inline void GadgetCheckBoxSetEnabledImage( GameWindow *g, const Image *image )						{ g->winSetEnabledImage( 0, image ); }
inline void GadgetCheckBoxSetEnabledColor( GameWindow *g, Color color )								{ g->winSetEnabledColor( 0, color ); }
inline void GadgetCheckBoxSetEnabledBorderColor( GameWindow *g, Color color )					{ g->winSetEnabledBorderColor( 0, color ); }
inline void GadgetCheckBoxSetEnabledUncheckedBoxImage( GameWindow *g, const Image *image )		{ g->winSetEnabledImage( 1, image ); }
inline void GadgetCheckBoxSetEnabledUncheckedBoxColor( GameWindow *g, Color color )				{ g->winSetEnabledColor( 1, color ); }
inline void GadgetCheckBoxSetEnabledUncheckedBoxBorderColor( GameWindow *g, Color color )	{ g->winSetEnabledBorderColor( 1, color ); }
inline void GadgetCheckBoxSetEnabledCheckedBoxImage( GameWindow *g, const Image *image )			{ g->winSetEnabledImage( 2, image ); }
inline void GadgetCheckBoxSetEnabledCheckedBoxColor( GameWindow *g, Color color )					{ g->winSetEnabledColor( 2, color ); }
inline void GadgetCheckBoxSetEnabledCheckedBoxBorderColor( GameWindow *g, Color color )		{ g->winSetEnabledBorderColor( 2, color ); }
inline const Image *GadgetCheckBoxGetEnabledImage( GameWindow *g )									{ return g->winGetEnabledImage( 0 ); }
inline Color		 GadgetCheckBoxGetEnabledColor( GameWindow *g )									{ return g->winGetEnabledColor( 0 ); }
inline Color		 GadgetCheckBoxGetEnabledBorderColor( GameWindow *g )						{ return g->winGetEnabledBorderColor( 0 ); }
inline const Image *GadgetCheckBoxGetEnabledUncheckedBoxImage( GameWindow *g )					{ return g->winGetEnabledImage( 1 ); }
inline Color		 GadgetCheckBoxGetEnabledUncheckedBoxColor( GameWindow *g )					{ return g->winGetEnabledColor( 1 ); }
inline Color		 GadgetCheckBoxGetEnabledUncheckedBoxBorderColor( GameWindow *g )		{ return g->winGetEnabledBorderColor( 1 ); }
inline const Image *GadgetCheckBoxGetEnabledCheckedBoxImage( GameWindow *g )						{ return g->winGetEnabledImage( 2 ); }
inline Color		 GadgetCheckBoxGetEnabledCheckedBoxColor( GameWindow *g )						{ return g->winGetEnabledColor( 2 ); }
inline Color		 GadgetCheckBoxGetEnabledCheckedBoxBorderColor( GameWindow *g )			{ return g->winGetEnabledBorderColor( 2 ); }

inline void GadgetCheckBoxSetDisabledImage( GameWindow *g, const Image *image )						{ g->winSetDisabledImage( 0, image ); }
inline void GadgetCheckBoxSetDisabledColor( GameWindow *g, Color color )								{ g->winSetDisabledColor( 0, color ); }
inline void GadgetCheckBoxSetDisabledBorderColor( GameWindow *g, Color color )					{ g->winSetDisabledBorderColor( 0, color ); }
inline void GadgetCheckBoxSetDisabledUncheckedBoxImage( GameWindow *g, const Image *image )		{ g->winSetDisabledImage( 1, image ); }
inline void GadgetCheckBoxSetDisabledUncheckedBoxColor( GameWindow *g, Color color )				{ g->winSetDisabledColor( 1, color ); }
inline void GadgetCheckBoxSetDisabledUncheckedBoxBorderColor( GameWindow *g, Color color )	{ g->winSetDisabledBorderColor( 1, color ); }
inline void GadgetCheckBoxSetDisabledCheckedBoxImage( GameWindow *g, const Image *image )			{ g->winSetDisabledImage( 2, image ); }
inline void GadgetCheckBoxSetDisabledCheckedBoxColor( GameWindow *g, Color color )					{ g->winSetDisabledColor( 2, color ); }
inline void GadgetCheckBoxSetDisabledCheckedBoxBorderColor( GameWindow *g, Color color )		{ g->winSetDisabledBorderColor( 2, color ); }
inline const Image *GadgetCheckBoxGetDisabledImage( GameWindow *g )									{ return g->winGetDisabledImage( 0 ); }
inline Color		 GadgetCheckBoxGetDisabledColor( GameWindow *g )									{ return g->winGetDisabledColor( 0 ); }
inline Color		 GadgetCheckBoxGetDisabledBorderColor( GameWindow *g )						{ return g->winGetDisabledBorderColor( 0 ); }
inline const Image *GadgetCheckBoxGetDisabledUncheckedBoxImage( GameWindow *g )					{ return g->winGetDisabledImage( 1 ); }
inline Color		 GadgetCheckBoxGetDisabledUncheckedBoxColor( GameWindow *g )					{ return g->winGetDisabledColor( 1 ); }
inline Color		 GadgetCheckBoxGetDisabledUncheckedBoxBorderColor( GameWindow *g )		{ return g->winGetDisabledBorderColor( 1 ); }
inline const Image *GadgetCheckBoxGetDisabledCheckedBoxImage( GameWindow *g )						{ return g->winGetDisabledImage( 2 ); }
inline Color		 GadgetCheckBoxGetDisabledCheckedBoxColor( GameWindow *g )						{ return g->winGetDisabledColor( 2 ); }
inline Color		 GadgetCheckBoxGetDisabledCheckedBoxBorderColor( GameWindow *g )			{ return g->winGetDisabledBorderColor( 2 ); }

inline void GadgetCheckBoxSetHiliteImage( GameWindow *g, const Image *image )						{ g->winSetHiliteImage( 0, image ); }
inline void GadgetCheckBoxSetHiliteColor( GameWindow *g, Color color )								{ g->winSetHiliteColor( 0, color ); }
inline void GadgetCheckBoxSetHiliteBorderColor( GameWindow *g, Color color )					{ g->winSetHiliteBorderColor( 0, color ); }
inline void GadgetCheckBoxSetHiliteUncheckedBoxImage( GameWindow *g, const Image *image )		{ g->winSetHiliteImage( 1, image ); }
inline void GadgetCheckBoxSetHiliteUncheckedBoxColor( GameWindow *g, Color color )				{ g->winSetHiliteColor( 1, color ); }
inline void GadgetCheckBoxSetHiliteUncheckedBoxBorderColor( GameWindow *g, Color color )	{ g->winSetHiliteBorderColor( 1, color ); }
inline void GadgetCheckBoxSetHiliteCheckedBoxImage( GameWindow *g, const Image *image )			{ g->winSetHiliteImage( 2, image ); }
inline void GadgetCheckBoxSetHiliteCheckedBoxColor( GameWindow *g, Color color )					{ g->winSetHiliteColor( 2, color ); }
inline void GadgetCheckBoxSetHiliteCheckedBoxBorderColor( GameWindow *g, Color color )		{ g->winSetHiliteBorderColor( 2, color ); }
inline const Image *GadgetCheckBoxGetHiliteImage( GameWindow *g )									{ return g->winGetHiliteImage( 0 ); }
inline Color		 GadgetCheckBoxGetHiliteColor( GameWindow *g )									{ return g->winGetHiliteColor( 0 ); }
inline Color		 GadgetCheckBoxGetHiliteBorderColor( GameWindow *g )						{ return g->winGetHiliteBorderColor( 0 ); }
inline const Image *GadgetCheckBoxGetHiliteUncheckedBoxImage( GameWindow *g )					{ return g->winGetHiliteImage( 1 ); }
inline Color		 GadgetCheckBoxGetHiliteUncheckedBoxColor( GameWindow *g )					{ return g->winGetHiliteColor( 1 ); }
inline Color		 GadgetCheckBoxGetHiliteUncheckedBoxBorderColor( GameWindow *g )		{ return g->winGetHiliteBorderColor( 1 ); }
inline const Image *GadgetCheckBoxGetHiliteCheckedBoxImage( GameWindow *g )						{ return g->winGetHiliteImage( 2 ); }
inline Color		 GadgetCheckBoxGetHiliteCheckedBoxColor( GameWindow *g )						{ return g->winGetHiliteColor( 2 ); }
inline Color		 GadgetCheckBoxGetHiliteCheckedBoxBorderColor( GameWindow *g )			{ return g->winGetHiliteBorderColor( 2 ); }

// EXTERNALS //////////////////////////////////////////////////////////////////
