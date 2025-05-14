/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 TheSuperHackers
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

#include <stdio.h>

#if defined(_MSC_VER) && _MSC_VER < 1300

#include <stdarg.h>

inline int vsnprintf(char* _Buffer, size_t _BufferCount, const char* _Format, va_list _ArgList)
{
    // Microsoft's _vsnprintf does not null terminate when writing the entire length.
    int result = _vsnprintf(_Buffer, _BufferCount, _Format, _ArgList);
    _Buffer[_BufferCount - 1] = '\0';
    return result;
}

// Yes, this is called vswprintf instead of vsnwprintf
inline int vswprintf(wchar_t* _Buffer, size_t _BufferCount, const wchar_t* _Format, va_list _ArgList)
{
    // Microsoft's _vsnwprintf does not null terminate when writing the entire length.
    int result = _vsnwprintf(_Buffer, _BufferCount, _Format, _ArgList);
    _Buffer[_BufferCount - 1] = L'\0';
    return result;
}

inline int snprintf(char* _Buffer, size_t _BufferCount, const char* _Format, ...)
{
    va_list _ArgList;
    va_start(_ArgList, _Format);
    int result = vsnprintf(_Buffer, _BufferCount, _Format, _ArgList);
    va_end(_ArgList);
    return result;
}

// Yes, this is called swprintf instead of snwprintf
inline int swprintf(wchar_t* _Buffer, size_t _BufferCount, const wchar_t* _Format, ...)
{
    va_list _ArgList;
    va_start(_ArgList, _Format);
    int result = vswprintf(_Buffer, _BufferCount, _Format, _ArgList);
    va_end(_ArgList);
    return result;
}

#endif
