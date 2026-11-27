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

#include "Common/AsciiString.h"
#include "Common/UnicodeString.h"

TEST(AsciiString, FormatAndConcat)
{
	AsciiString text;
	text.format("%s %d", "Generals", 2003);
	text.concat("!");
	EXPECT_STREQ(text.str(), "Generals 2003!");
	EXPECT_EQ(text.getLength(), 14);
}

TEST(AsciiString, TranslateRoundTripsUtf8)
{
	UnicodeString wide;
	wide.translate(AsciiString("\xC3\xBC"));
	ASSERT_EQ(wide.getLength(), 1);
	EXPECT_EQ(wide.str()[0], 0x00FC);

	AsciiString narrow;
	narrow.translate(wide);
	EXPECT_STREQ(narrow.str(), "\xC3\xBC");
}

TEST(UnicodeString, TranslateKeepsLegacyBytes)
{
	// A CP1252 byte that is not valid UTF-8 keeps its one-byte-to-one-wide-unit value.
	UnicodeString wide;
	wide.translate(AsciiString("\xFC"));
	ASSERT_EQ(wide.getLength(), 1);
	EXPECT_EQ(wide.str()[0], 0x00FC);
}
