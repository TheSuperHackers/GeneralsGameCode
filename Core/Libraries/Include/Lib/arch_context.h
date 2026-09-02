/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
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

// FILE: arch_context.h //////////////////////////////////////////////////////
//
// Maps the x86-32 Win32 CONTEXT register field names used throughout the
// two crash handlers (Core/Libraries/Source/WWVegas/WWLib/Except.cpp and
// Core/Libraries/Source/debug/debug_except.cpp) onto whichever field names
// the target architecture's CONTEXT struct actually has, plus the machine
// constant the stack-walk call sites need.
//
// On x86-32 the general-purpose registers are Eip/Esp/Ebp/Eax/Ebx/Ecx/Edx/
// Esi/Edi. On x86-64 they are Rip/Rsp/Rbp/Rax/Rbx/Rcx/Rdx/Rsi/Rdi and are
// twice as wide. CTX_PC/CTX_STACK/CTX_FRAME/CTX_AX/CTX_BX/CTX_CX/CTX_DX/
// CTX_SI/CTX_DI hide that difference behind one name per register so call
// sites don't need an #ifdef each. On x86-32 every one of these macros
// expands to exactly the original field access (e.g. CTX_PC(ctx) is
// ((ctx).Eip)), so the 32-bit build is unaffected.
//
// Deliberately NOT using the WOW64_* structures/constants GCC suggests
// (WOW64_FLOATING_SAVE_AREA, WOW64_SIZE_OF_80387_REGISTERS, ...): those
// describe a 32-bit process as inspected from a 64-bit one, not a native
// 64-bit process's own FPU/SSE state. Taking that suggestion would compile
// cleanly and read the wrong bytes -- a crash dump that is silently
// corrupt is worse than no crash dump. The FPU/SSE save area itself is a
// structural difference (CONTEXT.FltSave, an XMM_SAVE_AREA32, vs the
// 32-bit FLOATING_SAVE_AREA) rather than a field rename, so it is not
// covered by macros here -- the two crash handlers guard that block with
// their own #if per architecture, mirroring what the 32-bit block reports.

#pragma once

#include <windows.h>

#if defined(_WIN64) || defined(__x86_64__)

#define CTX_PC(ctx)		((ctx).Rip)
#define CTX_STACK(ctx)	((ctx).Rsp)
#define CTX_FRAME(ctx)	((ctx).Rbp)
#define CTX_AX(ctx)		((ctx).Rax)
#define CTX_BX(ctx)		((ctx).Rbx)
#define CTX_CX(ctx)		((ctx).Rcx)
#define CTX_DX(ctx)		((ctx).Rdx)
#define CTX_SI(ctx)		((ctx).Rsi)
#define CTX_DI(ctx)		((ctx).Rdi)

// Hex-digit width of a full register dump column, for the stream-based
// (Debug::Width()) register printers in debug_except.cpp -- 16 digits show
// a full 64-bit register instead of just its low half.
#define CTX_REG_WIDTH 16

#define CTX_STACKWALK_MACHINE IMAGE_FILE_MACHINE_AMD64

#else

#define CTX_PC(ctx)		((ctx).Eip)
#define CTX_STACK(ctx)	((ctx).Esp)
#define CTX_FRAME(ctx)	((ctx).Ebp)
#define CTX_AX(ctx)		((ctx).Eax)
#define CTX_BX(ctx)		((ctx).Ebx)
#define CTX_CX(ctx)		((ctx).Ecx)
#define CTX_DX(ctx)		((ctx).Edx)
#define CTX_SI(ctx)		((ctx).Esi)
#define CTX_DI(ctx)		((ctx).Edi)

#define CTX_REG_WIDTH 8

#define CTX_STACKWALK_MACHINE IMAGE_FILE_MACHINE_I386

#endif
