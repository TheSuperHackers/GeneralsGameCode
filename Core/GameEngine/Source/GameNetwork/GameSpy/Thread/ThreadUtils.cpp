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

//-------------------------------------------------------------------------

std::wstring MultiByteToWideCharSingleLine( const char *orig )
{
	if (orig == nullptr)
		return std::wstring();

	const Int requiredLength = MultiByteToWideChar(CP_UTF8, 0, orig, -1, nullptr, 0);
	if (requiredLength <= 0)
		return std::wstring();

	WideChar *dest = NEW WideChar[requiredLength];
	if (MultiByteToWideChar(CP_UTF8, 0, orig, -1, dest, requiredLength) != requiredLength)
	{
		delete[] dest;
		return std::wstring();
	}

	for (Int i = 0; i < requiredLength - 1; ++i)
	{
		if (dest[i] == L'\n' || dest[i] == L'\r')
			dest[i] = L' ';
	}

	const std::wstring ret = dest;
	delete[] dest;
	return ret;
}

std::string WideCharStringToMultiByte( const WideChar *orig )
{
	if (orig == nullptr)
		return std::string();

	const Int requiredLength = WideCharToMultiByte(CP_UTF8, 0, orig, -1, nullptr, 0, nullptr, nullptr);
	if (requiredLength <= 0)
		return std::string();

	char *dest = NEW char[requiredLength];
	if (WideCharToMultiByte(CP_UTF8, 0, orig, -1, dest, requiredLength, nullptr, nullptr) != requiredLength)
	{
		delete[] dest;
		return std::string();
	}

	const std::string ret = dest;
	delete[] dest;
	return ret;
}

//-------------------------------------------------------------------------

