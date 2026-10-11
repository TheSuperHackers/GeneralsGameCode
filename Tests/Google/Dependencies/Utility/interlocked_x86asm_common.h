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

#pragma once

#include <gtest/gtest.h>

#if defined(_MSC_VER) && defined(_M_IX86)

#include <windows.h>
#include <intrin.h>
#include <thread>

// The Windows SDK defines these names as macros for compiler intrinsics. They are removed, so that the functions
// that VC6 uses can be declared under them. The intrinsics stay available under their own names, which begin
// with an underscore, and serve as the reference.
#undef InterlockedCompareExchange
#undef InterlockedExchange
#undef InterlockedExchangeAdd
#undef InterlockedCompareExchange16
#undef InterlockedExchange16
#undef InterlockedExchangeAdd16
#undef InterlockedExchange8
#undef InterlockedExchangeAdd8
#undef InterlockedCompareExchange64
#undef InterlockedExchange64
#undef InterlockedExchangeAdd64
#undef InterlockedCompareExchangePointer
#undef InterlockedExchangePointer

// The namespace keeps the functions distinct from the ones of the Windows SDK.
namespace x86asm
{
#include "Utility/interlocked_x86asm.inl"
}

namespace interlocked_test
{
constexpr int ThreadCount = 4;
constexpr int ThreadIterations = 50000;

// Runs the function on every thread at once and passes it the index of its thread.
template <typename Function>
void runThreads(Function function)
{
	std::thread threads[ThreadCount];
	for (int index = 0; index < ThreadCount; ++index)
		threads[index] = std::thread([&function, index]() { function(index); });

	for (std::thread& thread : threads)
		thread.join();
}
} // namespace interlocked_test

#endif
