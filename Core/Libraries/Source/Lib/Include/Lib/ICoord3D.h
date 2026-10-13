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

#include "ICoord2D.h"

// NOTE: Keep the types simple; avoid constructors and destructors so they can be used within unions

struct ICoord3D
{
	Int x, y, z;

	ICoord2D asICoord2D() const
	{
		const ICoord2D xy = { x, y };
		return xy;
	}

	Int length() const { return (Int)sqrt( (double)(x*x + y*y + z*z) ); }
	Int lengthSqr() const { return x*x + y*y + z*z; }

	void zero()
	{
		x = 0;
		y = 0;
		z = 0;
	}

	bool is(Int value) const
	{
		return x == value && y == value && z == value;
	}

	void add( const ICoord3D &a )
	{
		x += a.x;
		y += a.y;
		z += a.z;
	}

	void sub( const ICoord3D &a )
	{
		x -= a.x;
		y -= a.y;
		z -= a.z;
	}

	void operator+=( const ICoord3D &a )
	{
		add(a);
	}

	void operator-=( const ICoord3D &a )
	{
		sub(a);
	}

	ICoord3D operator+() const
	{
		return *this;
	}

	ICoord3D operator-() const
	{
		ICoord3D c;
		c.x = -x;
		c.y = -y;
		c.z = -z;
		return c;
	}

	void set( const ICoord3D &a )
	{
		x = a.x;
		y = a.y;
		z = a.z;
	}

	void set( Int ax, Int ay, Int az )
	{
		x = ax;
		y = ay;
		z = az;
	}

	void updateMin( const ICoord3D &other )
	{
		if (x > other.x)
			x = other.x;

		if (y > other.y)
			y = other.y;

		if (z > other.z)
			z = other.z;
	}

	void updateMax( const ICoord3D &other )
	{
		if (x < other.x)
			x = other.x;

		if (y < other.y)
			y = other.y;

		if (z < other.z)
			z = other.z;
	}
};

inline ICoord3D operator+( const ICoord3D &a, const ICoord3D &b )
{
	ICoord3D c = a;
	c.add(b);
	return c;
}

inline ICoord3D operator-( const ICoord3D &a, const ICoord3D &b )
{
	ICoord3D c = a;
	c.sub(b);
	return c;
}
