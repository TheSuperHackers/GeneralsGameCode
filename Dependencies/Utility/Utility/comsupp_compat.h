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

/**
 * @file comsupp_compat.h
 * @brief COM Support compatibility layer for MinGW-w64
 *
 * Provides _com_util::ConvertStringToBSTR() and ConvertBSTRToString()
 * as header-only implementations for MinGW-w64 builds.
 *
 * These functions are required by the _bstr_t class (from ReactOS comutil.h)
 * for char* <-> BSTR conversions. They are called internally when constructing
 * _bstr_t objects from C strings or converting _bstr_t back to char*.
 *
 * Used indirectly by:
 * - Core/Libraries/Source/WWVegas/WW3D2/dx8webbrowser.cpp (8+ _bstr_t constructions)
 * - Core/GameEngine/Include/GameNetwork/WOLBrowser/FEBDispatch.h (_bstr_t usage)
 *
 * MinGW-w64 provides COM error handling (comdef.h) but lacks the string
 * conversion utilities. ReactOS provides comsupp.cpp with implementations,
 * but we use this header-only version to avoid building/linking an extra
 * library. This must be included BEFORE comutil.h to provide definitions
 * before _bstr_t's inline methods are instantiated.
 *
 * @note Include this header before <comutil.h> in MinGW builds to provide
 *       symbol definitions for _bstr_t's internal string conversion calls.
 *       Without this, you will get "undefined reference" linker errors.
 */

#pragma once

#ifdef __MINGW32__

#include <windows.h>
#include <ole2.h>
#include <oleauto.h>
#include <comdef.h>

namespace _com_util
{

inline BSTR WINAPI ConvertStringToBSTR(const char *pSrc)
{
    DWORD cwch;
    BSTR wsOut = nullptr;

    if (!pSrc)
        return nullptr;

    // Compute the needed size with the null terminator
    cwch = MultiByteToWideChar(CP_ACP, 0, pSrc, -1, nullptr, 0);
    if (cwch == 0)
        return nullptr;

    // Allocate the BSTR (without the null terminator)
    wsOut = SysAllocStringLen(nullptr, cwch - 1);
    if (!wsOut)
    {
        _com_issue_error(HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY));
        return nullptr;
    }

    // Convert the string
    if (MultiByteToWideChar(CP_ACP, 0, pSrc, -1, wsOut, cwch) == 0)
    {
        // We failed, clean everything up
        cwch = GetLastError();

        SysFreeString(wsOut);
        wsOut = nullptr;

        _com_issue_error(!IS_ERROR(cwch) ? HRESULT_FROM_WIN32(cwch) : cwch);
    }

    return wsOut;
}

inline char* WINAPI ConvertBSTRToString(BSTR pSrc)
{
    DWORD cb, cwch;
    char *szOut = nullptr;

    if (!pSrc)
        return nullptr;

    // Retrieve the size of the BSTR with the null terminator
    cwch = SysStringLen(pSrc) + 1;

    // Compute the needed size with the null terminator
    cb = WideCharToMultiByte(CP_ACP, 0, pSrc, cwch, nullptr, 0, nullptr, nullptr);
    if (cb == 0)
    {
        cwch = GetLastError();
        _com_issue_error(!IS_ERROR(cwch) ? HRESULT_FROM_WIN32(cwch) : cwch);
        return nullptr;
    }

    // Allocate the string
    szOut = (char*)::operator new(cb * sizeof(char));
    if (!szOut)
    {
        _com_issue_error(HRESULT_FROM_WIN32(ERROR_OUTOFMEMORY));
        return nullptr;
    }

    // Convert the string and null-terminate
    szOut[cb - 1] = '\0';
    if (WideCharToMultiByte(CP_ACP, 0, pSrc, cwch, szOut, cb, nullptr, nullptr) == 0)
    {
        // We failed, clean everything up
        cwch = GetLastError();

        ::operator delete(szOut);
        szOut = nullptr;

        _com_issue_error(!IS_ERROR(cwch) ? HRESULT_FROM_WIN32(cwch) : cwch);
    }

    return szOut;
}

}

// Provide vtMissing global variable
// Use inline variable (C++17) to avoid multiple definition errors
inline _variant_t vtMissing(DISP_E_PARAMNOTFOUND, VT_ERROR);

#endif // __MINGW32__
