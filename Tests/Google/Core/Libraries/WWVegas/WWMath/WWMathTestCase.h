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

#include <bit>
#include <cstddef>
#include <cstdint>
#include <format>
#include <ostream>
#include <string>

#include "WWMath/wwmath.h"

#include "WWMathKeys.h"

// The game simulation must compute the same bits from one build to the next.
// Every call has a dump of its bits, with one column per build configuration.
// VC6 does not build the tests, so the retail-compatible branch is checked on win32.
enum WWMathColumn
{
	WWMATH_COLUMN_NONE = -1,
	WWMATH_COLUMN_DETERMINISTIC,
	WWMATH_COLUMN_RETAIL_COMPATIBLE,
	WWMATH_COLUMN_COUNT
};

#if USE_DETERMINISTIC_MATH
static const WWMathColumn wwmathColumn = WWMATH_COLUMN_DETERMINISTIC;
#elif defined(_MSC_VER) && defined(_M_IX86)
static const WWMathColumn wwmathColumn = WWMATH_COLUMN_RETAIL_COMPATIBLE;
#else
static const WWMathColumn wwmathColumn = WWMATH_COLUMN_NONE;
#endif

inline void PrintTo(WWMathKey key, std::ostream *stream)
{
	*stream << wwmathKeyNames[toIndex(key)];
}

// A table must have one row per WWMathKey, in WWMathKey order, so the row of a key is at its index.
template <typename Entry, size_t Count>
constexpr bool hasEveryKeyInOrder(const Entry (&table)[Count])
{
	if (Count != WWMATH_KEY_COUNT)
	{
		return false;
	}

	for (size_t index = 0; index < Count; ++index)
	{
		if (toIndex(table[index].key) != index)
		{
			return false;
		}
	}

	return true;
}

template <typename T>
inline T runtimeValue(T value)
{
	volatile T runtime = value;
	return runtime;
}

inline std::string toBitsString(float value)
{
	return std::format("{:08X}", std::bit_cast<uint32_t>(value));
}

inline std::string toBitsString(double value)
{
	return std::format("{:016X}", std::bit_cast<uint64_t>(value));
}

inline std::string toBitsString(long value)
{
	return std::to_string(value);
}
