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

#include "WWLib/utf8.h"

//-------------------------------------------------------------------------

// TheSuperHackers @refactor bobtista 02/04/2026 Use WWLib UTF-8 functions instead of raw Win32 API calls
// TheSuperHackers @refactor CryoTheRenegade 04/08/2026 Use the shared ICU4C UTF conversion functions.
std::wstring MultiByteToWideCharSingleLine( const char* orig )
{
	const size_t srcLen = strlen(orig);
	const size_t len = Utf8_To_Wide_Len(orig, srcLen);
	if (len == 0)
	{
		return std::wstring();
	}

	std::wstring ret;
	if (len == UTF8_INVALID)
	{
		ret.resize(srcLen);
		for (size_t i = 0; i < srcLen; ++i)
		{
			ret[i] = static_cast<WideChar>(static_cast<unsigned char>(orig[i]));
		}
	}
	else
	{
		ret.resize(len);
		Utf8_To_Wide(&ret[0], len, orig, srcLen);
	}

	for (size_t i = 0; i < ret.size(); ++i)
	{
		if (ret[i] == L'\n' || ret[i] == L'\x0D')
		{
			ret[i] = L' ';
		}
	}

	return ret;
}

std::string WideCharStringToMultiByte( const WideChar* orig )
{
	const size_t srcLen = wcslen(orig);
	const size_t len = Wide_To_Utf8_Len(orig, srcLen);
	if (len == 0)
	{
		return std::string();
	}

	std::string ret;
	ret.resize(len);
	Wide_To_Utf8(&ret[0], len, orig, srcLen);
	return ret;
}

//-------------------------------------------------------------------------

