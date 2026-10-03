/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
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

#if defined(RTS_ICU_DYNAMIC) || defined(RTS_HAS_ICU_WINSDK)

#include <windows.h>

// Loads icu.dll on the first conversion and caches success or failure until unload.
// Conversion calls and unload are serialized with a Windows critical section,
// including on VC6. Function pointers stay private and cannot outlive the DLL.
// An explicit unload releases the DLL and allows the next conversion to retry.
class IcuLoader
{
public:
    // These types match the Windows ICU C ABI, including its 32-bit error enum.
    typedef unsigned short Char;
    typedef int Char32;
    typedef int ErrorCode;

    static bool isAvailable();
    static void unload();

    // Return false when ICU is unavailable; otherwise call ICU and return true.
    // The caller checks error for the conversion result, including preflight.
    static bool fromUtf8(Char* dest, int capacity, int* length, const char* src, int srcLength, ErrorCode* error);
    static bool toUtf8WithSub(char* dest, int capacity, int* length, const Char* src, int srcLength,
        Char32 substitution, int* substitutions, ErrorCode* error);

private:
    IcuLoader();
    IcuLoader(const IcuLoader&);
    IcuLoader& operator=(const IcuLoader&);
};

#endif
