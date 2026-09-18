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

// Owns one reference to icu.dll until normal process shutdown. The Windows SDK
// uses it to check availability before a delay-loaded call; VC6 calls the two
// resolved functions directly without ICU headers or an import library.
class IcuLoader
{
public:
    // These types match the Windows ICU C ABI, including its 32-bit error enum.
    typedef unsigned short Char;
    typedef int Char32;
    typedef int ErrorCode;
    typedef Char* (__cdecl* StrFromUtf8)(Char*, int, int*, const char*, int, ErrorCode*);
    typedef char* (__cdecl* StrToUtf8WithSub)(char*, int, int*, const Char*, int, Char32, int*, ErrorCode*);

    // VC6 requires access from its generated static-destruction helper.
    ~IcuLoader();

    static const IcuLoader& get();
    bool isAvailable() const;

    StrFromUtf8 fromUtf8() const
    {
        return m_fromUtf8;
    }

    StrToUtf8WithSub toUtf8WithSub() const
    {
        return m_toUtf8WithSub;
    }

private:
    IcuLoader();
    IcuLoader(const IcuLoader&);
    IcuLoader& operator=(const IcuLoader&);

    HMODULE m_module;
    StrFromUtf8 m_fromUtf8;
    StrToUtf8WithSub m_toUtf8WithSub;
};

#endif
