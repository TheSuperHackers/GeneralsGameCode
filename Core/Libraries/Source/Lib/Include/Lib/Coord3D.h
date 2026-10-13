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

#include "Coord2D.h"

// NOTE: Keep the types simple; avoid constructors and destructors so they can be used within unions

struct Coord3D
{
	Real x, y, z;

	Coord2D asCoord2D() const
	{
		const Coord2D xy = { x, y };
		return xy;
	}

	Real length() const { return (Real)sqrt( x*x + y*y + z*z ); }
	Real lengthSqr() const { return ( x*x + y*y + z*z ); }

	void normalize()
	{
		Real len = length();

		if( len != 0 )
		{
			x /= len;
			y /= len;
			z /= len;
		}
	}

	static void crossProduct( const Coord3D &a, const Coord3D &b, Coord3D &r )
	{
		r.x = (a.y * b.z - a.z * b.y);
		r.y = (a.z * b.x - a.x * b.z);
		r.z = (a.x * b.y - a.y * b.x);
	}

	void zero()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
	}

	bool is(Real value) const
	{
		return x == value && y == value && z == value;
	}

	void add( const Coord3D &a )
	{
		x += a.x;
		y += a.y;
		z += a.z;
	}

	void sub( const Coord3D &a )
	{
		x -= a.x;
		y -= a.y;
		z -= a.z;
	}

	void operator+=( const Coord3D &a )
	{
		add(a);
	}

	void operator-=( const Coord3D &a )
	{
		sub(a);
	}

	Coord3D operator+() const
	{
		return *this;
	}

	Coord3D operator-() const
	{
		Coord3D c;
		c.x = -x;
		c.y = -y;
		c.z = -z;
		return c;
	}

	void set( const Coord3D &a )
	{
		x = a.x;
		y = a.y;
		z = a.z;
	}

	void set( Real ax, Real ay, Real az )
	{
		x = ax;
		y = ay;
		z = az;
	}

	void scale( Real scale )
	{
		x *= scale;
		y *= scale;
		z *= scale;
	}

	Bool equals( const Coord3D &r )
	{
		return (x == r.x &&
						y == r.y &&
						z == r.z);
	}

	Bool operator==( const Coord3D &r ) const
	{
		return (x == r.x &&
						y == r.y &&
						z == r.z);
	}

	void updateMin( const Coord3D &other )
	{
		if (x > other.x)
			x = other.x;

		if (y > other.y)
			y = other.y;

		if (z > other.z)
			z = other.z;
	}

	void updateMax( const Coord3D &other )
	{
		if (x < other.x)
			x = other.x;

		if (y < other.y)
			y = other.y;

		if (z < other.z)
			z = other.z;
	}
};

inline Coord3D operator+( const Coord3D &a, const Coord3D &b )
{
	Coord3D c = a;
	c.add(b);
	return c;
}

inline Coord3D operator-( const Coord3D &a, const Coord3D &b )
{
	Coord3D c = a;
	c.sub(b);
	return c;
}
