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

#include "Coord3D.h"
#include "Region2D.h"

// NOTE: Keep the types simple; avoid constructors and destructors so they can be used within unions

// For alternative see AABoxClass
struct Region3D
{
	Coord3D lo, hi;						// axis-aligned bounding box

	// Keep only the overlapping portion of both regions.
	void intersectWith( const Region3D &other )
	{
		lo.updateMax(other.lo);
		hi.updateMin(other.hi);
	}

	// Expand to include the other region.
	void uniteWith( const Region3D &other )
	{
		lo.updateMin(other.lo);
		hi.updateMax(other.hi);
	}

	// Expand to include the point.
	void uniteWith( const Coord3D &point )
	{
		lo.updateMin(point);
		hi.updateMax(point);
	}

	Real width() const { return hi.x - lo.x; }
	Real height() const { return hi.y - lo.y; }
	Real depth() const { return hi.z - lo.z; }

	void zero() { lo.zero(); hi.zero(); }

	bool is(Real value) const
	{
		return lo.is(value) && hi.is(value);
	}

	// Set XY from a 2D region and leave Z unchanged.
	void setXY(const Region2D &region)
	{
		lo.x = region.lo.x;
		lo.y = region.lo.y;
		hi.x = region.hi.x;
		hi.y = region.hi.y;
	}

	// Set XY from any amount of points and leave Z unchanged.
	void setXYFromPoints(const Coord3D* points, Int count)
	{
		lo.x = points[0].x;
		lo.y = points[0].y;
		hi.x = points[0].x;
		hi.y = points[0].y;
		for (Int i = 1; i < count; ++i)
		{
			if (points[i].x < lo.x)
				lo.x = points[i].x;
			if (points[i].y < lo.y)
				lo.y = points[i].y;
			if (points[i].x > hi.x)
				hi.x = points[i].x;
			if (points[i].y > hi.y)
				hi.y = points[i].y;
		}
	}

	void setFromPoints(const Coord3D* points, Int count)
	{
		lo = points[0];
		hi = points[0];
		for (Int i = 1; i < count; ++i)
		{
			uniteWith(points[i]);
		}
	}

	Bool isInRegionNoZ( const Coord3D &query ) const
	{
		return (lo.x < query.x) && (query.x < hi.x) &&
					 (lo.y < query.y) && (query.y < hi.y);
	}

	Bool isInRegion( const Coord3D &query ) const
	{
		return (lo.x < query.x) && (query.x < hi.x) &&
					 (lo.y < query.y) && (query.y < hi.y) &&
					 (lo.z < query.z) && (query.z < hi.z);
	}
};
