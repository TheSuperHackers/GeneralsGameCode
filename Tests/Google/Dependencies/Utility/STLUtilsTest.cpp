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

#include <iterator>
#include <map>
#include <string>

#include "Utility/STLUtils.h"

namespace
{

typedef std::multimap<int, std::string> Multimap;

// Returns the elements in their order, as "key:value" separated by spaces.
std::string elements(const Multimap& mm)
{
	std::string result;
	for (Multimap::const_iterator it = mm.begin(); it != mm.end(); ++it)
	{
		if (!result.empty())
		{
			result += ' ';
		}
		result += std::to_string(it->first) + ':' + it->second;
	}
	return result;
}

Multimap::iterator at(Multimap& mm, ptrdiff_t index)
{
	Multimap::iterator it = mm.begin();
	std::advance(it, index);
	return it;
}

Multimap makeMultimap()
{
	Multimap mm;
	mm.insert(Multimap::value_type(0, "x"));
	mm.insert(Multimap::value_type(1, "a"));
	mm.insert(Multimap::value_type(1, "b"));
	mm.insert(Multimap::value_type(1, "c"));
	mm.insert(Multimap::value_type(1, "d"));
	mm.insert(Multimap::value_type(2, "y"));
	return mm;
}

} // namespace

TEST(STLUtils, MoveMappedValueToFrontKeepsValueAtFront)
{
	Multimap mm = makeMultimap();

	stl::move_mapped_value_to_front(at(mm, 1), at(mm, 1));

	EXPECT_EQ(elements(mm), "0:x 1:a 1:b 1:c 1:d 2:y");
}

TEST(STLUtils, MoveMappedValueToFrontMovesValueByOne)
{
	Multimap mm = makeMultimap();

	stl::move_mapped_value_to_front(at(mm, 1), at(mm, 2));

	EXPECT_EQ(elements(mm), "0:x 1:b 1:a 1:c 1:d 2:y");
}

// The values in between move back by one and keep their order. The keys and the other elements stay in place.
TEST(STLUtils, MoveMappedValueToFrontShiftsValuesInBetween)
{
	Multimap mm = makeMultimap();

	stl::move_mapped_value_to_front(at(mm, 1), at(mm, 4));

	EXPECT_EQ(elements(mm), "0:x 1:d 1:a 1:b 1:c 2:y");
}

TEST(STLUtils, MoveMappedValueToFrontWithinRange)
{
	Multimap mm = makeMultimap();

	stl::move_mapped_value_to_front(at(mm, 2), at(mm, 4));

	EXPECT_EQ(elements(mm), "0:x 1:a 1:d 1:b 1:c 2:y");
}
