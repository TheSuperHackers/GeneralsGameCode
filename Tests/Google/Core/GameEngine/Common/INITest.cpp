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

#include <cstdio>
#include <gtest/gtest.h>

#include "Common/INI.h"

TEST(INI, OversizedIntegersMatchLegacyParser)
{
	const char* tokens[] = {
		"9999999999999999999", "+9999999999999999999",
		"-9999999999999999999", "9999999999999999999suffix",
		"9223372036854775807", "-9223372036854775808",
		"9223372036854775808", "-9223372036854775809",
		"18446744073709551616", "-18446744073709551616",
		"99999999999999999999999999999999999999999999999999"
	};
	for (const char* token : tokens)
	{
		SCOPED_TRACE(token);
		// Preserve the old parser's behavior on this runtime, not a portable overflow policy.
		Int signedExpected = 0;
		UnsignedInt unsignedExpected = 0;
		ASSERT_EQ(std::sscanf(token, "%d", &signedExpected), 1);
		ASSERT_EQ(std::sscanf(token, "%u", &unsignedExpected), 1);
		EXPECT_EQ(INI::scanInt(token), signedExpected);
		EXPECT_EQ(INI::scanUnsignedInt(token), unsignedExpected);
	}
}

TEST(INI, InvalidIntegersAreRejected)
{
	for (const char* token : {"abc", "+", "-", "+-9999999999999999999"})
	{
		SCOPED_TRACE(token);
		EXPECT_THROW(INI::scanInt(token), decltype(INI_INVALID_DATA));
		EXPECT_THROW(INI::scanUnsignedInt(token), decltype(INI_INVALID_DATA));
	}
}

TEST(INI, RealOverflowIsRejected)
{
	EXPECT_THROW(INI::scanReal("1e100"), decltype(INI_INVALID_DATA));
}
