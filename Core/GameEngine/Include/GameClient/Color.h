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

// FILE: Color.h //////////////////////////////////////////////////////////////
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
// File name:  Color.h
//
// Created:    Colin Day, July 2001
//
// Desc:       Management of color representations
//
//-----------------------------------------------------------------------------
///////////////////////////////////////////////////////////////////////////////

#pragma once

// SYSTEM INCLUDES ////////////////////////////////////////////////////////////

// USER INCLUDES //////////////////////////////////////////////////////////////
#include "Lib/BaseType.h"

// FORWARD REFERENCES /////////////////////////////////////////////////////////

// TYPE DEFINES ///////////////////////////////////////////////////////////////
enum { GAME_COLOR_UNDEFINED = 0x00FFFFFF }; // this is white with zero alpha... safe to use!

/** @todo we need real color representation, this is just placeholder so we
can more easily identify sections of the code that need it */
typedef Int Color;

// INLINING ///////////////////////////////////////////////////////////////////

// EXTERNALS //////////////////////////////////////////////////////////////////

inline Color GameMakeColor( UnsignedByte red, UnsignedByte green, UnsignedByte blue, UnsignedByte alpha )
{
	return (alpha << 24) | (red << 16) | (green << 8) | (blue);
}

extern void GameGetColorComponents( Color color,
																	  UnsignedByte *red,
																	  UnsignedByte *green,
																	  UnsignedByte *blue,
																	  UnsignedByte *alpha );

// Put on ice until later - M Lorenzen
//extern void GameGetColorComponentsWithCheatSpy( Color color,
//																	  UnsignedByte *red,
//																	  UnsignedByte *green,
//																	  UnsignedByte *blue,
//																	  UnsignedByte *alpha );


extern void GameGetColorComponentsReal( Color color, Real *red, Real *green, Real *blue, Real *alpha );

extern Color GameDarkenColor( Color color, Int percent = 10 );

// Lifts a color toward legibility on dark backgrounds without flattening
// it out: pushes each channel additively (not by scaling) so very dark
// hues stay recognizably themselves. Pure black -> dark gray; e.g.
// ColorBlack (1,1,1) ends up around (80,80,80).
inline Color GameMakeColorReadable( Color color )
{
	Int a = (color >> 24) & 0xff;
	Int r = (color >> 16) & 0xff;
	Int g = (color >>  8) & 0xff;
	Int b =  color        & 0xff;

	Int luminance = ((r * 299) + (g * 587) + (b * 114)) / 1000;

	const Int MIN_LUMINANCE = 80;
	Int delta = MIN_LUMINANCE - luminance;
	if (delta > 0)
	{
		r += delta; if (r > 255) r = 255;
		g += delta; if (g > 255) g = 255;
		b += delta; if (b > 255) b = 255;
	}
	return (a << 24) | (r << 16) | (g << 8) | b;
}
