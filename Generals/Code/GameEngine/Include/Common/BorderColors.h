/*
**	Command & Conquer Generals(tm)
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


// jkmcd
#pragma once

struct BorderColor
{
	char *m_colorName;
	long m_borderColor;
};

const BorderColor BORDER_COLORS[] = 
{
	{ "Orange",					(int)0xFFFF8700, },
	{ "Green",					(int)0xFF00FF00, },
	{ "Blue",						(int)0xFF0000FF, },
	{ "Cyan",						(int)0xFF00FFFF, },
	{ "Magenta",				(int)0xFFFF00FF, },
	{ "Yellow",					(int)0xFFFFFF00, },
	{ "Purple",					(int)0xFF9E00FF, },
	{ "Pink",						(int)0xFFFF8670, },
};

const long BORDER_COLORS_SIZE = sizeof(BORDER_COLORS) / sizeof (BORDER_COLORS[0]);
