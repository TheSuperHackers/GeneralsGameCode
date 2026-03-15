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

#if defined(__MINGW32__) || defined(__MINGW64__)

/*
**	MinGW-w64 provides most mathematical constants in <math.h> when _USE_MATH_DEFINES is set
**	(which is done globally in cmake/mingw.cmake).
**
**	However, MinGW's <math.h> is missing one constant (M_1_SQRTPI) and uses a different name
**	for another (M_SQRT1_2 instead of M_SQRT_2). We define those here to match MSVC and Watcom.
**
**	This brings MinGW in line with MSVC (visualc.h) and Watcom (watcom.h)
**	which provide all 14 mathematical constants.
*/

#include <math.h>

/*
**	M_1_SQRTPI is not defined by MinGW's math.h
**	Define it to match visualc.h and watcom.h
*/
#ifndef M_1_SQRTPI
#define M_1_SQRTPI  0.564189583547756286948
#endif

/*
**	MinGW defines M_SQRT1_2 instead of M_SQRT_2
**	Both represent 1/sqrt(2), just different naming
**	Create an alias for compatibility
*/
#ifndef M_SQRT_2
#define M_SQRT_2    M_SQRT1_2
#endif

/*
**	At this point, all 14 mathematical constants are available:
**	M_E, M_LOG2E, M_LOG10E, M_LN2, M_LN10,
**	M_PI, M_PI_2, M_PI_4, M_1_PI, M_2_PI,
**	M_1_SQRTPI, M_2_SQRTPI, M_SQRT2, M_SQRT_2
*/

#endif
