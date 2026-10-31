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

#include "ICU/utf8.h"

TEST(Utf8, RoundTripsMultibyteText)
{
	const char utf8[] = "\xC3\xBC\xE2\x82\xAC\xF0\x9F\x98\x80"; // U+00FC U+20AC U+1F600
	const size_t utf8Len = sizeof(utf8) - 1;
	// unichar holds UTF-16 code units on Windows and UTF-32 codepoints on most other platforms.
#if defined(WCHAR_MAX) && (WCHAR_MAX <= 0xFFFF)
	const unichar expected[] = { 0x00FC, 0x20AC, 0xD83D, 0xDE00, 0 };
#else
	const unichar expected[] = { 0x00FC, 0x20AC, 0x1F600, 0 };
#endif
	const size_t wideLen = sizeof(expected) / sizeof(expected[0]) - 1;

	unichar wide[8];
	ASSERT_EQ(Utf8_To_Wide_Len(utf8, utf8Len), wideLen);
	EXPECT_EQ(Utf8_To_Wide(wide, 8, utf8, utf8Len), wideLen);
	EXPECT_STREQ(wide, expected);

	char narrow[16];
	ASSERT_EQ(Wide_To_Utf8_Len(wide, wideLen), utf8Len);
	EXPECT_EQ(Wide_To_Utf8(narrow, sizeof(narrow), wide, wideLen), utf8Len);
	EXPECT_STREQ(narrow, utf8);
}

TEST(Utf8, RejectsMalformedInput)
{
	const char malformed[] = "\xC3\x28";
	EXPECT_EQ(Utf8_To_Wide_Len(malformed, 2), UTF8_INVALID);

	unichar wide[4] = { L'x' };
	EXPECT_EQ(Utf8_To_Wide(wide, 4, malformed, 2), UTF8_INVALID);
	EXPECT_EQ(wide[0], L'\0');
}

TEST(Utf8, ReplacesLoneSurrogate)
{
	const unichar lone[] = { 0xD800 };
	char narrow[4];
	EXPECT_EQ(Wide_To_Utf8(narrow, sizeof(narrow), lone, 1), 3u);
	EXPECT_STREQ(narrow, "\xEF\xBF\xBD");
}

TEST(Utf8, EmptyInputIsEmpty)
{
	EXPECT_EQ(Utf8_To_Wide_Len("", 0), 0u);
	EXPECT_EQ(Wide_To_Utf8_Len(L"", 0), 0u);
}
