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

// This file contains macros to help with simple atomic integer interlocked operations.
#pragma once

#include <Utility/stdint_adapter.h>


// Interlocked helper function data types
#if defined(__linux__) || defined(__CYGWIN__)
typedef int16_t AtomicType16;
typedef int32_t AtomicType32;
typedef int64_t AtomicType64;

#elif defined(__APPLE__)
typedef SInt16 AtomicType16;
typedef SInt32 AtomicType32;
typedef SInt64 AtomicType64;

#elif defined(__OpenBSD__)
typedef uint16_t AtomicType16;
typedef uint32_t AtomicType32;
typedef uint64_t AtomicType64;

#elif defined(__NetBSD__) || defined(__FreeBSD__) || defined(__DragonFly__)
typedef uint16_t AtomicType16;
typedef uint32_t AtomicType32;
typedef uint64_t AtomicType64;

#elif defined(_WIN32) || defined(_WIN64)
typedef int16_t AtomicType16;
typedef long	AtomicType32;
typedef int64_t AtomicType64;

#else
#error platform not supported
#endif


#if defined(__linux__) || defined(__CYGWIN__)
inline AtomicType16 interlockedIncrement16(volatile AtomicType16 *addend) { return __sync_fetch_and_add(addend, AtomicType16{ 1 }); }
inline AtomicType32 interlockedIncrement32(volatile AtomicType32 *addend) { return __sync_fetch_and_add(addend, AtomicType32{ 1 }); }
inline AtomicType64 interlockedIncrement64(volatile AtomicType64 *addend) { return __sync_fetch_and_add(addend, AtomicType64{ 1 }); }

inline AtomicType16 interlockedDecrement16(volatile AtomicType16 *addend) { return __sync_fetch_and_sub(addend, AtomicType16{ 1 }); }
inline AtomicType32 interlockedDecrement32(volatile AtomicType32 *addend) { return __sync_fetch_and_sub(addend, AtomicType32{ 1 }); }
inline AtomicType64 interlockedDecrement64(volatile AtomicType64 *addend) { return __sync_fetch_and_sub(addend, AtomicType64{ 1 }); }

#elif defined(__APPLE__)
#include <OSAtomic.h>
inline AtomicType16 interlockedIncrement16(volatile AtomicType16 *addend) { return OSIncrementAtomic16(addend); }
inline AtomicType32 interlockedIncrement32(volatile AtomicType32 *addend) { return OSIncrementAtomic(addend); }
inline AtomicType64 interlockedIncrement64(volatile AtomicType64 *addend) { return OSIncrementAtomic64(addend); }

inline AtomicType16 interlockedIncrement16(volatile AtomicType16 *addend) { return OSDecrementAtomic16(addend); }
inline AtomicType32 interlockedIncrement32(volatile AtomicType32 *addend) { return OSDecrementAtomic(addend); }
inline AtomicType64 interlockedIncrement64(volatile AtomicType64 *addend) { return OSDecrementAtomic64(addend); }

#elif defined(__OpenBSD__)
#include <sys/atomic.h>
// Appears to have no native atomic operations for 16 bit integers.
inline AtomicType32 interlockedIncrement32(volatile AtomicType32 *addend) { return atomic_inc_int_nv(addend); }
inline AtomicType64 interlockedIncrement64(volatile AtomicType64 *addend) { return atomic_inc_long_nv(addend); }

inline AtomicType32 interlockedIncrement32(volatile AtomicType32 *addend) { return atomic_dec_int_nv(addend); }
inline AtomicType64 interlockedIncrement64(volatile AtomicType64 *addend) { return atomic_dec_long_nv(addend); }

#elif defined(__FreeBSD__) || defined(__DragonFly__)
#include <machine/atomic.h>
// Appears to have no native atomic operations for 16, 64 bit integers.
inline AtomicType32 interlockedIncrement32(volatile AtomicType32 *addend) { return atomic_fetchadd_32(addend, AtomicType32{ 1 }); }

inline AtomicType32 interlockedDecrement32(volatile AtomicType32 *addend) { return atomic_fetchadd_32(addend, ~AtomicType32{ 0 }); }

#elif defined(__NetBSD__)
#include <sys/atomic.h>
// Appears to have no native atomic operations for 16 bit integers.
inline AtomicType32 interlockedIncrement32(volatile AtomicType32 *addend) { return atomic_inc_32_nv(addend); }
inline AtomicType64 interlockedIncrement64(volatile AtomicType64 *addend) { return atomic_inc_64_nv(addend); }

inline AtomicType32 interlockedDecrement32(volatile AtomicType32 *addend) { return atomic_dec_32_nv(addend); }
inline AtomicType32 interlockedDecrement64(volatile AtomicType64 *addend) { return atomic_dec_64_nv(addend); }

#elif defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#if !(defined(_MSC_VER) && _MSC_VER < 1300)
inline AtomicType16 interlockedIncrement16(volatile AtomicType16 *addend) { return InterlockedIncrement16(addend); }
inline AtomicType32 interlockedIncrement32(volatile AtomicType32 *addend) { return InterlockedIncrement(addend); }
inline AtomicType64 interlockedIncrement64(volatile AtomicType64 *addend) { return InterlockedIncrement64(addend); }

inline AtomicType16 interlockedDecrement16(volatile AtomicType16 *addend) { return InterlockedDecrement16(addend); }
inline AtomicType32 interlockedDecrement32(volatile AtomicType32 *addend) { return InterlockedDecrement(addend); }
inline AtomicType64 interlockedDecrement64(volatile AtomicType64 *addend) { return InterlockedDecrement64(addend); }
#else
inline AtomicType16 interlockedIncrement16(volatile AtomicType16* ptr) {
    __asm {
        mov ecx, ptr        ; Load address of the value into ECX
        mov ax, 1           ; Value to add (1)
        lock xadd [ecx], ax ; Atomically add and exchange
        inc ax              ; Increment the value in AX
        movzx eax, ax       ; Zero extend AX to EAX
    }
}
inline AtomicType32 interlockedIncrement32(volatile AtomicType32 *addend) { return InterlockedIncrement(addend); }
// 64 bit interlockedIncrement64 does not handle carryover into the upper 32bits
inline AtomicType64 interlockedIncrement64(volatile AtomicType64* ptr) {
    __asm {
        mov eax, ptr            ; Load address of the value into EAX
        mov ecx, 1              ; Value to add (1)
        lock xadd [eax], ecx    ; Atomically add and exchange the lower 32 bits
        mov edx, [eax + 4]      ; Load the upper 32 bits
        add edx, 0              ; Add 0 to the upper 32 bits (no change)
        mov [eax + 4], edx      ; Store the upper 32 bits back
        mov eax, [ptr]          ; Move the new value (after increment) into EAX
        add eax, 1              ; Increment EAX to return the new value
    }
}

inline AtomicType16 interlockedDecrement16(volatile AtomicType16* ptr) {
    __asm {
        mov ecx, ptr        ; Load address of the value into ECX
        mov ax, 1           ; Value to subtract (1)
        neg ax              ; Negate the value to make it -1
        lock xadd [ecx], ax ; Atomically add and exchange
        dec ax              ; Decrement the value in AX
        movzx eax, ax       ; Zero extend AX to EAX
    }
} 
inline AtomicType32 interlockedDecrement32(volatile AtomicType32 *addend) { return InterlockedDecrement(addend); }
// 64 bit interlockedDecrement64 does not handle borrow from the upper 32bits
inline AtomicType64 interlockedDecrement64(volatile AtomicType64* ptr) {
    __asm {
        mov eax, ptr            ; Load address of the value into EAX
        mov ecx, 1              ; Value to subtract (1)
        lock xadd [eax], ecx    ; Atomically add and exchange the lower 32 bits
        neg ecx                 ; Negate ECX to get -1
        lock xadd [eax], ecx    ; Atomically subtract 1 from the lower 32 bits
        mov edx, [eax + 4]      ; Load the upper 32 bits
        add edx, 0              ; Add 0 to the upper 32 bits (no change)
        mov [eax + 4], edx      ; Store the upper 32 bits back
        mov eax, [ptr]          ; Move the new value (after decrement) into EAX
        sub eax, 1              ; Decrement EAX to return the new value
    }
}
#endif // !(_MSC_VER < 1300)

#else
#error platform not supported
#endif


// c++ template Interlocked helper functions, prevent compiler errors when implementation is not present and function is unused for a missing type
template<typename Type> inline Type interlockedIncrement(volatile Type *addend);
template<typename Type> inline Type interlockedDecrement(volatile Type *addend);

template<> inline AtomicType16 interlockedIncrement<AtomicType16>(volatile AtomicType16 *addend) { return interlockedIncrement16(addend); }
template<> inline AtomicType32 interlockedIncrement<AtomicType32>(volatile AtomicType32 *addend) { return interlockedIncrement32(addend); }
template<> inline AtomicType64 interlockedIncrement<AtomicType64>(volatile AtomicType64 *addend) { return interlockedIncrement64(addend); }

template<> inline AtomicType16 interlockedDecrement<AtomicType16>(volatile AtomicType16 *addend) { return interlockedDecrement16(addend); }
template<> inline AtomicType32 interlockedDecrement<AtomicType32>(volatile AtomicType32 *addend) { return interlockedDecrement32(addend); }
template<> inline AtomicType64 interlockedDecrement<AtomicType64>(volatile AtomicType64 *addend) { return interlockedDecrement64(addend); }