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

// Loads and unloads icu.dll with a shared reference count, like BinkLoader and
// MilesLoader. Every load needs a paired unload, even when loading fails.
// Load/unload are synchronized for conversion workers, including on VC6.
// Hold a reference while checking availability or using the resolved functions.
class IcuLoader
{
public:
    // These types match the Windows ICU C ABI, including its 32-bit error enum.
    typedef unsigned short Char;
    typedef int Char32;
    typedef int ErrorCode;
    typedef Char* (__cdecl* StrFromUtf8)(Char*, int, int*, const char*, int, ErrorCode*);
    typedef char* (__cdecl* StrToUtf8WithSub)(char*, int, int*, const Char*, int, Char32, int*, ErrorCode*);

    static bool load();
    static void unload();
    static bool isLoaded();
    static StrFromUtf8 fromUtf8();
    static StrToUtf8WithSub toUtf8WithSub();

private:
    IcuLoader();
    IcuLoader(const IcuLoader&);
    IcuLoader& operator=(const IcuLoader&);
};

#endif

// Keeps ICU available throughout a conversion or a longer application lifetime.
// Linked ICU needs no explicit DLL ownership; unavailable Windows ICU uses the
// Win32 conversion fallback. Scopes may overlap on different threads.
class IcuScope
{
public:
    IcuScope();
    ~IcuScope();

    bool isAvailable() const
    {
        return m_available;
    }

private:
    IcuScope(const IcuScope&);
    IcuScope& operator=(const IcuScope&);

    bool m_available;
};
