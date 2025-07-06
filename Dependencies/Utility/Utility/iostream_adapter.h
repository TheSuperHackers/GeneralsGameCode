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

// This file helps adapting modern iostream to legacy vs6 iostream,
// where symbols are not contained in the std namespace.

#pragma once

#if defined(USING_STLPORT) || (defined(_MSC_VER) && _MSC_VER < 1300)

#include <iostream.h>

#else

#include <iostream>

inline auto& cout = std::cout;
inline auto& cerr = std::cerr;

using streambuf = std::streambuf;
using ostream = std::ostream;

template <class _Elem, class _Traits>
std::basic_ostream<_Elem, _Traits>& endl(std::basic_ostream<_Elem, _Traits>& _Ostr)
{
    return std::endl(_Ostr);
}

template <class _Elem, class _Traits>
std::basic_ostream<_Elem, _Traits>& flush(std::basic_ostream<_Elem, _Traits>& _Ostr)
{
    return std::flush(_Ostr);
}

#endif
