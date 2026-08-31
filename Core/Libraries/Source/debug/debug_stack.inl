/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
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

// Used for dynamically linking to dbghelp.dll functions.

// keep this always as first entry
DBGHELP(SymInitialize,
        BOOL,
        (HANDLE hProcess, PCSTR UserSearchPath, BOOL fInvadeProcess))

DBGHELP(SymGetOptions,
        DWORD,
        ())

DBGHELP(SymSetOptions,
        DWORD,
        (DWORD SymOptions))

DBGHELP(StackWalk,
        BOOL,
        (DWORD MachineType, HANDLE hProcess, HANDLE hThread, LPSTACKFRAME StackFrame,
        LPVOID ContextRecord, PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryRoutine,
        PFUNCTION_TABLE_ACCESS_ROUTINE FunctionTableAccessRoutine,
        PGET_MODULE_BASE_ROUTINE GetModuleBaseRoutine,
        PTRANSLATE_ADDRESS_ROUTINE TranslateAddress))

// LPSTACKFRAME / PFUNCTION_TABLE_ACCESS_ROUTINE / PGET_MODULE_BASE_ROUTINE
// above are #defined to their ...64 forms by dbghelp.h on 64-bit builds
// (_IMAGEHLP64), so StackWalkType's signature already widens for free. These
// next two entries hand-roll a DWORD address parameter instead of going
// through those platform names, so gDbg._StackWalk's call site (which does
// use the widened StackWalkType) rejects them as arguments unless widened to
// match explicitly. VC6 (1998) predates the ...64 DbgHelp API, so the 32-bit
// branch is kept exactly as it was.
#if defined(_WIN64) || defined(__x86_64__)
DBGHELP(SymFunctionTableAccess,
        LPVOID,
        (HANDLE hProcess, DWORD64 AddrBase))

DBGHELP(SymGetModuleBase,
        DWORD64,
        (HANDLE hProcess, DWORD64 dwAddr))
#else
DBGHELP(SymFunctionTableAccess,
        LPVOID,
        (HANDLE hProcess, DWORD AddrBase))

DBGHELP(SymGetModuleBase,
        DWORD,
        (HANDLE hProcess, DWORD dwAddr))
#endif

// Real SymGetSymFromAddr64: BOOL(HANDLE, DWORD64 qwAddr,
// PDWORD64 pdwDisplacement, PIMAGEHLP_SYMBOL64 Symbol). Address and
// Displacement are hand-rolled DWORD/LPDWORD, same class of gap as
// SymFunctionTableAccess/SymGetModuleBase above, and Displacement is a
// write target: leaving it 32-bit on x64 is a stack buffer overflow every
// time a symbol is resolved (SymGetSymFromAddr64 writes 8 bytes through
// it). Symbol needs no separate widening here: PIMAGEHLP_SYMBOL is
// #defined to PIMAGEHLP_SYMBOL64 under _IMAGEHLP64 (imagehlp.h is already
// included above this point in debug_stack.cpp), so it widens for free.
#if defined(_WIN64) || defined(__x86_64__)
DBGHELP(SymGetSymFromAddr,
        BOOL,
        (HANDLE hProcess, DWORD64 Address, PDWORD64 Displacement,
        PIMAGEHLP_SYMBOL Symbol))
#else
DBGHELP(SymGetSymFromAddr,
        BOOL,
        (HANDLE hProcess, DWORD Address, LPDWORD Displacement,
        PIMAGEHLP_SYMBOL Symbol))
#endif

// Real SymGetLineFromAddr64: BOOL(HANDLE, DWORD64 qwAddr,
// PDWORD pdwDisplacement, PIMAGEHLP_LINE64 Line64) -- only the address
// parameter widens; pdwDisplacement genuinely stays PDWORD (not a write
// overflow), and Line widens for free the same way Symbol does above.
#if defined(_WIN64) || defined(__x86_64__)
DBGHELP(SymGetLineFromAddr,
        BOOL,
        (HANDLE hProcess, DWORD64 dwAddr, PDWORD pdwDisplacement,
        PIMAGEHLP_LINE Line))
#else
DBGHELP(SymGetLineFromAddr,
        BOOL,
        (HANDLE hProcess, DWORD dwAddr, PDWORD pdwDisplacement,
        PIMAGEHLP_LINE Line))
#endif

// keep this always as last entry
DBGHELP(SymCleanup,
        BOOL,
        (HANDLE hProcess))
