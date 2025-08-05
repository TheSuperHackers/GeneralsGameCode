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

// FILE: GadgetRadioButton.h //////////////////////////////////////////////////
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
// File name:  GadgetRadioButton.h
//
// Created:    Colin Day, June 2001
//
// Desc:       Helpful interface for RadioButtons
//
// RadioButton IMAGE/COLOR organization
// When control is enabled:
//   enabledDrawData[ 0 ] is the background image for the whole enabled control
//   enabledDrawData[ 1 ] is the enabled, unselected radio box
//   enabledDrawData[ 2 ] is the enabled, selected radio box
//
// When control is disabled:
//   disabledDrawData[ 0 ] is the background image for the whole disabled control
//   disabledDrawData[ 1 ] is the disabled, unselected radio box
//   disabledDrawData[ 2 ] si the disabled, selected radio box
//
// When control is hilited (mouse over it and enabled)
//   hiliteDrawData[ 0 ] is the background image for the whole hilited control
//   hiliteDrawData[ 1 ] is the hilited, unselected radio box
//   hiliteDrawData[ 2 ] is the hilited, selected radio box
//
// GadgetRadioGetEnabledImage // LEFT
// GadgetRadioGetEnabledUncheckedBoxImage // Middle
// GadgetRadioGetEnabledCheckedBoxImage // right
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef __GADGETRADIOBUTTON_H_
#define __GADGETRADIOBUTTON_H_

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/GameWindow.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////

// TYPE DEFINES ///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// INLINING ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

extern void GadgetRadioSetText( GameWindow *g, UnicodeString text );
extern void GadgetRadioSetSelection( GameWindow *g, Bool sendMsg );
extern void GadgetRadioSetGroup( GameWindow *g, Int group, Int screen );

inline void GadgetRadioSetEnabledImage( GameWindow *g, const Image *image )						{ g->winSetEnabledImage( 0, image ); }
inline void GadgetRadioSetEnabledColor( GameWindow *g, Color color )								{ g->winSetEnabledColor( 0, color ); }
inline void GadgetRadioSetEnabledBorderColor( GameWindow *g, Color color )					{ g->winSetEnabledBorderColor( 0, color ); }
inline void GadgetRadioSetEnabledUncheckedBoxImage( GameWindow *g, const Image *image )		{ g->winSetEnabledImage( 1, image ); }
inline void GadgetRadioSetEnabledUncheckedBoxColor( GameWindow *g, Color color )				{ g->winSetEnabledColor( 1, color ); }
inline void GadgetRadioSetEnabledUncheckedBoxBorderColor( GameWindow *g, Color color )	{ g->winSetEnabledBorderColor( 1, color ); }
inline void GadgetRadioSetEnabledCheckedBoxImage( GameWindow *g, const Image *image )			{ g->winSetEnabledImage( 2, image ); }
inline void GadgetRadioSetEnabledCheckedBoxColor( GameWindow *g, Color color )					{ g->winSetEnabledColor( 2, color ); }
inline void GadgetRadioSetEnabledCheckedBoxBorderColor( GameWindow *g, Color color )		{ g->winSetEnabledBorderColor( 2, color ); }
inline const Image *GadgetRadioGetEnabledImage( GameWindow *g )									{ return g->winGetEnabledImage( 0 ); }
inline Color		 GadgetRadioGetEnabledColor( GameWindow *g )									{ return g->winGetEnabledColor( 0 ); }
inline Color		 GadgetRadioGetEnabledBorderColor( GameWindow *g )						{ return g->winGetEnabledBorderColor( 0 ); }
inline const Image *GadgetRadioGetEnabledUncheckedBoxImage( GameWindow *g )					{ return g->winGetEnabledImage( 1 ); }
inline Color		 GadgetRadioGetEnabledUncheckedBoxColor( GameWindow *g )					{ return g->winGetEnabledColor( 1 ); }
inline Color		 GadgetRadioGetEnabledUncheckedBoxBorderColor( GameWindow *g )		{ return g->winGetEnabledBorderColor( 1 ); }
inline const Image *GadgetRadioGetEnabledCheckedBoxImage( GameWindow *g )						{ return g->winGetEnabledImage( 2 ); }
inline Color		 GadgetRadioGetEnabledCheckedBoxColor( GameWindow *g )						{ return g->winGetEnabledColor( 2 ); }
inline Color		 GadgetRadioGetEnabledCheckedBoxBorderColor( GameWindow *g )			{ return g->winGetEnabledBorderColor( 2 ); }

inline void GadgetRadioSetDisabledImage( GameWindow *g, const Image *image )						{ g->winSetDisabledImage( 0, image ); }
inline void GadgetRadioSetDisabledColor( GameWindow *g, Color color )								{ g->winSetDisabledColor( 0, color ); }
inline void GadgetRadioSetDisabledBorderColor( GameWindow *g, Color color )					{ g->winSetDisabledBorderColor( 0, color ); }
inline void GadgetRadioSetDisabledUncheckedBoxImage( GameWindow *g, const Image *image )		{ g->winSetDisabledImage( 1, image ); }
inline void GadgetRadioSetDisabledUncheckedBoxColor( GameWindow *g, Color color )				{ g->winSetDisabledColor( 1, color ); }
inline void GadgetRadioSetDisabledUncheckedBoxBorderColor( GameWindow *g, Color color )	{ g->winSetDisabledBorderColor( 1, color ); }
inline void GadgetRadioSetDisabledCheckedBoxImage( GameWindow *g, const Image *image )			{ g->winSetDisabledImage( 2, image ); }
inline void GadgetRadioSetDisabledCheckedBoxColor( GameWindow *g, Color color )					{ g->winSetDisabledColor( 2, color ); }
inline void GadgetRadioSetDisabledCheckedBoxBorderColor( GameWindow *g, Color color )		{ g->winSetDisabledBorderColor( 2, color ); }
inline const Image *GadgetRadioGetDisabledImage( GameWindow *g )									{ return g->winGetDisabledImage( 0 ); }
inline Color		 GadgetRadioGetDisabledColor( GameWindow *g )									{ return g->winGetDisabledColor( 0 ); }
inline Color		 GadgetRadioGetDisabledBorderColor( GameWindow *g )						{ return g->winGetDisabledBorderColor( 0 ); }
inline const Image *GadgetRadioGetDisabledUncheckedBoxImage( GameWindow *g )					{ return g->winGetDisabledImage( 1 ); }
inline Color		 GadgetRadioGetDisabledUncheckedBoxColor( GameWindow *g )					{ return g->winGetDisabledColor( 1 ); }
inline Color		 GadgetRadioGetDisabledUncheckedBoxBorderColor( GameWindow *g )		{ return g->winGetDisabledBorderColor( 1 ); }
inline const Image *GadgetRadioGetDisabledCheckedBoxImage( GameWindow *g )						{ return g->winGetDisabledImage( 2 ); }
inline Color		 GadgetRadioGetDisabledCheckedBoxColor( GameWindow *g )						{ return g->winGetDisabledColor( 2 ); }
inline Color		 GadgetRadioGetDisabledCheckedBoxBorderColor( GameWindow *g )			{ return g->winGetDisabledBorderColor( 2 ); }

inline void GadgetRadioSetHiliteImage( GameWindow *g, const Image *image )						{ g->winSetHiliteImage( 0, image ); }
inline void GadgetRadioSetHiliteColor( GameWindow *g, Color color )								{ g->winSetHiliteColor( 0, color ); }
inline void GadgetRadioSetHiliteBorderColor( GameWindow *g, Color color )					{ g->winSetHiliteBorderColor( 0, color ); }
inline void GadgetRadioSetHiliteUncheckedBoxImage( GameWindow *g, const Image *image )		{ g->winSetHiliteImage( 1, image ); }
inline void GadgetRadioSetHiliteUncheckedBoxColor( GameWindow *g, Color color )				{ g->winSetHiliteColor( 1, color ); }
inline void GadgetRadioSetHiliteUncheckedBoxBorderColor( GameWindow *g, Color color )	{ g->winSetHiliteBorderColor( 1, color ); }
inline void GadgetRadioSetHiliteCheckedBoxImage( GameWindow *g, const Image *image )			{ g->winSetHiliteImage( 2, image ); }
inline void GadgetRadioSetHiliteCheckedBoxColor( GameWindow *g, Color color )					{ g->winSetHiliteColor( 2, color ); }
inline void GadgetRadioSetHiliteCheckedBoxBorderColor( GameWindow *g, Color color )		{ g->winSetHiliteBorderColor( 2, color ); }
inline const Image *GadgetRadioGetHiliteImage( GameWindow *g )									{ return g->winGetHiliteImage( 0 ); }
inline Color		 GadgetRadioGetHiliteColor( GameWindow *g )									{ return g->winGetHiliteColor( 0 ); }
inline Color		 GadgetRadioGetHiliteBorderColor( GameWindow *g )						{ return g->winGetHiliteBorderColor( 0 ); }
inline const Image *GadgetRadioGetHiliteUncheckedBoxImage( GameWindow *g )					{ return g->winGetHiliteImage( 1 ); }
inline Color		 GadgetRadioGetHiliteUncheckedBoxColor( GameWindow *g )					{ return g->winGetHiliteColor( 1 ); }
inline Color		 GadgetRadioGetHiliteUncheckedBoxBorderColor( GameWindow *g )		{ return g->winGetHiliteBorderColor( 1 ); }
inline const Image *GadgetRadioGetHiliteCheckedBoxImage( GameWindow *g )						{ return g->winGetHiliteImage( 2 ); }
inline Color		 GadgetRadioGetHiliteCheckedBoxColor( GameWindow *g )						{ return g->winGetHiliteColor( 2 ); }
inline Color		 GadgetRadioGetHiliteCheckedBoxBorderColor( GameWindow *g )			{ return g->winGetHiliteBorderColor( 2 ); }

inline const Image *GadgetRadioGetSelectedImage( GameWindow *g )									{ return g->winGetHiliteImage( 3 ); }
inline const Image *GadgetRadioGetSelectedUncheckedBoxImage( GameWindow *g )					{ return g->winGetHiliteImage( 4 ); }
inline const Image *GadgetRadioGetSelectedCheckedBoxImage( GameWindow *g )						{ return g->winGetHiliteImage( 5 ); }


// EXTERNALS //////////////////////////////////////////////////////////////////

#endif // __GADGETRADIOBUTTON_H_

