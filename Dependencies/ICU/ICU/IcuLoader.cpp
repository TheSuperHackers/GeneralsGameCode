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

#include <string.h>
#include <wchar.h>

namespace
{

typedef IcuLoader::Char* (__cdecl* StrFromUtf8)(IcuLoader::Char*, int, int*, const char*, int, IcuLoader::ErrorCode*);
typedef char* (__cdecl* StrToUtf8WithSub)(char*, int, int*, const IcuLoader::Char*, int,
    IcuLoader::Char32, int*, IcuLoader::ErrorCode*);

bool LoadAttempted = false;
HMODULE Module = nullptr;
StrFromUtf8 FromUtf8 = nullptr;
StrToUtf8WithSub ToUtf8WithSub = nullptr;

void freeResources();

class LoaderCriticalSection
{
public:
    LoaderCriticalSection()
    {
        InitializeCriticalSection(&section);
    }

    ~LoaderCriticalSection()
    {
        // Also release ICU for tools that do not explicitly unload at shutdown.
        // Conversion workers must have stopped before static destruction.
        freeResources();
        DeleteCriticalSection(&section);
    }

    CRITICAL_SECTION section;

private:
    LoaderCriticalSection(const LoaderCriticalSection&);
    LoaderCriticalSection& operator=(const LoaderCriticalSection&);
};

LoaderCriticalSection CriticalSection;

class LoaderLock
{
public:
    LoaderLock()
    {
        EnterCriticalSection(&CriticalSection.section);
    }

    ~LoaderLock()
    {
        LeaveCriticalSection(&CriticalSection.section);
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

// The caller holds the critical section across loading and the ICU call.
bool load()
{
    if (LoadAttempted)
    {
        return Module != nullptr;
    }

    LoadAttempted = true;
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

} // namespace

bool IcuLoader::isAvailable()
{
    LoaderLock lock;
    return load();
}

void IcuLoader::unload()
{
    LoaderLock lock;
    freeResources();
    LoadAttempted = false;
}

bool IcuLoader::fromUtf8(Char* dest, int capacity, int* length, const char* src, int srcLength, ErrorCode* error)
{
    LoaderLock lock;
    if (!load())
    {
        return false;
    }

    FromUtf8(dest, capacity, length, src, srcLength, error);
    return true;
}

bool IcuLoader::toUtf8WithSub(char* dest, int capacity, int* length, const Char* src, int srcLength,
    Char32 substitution, int* substitutions, ErrorCode* error)
{
    LoaderLock lock;
    if (!load())
    {
        return false;
    }

    ToUtf8WithSub(dest, capacity, length, src, srcLength, substitution, substitutions, error);
    return true;
}

#endif
