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

#include <cmath>
#include <cstdio>
#include <limits>
#include <gtest/gtest.h>

#include "Common/INI.h"

TEST(INI, OversizedIntegersMatchLegacyParser)
{
	const struct {
		const char* token;
		Int signedExpected;
		UnsignedInt unsignedExpected;
	} cases[] = {
		{"9999999999999999999",                               -1, 2313682943u},
		{"+9999999999999999999",                              -1, 2313682943u},
		{"-9999999999999999999",                               0, 1981284353u},
		{"9999999999999999999suffix",                         -1, 2313682943u},
		{"9223372036854775807",                               -1, 4294967295u},
		{"-9223372036854775808",                               0,          0u},
		{"9223372036854775808",                               -1,          0u},
		{"-9223372036854775809",                               0, 4294967295u},
		{"18446744073709551616",                              -1, 4294967295u},
		{"-18446744073709551616",                              0, 4294967295u},
		{"99999999999999999999999999999999999999999999999999", -1, 4294967295u}
	};
	// Windows legacy parsing saturates at 64 bits, then keeps the low 32 bits.
	for (const auto& test : cases)
	{
		SCOPED_TRACE(test.token);
		EXPECT_EQ(INI::scanInt(test.token), test.signedExpected);
		EXPECT_EQ(INI::scanUnsignedInt(test.token), test.unsignedExpected);
#ifdef _WIN32
		// Characterize the Windows CRT; out-of-range scanf conversions are not portable.
		Int legacySigned;
		UnsignedInt legacyUnsigned;
		ASSERT_EQ(std::sscanf(test.token, "%d", &legacySigned), 1);
		ASSERT_EQ(std::sscanf(test.token, "%u", &legacyUnsigned), 1);
		EXPECT_EQ(legacySigned, test.signedExpected);
		EXPECT_EQ(legacyUnsigned, test.unsignedExpected);
#endif
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

TEST(INI, RealRangeErrorsMatchLegacyParser)
{
	const Real infinity = std::numeric_limits<Real>::infinity();
	const Real subnormal = std::numeric_limits<Real>::denorm_min();
	const struct {
		const char* token;
		Real expected;
	} cases[] = {
		{"1e100",         infinity},
		{"+1e100",        infinity},
		{"-1e100",       -infinity},
		{"3.5e38",        infinity},
		{"1e100suffix",   infinity},
		{"1e-100",            0.0f},
		{"+1e-100",           0.0f},
		{"-1e-100",          -0.0f},
		{"1e-45",        subnormal},
		{"-1e-45",      -subnormal},
		{"+1.25",            1.25f}
	};
	for (const auto& test : cases)
	{
		SCOPED_TRACE(test.token);
		const Real actual = INI::scanReal(test.token);
		EXPECT_EQ(actual, test.expected);
		EXPECT_EQ(std::signbit(actual), std::signbit(test.expected));
#ifdef _WIN32
		// As above, scanf is a Windows compatibility check, not a portable overflow oracle.
		Real legacy;
		ASSERT_EQ(std::sscanf(test.token, "%f", &legacy), 1);
		EXPECT_EQ(legacy, test.expected);
		EXPECT_EQ(std::signbit(legacy), std::signbit(test.expected));
#endif
	}
}

TEST(INI, InvalidRealsAreRejected)
{
	for (const char* token : {"abc", "+", "-", "+-1e100", "+-1e-100"})
	{
		SCOPED_TRACE(token);
		EXPECT_THROW(INI::scanReal(token), decltype(INI_INVALID_DATA));
	}
}
