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

#pragma once

#include <gtest/gtest.h>

#include <string.h>

using MemcchrFunction = const void* (*)(const void* data, int c, size_t n);

// Fills a buffer like "aaab" and checks that memcchr finds the first 'b'.
// If the searched length has no 'b', memcchr must return nullptr.
// Tests with variable lengths, and with variations of buffer contents.
inline void testMemcchr(MemcchrFunction memcchrFunction)
{
	const Int testCases[][3] = { { 'a', 'b', 'a' }, { 0xFF, 0x7F, -1 }, { 0x80, 0x00, 0x180 } };
	constexpr Int maxLength = 256;
	char buffer[maxLength + 1];
	for (Int length = 0; length <= maxLength; ++length)
	{
		for (Int firstB = 0; firstB <= length; ++firstB)
		{
			for (const auto& [a, b, c] : testCases)
			{
				memset(buffer, b, sizeof(buffer));
				memset(buffer, a, firstB);
				const void* expected = firstB < length ? buffer + firstB : nullptr;
				ASSERT_EQ(memcchrFunction(buffer, c, length), expected);
			}
		}
	}
}
