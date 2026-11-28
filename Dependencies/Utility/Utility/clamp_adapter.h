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

// This file provides std::clamp. The <algorithm> of VC6 and STLport 4.5.3 does not have it.
//
// The VC6 function differs from the standard function in these points:
// - It is not constexpr.
// - There is no overload that takes a comparison function.

#pragma once

#if defined(_MSC_VER) && _MSC_VER < 1300

namespace std
{
template <class T>
inline const T& clamp(const T& value, const T& low, const T& high)
{
	return value < low ? low : high < value ? high : value;
}
}

#else

#include <algorithm>

#endif
