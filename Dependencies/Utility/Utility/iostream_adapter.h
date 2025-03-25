// TheSuperHackers
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
