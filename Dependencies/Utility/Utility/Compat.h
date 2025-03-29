// TheSuperHackers
// This file contains macros to help compiling on non-windows platforms.

#pragma once

#ifndef _WIN32
// For size_t
#include <cstddef>
// For isdigit
#include <cctype>

// __forceinline
#ifndef __forceinline
#if defined __has_attribute && __has_attribute(always_inline)
#define __forceinline __attribute__((always_inline)) inline
#else
#define __forceinline inline
#endif
#endif

// _cdecl / __cdecl
#ifndef _cdecl
#define _cdecl
#endif
#ifndef __cdecl
#define __cdecl
#endif

// OutputDebugString
#ifndef OutputDebugString
#define OutputDebugString(str) printf("%s\n", str)
#endif

// _MAX_DRIVE, _MAX_DIR, _MAX_FNAME, _MAX_EXT, _MAX_PATH
#ifndef _MAX_DRIVE
#define _MAX_DRIVE 255
#endif
#ifndef _MAX_DIR
#define _MAX_DIR 255
#define _MAX_DIR 255
#endif
#ifndef _MAX_FNAME
#define _MAX_FNAME 255
#endif
#ifndef _MAX_EXT
#define _MAX_EXT 255
#endif
#ifndef _MAX_PATH
#define _MAX_PATH 255
#endif

#include "MemCompat.h"
#include "StringCompat.h"
#include "TCharCompat.h"
#include "WCharCompat.h"
#include "TimeCompat.h"
#include "ThreadCompat.h"

#endif