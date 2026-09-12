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

// FILE: GadgetProgressBar.h //////////////////////////////////////////////////
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
// File name:  GadgetProgressBar.h
//
// Created:    Colin Day, June 2001
//
// Desc:       Helpful interface for ProgressBars
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "GameClient/GameWindow.h"
// FORWARD REFERENCES /////////////////////////////////////////////////////////


extern void GadgetProgressBarSetProgress( GameWindow *g, Int progress );
// TYPE DEFINES ///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// INLINING ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
inline void GadgetProgressBarSetEnabledColor( GameWindow *g, Color color )								{ g->winSetEnabledColor( 0, color ); }
inline void GadgetProgressBarSetEnabledBorderColor( GameWindow *g, Color color )					{ g->winSetEnabledBorderColor( 0, color ); }
inline void GadgetProgressBarSetEnabledImageLeft( GameWindow *g, const Image *image )						{ g->winSetEnabledImage( 0, image ); }
inline void GadgetProgressBarSetEnabledImageRight( GameWindow *g, const Image *image )					{ g->winSetEnabledImage( 1, image ); }
inline void GadgetProgressBarSetEnabledImageCenter( GameWindow *g, const Image *image )					{ g->winSetEnabledImage( 2, image ); }
inline void GadgetProgressBarSetEnabledImageSmallCenter( GameWindow *g, const Image *image )		{ g->winSetEnabledImage( 3, image ); }
inline void GadgetProgressBarSetEnabledBarColor( GameWindow *g, Color color )							{ g->winSetEnabledColor( 4, color ); }
inline void GadgetProgressBarSetEnabledBarBorderColor( GameWindow *g, Color color )				{ g->winSetEnabledBorderColor( 4, color ); }
inline void GadgetProgressBarSetEnabledBarImageLeft( GameWindow *g, const Image *image )				{ g->winSetEnabledImage( 4, image ); }
inline void GadgetProgressBarSetEnabledBarImageRight( GameWindow *g, const Image *image )				{ g->winSetEnabledImage( 5, image ); }
inline void GadgetProgressBarSetEnabledBarImageCenter( GameWindow *g, const Image *image )			{ g->winSetEnabledImage( 6, image ); }
inline void GadgetProgressBarSetEnabledBarImageSmallCenter( GameWindow *g, const Image *image )	{ g->winSetEnabledImage( 7, image ); }
inline Color   GadgetProgressBarGetEnabledColor( GameWindow *g )								{ return g->winGetEnabledColor( 0 ); }
inline Color   GadgetProgressBarGetEnabledBorderColor( GameWindow *g )					{ return g->winGetEnabledBorderColor( 0 ); }
inline const Image *GadgetProgressBarGetEnabledImageLeft( GameWindow *g )							{ return g->winGetEnabledImage( 0 ); }
inline const Image * GadgetProgressBarGetEnabledImageRight( GameWindow *g )						{ return g->winGetEnabledImage( 1 ); }
inline const Image * GadgetProgressBarGetEnabledImageCenter( GameWindow *g )					{ return g->winGetEnabledImage( 2 ); }
inline const Image * GadgetProgressBarGetEnabledImageSmallCenter( GameWindow *g )			{ return g->winGetEnabledImage( 3 ); }
inline Color   GadgetProgressBarGetEnabledBarColor( GameWindow *g )							{ return g->winGetEnabledColor( 4 ); }
inline Color   GadgetProgressBarGetEnabledBarBorderColor( GameWindow *g )				{ return g->winGetEnabledBorderColor( 4 ); }
inline const Image * GadgetProgressBarGetEnabledBarImageLeft( GameWindow *g )					{ return g->winGetEnabledImage( 4 ); }
inline const Image * GadgetProgressBarGetEnabledBarImageRight( GameWindow *g )				{ return g->winGetEnabledImage( 5 ); }
inline const Image * GadgetProgressBarGetEnabledBarImageCenter( GameWindow *g )				{ return g->winGetEnabledImage( 6 ); }
inline const Image * GadgetProgressBarGetEnabledBarImageSmallCenter( GameWindow *g )	{ return g->winGetEnabledImage( 7 ); }

//-----------------------------------------------------------------------------
inline void GadgetProgressBarSetDisabledColor( GameWindow *g, Color color )									{ g->winSetDisabledColor( 0, color ); }
inline void GadgetProgressBarSetDisabledBorderColor( GameWindow *g, Color color )						{ g->winSetDisabledBorderColor( 0, color ); }
inline void GadgetProgressBarSetDisabledImageLeft( GameWindow *g, const Image *image )						{ g->winSetDisabledImage( 0, image ); }
inline void GadgetProgressBarSetDisabledImageRight( GameWindow *g, const Image *image )						{ g->winSetDisabledImage( 1, image ); }
inline void GadgetProgressBarSetDisabledImageCenter( GameWindow *g, const Image *image )					{ g->winSetDisabledImage( 2, image ); }
inline void GadgetProgressBarSetDisabledImageSmallCenter( GameWindow *g, const Image *image )			{ g->winSetDisabledImage( 3, image ); }
inline void GadgetProgressBarSetDisabledBarColor( GameWindow *g, Color color )							{ g->winSetDisabledColor( 4, color ); }
inline void GadgetProgressBarSetDisabledBarBorderColor( GameWindow *g, Color color )				{ g->winSetDisabledBorderColor( 4, color ); }
inline void GadgetProgressBarSetDisabledBarImageLeft( GameWindow *g, const Image *image )					{ g->winSetDisabledImage( 4, image ); }
inline void GadgetProgressBarSetDisabledBarImageRight( GameWindow *g, const Image *image )				{ g->winSetDisabledImage( 5, image ); }
inline void GadgetProgressBarSetDisabledBarImageCenter( GameWindow *g, const Image *image )				{ g->winSetDisabledImage( 6, image ); }
inline void GadgetProgressBarSetDisabledBarImageSmallCenter( GameWindow *g, const Image *image )	{ g->winSetDisabledImage( 7, image ); }
inline Color   GadgetProgressBarGetDisabledColor( GameWindow *g )								{ return g->winGetDisabledColor( 0 ); }
inline Color   GadgetProgressBarGetDisabledBorderColor( GameWindow *g )					{ return g->winGetDisabledBorderColor( 0 ); }
inline const Image *GadgetProgressBarGetDisabledImageLeft( GameWindow *g )						{ return g->winGetDisabledImage( 0 ); }
inline const Image * GadgetProgressBarGetDisabledImageRight( GameWindow *g )					{ return g->winGetDisabledImage( 1 ); }
inline const Image * GadgetProgressBarGetDisabledImageCenter( GameWindow *g )					{ return g->winGetDisabledImage( 2 ); }
inline const Image * GadgetProgressBarGetDisabledImageSmallCenter( GameWindow *g )		{ return g->winGetDisabledImage( 3 ); }
inline Color   GadgetProgressBarGetDisabledBarColor( GameWindow *g )						{ return g->winGetDisabledColor( 4 ); }
inline Color   GadgetProgressBarGetDisabledBarBorderColor( GameWindow *g )			{ return g->winGetDisabledBorderColor( 4 ); }
inline const Image * GadgetProgressBarGetDisabledBarImageLeft( GameWindow *g )				{ return g->winGetDisabledImage( 4 ); }
inline const Image * GadgetProgressBarGetDisabledBarImageRight( GameWindow *g )				{ return g->winGetDisabledImage( 5 ); }
inline const Image * GadgetProgressBarGetDisabledBarImageCenter( GameWindow *g )			{ return g->winGetDisabledImage( 6 ); }
inline const Image * GadgetProgressBarGetDisabledBarImageSmallCenter( GameWindow *g )	{ return g->winGetDisabledImage( 7 ); }

//-----------------------------------------------------------------------------
inline void GadgetProgressBarSetHiliteColor( GameWindow *g, Color color )									{ g->winSetHiliteColor( 0, color ); }
inline void GadgetProgressBarSetHiliteBorderColor( GameWindow *g, Color color )						{ g->winSetHiliteBorderColor( 0, color ); }
inline void GadgetProgressBarSetHiliteImageLeft( GameWindow *g, const Image *image )						{ g->winSetHiliteImage( 0, image ); }
inline void GadgetProgressBarSetHiliteImageRight( GameWindow *g, const Image *image )						{ g->winSetHiliteImage( 1, image ); }
inline void GadgetProgressBarSetHiliteImageCenter( GameWindow *g, const Image *image )					{ g->winSetHiliteImage( 2, image ); }
inline void GadgetProgressBarSetHiliteImageSmallCenter( GameWindow *g, const Image *image )			{ g->winSetHiliteImage( 3, image ); }
inline void GadgetProgressBarSetHiliteBarColor( GameWindow *g, Color color )							{ g->winSetHiliteColor( 4, color ); }
inline void GadgetProgressBarSetHiliteBarBorderColor( GameWindow *g, Color color )				{ g->winSetHiliteBorderColor( 4, color ); }
inline void GadgetProgressBarSetHiliteBarImageLeft( GameWindow *g, const Image *image )					{ g->winSetHiliteImage( 4, image ); }
inline void GadgetProgressBarSetHiliteBarImageRight( GameWindow *g, const Image *image )				{ g->winSetHiliteImage( 5, image ); }
inline void GadgetProgressBarSetHiliteBarImageCenter( GameWindow *g, const Image *image )				{ g->winSetHiliteImage( 6, image ); }
inline void GadgetProgressBarSetHiliteBarImageSmallCenter( GameWindow *g, const Image *image )	{ g->winSetHiliteImage( 7, image ); }
inline Color   GadgetProgressBarGetHiliteColor( GameWindow *g )								{ return g->winGetHiliteColor( 0 ); }
inline Color   GadgetProgressBarGetHiliteBorderColor( GameWindow *g )					{ return g->winGetHiliteBorderColor( 0 ); }
inline const Image *GadgetProgressBarGetHiliteImageLeft( GameWindow *g )						{ return g->winGetHiliteImage( 0 ); }
inline const Image * GadgetProgressBarGetHiliteImageRight( GameWindow *g )					{ return g->winGetHiliteImage( 1 ); }
inline const Image * GadgetProgressBarGetHiliteImageCenter( GameWindow *g )					{ return g->winGetHiliteImage( 2 ); }
inline const Image * GadgetProgressBarGetHiliteImageSmallCenter( GameWindow *g )		{ return g->winGetHiliteImage( 3 ); }
inline Color   GadgetProgressBarGetHiliteBarColor( GameWindow *g )						{ return g->winGetHiliteColor( 4 ); }
inline Color   GadgetProgressBarGetHiliteBarBorderColor( GameWindow *g )			{ return g->winGetHiliteBorderColor( 4 ); }
inline const Image * GadgetProgressBarGetHiliteBarImageLeft( GameWindow *g )				{ return g->winGetHiliteImage( 4 ); }
inline const Image * GadgetProgressBarGetHiliteBarImageRight( GameWindow *g )				{ return g->winGetHiliteImage( 5 ); }
inline const Image * GadgetProgressBarGetHiliteBarImageCenter( GameWindow *g )			{ return g->winGetHiliteImage( 6 ); }
inline const Image * GadgetProgressBarGetHiliteBarImageSmallCenter( GameWindow *g )	{ return g->winGetHiliteImage( 7 ); }

// EXTERNALS //////////////////////////////////////////////////////////////////
