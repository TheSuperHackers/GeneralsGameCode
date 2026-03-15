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
 * @file atl_compat.h
 * @brief ATL compatibility layer for MinGW-w64 with ReactOS ATL
 *
 * Provides compatibility definitions for using ReactOS ATL headers
 * with MinGW-w64 GCC compiler. Uses ReactOS PSEH in C++-compatible
 * dummy mode (_USE_DUMMY_PSEH) because MinGW-w64's PSEH uses GNU C
 * nested functions which are not valid in C++.
 */

#pragma once

#ifdef __MINGW32__

// Include Windows types needed for ATL compatibility
#include <wtypes.h>

// Include PSEH compatibility first (uses ReactOS PSEH in dummy mode)
#include "pseh_compat.h"

// Define _ATL_IIDOF macro for ReactOS ATL (uses MinGW-w64's __uuidof)
#ifndef _ATL_IIDOF
#define _ATL_IIDOF(x) __uuidof(x)
#endif

// Forward declare _Delegate function for COM aggregation support
// ReactOS ATL's COM_INTERFACE_ENTRY_AGGREGATE macro uses _Delegate but doesn't define it
// The actual function pointer will be defined after atlbase.h provides _ATL_CREATORARGFUNC
extern "C" HRESULT WINAPI _ATL_DelegateQueryInterface(void* pv, REFIID riid, LPVOID* ppv, DWORD_PTR dw);

// Suppress additional warnings from ReactOS ATL headers
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wattributes"

// NOTE: ReactOS ATL compile definitions are set in cmake/reactos-atl.cmake:
//   - _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
//   - _ATL_NO_DEBUG_CRT
//   - ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW
//   - ATL_NO_DEFAULT_LIBS
// These are applied via target_compile_definitions() on the reactos_atl target.
// Any target linking to reactos_atl will automatically inherit these definitions.
//
// IMPORTANT: _ATL_NO_AUTOMATIC_NAMESPACE is NOT defined because the codebase
// uses ATL types (CComModule, CComObject, CString, etc.) without namespace
// qualification and relies on the automatic 'using namespace ATL;' from ATL headers.

// Define _Delegate implementation and macro now that ATL types are available
inline HRESULT WINAPI _ATL_DelegateQueryInterface(void* pv, REFIID riid, LPVOID* ppv, DWORD_PTR dw)
{
    IUnknown** ppunk = reinterpret_cast<IUnknown**>(reinterpret_cast<char*>(pv) + dw);
    if (*ppunk == nullptr)
        return E_NOINTERFACE;
    return (*ppunk)->QueryInterface(riid, ppv);
}

#ifndef _Delegate
#define _Delegate ((ATL::_ATL_CREATORARGFUNC*)_ATL_DelegateQueryInterface)
#endif

// Restore compiler warnings after ATL includes
#define ATL_COMPAT_RESTORE_WARNINGS() \
    do { \
        _Pragma("GCC diagnostic pop") \
        PSEH_COMPAT_RESTORE_WARNINGS() \
    } while(0)

#else
// Non-MinGW platforms don't need these workarounds
#define ATL_COMPAT_RESTORE_WARNINGS()
#endif // __MINGW32__
