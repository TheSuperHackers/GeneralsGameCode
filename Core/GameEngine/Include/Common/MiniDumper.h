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

#ifdef RTS_ENABLE_CRASHDUMP
#include <imagehlp.h>
#include "Common/MiniDumper_compat.h"

enum DumpType CPP_11(: Int)
{
	// Smallest dump type with call stacks and some supporting variables
	DUMP_TYPE_MINIMAL,
	// Large dump including all memory regions allocated by the GameMemory implementaion
	DUMP_TYPE_GAMEMEMORY,
	// Largest dump size including complete memory contents of the process
	DUMP_TYPE_FULL,
};

enum MiniDumperExitCode CPP_11(: Int)
{
	DUMPER_EXIT_SUCCESS = 0x0,
	DUMPER_EXIT_FAILURE_WAIT = 0x37DA1040,
	DUMPER_EXIT_FAILURE_PARAM = 0x4EA527BB,
	DUMPER_EXIT_FORCED_TERMINATE = 0x158B1154,
};

class MiniDumper
{
public:
	MiniDumper();
	Bool IsInitialized() const;
	void TriggerMiniDump(DumpType dumpType);
	void TriggerMiniDumpForException(struct _EXCEPTION_POINTERS* e_info, DumpType dumpType);
	static void initMiniDumper(const AsciiString& userDirPath);
	static void shutdownMiniDumper();
	static LONG WINAPI DumpingExceptionFilter(struct _EXCEPTION_POINTERS* e_info);

private:
	void Initialize(const AsciiString& userDirPath);
	void ShutDown();
	void CreateMiniDump(DumpType dumpType);
	BOOL DumpMemoryObjects(ULONG64& memoryBase, ULONG& memorySize);
	void CleanupResources();
	Bool IsDumpThreadStillRunning() const;

	// Callbacks from dbghelp
	static BOOL CALLBACK MiniDumpCallback(PVOID CallbackParam, PMINIDUMP_CALLBACK_INPUT CallbackInput, PMINIDUMP_CALLBACK_OUTPUT CallbackOutput);
	BOOL CallbackInternal(const MINIDUMP_CALLBACK_INPUT& input, MINIDUMP_CALLBACK_OUTPUT& output);

	// Thread procs
	static DWORD WINAPI MiniDumpThreadProc(LPVOID lpParam);
	DWORD ThreadProcInternal();

	// Dump file directory bookkeeping
	Bool InitializeDumpDirectory(const AsciiString& userDirPath);
	static void KeepNewestFiles(const std::string& directory, const std::string& fileWildcard, const Int keepCount);

	// Struct to hold file information
	struct FileInfo
	{
		std::string name;
		FILETIME lastWriteTime;
	};

	static bool CompareByLastWriteTime(const FileInfo& a, const FileInfo& b);

private:
	Bool m_miniDumpInitialized;
	DumpType m_requestedDumpType;

	// Path buffers
	Char m_dumpDir[MAX_PATH];
	Char m_dumpFile[MAX_PATH];
	Char m_sysDbgHelpPath[MAX_PATH];
	WideChar m_executablePath[MAX_PATH];

	// Module handles
	HMODULE m_dbgHlp;

	// Event handles
	HANDLE m_dumpRequested;
	HANDLE m_dumpComplete;
	HANDLE m_quitting;

	// Thread handles
	HANDLE m_dumpThread;
	DWORD m_dumpThreadId;

#ifndef DISABLE_GAMEMEMORY
	// Internal memory dumping progress state
	int m_dumpObjectsState;
	int m_dumpObjectsSubState;
	int m_dmaRawBlockIndex;

	AllocationRangeIterator m_rangeIter;
#endif

	// Function pointer to MiniDumpWriteDump in dbghelp.dll
	typedef BOOL(WINAPI* MiniDumpWriteDump_t)(
		HANDLE hProcess,
		DWORD ProcessId,
		HANDLE hFile,
		MINIDUMP_TYPE DumpType,
		PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
		PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
		PMINIDUMP_CALLBACK_INFORMATION CallbackParam
		);

	MiniDumpWriteDump_t m_pMiniDumpWriteDump;
};

extern MiniDumper* TheMiniDumper;
#endif
