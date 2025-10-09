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

#include "always.h"

#include <win.h>
#include <imagehlp.h> // Must be included after Windows.h
#include <set>

#include "MallocAllocator.h"

// This static class can load and unload dbghelp.dll
// Internally it must not use new and delete because it can be created during game memory initialization.

class DbgHelpLoader
{
private:

  static DbgHelpLoader* Inst; // Is singleton class

  DbgHelpLoader();
  ~DbgHelpLoader();

public:

  // Returns whether dbghelp.dll is loaded
  static bool isLoaded();

  // Returns whether dbghelp.dll is loaded from the system directory
  static bool isLoadedFromSystem();

  static bool load();
  static bool reload();
  static void unload();

  static BOOL WINAPI symInitialize(
    HANDLE hProcess,
    LPSTR UserSearchPath,
    BOOL fInvadeProcess);

  static BOOL WINAPI symCleanup(
    HANDLE hProcess);

  static BOOL WINAPI symLoadModule(
    HANDLE hProcess,
    HANDLE hFile,
    LPSTR ImageName,
    LPSTR ModuleName,
    DWORD BaseOfDll,
    DWORD SizeOfDll);

  static DWORD WINAPI symGetModuleBase(
    HANDLE hProcess,
    DWORD dwAddr);

  static BOOL WINAPI symUnloadModule(
    HANDLE hProcess,
    DWORD BaseOfDll);

  static BOOL WINAPI symGetSymFromAddr(
    HANDLE hProcess,
    DWORD Address,
    LPDWORD Displacement,
    PIMAGEHLP_SYMBOL Symbol);

  static BOOL WINAPI symGetLineFromAddr(
    HANDLE hProcess,
    DWORD dwAddr,
    PDWORD pdwDisplacement,
    PIMAGEHLP_LINE Line);

  static DWORD WINAPI symSetOptions(
    DWORD SymOptions);

  static LPVOID WINAPI symFunctionTableAccess(
    HANDLE hProcess,
    DWORD AddrBase);

  static BOOL WINAPI stackWalk(
    DWORD MachineType,
    HANDLE hProcess,
    HANDLE hThread,
    LPSTACKFRAME StackFrame,
    LPVOID ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE GetModuleBaseRoutine,
    PTRANSLATE_ADDRESS_ROUTINE TranslateAddress);

private:

  typedef BOOL (WINAPI *SymInitialize_t) (
    HANDLE hProcess,
    LPSTR UserSearchPath,
    BOOL fInvadeProcess);

  typedef BOOL (WINAPI *SymCleanup_t) (
    HANDLE hProcess);

  typedef BOOL (WINAPI *SymLoadModule_t) (
    HANDLE hProcess,
    HANDLE hFile,
    LPSTR ImageName,
    LPSTR ModuleName,
    DWORD BaseOfDll,
    DWORD SizeOfDll);

  typedef DWORD (WINAPI *SymGetModuleBase_t) (
    HANDLE hProcess,
    DWORD dwAddr);

  typedef BOOL (WINAPI *SymUnloadModule_t) (
    HANDLE hProcess,
    DWORD BaseOfDll);

  typedef BOOL (WINAPI *SymGetSymFromAddr_t) (
    HANDLE hProcess,
    DWORD Address,
    LPDWORD Displacement,
    PIMAGEHLP_SYMBOL Symbol);

  typedef BOOL (WINAPI* SymGetLineFromAddr_t) (
    HANDLE hProcess,
    DWORD dwAddr,
    PDWORD pdwDisplacement,
    PIMAGEHLP_LINE Line);

  typedef DWORD (WINAPI *SymSetOptions_t) (
    DWORD SymOptions);

  typedef LPVOID (WINAPI *SymFunctionTableAccess_t) (
    HANDLE hProcess,
    DWORD AddrBase);

  typedef BOOL (WINAPI *StackWalk_t) (
    DWORD MachineType,
    HANDLE hProcess,
    HANDLE hThread,
    LPSTACKFRAME StackFrame,
    LPVOID ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE GetModuleBaseRoutine,
    PTRANSLATE_ADDRESS_ROUTINE TranslateAddress);

  SymInitialize_t m_symInitialize;
  SymCleanup_t m_symCleanup;
  SymLoadModule_t m_symLoadModule;
  SymUnloadModule_t m_symUnloadModule;
  SymGetModuleBase_t m_symGetModuleBase;
  SymGetSymFromAddr_t m_symGetSymFromAddr;
  SymGetLineFromAddr_t m_symGetLineFromAddr;
  SymSetOptions_t m_symSetOptions;
  SymFunctionTableAccess_t m_symFunctionTableAccess;
  StackWalk_t m_stackWalk;

  typedef std::set<HANDLE, std::less<HANDLE>, stl::malloc_allocator<HANDLE> > Processes;

  Processes m_initializedProcesses;
  HMODULE m_dllModule;
  bool m_failed;
  bool m_loadedFromSystem;
};
