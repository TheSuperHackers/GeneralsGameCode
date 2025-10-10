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

#include "DbgHelpLoader.h"


DbgHelpLoader* DbgHelpLoader::Inst = NULL;

DbgHelpLoader::DbgHelpLoader()
	: m_symInitialize(NULL)
	, m_symCleanup(NULL)
	, m_symLoadModule(NULL)
	, m_symUnloadModule(NULL)
	, m_symGetModuleBase(NULL)
	, m_symGetSymFromAddr(NULL)
	, m_symGetLineFromAddr(NULL)
	, m_symSetOptions(NULL)
	, m_symFunctionTableAccess(NULL)
	, m_stackWalk(NULL)
	, m_dllModule(HMODULE(0))
	, m_failed(false)
	, m_loadedFromSystem(false)
{
}

DbgHelpLoader::~DbgHelpLoader()
{
}

bool DbgHelpLoader::isLoaded()
{
	return Inst != NULL && Inst->m_dllModule != HMODULE(0);
}

bool DbgHelpLoader::isLoadedFromSystem()
{
	return Inst != NULL && Inst->m_loadedFromSystem;
}

bool DbgHelpLoader::load()
{
	if (Inst == NULL)
	{
		// Cannot use new/delete here when this is loaded during game memory initialization.
		void* p = GlobalAlloc(GMEM_FIXED, sizeof(DbgHelpLoader));
		Inst = new (p) DbgHelpLoader();
	}

	// Optimization: return early if it failed before.
	if (Inst->m_failed)
		return false;

	// Try load dbghelp.dll from the system directory first.
	char dllFilename[MAX_PATH];
	::GetSystemDirectoryA(dllFilename, ARRAY_SIZE(dllFilename));
	strlcat(dllFilename, "\\dbghelp.dll", ARRAY_SIZE(dllFilename));

	Inst->m_dllModule = ::LoadLibraryA(dllFilename);
	if (Inst->m_dllModule == HMODULE(0))
	{
		// Not found. Try load dbghelp.dll from the work directory.
		Inst->m_dllModule = ::LoadLibraryA("dbghelp.dll");
		if (Inst->m_dllModule == HMODULE(0))
		{
			Inst->m_failed = true;
			return false;
		}
	}
	else
	{
		Inst->m_loadedFromSystem = true;
	}

	Inst->m_symInitialize = reinterpret_cast<SymInitialize_t>(::GetProcAddress(Inst->m_dllModule, "SymInitialize"));
	Inst->m_symCleanup = reinterpret_cast<SymCleanup_t>(::GetProcAddress(Inst->m_dllModule, "SymCleanup"));
	Inst->m_symLoadModule = reinterpret_cast<SymLoadModule_t>(::GetProcAddress(Inst->m_dllModule, "SymLoadModule"));
	Inst->m_symUnloadModule = reinterpret_cast<SymUnloadModule_t>(::GetProcAddress(Inst->m_dllModule, "SymUnloadModule"));
	Inst->m_symGetModuleBase = reinterpret_cast<SymGetModuleBase_t>(::GetProcAddress(Inst->m_dllModule, "SymGetModuleBase"));
	Inst->m_symGetSymFromAddr = reinterpret_cast<SymGetSymFromAddr_t>(::GetProcAddress(Inst->m_dllModule, "SymGetSymFromAddr"));
	Inst->m_symGetLineFromAddr = reinterpret_cast<SymGetLineFromAddr_t>(::GetProcAddress(Inst->m_dllModule, "SymGetLineFromAddr"));
	Inst->m_symSetOptions = reinterpret_cast<SymSetOptions_t>(::GetProcAddress(Inst->m_dllModule, "SymSetOptions"));
	Inst->m_symFunctionTableAccess = reinterpret_cast<SymFunctionTableAccess_t>(::GetProcAddress(Inst->m_dllModule, "SymFunctionTableAccess"));
	Inst->m_stackWalk = reinterpret_cast<StackWalk_t>(::GetProcAddress(Inst->m_dllModule, "StackWalk"));

	if (Inst->m_symInitialize == NULL || Inst->m_symCleanup == NULL)
	{
		unload();
		Inst->m_failed = true;
		return false;
	}

	return true;
}

bool DbgHelpLoader::reload()
{
	unload();
	return load();
}

void DbgHelpLoader::unload()
{
	if (Inst == NULL)
		return;

	while (!Inst->m_initializedProcesses.empty())
	{
		symCleanup(*Inst->m_initializedProcesses.begin());
	}

	if (Inst->m_dllModule != HMODULE(0))
	{
		::FreeLibrary(Inst->m_dllModule);
		Inst->m_dllModule = HMODULE(0);
	}

	Inst->~DbgHelpLoader();
	GlobalFree(Inst);
	Inst = NULL;
}

BOOL DbgHelpLoader::symInitialize(
	HANDLE hProcess,
	LPSTR UserSearchPath,
	BOOL fInvadeProcess)
{
	if (Inst == NULL)
		return FALSE;

	if (Inst->m_initializedProcesses.find(hProcess) != Inst->m_initializedProcesses.end())
	{
		// Was already initialized.
		return TRUE;
	}

	if (Inst->m_symInitialize)
	{
		if (Inst->m_symInitialize(hProcess, UserSearchPath, fInvadeProcess) != FALSE)
		{
			// Is now initialized.
			Inst->m_initializedProcesses.insert(hProcess);
			return TRUE;
		}
	}

	return FALSE;
}

BOOL DbgHelpLoader::symCleanup(
	HANDLE hProcess)
{
	if (Inst == NULL)
		return FALSE;

	if (stl::find_and_erase(Inst->m_initializedProcesses, hProcess))
	{
		if (Inst->m_symCleanup)
		{
			return Inst->m_symCleanup(hProcess);
		}
	}

	return FALSE;
}

BOOL DbgHelpLoader::symLoadModule(
	HANDLE hProcess,
	HANDLE hFile,
	LPSTR ImageName,
	LPSTR ModuleName,
	DWORD BaseOfDll,
	DWORD SizeOfDll)
{
	if (Inst != NULL && Inst->m_symLoadModule)
		return Inst->m_symLoadModule(hProcess, hFile, ImageName, ModuleName, BaseOfDll, SizeOfDll);

	return FALSE;
}

DWORD DbgHelpLoader::symGetModuleBase(
	HANDLE hProcess,
	DWORD dwAddr)
{
	if (Inst != NULL && Inst->m_symGetModuleBase)
		return Inst->m_symGetModuleBase(hProcess, dwAddr);

	return 0u;
}

BOOL DbgHelpLoader::symUnloadModule(
	HANDLE hProcess,
	DWORD BaseOfDll)
{
	if (Inst != NULL && Inst->m_symUnloadModule)
		return Inst->m_symUnloadModule(hProcess, BaseOfDll);

	return FALSE;
}

BOOL DbgHelpLoader::symGetSymFromAddr(
	HANDLE hProcess,
	DWORD Address,
	LPDWORD Displacement,
	PIMAGEHLP_SYMBOL Symbol)
{
	if (Inst != NULL && Inst->m_symGetSymFromAddr)
		return Inst->m_symGetSymFromAddr(hProcess, Address, Displacement, Symbol);

	return FALSE;
}

BOOL DbgHelpLoader::symGetLineFromAddr(
	HANDLE hProcess,
	DWORD dwAddr,
	PDWORD pdwDisplacement,
	PIMAGEHLP_LINE Line)
{
	if (Inst != NULL && Inst->m_symGetLineFromAddr)
		return Inst->m_symGetLineFromAddr(hProcess, dwAddr, pdwDisplacement, Line);

	return FALSE;
}

DWORD DbgHelpLoader::symSetOptions(
	DWORD SymOptions)
{
	if (Inst != NULL && Inst->m_symSetOptions)
		return Inst->m_symSetOptions(SymOptions);

	return 0u;
}

LPVOID DbgHelpLoader::symFunctionTableAccess(
	HANDLE hProcess,
	DWORD AddrBase)
{
	if (Inst != NULL && Inst->m_symFunctionTableAccess)
		return Inst->m_symFunctionTableAccess(hProcess, AddrBase);

	return NULL;
}

BOOL DbgHelpLoader::stackWalk(
	DWORD MachineType,
	HANDLE hProcess,
	HANDLE hThread,
	LPSTACKFRAME StackFrame,
	LPVOID ContextRecord,
	PREAD_PROCESS_MEMORY_ROUTINE ReadMemoryRoutine,
	PFUNCTION_TABLE_ACCESS_ROUTINE FunctionTableAccessRoutine,
	PGET_MODULE_BASE_ROUTINE GetModuleBaseRoutine,
	PTRANSLATE_ADDRESS_ROUTINE TranslateAddress)
{
	if (Inst != NULL && Inst->m_stackWalk)
		return Inst->m_stackWalk(MachineType, hProcess, hThread, StackFrame, ContextRecord, ReadMemoryRoutine, FunctionTableAccessRoutine, GetModuleBaseRoutine, TranslateAddress);

	return FALSE;
}
