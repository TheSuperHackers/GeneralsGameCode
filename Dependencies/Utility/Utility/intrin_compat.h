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

// This file contains macros to help compiling on non-windows platforms and VC6 compatibility macros.
#pragma once

// VC6 macros
#if defined(_MSC_VER) && _MSC_VER < 1300

#ifndef __debugbreak
#define __debugbreak() __asm { int 3 }
#endif

#ifndef _rdtsc
static inline __int64 _rdtsc()
{
	int h;
	int l;

	__asm {
		_emit 0Fh
		_emit 31h
		mov	[h],edx
		mov	[l],eax
	}

    __int64 result ((__int64)h << 32 | l);
    return result;
}
#endif //_rdtsc

#if defined _WIN32 && (defined _M_IX86 || defined _M_AMD64)
#ifndef cpuid
#define cpuid(regs, cpuid_type) __asm\
{\
    __asm { pushad }\
    __asm { mov eax,[cpuid_type] }\
    __asm { xor ebx,ebx }\
    __asm { xor ecx,ecx }\
    __asm { xor edx,edx }\
    __asm { cpuid}\
    __asm { mov [regs + 0],eax }\
    __asm { mov [regs + 4],ebx }\
    __asm { mov [regs + 8],ecx }\
    __asm { mov [regs + 12],edx }\
    __asm { popad }\
}
#endif
#endif //defined _WIN32 && (defined _M_IX86 || defined _M_AMD64)

#endif // defined(_MSC_VER) && _MSC_VER < 1300


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
#if _WIN32
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

#ifndef cpuid
#if defined(_MSC_VER)
#include <intrin.h>
#define cpuid(regs, cpuid_type) __cpuid(reinterpret_cast<int *>(regs), cpuid_type)
#elif (defined __clang__ || defined __GNUC__) && (defined __i386__ || defined __amd64__)
#include <cpuid.h>
#define cpuid(regs, cpuid_type) __cpuid(cpuid_type, regs[0], regs[1], regs[2], regs[3])
#else
/* Just return 0 for everything if its not x86 */
#include <string.h>
#define cpuid(regs, cpuid_type) memset(regs, 0, 16)
#endif
#endif //cpuid

#endif // defined(_MSC_VER) && _MSC_VER < 1300
