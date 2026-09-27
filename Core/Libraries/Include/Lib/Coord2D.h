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

#include "trig.h"

// NOTE: Keep the types simple; avoid constructors and destructors so they can be used within unions

struct Coord2D
{
	Real x, y;

	void zero()
	{
		x = 0.0f;
		y = 0.0f;
	}

	bool is(Real value) const
	{
		return x == value && y == value;
	}

	Real length() const { return (Real)sqrt( x*x + y*y ); }
	Real lengthSqr() const { return x*x + y*y; }

	void normalize()
	{
		Real len = length();
		if( len != 0 )
		{
			x /= len;
			y /= len;
		}
	}

	Real toAngle() const;  ///< turn 2D vector into angle (where angle 0 is down the +x axis)

	void add( const Coord2D &a )
	{
		x += a.x;
		y += a.y;
	}

	void sub( const Coord2D &a )
	{
		x -= a.x;
		y -= a.y;
	}

	void operator+=( const Coord2D &a )
	{
		add(a);
	}

	void operator-=( const Coord2D &a )
	{
		sub(a);
	}

	Coord2D operator+() const
	{
		return *this;
	}

	Coord2D operator-() const
	{
		Coord2D c;
		c.x = -x;
		c.y = -y;
		return c;
	}

	void set( const Coord2D &a )
	{
		x = a.x;
		y = a.y;
	}

	void set( Real ax, Real ay )
	{
		x = ax;
		y = ay;
	}

	void updateMin( const Coord2D &other )
	{
		if (x > other.x)
			x = other.x;

		if (y > other.y)
			y = other.y;
	}

	void updateMax( const Coord2D &other )
	{
		if (x < other.x)
			x = other.x;

		if (y < other.y)
			y = other.y;
	}
};

inline Coord2D operator+( const Coord2D &a, const Coord2D &b )
{
	Coord2D c = a;
	c.add(b);
	return c;
}

inline Coord2D operator-( const Coord2D &a, const Coord2D &b )
{
	Coord2D c = a;
	c.sub(b);
	return c;
}

inline Real Coord2D::toAngle() const
{
#if RTS_GENERALS /*&& RETAIL_COMPATIBLE_CRC*/
	Coord2D vector;

	vector.x = x;
	vector.y = y;

	Real dist = (Real)sqrt(vector.x * vector.x + vector.y * vector.y);

	// normalize
	if (dist == 0.0f)
		return 0.0f;

	Coord2D dir;
	dir.x = 1.0f;
	dir.y = 0.0f;

	Real distInv = 1.0f / dist;
	vector.x *= distInv;
	vector.y *= distInv;

	// dot of two unit vectors is cos of angle
	Real c = dir.x*vector.x + dir.y*vector.y;

	// bound it in case of numerical error
	if (c < -1.0)
		c = -1.0;
	else if (c > 1.0)
		c = 1.0;

	Real value = (Real)ACos( (Real)c );

	// Determine sign by checking Z component of dir cross vector
	// Note this is assumes 2D, and is identical to dotting the perpendicular of v with dir
	Real perpZ = dir.x * vector.y - dir.y * vector.x;
	if (perpZ < 0.0f)
		value = -value;

	// note: to make this 3D, 'dir' and 'vector' can be normalized and dotted just as they are
	// to test sign, compute N = dir X vector, then P = N x dir, then S = P . vector, where sign of
	// S is sign of angle - MSB

	return value;
#else
	const Real len = length();
	if (len == 0.0f)
		return 0.0f;

	Real c = x/len;
	// bound it in case of numerical error
	if (c < -1.0f)
		c = -1.0f;
	else if (c > 1.0f)
		c = 1.0f;

	return y < 0.0f ? -ACos(c) : ACos(c);
#endif
}
