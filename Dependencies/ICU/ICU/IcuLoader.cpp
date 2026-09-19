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
#include <string.h>
#include <wchar.h>

namespace
{

LONG Gate = 0;
unsigned int ReferenceCount = 0;
HMODULE Module = nullptr;
IcuLoader::StrFromUtf8 FromUtf8 = nullptr;
IcuLoader::StrToUtf8WithSub ToUtf8WithSub = nullptr;

class LoaderLock
{
public:
    LoaderLock()
    {
        // LONG* works with both VC6 and current SDK interlocked signatures.
        while (InterlockedCompareExchange(&Gate, 1, 0) != 0)
        {
            Sleep(0);
        }
    }

    ~LoaderLock()
    {
        InterlockedExchange(&Gate, 0);
    }

private:
    LoaderLock(const LoaderLock&);
    LoaderLock& operator=(const LoaderLock&);
};

HMODULE loadModule()
{
    // Use absolute paths so -setCwd and PATH cannot supply an unexpected DLL.
    // Wide paths also allow app-local ICU in non-ASCII installation directories.
    wchar_t path[MAX_PATH];
    const wchar_t name[] = L"icu.dll";
    const DWORD length = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (length > 0 && length < MAX_PATH)
    {
        wchar_t* separator = wcsrchr(path, L'\\');
        if (separator != nullptr && separator + 1 - path + sizeof(name) / sizeof(name[0]) <= MAX_PATH)
        {
            memcpy(separator + 1, name, sizeof(name));
            HMODULE module = LoadLibraryW(path);
            if (module != nullptr)
            {
                return module;
            }
        }
    }

    const UINT systemLength = GetSystemDirectoryW(path, MAX_PATH);
    if (systemLength > 0 && systemLength + 1 + sizeof(name) / sizeof(name[0]) <= MAX_PATH)
    {
        path[systemLength] = L'\\';
        memcpy(path + systemLength + 1, name, sizeof(name));
        return LoadLibraryW(path);
    }

    return nullptr;
}

void freeResources()
{
    if (Module != nullptr)
    {
        FreeLibrary(Module);
        Module = nullptr;
    }

    FromUtf8 = nullptr;
    ToUtf8WithSub = nullptr;
}

} // namespace

bool IcuLoader::load()
{
    LoaderLock lock;
    // Failed attempts also retain a reference so overlapping callers share the
    // result. Once all callers unload, a later load can retry.
    if (++ReferenceCount > 1)
    {
        return isLoaded();
    }

    Module = loadModule();
    if (Module == nullptr)
    {
        return false;
    }

    FromUtf8 = reinterpret_cast<StrFromUtf8>(GetProcAddress(Module, "u_strFromUTF8"));
    ToUtf8WithSub = reinterpret_cast<StrToUtf8WithSub>(GetProcAddress(Module, "u_strToUTF8WithSub"));
    if (FromUtf8 == nullptr || ToUtf8WithSub == nullptr)
    {
        freeResources();
        return false;
    }

    return true;
}

void IcuLoader::unload()
{
    LoaderLock lock;
    if (ReferenceCount == 0)
    {
        return;
    }

    if (--ReferenceCount == 0)
    {
        freeResources();
    }
}

bool IcuLoader::isLoaded()
{
    return Module != nullptr;
}

IcuLoader::StrFromUtf8 IcuLoader::fromUtf8()
{
    return FromUtf8;
}

IcuLoader::StrToUtf8WithSub IcuLoader::toUtf8WithSub()
{
    return ToUtf8WithSub;
}

#endif

IcuScope::IcuScope()
{
#if defined(RTS_ICU_DYNAMIC) || defined(RTS_HAS_ICU_WINSDK)
    m_available = IcuLoader::load();
#elif defined(RTS_HAS_ICU)
    m_available = true;
#else
    m_available = false;
#endif
}

IcuScope::~IcuScope()
{
#if defined(RTS_ICU_DYNAMIC) || defined(RTS_HAS_ICU_WINSDK)
    IcuLoader::unload();
#endif
}
