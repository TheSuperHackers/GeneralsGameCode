/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// Trig.cpp
// fast trig functions
// Author: Michael S. Booth, March 1994
// Converted to Generals by Matthew D. Campbell, February 2002

#include <math.h>

#include "Lib/BaseDefines.h"
#include "Lib/trig.h"

#if USE_DETERMINISTIC_MATH
#include "gmath.h"
#endif

Real Sin(Real x)
{
#if USE_DETERMINISTIC_MATH
	return gm_sinf(x);
#else
	return sinf(x);
#endif
}

Real Cos(Real x)
{
#if USE_DETERMINISTIC_MATH
	return gm_cosf(x);
#else
	return cosf(x);
#endif
}

Real Tan(Real x)
{
#if USE_DETERMINISTIC_MATH
	return gm_tanf(x);
#else
	return tanf(x);
#endif
}

Real ACos(Real x)
{
#if USE_DETERMINISTIC_MATH
	return gm_acosf(x);
#else
	return acosf(x);
#endif
}

Real ASin(Real x)
{
#if USE_DETERMINISTIC_MATH
	return gm_asinf(x);
#else
	return asinf(x);
#endif
}

Real Sqrt(Real x)
{
#if USE_DETERMINISTIC_MATH
	return gm_sqrtf(x);
#else
	return (Real)sqrt((double)x);
#endif
}

double Sqrt(double x)
{
#if USE_DETERMINISTIC_MATH
	return gm_sqrt(x);
#else
	return sqrt(x);
#endif
}
