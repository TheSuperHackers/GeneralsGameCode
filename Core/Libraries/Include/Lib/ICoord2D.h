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

// NOTE: Keep the types simple; avoid constructors and destructors so they can be used within unions

struct ICoord2D
{
	Int x, y;

	void zero()
	{
		x = 0;
		y = 0;
	}

	bool is(Int value) const
	{
		return x == value && y == value;
	}

	Int length() const { return (Int)sqrt( (double)(x*x + y*y) ); }
	Int lengthSqr() const { return x*x + y*y; }

	void add( const ICoord2D &a )
	{
		x += a.x;
		y += a.y;
	}

	void sub( const ICoord2D &a )
	{
		x -= a.x;
		y -= a.y;
	}

	void operator+=( const ICoord2D &a )
	{
		add(a);
	}

	void operator-=( const ICoord2D &a )
	{
		sub(a);
	}

	ICoord2D operator+() const
	{
		return *this;
	}

	ICoord2D operator-() const
	{
		ICoord2D c;
		c.x = -x;
		c.y = -y;
		return c;
	}

	void set( const ICoord2D &a )
	{
		x = a.x;
		y = a.y;
	}

	void set( Int ax, Int ay )
	{
		x = ax;
		y = ay;
	}

	void updateMin( const ICoord2D &other )
	{
		if (x > other.x)
			x = other.x;

		if (y > other.y)
			y = other.y;
	}

	void updateMax( const ICoord2D &other )
	{
		if (x < other.x)
			x = other.x;

		if (y < other.y)
			y = other.y;
	}
};

inline ICoord2D operator+( const ICoord2D &a, const ICoord2D &b )
{
	ICoord2D c = a;
	c.add(b);
	return c;
}

inline ICoord2D operator-( const ICoord2D &a, const ICoord2D &b )
{
	ICoord2D c = a;
	c.sub(b);
	return c;
}
