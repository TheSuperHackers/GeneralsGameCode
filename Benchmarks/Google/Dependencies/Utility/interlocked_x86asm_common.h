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

#include <benchmark/benchmark.h>

#if defined(_MSC_VER) && defined(_M_IX86)

#include <windows.h>
#include <intrin.h>
#include <process.h>

// The Windows SDK defines these names as macros for compiler intrinsics. They are removed, so that the functions
// that VC6 uses can be declared under them. The intrinsics stay available under their own names, which begin
// with an underscore.
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

namespace interlocked_benchmark
{
// Calls the function in a loop on a second thread for as long as the object exists, so that a benchmark can
// measure an operation that competes with another thread.
// The thread is started with the C runtime. Neither ->Threads() of Google Benchmark nor std::thread can be used:
// both free memory with operator delete on the new thread. In the benchmark executables that is the memory manager
// of the game, and nothing there sets up the locks it needs for more than one thread. For the same reason the
// function must not allocate.
template <typename Function>
class CompetingThread
{
public:
	explicit CompetingThread(Function function)
		: m_function(function)
		, m_stop(0)
		, m_thread(reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, &CompetingThread::run, this, 0, nullptr)))
	{
	}

	~CompetingThread()
	{
		_InterlockedExchange(&m_stop, 1);
		WaitForSingleObject(m_thread, INFINITE);
		CloseHandle(m_thread);
	}

	CompetingThread(const CompetingThread &) = delete;
	CompetingThread &operator=(const CompetingThread &) = delete;

private:
	static unsigned __stdcall run(void *argument)
	{
		CompetingThread *self = static_cast<CompetingThread *>(argument);
		while (self->m_stop == 0)
			self->m_function();
		return 0;
	}

	Function m_function;
	volatile long m_stop;
	HANDLE m_thread;
};
} // namespace interlocked_benchmark

#endif
