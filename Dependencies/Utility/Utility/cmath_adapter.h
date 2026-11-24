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

// This file provides std::isnan and std::isfinite. The <cmath> of VC6 and STLport 4.5.3 has neither, and VC6
// gets functions that call _isnan and _finite of <float.h>.
//
// The VC6 functions differ from the standard functions in these points:
// - There are only float and double overloads. An integer or long double argument is ambiguous.
// - A float argument is converted to double before the check. NaN and infinity stay NaN and infinity in that
//   conversion, so the result is the same.

#pragma once

#if defined(_MSC_VER) && _MSC_VER < 1300

#include <float.h>

namespace std
{
inline bool isnan(float x)
{
	return _isnan(x) != 0;
}

inline bool isnan(double x)
{
	return _isnan(x) != 0;
}

inline bool isfinite(float x)
{
	return _finite(x) != 0;
}

inline bool isfinite(double x)
{
	return _finite(x) != 0;
}
}

#else

#include <cmath>

#endif
