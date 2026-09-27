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

#if defined(_MSC_VER)

/*
** Make the inline depth 255
*/
#pragma inline_depth(255)

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
// Templates create LLLOOONNNGGG identifiers!
#pragma warning(disable : 4786)

// 'function selected for automatic inline expansion'.  Cool, but since we're treating
// warnings as errors, don't warn me about this!
#pragma warning(disable : 4711)

// 'assignment within condition expression'. actually a pretty useful warning,
#pragma warning(error : 4706)

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

// Inherited from WWLib, former visualc.h:

// "overflow in floating-point constant arithmetic" This warning occurs even if the
// loss of precision is insignificant.
#pragma warning(disable : 4056)

// "typedef-name used as a synonym for class-name". This is by design and should
// not be a warning.
#pragma warning(disable : 4097)

// "conversion from 'double' to 'float', possible loss of data" Yea, so what?
#pragma warning(disable : 4244)

// "'this' used in base member initializer list" Using "this" in a base member
// initializer is valid -- no need for this warning.
#pragma warning(disable : 4355)

// 'copy constructor could not be generated'
#pragma warning(disable : 4511)

// 'assignment operator could not be generated'
#pragma warning(disable : 4512)

// "function not inlined" This warning is typically useless. The inline keyword
// only serves as a suggestion to the compiler and it may or may not inline a
// function on a case by case basis. No need to be told of this.
#pragma warning(disable : 4710)

#endif // defined(_MSC_VER)
