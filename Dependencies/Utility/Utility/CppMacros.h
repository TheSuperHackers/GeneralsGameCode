// TheSuperHackers
// This file contains macros to help upgrade the code for newer cpp standards.

#pragma once

#if __cplusplus >= 201703L
#define NOEXCEPT_17 noexcept
#else
#define NOEXCEPT_17
#endif

#if __cplusplus >= 201103L
    #define CPP_11(code) code
#else
    #define CPP_11(code)
#endif
