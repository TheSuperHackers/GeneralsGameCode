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

// FILE: BaseTypeCore.h ///////////////////////////////////////////////////////////
//
// Project:  RTS3
//
// Basic types and constants
// Author: Michael S. Booth, January 1995, September 2000
// TheSuperHackers @build feliwir 11/04/2025 Move common BaseType.h code to BaseTypeCore.h
//
///////////////////////////////////////////////////////////////////////////////

// tell the compiler to only load this file once

#pragma once

#include <math.h>
#include <string.h>
// TheSuperHackers @build feliwir 07/04/2025 Adds utility macros for cross-platform compatibility
#include <Utility/compat.h>
#include <Utility/stdint_adapter.h>

/*
**	Turn off some unneeded warnings.
**	Within the windows headers themselves, Microsoft has disabled the warnings 4290, 4514,
**	4069, 4200, 4237, 4103, 4001, 4035, 4164. Makes you wonder, eh?
*/

// "unreferenced inline function has been removed" Yea, so what?
#pragma warning(disable : 4514)

// Unreferenced local function removed.
#pragma warning(disable : 4505)

// 'unreferenced formal parameter'
#pragma warning(disable : 4100)

// 'identifier was truncated to '255' characters in the browser information':
// Tempates create LLLOOONNNGGG identifiers!
#pragma warning(disable : 4786)

// 'function selected for automatic inline expansion'.  Cool, but since we're treating
// warnings as errors, don't warn me about this!
#pragma warning(disable : 4711)

#if 0
// 'assignment within condition expression'. actually a pretty useful warning,
// but way too much existing code violates it.
//#pragma warning(disable : 4706)
#else
// actually, it turned out not to be too bad, so this is now ENABLED. (srj)
#pragma warning(error : 4706)
#endif

// 'conditional expression is constant'. used lots in debug builds.
#pragma warning(disable : 4127)

// 'nonstandard extension used : nameless struct/union'. MS headers violate this...
#pragma warning(disable : 4201)

// 'unreachable code'. STL violates this...
#pragma warning(disable : 4702)

// 'local variable is initialized but not referenced'. good thing to know about...
#pragma warning(error : 4189)

// 'unreferenced local variable'. good thing to know about...
#pragma warning(error : 4101)

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
#define TRUE true
#endif

#ifndef FALSE
#define FALSE false
#endif

//--------------------------------------------------------------------
// Fundamental type definitions
//--------------------------------------------------------------------
typedef float						Real;					// 4 bytes
typedef int32_t						Int;					// 4 bytes
typedef uint32_t	                UnsignedInt;	  	    // 4 bytes
typedef uint16_t	                UnsignedShort;		    // 2 bytes
typedef int16_t						Short;					// 2 bytes
typedef unsigned char	            UnsignedByte;			// 1 byte		USED TO BE "Byte"
typedef char						Byte;					// 1 byte		USED TO BE "SignedByte"
typedef char						Char;					// 1 byte of text
typedef bool						Bool;					//
// note, the types below should use "long long", but MSVC doesn't support it yet
typedef int64_t						Int64;						// 8 bytes
typedef uint64_t					UnsignedInt64;	  	        // 8 bytes
