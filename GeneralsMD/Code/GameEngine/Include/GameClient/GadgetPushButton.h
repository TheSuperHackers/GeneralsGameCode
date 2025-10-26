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

// FILE: GadgetPushButton.h ///////////////////////////////////////////////////
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
// File name:  GadgetPushButton.h
//
// Created:    Colin Day, June 2001
//
// Desc:       Helpful interface for PushButtons
//
// PushButton IMAGE/COLOR organization
// When control is enabled:
//   enabledDrawData[ 0 ] is the background image for the whole enabled control
//   enabledDrawData[ 1 ] is the enabled, selected button graphic
//
// When control is disabled:
//   disabledDrawData[ 0 ] is the background image for the whole disabled control
//	 disabledDrawData[ 1 ] is the disabled, but selected button graphic
//
// When control is hilited (mouse over it and enabled)
//   hiliteDrawData[ 0 ] is the background image for the whole hilited control
//   hiliteDrawData[ 1 ] is the hilited, selected button graphic
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

void GadgetCheckLikeButtonSetVisualCheck( GameWindow *g, Bool checked );
Bool GadgetCheckLikeButtonIsChecked( GameWindow *g );
void GadgetButtonEnableCheckLike( GameWindow *g, Bool makeCheckLike, Bool initiallyChecked );

void GadgetButtonSetText( GameWindow *g, UnicodeString text );
void GadgetButtonDrawClock( GameWindow *g, Int percent, Color color ); //Darkens the progress
void GadgetButtonDrawInverseClock( GameWindow *g, Int percent, Color color ); //Darkens the remaining portion.
void GadgetButtonDrawOverlayImage( GameWindow *g, const Image *image );
void GadgetButtonSetBorder( GameWindow *g, Color color, Bool drawBorder = TRUE );
void GadgetButtonSetData(GameWindow *g, void *data);
void *GadgetButtonGetData(GameWindow *g);
void GadgetButtonSetAltSound( GameWindow *g, AsciiString altSound );
inline void GadgetButtonSetEnabledImage( GameWindow *g, const Image *image )					{ g->winSetEnabledImage( 0, image ); g->winSetEnabledImage( 5, NULL );g->winSetEnabledImage( 6, NULL );}
inline void GadgetButtonSetEnabledColor( GameWindow *g, Color color )							{ g->winSetEnabledColor( 0, color ); }
inline void GadgetButtonSetEnabledBorderColor( GameWindow *g, Color color )				{ g->winSetEnabledBorderColor( 0, color ); }
inline void GadgetButtonSetEnabledSelectedImage( GameWindow *g, const Image *image )			{ g->winSetEnabledImage( 1, image ); g->winSetEnabledImage( 2, NULL );g->winSetEnabledImage( 3, NULL );}
inline void GadgetButtonSetEnabledSelectedColor( GameWindow *g, Color color )					{ g->winSetEnabledColor( 1, color ); }
inline void GadgetButtonSetEnabledSelectedBorderColor( GameWindow *g, Color color )		{ g->winSetEnabledBorderColor( 1, color ); }
inline const Image *GadgetButtonGetEnabledImage( GameWindow *g )							{ return g->winGetEnabledImage( 0 ); }
inline Color		 GadgetButtonGetEnabledColor( GameWindow *g )							{ return g->winGetEnabledColor( 0 ); }
inline Color		 GadgetButtonGetEnabledBorderColor( GameWindow *g )				{ return g->winGetEnabledBorderColor( 0 ); }
inline const Image *GadgetButtonGetEnabledSelectedImage( GameWindow *g )					{ return g->winGetEnabledImage( 1 ); }
inline Color		 GadgetButtonGetEnabledSelectedColor( GameWindow *g )					{ return g->winGetEnabledColor( 1 ); }
inline Color		 GadgetButtonGetEnabledSelectedBorderColor( GameWindow *g )		{ return g->winGetEnabledBorderColor( 1 ); }

inline void GadgetButtonSetDisabledImage( GameWindow *g, const Image *image )					{ g->winSetDisabledImage( 0, image ); g->winSetEnabledImage( 5, NULL );g->winSetEnabledImage( 6, NULL );}
inline void GadgetButtonSetDisabledColor( GameWindow *g, Color color )							{ g->winSetDisabledColor( 0, color ); }
inline void GadgetButtonSetDisabledBorderColor( GameWindow *g, Color color )				{ g->winSetDisabledBorderColor( 0, color ); }
inline void GadgetButtonSetDisabledSelectedImage( GameWindow *g, const Image *image )			{ g->winSetDisabledImage( 1, image ); g->winSetEnabledImage( 2, NULL );g->winSetEnabledImage( 3, NULL );}
inline void GadgetButtonSetDisabledSelectedColor( GameWindow *g, Color color )					{ g->winSetDisabledColor( 1, color ); }
inline void GadgetButtonSetDisabledSelectedBorderColor( GameWindow *g, Color color )		{ g->winSetDisabledBorderColor( 1, color ); }
inline const Image *GadgetButtonGetDisabledImage( GameWindow *g )							{ return g->winGetDisabledImage( 0 ); }
inline Color		 GadgetButtonGetDisabledColor( GameWindow *g )							{ return g->winGetDisabledColor( 0 ); }
inline Color		 GadgetButtonGetDisabledBorderColor( GameWindow *g )				{ return g->winGetDisabledBorderColor( 0 ); }
inline const Image *GadgetButtonGetDisabledSelectedImage( GameWindow *g )					{ return g->winGetDisabledImage( 1 ); }
inline Color		 GadgetButtonGetDisabledSelectedColor( GameWindow *g )					{ return g->winGetDisabledColor( 1 ); }
inline Color		 GadgetButtonGetDisabledSelectedBorderColor( GameWindow *g )		{ return g->winGetDisabledBorderColor( 1 ); }

inline void GadgetButtonSetHiliteImage( GameWindow *g, const Image *image )					{ g->winSetHiliteImage( 0, image ); g->winSetEnabledImage( 5, NULL );g->winSetEnabledImage( 6, NULL );}
inline void GadgetButtonSetHiliteColor( GameWindow *g, Color color )							{ g->winSetHiliteColor( 0, color ); }
inline void GadgetButtonSetHiliteBorderColor( GameWindow *g, Color color )				{ g->winSetHiliteBorderColor( 0, color ); }
inline void GadgetButtonSetHiliteSelectedImage( GameWindow *g, const Image *image )			{ g->winSetHiliteImage( 1, image ); g->winSetEnabledImage( 2, NULL );g->winSetEnabledImage( 3, NULL );}
inline void GadgetButtonSetHiliteSelectedColor( GameWindow *g, Color color )					{ g->winSetHiliteColor( 1, color ); }
inline void GadgetButtonSetHiliteSelectedBorderColor( GameWindow *g, Color color )		{ g->winSetHiliteBorderColor( 1, color ); }
inline const Image *GadgetButtonGetHiliteImage( GameWindow *g )							{ return g->winGetHiliteImage( 0 ); }
inline Color		 GadgetButtonGetHiliteColor( GameWindow *g )							{ return g->winGetHiliteColor( 0 ); }
inline Color		 GadgetButtonGetHiliteBorderColor( GameWindow *g )				{ return g->winGetHiliteBorderColor( 0 ); }
inline const Image *GadgetButtonGetHiliteSelectedImage( GameWindow *g )					{ return g->winGetHiliteImage( 1 ); }
inline Color		 GadgetButtonGetHiliteSelectedColor( GameWindow *g )					{ return g->winGetHiliteColor( 1 ); }
inline Color		 GadgetButtonGetHiliteSelectedBorderColor( GameWindow *g )		{ return g->winGetHiliteBorderColor( 1 ); }

inline const Image *GadgetButtonGetLeftHiliteSelectedImage( GameWindow *g )					{ return g->winGetHiliteImage( 1 ); }
inline const Image *GadgetButtonGetMiddleHiliteSelectedImage( GameWindow *g )					{ return g->winGetHiliteImage( 3 ); }
inline const Image *GadgetButtonGetRightHiliteSelectedImage( GameWindow *g )					{ return g->winGetHiliteImage( 4 ); }

inline const Image *GadgetButtonGetLeftHiliteImage( GameWindow *g )							{ return g->winGetHiliteImage( 0 ); }
inline const Image *GadgetButtonGetMiddleHiliteImage( GameWindow *g )							{ return g->winGetHiliteImage( 5 ); }
inline const Image *GadgetButtonGetRightHiliteImage( GameWindow *g )							{ return g->winGetHiliteImage( 6 ); }

inline void GadgetButtonSetLeftHiliteSelectedImage( GameWindow *g, const Image *image )			{ g->winSetHiliteImage( 1, image ); }
inline void GadgetButtonSetMiddleHiliteSelectedImage( GameWindow *g, const Image *image )			{ g->winSetHiliteImage( 3, image ); }
inline void GadgetButtonSetRightHiliteSelectedImage( GameWindow *g, const Image *image )			{ g->winSetHiliteImage( 4, image ); }

inline void GadgetButtonSetLeftHiliteImage( GameWindow *g, const Image *image )					{ g->winSetHiliteImage( 0, image ); }
inline void GadgetButtonSetMiddleHiliteImage( GameWindow *g, const Image *image )					{ g->winSetHiliteImage( 5, image ); }
inline void GadgetButtonSetRightHiliteImage( GameWindow *g, const Image *image )					{ g->winSetHiliteImage( 6, image ); }


inline const Image *GadgetButtonGetLeftDisabledSelectedImage( GameWindow *g )					{ return g->winGetDisabledImage( 1 ); }
inline const Image *GadgetButtonGetMiddleDisabledSelectedImage( GameWindow *g )					{ return g->winGetDisabledImage( 3 ); }
inline const Image *GadgetButtonGetRightDisabledSelectedImage( GameWindow *g )					{ return g->winGetDisabledImage( 4 ); }

inline const Image *GadgetButtonGetLeftDisabledImage( GameWindow *g )							{ return g->winGetDisabledImage( 0 ); }
inline const Image *GadgetButtonGetMiddleDisabledImage( GameWindow *g )							{ return g->winGetDisabledImage( 5 ); }
inline const Image *GadgetButtonGetRightDisabledImage( GameWindow *g )							{ return g->winGetDisabledImage( 6 ); }

inline void GadgetButtonSetLeftDisabledSelectedImage( GameWindow *g, const Image *image )			{ g->winSetDisabledImage( 1, image ); }
inline void GadgetButtonSetMiddleDisabledSelectedImage( GameWindow *g, const Image *image )			{ g->winSetDisabledImage( 3, image ); }
inline void GadgetButtonSetRightDisabledSelectedImage( GameWindow *g, const Image *image )			{ g->winSetDisabledImage( 4, image ); }

inline void GadgetButtonSetLeftDisabledImage( GameWindow *g, const Image *image )					{ g->winSetDisabledImage( 0, image ); }
inline void GadgetButtonSetMiddleDisabledImage( GameWindow *g, const Image *image )					{ g->winSetDisabledImage( 5, image ); }
inline void GadgetButtonSetRightDisabledImage( GameWindow *g, const Image *image )					{ g->winSetDisabledImage( 6, image ); }

inline const Image *GadgetButtonGetLeftEnabledSelectedImage( GameWindow *g )					{ return g->winGetEnabledImage( 1 ); }
inline const Image *GadgetButtonGetMiddleEnabledSelectedImage( GameWindow *g )					{ return g->winGetEnabledImage( 3 ); }
inline const Image *GadgetButtonGetRightEnabledSelectedImage( GameWindow *g )					{ return g->winGetEnabledImage( 4 ); }

inline const Image *GadgetButtonGetLeftEnabledImage( GameWindow *g )							{ return g->winGetEnabledImage( 0 ); }
inline const Image *GadgetButtonGetMiddleEnabledImage( GameWindow *g )							{ return g->winGetEnabledImage( 5 ); }
inline const Image *GadgetButtonGetRightEnabledImage( GameWindow *g )							{ return g->winGetEnabledImage( 6 ); }

inline void GadgetButtonSetLeftEnabledSelectedImage( GameWindow *g, const Image *image )			{ g->winSetEnabledImage( 1, image ); }
inline void GadgetButtonSetMiddleEnabledSelectedImage( GameWindow *g, const Image *image )			{ g->winSetEnabledImage( 3, image ); }
inline void GadgetButtonSetRightEnabledSelectedImage( GameWindow *g, const Image *image )			{ g->winSetEnabledImage( 4, image ); }

inline void GadgetButtonSetLeftEnabledImage( GameWindow *g, const Image *image )					{ g->winSetEnabledImage( 0, image ); }
inline void GadgetButtonSetMiddleEnabledImage( GameWindow *g, const Image *image )					{ g->winSetEnabledImage( 5, image ); }
inline void GadgetButtonSetRightEnabledImage( GameWindow *g, const Image *image )					{ g->winSetEnabledImage( 6, image ); }
// EXTERNALS //////////////////////////////////////////////////////////////////
