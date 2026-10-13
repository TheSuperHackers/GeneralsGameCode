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

#pragma once

#include "BaseFunctions.h"

// NOTE: Keep the types simple; avoid constructors and destructors so they can be used within unions

// real-valued range defined by low and high values
struct RealRange
{
	Real lo, hi;							// low and high values of the range

	void zero()
	{
		lo = 0.0f;
		hi = 0.0f;
	}

	bool is(Real value) const
	{
		return lo == value && hi == value;
	}

	// combine the given range with us such that we now encompass
	// both ranges
	void combine( RealRange &other )
	{
		lo = min( lo, other.lo );
		hi = max( hi, other.hi );
	}
};
