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

struct RGBColor
{
	Real red, green, blue;		// range between 0 and 1

	Int getAsInt() const
	{
		return
			((Int)(red * 255.0) << 16) |
			((Int)(green * 255.0) << 8) |
			((Int)(blue * 255.0) << 0);
	}

	void setFromInt(Int c)
	{
		red = ((c >> 16) & 0xff) / 255.0f;
		green = ((c >>  8) & 0xff) / 255.0f;
		blue = ((c >>  0) & 0xff) / 255.0f;
	}

	RGBColor& operator+=(const RGBColor& c)
	{
		red += c.red;
		green += c.green;
		blue += c.blue;
		return *this;
	}

	RGBColor& operator-=(const RGBColor& c)
	{
		red -= c.red;
		green -= c.green;
		blue -= c.blue;
		return *this;
	}

	RGBColor& operator*=(const RGBColor& c)
	{
		red *= c.red;
		green *= c.green;
		blue *= c.blue;
		return *this;
	}

	RGBColor& operator/=(const RGBColor& c)
	{
		red /= c.red;
		green /= c.green;
		blue /= c.blue;
		return *this;
	}

	RGBColor operator+(const RGBColor& c) const
	{
		RGBColor res = *this;
		res += c;
		return res;
	}

	RGBColor operator-(const RGBColor& c) const
	{
		RGBColor res = *this;
		res -= c;
		return res;
	}

	RGBColor operator*(const RGBColor& c) const
	{
		RGBColor res = *this;
		res *= c;
		return res;
	}

	RGBColor operator/(const RGBColor& c) const
	{
		RGBColor res = *this;
		res /= c;
		return res;
	}

	RGBColor& operator+=(Real s)
	{
		red += s;
		green += s;
		blue += s;
		return *this;
	}

	RGBColor& operator-=(Real s)
	{
		red -= s;
		green -= s;
		blue -= s;
		return *this;
	}

	RGBColor& operator*=(Real s)
	{
		red *= s;
		green *= s;
		blue *= s;
		return *this;
	}

	RGBColor& operator/=(Real s)
	{
		red /= s;
		green /= s;
		blue /= s;
		return *this;
	}

	RGBColor operator+(Real s) const
	{
		RGBColor res = *this;
		res += s;
		return res;
	}

	RGBColor operator-(Real s) const
	{
		RGBColor res = *this;
		res -= s;
		return res;
	}

	RGBColor operator*(Real s) const
	{
		RGBColor res = *this;
		res *= s;
		return res;
	}

	RGBColor operator/(Real s) const
	{
		RGBColor res = *this;
		res /= s;
		return res;
	}

};

struct RGBAColorReal
{

	Real red, green, blue, alpha;  // range between 0.0 and 1.0

};

struct RGBAColorInt
{

	UnsignedInt red, green, blue, alpha;  // range between 0 and 255

};
