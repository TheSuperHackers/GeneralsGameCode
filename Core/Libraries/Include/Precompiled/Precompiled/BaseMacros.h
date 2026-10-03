/*
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

#pragma once

// This macro serves as a general way to determine the number of elements within an array.
#ifndef ARRAY_SIZE
#if (defined(_MSC_VER) && _MSC_VER < 1300) || !defined(__cplusplus)
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#else
// The templated version will reject pointers on compilation.
template <typename Type, size_t Size> char (*ArraySizeHelper(Type(&)[Size]))[Size];
#define ARRAY_SIZE(arr) sizeof(*ArraySizeHelper(arr))
#endif
#endif

#ifndef PI
#define PI     3.14159265359f
#define TWO_PI 6.28318530718f
#endif

// MSVC math.h defines overloaded functions with this name...
//#ifndef abs
//#define abs(x) (((x) < 0) ? -(x) : (x))
//#endif

#ifndef MIN
#define MIN(x,y) (((x)<(y)) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x,y) (((x)>(y)) ? (x) : (y))
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

//-----------------------------------------------------------------------------
// For twiddling bits
// TheSuperHackers @build BitTest was renamed to BitIsSet
// to prevent conflict with BitTest macro from winnt.h
//-----------------------------------------------------------------------------
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
