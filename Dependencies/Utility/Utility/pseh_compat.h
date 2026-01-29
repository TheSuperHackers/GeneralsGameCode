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
 * @file pseh_compat.h
 * @brief PSEH compatibility for MinGW-w64 with ReactOS ATL
 *
 * ReactOS ATL headers include <pseh/pseh2.h>. This header ensures
 * that ReactOS PSEH is used in C++-compatible dummy mode (_USE_DUMMY_PSEH).
 * The cmake configuration (reactos-atl.cmake) adds ReactOS PSEH headers
 * to the include path before system headers, and defines _USE_DUMMY_PSEH.
 */

#pragma once

#ifdef __MINGW32__

// Suppress PSEH-related warnings from ReactOS PSEH headers
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wattributes"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-label"

// ReactOS PSEH headers will be included by ATL headers
// No need to include them explicitly here

// Restore compiler warnings after PSEH includes
#define PSEH_COMPAT_RESTORE_WARNINGS() \
    _Pragma("GCC diagnostic pop")

#else
// Non-MinGW platforms use native SEH or don't need PSEH
#define PSEH_COMPAT_RESTORE_WARNINGS()
#endif // __MINGW32__
