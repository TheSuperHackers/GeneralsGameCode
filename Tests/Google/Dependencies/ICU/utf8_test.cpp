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

#include <string.h>

#include "ICU/utf8.h"

// unichar holds UTF-16 code units on Windows and UTF-32 codepoints on most other platforms.
#if defined(WCHAR_MAX) && (WCHAR_MAX <= 0xFFFF)
#define UTF8_TEST_WIDE_IS_UTF16 1
#else
#define UTF8_TEST_WIDE_IS_UTF16 0
#endif

#define UTF8_TEST_COUNT(a) (sizeof(a) / sizeof(a[0]))

namespace
{

const char NarrowSentinel = '#';
const unichar WideSentinel = 0x7777;

struct Utf8Bytes
{
	const char* str;
	size_t len;
};

#define UTF8_TEST_BYTES(s) { s, sizeof(s) - 1 }

void FillNarrow(char* dest, size_t destLen)
{
	memset(dest, NarrowSentinel, destLen);
}

void FillWide(unichar* dest, size_t destLen)
{
	for (size_t i = 0; i < destLen; ++i)
	{
		dest[i] = WideSentinel;
	}
}

void ExpectWideToUtf8(const unichar* wide, size_t wideLen, const char* utf8, size_t utf8Len)
{
	EXPECT_EQ(Wide_To_Utf8_Len(wide, wideLen), utf8Len);

	char dest[32];
	FillNarrow(dest, sizeof(dest));
	ASSERT_EQ(Wide_To_Utf8(dest, sizeof(dest), wide, wideLen), utf8Len);
	EXPECT_EQ(memcmp(dest, utf8, utf8Len), 0);
	EXPECT_EQ(dest[utf8Len], '\0');
}

void ExpectUtf8ToWide(const char* utf8, size_t utf8Len, const unichar* wide, size_t wideLen)
{
	EXPECT_EQ(Utf8_To_Wide_Len(utf8, utf8Len), wideLen);

	unichar dest[32];
	FillWide(dest, UTF8_TEST_COUNT(dest));
	ASSERT_EQ(Utf8_To_Wide(dest, UTF8_TEST_COUNT(dest), utf8, utf8Len), wideLen);
	EXPECT_EQ(memcmp(dest, wide, wideLen * sizeof(unichar)), 0);
	EXPECT_EQ(dest[wideLen], L'\0');
}

void ExpectRoundTrip(const char* utf8, size_t utf8Len, const unichar* wide, size_t wideLen)
{
	ExpectUtf8ToWide(utf8, utf8Len, wide, wideLen);
	ExpectWideToUtf8(wide, wideLen, utf8, utf8Len);
}

void ExpectInvalidUtf8(const char* utf8, size_t utf8Len)
{
	EXPECT_EQ(Utf8_To_Wide_Len(utf8, utf8Len), UTF8_INVALID);

	unichar dest[8];
	FillWide(dest, UTF8_TEST_COUNT(dest));
	EXPECT_EQ(Utf8_To_Wide(dest, UTF8_TEST_COUNT(dest), utf8, utf8Len), UTF8_INVALID);
	EXPECT_EQ(dest[0], L'\0');
}

} // namespace

TEST(Utf8, EmptyInputIsEmpty)
{
	EXPECT_EQ(Utf8_To_Wide_Len("", 0), 0u);
	EXPECT_EQ(Wide_To_Utf8_Len(L"", 0), 0u);

	char narrow[4];
	FillNarrow(narrow, sizeof(narrow));
	EXPECT_EQ(Wide_To_Utf8(narrow, sizeof(narrow), L"", 0), 0u);
	EXPECT_EQ(narrow[0], '\0');

	unichar wide[4];
	FillWide(wide, UTF8_TEST_COUNT(wide));
	EXPECT_EQ(Utf8_To_Wide(wide, UTF8_TEST_COUNT(wide), "", 0), 0u);
	EXPECT_EQ(wide[0], L'\0');
}

TEST(Utf8, RoundTripsAscii)
{
	const char utf8[] = "Generals 2003!";
	const unichar wide[] = { 'G', 'e', 'n', 'e', 'r', 'a', 'l', 's', ' ', '2', '0', '0', '3', '!' };
	ExpectRoundTrip(utf8, sizeof(utf8) - 1, wide, UTF8_TEST_COUNT(wide));
}

TEST(Utf8, RoundTripsEncodingLengthBoundaries)
{
	{
		SCOPED_TRACE("U+007F");
		const unichar wide[] = { 0x007F };
		ExpectRoundTrip("\x7F", 1, wide, 1);
	}
	{
		SCOPED_TRACE("U+0080");
		const unichar wide[] = { 0x0080 };
		ExpectRoundTrip("\xC2\x80", 2, wide, 1);
	}
	{
		SCOPED_TRACE("U+07FF");
		const unichar wide[] = { 0x07FF };
		ExpectRoundTrip("\xDF\xBF", 2, wide, 1);
	}
	{
		SCOPED_TRACE("U+0800");
		const unichar wide[] = { 0x0800 };
		ExpectRoundTrip("\xE0\xA0\x80", 3, wide, 1);
	}
	{
		SCOPED_TRACE("U+D7FF");
		const unichar wide[] = { 0xD7FF };
		ExpectRoundTrip("\xED\x9F\xBF", 3, wide, 1);
	}
	{
		SCOPED_TRACE("U+E000");
		const unichar wide[] = { 0xE000 };
		ExpectRoundTrip("\xEE\x80\x80", 3, wide, 1);
	}
	{
		SCOPED_TRACE("U+FFFF");
		const unichar wide[] = { 0xFFFF };
		ExpectRoundTrip("\xEF\xBF\xBF", 3, wide, 1);
	}
}

TEST(Utf8, RoundTripsSupplementaryCodepoints)
{
#if UTF8_TEST_WIDE_IS_UTF16
	const unichar first[] = { 0xD800, 0xDC00 };
	const unichar emoji[] = { 0xD83D, 0xDE00 };
	const unichar last[] = { 0xDBFF, 0xDFFF };
#else
	const unichar first[] = { 0x10000 };
	const unichar emoji[] = { 0x1F600 };
	const unichar last[] = { 0x10FFFF };
#endif
	{
		SCOPED_TRACE("U+10000");
		ExpectRoundTrip("\xF0\x90\x80\x80", 4, first, UTF8_TEST_COUNT(first));
	}
	{
		SCOPED_TRACE("U+1F600");
		ExpectRoundTrip("\xF0\x9F\x98\x80", 4, emoji, UTF8_TEST_COUNT(emoji));
	}
	{
		SCOPED_TRACE("U+10FFFF");
		ExpectRoundTrip("\xF4\x8F\xBF\xBF", 4, last, UTF8_TEST_COUNT(last));
	}
}

TEST(Utf8, RoundTripsMixedText)
{
	const char utf8[] = "a\xC3\xBC\xE2\x82\xAC\xF0\x9F\x98\x80z"; // a U+00FC U+20AC U+1F600 z
#if UTF8_TEST_WIDE_IS_UTF16
	const unichar wide[] = { 'a', 0x00FC, 0x20AC, 0xD83D, 0xDE00, 'z' };
#else
	const unichar wide[] = { 'a', 0x00FC, 0x20AC, 0x1F600, 'z' };
#endif
	ExpectRoundTrip(utf8, sizeof(utf8) - 1, wide, UTF8_TEST_COUNT(wide));
}

TEST(Utf8, ConvertsEmbeddedNull)
{
	const unichar wide[] = { 'a', 0, 'b' };
	ExpectRoundTrip("a\0b", 3, wide, UTF8_TEST_COUNT(wide));
}

TEST(Utf8, ConvertsOnlySourceLength)
{
	const unichar wide[] = { 'a', 'b' };
	ExpectUtf8ToWide("abc", 2, wide, UTF8_TEST_COUNT(wide));
	ExpectWideToUtf8(L"abc", 2, "ab", 2);
}

TEST(Utf8, ReplacesLoneSurrogates)
{
	{
		SCOPED_TRACE("lone high surrogate");
		const unichar wide[] = { 0xD800 };
		ExpectWideToUtf8(wide, UTF8_TEST_COUNT(wide), "\xEF\xBF\xBD", 3);
	}
	{
		SCOPED_TRACE("lone low surrogate");
		const unichar wide[] = { 0xDC00 };
		ExpectWideToUtf8(wide, UTF8_TEST_COUNT(wide), "\xEF\xBF\xBD", 3);
	}
	{
		SCOPED_TRACE("reversed surrogate pair");
		const unichar wide[] = { 0xDC00, 0xD800 };
		ExpectWideToUtf8(wide, UTF8_TEST_COUNT(wide), "\xEF\xBF\xBD\xEF\xBF\xBD", 6);
	}
	{
		SCOPED_TRACE("surrogate between text");
		const unichar wide[] = { 'a', 0xDBFF, 'b' };
		ExpectWideToUtf8(wide, UTF8_TEST_COUNT(wide), "a\xEF\xBF\xBD" "b", 5);
	}
	{
		SCOPED_TRACE("high surrogate cut off by source length");
		const unichar wide[] = { 'a', 0xD83D, 0xDE00 };
		ExpectWideToUtf8(wide, 2, "a\xEF\xBF\xBD", 4);
	}
}

#if !UTF8_TEST_WIDE_IS_UTF16
TEST(Utf8, ReplacesWideValuesBeyondUnicode)
{
	const unichar wide[] = { 'a', static_cast<unichar>(0x110000), 'b' };
	ExpectWideToUtf8(wide, UTF8_TEST_COUNT(wide), "a\xEF\xBF\xBD" "b", 5);
}
#endif

TEST(Utf8, RejectsMalformedInput)
{
	const Utf8Bytes malformed[] =
	{
		UTF8_TEST_BYTES("\x80"), // lone continuation byte
		UTF8_TEST_BYTES("\xBF"),
		UTF8_TEST_BYTES("\xC3"), // truncated sequences
		UTF8_TEST_BYTES("\xE2\x82"),
		UTF8_TEST_BYTES("\xF0\x9F\x98"),
		UTF8_TEST_BYTES("\xC3\x28"), // lead byte followed by a non-continuation byte
		UTF8_TEST_BYTES("\xE2\x28\xAC"),
		UTF8_TEST_BYTES("\xC0\x80"), // overlong encodings
		UTF8_TEST_BYTES("\xC1\xBF"),
		UTF8_TEST_BYTES("\xE0\x80\x80"),
		UTF8_TEST_BYTES("\xE0\x9F\xBF"),
		UTF8_TEST_BYTES("\xF0\x80\x80\x80"),
		UTF8_TEST_BYTES("\xF0\x8F\xBF\xBF"),
		UTF8_TEST_BYTES("\xED\xA0\x80"), // surrogate codepoints
		UTF8_TEST_BYTES("\xED\xBF\xBF"),
		UTF8_TEST_BYTES("\xF4\x90\x80\x80"), // beyond U+10FFFF
		UTF8_TEST_BYTES("\xF5\x80\x80\x80"),
		UTF8_TEST_BYTES("\xF8\x88\x80\x80\x80"), // 5 and 6 byte forms
		UTF8_TEST_BYTES("\xFC\x84\x80\x80\x80\x80"),
		UTF8_TEST_BYTES("\xFE"),
		UTF8_TEST_BYTES("\xFF"),
		UTF8_TEST_BYTES("abc\x80"), // malformed after valid text
		UTF8_TEST_BYTES("\xC3\xBC\xC3"),
	};

	for (size_t i = 0; i < UTF8_TEST_COUNT(malformed); ++i)
	{
		SCOPED_TRACE(testing::Message() << "malformed[" << i << "]");
		ExpectInvalidUtf8(malformed[i].str, malformed[i].len);
	}
}

TEST(Utf8, RejectsSequenceCutOffBySourceLength)
{
	ExpectInvalidUtf8("\xC3\xBC", 1);
	ExpectInvalidUtf8("\xF0\x9F\x98\x80", 3);
}

TEST(Utf8, MeasuresWithoutDestination)
{
	const unichar wide[] = { 'a', 0x00FC, 0x20AC };
	EXPECT_EQ(Wide_To_Utf8(nullptr, 0, wide, UTF8_TEST_COUNT(wide)), 6u);
	EXPECT_EQ(Utf8_To_Wide(nullptr, 0, "a\xC3\xBC\xE2\x82\xAC", 6), 3u);
	EXPECT_EQ(Utf8_To_Wide(nullptr, 0, "\x80", 1), UTF8_INVALID);
}

TEST(Utf8, ExactFitIsNotNullTerminated)
{
	const unichar wide[] = { 'a', 0x00FC };

	char narrow[8];
	FillNarrow(narrow, sizeof(narrow));
	EXPECT_EQ(Wide_To_Utf8(narrow, 3, wide, UTF8_TEST_COUNT(wide)), 3u);
	EXPECT_EQ(memcmp(narrow, "a\xC3\xBC", 3), 0);
	EXPECT_EQ(narrow[3], NarrowSentinel);

	unichar dest[8];
	FillWide(dest, UTF8_TEST_COUNT(dest));
	EXPECT_EQ(Utf8_To_Wide(dest, 2, "a\xC3\xBC", 3), 2u);
	EXPECT_EQ(memcmp(dest, wide, sizeof(wide)), 0);
	EXPECT_EQ(dest[2], WideSentinel);
}

TEST(Utf8, WideToUtf8TruncatesOnCodepointBoundary)
{
	const unichar wide[] = { 'a', 0x20AC, 'b' }; // "a\xE2\x82\xAC" "b"
	const size_t needed = 5;

	{
		SCOPED_TRACE("no room for the next codepoint, room for the terminator");
		char narrow[8];
		FillNarrow(narrow, sizeof(narrow));
		EXPECT_EQ(Wide_To_Utf8(narrow, 3, wide, UTF8_TEST_COUNT(wide)), needed);
		EXPECT_EQ(narrow[0], 'a');
		EXPECT_EQ(narrow[1], '\0');
		EXPECT_EQ(narrow[3], NarrowSentinel);
	}
	{
		SCOPED_TRACE("no room for the terminator");
		char narrow[8];
		FillNarrow(narrow, sizeof(narrow));
		EXPECT_EQ(Wide_To_Utf8(narrow, 4, wide, UTF8_TEST_COUNT(wide)), needed);
		EXPECT_EQ(memcmp(narrow, "a\xE2\x82\xAC", 4), 0);
		EXPECT_EQ(narrow[4], NarrowSentinel);
	}
	{
		SCOPED_TRACE("the first codepoint does not fit");
		const unichar euro[] = { 0x20AC };
		char narrow[8];
		FillNarrow(narrow, sizeof(narrow));
		EXPECT_EQ(Wide_To_Utf8(narrow, 2, euro, UTF8_TEST_COUNT(euro)), 3u);
		EXPECT_EQ(narrow[0], '\0');
		EXPECT_EQ(narrow[1], NarrowSentinel);
	}
}

TEST(Utf8, Utf8ToWideTruncatesOnCodepointBoundary)
{
	const char utf8[] = "ab\xF0\x9F\x98\x80" "c"; // a b U+1F600 c
	const size_t utf8Len = sizeof(utf8) - 1;

	unichar dest[8];
	FillWide(dest, UTF8_TEST_COUNT(dest));
#if UTF8_TEST_WIDE_IS_UTF16
	// The surrogate pair does not fit, so neither half is written.
	EXPECT_EQ(Utf8_To_Wide(dest, 3, utf8, utf8Len), 5u);
	EXPECT_EQ(dest[0], L'a');
	EXPECT_EQ(dest[1], L'b');
	EXPECT_EQ(dest[2], L'\0');
#else
	EXPECT_EQ(Utf8_To_Wide(dest, 3, utf8, utf8Len), 4u);
	EXPECT_EQ(dest[0], L'a');
	EXPECT_EQ(dest[1], L'b');
	EXPECT_EQ(dest[2], static_cast<unichar>(0x1F600));
#endif
	EXPECT_EQ(dest[3], WideSentinel);

	FillWide(dest, UTF8_TEST_COUNT(dest));
	EXPECT_EQ(Utf8_To_Wide(dest, 1, "\xC3\xBC\xC3\xBC", 4), 2u);
	EXPECT_EQ(dest[0], static_cast<unichar>(0x00FC));
	EXPECT_EQ(dest[1], WideSentinel);
}

TEST(Utf8, TruncateLenKeepsStringThatFits)
{
	EXPECT_EQ(Utf8_Truncate_Len("abc", 3, 5), 3u);
	EXPECT_EQ(Utf8_Truncate_Len("abc", 3, 3), 3u);
	EXPECT_EQ(Utf8_Truncate_Len("", 0, 0), 0u);
}

TEST(Utf8, TruncateLenCutsAscii)
{
	EXPECT_EQ(Utf8_Truncate_Len("abc", 3, 2), 2u);
	EXPECT_EQ(Utf8_Truncate_Len("abc", 3, 0), 0u);
}

TEST(Utf8, TruncateLenDoesNotSplitSequences)
{
	EXPECT_EQ(Utf8_Truncate_Len("a\xC3\xBC", 3, 2), 1u);
	EXPECT_EQ(Utf8_Truncate_Len("\xC3\xBC" "a", 3, 2), 2u);
	EXPECT_EQ(Utf8_Truncate_Len("a\xE2\x82\xAC", 4, 3), 1u);
	EXPECT_EQ(Utf8_Truncate_Len("a\xE2\x82\xAC", 4, 2), 1u);
	EXPECT_EQ(Utf8_Truncate_Len("\xF0\x9F\x98\x80", 4, 3), 0u);
	EXPECT_EQ(Utf8_Truncate_Len("\xF0\x9F\x98\x80" "a", 5, 4), 4u);
}

TEST(Utf8, TruncateLenReturnsZeroWithoutLeadByte)
{
	EXPECT_EQ(Utf8_Truncate_Len("\x80\x80\x80", 3, 2), 0u);
}
