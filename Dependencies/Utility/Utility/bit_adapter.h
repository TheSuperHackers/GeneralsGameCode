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

// This file provides std::bit_cast. VC6 has no <bit> header and gets a function that copies the bytes with memcpy.
// Use it to reinterpret the bits of a float as an integer or the other way around, instead of reading the value
// through a pointer of the other type, which violates the strict aliasing rule.
//
// The VC6 function differs from the standard function in these points:
// - It is not constexpr.
// - It does not check that the types are trivially copyable.
// - The destination type must be default constructible.

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
}

#else

#include <bit>

#endif
