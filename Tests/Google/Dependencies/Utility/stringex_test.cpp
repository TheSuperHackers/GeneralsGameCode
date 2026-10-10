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

#include <gtest/gtest.h>

#include "Utility/stringex.h"

namespace
{
using MemcchrFunction = const void* (*)(const void* data, int c, size_t n);

// Fills a buffer like "aaab" and checks that memcchr finds the first 'b'.
// If the searched length has no 'b', memcchr must return nullptr.
// Tests with variable lengths.
void testMemcchr(MemcchrFunction memcchrFunction)
{
	constexpr Int maxLength = 256;
	char buffer[maxLength + 1];
	for (Int length = 0; length <= maxLength; ++length)
	{
		for (Int firstB = 0; firstB <= length; ++firstB)
		{
			memset(buffer, 'b', sizeof(buffer));
			memset(buffer, 'a', firstB);
			const void* expected = firstB < length ? buffer + firstB : nullptr;
			ASSERT_EQ(memcchrFunction(buffer, 'a', length), expected);
		}
	}
}
} // namespace

TEST(StringEx, StrlcpyTruncatesAndTerminates)
{
	char buffer[4];
	EXPECT_EQ(strlcpy_t(buffer, "hello"), 5u);
	EXPECT_STREQ(buffer, "hel");
}

#include "memcchr_portable.h"
TEST(StringEx, MemcchrPortable)
{
	testMemcchr(memcchr_portable::memcchr);
}

#if defined(_M_IX86) || defined(_M_X64) || defined(__SSE2__)
#include "memcchr_sse2.h"
TEST(StringEx, MemcchrSse2)
{
	testMemcchr(memcchr_sse2::memcchr);
}
#endif

#if defined(_MSC_VER) && defined(_M_IX86)
#include "memcchr_x86asm.h"
TEST(StringEx, MemcchrX86Asm)
{
	testMemcchr(memcchr_x86asm::memcchr);
}
#endif
