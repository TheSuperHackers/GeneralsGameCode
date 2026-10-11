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

// Each assembler function that VC6 uses is measured next to the compiler intrinsic that it stands in for.
// The assembler functions are called, the intrinsics are expanded in place, so the difference is the call.

namespace
{
template <typename Value, typename Operation>
void runOperation(benchmark::State &state, Operation operation)
{
	// Aligned, because the stack of x86 aligns a 64 bit value to 4 bytes only, and a locked instruction on a value
	// that crosses a cache line takes about a hundred times as long.
	alignas(Value) volatile Value value = 0;
	for (auto _ : state)
	{
		benchmark::DoNotOptimize(operation(&value));
	}
}
} // namespace

static void BM_InterlockedExchangeAdd8_x86asm(benchmark::State &state)
{
	runOperation<char>(state, [](volatile char *value) { return x86asm::InterlockedExchangeAdd8(value, 1); });
}
BENCHMARK(BM_InterlockedExchangeAdd8_x86asm);

static void BM_InterlockedExchangeAdd8_intrinsic(benchmark::State &state)
{
	runOperation<char>(state, [](volatile char *value) { return _InterlockedExchangeAdd8(value, 1); });
}
BENCHMARK(BM_InterlockedExchangeAdd8_intrinsic);

static void BM_InterlockedExchangeAdd16_x86asm(benchmark::State &state)
{
	runOperation<short>(state, [](volatile short *value) { return x86asm::InterlockedExchangeAdd16(value, 1); });
}
BENCHMARK(BM_InterlockedExchangeAdd16_x86asm);

static void BM_InterlockedExchangeAdd16_intrinsic(benchmark::State &state)
{
	runOperation<short>(state, [](volatile short *value) { return _InterlockedExchangeAdd16(value, 1); });
}
BENCHMARK(BM_InterlockedExchangeAdd16_intrinsic);

static void BM_InterlockedExchangeAdd_x86asm(benchmark::State &state)
{
	runOperation<long>(state, [](volatile long *value) { return x86asm::InterlockedExchangeAdd(value, 1); });
}
BENCHMARK(BM_InterlockedExchangeAdd_x86asm);

static void BM_InterlockedExchangeAdd_intrinsic(benchmark::State &state)
{
	runOperation<long>(state, [](volatile long *value) { return _InterlockedExchangeAdd(value, 1); });
}
BENCHMARK(BM_InterlockedExchangeAdd_intrinsic);

// The processor has no instruction that adds or exchanges 64 bits. The Windows SDK has these two as inline
// functions that repeat the compare exchange intrinsic, and the assembler functions do the same.
static void BM_InterlockedExchangeAdd64_x86asm(benchmark::State &state)
{
	runOperation<long long>(state, [](volatile long long *value) { return x86asm::InterlockedExchangeAdd64(value, 1); });
}
BENCHMARK(BM_InterlockedExchangeAdd64_x86asm);

static void BM_InterlockedExchangeAdd64_sdk(benchmark::State &state)
{
	runOperation<long long>(state, [](volatile long long *value) { return _InlineInterlockedExchangeAdd64(value, 1); });
}
BENCHMARK(BM_InterlockedExchangeAdd64_sdk);

static void BM_InterlockedExchange64_x86asm(benchmark::State &state)
{
	runOperation<long long>(state, [](volatile long long *value) { return x86asm::InterlockedExchange64(value, 1); });
}
BENCHMARK(BM_InterlockedExchange64_x86asm);

static void BM_InterlockedExchange64_sdk(benchmark::State &state)
{
	runOperation<long long>(state, [](volatile long long *value) { return _InlineInterlockedExchange64(value, 1); });
}
BENCHMARK(BM_InterlockedExchange64_sdk);

// The compare exchange always finds the comparand and stores the same value again.
static void BM_InterlockedCompareExchange8_x86asm(benchmark::State &state)
{
	runOperation<char>(state, [](volatile char *value) { return x86asm::InterlockedCompareExchange8(value, 0, 0); });
}
BENCHMARK(BM_InterlockedCompareExchange8_x86asm);

static void BM_InterlockedCompareExchange8_intrinsic(benchmark::State &state)
{
	runOperation<char>(state, [](volatile char *value) { return _InterlockedCompareExchange8(value, 0, 0); });
}
BENCHMARK(BM_InterlockedCompareExchange8_intrinsic);

static void BM_InterlockedCompareExchange16_x86asm(benchmark::State &state)
{
	runOperation<short>(state, [](volatile short *value) { return x86asm::InterlockedCompareExchange16(value, 0, 0); });
}
BENCHMARK(BM_InterlockedCompareExchange16_x86asm);

static void BM_InterlockedCompareExchange16_intrinsic(benchmark::State &state)
{
	runOperation<short>(state, [](volatile short *value) { return _InterlockedCompareExchange16(value, 0, 0); });
}
BENCHMARK(BM_InterlockedCompareExchange16_intrinsic);

static void BM_InterlockedCompareExchange_x86asm(benchmark::State &state)
{
	runOperation<long>(state, [](volatile long *value) { return x86asm::InterlockedCompareExchange(value, 0, 0); });
}
BENCHMARK(BM_InterlockedCompareExchange_x86asm);

static void BM_InterlockedCompareExchange_intrinsic(benchmark::State &state)
{
	runOperation<long>(state, [](volatile long *value) { return _InterlockedCompareExchange(value, 0, 0); });
}
BENCHMARK(BM_InterlockedCompareExchange_intrinsic);

static void BM_InterlockedCompareExchange64_x86asm(benchmark::State &state)
{
	runOperation<long long>(state, [](volatile long long *value) { return x86asm::InterlockedCompareExchange64(value, 0, 0); });
}
BENCHMARK(BM_InterlockedCompareExchange64_x86asm);

static void BM_InterlockedCompareExchange64_intrinsic(benchmark::State &state)
{
	runOperation<long long>(state, [](volatile long long *value) { return _InterlockedCompareExchange64(value, 0, 0); });
}
BENCHMARK(BM_InterlockedCompareExchange64_intrinsic);

static void BM_ReadAcquire_x86asm(benchmark::State &state)
{
	runOperation<long>(state, [](volatile long *value) { return x86asm::ReadAcquire(value); });
}
BENCHMARK(BM_ReadAcquire_x86asm);

// The plain read of a volatile value that the function stands in for.
static void BM_ReadAcquire_volatile(benchmark::State &state)
{
	runOperation<long>(state, [](volatile long *value) { return *value; });
}
BENCHMARK(BM_ReadAcquire_volatile);

#endif
