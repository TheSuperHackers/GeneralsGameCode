/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
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

// This file provides std::atomic. VC6 has no <atomic> header and gets a class that is built on the functions of
// interlocked_adapter.h. It supports bool, the integer types, float, double, enums and pointers.
//
// The VC6 class has the size of the type it holds, like the standard class. It differs from it in these points:
// - Its constructor is not constexpr. An object with static storage duration is initialized when its constructor
//   runs during the static initialization, where the standard class initializes it at compile time. A value that
//   another static initializer stored into the object before that is overwritten.
// - An atomic pointer has no fetch_add, fetch_sub, ++, --, += and -=.
// - A 64 bit value is loaded with a locked instruction, which costs as much as storing it.
// - A 64 bit value is aligned to 8 bytes as a global and inside a class, but only to 4 bytes on the stack and in
//   memory of an allocator that aligns to 4 bytes. A value that crosses a cache line works correctly, but every
//   operation on it takes about a hundred times as long.

#pragma once

#if defined(_MSC_VER) && _MSC_VER < 1300

	#include <stddef.h>
	#include "Utility/interlocked_adapter.h"
	namespace std
	{
	#include "atomic_x86interlocked.inl"
	}

#else

	#include <atomic>

#endif
