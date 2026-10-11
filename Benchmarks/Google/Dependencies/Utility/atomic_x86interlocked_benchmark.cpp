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

#include "interlocked_x86asm_common.h"

#if defined(_MSC_VER) && defined(_M_IX86)

#include <atomic>
#include <stddef.h>

// The class that VC6 uses as std::atomic, compiled by this compiler and running on the assembler functions.
namespace x86asm
{
#include "Utility/atomic_x86interlocked.inl"
}

// Every benchmark runs with the class of VC6 and with std::atomic, which it stands in for. The numbers are for
// the code of this compiler. VC6 generates different code around the same assembler functions.
//
// The object is static, so that the competing thread of a benchmark can work on the same one.

template <template <typename> class Atomic, typename T>
static void BM_AtomicLoad(benchmark::State &state)
{
	static Atomic<T> value;
	for (auto _ : state)
	{
		benchmark::DoNotOptimize(value.load());
	}
}
BENCHMARK_TEMPLATE(BM_AtomicLoad, x86asm::atomic, bool);
BENCHMARK_TEMPLATE(BM_AtomicLoad, std::atomic, bool);
BENCHMARK_TEMPLATE(BM_AtomicLoad, x86asm::atomic, unsigned char);
BENCHMARK_TEMPLATE(BM_AtomicLoad, std::atomic, unsigned char);
BENCHMARK_TEMPLATE(BM_AtomicLoad, x86asm::atomic, short);
BENCHMARK_TEMPLATE(BM_AtomicLoad, std::atomic, short);
BENCHMARK_TEMPLATE(BM_AtomicLoad, x86asm::atomic, int);
BENCHMARK_TEMPLATE(BM_AtomicLoad, std::atomic, int);
BENCHMARK_TEMPLATE(BM_AtomicLoad, x86asm::atomic, float);
BENCHMARK_TEMPLATE(BM_AtomicLoad, std::atomic, float);

template <template <typename> class Atomic, typename T>
static void BM_AtomicStore(benchmark::State &state)
{
	static Atomic<T> value;
	for (auto _ : state)
	{
		value.store(T(1));
	}
}
BENCHMARK_TEMPLATE(BM_AtomicStore, x86asm::atomic, bool);
BENCHMARK_TEMPLATE(BM_AtomicStore, std::atomic, bool);
BENCHMARK_TEMPLATE(BM_AtomicStore, x86asm::atomic, int);
BENCHMARK_TEMPLATE(BM_AtomicStore, std::atomic, int);

template <template <typename> class Atomic, typename T>
static void BM_AtomicFetchAdd(benchmark::State &state)
{
	static Atomic<T> value;
	for (auto _ : state)
	{
		benchmark::DoNotOptimize(value.fetch_add(T(1)));
	}
}
BENCHMARK_TEMPLATE(BM_AtomicFetchAdd, x86asm::atomic, unsigned char);
BENCHMARK_TEMPLATE(BM_AtomicFetchAdd, std::atomic, unsigned char);
BENCHMARK_TEMPLATE(BM_AtomicFetchAdd, x86asm::atomic, short);
BENCHMARK_TEMPLATE(BM_AtomicFetchAdd, std::atomic, short);
BENCHMARK_TEMPLATE(BM_AtomicFetchAdd, x86asm::atomic, int);
BENCHMARK_TEMPLATE(BM_AtomicFetchAdd, std::atomic, int);
// For a float both classes add with a retry loop around a compare exchange.
BENCHMARK_TEMPLATE(BM_AtomicFetchAdd, x86asm::atomic, float);
BENCHMARK_TEMPLATE(BM_AtomicFetchAdd, std::atomic, float);

// The bitwise functions are a retry loop around a compare exchange in the class of VC6.
template <template <typename> class Atomic, typename T>
static void BM_AtomicFetchOr(benchmark::State &state)
{
	static Atomic<T> value;
	for (auto _ : state)
	{
		benchmark::DoNotOptimize(value.fetch_or(T(1)));
	}
}
BENCHMARK_TEMPLATE(BM_AtomicFetchOr, x86asm::atomic, unsigned char);
BENCHMARK_TEMPLATE(BM_AtomicFetchOr, std::atomic, unsigned char);
BENCHMARK_TEMPLATE(BM_AtomicFetchOr, x86asm::atomic, short);
BENCHMARK_TEMPLATE(BM_AtomicFetchOr, std::atomic, short);
BENCHMARK_TEMPLATE(BM_AtomicFetchOr, x86asm::atomic, int);
BENCHMARK_TEMPLATE(BM_AtomicFetchOr, std::atomic, int);

// The compare exchange always finds the expected value and stores the same value again.
template <template <typename> class Atomic, typename T>
static void BM_AtomicCompareExchange(benchmark::State &state)
{
	static Atomic<T> value;
	T expected = T();
	for (auto _ : state)
	{
		benchmark::DoNotOptimize(value.compare_exchange_strong(expected, expected));
	}
}
BENCHMARK_TEMPLATE(BM_AtomicCompareExchange, x86asm::atomic, bool);
BENCHMARK_TEMPLATE(BM_AtomicCompareExchange, std::atomic, bool);
BENCHMARK_TEMPLATE(BM_AtomicCompareExchange, x86asm::atomic, unsigned char);
BENCHMARK_TEMPLATE(BM_AtomicCompareExchange, std::atomic, unsigned char);
BENCHMARK_TEMPLATE(BM_AtomicCompareExchange, x86asm::atomic, short);
BENCHMARK_TEMPLATE(BM_AtomicCompareExchange, std::atomic, short);
BENCHMARK_TEMPLATE(BM_AtomicCompareExchange, x86asm::atomic, int);
BENCHMARK_TEMPLATE(BM_AtomicCompareExchange, std::atomic, int);

// The same operations while a second thread keeps adding to the object.
template <template <typename> class Atomic, typename T>
static void BM_AtomicLoadCompeting(benchmark::State &state)
{
	static Atomic<T> value;
	interlocked_benchmark::CompetingThread other([]() { value.fetch_add(T(1)); });
	for (auto _ : state)
	{
		benchmark::DoNotOptimize(value.load());
	}
}
BENCHMARK_TEMPLATE(BM_AtomicLoadCompeting, x86asm::atomic, int);
BENCHMARK_TEMPLATE(BM_AtomicLoadCompeting, std::atomic, int);

template <template <typename> class Atomic, typename T>
static void BM_AtomicFetchAddCompeting(benchmark::State &state)
{
	static Atomic<T> value;
	interlocked_benchmark::CompetingThread other([]() { value.fetch_add(T(1)); });
	for (auto _ : state)
	{
		benchmark::DoNotOptimize(value.fetch_add(T(1)));
	}
}
BENCHMARK_TEMPLATE(BM_AtomicFetchAddCompeting, x86asm::atomic, int);
BENCHMARK_TEMPLATE(BM_AtomicFetchAddCompeting, std::atomic, int);

// The retry loop of the class of VC6 has to repeat when the other thread got in between.
template <template <typename> class Atomic, typename T>
static void BM_AtomicFetchOrCompeting(benchmark::State &state)
{
	static Atomic<T> value;
	interlocked_benchmark::CompetingThread other([]() { value.fetch_add(T(1)); });
	for (auto _ : state)
	{
		benchmark::DoNotOptimize(value.fetch_or(T(1)));
	}
}
BENCHMARK_TEMPLATE(BM_AtomicFetchOrCompeting, x86asm::atomic, int);
BENCHMARK_TEMPLATE(BM_AtomicFetchOrCompeting, std::atomic, int);

#endif
