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
#include <string>

#include "WWMathCalls.h"
#include "WWMathDumps.h"
#include "WWMathKeys.h"
#include "WWMathTestCase.h"

// Each WWMathKey is one test. The test runs the call of the key from wwmathCalls
// and compares its bits with the dump of the key from wwmathDumps, in the column
// of this build. Both tables hold the row of a key at the index of the key.

static std::string testNameOf(const ::testing::TestParamInfo<WWMathKey> &info)
{
	return wwmathKeyNames[toIndex(info.param)];
}

class WWMathBits : public ::testing::TestWithParam<WWMathKey>
{
protected:
	void SetUp() override
	{
		if (wwmathColumn == WWMATH_COLUMN_NONE)
		{
			GTEST_SKIP() << "No WWMath dump column for this build configuration";
		}
	}
};

TEST_P(WWMathBits, MatchesDump)
{
	const size_t index = toIndex(GetParam());
	const std::string computed = wwmathCalls[index].call();
	const char *dump = wwmathDumps[index].dump[wwmathColumn];
	ASSERT_NE(dump, nullptr) << "No dump in this column, computed " << computed;

	EXPECT_EQ(computed, dump);
}

INSTANTIATE_TEST_SUITE_P(WWMath, WWMathBits, ::testing::ValuesIn(wwmathKeys), testNameOf);
