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

#include "ICoord3D.h"

// NOTE: Keep the types simple; avoid constructors and destructors so they can be used within unions

struct IRegion3D
{
	ICoord3D lo, hi;					// axis-aligned bounding box

	// Keep only the overlapping portion of both regions.
	void intersectWith( const IRegion3D &other )
	{
		lo.updateMax(other.lo);
		hi.updateMin(other.hi);
	}

	// Expand to include the other region.
	void uniteWith( const IRegion3D &other )
	{
		lo.updateMin(other.lo);
		hi.updateMax(other.hi);
	}

	// Expand to include the point.
	void uniteWith( const ICoord3D &point )
	{
		lo.updateMin(point);
		hi.updateMax(point);
	}

	void zero()
	{
		lo.zero();
		hi.zero();
	}

	bool is(Int value) const
	{
		return lo.is(value) && hi.is(value);
	}

	Int width() const { return hi.x - lo.x; }
	Int height() const { return hi.y - lo.y; }
	Int depth() const { return hi.z - lo.z; }
};
