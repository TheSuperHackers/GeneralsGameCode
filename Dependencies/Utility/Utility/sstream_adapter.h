// TheSuperHackers
// This file helps adapting modern sstream to legacy vs6 strstrea,
// where symbols are not contained in the std namespace.

#pragma once

#if defined(USING_STLPORT) || (defined(_MSC_VER) && _MSC_VER < 1300)

#include <strstrea.h>

#define STRSTREAM_CSTR(_stringstream) _stringstream.str()

#else

#include <sstream>

using strstream = std::stringstream;

#define STRSTREAM_CSTR(_stringstream) _stringstream.str().c_str()

#endif
