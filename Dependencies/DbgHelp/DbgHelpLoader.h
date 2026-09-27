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

#pragma once

#include "imagehlp_adapter.h"

// This static class loads and unloads dbghelp.dll at runtime.
//
// The dbghelp functions declared in the DbgHelp namespace below are implemented in this library
// and forward to the matching export of the loaded module, so they can be called almost as if
// dbghelp.dll was linked statically. When the module is not loaded, every function fails with a
// neutral return value.
//
// Is thread safe, because dbghelp.dll is not thread safe for the most part. Every call to load
// needs a paired call to unload, no matter if the load was successful. Internally it must not use
// new and delete because it can be used during game memory initialization.

class DbgHelpLoader
{
public:

	// Returns whether dbghelp.dll is loaded
	static bool isLoaded();

	// Returns whether dbghelp.dll is loaded from the system directory
	static bool isLoadedFromSystem();

	// Returns whether dbghelp.dll was attempted to be loaded but failed
	static bool isFailed();

	static bool load();
	static void unload();
};

// The dbghelp functions are declared in a namespace, because they cannot take the global names of
// the declarations in imagehlp.h. Other libraries such as Tracy link dbghelp.lib, which already
// defines the global names, so defining them here too would fail to link with duplicate symbols.
// Calling the unqualified global names instead would bypass this loader.
namespace DbgHelp
{

BOOL WINAPI SymInitialize(
	HANDLE hProcess,
	PCSTR UserSearchPath,
	BOOL fInvadeProcess);

BOOL WINAPI SymCleanup(
	HANDLE hProcess);

DWORD WINAPI SymLoadModule(
	HANDLE hProcess,
	HANDLE hFile,
	PCSTR ImageName,
	PCSTR ModuleName,
	DWORD BaseOfDll,
	DWORD SizeOfDll);

DWORD WINAPI SymGetModuleBase(
	HANDLE hProcess,
	DWORD dwAddr);

BOOL WINAPI SymUnloadModule(
	HANDLE hProcess,
	DWORD BaseOfDll);

BOOL WINAPI SymGetSymFromAddr(
	HANDLE hProcess,
	DWORD dwAddr,
	PDWORD pdwDisplacement,
	PIMAGEHLP_SYMBOL Symbol);

BOOL WINAPI SymGetLineFromAddr(
	HANDLE hProcess,
	DWORD dwAddr,
	PDWORD pdwDisplacement,
	PIMAGEHLP_LINE Line);

DWORD WINAPI SymSetOptions(
	DWORD SymOptions);

PVOID WINAPI SymFunctionTableAccess(
	HANDLE hProcess,
	DWORD AddrBase);

BOOL WINAPI StackWalk(
	DWORD MachineType,
	HANDLE hProcess,
	HANDLE hThread,
	LPSTACKFRAME StackFrame,
	PVOID ContextRecord,
	PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryRoutine,
	PFUNCTION_TABLE_ACCESS_ROUTINE FunctionTableAccessRoutine,
	PGET_MODULE_BASE_ROUTINE GetModuleBaseRoutine,
	PTRANSLATE_ADDRESS_ROUTINE TranslateAddress);

BOOL WINAPI MiniDumpWriteDump(
	HANDLE hProcess,
	DWORD ProcessId,
	HANDLE hFile,
	MINIDUMP_TYPE DumpType,
	PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
	PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
	PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

} // namespace DbgHelp
