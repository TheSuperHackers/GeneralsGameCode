#pragma once

// VC6 macros
#if defined(_MSC_VER) && _MSC_VER < 1300
#define __debugbreak() __asm { int 3 }
#endif

// Non-VC6 macros
#if !(defined(_MSC_VER) && _MSC_VER < 1300)
#include <cstdint>

#if !defined(_lrotl) && !defined(_WIN32)
static inline uint32_t _lrotl(uint32_t value, int shift)
{
#if defined(__has_builtin) && __has_builtin(__builtin_rotateleft32)
    return __builtin_rotateleft32(value, shift);
#else
    shift &= 31;
    return ((value << shift) | (value >> (32 - shift)));
#endif
}
#endif

#ifndef _rdtsc
#ifdef _WIN32
#include <intrin.h>
#pragma intrinsic(__rdtsc)
#endif

static inline uint64_t _rdtsc()
{
#ifdef _WIN32
    return __rdtsc();
#elif defined(__has_builtin) && __has_builtin(__builtin_readcyclecounter)
    return __builtin_readcyclecounter();
#elif defined(__has_builtin) && __has_builtin(__builtin_ia32_rdtsc)
    return __builtin_ia32_rdtsc();
#else
#error "No implementation for _rdtsc"
#endif
}
#endif
 
#ifdef _MSC_VER
#include <intrin.h>
#pragma intrinsic(_ReturnAddress)
#elif defined(__has_builtin)
    #if __has_builtin(__builtin_return_address)
    static inline uintptr_t _ReturnAddress()
    {
        return reinterpret_cast<uintptr_t>(__builtin_return_address(0));
    }
    #else
    #error "No implementation for _ReturnAddress"
    #endif
#else
#error "No implementation for _ReturnAddress"
#endif

#if defined(__has_builtin) 
    #if  __has_builtin(__builtin_debugtrap)
    #define __debugbreak() __builtin_debugtrap()
    #elif __has_builtin(__builtin_trap)
    #define __debugbreak() __builtin_trap()
    #else
    #error "No implementation for __debugbreak"
    #endif
#elif !defined(_MSC_VER)
#error "No implementation for __debugbreak"
#endif

#endif // defined(_MSC_VER) && _MSC_VER < 1300
