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

#include "ICU/IcuLoader.h"

#if defined(RTS_ICU_DYNAMIC) || defined(RTS_HAS_ICU_WINSDK)

#include <Utility/interlocked_adapter.h>

const IcuLoader& IcuLoader::get()
{
    // VC6 does not synchronize function-local static construction. Serialize it
    // on every toolchain, including publication of the resolved function pointers.
    // LONG* works with both VC6 and current SDK InterlockedExchange signatures.
    static LONG gate = 0;
    while (InterlockedCompareExchange(&gate, 1, 0) != 0)
    {
        Sleep(0);
    }

    static IcuLoader loader;
    InterlockedExchange(&gate, 0);
    return loader;
}

IcuLoader::IcuLoader() : m_module(nullptr), m_fromUtf8(nullptr), m_toUtf8WithSub(nullptr)
{
    // Match the Windows SDK delay loader's DLL search order, including app-local ICU.
    m_module = LoadLibraryA("icu.dll");
    if (m_module == nullptr)
    {
        return;
    }

    m_fromUtf8 = reinterpret_cast<StrFromUtf8>(GetProcAddress(m_module, "u_strFromUTF8"));
    m_toUtf8WithSub = reinterpret_cast<StrToUtf8WithSub>(GetProcAddress(m_module, "u_strToUTF8WithSub"));
    if (m_fromUtf8 == nullptr || m_toUtf8WithSub == nullptr)
    {
        FreeLibrary(m_module);
        m_module = nullptr;
        m_fromUtf8 = nullptr;
        m_toUtf8WithSub = nullptr;
    }
}

IcuLoader::~IcuLoader()
{
    if (m_module != nullptr)
    {
        FreeLibrary(m_module);
    }
}

bool IcuLoader::isAvailable() const
{
    return m_module != nullptr;
}

#endif
