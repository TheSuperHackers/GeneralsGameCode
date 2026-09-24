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

// FILE: BaseType.h ///////////////////////////////////////////////////////////
//
// Project:  RTS3
//
// Basic types and constants
// Author: Michael S. Booth, January 1995, September 2000
//
///////////////////////////////////////////////////////////////////////////////

// tell the compiler to only load this file once

#pragma once

#include "Lib/BaseTypeCore.h"

//-----------------------------------------------------------------------------
typedef wchar_t WideChar;  ///< multi-byte character representations

//-----------------------------------------------------------------------------
// For twiddling bits
//-----------------------------------------------------------------------------
// TheSuperHackers @build xezon 17/03/2025 Renames BitTest to BitIsSet to prevent conflict with BitTest macro from winnt.h
#define BitIsSet( x, i ) ( ( (x) & (i) ) != 0 )
#define BitsAreSet( x, i ) ( ( (x) & (i) ) == (i) )
#define BitSet( x, i ) ( (x) |= (i) )
#define BitClear( x, i ) ( (x ) &= ~(i) )
#define BitToggle( x, i ) ( (x) ^= (i) )

//-------------------------------------------------------------------------------------------------
#define REAL_TO_INT(x)						((Int)(x))
#define REAL_TO_UNSIGNEDINT(x)		((UnsignedInt)(x))
#define REAL_TO_SHORT(x)					((Short)(x))
#define REAL_TO_UNSIGNEDSHORT(x)	((UnsignedShort)(x))
#define REAL_TO_BYTE(x)						((Byte)(x))
#define REAL_TO_UNSIGNEDBYTE(x)		((UnsignedByte)(x))
#define REAL_TO_CHAR(x)						((Char)(x))
#define DOUBLE_TO_REAL(x)					((Real)(x))
#define DOUBLE_TO_INT(x)					((Int)(x))
#define INT_TO_REAL(x)						((Real)(x))

#include "BaseFunctions.h"
#include "RealRange.h"

#include "trig.h"

#include "ICoord2D.h"
#include "ICoord3D.h"
#include "Coord2D.h"
#include "Coord3D.h"

#include "IRegion2D.h"
#include "IRegion3D.h"
#include "Region2D.h"
#include "Region3D.h"

#include "RGBColor.h"
