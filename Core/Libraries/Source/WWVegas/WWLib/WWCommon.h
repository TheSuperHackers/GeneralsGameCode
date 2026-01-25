/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
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

#include "refcount.h"
#include "STLUtils.h"
#include "stringex.h"
#include <Utility/stdio_adapter.h>


// This macro serves as a general way to determine the number of elements within an array.
#ifndef ARRAY_SIZE
#if defined(_MSC_VER) && _MSC_VER < 1300
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#else
template <typename Type, size_t Size> char (*ArraySizeHelper(Type(&)[Size]))[Size];
#define ARRAY_SIZE(arr) sizeof(*ArraySizeHelper(arr))
#endif
#endif // ARRAY_SIZE

enum
{
	// TheSuperHackers @info The original WWSync was 33 ms, ~30 fps, integer.
	// Changing this will require tweaking all Drawable code that concerns the ww3d time step, including locomotion physics.
	WWSyncPerSecond = 30,
	WWSyncMilliseconds = 1000 / WWSyncPerSecond,
};

#if defined(_MSC_VER) && _MSC_VER < 1300
typedef unsigned MemValueType;
typedef long Interlocked32; // To use with Interlocked functions
#else
typedef unsigned long long MemValueType;
typedef volatile long Interlocked32; // To use with Interlocked functions
#endif
