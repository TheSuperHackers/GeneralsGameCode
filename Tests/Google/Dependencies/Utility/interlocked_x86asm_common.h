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
#include <process.h>

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
// The threads are started with the C runtime and not with std::thread, which frees memory with operator delete on
// the new thread. In the test executables that is the memory manager of the game, and nothing there sets up the
// locks it needs for more than one thread. For the same reason the function must not allocate.
template <typename Function>
void runThreads(Function function)
{
	struct Context
	{
		Function* function;
		int index;

		static unsigned __stdcall run(void* argument)
		{
			const Context* context = static_cast<const Context*>(argument);
			(*context->function)(context->index);
			return 0;
		}
	};

	Context contexts[ThreadCount];
	HANDLE threads[ThreadCount];
	for (int index = 0; index < ThreadCount; ++index)
	{
		contexts[index].function = &function;
		contexts[index].index = index;
		threads[index] = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, &Context::run, &contexts[index], 0, nullptr));
	}

	WaitForMultipleObjects(ThreadCount, threads, TRUE, INFINITE);
	for (HANDLE thread : threads)
		CloseHandle(thread);
}
} // namespace interlocked_test

#endif
