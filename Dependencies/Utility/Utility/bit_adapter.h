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

// This file provides std::bit_cast and std::bit_ceil. VC6 has no <bit> header and gets fallback functions: bit_cast
// copies the bytes with memcpy, and bit_ceil doubles 1 until it reaches the value.
//
// Use std::bit_cast to reinterpret the bits of a float as an integer or the other way around, instead of reading
// the value through a pointer of the other type, which violates the strict aliasing rule.
//
// The VC6 functions differ from the standard functions in these points:
// - They are not constexpr.
// - bit_cast does not check that the types are trivially copyable.
// - bit_cast requires the destination type to be default constructible.
// - bit_ceil does not check that the type is an unsigned integer.

#pragma once

#if defined(_MSC_VER) && _MSC_VER < 1300

#include <string.h>

namespace std
{
template <class To, class From>
inline To bit_cast(const From& from)
{
	static_assert_cpp98(sizeof(To) == sizeof(From), "The types must have the same size");
	To to;
	::memcpy(&to, &from, sizeof(To));
	return to;
}

template <class T>
inline T bit_ceil(T x)
{
	T result = 1;
	while (result < x)
		result <<= 1;

	return result;
}
}

#else

#include <bit>

#endif
