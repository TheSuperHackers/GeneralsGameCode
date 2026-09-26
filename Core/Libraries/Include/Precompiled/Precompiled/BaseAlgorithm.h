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

#if __cplusplus >= 201103L
#include <type_traits>
#endif

#ifdef min
#error min is defined
#endif

#ifdef max
#error max is defined
#endif

template <typename T>
inline T min(T a, T b) { return (a < b) ? a : b; }

template <typename T>
inline T max(T a, T b) { return (a > b) ? a : b; }

template <typename NUM>
inline NUM clamp(NUM lo, NUM val, NUM hi)
{
	if (val < lo)
		return lo;
	else if (val > hi)
		return hi;
	else
		return val;
}

template <typename NUM>
inline NUM sqr(NUM x)
{
	return x*x;
}

template <typename NUM>
inline int sign(NUM x)
{
	if (x > 0)
		return 1;
	else if (x < 0)
		return -1;
	else
		return 0;
}

template <typename NUM>
inline NUM highestBit(NUM x)
{
	static_assert(sizeof(NUM) <= 8, "NUM must be 8 bytes or less");
	UnsignedInt64 y = static_cast<UnsignedInt64>(x);

	y |= (y >> 1);
	y |= (y >> 2);
	y |= (y >> 4);
	y |= (y >> 8);
	y |= (y >> 16);
	y |= (y >> 32);

	return static_cast<NUM>(y & ~(y >> 1));
}

template <typename PTR>
inline PTR maxPtr(PTR x, PTR y) noexcept
{
	static_assert(std::is_pointer<PTR>::value, "maxPtr is for pointer types only!");

	if (x == nullptr)
		return y;

	if (y == nullptr)
		return x;

	if (x > y)
		return x;

	return y;
}

template <typename PTR>
inline PTR minPtr(PTR x, PTR y) noexcept
{
	static_assert(std::is_pointer<PTR>::value, "minPtr is for pointer types only!");

	if (x == nullptr)
		return y;

	if (y == nullptr)
		return x;

	if (x < y)
		return x;

	return y;
}
