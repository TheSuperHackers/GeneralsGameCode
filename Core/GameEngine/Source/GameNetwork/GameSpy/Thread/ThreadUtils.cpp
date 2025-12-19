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

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

// FILE: ThreadUtils.cpp //////////////////////////////////////////////////////
// GameSpy thread utils
// Author: Matthew D. Campbell, July 2002

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine
#include "utf8.h"

//-------------------------------------------------------------------------

std::wstring MultiByteToWideCharSingleLine( const char *orig )
{
	const size_t size = get_size_as_widechar( orig );
	WideChar *dest = NEW WideChar[size / sizeof(WideChar)];

	convert_utf8_to_widechar( orig, dest, size );
	WideChar *c = NULL;
	do
	{
		c = wcschr(dest, L'\n');
		if (c)
		{
			*c = L' ';
		}
	}
	while ( c != NULL );
	do
	{
		c = wcschr(dest, L'\r');
		if (c)
		{
			*c = L' ';
		}
	}
	while ( c != NULL );

	std::wstring ret = dest;
	delete[] dest;
	return ret;
}

std::string WideCharStringToMultiByte( const WideChar *orig )
{
	std::string ret;
	const size_t size = get_size_as_utf8( orig );
	if (size > 0)
	{
		char *dest = NEW char[size];
		convert_widechar_to_utf8( orig, dest, size );
		ret = dest;
		delete[] dest;
	}
	return ret;
}

//-------------------------------------------------------------------------

