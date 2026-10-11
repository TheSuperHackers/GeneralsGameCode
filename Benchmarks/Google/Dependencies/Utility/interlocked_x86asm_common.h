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
#include <atomic>
#include <intrin.h>
#include <thread>

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

namespace interlocked_benchmark
{
// Calls the function in a loop on a second thread for as long as the object exists, so that a benchmark can
// measure an operation that competes with another thread.
template <typename Function>
class CompetingThread
{
public:
	explicit CompetingThread(Function function)
		: m_stop(false)
		, m_thread([this, function]()
			{
				while (!m_stop.load())
					function();
			})
	{
	}

	~CompetingThread()
	{
		m_stop.store(true);
		m_thread.join();
	}

	CompetingThread(const CompetingThread &) = delete;
	CompetingThread &operator=(const CompetingThread &) = delete;

private:
	std::atomic<bool> m_stop;
	std::thread m_thread;
};
} // namespace interlocked_benchmark

#endif
