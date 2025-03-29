// TheSuperHackers
// This file contains macros to help upgrade the code for newer cpp standards.

#pragma once

#if __cplusplus >= 201703L
#define NOEXCEPT_17 noexcept
#else
#define NOEXCEPT_17
#endif

// SuperHackers patch @roossienb/Skyaero, @Mauller, 2025-03-27
// Register is deprecated since C++17
// We create a macro to replace it with register if the cpp version is less than 2017
#if __cplusplus >= 201703L
#define REGISTER
#else
#define REGISTER register
#endif