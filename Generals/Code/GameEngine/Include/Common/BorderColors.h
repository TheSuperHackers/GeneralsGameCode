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
	const char *m_colorName;
	long m_borderColor;
};

const BorderColor BORDER_COLORS[] = 
{
	{ "Orange",					0xFFFF8700, },
	{ "Green",					0xFF00FF00, },
	{ "Blue",						0xFF0000FF, },
	{ "Cyan",						0xFF00FFFF, },
	{ "Magenta",				0xFFFF00FF, },
	{ "Yellow",					0xFFFFFF00, },
	{ "Purple",					0xFF9E00FF, },
	{ "Pink",						0xFFFF8670, },
};

const long BORDER_COLORS_SIZE = sizeof(BORDER_COLORS) / sizeof (BORDER_COLORS[0]);
