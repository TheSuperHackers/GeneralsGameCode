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

/******************************************************************************
*
* FILE
*     $Archive:  $
*
* DESCRIPTION
*     Generic user type definitions
*
* PROGRAMMER
*     Denzil E. Long, Jr.
*     $Author:  $
*
* VERSION INFO
*     $Modtime:  $
*     $Revision:  $
*
******************************************************************************/

#pragma once

//! Signed integer value
typedef int Int;

//! Unsigned integer value
typedef unsigned int UInt;

//! Signed 8bit value (-127 - 128)
typedef char Int8;

//! Unsigned 8bit value (0 - 255)
typedef unsigned char UInt8;

//! Signed 16bit value (-32767 - 32768)
typedef short Int16;

//! Unsigned 16bit value (0 - 65535)
typedef unsigned short UInt16;

//! Signed 32bit value
typedef long Int32;

//! Unsigned 32bit value
typedef unsigned long UInt32;

//! Signed character (ASCII)
typedef char Char;

//! Unsigned character (ANSI)
typedef unsigned char UChar;

//! Wide character (Unicode)
#if defined(_MSC_VER) && _MSC_VER < 1300
typedef unsigned short WChar;
#else
typedef wchar_t WChar;
#endif

//! 32bit floating point value
typedef float Float32;

//! 64bit floating point value
typedef double Float64;

//! Floating point value
typedef Float32 Float;

//! TriState
typedef enum {OFF = false, ON = true, PENDING = -1} TriState;

