// TheSuperHackers
// This file contains macros to help compiling on non-windows platforms.

#pragma once

#ifndef _WIN32
// Integer types
#define __int64 int64_t

// String functions
#define stricmp strcasecmp

// __forceinline
#ifndef __forceinline
#if defined __has_attribute && __has_attribute(always_inline)
#define __forceinline __attribute__((always_inline)) inline
#else
#define __forceinline inline
#endif
#endif
#endif