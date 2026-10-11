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

namespace
{
const short Values16[] = { 0, 1, 2, -1, -2, 127, 128, 255, 256, 0x5555, 0x7ffe, 0x7fff, -0x7fff, -0x8000 };
const long Values32[] = { 0, 1, 2, -1, -2, 0x7fff, 0x8000, 0xffff, 0x10000, 0x55555555, 0x7ffffffe, 0x7fffffff,
	-0x7fffffff, -0x7fffffff - 1 };
const long long Values64[] = { 0, 1, 2, -1, -2, 0x7fffffff, 0x80000000, 0xffffffff, 0x100000000, 0x100000001,
	-0x100000000, 0x5555555555555555, 0x7ffffffffffffffe, 0x7fffffffffffffff, -0x7fffffffffffffff,
	-0x7fffffffffffffff - 1 };

unsigned long lowHalf(long long value)
{
	return static_cast<unsigned long>(value);
}

unsigned long highHalf(long long value)
{
	return static_cast<unsigned long>(static_cast<unsigned long long>(value) >> 32);
}

using interlocked_test::runThreads;
using interlocked_test::ThreadCount;
using interlocked_test::ThreadIterations;
} // namespace

// Every function is given the same input as the compiler intrinsic of the same name and must return and store
// the same value. The 8 bit functions are tested with every pair of values.
TEST(InterlockedX86Asm, Functions8MatchIntrinsics)
{
	for (int a = -128; a <= 127; ++a)
	{
		for (int b = -128; b <= 127; ++b)
		{
			const char first = static_cast<char>(a);
			const char second = static_cast<char>(b);
			volatile char ours = first;
			volatile char theirs = first;

			ASSERT_EQ(x86asm::ReadAcquire8(&ours), first);

			ASSERT_EQ(x86asm::InterlockedExchange8(&ours, second), _InterlockedExchange8(&theirs, second));
			ASSERT_EQ(ours, theirs);

			ASSERT_EQ(x86asm::InterlockedExchangeAdd8(&ours, first), _InterlockedExchangeAdd8(&theirs, first));
			ASSERT_EQ(ours, theirs);

			// The comparand is the stored value in the first call and another value in the second.
			const char sum = static_cast<char>(a + b);
			ASSERT_EQ(x86asm::InterlockedCompareExchange8(&ours, second, sum),
				_InterlockedCompareExchange8(&theirs, second, sum));
			ASSERT_EQ(ours, theirs);

			const char other = static_cast<char>(b + 1);
			ASSERT_EQ(x86asm::InterlockedCompareExchange8(&ours, first, other),
				_InterlockedCompareExchange8(&theirs, first, other));
			ASSERT_EQ(ours, second);
			ASSERT_EQ(ours, theirs);

			ASSERT_EQ(x86asm::ReadAcquire8(&ours), theirs);
		}
	}
}

TEST(InterlockedX86Asm, Functions16MatchIntrinsics)
{
	for (const short first : Values16)
	{
		for (const short second : Values16)
		{
			volatile short ours = first;
			volatile short theirs = first;

			ASSERT_EQ(x86asm::ReadAcquire16(&ours), first);

			ASSERT_EQ(x86asm::InterlockedExchange16(&ours, second), _InterlockedExchange16(&theirs, second));
			ASSERT_EQ(ours, theirs);

			ASSERT_EQ(x86asm::InterlockedExchangeAdd16(&ours, first), _InterlockedExchangeAdd16(&theirs, first));
			ASSERT_EQ(ours, theirs);

			const short sum = static_cast<short>(first + second);
			ASSERT_EQ(x86asm::InterlockedCompareExchange16(&ours, second, sum),
				_InterlockedCompareExchange16(&theirs, second, sum));
			ASSERT_EQ(ours, theirs);

			const short other = static_cast<short>(second + 1);
			ASSERT_EQ(x86asm::InterlockedCompareExchange16(&ours, first, other),
				_InterlockedCompareExchange16(&theirs, first, other));
			ASSERT_EQ(ours, second);
			ASSERT_EQ(ours, theirs);

			ASSERT_EQ(x86asm::ReadAcquire16(&ours), theirs);
		}
	}
}

TEST(InterlockedX86Asm, Functions32MatchIntrinsics)
{
	for (const long first : Values32)
	{
		for (const long second : Values32)
		{
			volatile long ours = first;
			volatile long theirs = first;

			ASSERT_EQ(x86asm::ReadAcquire(&ours), first);

			ASSERT_EQ(x86asm::InterlockedExchange(&ours, second), _InterlockedExchange(&theirs, second));
			ASSERT_EQ(ours, theirs);

			ASSERT_EQ(x86asm::InterlockedExchangeAdd(&ours, first), _InterlockedExchangeAdd(&theirs, first));
			ASSERT_EQ(ours, theirs);

			const long sum = static_cast<long>(static_cast<unsigned long>(first) + static_cast<unsigned long>(second));
			ASSERT_EQ(x86asm::InterlockedCompareExchange(&ours, second, sum),
				_InterlockedCompareExchange(&theirs, second, sum));
			ASSERT_EQ(ours, theirs);

			const long other = static_cast<long>(static_cast<unsigned long>(second) + 1);
			ASSERT_EQ(x86asm::InterlockedCompareExchange(&ours, first, other),
				_InterlockedCompareExchange(&theirs, first, other));
			ASSERT_EQ(ours, second);
			ASSERT_EQ(ours, theirs);

			ASSERT_EQ(x86asm::ReadAcquire(&ours), theirs);
		}
	}
}

// The processor has no instruction that exchanges or adds 64 bits, so the Windows SDK has these two as inline
// functions around the compare exchange intrinsic. They are the reference for the functions of the same name.
TEST(InterlockedX86Asm, Functions64MatchWindowsSdk)
{
	for (const long long first : Values64)
	{
		for (const long long second : Values64)
		{
			volatile long long ours = first;
			volatile long long theirs = first;

			ASSERT_EQ(x86asm::ReadAcquire64(&ours), first);

			ASSERT_EQ(x86asm::InterlockedExchange64(&ours, second), _InlineInterlockedExchange64(&theirs, second));
			ASSERT_EQ(ours, theirs);

			ASSERT_EQ(x86asm::InterlockedExchangeAdd64(&ours, first), _InlineInterlockedExchangeAdd64(&theirs, first));
			ASSERT_EQ(ours, theirs);

			const long long sum = static_cast<long long>(static_cast<unsigned long long>(first) + static_cast<unsigned long long>(second));
			ASSERT_EQ(x86asm::InterlockedCompareExchange64(&ours, second, sum),
				_InterlockedCompareExchange64(&theirs, second, sum));
			ASSERT_EQ(ours, theirs);

			const long long other = static_cast<long long>(static_cast<unsigned long long>(second) + 1);
			ASSERT_EQ(x86asm::InterlockedCompareExchange64(&ours, first, other),
				_InterlockedCompareExchange64(&theirs, first, other));
			ASSERT_EQ(ours, second);
			ASSERT_EQ(ours, theirs);

			// A comparand that differs in the upper half only must not match either.
			const long long otherHigh = second ^ 0x100000000;
			ASSERT_EQ(x86asm::InterlockedCompareExchange64(&ours, first, otherHigh), second);
			ASSERT_EQ(ours, second);

			// A compare exchange of 0 for 0 reads the value and leaves it as it is.
			ASSERT_EQ(x86asm::InterlockedCompareExchange64(&ours, 0, 0), second);
			ASSERT_EQ(ours, second);

			ASSERT_EQ(x86asm::ReadAcquire64(&ours), theirs);
		}
	}
}

TEST(InterlockedX86Asm, PointerFunctionsExchange)
{
	int values[3] = {};
	void* volatile pointer = &values[0];

	// A comparand that does not match leaves the pointer alone and returns what is stored.
	EXPECT_EQ(x86asm::InterlockedCompareExchangePointer(&pointer, &values[2], &values[1]), &values[0]);
	EXPECT_EQ(pointer, &values[0]);

	EXPECT_EQ(x86asm::InterlockedCompareExchangePointer(&pointer, &values[1], &values[0]), &values[0]);
	EXPECT_EQ(pointer, &values[1]);

	EXPECT_EQ(x86asm::InterlockedExchangePointer(&pointer, nullptr), &values[1]);
	EXPECT_EQ(pointer, nullptr);
}

// A function that wrote more bytes than its size would change a neighbour of the value.
TEST(InterlockedX86Asm, FunctionsWriteOnlyTheirSize)
{
	struct
	{
		char before;
		volatile char value;
		char after;
	} bytes = { 0x11, 0, 0x22 };

	x86asm::InterlockedExchange8(&bytes.value, -1);
	x86asm::InterlockedExchangeAdd8(&bytes.value, 1);
	x86asm::InterlockedCompareExchange8(&bytes.value, -1, 0);
	EXPECT_EQ(bytes.value, -1);
	EXPECT_EQ(bytes.before, 0x11);
	EXPECT_EQ(bytes.after, 0x22);

	struct
	{
		short before;
		volatile short value;
		short after;
	} words = { 0x1111, 0, 0x2222 };

	x86asm::InterlockedExchange16(&words.value, -1);
	x86asm::InterlockedExchangeAdd16(&words.value, 1);
	x86asm::InterlockedCompareExchange16(&words.value, -1, 0);
	EXPECT_EQ(words.value, -1);
	EXPECT_EQ(words.before, 0x1111);
	EXPECT_EQ(words.after, 0x2222);

	struct
	{
		long before;
		volatile long value;
		long after;
	} dwords = { 0x11111111, 0, 0x22222222 };

	x86asm::InterlockedExchange(&dwords.value, -1);
	x86asm::InterlockedExchangeAdd(&dwords.value, 1);
	x86asm::InterlockedCompareExchange(&dwords.value, -1, 0);
	EXPECT_EQ(dwords.value, -1);
	EXPECT_EQ(dwords.before, 0x11111111);
	EXPECT_EQ(dwords.after, 0x22222222);

	struct
	{
		long long before;
		volatile long long value;
		long long after;
	} qwords = { 0x1111111111111111, 0, 0x2222222222222222 };

	x86asm::InterlockedExchange64(&qwords.value, -1);
	x86asm::InterlockedExchangeAdd64(&qwords.value, 1);
	x86asm::InterlockedCompareExchange64(&qwords.value, -1, 0);
	EXPECT_EQ(qwords.value, -1);
	EXPECT_EQ(qwords.before, 0x1111111111111111);
	EXPECT_EQ(qwords.after, 0x2222222222222222);
}

// Additions of several threads must all arrive. The 8 and 16 bit counters wrap around on the way, and every
// addition to the 64 bit counter carries into its upper half.
TEST(InterlockedX86Asm, ExchangeAddCountsWithThreads)
{
	const long long step64 = 0x180000001;
	volatile char counter8 = 0;
	volatile short counter16 = 0;
	volatile long counter32 = 0;
	alignas(8) volatile long long counter64 = 0;

	runThreads([&](int)
	{
		for (int i = 0; i < ThreadIterations; ++i)
		{
			x86asm::InterlockedExchangeAdd8(&counter8, 3);
			x86asm::InterlockedExchangeAdd16(&counter16, 7);
			x86asm::InterlockedExchangeAdd(&counter32, 11);
			x86asm::InterlockedExchangeAdd64(&counter64, step64);
		}
	});

	const long total = static_cast<long>(ThreadCount) * ThreadIterations;
	EXPECT_EQ(counter8, static_cast<char>(total * 3));
	EXPECT_EQ(counter16, static_cast<short>(total * 7));
	EXPECT_EQ(counter32, total * 11);
	EXPECT_EQ(counter64, total * step64);
}

// The functions are used as spin locks around counters that are not protected otherwise.
TEST(InterlockedX86Asm, SpinLocksExcludeThreads)
{
	volatile char lock8 = 0;
	volatile short lock16 = 0;
	volatile long lock32 = 0;
	alignas(8) volatile long long lock64 = 0;
	long count8 = 0;
	long count16 = 0;
	long count32 = 0;
	long count64 = 0;

	runThreads([&](int)
	{
		for (int i = 0; i < ThreadIterations; ++i)
		{
			while (x86asm::InterlockedCompareExchange8(&lock8, 1, 0) != 0) {}
			++count8;
			x86asm::InterlockedExchange8(&lock8, 0);

			while (x86asm::InterlockedExchange16(&lock16, 1) != 0) {}
			++count16;
			x86asm::InterlockedCompareExchange16(&lock16, 0, 1);

			while (x86asm::InterlockedCompareExchange(&lock32, 1, 0) != 0) {}
			++count32;
			x86asm::InterlockedExchange(&lock32, 0);

			// The value that locks is in the upper half.
			while (x86asm::InterlockedCompareExchange64(&lock64, 0x100000000, 0) != 0) {}
			++count64;
			x86asm::InterlockedExchange64(&lock64, 0);
		}
	});

	const long total = static_cast<long>(ThreadCount) * ThreadIterations;
	EXPECT_EQ(count8, total);
	EXPECT_EQ(count16, total);
	EXPECT_EQ(count32, total);
	EXPECT_EQ(count64, total);
}

// Every thread stores values whose two halves are equal. A function that read or stored the 8 bytes in two steps
// would hand out the halves of two different values.
TEST(InterlockedX86Asm, Functions64NeverSeeHalfAValue)
{
	alignas(8) volatile long long shared = 0;
	long halves[ThreadCount] = {};

	runThreads([&](int index)
	{
		long seen = 0;
		for (int i = 0; i < ThreadIterations; ++i)
		{
			const long long half = index * 1000000 + i;
			const long long exchanged = x86asm::InterlockedExchange64(&shared, (half << 32) | half);
			if (highHalf(exchanged) != lowHalf(exchanged))
				++seen;

			const long long read = x86asm::InterlockedCompareExchange64(&shared, 0, 0);
			if (highHalf(read) != lowHalf(read))
				++seen;
		}
		halves[index] = seen;
	});

	for (const long seen : halves)
		EXPECT_EQ(seen, 0);
}

// Every thread swaps the pointer it holds with the shared one. No pointer may get lost or appear twice.
TEST(InterlockedX86Asm, ExchangePointerPassesTokensBetweenThreads)
{
	int tokens[ThreadCount + 1] = {};
	void* volatile shared = &tokens[ThreadCount];
	void* held[ThreadCount] = {};

	runThreads([&](int index)
	{
		void* token = &tokens[index];
		for (int i = 0; i < ThreadIterations; ++i)
			token = x86asm::InterlockedExchangePointer(&shared, token);
		held[index] = token;
	});

	for (int index = 0; index < ThreadCount; ++index)
		++*static_cast<int*>(held[index]);
	++*static_cast<int*>(shared);

	for (const int token : tokens)
		EXPECT_EQ(token, 1);
}

#endif
