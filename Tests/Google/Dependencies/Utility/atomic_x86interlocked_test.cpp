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
#include <type_traits>

// The class that VC6 uses as std::atomic, compiled by this compiler. It is included into the namespace of the
// assembler functions, so that it runs on them as it does with VC6. The real std::atomic is the reference.
namespace x86asm
{
#include "Utility/atomic_x86interlocked.inl"
}

namespace
{
enum Color
{
	Color_Red,
	Color_Green = 5,
	Color_Blue = -3,
	Color_Big = 0x40000000
};

const long BoundaryValues[] = { 0, 1, 2, 3, -1, -2, 126, 127, 128, 129, -127, -128, -129, 254, 255, 256, 257,
	32766, 32767, 32768, 32769, -32767, -32768, -32769, 65534, 65535, 65536, 65537, 0x55555555, 0x7ffffffe,
	0x7fffffff, -0x7fffffff, -0x7fffffff - 1 };

// Applies an operation to both objects. They must return the same value and hold the same value afterwards.
template <typename T, typename Operation>
::testing::AssertionResult sameResult(x86asm::atomic<T>& ours, std::atomic<T>& theirs, Operation operation)
{
	const T ourResult = operation(ours);
	const T theirResult = operation(theirs);
	if (ourResult != theirResult)
	{
		return ::testing::AssertionFailure() << "returned " << static_cast<long long>(ourResult) << " instead of "
			<< static_cast<long long>(theirResult);
	}

	const T ourValue = ours.load();
	const T theirValue = theirs.load();
	if (ourValue != theirValue)
	{
		return ::testing::AssertionFailure() << "holds " << static_cast<long long>(ourValue) << " instead of "
			<< static_cast<long long>(theirValue);
	}

	return ::testing::AssertionSuccess();
}

// Applies a compare exchange to both objects. They must agree on the result, on the expected value that is
// handed back and on the value they hold afterwards.
template <typename T>
::testing::AssertionResult sameCompareExchange(x86asm::atomic<T>& ours, std::atomic<T>& theirs, T expected, T desired)
{
	T ourExpected = expected;
	T theirExpected = expected;
	if (ours.compare_exchange_strong(ourExpected, desired) != theirs.compare_exchange_strong(theirExpected, desired))
		return ::testing::AssertionFailure() << "compare_exchange_strong returned a different result";

	if (ourExpected != theirExpected || ours.load() != theirs.load())
		return ::testing::AssertionFailure() << "compare_exchange_strong left a different value";

	return ::testing::AssertionSuccess();
}

template <typename T>
bool sameLayout()
{
	return sizeof(x86asm::atomic<T>) == sizeof(std::atomic<T>) && alignof(x86asm::atomic<T>) == alignof(std::atomic<T>);
}
} // namespace

template <typename T>
class AtomicX86InterlockedInteger : public ::testing::Test
{
};

using IntegerTypes = ::testing::Types<char, signed char, unsigned char, short, unsigned short, int, unsigned int,
	long, unsigned long, wchar_t>;
TYPED_TEST_SUITE(AtomicX86InterlockedInteger, IntegerTypes);

// Every function and operator, for every pair of the boundary values.
TYPED_TEST(AtomicX86InterlockedInteger, OperationsMatchStdAtomic)
{
	using T = TypeParam;
	x86asm::atomic<T> ours;
	std::atomic<T> theirs;
	ASSERT_EQ(ours.load(), theirs.load());

	for (const long first : BoundaryValues)
	{
		for (const long second : BoundaryValues)
		{
			const T a = static_cast<T>(first);
			const T b = static_cast<T>(second);

			ASSERT_TRUE(sameResult(ours, theirs, [=](auto& x) { x.store(a); return x.load(); }));

			ASSERT_TRUE(sameResult(ours, theirs, [=](auto& x) { return x.fetch_add(b); }));
			ASSERT_TRUE(sameResult(ours, theirs, [=](auto& x) { return x.fetch_sub(a); }));
			ASSERT_TRUE(sameResult(ours, theirs, [=](auto& x) { return x.fetch_and(b); }));
			ASSERT_TRUE(sameResult(ours, theirs, [=](auto& x) { return x.fetch_or(a); }));
			ASSERT_TRUE(sameResult(ours, theirs, [=](auto& x) { return x.fetch_xor(b); }));

			ASSERT_TRUE(sameResult(ours, theirs, [](auto& x) { return ++x; }));
			ASSERT_TRUE(sameResult(ours, theirs, [](auto& x) { return x++; }));
			ASSERT_TRUE(sameResult(ours, theirs, [](auto& x) { return --x; }));
			ASSERT_TRUE(sameResult(ours, theirs, [](auto& x) { return x--; }));

			ASSERT_TRUE(sameResult(ours, theirs, [=](auto& x) { return x += b; }));
			ASSERT_TRUE(sameResult(ours, theirs, [=](auto& x) { return x -= a; }));
			ASSERT_TRUE(sameResult(ours, theirs, [=](auto& x) { return x &= b; }));
			ASSERT_TRUE(sameResult(ours, theirs, [=](auto& x) { return x |= a; }));
			ASSERT_TRUE(sameResult(ours, theirs, [=](auto& x) { return x ^= b; }));

			ASSERT_TRUE(sameResult(ours, theirs, [=](auto& x) { return x = b; }));
			ASSERT_TRUE(sameResult(ours, theirs, [=](auto& x) { return x.exchange(a); }));
			ASSERT_TRUE(sameResult(ours, theirs, [](auto& x) { const T converted = x; return converted; }));

			// The object holds a. The first call exchanges, the second finds b and hands it back.
			ASSERT_TRUE(sameCompareExchange(ours, theirs, a, b));
			ASSERT_TRUE(sameCompareExchange(ours, theirs, static_cast<T>(b + 1), a));

			T expected = static_cast<T>(a + 1);
			while (!ours.compare_exchange_weak(expected, a))
			{
			}
			theirs.store(a);
			ASSERT_EQ(ours.load(), a);
		}
	}
}

// A long sequence of operations that is the same in every run.
TYPED_TEST(AtomicX86InterlockedInteger, RandomSequenceMatchesStdAtomic)
{
	using T = TypeParam;
	x86asm::atomic<T> ours(static_cast<T>(5));
	std::atomic<T> theirs(static_cast<T>(5));
	unsigned long seed = 12345;
	const auto random = [&seed]()
	{
		seed = seed * 1664525ul + 1013904223ul;
		return seed >> 4;
	};

	for (int i = 0; i < 20000; ++i)
	{
		const unsigned long choice = random() % 18;
		const unsigned long low = random();
		const unsigned long high = random();
		const T v = static_cast<T>(low ^ (high << 12));

		switch (choice)
		{
		case 0: ASSERT_TRUE(sameResult(ours, theirs, [=](auto& x) { x.store(v); return x.load(); })); break;
		case 1: ASSERT_TRUE(sameResult(ours, theirs, [=](auto& x) { return x.exchange(v); })); break;
		case 2: ASSERT_TRUE(sameResult(ours, theirs, [=](auto& x) { return x.fetch_add(v); })); break;
		case 3: ASSERT_TRUE(sameResult(ours, theirs, [=](auto& x) { return x.fetch_sub(v); })); break;
		case 4: ASSERT_TRUE(sameResult(ours, theirs, [=](auto& x) { return x.fetch_and(v); })); break;
		case 5: ASSERT_TRUE(sameResult(ours, theirs, [=](auto& x) { return x.fetch_or(v); })); break;
		case 6: ASSERT_TRUE(sameResult(ours, theirs, [=](auto& x) { return x.fetch_xor(v); })); break;
		case 7: ASSERT_TRUE(sameResult(ours, theirs, [](auto& x) { return ++x; })); break;
		case 8: ASSERT_TRUE(sameResult(ours, theirs, [](auto& x) { return x++; })); break;
		case 9: ASSERT_TRUE(sameResult(ours, theirs, [](auto& x) { return --x; })); break;
		case 10: ASSERT_TRUE(sameResult(ours, theirs, [](auto& x) { return x--; })); break;
		case 11: ASSERT_TRUE(sameResult(ours, theirs, [=](auto& x) { return x += v; })); break;
		case 12: ASSERT_TRUE(sameResult(ours, theirs, [=](auto& x) { return x -= v; })); break;
		case 13: ASSERT_TRUE(sameResult(ours, theirs, [=](auto& x) { return x &= v; })); break;
		case 14: ASSERT_TRUE(sameResult(ours, theirs, [=](auto& x) { return x |= v; })); break;
		case 15: ASSERT_TRUE(sameResult(ours, theirs, [=](auto& x) { return x ^= v; })); break;
		case 16: ASSERT_TRUE(sameCompareExchange(ours, theirs, v, static_cast<T>(v + 1))); break;
		default: ASSERT_TRUE(sameCompareExchange(ours, theirs, theirs.load(), v)); break;
		}
	}
}

// The memory orders are accepted by every function that takes them in the standard class.
TEST(AtomicX86Interlocked, MemoryOrderArgumentsAreAccepted)
{
	x86asm::atomic<int> value(1);
	value.store(2, x86asm::memory_order_relaxed);
	EXPECT_EQ(value.load(x86asm::memory_order_acquire), 2);
	EXPECT_EQ(value.exchange(3, x86asm::memory_order_acq_rel), 2);
	EXPECT_EQ(value.fetch_add(4, x86asm::memory_order_seq_cst), 3);
	EXPECT_EQ(value.fetch_sub(1, x86asm::memory_order_relaxed), 7);
	EXPECT_EQ(value.fetch_and(6, x86asm::memory_order_release), 6);
	EXPECT_EQ(value.fetch_or(1, x86asm::memory_order_relaxed), 6);
	EXPECT_EQ(value.fetch_xor(2, x86asm::memory_order_consume), 7);

	int expected = 5;
	EXPECT_TRUE(value.compare_exchange_strong(expected, 8, x86asm::memory_order_seq_cst));
	EXPECT_FALSE(value.compare_exchange_strong(expected, 9, x86asm::memory_order_acq_rel, x86asm::memory_order_relaxed));
	EXPECT_EQ(expected, 8);
	EXPECT_TRUE(value.compare_exchange_weak(expected, 9, x86asm::memory_order_relaxed));
	expected = 9;
	EXPECT_TRUE(value.compare_exchange_weak(expected, 10, x86asm::memory_order_release, x86asm::memory_order_relaxed));
	EXPECT_EQ(value.load(), 10);

	x86asm::atomic<bool> flag;
	flag.store(true, x86asm::memory_order_release);
	EXPECT_TRUE(flag.load(x86asm::memory_order_relaxed));
	EXPECT_TRUE(flag.exchange(false, x86asm::memory_order_acq_rel));
}

TEST(AtomicX86Interlocked, BoolMatchesStdAtomic)
{
	x86asm::atomic<bool> ours;
	std::atomic<bool> theirs;
	EXPECT_EQ(ours.load(), theirs.load());

	for (const bool a : { false, true })
	{
		for (const bool b : { false, true })
		{
			ours.store(a);
			theirs.store(a);
			EXPECT_EQ(ours.exchange(b), theirs.exchange(b));
			EXPECT_EQ(ours = a, theirs = a);
			EXPECT_EQ(!ours, !theirs);
			EXPECT_EQ(ours == b, theirs == b);
			// Comparisons with an int go through the conversion to bool.
			EXPECT_EQ(ours == 1, theirs == 1);
			EXPECT_EQ(ours != 0, theirs != 0);

			for (const bool expected : { false, true })
			{
				bool ourExpected = expected;
				bool theirExpected = expected;
				EXPECT_EQ(ours.compare_exchange_strong(ourExpected, b), theirs.compare_exchange_strong(theirExpected, b));
				EXPECT_EQ(ourExpected, theirExpected);
				EXPECT_EQ(ours.load(), theirs.load());
			}
		}
	}
}

TEST(AtomicX86Interlocked, EnumMatchesStdAtomic)
{
	const Color colors[] = { Color_Red, Color_Green, Color_Blue, Color_Big };
	x86asm::atomic<Color> ours;
	std::atomic<Color> theirs;
	EXPECT_EQ(ours.load(), theirs.load());

	for (const Color a : colors)
	{
		for (const Color b : colors)
		{
			ours.store(a);
			theirs.store(a);
			EXPECT_EQ(ours.exchange(b), theirs.exchange(b));
			EXPECT_EQ(ours = a, theirs = a);
			const Color converted = ours;
			EXPECT_EQ(converted, a);

			for (const Color expected : colors)
			{
				Color ourExpected = expected;
				Color theirExpected = expected;
				EXPECT_EQ(ours.compare_exchange_strong(ourExpected, b), theirs.compare_exchange_strong(theirExpected, b));
				EXPECT_EQ(ourExpected, theirExpected);
				EXPECT_EQ(ours.load(), theirs.load());
			}
		}
	}
}

TEST(AtomicX86Interlocked, PointerMatchesStdAtomic)
{
	int values[3] = {};
	x86asm::atomic<int*> ours;
	std::atomic<int*> theirs;
	EXPECT_EQ(ours.load(), nullptr);

	for (int* const a : { &values[0], &values[1], static_cast<int*>(nullptr) })
	{
		for (int* const b : { &values[1], &values[2], static_cast<int*>(nullptr) })
		{
			ours.store(a);
			theirs.store(a);
			EXPECT_EQ(ours.exchange(b), theirs.exchange(b));
			EXPECT_EQ(ours = a, theirs = a);
			int* const converted = ours;
			EXPECT_EQ(converted, a);
			EXPECT_EQ(!ours, !theirs);

			for (int* const expected : { a, b })
			{
				int* ourExpected = expected;
				int* theirExpected = expected;
				EXPECT_EQ(ours.compare_exchange_strong(ourExpected, b), theirs.compare_exchange_strong(theirExpected, b));
				EXPECT_EQ(ourExpected, theirExpected);
				EXPECT_EQ(ours.load(), theirs.load());
			}
		}
	}

	const char* const text = "atomic";
	x86asm::atomic<const char*> characters(text);
	EXPECT_EQ(characters.exchange(text + 3), text);
	EXPECT_EQ(*characters.load(), 'm');

	x86asm::atomic<void*> untyped;
	void* expected = nullptr;
	EXPECT_TRUE(untyped.compare_exchange_strong(expected, values));
	EXPECT_EQ(untyped.load(), values);
}

// A class that holds an atomic must have the same layout with VC6 as with the standard class.
TEST(AtomicX86Interlocked, SizeAndAlignmentMatchStdAtomic)
{
	EXPECT_TRUE(sameLayout<bool>());
	EXPECT_TRUE(sameLayout<char>());
	EXPECT_TRUE(sameLayout<signed char>());
	EXPECT_TRUE(sameLayout<unsigned char>());
	EXPECT_TRUE(sameLayout<short>());
	EXPECT_TRUE(sameLayout<unsigned short>());
	EXPECT_TRUE(sameLayout<wchar_t>());
	EXPECT_TRUE(sameLayout<int>());
	EXPECT_TRUE(sameLayout<unsigned int>());
	EXPECT_TRUE(sameLayout<long>());
	EXPECT_TRUE(sameLayout<unsigned long>());
	EXPECT_TRUE(sameLayout<Color>());
	EXPECT_TRUE(sameLayout<int*>());
}

// The aliases name the same types as in the standard header.
static_assert(std::is_same_v<x86asm::atomic_bool, x86asm::atomic<bool>>, "atomic_bool");
static_assert(std::is_same_v<x86asm::atomic_char, x86asm::atomic<char>>, "atomic_char");
static_assert(std::is_same_v<x86asm::atomic_schar, x86asm::atomic<signed char>>, "atomic_schar");
static_assert(std::is_same_v<x86asm::atomic_uchar, x86asm::atomic<unsigned char>>, "atomic_uchar");
static_assert(std::is_same_v<x86asm::atomic_short, x86asm::atomic<short>>, "atomic_short");
static_assert(std::is_same_v<x86asm::atomic_ushort, x86asm::atomic<unsigned short>>, "atomic_ushort");
static_assert(std::is_same_v<x86asm::atomic_int, x86asm::atomic<int>>, "atomic_int");
static_assert(std::is_same_v<x86asm::atomic_uint, x86asm::atomic<unsigned int>>, "atomic_uint");
static_assert(std::is_same_v<x86asm::atomic_long, x86asm::atomic<long>>, "atomic_long");
static_assert(std::is_same_v<x86asm::atomic_ulong, x86asm::atomic<unsigned long>>, "atomic_ulong");
static_assert(std::is_same_v<x86asm::atomic_wchar_t, x86asm::atomic<wchar_t>>, "atomic_wchar_t");
static_assert(std::is_same_v<x86asm::atomic_int8_t, x86asm::atomic<int8_t>>, "atomic_int8_t");
static_assert(std::is_same_v<x86asm::atomic_uint8_t, x86asm::atomic<uint8_t>>, "atomic_uint8_t");
static_assert(std::is_same_v<x86asm::atomic_int16_t, x86asm::atomic<int16_t>>, "atomic_int16_t");
static_assert(std::is_same_v<x86asm::atomic_uint16_t, x86asm::atomic<uint16_t>>, "atomic_uint16_t");
static_assert(std::is_same_v<x86asm::atomic_int32_t, x86asm::atomic<int32_t>>, "atomic_int32_t");
static_assert(std::is_same_v<x86asm::atomic_uint32_t, x86asm::atomic<uint32_t>>, "atomic_uint32_t");
static_assert(std::is_same_v<x86asm::atomic_intptr_t, x86asm::atomic<intptr_t>>, "atomic_intptr_t");
static_assert(std::is_same_v<x86asm::atomic_uintptr_t, x86asm::atomic<uintptr_t>>, "atomic_uintptr_t");
static_assert(std::is_same_v<x86asm::atomic_size_t, x86asm::atomic<size_t>>, "atomic_size_t");
static_assert(std::is_same_v<x86asm::atomic_ptrdiff_t, x86asm::atomic<ptrdiff_t>>, "atomic_ptrdiff_t");

// Changes of several threads must all arrive. The narrow counters wrap around on the way.
TEST(AtomicX86Interlocked, CountersAreExactWithThreads)
{
	x86asm::atomic<int> intCounter;
	x86asm::atomic<unsigned int> unsignedCounter;
	x86asm::atomic<short> shortCounter;
	x86asm::atomic<unsigned short> unsignedShortCounter;
	x86asm::atomic<char> charCounter;
	x86asm::atomic<unsigned char> unsignedCharCounter;

	interlocked_test::runThreads([&](int)
	{
		for (int i = 0; i < interlocked_test::ThreadIterations; ++i)
		{
			++intCounter; intCounter += 2; intCounter.fetch_add(4); intCounter--; intCounter.fetch_sub(1);
			unsignedCounter++; unsignedCounter -= 3; --unsignedCounter;
			++shortCounter; shortCounter += 2; shortCounter.fetch_sub(1);
			unsignedShortCounter++; unsignedShortCounter.fetch_add(4);
			++charCounter; charCounter.fetch_add(2); charCounter -= 1;
			unsignedCharCounter++; unsignedCharCounter += 6;
		}
	});

	const long total = static_cast<long>(interlocked_test::ThreadCount) * interlocked_test::ThreadIterations;
	EXPECT_EQ(intCounter.load(), total * 5);
	EXPECT_EQ(unsignedCounter.load(), static_cast<unsigned int>(total * -3));
	EXPECT_EQ(shortCounter.load(), static_cast<short>(total * 2));
	EXPECT_EQ(unsignedShortCounter.load(), static_cast<unsigned short>(total * 5));
	EXPECT_EQ(charCounter.load(), static_cast<char>(total * 2));
	EXPECT_EQ(unsignedCharCounter.load(), static_cast<unsigned char>(total * 7));
}

// Every thread owns one bit and is the only one to change it. A lost update of the retry loop flips the bit of
// another thread, which that thread notices in the value that its next operation returns.
TEST(AtomicX86Interlocked, BitwiseFunctionsKeepBitsOfOtherThreads)
{
	x86asm::atomic<unsigned char> byteBits;
	x86asm::atomic<short> shortBits;
	x86asm::atomic<int> intBits;
	x86asm::atomic<int> errors;

	interlocked_test::runThreads([&](int index)
	{
		const unsigned char byteBit = static_cast<unsigned char>(1 << index);
		const short shortBit = static_cast<short>(1 << (index + 8));
		const int intBit = 1 << (index + 24);
		int seen = 0;

		for (int i = 0; i < interlocked_test::ThreadIterations; ++i)
		{
			if (byteBits.fetch_or(byteBit) & byteBit) ++seen;
			if (!(byteBits.fetch_xor(byteBit) & byteBit)) ++seen;
			if (!((byteBits |= byteBit) & byteBit)) ++seen;
			if (!(byteBits.fetch_and(static_cast<unsigned char>(~byteBit)) & byteBit)) ++seen;

			if (shortBits.fetch_or(shortBit) & shortBit) ++seen;
			if (!(shortBits.fetch_and(static_cast<short>(~shortBit)) & shortBit)) ++seen;
			if (!((shortBits ^= shortBit) & shortBit)) ++seen;
			if ((shortBits &= static_cast<short>(~shortBit)) & shortBit) ++seen;

			if (intBits.fetch_xor(intBit) & intBit) ++seen;
			if (!(intBits.fetch_or(intBit) & intBit)) ++seen;
			if (!(intBits.fetch_and(~intBit) & intBit)) ++seen;
		}

		errors += seen;
	});

	EXPECT_EQ(errors.load(), 0);
	EXPECT_EQ(byteBits.load(), 0);
	EXPECT_EQ(shortBits.load(), 0);
	EXPECT_EQ(intBits.load(), 0);
}

TEST(AtomicX86Interlocked, SpinLocksExcludeThreads)
{
	x86asm::atomic<bool> flagLock;
	x86asm::atomic<int> intLock;
	long flagCount = 0;
	long intCount = 0;

	interlocked_test::runThreads([&](int)
	{
		for (int i = 0; i < interlocked_test::ThreadIterations; ++i)
		{
			bool expected = false;
			while (!flagLock.compare_exchange_weak(expected, true))
				expected = false;
			++flagCount;
			flagLock.store(false);

			while (intLock.exchange(1) != 0)
			{
			}
			++intCount;
			intLock = 0;
		}
	});

	const long total = static_cast<long>(interlocked_test::ThreadCount) * interlocked_test::ThreadIterations;
	EXPECT_EQ(flagCount, total);
	EXPECT_EQ(intCount, total);
}

// Threads compete to publish a pointer and then an enum. Only one of them may win each round.
TEST(AtomicX86Interlocked, CompareExchangeLetsOneThreadWin)
{
	int slots[interlocked_test::ThreadCount] = {};
	x86asm::atomic<int*> winner;
	x86asm::atomic<Color> phase;
	x86asm::atomic<int> wins;

	interlocked_test::runThreads([&](int index)
	{
		for (int i = 0; i < interlocked_test::ThreadIterations; ++i)
		{
			int* expectedWinner = nullptr;
			if (winner.compare_exchange_strong(expectedWinner, &slots[index]))
			{
				Color expectedPhase = Color_Red;
				if (phase.compare_exchange_strong(expectedPhase, Color_Green))
					++wins;
				++*winner.load();
				phase.store(Color_Red);
				winner.store(nullptr);
			}
		}
	});

	long slotSum = 0;
	for (const int slot : slots)
		slotSum += slot;
	EXPECT_GT(slotSum, 0);
	EXPECT_EQ(slotSum, wins.load());
}

// A thread that waits in a loop for a value must see the store of another thread.
TEST(AtomicX86Interlocked, WaitingThreadSeesStore)
{
	x86asm::atomic<bool> flag;
	x86asm::atomic<int> value;
	x86asm::atomic<int> waiting;
	x86asm::atomic<int> seen;

	interlocked_test::runThreads([&](int index)
	{
		switch (index)
		{
		case 0:
			++waiting;
			while (!flag.load())
			{
			}
			++seen;
			break;
		case 1:
			++waiting;
			while (value == 0)
			{
			}
			++seen;
			break;
		case 2:
			// Stores once both other threads are in front of their loops.
			while (waiting.load() != 2)
			{
			}
			flag = true;
			value = 7;
			break;
		default:
			break;
		}
	});

	EXPECT_EQ(seen.load(), 2);
}

#endif
