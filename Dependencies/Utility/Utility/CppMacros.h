// TheSuperHackers
// This file contains macros to help upgrade the code for newer cpp standards.

#pragma once

#if __cplusplus >= 201703L
#define NOEXCEPT_17 noexcept
#else
#define NOEXCEPT_17
#endif
