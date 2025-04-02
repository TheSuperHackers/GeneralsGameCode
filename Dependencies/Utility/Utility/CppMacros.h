// TheSuperHackers
// This file contains macros to help upgrade the code for newer cpp standards.

#pragma once

#include <cassert>
#define UNIMPLEMEMTED_ERROR(msg) do { \
  assert(("Unimplemented: ", msg, 0)); \
  } while(0)

#if __cplusplus >= 201703L
#define NOEXCEPT_17 noexcept
#else
#define NOEXCEPT_17
#endif

// noexcept for methods of IUNKNOWN interface
#if defined(_MSC_VER)
#define IUNKNOWN_NOEXCEPT NOEXCEPT_17
#else
#define IUNKNOWN_NOEXCEPT
#endif

#if __cplusplus >= 201103L
    #define CPP_11(code) code
#else
    #define CPP_11(code)
#endif
